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

#include "core/components_ng/relaxed_interaction/parsers/click_command_parser.h"

#include <string>
#include <vector>

#include "base/geometry/ng/point_t.h"
#include "base/log/log.h"
#include "core/components_ng/relaxed_interaction/executors/mouse_click_executor.h"
#include "core/components_ng/relaxed_interaction/executors/strict_click_executor.h"
#include "core/components_ng/relaxed_interaction/executors/simulate_touch_executor.h"

namespace OHOS::Ace::NG {
namespace {
constexpr int32_t MAX_CLICK_COUNT = 3;

bool ParseMouseButton(const JsonValue& source, ClickInput& input)
{
    const bool hasMouseButton = source.Contains("mouse_button");
    if (!hasMouseButton && !source.Contains("button")) {
        return true;
    }
    auto button = source.GetValue(hasMouseButton ? "mouse_button" : "button");
    if (!button || !button->IsString()) {
        return false;
    }
    if (hasMouseButton && source.Contains("button")) {
        auto legacyButton = source.GetValue("button");
        if (!legacyButton->IsString() || legacyButton->GetString() != button->GetString()) {
            return false;
        }
    }
    input.button = button->GetString();
    if (input.button != "left" && input.button != "right" && input.button != "middle" &&
        input.button != "forward" && input.button != "back") {
        return false;
    }
    input.hasButton = true;
    return true;
}

bool ParseClickCount(const JsonValue& source, ClickInput& input)
{
    if (!source.Contains("click_count")) {
        return true;
    }
    auto count = source.GetValue("click_count");
    if (!count || !count->IsNumber() || count->GetDouble() != count->GetInt()) {
        return false;
    }
    input.clickCount = count->GetInt();
    if (input.clickCount < 1 || input.clickCount > MAX_CLICK_COUNT) {
        return false;
    }
    // Only an explicit button switches to the mouse device; click_count alone keeps the
    // touch-compatible path per the requirement.
    return true;
}

bool AppendModifiers(const JsonValue& modifier, std::vector<std::string>& modifiers)
{
    if (modifier.IsString()) {
        modifiers.emplace_back(modifier.GetString());
        return true;
    }
    if (!modifier.IsArray()) {
        return false;
    }
    for (int32_t i = 0; i < modifier.GetArraySize(); ++i) {
        auto item = modifier.GetArrayItem(i);
        if (!item || !item->IsString()) {
            return false;
        }
        modifiers.emplace_back(item->GetString());
    }
    return true;
}

bool ParseClickModifiers(const JsonValue& source, ClickInput& input)
{
    const char* modifierKey = source.Contains("modifier") ? "modifier" : "modifiers";
    if (!source.Contains(modifierKey)) {
        return true;
    }
    auto modifier = source.GetValue(modifierKey);
    if (!modifier || !AppendModifiers(*modifier, input.modifiers)) {
        return false;
    }
    for (const auto& value : input.modifiers) {
        if (value != "ctrl" && value != "shift" && value != "alt") {
            return false;
        }
    }
    // Modifiers alone must not switch the input device; only an explicit button does.
    return true;
}
} // namespace

ClickCommandParser::ClickCommandParser(WeakPtr<PipelineContext> context) : BaseCommandParser(context) {}

std::vector<std::unique_ptr<BaseExecutor>> ClickCommandParser::Parse(const std::unique_ptr<JsonValue>& json)
{
    ExecuteMode mode = ExecuteMode::AUTO;
    PointF coordinates(0.0f, 0.0f);
    if (!ParseParameters(json, mode, coordinates)) {
        return {};
    }
    // PC mouse options route to the dedicated mouse click executor; button remains a compatibility alias.
    ClickInput pcInput;
    pcInput.coordinates = coordinates;
    if (!ParsePcOptions(json.get(), pcInput)) {
        return {};
    }
    if (pcInput.hasButton) {
        std::vector<std::unique_ptr<BaseExecutor>> executors;
        executors.emplace_back(std::make_unique<MouseClickExecutor>(context_, pcInput));
        return executors;
    }
    // click_count also drives the touch-compatible path: the touch executors repeat
    // their click per frame-paced step (the parsed pcInput would otherwise be dropped).
    return CreateExecutor(mode, coordinates, pcInput.clickCount);
}

bool ClickCommandParser::ParsePcOptions(const JsonValue* cmdJson, ClickInput& input)
{
    const auto* source = cmdJson;
    auto actionInfo = cmdJson->GetValue("action_info");
    if (actionInfo && actionInfo->IsObject()) {
        source = actionInfo.get();
    }
    if (!source->Contains("mouse_button") && !source->Contains("button") && !source->Contains("click_count") &&
        !source->Contains("modifier") && !source->Contains("modifiers")) {
        return true;
    }
    return ParseMouseButton(*source, input) && ParseClickCount(*source, input) && ParseClickModifiers(*source, input);
}

bool ClickCommandParser::Validate(const std::unique_ptr<JsonValue>& json)
{
    ExecuteMode mode = ExecuteMode::AUTO;
    PointF coordinates(0.0f, 0.0f);
    return ParseParameters(json, mode, coordinates);
}

bool ClickCommandParser::ParseParameters(
    const std::unique_ptr<JsonValue>& json, ExecuteMode& mode, PointF& coordinates)
{
    if (!json || !json->IsObject()) {
        TAG_LOGE(AceLogTag::ACE_UIEVENT, "Invalid json object");
        return false;
    }

    if (!ParseMode(json.get(), mode)) {
        TAG_LOGE(AceLogTag::ACE_UIEVENT, "Invalid mode, must be one of: auto, simulating, target");
        return false;
    }

    // action_info.coordinates is required for all modes
    if (!ParseCoordinates(json.get(), coordinates)) {
        TAG_LOGE(AceLogTag::ACE_UIEVENT, "Failed to parse required action_info.coordinates");
        return false;
    }

    TAG_LOGD(AceLogTag::ACE_UIEVENT, "Parsed click command: mode=%{public}d", mode);

    return true;
}

bool ClickCommandParser::ParseMode(const JsonValue* cmdJson, ExecuteMode& mode)
{
    if (!cmdJson->Contains("mode")) {
        mode = ExecuteMode::AUTO;
        return true;
    }

    std::string modeStr = cmdJson->GetString("mode");
    if (modeStr == "simulating") {
        mode = ExecuteMode::SIMULATING;
        return true;
    } else if (modeStr == "target") {
        mode = ExecuteMode::TARGET;
        return true;
    } else if (modeStr == "auto") {
        mode = ExecuteMode::AUTO;
        return true;
    }

    return false;
}

bool ClickCommandParser::ParseCoordinates(const JsonValue* cmdJson, PointF& coordinates)
{
    auto actionInfoJson = cmdJson->GetValue("action_info");
    if (!actionInfoJson || !actionInfoJson->IsObject()) {
        TAG_LOGE(AceLogTag::ACE_UIEVENT, "Missing or invalid action_info object");
        return false;
    }

    auto coordinatesJson = actionInfoJson->GetValue("coordinates");
    if (!coordinatesJson || !coordinatesJson->IsObject()) {
        TAG_LOGE(AceLogTag::ACE_UIEVENT, "Missing or invalid coordinates object");
        return false;
    }

    auto x1Value = coordinatesJson->GetValue("x1");
    auto y1Value = coordinatesJson->GetValue("y1");
    if (!x1Value || !y1Value) {
        TAG_LOGE(AceLogTag::ACE_UIEVENT, "Missing required coordinates x1 or y1");
        return false;
    }

    if (!x1Value->IsNumber() || !y1Value->IsNumber()) {
        TAG_LOGE(AceLogTag::ACE_UIEVENT, "Coordinates x1 or y1 is not a valid number");
        return false;
    }

    float x1 = x1Value->GetDouble();
    float y1 = y1Value->GetDouble();
    coordinates = PointF(x1, y1);
    return true;
}

std::vector<std::unique_ptr<BaseExecutor>> ClickCommandParser::CreateExecutor(
    ExecuteMode mode, const PointF& coordinates, int32_t clickCount)
{
    std::vector<std::unique_ptr<BaseExecutor>> executors;

    if (mode == ExecuteMode::SIMULATING) {
        TAG_LOGD(AceLogTag::ACE_UIEVENT, "Creating SimulateTouchExecutor for simulating mode");
        executors.emplace_back(std::make_unique<SimulateTouchExecutor>(context_, coordinates, clickCount));
    } else if (mode == ExecuteMode::AUTO) {
        // For auto mode, create both StrictClickExecutor and SimulateTouchExecutor
        // StrictClickExecutor goes first, SimulateTouchExecutor as fallback
        TAG_LOGD(AceLogTag::ACE_UIEVENT, "Creating StrictClickExecutor and SimulateTouchExecutor for auto mode");
        executors.emplace_back(std::make_unique<StrictClickExecutor>(context_, coordinates, clickCount));
        executors.emplace_back(std::make_unique<SimulateTouchExecutor>(context_, coordinates, clickCount));
    } else if (mode == ExecuteMode::TARGET) {
        // For target mode, only create StrictClickExecutor
        TAG_LOGD(AceLogTag::ACE_UIEVENT, "Creating StrictClickExecutor for target mode");
        executors.emplace_back(std::make_unique<StrictClickExecutor>(context_, coordinates, clickCount));
    }

    return executors;
}

} // namespace OHOS::Ace::NG

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


#include <algorithm>
#include <charconv>
#include <initializer_list>
#include <iterator>

#include "base/log/dump_log.h"
#include "core/accessibility/hidumper/accessibility_hidumper.h"
#include "core/components_ng/base/frame_node.h"

namespace OHOS::Ace {

namespace {
constexpr size_t FOCUS_SEARCH_COMMAND_INDEX = 1;
constexpr size_t FOCUS_SEARCH_VALUES_BEGIN = FOCUS_SEARCH_COMMAND_INDEX + 1;

// Dump protocol values mirror accessibility_def.h without an OHOS dependency in cross-platform core code.
enum class DumpFocusDirection : int64_t {
    FORWARD = 16,
    BACKWARD = 32,
    FIND_LAST = 64,
    GET_FORWARD_SCROLL_ANCESTOR = 128,
    GET_BACKWARD_SCROLL_ANCESTOR = 256,
    GET_SCROLLABLE_ANCESTOR = 512,
};
enum class DumpFocusRule : int64_t {
    DEFAULT = 1,
    FOCUS_BY_LINK = 2,
    FOCUS_BY_TITLE = 3,
};
enum class DumpFocusCondition : int64_t {
    BYPASS_SELF = 1,
    BYPASS_SELF_DESCENDANTS = 2,
    CHECK_SELF = 4,
    CHECK_SELF_BYPASS_DESCENDANTS = 8,
};

struct FocusSearchValues {
    int64_t nodeId = -1;
    int64_t direction = 0;
    int64_t rule = 0;
    int64_t condition = 0;
};

template<typename Enum>
bool IsSupportedFocusValue(int64_t value, std::initializer_list<Enum> supported)
{
    return std::any_of(supported.begin(), supported.end(), [value](Enum candidate) {
        return value == static_cast<int64_t>(candidate);
    });
}

bool ParseFocusSearchValues(const std::vector<std::string>& params, FocusSearchValues& values)
{
    int64_t* fields[] = { &values.nodeId, &values.direction, &values.rule, &values.condition };
    if (params.size() != FOCUS_SEARCH_VALUES_BEGIN + std::size(fields) ||
        params[FOCUS_SEARCH_COMMAND_INDEX] != "--third-focus-search") {
        DumpLog::GetInstance().Print(
            "Usage: -accessibility --third-focus-search <splicedId> <direction:16|32|64|128|256|512> "
            "<rule:1|2|3> <condition:1|2|4|8>");
        return false;
    }
    auto arg = params.begin() + FOCUS_SEARCH_VALUES_BEGIN;
    for (auto* field : fields) {
        const auto& text = *arg;
        auto result = std::from_chars(text.data(), text.data() + text.size(), *field);
        if (result.ec != std::errc() || result.ptr != text.data() + text.size()) {
            return false;
        }
        ++arg;
    }
    return true;
}
} // namespace

bool AccessibilityHidumper::ParseThirdFocusSearch(
    const std::vector<std::string>& params, DumpInfoArgument& argument)
{
    FocusSearchValues values;
    if (!ParseFocusSearchValues(params, values) || values.nodeId < 0 ||
        !IsSupportedFocusValue(values.direction, { DumpFocusDirection::FORWARD, DumpFocusDirection::BACKWARD,
            DumpFocusDirection::FIND_LAST, DumpFocusDirection::GET_FORWARD_SCROLL_ANCESTOR,
            DumpFocusDirection::GET_BACKWARD_SCROLL_ANCESTOR, DumpFocusDirection::GET_SCROLLABLE_ANCESTOR }) ||
        !IsSupportedFocusValue(values.rule,
            { DumpFocusRule::DEFAULT, DumpFocusRule::FOCUS_BY_LINK, DumpFocusRule::FOCUS_BY_TITLE }) ||
        !IsSupportedFocusValue(values.condition, { DumpFocusCondition::BYPASS_SELF,
            DumpFocusCondition::BYPASS_SELF_DESCENDANTS, DumpFocusCondition::CHECK_SELF,
            DumpFocusCondition::CHECK_SELF_BYPASS_DESCENDANTS })) {
        return false;
    }
    argument.mode = DumpMode::THIRD_FOCUS_SEARCH;
    argument.nodeId = values.nodeId;
    argument.focusDirection = static_cast<int32_t>(values.direction);
    argument.focusMoveRule = static_cast<int32_t>(values.rule);
    argument.focusCondition = static_cast<int32_t>(values.condition);
    return true;
}

bool AccessibilityHidumper::DumpProcessInjectActionParameters(
    const std::vector<std::string>& params,
    int64_t& nodeId,
    int32_t& result,
    InjectActionType& actionType)
{
    constexpr int32_t NUM_PARAMETERS_DIMENSION = 1;
    if (params.size() < 1) {
        return false;
    }

    for (auto arg = params.begin(); arg != params.end(); ++arg) {
        if (*arg == "--inject-action") {
            if (std::distance(arg, params.end()) <= NUM_PARAMETERS_DIMENSION) {
                DumpLog::GetInstance().Print(std::string("Error: parameters need with data"));
                return false;
            }
            ++arg;
            nodeId = StringUtils::StringToLongInt(*arg, -1);
        } else if (*arg == "--NotifyChildAction") {
            if (std::distance(arg, params.end()) <= NUM_PARAMETERS_DIMENSION) {
                DumpLog::GetInstance().Print(std::string("Error: parameters need with data"));
                return false;
            }
            ++arg;
            result = StringUtils::StringToInt(*arg, 0);
            actionType = InjectActionType::NOTIFY_CHILD_ACTION;
            return true;
        }  else if (*arg == "--SecurityClickAction") {
            actionType = InjectActionType::SECURITY_CLICK_ACTION;
            return true;
        } else if (*arg == "--CustomActionTest") {
            if (std::distance(arg, params.end()) <= NUM_PARAMETERS_DIMENSION) {
                DumpLog::GetInstance().Print(std::string("Error: parameters need with action name"));
                return false;
            }
            ++arg;
            result = StringUtils::StringToInt(*arg, 0);
            actionType = InjectActionType::CUSTOM_ACTION_TEST;
            return true;
        }
    }
    return false;
}

bool AccessibilityHidumper::DumpProcessCustomActionParameters(
    const std::vector<std::string>& params,
    int64_t& nodeId,
    std::string& actionName,
    bool& listActions)
{
    constexpr int32_t NUM_PARAMETERS_DIMENSION = 1;
    if (params.size() < 1) {
        return false;
    }

    listActions = false;
    actionName.clear();
    bool result = false;
    for (auto arg = params.begin(); arg != params.end(); ++arg) {
        if (*arg == "--custom-action-test") {
            if (std::distance(arg, params.end()) <= NUM_PARAMETERS_DIMENSION) {
                DumpLog::GetInstance().Print(std::string("Error: parameters need with node id"));
                return false;
            }
            ++arg;
            nodeId = StringUtils::StringToLongInt(*arg, -1);
            result = true;
        } else if (*arg == "--execute") {
            if (std::distance(arg, params.end()) <= NUM_PARAMETERS_DIMENSION) {
                DumpLog::GetInstance().Print(std::string("Error: parameters need with action name"));
                return false;
            }
            ++arg;
            actionName = *arg;
        } else if (*arg == "--list") {
            listActions = true;
        }
    }
    return result;
}

bool AccessibilityHidumper::DumpProcessExecuteActionParameters(
    const std::vector<std::string>& params,
    ExecuteActionArgument& argument)
{
    constexpr size_t MIN_EXECUTE_ACTION_PARAMS = 2;
    for (auto arg = params.begin(); arg != params.end(); ++arg) {
        if (*arg != "--execute-action") {
            continue;
        }
        if (std::distance(arg, params.end()) <= static_cast<int32_t>(MIN_EXECUTE_ACTION_PARAMS)) {
            DumpLog::GetInstance().Print("Error: --execute-action needs elementId and actionType");
            return false;
        }
        ++arg;
        argument.elementId = StringUtils::StringToLongInt(*arg, -1);
        ++arg;
        argument.actionType = StringUtils::StringToInt(*arg, 0);
        while (std::distance(arg, params.end()) > 1) {
            ++arg;
            std::string key = *arg;
            if (std::distance(arg, params.end()) <= 1) {
                DumpLog::GetInstance().Print("Error: key without value");
                return false;
            }
            ++arg;
            argument.actionArguments[key] = *arg;
        }
        return true;
    }
    return false;
}
}  // namespace OHOS::Ace

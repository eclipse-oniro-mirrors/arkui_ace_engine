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

#include "core/components_ng/relaxed_interaction/parsers/key_command_parser.h"

#include <cmath>
#include <limits>
#include <memory>
#include <vector>

#include "base/log/log.h"
#include "core/event/key_event_recognizer.h"
#include "base/memory/referenced.h"
#include "core/components_ng/relaxed_interaction/executors/key_executor.h"

namespace OHOS::Ace::NG {

KeyCommandParser::KeyCommandParser(WeakPtr<PipelineContext> context) : BaseCommandParser(context) {}

std::vector<std::unique_ptr<BaseExecutor>> KeyCommandParser::Parse(const std::unique_ptr<JsonValue>& json)
{
    int32_t keyCode = 0;
    if (!ParseKeyCode(json, keyCode)) {
        return {};
    }
    std::vector<std::unique_ptr<BaseExecutor>> executors;
    executors.emplace_back(std::make_unique<KeyExecutor>(context_, keyCode));
    return executors;
}

bool KeyCommandParser::Validate(const std::unique_ptr<JsonValue>& json)
{
    int32_t keyCode = 0;
    return ParseKeyCode(json, keyCode);
}

bool KeyCommandParser::ParseKeyCode(const std::unique_ptr<JsonValue>& json, int32_t& keyCode)
{
    if (!json || !json->IsObject()) {
        TAG_LOGE(AceLogTag::ACE_UIEVENT, "Invalid json object");
        return false;
    }

    auto actionInfo = json->GetValue("action_info");
    if (!actionInfo || !actionInfo->IsObject()) {
        TAG_LOGE(AceLogTag::ACE_UIEVENT, "Missing or invalid action_info object");
        return false;
    }

    auto keyCodeJson = actionInfo->GetValue("code");
    if (!keyCodeJson || !keyCodeJson->IsNumber()) {
        TAG_LOGE(AceLogTag::ACE_UIEVENT, "Missing or invalid code");
        return false;
    }

    double keyCodeValue = keyCodeJson->GetDouble();
    if (!std::isfinite(keyCodeValue) || std::trunc(keyCodeValue) != keyCodeValue ||
        keyCodeValue < std::numeric_limits<int32_t>::min() || keyCodeValue > std::numeric_limits<int32_t>::max()) {
        TAG_LOGE(AceLogTag::ACE_UIEVENT, "code must be a 32-bit integer");
        return false;
    }
    keyCode = static_cast<int32_t>(keyCodeValue);
    if (!IsValidKeyCode(keyCode)) {
        return false;
    }
    TAG_LOGD(AceLogTag::ACE_UIEVENT, "Parsed key command");
    return true;
}

} // namespace OHOS::Ace::NG

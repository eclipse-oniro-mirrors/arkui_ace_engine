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

#include "core/components_ng/relaxed_interaction/executor_generator.h"

#include "base/log/log.h"
#include "core/components_ng/relaxed_interaction/parsers/backpress_command_parser.h"
#include "core/components_ng/relaxed_interaction/parsers/click_command_parser.h"
#include "core/components_ng/relaxed_interaction/parsers/content_switch_command_parser.h"
#include "core/components_ng/relaxed_interaction/parsers/fallback_command_parser.h"
#include "core/components_ng/relaxed_interaction/parsers/key_command_parser.h"
#include "core/components_ng/relaxed_interaction/parsers/scroll_command_parser.h"
#include "core/components_ng/relaxed_interaction/parsers/tap_command_parser.h"
#include "core/components_ng/relaxed_interaction/parsers/slide_command_parser.h"

namespace OHOS::Ace::NG {

ExecutorGenerator::ExecutorGenerator(WeakPtr<PipelineContext> context)
    : context_(context)
{
    RegisterDefaultParsers();
}

ExecutorGenerator::~ExecutorGenerator() = default;

void ExecutorGenerator::RegisterDefaultParsers()
{
    parserRegistry_[BACKPRESS] = std::make_unique<BackpressCommandParser>(context_);
    parserRegistry_[FALLBACK] = std::make_unique<FallbackCommandParser>(context_);
    parserRegistry_[CLICK] = std::make_unique<ClickCommandParser>(context_);
    parserRegistry_[CONTENT_SWITCH] = std::make_unique<ContentSwitchCommandParser>(context_);
    parserRegistry_[SCROLL] = std::make_unique<ScrollCommandParser>(context_);
    parserRegistry_[TAP] = std::make_unique<TapCommandParser>(context_);
    parserRegistry_[SLIDE] = std::make_unique<SlideCommandParser>(context_);
    parserRegistry_[KEY] = std::make_unique<KeyCommandParser>(context_);
}

bool ExecutorGenerator::ValidateCommand(const std::unique_ptr<JsonValue>& json)
{
    if (!json || !json->IsObject()) {
        return false;
    }
    auto cmd = json->GetValue(CMD_KEY);
    if (!cmd || !cmd->IsObject()) {
        return false;
    }
    const auto type = cmd->GetString(TYPE_KEY);
    if (type == CLICK) {
        return ClickCommandParser::Validate(cmd);
    }
    if (type == CONTENT_SWITCH) {
        return ContentSwitchCommandParser::Validate(cmd);
    }
    if (type == SCROLL) {
        return ScrollCommandParser::Validate(cmd);
    }
    if (type == KEY) {
        return KeyCommandParser::Validate(cmd);
    }
    return type == BACKPRESS;
}

std::vector<std::unique_ptr<BaseExecutor>> ExecutorGenerator::ParseCommand(const std::unique_ptr<JsonValue>& json)
{
    if (json->Contains(FALLBACK_CMD_KEY)) {
        TAG_LOGD(AceLogTag::ACE_UIEVENT, "parse fallback command");
        return ParseFallbackCmd(json);
    }
    TAG_LOGD(AceLogTag::ACE_UIEVENT, "parse regular command");
    return ParseRegularCmd(json);
}

std::vector<std::unique_ptr<BaseExecutor>> ExecutorGenerator::ParseFallbackCmd(const std::unique_ptr<JsonValue>& json)
{
    if (parserRegistry_.find(FALLBACK) == parserRegistry_.end()) {
        return {};
    }

    auto jsonCmd = json->GetValue(FALLBACK_CMD_KEY);
    if (!jsonCmd || !jsonCmd->IsValid() || !jsonCmd->IsObject()) {
        return {};
    }
    return parserRegistry_[FALLBACK]->Parse(jsonCmd);
}

std::vector<std::unique_ptr<BaseExecutor>> ExecutorGenerator::ParseRegularCmd(const std::unique_ptr<JsonValue>& json)
{
    auto jsonCmd = json->GetValue(CMD_KEY);
    if (!jsonCmd || !jsonCmd->IsValid() || !jsonCmd->IsObject()) {
        return {};
    }

    std::string type = jsonCmd->GetString(TYPE_KEY);
    auto parserIt = parserRegistry_.find(type);
    if (parserIt == parserRegistry_.end()) {
        return {};
    }

    return parserIt->second->Parse(jsonCmd);
}
} // namespace OHOS::Ace::NG

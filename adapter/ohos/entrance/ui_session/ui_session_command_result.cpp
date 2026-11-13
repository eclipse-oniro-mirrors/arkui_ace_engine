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

#include "adapter/ohos/entrance/ui_session/ui_session_command_result.h"

#include "adapter/ohos/entrance/ui_session/ui_session_batch_command.h"
#include "base/utils/string_utils.h"
#include "core/components_ng/pattern/pattern.h"

namespace OHOS::Ace {
namespace {
constexpr size_t MAX_COMMAND_MESSAGE_BYTES = 128;
} // namespace

std::string SerializeCommandResult(const NG::CommandResult& result)
{
    if (result.success) {
        return R"({"result":"SUCCESS"})";
    }
    const char* type = "EXECUTE_FAIL";
    switch (result.errorType) {
        case NG::CommandErrorType::NODE_NOT_FOUND: type = "NODE_NOT_FOUND"; break;
        case NG::CommandErrorType::INVALID_COMMAND: type = "INVALID_COMMAND"; break;
        case NG::CommandErrorType::INVALID_PARAM: type = "INVALID_PARAM"; break;
        default: break;
    }
    auto message = result.errorMessage;
    if (message.size() > MAX_COMMAND_MESSAGE_BYTES) {
        message.clear();
    }
    return std::string(R"({"result":"FAILED","errorType":")") + type +
        "\",\"errorMessage\":" + StringUtils::EscapeJsonString(message) + "}";
}
std::string UiSessionBatchCommand::ExecuteWithResult(const BatchParseResult& parsed,
    const std::function<BatchResolvedTarget(int32_t)>& resolveTarget)
{
    using NG::CommandErrorType;
    using NG::CommandResult;
    if (parsed.status != BatchParseStatus::VALID_BATCH) {
        return SerializeCommandResult(CommandResult::Error(CommandErrorType::INVALID_PARAM));
    }
    bool success = true;
    std::string entries;
    for (const auto& item : parsed.resultItems) {
        auto result = CommandResult::Success();
        if (item.semanticError == BatchDiagnosticReason::INVALID_NODE_ID ||
            item.semanticError == BatchDiagnosticReason::DUPLICATE_NODE_ID) {
            result = CommandResult::Error(CommandErrorType::INVALID_PARAM);
        } else {
            auto target = resolveTarget ? resolveTarget(item.nodeId) : BatchResolvedTarget {};
            if (target.status == BatchTargetStatus::NOT_FOUND) {
                result = CommandResult::Error(CommandErrorType::NODE_NOT_FOUND);
            } else if (target.status == BatchTargetStatus::UNSUPPORTED_TYPE) {
                result = CommandResult::Error(CommandErrorType::INVALID_COMMAND);
            } else if (item.semanticError == BatchDiagnosticReason::UNSUPPORTED_CMD) {
                result = CommandResult::Error(CommandErrorType::INVALID_COMMAND);
            } else if (item.semanticError != BatchDiagnosticReason::NONE) {
                result = CommandResult::Error(CommandErrorType::INVALID_PARAM);
            } else if (!target.execute || !target.execute(item.canonicalCommand)) {
                result = CommandResult::Error(CommandErrorType::EXECUTE_FAIL);
            }
        }
        success = success && result.success;
        if (!entries.empty()) {
            entries += ",";
        }
        auto json = SerializeCommandResult(result);
        auto key = item.semanticError == BatchDiagnosticReason::INVALID_NODE_ID
            ? "\"nodeKey\":" + StringUtils::EscapeJsonString(item.nodeKey)
            : "\"nodeId\":" + std::to_string(item.nodeId);
        entries += "{" + key + "," + json.substr(1);
    }
    return std::string("{\"result\":\"") + (success ? "SUCCESS" : "FAILED") +
        "\",\"results\":[" + entries + "]}";
}

} // namespace OHOS::Ace

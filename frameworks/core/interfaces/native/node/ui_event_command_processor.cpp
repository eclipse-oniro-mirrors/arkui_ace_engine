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

#include "ui_event_injection.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_set>

#include "base/log/log_wrapper.h"
#include "base/thread/task_executor.h"
#include "core/common/container.h"
#include "core/common/container_scope.h"
#include "core/components_ng/base/frame_node.h"
#include "interfaces/native/native_type.h"
#include "core/interfaces/native/utility/error_message_macros.h"
#include "core/pipeline/base/element_register.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
namespace {

const std::string TAG = "UIEventInjection";

const std::unordered_set<std::string> SUPPORTED_NODE_TAGS = {
    "TextInput", "TextArea", "Text", "RichEditor", "Swiper", "Tabs"
};

enum class CommandPhase { QUEUED, EXECUTING, COMPLETION_PENDING, COMPLETED, CANCELLED };

struct CommandState {
    uint64_t token = 0;
    int32_t instanceId = 0;
    WeakPtr<Container> owner;
    CommandPhase phase = CommandPhase::QUEUED;
    bool cancelRequested = false;
};

struct CommandContext {
    int32_t instanceId = 0;
    uint32_t uniqueId = 0;
    WeakPtr<PipelineContext> pipeline;
    RefPtr<TaskExecutor> taskExecutor;
    std::string convertedJson;
    void (*callback)(ArkUI_Int32, void*) = nullptr;
    void* userData = nullptr;
    std::shared_ptr<CommandState> state;
};

OH_ArkUI_NativeModule_UIEventInjection_ResultCode MapErrorCodeToResultCode(ArkUI_ErrorCode code)
{
    switch (code) {
        case ARKUI_ERROR_CODE_NO_ERROR:
            return OH_ARKUI_NATIVE_MODULE_UI_EVENT_INJECTION_RESULT_SUCCESS;
        case ARKUI_ERROR_CODE_NODE_NOT_FOUND:
        case ARKUI_ERROR_CODE_UI_CONTEXT_INVALID:
            return OH_ARKUI_NATIVE_MODULE_UI_EVENT_INJECTION_RESULT_TARGET_NOT_FOUND;
        default:
            return OH_ARKUI_NATIVE_MODULE_UI_EVENT_INJECTION_RESULT_COMMAND_NOT_SUPPORTED;
    }
}

std::mutex g_commandMutex;
std::shared_ptr<CommandState> g_currentCommand;
uint64_t g_nextCommandToken = 0;

// Called with g_commandMutex held. State identity also protects against token wraparound.
bool IsCurrentCommand(const std::shared_ptr<CommandState>& state)
{
    return state && g_currentCommand == state && g_currentCommand->token == state->token;
}

// Called with g_commandMutex held after checking or reclaiming the process slot.
std::shared_ptr<CommandState> CreateCurrentCommand(int32_t instanceId, const RefPtr<Container>& owner)
{
    auto state = std::make_shared<CommandState>();
    state->token = ++g_nextCommandToken;
    state->instanceId = instanceId;
    state->owner = owner;
    g_currentCommand = state;
    return state;
}

std::shared_ptr<CommandState> TryAcquireCommand(int32_t instanceId, const RefPtr<Container>& owner)
{
    std::shared_ptr<CommandState> previous;
    {
        std::lock_guard<std::mutex> lock(g_commandMutex);
        if (!g_currentCommand) {
            return CreateCurrentCommand(instanceId, owner);
        }
        previous = g_currentCommand;
    }
    // The registry query has its own lock. Never read another UI thread's Pipeline fields here.
    auto registeredOwner = Container::GetContainer(previous->instanceId);
    auto originalOwner = previous->owner.Upgrade();
    bool ownerIsRegistered = originalOwner && registeredOwner == originalOwner;
    std::lock_guard<std::mutex> lock(g_commandMutex);
    if (!g_currentCommand) {
        return CreateCurrentCommand(instanceId, owner);
    }
    // A different command won the slot during the lookup. Do not retry against its owner.
    if (!IsCurrentCommand(previous) || ownerIsRegistered) {
        return nullptr;
    }
    if (previous->phase == CommandPhase::EXECUTING) {
        previous->cancelRequested = true;
        return nullptr;
    }
    previous->phase = CommandPhase::CANCELLED;
    return CreateCurrentCommand(instanceId, owner);
}

void CancelCommand(const std::shared_ptr<CommandState>& state)
{
    std::lock_guard<std::mutex> lock(g_commandMutex);
    if (IsCurrentCommand(state)) {
        state->phase = CommandPhase::CANCELLED;
        g_currentCommand.reset();
    }
}

bool BeginCommand(const std::shared_ptr<CommandState>& state)
{
    std::lock_guard<std::mutex> lock(g_commandMutex);
    if (!IsCurrentCommand(state) || state->phase != CommandPhase::QUEUED) {
        return false;
    }
    state->phase = CommandPhase::EXECUTING;
    return true;
}

bool PrepareCompletion(const std::shared_ptr<CommandState>& state)
{
    std::lock_guard<std::mutex> lock(g_commandMutex);
    if (!IsCurrentCommand(state) || state->phase != CommandPhase::EXECUTING) {
        return false;
    }
    if (state->cancelRequested) {
        state->phase = CommandPhase::CANCELLED;
        g_currentCommand.reset();
        return false;
    }
    state->phase = CommandPhase::COMPLETION_PENDING;
    return true;
}

bool CompleteOnce(const std::shared_ptr<CommandState>& state)
{
    std::lock_guard<std::mutex> lock(g_commandMutex);
    if (!IsCurrentCommand(state) || state->phase != CommandPhase::COMPLETION_PENDING) {
        return false;
    }
    // Claim callback execution before releasing the slot; a claimed callback is not cancellable.
    state->phase = CommandPhase::COMPLETED;
    g_currentCommand.reset();
    return true;
}

RefPtr<FrameNode> ResolveFrameNodeRefPtr(uint32_t uniqueId)
{
    return AceType::DynamicCast<FrameNode>(OHOS::Ace::ElementRegister::GetInstance()->GetNodeById(uniqueId));
}

void BuildTextCommandJson(const std::string& cmdType, const std::unique_ptr<JsonValue>& actionInfo,
    const std::unique_ptr<JsonValue>& resultJson)
{
    auto params = OHOS::Ace::JsonUtil::Create(true);
    auto value = actionInfo->GetString("value");
    if (cmdType == "setText" || cmdType == "addText") {
        params->Put("value", value.c_str());
    }
    if (cmdType == "addText" && actionInfo->Contains("offset")) {
        params->Put("offset", actionInfo->GetInt("offset"));
    }
    if (cmdType == "deleteText") {
        params->Put("start", actionInfo->Contains("start") ? std::max(0, actionInfo->GetInt("start")) : 0);
        params->Put("end", actionInfo->Contains("end") ? std::max(0, actionInfo->GetInt("end")) : 0);
    }
    resultJson->Put("params", params);
}

void BuildSelectTextCommandJson(const std::unique_ptr<JsonValue>& actionInfo,
    const std::unique_ptr<JsonValue>& resultJson)
{
    resultJson->Put("selectionStart", actionInfo->GetInt("selectionStart"));
    resultJson->Put("selectionEnd", actionInfo->GetInt("selectionEnd"));
}

void BuildSwiperSwitchCommandJson(const std::unique_ptr<JsonValue>& actionInfo,
    const std::unique_ptr<JsonValue>& resultJson)
{
    auto params = OHOS::Ace::JsonUtil::Create(true);
    auto direction = actionInfo->GetString("direction");
    params->Put("type", direction.c_str());
    if (direction == "index") {
        params->Put("index", std::to_string(actionInfo->GetInt("index")).c_str());
    }
    resultJson->Replace("cmd", "change");
    resultJson->Put("params", params);
}

void BuildTabsSwitchCommandJson(const std::unique_ptr<JsonValue>& actionInfo,
    const std::unique_ptr<JsonValue>& resultJson)
{
    auto params = OHOS::Ace::JsonUtil::Create(true);
    params->Put("index", std::to_string(actionInfo->GetInt("index")).c_str());
    resultJson->Replace("cmd", "changeIndex");
    resultJson->Put("params", params);
}

std::string ConvertCommandToJson(const std::string& inputJson)
{
    auto json = OHOS::Ace::JsonUtil::ParseJsonString(inputJson);
    if (!json || !json->IsValid() || !json->IsObject()) {
        LOGW("[%{public}s] Input JSON is not valid", TAG.c_str());
        return "";
    }
    auto cmdObj = json->GetValue("cmd");
    if (!cmdObj || !cmdObj->IsObject()) {
        LOGW("[%{public}s] Missing cmd object", TAG.c_str());
        return "";
    }
    auto cmdType = cmdObj->GetString("type");
    if (cmdType.empty()) {
        LOGW("[%{public}s] Missing cmd.type", TAG.c_str());
        return "";
    }
    auto actionInfo = cmdObj->GetValue("action_info");
    if (!actionInfo || !actionInfo->IsObject()) {
        LOGW("[%{public}s] Missing action_info object", TAG.c_str());
        return "";
    }
    auto resultJson = OHOS::Ace::JsonUtil::Create(true);
    resultJson->Put("cmd", cmdType.c_str());
    if (cmdType == "setText" || cmdType == "addText" || cmdType == "deleteText") {
        BuildTextCommandJson(cmdType, actionInfo, resultJson);
    } else if (cmdType == "selectText") {
        BuildSelectTextCommandJson(actionInfo, resultJson);
    } else if (cmdType == "swiperSwitch") {
        BuildSwiperSwitchCommandJson(actionInfo, resultJson);
    } else if (cmdType == "tabsSwitch") {
        BuildTabsSwitchCommandJson(actionInfo, resultJson);
    } else if (cmdType != "copy") {
        LOGW("[%{public}s] Unknown command type: %{public}s", TAG.c_str(), cmdType.c_str());
        return "";
    }
    return resultJson->ToString();
}

bool IsInt32Field(const JsonValue& object, const char* name, bool required = true)
{
    if (!object.Contains(name)) {
        return !required;
    }
    auto value = object.GetValue(name);
    if (!value || !value->IsNumber()) {
        return false;
    }
    const double number = value->GetDouble();
    return std::isfinite(number) && std::trunc(number) == number &&
        number >= std::numeric_limits<int32_t>::min() && number <= std::numeric_limits<int32_t>::max();
}

bool ValidateCommandFields(const std::string& inputJson, const std::string& cmdType)
{
    auto json = OHOS::Ace::JsonUtil::ParseJsonString(inputJson);
    auto cmdObj = json->GetValue("cmd");
    auto actionInfo = cmdObj->GetValue("action_info");
    if (cmdType == "setText" || cmdType == "addText") {
        auto value = actionInfo->GetValue("value");
        return value && value->IsString() &&
            (cmdType != "addText" || IsInt32Field(*actionInfo, "offset", false));
    }
    if (cmdType == "deleteText") {
        return IsInt32Field(*actionInfo, "start", false) && IsInt32Field(*actionInfo, "end", false);
    }
    if (cmdType == "selectText") {
        return IsInt32Field(*actionInfo, "selectionStart") && IsInt32Field(*actionInfo, "selectionEnd");
    }
    if (cmdType == "swiperSwitch") {
        auto direction = actionInfo->GetValue("direction");
        if (!direction || !direction->IsString()) {
            return false;
        }
        const auto type = direction->GetString();
        if (type == "index") {
            return IsInt32Field(*actionInfo, "index");
        }
        return type == "forward" || type == "backward";
    }
    if (cmdType == "tabsSwitch") {
        return IsInt32Field(*actionInfo, "index");
    }
    return true;
}

ArkUI_ErrorCode MapCommandResultToErrorCode(int32_t result)
{
    if (result == RET_FAILED) {
        return ARKUI_ERROR_CODE_ATTRIBUTE_OR_EVENT_NOT_SUPPORTED;
    }
    if (result != RET_SUCCESS) {
        return ARKUI_ERROR_CODE_INTERNAL_ERROR;
    }
    return ARKUI_ERROR_CODE_NO_ERROR;
}

const char* MapResultToMessage(int32_t result)
{
    if (result == RET_FAILED) {
        return "command not supported by the node";
    }
    return "OnRecvCommand returned an internal error";
}

void PostCompletion(const CommandContext& ctx, ArkUI_ErrorCode code)
{
    if (!PrepareCompletion(ctx.state)) {
        return;
    }
    auto resultCode = MapErrorCodeToResultCode(code);
    // The modifier's callback type is for transport only. Invoke through the original public type.
    auto callback = reinterpret_cast<OH_ArkUI_NativeModule_UIEventInjectionCallback>(ctx.callback);
    auto complete = [state = ctx.state, instanceId = ctx.instanceId, callback, userData = ctx.userData, resultCode]() {
        if (!CompleteOnce(state)) {
            return;
        }
        ContainerScope scope(instanceId);
        callback(resultCode, userData);
    };
    if (!ctx.taskExecutor->PostTask(complete, TaskExecutor::TaskType::UI,
        "UIEventInjection.CompleteCompositeCommand")) {
        // A rejected completion task runs here exactly once, with the original command result.
        complete();
    }
}

void NotifyFailure(const CommandContext& ctx, ArkUI_ErrorCode code, const char* message)
{
    SET_ERROR_CODE_AND_MESSAGE_IN_BACKEND(code, message);
    PostCompletion(ctx, code);
}

void ExecuteCommandOnUIThread(const CommandContext& ctx)
{
    if (!BeginCommand(ctx.state)) {
        return;
    }
    auto pipeline = ctx.pipeline.Upgrade();
    if (!pipeline || pipeline->IsDestroyed() ||
        pipeline != PipelineContext::GetContextByContainerId(ctx.instanceId)) {
        NotifyFailure(ctx, ARKUI_ERROR_CODE_UI_CONTEXT_INVALID,
            "pipeline context became invalid after dispatch");
        return;
    }
    auto frameNode = ResolveFrameNodeRefPtr(ctx.uniqueId);
    if (!frameNode) {
        NotifyFailure(ctx, ARKUI_ERROR_CODE_NODE_NOT_FOUND,
            "node was destroyed after dispatch; no longer resolvable at execution");
        return;
    }
    if (frameNode->GetInstanceId() != ctx.instanceId) {
        NotifyFailure(ctx, ARKUI_ERROR_CODE_NODE_NOT_FOUND,
            "node instance id changed after dispatch");
        return;
    }
    const auto& nodeTag = frameNode->GetTag();
    if (SUPPORTED_NODE_TAGS.find(nodeTag) == SUPPORTED_NODE_TAGS.end()) {
        NotifyFailure(ctx, ARKUI_ERROR_CODE_ATTRIBUTE_OR_EVENT_NOT_SUPPORTED,
            "node type not supported for command injection");
        return;
    }
    int32_t result = 0;
    {
        OHOS::Ace::ContainerScope scope(frameNode->GetInstanceId());
        result = frameNode->OnRecvCommand(ctx.convertedJson);
    }
    auto errorCode = MapCommandResultToErrorCode(result);
    if (errorCode != ARKUI_ERROR_CODE_NO_ERROR) {
        SET_ERROR_CODE_AND_MESSAGE_IN_BACKEND(errorCode, MapResultToMessage(result));
    }
    PostCompletion(ctx, errorCode);
}

ArkUI_Int32 PrepareRuntimeContext(ArkUI_Int32 instanceId, RefPtr<PipelineContext>& pipeline,
    RefPtr<OHOS::Ace::TaskExecutor>& taskExecutor)
{
    pipeline = PipelineContext::GetContextByContainerId(instanceId);
    if (!pipeline || pipeline->IsDestroyed()) {
        SET_ERROR_CODE_AND_MESSAGE_IN_BACKEND(ARKUI_ERROR_CODE_UI_CONTEXT_INVALID,
            "pipeline context not found for instance");
        return static_cast<ArkUI_Int32>(ARKUI_ERROR_CODE_UI_CONTEXT_INVALID);
    }
    taskExecutor = pipeline->GetTaskExecutor();
    if (!taskExecutor) {
        SET_ERROR_CODE_AND_MESSAGE_IN_BACKEND(ARKUI_ERROR_CODE_PARAM_INVALID,
            "task executor not available");
        return static_cast<ArkUI_Int32>(ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    if (!taskExecutor->WillRunOnCurrentThread(OHOS::Ace::TaskExecutor::TaskType::UI)) {
        LOGF_ABORT("[%{public}s] InjectCompositeCommand must be called on the UI thread of instance %{public}d. "
            "Calling from a non-UI thread is a fatal error. Aborting.", TAG.c_str(), instanceId);
    }
    return static_cast<ArkUI_Int32>(ARKUI_ERROR_CODE_NO_ERROR);
}

ArkUI_Int32 ParseCommandPayload(const ArkUI_CharPtr json, ArkUI_Uint32 jsonSize,
    std::string& outInput, std::string& outCmdType)
{
    if (json == nullptr) {
        SET_ERROR_CODE_AND_MESSAGE_IN_BACKEND(ARKUI_ERROR_CODE_PARAM_INVALID, "command json is null");
        return static_cast<ArkUI_Int32>(ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    outInput = std::string(json, jsonSize);
    auto parsedJson = OHOS::Ace::JsonUtil::ParseJsonString(outInput);
    if (!parsedJson || !parsedJson->IsValid() || !parsedJson->IsObject()) {
        SET_ERROR_CODE_AND_MESSAGE_IN_BACKEND(ARKUI_ERROR_CODE_PARAM_INVALID, "invalid json payload");
        return static_cast<ArkUI_Int32>(ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    auto cmdObj = parsedJson->GetValue("cmd");
    if (!cmdObj || !cmdObj->IsObject()) {
        SET_ERROR_CODE_AND_MESSAGE_IN_BACKEND(ARKUI_ERROR_CODE_PARAM_INVALID, "missing cmd object");
        return static_cast<ArkUI_Int32>(ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    outCmdType = cmdObj->GetString("type");
    if (outCmdType.empty()) {
        SET_ERROR_CODE_AND_MESSAGE_IN_BACKEND(ARKUI_ERROR_CODE_PARAM_INVALID, "missing cmd.type");
        return static_cast<ArkUI_Int32>(ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    static const std::unordered_set<std::string> validTypes = {
        "setText", "addText", "deleteText", "selectText", "copy", "swiperSwitch", "tabsSwitch"
    };
    if (validTypes.find(outCmdType) == validTypes.end()) {
        SET_ERROR_CODE_AND_MESSAGE_IN_BACKEND(ARKUI_ERROR_CODE_PARAM_INVALID, "unknown command type");
        return static_cast<ArkUI_Int32>(ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    auto actionInfo = cmdObj->GetValue("action_info");
    if (!actionInfo || !actionInfo->IsObject()) {
        SET_ERROR_CODE_AND_MESSAGE_IN_BACKEND(ARKUI_ERROR_CODE_PARAM_INVALID, "missing action_info object");
        return static_cast<ArkUI_Int32>(ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    return static_cast<ArkUI_Int32>(ARKUI_ERROR_CODE_NO_ERROR);
}

ArkUI_Int32 BuildCommandPayload(const std::string& input, const std::string& cmdType, std::string& outConverted)
{
    if (!ValidateCommandFields(input, cmdType)) {
        SET_ERROR_CODE_AND_MESSAGE_IN_BACKEND(ARKUI_ERROR_CODE_PARAM_INVALID,
            "missing or invalid field for command type");
        return static_cast<ArkUI_Int32>(ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    outConverted = ConvertCommandToJson(input);
    if (outConverted.empty()) {
        SET_ERROR_CODE_AND_MESSAGE_IN_BACKEND(ARKUI_ERROR_CODE_PARAM_INVALID, "failed to convert command json");
        return static_cast<ArkUI_Int32>(ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    return static_cast<ArkUI_Int32>(ARKUI_ERROR_CODE_NO_ERROR);
}

ArkUI_Int32 EnqueueCommand(CommandContext& ctx, const RefPtr<OHOS::Ace::TaskExecutor>& taskExecutor)
{
    auto probe = ResolveFrameNodeRefPtr(ctx.uniqueId);
    if (!probe) {
        SET_ERROR_CODE_AND_MESSAGE_IN_BACKEND(ARKUI_ERROR_CODE_NODE_NOT_FOUND,
            "uniqueId does not resolve to a node");
        return static_cast<ArkUI_Int32>(ARKUI_ERROR_CODE_NODE_NOT_FOUND);
    }
    if (probe->GetInstanceId() != ctx.instanceId) {
        SET_ERROR_CODE_AND_MESSAGE_IN_BACKEND(ARKUI_ERROR_CODE_NODE_NOT_FOUND,
            "node instance id does not match the specified uiContext instance");
        return static_cast<ArkUI_Int32>(ARKUI_ERROR_CODE_NODE_NOT_FOUND);
    }
    auto owner = Container::GetContainer(ctx.instanceId);
    if (!owner) {
        SET_ERROR_CODE_AND_MESSAGE_IN_BACKEND(ARKUI_ERROR_CODE_UI_CONTEXT_INVALID,
            "UI instance is no longer registered");
        return static_cast<ArkUI_Int32>(ARKUI_ERROR_CODE_UI_CONTEXT_INVALID);
    }
    ctx.state = TryAcquireCommand(ctx.instanceId, owner);
    if (!ctx.state) {
        SET_ERROR_CODE_AND_MESSAGE_IN_BACKEND(ARKUI_ERROR_CODE_COMMAND_UNFINISHED,
            "a previous composite command is still in-flight");
        return static_cast<ArkUI_Int32>(ARKUI_ERROR_CODE_COMMAND_UNFINISHED);
    }
    bool posted = taskExecutor->PostTask(
        [ctx]() { ExecuteCommandOnUIThread(ctx); },
        OHOS::Ace::TaskExecutor::TaskType::UI,
        "UIEventInjection.InjectCompositeCommand");
    if (!posted) {
        CancelCommand(ctx.state);
        SET_ERROR_CODE_AND_MESSAGE_IN_BACKEND(ARKUI_ERROR_CODE_PARAM_INVALID,
            "failed to post composite command to UI thread");
        return static_cast<ArkUI_Int32>(ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    return static_cast<ArkUI_Int32>(ARKUI_ERROR_CODE_NO_ERROR);
}

} // namespace

ArkUI_Int32 InjectCompositeCommandImpl(const ArkUIInjectCommandParams* params)
{
    if (params == nullptr) {
        SET_ERROR_CODE_AND_MESSAGE_IN_BACKEND(ARKUI_ERROR_CODE_PARAM_INVALID, "params must not be null");
        return static_cast<ArkUI_Int32>(ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    if (params->callback == nullptr) {
        SET_ERROR_CODE_AND_MESSAGE_IN_BACKEND(ARKUI_ERROR_CODE_PARAM_INVALID, "callback must not be null");
        return static_cast<ArkUI_Int32>(ARKUI_ERROR_CODE_PARAM_INVALID);
    }
    RefPtr<PipelineContext> pipeline;
    RefPtr<OHOS::Ace::TaskExecutor> taskExecutor;
    auto rc = PrepareRuntimeContext(params->instanceId, pipeline, taskExecutor);
    if (rc != static_cast<ArkUI_Int32>(ARKUI_ERROR_CODE_NO_ERROR)) {
        return rc;
    }
    std::string inputJson;
    std::string cmdType;
    rc = ParseCommandPayload(params->json, params->jsonSize, inputJson, cmdType);
    if (rc != static_cast<ArkUI_Int32>(ARKUI_ERROR_CODE_NO_ERROR)) {
        return rc;
    }
    std::string convertedJson;
    rc = BuildCommandPayload(inputJson, cmdType, convertedJson);
    if (rc != static_cast<ArkUI_Int32>(ARKUI_ERROR_CODE_NO_ERROR)) {
        return rc;
    }
    CommandContext ctx;
    ctx.instanceId = params->instanceId;
    ctx.uniqueId = params->uniqueId;
    ctx.pipeline = pipeline;
    ctx.taskExecutor = taskExecutor;
    ctx.convertedJson = std::move(convertedJson);
    ctx.callback = params->callback;
    ctx.userData = params->userData;
    return EnqueueCommand(ctx, taskExecutor);
}

} // namespace OHOS::Ace::NG

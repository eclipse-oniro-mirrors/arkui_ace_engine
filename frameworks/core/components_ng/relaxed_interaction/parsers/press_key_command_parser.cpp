/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 */
#include "core/components_ng/relaxed_interaction/parsers/press_key_command_parser.h"

#include <algorithm>
#include <cctype>
#include <map>

#include "core/components_ng/relaxed_interaction/executors/press_key_executor.h"

namespace OHOS::Ace::NG {
namespace {
constexpr int32_t MAX_KEY_HOLD_DURATION_MS = 5000;

bool ParseKey(const std::string& value, KeyCode& key)
{
    std::string name;
    for (char character : value) {
        name += static_cast<char>(std::toupper(static_cast<unsigned char>(character)));
    }
    if (name.size() == 1 && name[0] >= 'A' && name[0] <= 'Z') {
        key = static_cast<KeyCode>(static_cast<int32_t>(KeyCode::KEY_A) + name[0] - 'A');
        return true;
    }
    if (name.size() == 1 && name[0] >= '0' && name[0] <= '9') {
        key = static_cast<KeyCode>(static_cast<int32_t>(KeyCode::KEY_0) + name[0] - '0');
        return true;
    }
    static const std::map<std::string, KeyCode> keys = {
        { "ENTER", KeyCode::KEY_ENTER }, { "RETURN", KeyCode::KEY_ENTER },
        { "ESC", KeyCode::KEY_ESCAPE }, { "ESCAPE", KeyCode::KEY_ESCAPE },
        { "SPACE", KeyCode::KEY_SPACE }, { "TAB", KeyCode::KEY_TAB },
        { "BACKSPACE", KeyCode::KEY_DEL }, { "DELETE", KeyCode::KEY_FORWARD_DEL },
        { "CTRL", KeyCode::KEY_CTRL_LEFT }, { "CONTROL", KeyCode::KEY_CTRL_LEFT },
        { "SHIFT", KeyCode::KEY_SHIFT_LEFT }, { "ALT", KeyCode::KEY_ALT_LEFT },
        { "HOME", KeyCode::KEY_HOME }, { "END", KeyCode::KEY_MOVE_END },
        { "UP", KeyCode::KEY_DPAD_UP }, { "DOWN", KeyCode::KEY_DPAD_DOWN },
        { "LEFT", KeyCode::KEY_DPAD_LEFT }, { "RIGHT", KeyCode::KEY_DPAD_RIGHT },
    };
    auto iter = keys.find(name);
    if (iter == keys.end()) {
        return false;
    }
    key = iter->second;
    return true;
}

// Append one key segment, rejecting duplicates so repeated keys fail early.
bool AppendKey(const std::string& text, KeyInput& input)
{
    KeyCode key;
    if (!ParseKey(text, key)) {
        return false;
    }
    if (std::find(input.keys.begin(), input.keys.end(), key) != input.keys.end()) {
        return false;
    }
    input.keys.emplace_back(key);
    return true;
}

// Combo syntax "ctrl+a": split on '+' keeping the given order; every segment
// must be a non-empty, valid key name and duplicates are rejected.
bool AppendCombo(const std::string& text, KeyInput& input)
{
    size_t begin = 0;
    while (begin <= text.size()) {
        size_t end = text.find('+', begin);
        if (end == begin) {
            return false;
        }
        if (!AppendKey(text.substr(begin, end - begin), input)) {
            return false;
        }
        if (end == std::string::npos) {
            break;
        }
        begin = end + 1;
    }
    return true;
}

bool AppendKeyValue(const JsonValue& value, KeyInput& input)
{
    if (!value.IsString()) {
        return false;
    }
    const auto text = value.GetString();
    if (text.find('+') != std::string::npos) {
        return AppendCombo(text, input);
    }
    return AppendKey(text, input);
}
} // namespace

std::vector<std::unique_ptr<BaseExecutor>> PressKeyCommandParser::Parse(const std::unique_ptr<JsonValue>& json)
{
    std::vector<std::unique_ptr<BaseExecutor>> executors;
    if (!json || !json->IsObject()) {
        return executors;
    }
    // action_info is the single contract form; it must be a present, well-formed object.
    // actionInfo outlives the branch so the source pointer stays valid for the whole parse.
    auto actionInfo = json->GetValue("action_info");
    if (!actionInfo || !actionInfo->IsObject()) {
        return executors;
    }
    const JsonValue* source = actionInfo.get();
    auto keyValue = source->Contains("key") ? source->GetValue("key") : source->GetValue("keys");
    if (!keyValue) {
        return executors;
    }
    KeyInput input;
    if (keyValue->IsArray()) {
        for (int32_t index = 0; index < keyValue->GetArraySize(); ++index) {
            auto item = keyValue->GetArrayItem(index);
            if (!item || !AppendKeyValue(*item, input)) {
                return executors;
            }
        }
    } else if (!AppendKeyValue(*keyValue, input)) {
        return executors;
    }
    if (input.keys.empty()) {
        return executors;
    }
    if (source->Contains("hold_ms")) {
        auto hold = source->GetValue("hold_ms");
        if (!hold || !hold->IsNumber() || hold->GetDouble() != hold->GetInt()) {
            return executors;
        }
        input.holdMs = hold->GetInt();
        if (input.holdMs < 0 || input.holdMs > MAX_KEY_HOLD_DURATION_MS) {
            return executors;
        }
    }
    executors.emplace_back(std::make_unique<PressKeyExecutor>(context_, input));
    return executors;
}
} // namespace OHOS::Ace::NG

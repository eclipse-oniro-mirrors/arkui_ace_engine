/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 */
#include "core/components_ng/relaxed_interaction/parsers/wheel_scroll_command_parser.h"

#include "core/components_ng/relaxed_interaction/executors/wheel_scroll_executor.h"

namespace OHOS::Ace::NG {
namespace {
bool ParseModifiers(const JsonValue* json, std::vector<std::string>& modifiers)
{
    if (!json || (!json->Contains("modifier") && !json->Contains("modifiers"))) {
        return true;
    }
    auto value = json->Contains("modifier") ? json->GetValue("modifier") : json->GetValue("modifiers");
    if (!value) {
        return true;
    }
    if (value->IsString()) {
        modifiers.emplace_back(value->GetString());
    } else if (value->IsArray()) {
        for (int32_t index = 0; index < value->GetArraySize(); ++index) {
            auto item = value->GetArrayItem(index);
            if (!item || !item->IsString()) {
                return false;
            }
            modifiers.emplace_back(item->GetString());
        }
    } else {
        return false;
    }
    for (const auto& modifier : modifiers) {
        if (modifier != "ctrl" && modifier != "shift" && modifier != "alt") {
            return false;
        }
    }
    return true;
}

bool ParseDirection(const JsonValue* options, WheelInput& input)
{
    const auto* source = options;
    if (!source->Contains("direction")) {
        return true;
    }
    auto direction = source->GetValue("direction");
    if (!direction->IsString()) {
        return false;
    }
    const auto name = direction->GetString();
    if (name == "horizontal") {
        input.direction = WheelScrollDirection::HORIZONTAL;
        return true;
    }
    return name == "vertical";
}

bool ParseCoordinates(const JsonValue* options, WheelInput& input)
{
    if (!options->Contains("coordinates")) {
        // 缺省坐标由执行器按有效窗口矩形中心补齐。
        return true;
    }
    auto coordinates = options->GetValue("coordinates");
    if (!coordinates || !coordinates->IsObject()) {
        return false;
    }
    auto x = coordinates->GetValue("x1");
    auto y = coordinates->GetValue("y1");
    if (!x || !y || !x->IsNumber() || !y->IsNumber()) {
        return false;
    }
    input.coordinates = PointF(x->GetDouble(), y->GetDouble());
    input.hasCoordinates = true;
    return true;
}
} // namespace

std::vector<std::unique_ptr<BaseExecutor>> WheelScrollCommandParser::Parse(const std::unique_ptr<JsonValue>& json)
{
    std::vector<std::unique_ptr<BaseExecutor>> executors;
    if (!json || !json->IsObject()) {
        return executors;
    }
    auto actionInfo = json->GetValue("action_info");
    if (!actionInfo || !actionInfo->IsObject()) {
        return executors;
    }
    const JsonValue* options = actionInfo.get();
    auto delta = options->GetValue("delta");
    if (!delta || !delta->IsNumber() || delta->GetDouble() != delta->GetInt()) {
        return executors;
    }
    WheelInput input;
    input.delta = delta->GetInt();
    if (!ParseDirection(options, input) || !ParseModifiers(options, input.modifiers)) {
        return executors;
    }
    if (!ParseCoordinates(options, input)) {
        return executors;
    }
    executors.emplace_back(std::make_unique<WheelScrollExecutor>(context_, input));
    return executors;
}
} // namespace OHOS::Ace::NG

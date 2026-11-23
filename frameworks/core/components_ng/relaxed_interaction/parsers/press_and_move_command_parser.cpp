/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 */
#include "core/components_ng/relaxed_interaction/parsers/press_and_move_command_parser.h"

#include "core/components_ng/relaxed_interaction/executors/press_and_move_executor.h"

namespace OHOS::Ace::NG {
namespace {
bool ParsePoint(const JsonValue* json, const char* xKey, const char* yKey, float& x, float& y)
{
    if (!json || !json->IsObject() || !json->Contains(xKey) || !json->Contains(yKey)) {
        return false;
    }
    auto xValue = json->GetValue(xKey);
    auto yValue = json->GetValue(yKey);
    if (!xValue || !yValue || !xValue->IsNumber() || !yValue->IsNumber()) {
        return false;
    }
    x = static_cast<float>(xValue->GetDouble());
    y = static_cast<float>(yValue->GetDouble());
    return true;
}
} // namespace

std::vector<std::unique_ptr<BaseExecutor>> PressAndMoveCommandParser::Parse(
    const std::unique_ptr<JsonValue>& json)
{
    std::vector<std::unique_ptr<BaseExecutor>> executors;
    if (!json || !json->IsObject()) {
        return executors;
    }
    // action_info is the single contract form; it must be a present, well-formed object.
    auto actionInfo = json->GetValue("action_info");
    if (!actionInfo || !actionInfo->IsObject()) {
        return executors;
    }
    const JsonValue* source = actionInfo.get();
    MoveInput input;
    auto from = source->GetValue("from");
    auto to = source->GetValue("to");
    if (!from || !to ||
        !ParsePoint(from.get(), "x", "y", input.coordinates.x1, input.coordinates.y1) ||
        !ParsePoint(to.get(), "x", "y", input.coordinates.x2, input.coordinates.y2)) {
        return executors;
    }
    if (source->Contains("input_type")) {
        auto inputType = source->GetValue("input_type");
        if (!inputType->IsString()) {
            return executors;
        }
        if (inputType->GetString() == "mouse") {
            input.inputType = MoveInputType::MOUSE;
        } else if (inputType->GetString() != "touch") {
            return executors;
        }
    }
    executors.emplace_back(std::make_unique<PressAndMoveExecutor>(context_, input));
    return executors;
}
} // namespace OHOS::Ace::NG

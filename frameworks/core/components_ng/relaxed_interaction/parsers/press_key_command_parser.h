/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 */
#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_RELAXED_INTERACTION_PRESS_KEY_COMMAND_PARSER_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_RELAXED_INTERACTION_PRESS_KEY_COMMAND_PARSER_H

#include "core/components_ng/relaxed_interaction/base_command_parser.h"
#include "core/components_ng/relaxed_interaction/relaxed_interaction_types.h"

namespace OHOS::Ace::NG {
class PressKeyCommandParser final : public BaseCommandParser {
public:
    explicit PressKeyCommandParser(WeakPtr<PipelineContext> context) : BaseCommandParser(context) {}
    ~PressKeyCommandParser() override = default;
    std::vector<std::unique_ptr<BaseExecutor>> Parse(const std::unique_ptr<JsonValue>& json) override;
};
} // namespace OHOS::Ace::NG
#endif

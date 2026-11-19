/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 */
#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_RELAXED_INTERACTION_MOUSE_CLICK_EXECUTOR_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_RELAXED_INTERACTION_MOUSE_CLICK_EXECUTOR_H

#include <vector>

#include "core/components_ng/relaxed_interaction/base_executor.h"
#include "core/event/mouse_event.h"

namespace OHOS::Ace::NG {
class MouseClickExecutor final : public BaseExecutor {
public:
    MouseClickExecutor(WeakPtr<PipelineContext> context, const ClickInput& input);
    ~MouseClickExecutor() override;

    void Cancel() override;
    ExecutorResult ExecuteStep() override;
    std::string GetType() const override { return "mouse_click"; }
    bool IsSingleStep() const override { return false; }

private:
    std::vector<MouseEvent> events_;
    size_t nextEvent_ = 0;
    bool pressed_ = false;
    bool completed_ = false;
    bool denyChecked_ = false;
};
} // namespace OHOS::Ace::NG
#endif

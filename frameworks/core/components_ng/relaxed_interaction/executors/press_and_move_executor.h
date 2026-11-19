/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 */
#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_RELAXED_INTERACTION_PRESS_AND_MOVE_EXECUTOR_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_RELAXED_INTERACTION_PRESS_AND_MOVE_EXECUTOR_H

#include <vector>

#include "core/components_ng/relaxed_interaction/base_executor.h"
#include "core/event/mouse_event.h"
#include "ui/event/touch_event.h"

namespace OHOS::Ace::NG {
class PressAndMoveExecutor final : public BaseExecutor {
public:
    PressAndMoveExecutor(WeakPtr<PipelineContext> context, const MoveInput& input);
    ~PressAndMoveExecutor() override;
    void Cancel() override;
    ExecutorResult ExecuteStep() override;
    std::string GetType() const override { return "press_and_move"; }
    bool IsSingleStep() const override { return false; }

private:
    bool CheckDenyOnce();
    void RestoreDragSuppression();

    MoveInputType inputType_;
    std::vector<TouchEvent> touchEvents_;
    std::vector<MouseEvent> mouseEvents_;
    size_t nextEvent_ = 0;
    bool pressed_ = false;
    bool completed_ = false;
    bool dragSuppressed_ = false;
    bool denyChecked_ = false;
};
} // namespace OHOS::Ace::NG
#endif

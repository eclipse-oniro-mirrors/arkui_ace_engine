/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 */
#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_RELAXED_INTERACTION_PRESS_KEY_EXECUTOR_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_RELAXED_INTERACTION_PRESS_KEY_EXECUTOR_H

#include <chrono>
#include <vector>

#include "core/components_ng/relaxed_interaction/base_executor.h"
#include "core/event/key_event.h"

namespace OHOS::Ace::NG {
class PressKeyExecutor final : public BaseExecutor {
public:
    PressKeyExecutor(WeakPtr<PipelineContext> context, const KeyInput& input);
    ~PressKeyExecutor() override;

    void Cancel() override;
    ExecutorResult ExecuteStep() override;
    std::string GetType() const override { return "press_key"; }
    bool IsSingleStep() const override { return false; }
    int32_t GetHoldMs() const override { return holdMs_; }

private:
    ExecutorResult FinishSequence(const RefPtr<PipelineContext>& context);
    bool DispatchKeyEvent(const RefPtr<PipelineContext>& context, KeyEvent event);

    std::vector<KeyEvent> events_;
    std::vector<KeyCode> pressedKeys_;
    size_t nextEvent_ = 0;
    int32_t holdMs_ = 0;
    size_t keyCount_ = 0;
    std::chrono::steady_clock::time_point holdStart_;
    bool holdStarted_ = false;
    bool completed_ = false;
    bool denyChecked_ = false;
    // Mirrors key_executor semantics: the sequence succeeds only when the pipeline
    // handled at least one of the dispatched DOWN/UP events.
    bool anyHandled_ = false;
};
} // namespace OHOS::Ace::NG
#endif

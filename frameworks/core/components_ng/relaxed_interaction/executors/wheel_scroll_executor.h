/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 */
#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_RELAXED_INTERACTION_WHEEL_SCROLL_EXECUTOR_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_RELAXED_INTERACTION_WHEEL_SCROLL_EXECUTOR_H

#include "core/components_ng/relaxed_interaction/base_executor.h"
#include "core/event/key_code.h"

namespace OHOS::Ace::NG {
class WheelScrollExecutor final : public BaseExecutor {
public:
    WheelScrollExecutor(WeakPtr<PipelineContext> context, const WheelInput& input);
    ~WheelScrollExecutor() override
    {
        Cancel();
    }
    ExecutorResult ExecuteStep() override;
    void Cancel() override;
    std::string GetType() const override { return "wheel_scroll"; }
    bool IsSingleStep() const override { return false; }

private:
    PointF coordinates_;
    int32_t delta_ = 0;
    WheelScrollDirection direction_ = WheelScrollDirection::VERTICAL;
    std::vector<KeyCode> pressedCodes_;
    bool hasCoordinates_ = false;
    bool began_ = false;
    bool ended_ = false;
    bool denyChecked_ = false;
};
} // namespace OHOS::Ace::NG
#endif

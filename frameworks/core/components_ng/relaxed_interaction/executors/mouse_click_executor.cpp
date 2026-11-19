/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 */
#include "core/components_ng/relaxed_interaction/executors/mouse_click_executor.h"

#include "core/components_ng/relaxed_interaction/utils/relaxed_event_factory.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
namespace {
MouseButton ToMouseButton(const std::string& button)
{
    if (button == "right") {
        return MouseButton::RIGHT_BUTTON;
    }
    if (button == "middle") {
        return MouseButton::MIDDLE_BUTTON;
    }
    if (button == "back") {
        return MouseButton::BACK_BUTTON;
    }
    if (button == "forward") {
        return MouseButton::FORWARD_BUTTON;
    }
    return MouseButton::LEFT_BUTTON;
}

std::vector<KeyCode> ToModifierKeys(const std::vector<std::string>& modifiers)
{
    std::vector<KeyCode> keys;
    for (const auto& modifier : modifiers) {
        if (modifier == "ctrl") {
            keys.emplace_back(KeyCode::KEY_CTRL_LEFT);
        } else if (modifier == "shift") {
            keys.emplace_back(KeyCode::KEY_SHIFT_LEFT);
        } else if (modifier == "alt") {
            keys.emplace_back(KeyCode::KEY_ALT_LEFT);
        }
    }
    return keys;
}
} // namespace

MouseClickExecutor::MouseClickExecutor(WeakPtr<PipelineContext> context, const ClickInput& input)
    : BaseExecutor(context)
{
    auto button = ToMouseButton(input.button);
    auto pressedKeys = ToModifierKeys(input.modifiers);
    for (int32_t index = 0; index < input.clickCount; ++index) {
        events_.emplace_back(RelaxedEventFactory::CreateMouseEvent(input.coordinates, MouseAction::PRESS, button,
            static_cast<int32_t>(button), pressedKeys));
        events_.emplace_back(RelaxedEventFactory::CreateMouseEvent(input.coordinates, MouseAction::RELEASE, button,
            0, pressedKeys));
    }
}

MouseClickExecutor::~MouseClickExecutor()
{
    Cancel();
}

void MouseClickExecutor::Cancel()
{
    if (completed_) {
        return;
    }
    const auto lastEvent = nextEvent_;
    const bool needsCancel = pressed_ && lastEvent > 0;
    completed_ = true;
    pressed_ = false;
    nextEvent_ = events_.size();
    auto context = context_.Upgrade();
    if (!context || context->IsDestroyed() || !needsCancel) {
        return;
    }
    auto cancel = events_[lastEvent - 1];
    cancel.action = MouseAction::CANCEL;
    cancel.pressedButtons = 0;
    cancel.pressedButtonsArray.clear();
    context->OnMouseEvent(cancel);
}

ExecutorResult MouseClickExecutor::ExecuteStep()
{
    if (completed_ || nextEvent_ >= events_.size()) {
        return ExecutorResult::SUCCESS;
    }
    auto context = context_.Upgrade();
    if (!context || context->IsDestroyed()) {
        Cancel();
        return SetFailureState(ExecutionState::CONTEXT_UNAVAILABLE);
    }
    if (!denyChecked_) {
        denyChecked_ = true;
        if (IsDenied(PointF(events_[nextEvent_].x, events_[nextEvent_].y))) {
            Cancel();
            return SetFailureState(ExecutionState::CONTROL_DENIED);
        }
    }
    if (completed_) {
        return ExecutorResult::SUCCESS;
    }
    if (context->IsDestroyed()) {
        Cancel();
        return SetFailureState(ExecutionState::CONTEXT_UNAVAILABLE);
    }
    auto event = events_[nextEvent_++];
    pressed_ = event.action == MouseAction::PRESS;
    completed_ = nextEvent_ == events_.size();
    context->OnMouseEvent(event);
    return completed_ ? ExecutorResult::SUCCESS : ExecutorResult::PENDING;
}
} // namespace OHOS::Ace::NG

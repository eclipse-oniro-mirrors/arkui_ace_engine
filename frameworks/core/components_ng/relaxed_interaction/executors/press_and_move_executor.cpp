/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 */
#include "core/components_ng/relaxed_interaction/executors/press_and_move_executor.h"

#include <algorithm>
#include <chrono>

#include "core/components_ng/relaxed_interaction/utils/relaxed_event_factory.h"
#include "core/common/event_manager.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
namespace {
constexpr int32_t MOVE_STEPS = 8;
PointF Interpolate(const Coordinates& coordinates, int32_t step)
{
    const auto ratio = static_cast<float>(step) / MOVE_STEPS;
    return PointF(coordinates.x1 + (coordinates.x2 - coordinates.x1) * ratio,
        coordinates.y1 + (coordinates.y2 - coordinates.y1) * ratio);
}
} // namespace

PressAndMoveExecutor::PressAndMoveExecutor(WeakPtr<PipelineContext> context, const MoveInput& input)
    : BaseExecutor(context), inputType_(input.inputType)
{
    auto coordinates = input.coordinates;
    auto contextRef = context_.Upgrade();
    if (contextRef) {
        auto root = contextRef->GetRootRect();
        if (root.Width() > 0.0f && root.Height() > 0.0f) {
            coordinates.x1 = std::clamp(coordinates.x1, root.Left(), root.Right());
            coordinates.x2 = std::clamp(coordinates.x2, root.Left(), root.Right());
            coordinates.y1 = std::clamp(coordinates.y1, root.Top(), root.Bottom());
            coordinates.y2 = std::clamp(coordinates.y2, root.Top(), root.Bottom());
        }
    }
    if (inputType_ == MoveInputType::TOUCH) {
        touchEvents_.emplace_back(RelaxedEventFactory::CreateTouchEvent(
            PointF(coordinates.x1, coordinates.y1), TouchType::DOWN));
        for (int32_t step = 1; step <= MOVE_STEPS; ++step) {
            touchEvents_.emplace_back(RelaxedEventFactory::CreateTouchEvent(Interpolate(coordinates, step),
                TouchType::MOVE));
        }
        touchEvents_.emplace_back(RelaxedEventFactory::CreateTouchEvent(
            PointF(coordinates.x2, coordinates.y2), TouchType::UP));
    } else {
        const auto button = MouseButton::LEFT_BUTTON;
        // The whole press/move/release sequence shares one press moment (real mouse sequences
        // keep pressedTime at the press start); action time stays per event.
        const auto sequenceTime = std::chrono::high_resolution_clock::now();
        mouseEvents_.emplace_back(RelaxedEventFactory::CreateMouseEvent(
            PointF(coordinates.x1, coordinates.y1), MouseAction::PRESS, button,
            static_cast<int32_t>(button), {}, sequenceTime));
        for (int32_t step = 1; step <= MOVE_STEPS; ++step) {
            mouseEvents_.emplace_back(RelaxedEventFactory::CreateMouseEvent(Interpolate(coordinates, step),
                MouseAction::MOVE, button, static_cast<int32_t>(button), {}, sequenceTime));
        }
        mouseEvents_.emplace_back(RelaxedEventFactory::CreateMouseEvent(
            PointF(coordinates.x2, coordinates.y2), MouseAction::RELEASE, button, 0, {}, sequenceTime));
    }
}

PressAndMoveExecutor::~PressAndMoveExecutor()
{
    Cancel();
}

void PressAndMoveExecutor::RestoreDragSuppression()
{
    if (!dragSuppressed_) {
        return;
    }
    dragSuppressed_ = false;
    auto context = context_.Upgrade();
    if (context) {
        auto eventManager = context->GetEventManager();
        if (eventManager) {
            eventManager->SetRelaxedInteractionDragSuppressed(false);
        }
    }
}

void PressAndMoveExecutor::Cancel()
{
    if (completed_) {
        return;
    }
    const auto lastEvent = nextEvent_;
    const bool needsCancel = pressed_ && lastEvent > 0;
    completed_ = true;
    pressed_ = false;
    nextEvent_ = inputType_ == MoveInputType::TOUCH ? touchEvents_.size() : mouseEvents_.size();
    RestoreDragSuppression();
    auto context = context_.Upgrade();
    if (!context || context->IsDestroyed() || !needsCancel) {
        return;
    }
    if (inputType_ == MoveInputType::TOUCH) {
        auto cancel = touchEvents_[lastEvent - 1];
        cancel.type = TouchType::CANCEL;
        for (auto& pointer : cancel.pointers) {
            pointer.isPressed = false;
        }
        context->OnTouchEvent(cancel);
    } else {
        auto cancel = mouseEvents_[lastEvent - 1];
        cancel.action = MouseAction::CANCEL;
        cancel.pressedButtons = 0;
        cancel.pressedButtonsArray.clear();
        context->OnMouseEvent(cancel);
    }
}

// One-shot deny and special-component checks at the first injected point. Returns false
// when the move is denied or targets a special component; the failure state has already
// been recorded on the executor.
bool PressAndMoveExecutor::CheckDenyOnce()
{
    denyChecked_ = true;
    const auto point = inputType_ == MoveInputType::TOUCH
        ? PointF(touchEvents_.front().x, touchEvents_.front().y)
        : PointF(mouseEvents_.front().x, mouseEvents_.front().y);
    if (IsDenied(point)) {
        SetFailureState(ExecutionState::CONTROL_DENIED);
        return false;
    }
    if (DetectSpecialComponentHit(point.GetX(), point.GetY())) {
        return false;
    }
    return true;
}

ExecutorResult PressAndMoveExecutor::ExecuteStep()
{
    const auto eventCount = inputType_ == MoveInputType::TOUCH ? touchEvents_.size() : mouseEvents_.size();
    if (completed_ || nextEvent_ >= eventCount) {
        return ExecutorResult::SUCCESS;
    }
    auto context = context_.Upgrade();
    if (!context || context->IsDestroyed()) {
        Cancel();
        return SetFailureState(ExecutionState::CONTEXT_UNAVAILABLE);
    }
    if (!denyChecked_ && !CheckDenyOnce()) {
        Cancel();
        return ExecutorResult::FAILED;
    }
    if (completed_) {
        return ExecutorResult::SUCCESS;
    }
    if (context->IsDestroyed()) {
        Cancel();
        return SetFailureState(ExecutionState::CONTEXT_UNAVAILABLE);
    }
    if (!dragSuppressed_) {
        auto eventManager = context->GetEventManager();
        if (eventManager) {
            eventManager->SetRelaxedInteractionDragSuppressed(true);
            dragSuppressed_ = true;
        }
    }
    const auto eventIndex = nextEvent_++;
    completed_ = nextEvent_ == eventCount;
    pressed_ = !completed_;
    if (inputType_ == MoveInputType::TOUCH) {
        context->OnTouchEvent(touchEvents_[eventIndex]);
    } else {
        context->OnMouseEvent(mouseEvents_[eventIndex]);
    }
    if (completed_) {
        RestoreDragSuppression();
    }
    return completed_ ? ExecutorResult::SUCCESS : ExecutorResult::PENDING;
}
} // namespace OHOS::Ace::NG

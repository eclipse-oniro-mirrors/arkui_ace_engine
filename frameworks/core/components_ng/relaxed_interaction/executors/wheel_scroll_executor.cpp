/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 */
#include "core/components_ng/relaxed_interaction/executors/wheel_scroll_executor.h"

#include <algorithm>

#include "core/components_ng/relaxed_interaction/utils/relaxed_event_factory.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
namespace {
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
WheelScrollExecutor::WheelScrollExecutor(WeakPtr<PipelineContext> context, const WheelInput& input)
    : BaseExecutor(context), coordinates_(input.coordinates), delta_(input.delta), direction_(input.direction),
      pressedCodes_(ToModifierKeys(input.modifiers)), hasCoordinates_(input.hasCoordinates)
{}

ExecutorResult WheelScrollExecutor::ExecuteStep()
{
    auto context = context_.Upgrade();
    if (!context || context->IsDestroyed()) {
        return SetFailureState(ExecutionState::CONTEXT_UNAVAILABLE);
    }
    if (!began_) {
        const auto rootRect = context->GetRootRect();
        if (!hasCoordinates_) {
            if (!rootRect.IsValid() || rootRect.IsEmpty()) {
                return SetFailureState(ExecutionState::TARGET_NOT_FOUND);
            }
            coordinates_ = PointF(rootRect.Left() + rootRect.Width() / 2.0f,
                rootRect.Top() + rootRect.Height() / 2.0f);
        } else if (rootRect.IsValid() && !rootRect.IsEmpty()) {
            // Uniform with press_and_move/mouse_click: clip explicit coordinates into the
            // window rect so off-window scroll lands on the nearest in-window point.
            coordinates_.SetX(std::clamp(coordinates_.GetX(), rootRect.Left(), rootRect.Right()));
            coordinates_.SetY(std::clamp(coordinates_.GetY(), rootRect.Top(), rootRect.Bottom()));
        }
        if (!denyChecked_) {
            denyChecked_ = true;
            if (IsDenied(coordinates_)) {
                return SetFailureState(ExecutionState::CONTROL_DENIED);
            }
        }
        if (DetectSpecialComponentHit(coordinates_.GetX(), coordinates_.GetY())) {
            return ExecutorResult::FAILED;
        }
        const auto windowOffset = context->GetDisplayWindowRectInfo().GetOffset();
        // The first frame opens the axis session and carries the scroll delta.
        context->OnAxisEvent(RelaxedEventFactory::CreateAxisEvent(
            coordinates_, windowOffset, 0.0, pressedCodes_, direction_, AxisAction::BEGIN));
        context->OnAxisEvent(RelaxedEventFactory::CreateAxisEvent(
            coordinates_, windowOffset, delta_, pressedCodes_, direction_, AxisAction::UPDATE));
        began_ = true;
        // END must land on a later frame: the axis pipeline rejects an END sent in the same frame.
        return ExecutorResult::PENDING;
    }
    const auto windowOffset = context->GetDisplayWindowRectInfo().GetOffset();
    context->OnAxisEvent(RelaxedEventFactory::CreateAxisEvent(
        coordinates_, windowOffset, 0.0, pressedCodes_, direction_, AxisAction::END));
    ended_ = true;
    return ExecutorResult::SUCCESS;
}

void WheelScrollExecutor::Cancel()
{
    if (!began_ || ended_) {
        return;
    }
    ended_ = true;
    auto context = context_.Upgrade();
    if (!context || context->IsDestroyed()) {
        return;
    }
    // Close the axis session so an interrupted scroll leaves no open BEGIN.
    const auto windowOffset = context->GetDisplayWindowRectInfo().GetOffset();
    context->OnAxisEvent(RelaxedEventFactory::CreateAxisEvent(
        coordinates_, windowOffset, 0.0, pressedCodes_, direction_, AxisAction::END));
}
} // namespace OHOS::Ace::NG

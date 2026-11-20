/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "core/components_ng/relaxed_interaction/executors/press_key_executor.h"

#include <algorithm>

#include "core/components_ng/relaxed_interaction/utils/relaxed_event_factory.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
PressKeyExecutor::PressKeyExecutor(WeakPtr<PipelineContext> context, const KeyInput& input)
    : BaseExecutor(context), holdMs_(input.holdMs), keyCount_(input.keys.size())
{
    std::vector<KeyCode> pressedKeys;
    for (const auto& key : input.keys) {
        if (std::find(pressedKeys.begin(), pressedKeys.end(), key) == pressedKeys.end()) {
            pressedKeys.emplace_back(key);
        }
        events_.emplace_back(RelaxedEventFactory::CreateKeyEvent(key, KeyAction::DOWN, pressedKeys));
    }
    for (auto iter = input.keys.rbegin(); iter != input.keys.rend(); ++iter) {
        pressedKeys.erase(std::remove(pressedKeys.begin(), pressedKeys.end(), *iter), pressedKeys.end());
        events_.emplace_back(RelaxedEventFactory::CreateKeyEvent(*iter, KeyAction::UP, pressedKeys));
    }
}

PressKeyExecutor::~PressKeyExecutor()
{
    Cancel();
}

void PressKeyExecutor::Cancel()
{
    if (completed_) {
        return;
    }
    auto pressedKeys = std::move(pressedKeys_);
    completed_ = true;
    nextEvent_ = events_.size();
    auto context = context_.Upgrade();
    if (!context || context->IsDestroyed()) {
        return;
    }
    while (!pressedKeys.empty()) {
        if (context->IsDestroyed()) {
            break;
        }
        const auto key = pressedKeys.back();
        pressedKeys.pop_back();
        context->OnNonPointerEvent(RelaxedEventFactory::CreateKeyEvent(key, KeyAction::UP, pressedKeys));
    }
}

ExecutorResult PressKeyExecutor::ExecuteStep()
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
        if (IsDenied(PointF())) {
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
    if (nextEvent_ == 0 && !holdStarted_) {
        holdStarted_ = true;
        holdStart_ = std::chrono::steady_clock::now();
        anyHandled_ |= DispatchKeyEvent(context, events_[nextEvent_++]);
        if (completed_) {
            return ExecutorResult::SUCCESS;
        }
        if (holdMs_ > 0) {
            return ExecutorResult::PENDING;
        }
    } else if (holdMs_ > 0 && nextEvent_ < keyCount_) {
        anyHandled_ |= DispatchKeyEvent(context, events_[nextEvent_++]);
    }
    if (completed_) {
        return ExecutorResult::SUCCESS;
    }
    if (holdMs_ > 0 && std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - holdStart_).count() < holdMs_) {
        return ExecutorResult::PENDING;
    }
    return FinishSequence(context);
}

ExecutorResult PressKeyExecutor::FinishSequence(const RefPtr<PipelineContext>& context)
{
    while (!completed_ && nextEvent_ < events_.size()) {
        if (context->IsDestroyed()) {
            Cancel();
            return SetFailureState(ExecutionState::CONTEXT_UNAVAILABLE);
        }
        anyHandled_ |= DispatchKeyEvent(context, events_[nextEvent_++]);
    }
    completed_ = true;
    if (!anyHandled_) {
        // Every DOWN/UP was swallowed (no focus or a custom dispatcher); report the
        // sequence as failed instead of a silent no-op success.
        return SetFailureState(ExecutionState::FAILED);
    }
    return ExecutorResult::SUCCESS;
}

bool PressKeyExecutor::DispatchKeyEvent(const RefPtr<PipelineContext>& context, KeyEvent event)
{
    if (event.action == KeyAction::DOWN) {
        if (std::find(pressedKeys_.begin(), pressedKeys_.end(), event.code) == pressedKeys_.end()) {
            pressedKeys_.emplace_back(event.code);
        }
    } else if (event.action == KeyAction::UP) {
        pressedKeys_.erase(std::remove(pressedKeys_.begin(), pressedKeys_.end(), event.code), pressedKeys_.end());
    }
    event.pressedCodes = pressedKeys_;
    return context->OnNonPointerEvent(event);
}
} // namespace OHOS::Ace::NG

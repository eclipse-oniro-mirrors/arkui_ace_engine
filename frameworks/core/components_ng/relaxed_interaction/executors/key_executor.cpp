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

#include "core/components_ng/relaxed_interaction/executors/key_executor.h"

#include "core/event/key_event.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {

KeyExecutor::KeyExecutor(WeakPtr<PipelineContext> context, int32_t keyCode)
    : BaseExecutor(context), keyCode_(keyCode) {}

ExecutorResult KeyExecutor::ExecuteStep()
{
    auto context = context_.Upgrade();
    if (!context) {
        return SetFailureState(ExecutionState::TARGET_NOT_FOUND);
    }

    KeyEvent downEvent(static_cast<KeyCode>(keyCode_), KeyAction::DOWN);
    downEvent.isInjected = true;
    downEvent.pressedCodes.push_back(downEvent.code);
    bool handled = context->OnNonPointerEvent(downEvent);
    if (context->IsDestroyed()) {
        return SetFailureState(ExecutionState::TARGET_NOT_FOUND);
    }
    KeyEvent upEvent(static_cast<KeyCode>(keyCode_), KeyAction::UP);
    upEvent.isInjected = true;
    bool upHandled = context->OnNonPointerEvent(upEvent);
    return (handled || upHandled) ? ExecutorResult::SUCCESS : SetFailureState();
}

} // namespace OHOS::Ace::NG

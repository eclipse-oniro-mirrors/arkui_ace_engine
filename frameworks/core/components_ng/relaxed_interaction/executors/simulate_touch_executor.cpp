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

#include "core/components_ng/relaxed_interaction/executors/simulate_touch_executor.h"

#include <string>
#include <unistd.h>

#include "base/log/log.h"
#include "base/utils/time_util.h"
#include "core/event/touch_event.h"
#include "core/pipeline_ng/pipeline_context.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/relaxed_interaction/utils/relaxed_event_factory.h"

namespace OHOS::Ace::NG {

SimulateTouchExecutor::SimulateTouchExecutor(WeakPtr<PipelineContext> context, const PointF& coordinates)
    : BaseExecutor(context), coordinates_(coordinates) {}

ExecutorResult SimulateTouchExecutor::ExecuteStep()
{
    if (cancelRequested_) {
        return SetFailureState(ExecutionState::INTERRUPTED_BY_USER);
    }
    auto context = context_.Upgrade();
    if (!context) {
        TAG_LOGE(AceLogTag::ACE_UIEVENT, "Pipeline context is null");
        return SetFailureState(ExecutionState::TARGET_NOT_FOUND);
    }

    // Create touch event
    TouchEvent downEvent = RelaxedEventFactory::CreateTouchEvent(coordinates_, TouchType::DOWN);
    TouchEvent upEvent = RelaxedEventFactory::CreateTouchEvent(coordinates_, TouchType::UP);

    // Dispatch touch events
    touchActive_ = true;
    context->OnTouchEvent(downEvent);
    if (context->IsDestroyed()) {
        touchActive_ = false;
        return SetFailureState(ExecutionState::TARGET_NOT_FOUND);
    }
    if (cancelRequested_) {
        Cancel();
        return SetFailureState(ExecutionState::INTERRUPTED_BY_USER);
    }
    touchActive_ = false;
    context->OnTouchEvent(upEvent);

    return ExecutorResult::SUCCESS;
}

void SimulateTouchExecutor::Cancel()
{
    RequestCancel();
    if (!touchActive_) {
        return;
    }
    touchActive_ = false;
    auto context = context_.Upgrade();
    if (context && !context->IsDestroyed()) {
        context->OnTouchEvent(RelaxedEventFactory::CreateTouchEvent(coordinates_, TouchType::CANCEL));
    }
}

} // namespace OHOS::Ace::NG
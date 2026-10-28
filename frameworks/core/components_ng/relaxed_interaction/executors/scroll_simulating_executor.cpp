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

#include "core/components_ng/relaxed_interaction/executors/scroll_simulating_executor.h"

#include <cmath>

#include "base/log/log.h"
#include "base/utils/system_properties.h"
#include "core/components_ng/event/input_event.h"
#include "core/components_ng/pattern/scroll/scroll_pattern.h"
#include "core/components_ng/relaxed_interaction/relaxed_interaction_types.h"
#include "core/components_ng/relaxed_interaction/utils/relaxed_event_factory.h"
#include "core/pipeline_ng/pipeline_context.h"

static constexpr double SPEED_PER_FRAME = 100.0;
namespace OHOS::Ace::NG {

std::queue<TouchEvent> ScrollSimulatingExecutor::TouchEventBuilder::Build(const Coordinates& coordinates)
{
    // DOWN
    PointF pointDown(coordinates.x1, coordinates.y1);
    TouchEvent down = RelaxedEventFactory::CreateTouchEvent(pointDown, TouchType::DOWN);
    touchEvents_.push(down);

    // MOVE
    MoveBySteps(coordinates.x1, coordinates.y1, coordinates.x2, coordinates.y2);

    // UP
    PointF pointUp(coordinates.x2, coordinates.y2);
    TouchEvent up = RelaxedEventFactory::CreateTouchEvent(pointUp, TouchType::UP);
    touchEvents_.push(up);

    return touchEvents_;
}

void ScrollSimulatingExecutor::TouchEventBuilder::MoveBySteps(double x1, double y1, double x2, double y2)
{
    int32_t steps = SumSteps(x1, y1, x2, y2, SPEED_PER_FRAME);
    steps = std::max(steps, 1);
    if (steps == 0) {
        return;
    }
    double stepX = (x2 - x1) / steps;
    double stepY = (y2 - y1) / steps;
    for (int32_t i = 1; i <= steps; ++i) {
        PointF point(static_cast<float>(x1 + stepX * i), static_cast<float>(y1 + stepY * i));
        TouchEvent move = RelaxedEventFactory::CreateTouchEvent(point, TouchType::MOVE);
        touchEvents_.push(move);
    }
}

int32_t ScrollSimulatingExecutor::TouchEventBuilder::SumSteps(double x1, double y1, double x2, double y2, double speed)
{
    double dx = x2 - x1;
    double dy = y2 - y1;
    double dist = std::sqrt(dx * dx + dy * dy);
    double steps = 0.0;
    if (speed != 0) {
        steps = dist / speed;
    }
    return static_cast<int32_t>(steps);
}

ScrollSimulatingExecutor::ScrollSimulatingExecutor(WeakPtr<PipelineContext> context, const Command& cmd)
    : BaseExecutor(context), cmd_(cmd)
{
    auto coordinates = cmd_.actionInfo.coordinates;
    if (!cmd_.isY2Set) {
        coordinates.x2 = coordinates.x1;
        coordinates.y2 = coordinates.y1;
        switch (cmd_.actionInfo.direction) {
            case RelaxedScrollDirection::LEFT:
                coordinates.x2 += cmd_.actionInfo.distance;
                break;
            case RelaxedScrollDirection::RIGHT:
                coordinates.x2 -= cmd_.actionInfo.distance;
                break;
            case RelaxedScrollDirection::BACKWARD:
            case RelaxedScrollDirection::UP:
                coordinates.y2 += cmd_.actionInfo.distance;
                break;
            default:
                coordinates.y2 -= cmd_.actionInfo.distance;
                break;
        }
    }
    touchEvents_ = TouchEventBuilder().Build(coordinates);
}

ExecutorResult ScrollSimulatingExecutor::ExecuteStep()
{
    auto context = context_.Upgrade();
    if (!context) {
        TAG_LOGD(AceLogTag::ACE_UIEVENT, "ScrollSimulatingExecutor failed.");
        return SetFailureState(ExecutionState::TARGET_NOT_FOUND);
    }

    if (touchEvents_.empty()) {
        TAG_LOGD(AceLogTag::ACE_UIEVENT, "ScrollSimulatingExecutor failed, touchEvents is empty.");
        return ExecutorResult::FAILED;
    }
    auto event = touchEvents_.front();
    touchActive_ = event.type == TouchType::DOWN || event.type == TouchType::MOVE;
    lastPosition_ = PointF(event.x, event.y);
    context->OnTouchEvent(event);
    touchEvents_.pop();
    if (touchEvents_.empty()) {
        return ExecutorResult::SUCCESS;
    }
    return ExecutorResult::PENDING;
}

void ScrollSimulatingExecutor::Cancel()
{
    if (!touchActive_) {
        return;
    }
    touchActive_ = false;
    auto context = context_.Upgrade();
    if (context && !context->IsDestroyed()) {
        context->OnTouchEvent(RelaxedEventFactory::CreateTouchEvent(lastPosition_, TouchType::CANCEL));
    }
}

} // namespace OHOS::Ace::NG
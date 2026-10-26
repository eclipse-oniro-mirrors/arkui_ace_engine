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

#include "core/components_ng/pattern/scrollable/scrollable.h"

#include <functional>

#include "base/log/log.h"
#include "base/thread/task_executor.h"
#include "base/utils/time_util.h"
#include "core/animation/scroll_motion.h"
#include "core/common/container.h"
#include "core/components_ng/base/modifier.h"
#include "core/components_ng/pattern/scrollable/item_snap_prediction.h"
#include "core/components_ng/pattern/scrollable/scrollable_animation_consts.h"
#include "core/components_ng/render/animation_utils.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
namespace {
// Spring curve for one item snap stage; long stages reduce the minimum amplitude ratio so the
// spring settles without visible oscillation around the target. Shape constants are shared
// with the legacy snap animation via scrollable_animation_consts.h.
RefPtr<ResponsiveSpringMotion> CreateItemSnapSpringCurve(float endPos, float currentPos)
{
    auto curve = AceType::MakeRefPtr<ResponsiveSpringMotion>(SNAP_SPRING_RESPONSE, SNAP_SPRING_DAMP, 0.0f);
    if (NearZero(endPos - currentPos)) {
        return curve;
    }
    auto minimumAmplitudeRatio =
        static_cast<float>(SNAP_SPRING_MINIMUM_AMPLITUDE_PX / std::abs(endPos - currentPos));
    minimumAmplitudeRatio = std::min(minimumAmplitudeRatio, SNAP_SPRING_AMPLITUDE_RATIO);
    if (LessNotEqualCustomPrecision(minimumAmplitudeRatio,
        ResponsiveSpringMotion::DEFAULT_RESPONSIVE_SPRING_AMPLITUDE_RATIO)) {
        curve->UpdateMinimumAmplitudeRatio(minimumAmplitudeRatio);
    }
    return curve;
}
// Re-evaluate the cached provider target after layout, without issuing another user callback.
std::function<void(float, float)> CreateItemSnapContinuation(const WeakPtr<Scrollable>& weak, bool fromScrollBar,
    int32_t source, std::function<float(float)>&& targetGetter, std::function<void(float)>&& onFinish)
{
    return [weak, fromScrollBar, source, targetGetter = std::move(targetGetter), onFinish = std::move(onFinish)](
               float providerVelocity, float motionVelocity) mutable {
        auto scroll = weak.Upgrade();
        CHECK_NULL_VOID(scroll);
        const float target = targetGetter(providerVelocity);
        if (!std::isfinite(target) || NearEqual(target, scroll->GetCurrentPos(), 0.1f)) {
            onFinish(motionVelocity);
            return;
        }
        scroll->StartItemSnapTwoStageAnimation(
            0.0f, motionVelocity, fromScrollBar, std::move(targetGetter), source);
    };
}
} // namespace

void Scrollable::SetItemSnapTakeoverDistance(float distance)
{
    itemSnapTakeoverDistance_ = distance;
}

void Scrollable::SetItemSnapFlingRate(float rate)
{
    itemSnapFlingRate_ = rate;
}

void Scrollable::SetItemSnapTrackingCallback(std::function<void()>&& callback)
{
    itemSnapTrackingCallback_ = std::move(callback);
}

bool Scrollable::IsItemSnapTracking() const
{
    return static_cast<bool>(itemSnapTrackingCallback_);
}

float Scrollable::GetItemSnapFlingRate() const
{
    return itemSnapFlingRate_;
}

void Scrollable::SetItemSnapTargetReady(bool ready)
{
    itemSnapTargetReady_ = ready;
}

void Scrollable::SetItemSnapLookaheadDistance(float distance)
{
    itemSnapLookaheadDistance_ = distance;
}

void Scrollable::RunItemSnapStage(
    float endPos, float velocity, bool finalStage, std::function<void(float, float)>&& onHandoff)
{
    endPos_ = endPos;
    // finalPosition_ carries the FINAL snap target only; during the approach stage it keeps its
    // previous value so interrupt logic never mistakes the approach waypoint for the target.
    if (finalStage) {
        finalPosition_ = endPos;
    }
    ACE_SCOPED_TRACE("Item snap stage start, start:%f, end:%f, vel:%f, final:%u, id:%d", currentPos_, endPos,
        velocity, finalStage, nodeId_);
    AnimationOption option;
    option.SetDuration(CUSTOM_SPRING_ANIMATION_DURATION);
    option.SetCurve(CreateItemSnapSpringCurve(endPos, currentPos_));
    if (!snapOffsetProperty_) {
        GetSnapProperty();
    }
    snapOffsetProperty_->Set(currentPos_);
    snapOffsetProperty_->SetPropertyUnit(PropertyUnit::PIXEL_POSITION);
    // The item snap chain owns its lifecycle via the generation token instead of
    // updateSnapAnimationCount_: interrupted/superseded stages are invalidated atomically, so a
    // late removed-callback can neither leak the counter nor hijack a newer chain's handoff.
    const uint32_t generation = ++itemSnapGeneration_;
    itemSnapChainActive_ = true;
    snapOffsetProperty_->AnimateWithVelocity(option, endPos, velocity,
        [weak = AceType::WeakClaim(this), id = Container::CurrentId(), finalStage, source = snapAnimationSource_,
            startVelocity = velocity, generation, onHandoff = std::move(onHandoff)]() mutable {
            ContainerScope scope(id);
            auto scroll = weak.Upgrade();
            CHECK_NULL_VOID(scroll);
            scroll->OnItemSnapStageFinished(generation, finalStage, source, startVelocity, std::move(onHandoff));
        });
    state_ = AnimationState::SNAP;
    auto context = context_.Upgrade();
    CHECK_NULL_VOID(context);
    lastVsyncTime_ = context->GetVsyncTime();
    MarkNeedFlushAnimationStartTime();
}

// Feat-20260806915268 item snap stage helpers (definitions live in scrollable_item_snap.cpp).
// Dispatches a finished stage by generation; posts the handoff as a UI task and re-checks
// the generation before running it. The handoff carries TWO velocities: providerVelocity
// feeds calculateSnapOffset's decision (the release fling speed — the spring is at rest
// when a stage completes, so the per-frame residual there is noise, not signal), while
// motionVelocity keeps the stage-2 spring's physical continuity (the actual residual).
void Scrollable::OnItemSnapStageFinished(uint32_t generation, bool finalStage, int32_t source, float startVelocity,
    std::function<void(float, float)>&& onHandoff)
{
    if (itemSnapGeneration_ != generation) {
        // Superseded by a newer stage or interrupted: this stage is fully invalidated.
        return;
    }
    if (finalStage && !onHandoff) {
        state_ = AnimationState::IDLE;
        nextStep_.reset();
        itemSnapChainActive_ = false;
        axisSnapDistance_ = 0.f;
        snapDirection_ = SnapDirection::NONE;
        snapAnimationFromScrollBar_ = false;
        ACE_SCOPED_TRACE("Item snap final stage finish, id:%d", nodeId_);
        ProcessScrollMotionStop(source);
        return;
    }
    if (onHandoff) {
        // Hand off while the snap chain is still current. A completed spring is at rest, so
        // the per-frame snapVelocity_ at this point is a small residual (1~10 px/s on a real
        // backend), NOT the fling speed the provider is documented to approximate — the
        // NearZero fallback below could never trigger there. Always report the stage start
        // (release) velocity to the provider; keep the residual only as the stage-2 spring's
        // physical initial velocity so the takeover stays visually continuous.
        PostItemSnapHandoff(generation, startVelocity, snapVelocity_, std::move(onHandoff));
    }
}

void Scrollable::PostItemSnapHandoff(
    uint32_t generation, float providerVelocity, float motionVelocity,
    std::function<void(float, float)>&& onHandoff)
{
    // Defer the handoff out of the animation completion callback: starting the stage-2 spring
    // reentrantly from inside the backend finish callback risks replace/reentrancy issues and
    // breaks animation scheduling. The handoff runs on the next UI task (frame boundary) and is
    // cancelled by a newer generation.
    auto context = context_.Upgrade();
    CHECK_NULL_VOID(context);
    auto weak = WeakClaim(this);
    auto id = Container::CurrentId();
    auto taskExecutor = SingleTaskExecutor::Make(context->GetTaskExecutor(), TaskExecutor::TaskType::UI);
    taskExecutor.PostTask(
        [weak, id, generation, providerVelocity, motionVelocity, onHandoff = std::move(onHandoff)]() mutable {
            ContainerScope scope(id);
            auto scroll = weak.Upgrade();
            CHECK_NULL_VOID(scroll);
            if (scroll->itemSnapGeneration_ != generation) {
                return;
            }
            onHandoff(providerVelocity, motionVelocity);
        },
        "ArkUIScrollItemSnapHandoff");
}

// Feat-20260806915268 cmetrics helper: the snap branch of ProcessAxisUpdateEvent, extracted
// verbatim (zero behavior change) to keep the parent under the 50-nbnc gate.
bool Scrollable::HandleAxisSnapUpdate(float mainDelta, bool fromScrollBar)
{
    auto snapType = GetSnapType();
    if (snapType == SnapType::NONE_SNAP || !startSnapAnimationCallback_) {
        return false;
    }
    auto context = context_.Upgrade();
    CHECK_NULL_RETURN(context, true);
    auto currentVsyncTime = context->GetVsyncTime();
    CHECK_NULL_RETURN(lastAxisVsyncTime_ != currentVsyncTime, true);
    auto snapDelta = 0.f;
    auto snapDirection = SnapDirection::NONE;
    auto isInitScroll = (snapType == SnapType::LIST_SNAP && snapDirection_ == SnapDirection::NONE) ||
                        (snapType == SnapType::SCROLL_SNAP && state_ == AnimationState::IDLE);
    if (isInitScroll) {
        snapDirection = Positive(mainDelta) ? SnapDirection::FORWARD : SnapDirection::BACKWARD;
    } else {
        if (snapDirection_ == SnapDirection::BACKWARD && !Positive(mainDelta)) {
            snapDelta = endPos_ - currentPos_;
            snapDirection = SnapDirection::BACKWARD;
        } else if (snapDirection_ == SnapDirection::FORWARD && Positive(mainDelta)) {
            snapDelta = endPos_ - currentPos_;
            snapDirection = SnapDirection::FORWARD;
        } else if ((snapDirection_ == SnapDirection::BACKWARD && Positive(mainDelta)) ||
                   (snapDirection_ == SnapDirection::FORWARD && !Positive(mainDelta))) {
            snapDelta = 0.f;
            snapDirection = SnapDirection::NONE;
        }
    }
    ACE_SCOPED_TRACE("ProcessAxisUpdateEvent start SnapAnimation, snapDelta:%f, snapDirection:%d, "
                     "lastSnapDirection:%d, id:%d, tag:%s",
        snapDelta, snapDirection, snapDirection_, nodeId_, nodeTag_.c_str());
    SnapAnimationOptions snapAnimationOptions = { .snapDelta = snapDelta, .animationVelocity = currentVelocity_,
        .snapDirection = snapDirection, .source = SCROLL_FROM_AXIS, .fromScrollBar = fromScrollBar };
    if (startSnapAnimationCallback_(snapAnimationOptions)) {
        // Only a started snap animation consumes the vsync slot; a declined attempt must
        // stay retryable within the same frame (keyboard reverse key declines with zero delta).
        lastAxisVsyncTime_ = currentVsyncTime;
        // A provider strategy only PARKS its kickoff (StartItemSnapAnimation returns true
        // before running). The axis path has no HandleDragEnd / StartScrollAnimation exit
        // that would flush it, so without this the wheel scrolls nothing on provider
        // grids. Built-in align has nothing parked; the flush is a no-op there.
        FlushPendingItemSnapKickoff();
    }
    auto isNeedAdjustDirection = (snapType == SnapType::SCROLL_SNAP && snapDirection == SnapDirection::NONE);
    if (isNeedAdjustDirection) {
        snapDirection_ = Positive(mainDelta) ? SnapDirection::FORWARD : SnapDirection::BACKWARD;
    } else {
        snapDirection_ = snapDirection;
    }
    return true;
}

// Feat-20260806915268: two-stage item snap for a custom ScrollSnapOffsetProvider. Stage 1 (approach)
// springs from the current position by [approachDelta]; on completion the [targetGetter] is
// invoked with the handoff velocity to resolve the absolute final target, and stage 2 springs
// to that target. The provider receives the stage start (release fling) velocity; the stage-2
// spring receives the physical residual so the motion stays continuous (see
// OnItemSnapStageFinished). Both stages are interruptible by a new gesture through the
// existing snap-animation stop paths. When [approachDelta] is near zero the approach stage
// is skipped.
void Scrollable::StartItemSnapTwoStageAnimation(float approachDelta, float scrollSnapVelocity, bool fromScrollBar,
    std::function<float(float handoffVelocity)>&& targetGetter, int32_t source)
{
    snapAnimationFromScrollBar_ = fromScrollBar;
    snapAnimationSource_ = source;
    if (!snapOffsetProperty_) {
        GetSnapProperty();
    }
    snapVelocity_ = scrollSnapVelocity;

    auto handoff = [weak = AceType::WeakClaim(this), targetGetter = std::move(targetGetter), fromScrollBar, source](
                       float providerVelocity, float motionVelocity) mutable {
        auto scroll = weak.Upgrade();
        CHECK_NULL_VOID(scroll);
        if (!targetGetter) {
            return;
        }
        float resolved = targetGetter(providerVelocity);
        if (!std::isfinite(resolved)) {
            // Non-finite final target: cancel the snap chain and restore the idle state so the
            // scrollable keeps responding to gestures (the position stays at the approach end).
            scroll->state_ = AnimationState::IDLE;
            scroll->itemSnapChainActive_ = false;
            scroll->nextStep_.reset();
            scroll->axisSnapDistance_ = 0.f;
            scroll->snapDirection_ = SnapDirection::NONE;
            scroll->ProcessScrollMotionStop(source);
            return;
        }
        scroll->snapAnimationFromScrollBar_ = fromScrollBar;
        scroll->snapAnimationSource_ = source;
        auto finish = [weak, source](float velocity) {
            auto current = weak.Upgrade();
            CHECK_NULL_VOID(current);
            current->OnItemSnapStageFinished(current->itemSnapGeneration_, true, source, velocity, nullptr);
        };
        scroll->RunItemSnapStage(resolved, motionVelocity, true,
            CreateItemSnapContinuation(weak, fromScrollBar, source, std::move(targetGetter), std::move(finish)));
    };

    if (NearZero(approachDelta)) {
        handoff(scrollSnapVelocity, scrollSnapVelocity);
        return;
    }
    RunItemSnapStage(currentPos_ + approachDelta, scrollSnapVelocity, false, std::move(handoff));
}

bool Scrollable::TryItemSnapTakeover()
{
    if (state_ != AnimationState::FRICTION || !needItemSnapSettle_ || !startSnapAnimationCallback_ ||
        (itemSnapTrackingCallback_ && !itemSnapTargetReady_) ||
        GetSnapType() == SnapType::NONE_SNAP ||
        !std::isfinite(itemSnapTakeoverDistance_) || !GreatNotEqual(itemSnapTakeoverDistance_, 0.0f) ||
        GreatNotEqual(std::abs(finalPosition_ - currentPos_), itemSnapTakeoverDistance_) ||
        isTouching_ || scrollPause_ || canOverScroll_) {
        return false;
    }
    auto context = context_.Upgrade();
    CHECK_NULL_RETURN(context, false);
    auto executor = context->GetTaskExecutor();
    CHECK_NULL_RETURN(executor, false);
    const auto generation = ++itemSnapGeneration_;
    const auto remaining = finalPosition_ - currentPos_;
    const auto velocity = currentVelocity_;
    needItemSnapSettle_ = false;
    itemSnapTakeoverDistance_ = 0.0f;
    // A removed friction callback must not stop the replacement spring.
    ++frictionGeneration_;
    StopFrictionAnimation();
    state_ = AnimationState::TRANSITION;
    itemSnapTakeoverPending_ = true;
    const auto id = Container::CurrentId();
    ACE_SCOPED_TRACE("Item snap takeover, pos:%f, remaining:%f, velocity:%f, id:%d",
        currentPos_, remaining, velocity, nodeId_);
    auto posted = executor->PostTask([weak = WeakClaim(this), generation, remaining, velocity, id]() {
        ContainerScope scope(id);
        auto scroll = weak.Upgrade();
        CHECK_NULL_VOID(scroll);
        scroll->CompleteItemSnapTakeover(generation, remaining, velocity);
    }, TaskExecutor::TaskType::UI, "ArkUIScrollItemSnapTakeover");
    if (!posted) {
        itemSnapTakeoverPending_ = false;
        state_ = AnimationState::IDLE;
        ProcessScrollMotionStop(SCROLL_FROM_ANIMATION);
    }
    return true;
}

void Scrollable::CompleteItemSnapTakeover(uint32_t generation, float remaining, float velocity)
{
    if (itemSnapGeneration_ != generation || isTouching_ ||
        state_ != AnimationState::TRANSITION) {
        return;
    }
    SnapAnimationOptions options {
        .snapDelta = remaining,
        .animationVelocity = velocity,
        .fromItemSnapFlingStop = true,
        .fromItemSnapTakeover = true,
    };
    itemSnapTakeoverPending_ = false;
    state_ = AnimationState::IDLE;
    if (startSnapAnimationCallback_ && startSnapAnimationCallback_(options)) {
        FlushPendingItemSnapKickoff();
        // An idle frame must not backdate the replacement spring.
        auto pipeline = context_.Upgrade();
        if (pipeline && pipeline->GetWindow()) {
            const auto startTime = ResolveItemSnapStartTime(
                pipeline->GetVsyncTime(), static_cast<uint64_t>(GetSysTimestamp()));
            pipeline->GetWindow()->FlushAnimationStartTime(startTime);
            // The old friction may have delivered its last frame before this queued task.
            pipeline->GetWindow()->RequestFrame();
        }
        return;
    }
    ProcessScrollMotionStop(SCROLL_FROM_ANIMATION);
}

} // namespace OHOS::Ace::NG

/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

#include "core/components_ng/base/modifier.h"

#include "base/log/jank_frame_report.h"
#include "core/animation/scroll_motion.h"
#include "base/perfmonitor/perf_constants.h"
#include "base/perfmonitor/perf_monitor.h"
#include "base/ressched/ressched_report.h"
#include "base/utils/multi_thread.h"
#include "core/common/container.h"
#include "core/common/layout_inspector.h"
#include "core/components_ng/pattern/scrollable/item_snap_prediction.h"
#include "core/components_ng/pattern/scrollable/scrollable_animation_consts.h"
#include "core/components_ng/pattern/scrollable/scrollable_theme.h"
#include "core/components_ng/render/animation_utils.h"
#include "core/pipeline_ng/pipeline_context.h"
#include "base/log/event_report.h"
#include "core/pipeline/base/constants.h"
#include "core/components/common/properties/os_content.h"

namespace OHOS::Ace::NG {
namespace {

constexpr double CAP_COEFFICIENT = 0.45;
constexpr int32_t FIRST_THRESHOLD = 4;
constexpr int32_t SECOND_THRESHOLD = 10;
constexpr double CAP_FIXED_VALUE = 16.0;
constexpr uint32_t DRAG_INTERVAL_TIME = 400;
constexpr uint32_t MULTI_FLING_DISTANCE = 125;

#ifndef WEARABLE_PRODUCT
constexpr double FRICTION = 0.6;
constexpr double API11_FRICTION = 0.7;
constexpr double API12_FRICTION = 0.75;
constexpr double SLOW_FRICTION_THRESHOLD = 3000.0;
constexpr double SLOW_FRICTION = 1.0;
constexpr double VELOCITY_SCALE = 1.0;
constexpr double SLOW_VELOCITY_SCALE = 1.2;
constexpr double NEW_VELOCITY_SCALE = 1.5;
constexpr double ADJUSTABLE_VELOCITY = 3000.0;
#else
constexpr double DISTANCE_EPSILON = 1.0;
constexpr double FRICTION = 0.9;
constexpr double VELOCITY_SCALE = 0.8;
constexpr double ADJUSTABLE_VELOCITY = 0.0;
#endif
constexpr uint64_t MILLOS_PER_NANO_SECONDS = 1000 * 1000 * 1000;
constexpr uint64_t MIN_DIFF_VSYNC = 1000 * 1000; // min is 1ms
constexpr float DEFAULT_THRESHOLD = 0.75f;
constexpr float SLOW_SPRING_RESPONSE = ITEM_SNAP_FIXED_RESPONSE;
constexpr float SLOW_SPRING_DAMP = ITEM_SNAP_FIXED_DAMPING;
constexpr uint32_t MAX_VSYNC_DIFF_TIME = 100 * 1000 * 1000; // max 100 ms
constexpr float START_FRICTION_VELOCITY_THRESHOLD = 240.0f;
constexpr float FRICTION_VELOCITY_THRESHOLD = 120.0f;
constexpr float SPRING_ACCURACY = 0.1;
constexpr float SCROLL_SNAP_MIN_STEP_THRESHOLD = 10.0f;
constexpr float SCROLL_SNAP_MIN_STEP = 1.0f;
#ifdef OHOS_PLATFORM
constexpr int64_t INCREASE_CPU_TIME_ONCE = 4000000000; // 4s(unit: ns)
#endif

#ifdef SUPPORT_DIGITAL_CROWN
constexpr double ANGULAR_VELOCITY_FACTOR  = 0.001f;
constexpr float ANGULAR_VELOCITY_SLOW = 0.07f;
constexpr float ANGULAR_VELOCITY_MEDIUM = 0.2f;
constexpr float ANGULAR_VELOCITY_FAST = 0.54f;
constexpr float DISPLAY_CONTROL_RATIO_VERY_SLOW = 1.19f;
constexpr float DISPLAY_CONTROL_RATIO_SLOW = 1.87f;
constexpr float DISPLAY_CONTROL_RATIO_MEDIUM = 1.67f;
constexpr float DISPLAY_CONTROL_RATIO_FAST = 1.59f;
constexpr float CROWN_SENSITIVITY_LOW = 0.8f;
constexpr float CROWN_SENSITIVITY_MEDIUM = 1.0f;
constexpr float CROWN_SENSITIVITY_HIGH = 1.2f;

constexpr float CROWN_START_FRICTION_VELOCITY_THRESHOLD = 6.0f;
#endif

RefPtr<ResponsiveSpringMotion> CreateScrollSnapSpringCurve(float scrollSnapDelta, bool useSlowSpring)
{
    auto curve = AceType::MakeRefPtr<ResponsiveSpringMotion>(
        useSlowSpring ? SLOW_SPRING_RESPONSE : SNAP_SPRING_RESPONSE,
        useSlowSpring ? SLOW_SPRING_DAMP : SNAP_SPRING_DAMP, 0.0f);
    auto minimumAmplitudeRatio = SNAP_SPRING_MINIMUM_AMPLITUDE_PX / std::abs(scrollSnapDelta);
    minimumAmplitudeRatio = std::min(minimumAmplitudeRatio, SNAP_SPRING_AMPLITUDE_RATIO);
    if (LessNotEqualCustomPrecision(minimumAmplitudeRatio,
        ResponsiveSpringMotion::DEFAULT_RESPONSIVE_SPRING_AMPLITUDE_RATIO)) {
        curve->UpdateMinimumAmplitudeRatio(minimumAmplitudeRatio);
    }
    return curve;
}

} // namespace

Scrollable::Scrollable() = default;

Scrollable::Scrollable(ScrollPositionCallback&& callback, Axis axis)
    : callback_(std::move(callback)), axis_(axis)
{}

Scrollable::Scrollable(const ScrollPositionCallback& callback, Axis axis)
    : callback_(callback), axis_(axis)
{}

double Scrollable::GetVelocityScale()
{
    return Container::GreatOrEqualAPITargetVersion(PlatformVersion::VERSION_ELEVEN) ? NEW_VELOCITY_SCALE
                                                                                    : VELOCITY_SCALE;
}

Scrollable::~Scrollable()
{
    // If animation still runs, force stop it.
    if (!IsStopped()) {
#ifndef CROSS_PLATFORM
        PerfMonitor::GetPerfMonitor()->EndCommercial(PerfConstants::APP_LIST_FLING, false);
#endif
        AceAsyncTraceEndCommercial(0, (TRAILING_ANIMATION + std::to_string(nodeId_) + std::string(" ") +
            nodeTag_).c_str());
        if (!context_.Invalid()) {
            auto context = context_.Upgrade();
            if (context != nullptr) {
                context->SetUiDvsyncSwitch(false);
            }
        }
    }
    StopFrictionAnimation();
    StopSpringAnimation();
    StopSnapAnimation();
}

void Scrollable::Initialize(const RefPtr<FrameNode>& host)
{
    CHECK_NULL_VOID(host);
    weakHost_ = host;
    auto pipeline = host->GetContextRefPtr();
    context_ = pipeline;
    InitPanRecognizerNG();
    available_ = true;
    CHECK_NULL_VOID(pipeline);
    auto scrollableTheme = pipeline->GetTheme<ScrollableTheme>();
    CHECK_NULL_VOID(scrollableTheme);
    flingVelocityScale_ = pipeline->GreatOrEqualAPITargetVersion(PlatformVersion::VERSION_ELEVEN)
                              ? scrollableTheme->GetFlingVelocityScale()
                              : VELOCITY_SCALE;
    springVelocityScale_ = pipeline->GreatOrEqualAPITargetVersion(PlatformVersion::VERSION_ELEVEN)
                               ? scrollableTheme->GetSpringVelocityScale()
                               : VELOCITY_SCALE;
    ratio_ = scrollableTheme->GetRatio();
    springResponse_ = scrollableTheme->GetSpringResponse();
    touchPadVelocityScaleRate_ = scrollableTheme->GetTouchPadVelocityScaleRate();
    if (friction_ == -1) {
        if (pipeline->GreatOrEqualAPITargetVersion(PlatformVersion::VERSION_THIRTEEN)) {
            defaultFriction_ = scrollableTheme->GetFriction();
        } else if (pipeline->GreatOrEqualAPITargetVersion(PlatformVersion::VERSION_TWELVE)) {
            defaultFriction_ = API12_FRICTION;
        } else if (pipeline->GreatOrEqualAPITargetVersion(PlatformVersion::VERSION_ELEVEN)) {
            defaultFriction_ = API11_FRICTION;
        } else {
            defaultFriction_ = FRICTION;
        }
    }
}

void Scrollable::InitAxisAnimator()
{
    CHECK_NULL_VOID(!axisAnimator_);
    auto axisAnimationCallback = [weak = WeakClaim(this)](float offset) {
        auto scrollable = weak.Upgrade();
        CHECK_NULL_VOID(scrollable);
        scrollable->ReportToDragFRCScene(scrollable->currentVelocity_, NG::SceneStatus::RUNNING);
        scrollable->ProcessScrollMotion(offset, SCROLL_FROM_AXIS);
    };
    auto axisAnimationStartCallback = [weak = WeakClaim(this)](float position) {
        auto scrollable = weak.Upgrade();
        CHECK_NULL_VOID(scrollable && scrollable->onScrollStartRec_);
        scrollable->ReportToDragFRCScene(scrollable->currentVelocity_, NG::SceneStatus::START);
        scrollable->onScrollStartRec_(position);
    };
    auto axisAnimationFinishCallback = [weak = WeakClaim(this)]() {
        auto scrollable = weak.Upgrade();
        CHECK_NULL_VOID(scrollable);
        scrollable->ReportToDragFRCScene(scrollable->currentVelocity_, NG::SceneStatus::END);
        scrollable->ProcessScrollMotionStop(SCROLL_FROM_AXIS);
    };
    axisAnimator_ = AceType::MakeRefPtr<AxisAnimator>(std::move(axisAnimationCallback),
        std::move(axisAnimationStartCallback), std::move(axisAnimationFinishCallback));
    axisAnimator_->Initialize(context_);
}

void Scrollable::InitPanRecognizerNG()
{
    PanDirection panDirection;
    panDirection.type = axis_ == Axis::VERTICAL ? PanDirection::VERTICAL : PanDirection::HORIZONTAL;
    double distance = SystemProperties::GetScrollableDistance();
    PanDistanceMap distanceMap = { { SourceTool::UNKNOWN, distance } };
    if (LessOrEqual(distance, 0.0)) {
        distanceMap = { { SourceTool::UNKNOWN, DEFAULT_PAN_DISTANCE.ConvertToPx() },
            { SourceTool::PEN, DEFAULT_PEN_PAN_DISTANCE.ConvertToPx() } };
    }
    panRecognizerNG_ =
        AceType::MakeRefPtr<NG::PanRecognizer>(DEFAULT_PAN_FINGER, panDirection, distanceMap);
    panRecognizerNG_->SetIsAllowMouse(false);
    SetOnActionStart();
    SetOnActionUpdate();
    SetOnActionExtUpdate();
    SetOnActionEnd();
    SetOnActionCancel();
    SetPanEndCallback();
}

void Scrollable::SetOnActionStart()
{
    CHECK_NULL_VOID(panRecognizerNG_);
    auto actionStart = [weakScroll = AceType::WeakClaim(this)](const GestureEvent& info) {
        auto scroll = weakScroll.Upgrade();
        CHECK_NULL_VOID(scroll);
        scroll->HandleDragStart(info);
    };
    panRecognizerNG_->SetOnActionStart(actionStart);
}

void Scrollable::SetOnActionExtUpdate()
{
    CHECK_NULL_VOID(panRecognizerNG_);
    auto actionExtUpdate = [weakScroll = AceType::WeakClaim(this)](const GestureEvent& info) {
        auto scroll = weakScroll.Upgrade();
        CHECK_NULL_VOID(scroll);
        scroll->HandleExtDragUpdate();
    };
    panRecognizerNG_->SetOnActionExtUpdate(actionExtUpdate);
}

void Scrollable::SetOnActionUpdate()
{
    CHECK_NULL_VOID(panRecognizerNG_);
    auto actionUpdate = [weakScroll = AceType::WeakClaim(this)](const GestureEvent& info) {
        auto scroll = weakScroll.Upgrade();
        CHECK_NULL_VOID(scroll);
        scroll->HandleDragUpdate(info);
    };
    panRecognizerNG_->SetOnActionUpdate(actionUpdate);
}

void Scrollable::SetOnActionEnd()
{
    CHECK_NULL_VOID(panRecognizerNG_);
    auto actionEnd = [weakScroll = AceType::WeakClaim(this)](GestureEvent& info) {
        auto scroll = weakScroll.Upgrade();
        if (scroll) {
            scroll->HandleDragEnd(info);
            scroll->ProcessPanActionEndEvents(info);
            scroll->isDragging_ = false;
        }
    };
    panRecognizerNG_->SetOnActionEnd(actionEnd);
}

void Scrollable::SetPanEndCallback()
{
    CHECK_NULL_VOID(panRecognizerNG_);
    auto panEndCallback = [weakScroll = AceType::WeakClaim(this)](GestureEvent& info) {
        auto scroll = weakScroll.Upgrade();
        if (scroll) {
            auto tempInfo = info;
            tempInfo.SetMainDelta(0.0);
            tempInfo.SetMainVelocity(0.0);
            ACE_SCOPED_TRACE("Trigger PanEndCallback, id:%d, tag:%s", scroll->nodeId_, scroll->nodeTag_.c_str());
            scroll->HandleDragEnd(tempInfo, true);
            scroll->ProcessPanActionEndEvents(tempInfo);
            scroll->isDragging_ = false;
        }
    };
    panRecognizerNG_->SetPanEndCallback(panEndCallback);
}

void Scrollable::ProcessPanActionEndEvents(const GestureEvent& info)
{
    CHECK_NULL_VOID(!panActionEndEvents_.empty());
    std::for_each(panActionEndEvents_.begin(), panActionEndEvents_.end(), [info](GestureEventFunc& event) {
        auto gestureInfo = info;
        event(gestureInfo);
    });
}

void Scrollable::SetOnActionCancel()
{
    CHECK_NULL_VOID(panRecognizerNG_);
    auto actionCancel = [weakScroll = AceType::WeakClaim(this)](const GestureEvent& info) {
        auto scroll = weakScroll.Upgrade();
        if (!scroll) {
            return;
        }
        if (scroll->dragCancelCallback_) {
            scroll->dragCancelCallback_();
        }
        GestureEvent nullInfo;
        scroll->HandleDragEnd(nullInfo);
        if (scroll->panActionEndEvents_.empty()) {
            scroll->isDragging_ = false;
            return;
        }
        std::for_each(scroll->panActionEndEvents_.begin(), scroll->panActionEndEvents_.end(),
            [nullInfo](GestureEventFunc& event) {
                auto gestureInfo = nullInfo;
                event(gestureInfo);
            });
        scroll->isDragging_ = false;
    };
    panRecognizerNG_->SetOnActionCancel(actionCancel);
}

#ifdef SUPPORT_DIGITAL_CROWN
void Scrollable::ListenDigitalCrownEvent(const RefPtr<FrameNode>& frameNode)
{
    CHECK_NULL_VOID(frameNode);
    auto focusHub = frameNode->GetFocusHub();
    CHECK_NULL_VOID(focusHub);

    auto onCrownEvent = [weakScroll = AceType::WeakClaim(this), weakNode = AceType::WeakClaim(AceType::RawPtr(
        frameNode))](const CrownEvent& event) -> bool {
        auto scroll = weakScroll.Upgrade();
        CHECK_NULL_RETURN(scroll, false);
        auto node = weakNode.Upgrade();
        CHECK_NULL_RETURN(node, false);

        auto centerOffset = node->GetGeometryNode()->GetContentRect().Center();
        scroll->HandleCrownEvent(event, centerOffset);
        return true;
    };
    focusHub->SetOnCrownEventInternal(std::move(onCrownEvent));

    focusHub->SetOnBlurReasonInternal([weak = WeakClaim(this)](const BlurReason& blurReason) {
        auto scroll = weak.Upgrade();
        CHECK_NULL_VOID(scroll);
        GestureEvent info;
        info.SetSourceDevice(SourceType::CROWN);
        info.SetSourceTool(SourceTool::UNKNOWN);
        scroll->HandleCrownActionCancel(info);
    });
}

double Scrollable::GetCrownRotatePx(const CrownEvent& event) const
{
    double velocity = std::abs(event.angularVelocity * ANGULAR_VELOCITY_FACTOR);
    double px = 0.0;
    if (LessOrEqualCustomPrecision(velocity, ANGULAR_VELOCITY_SLOW, 0.01f)) {  // very slow
        px = (Dimension(DISPLAY_CONTROL_RATIO_VERY_SLOW, DimensionUnit::VP) * event.degree).ConvertToPx();
    } else if (LessOrEqualCustomPrecision(velocity, ANGULAR_VELOCITY_MEDIUM, 0.01f)) {  // slow
        px = (Dimension(DISPLAY_CONTROL_RATIO_SLOW, DimensionUnit::VP) * event.degree).ConvertToPx();
    } else if (LessOrEqualCustomPrecision(velocity, ANGULAR_VELOCITY_FAST, 0.01f)) {  // medium
        px = (Dimension(DISPLAY_CONTROL_RATIO_MEDIUM, DimensionUnit::VP) * event.degree).ConvertToPx();
    } else {  // fast
        px = (Dimension(DISPLAY_CONTROL_RATIO_FAST, DimensionUnit::VP) * event.degree).ConvertToPx();
    }
    switch (crownSensitivity_) {
        case CrownSensitivity::LOW:
            px *= CROWN_SENSITIVITY_LOW;
            break;
        case CrownSensitivity::MEDIUM:
            px *= CROWN_SENSITIVITY_MEDIUM;
            break;
        case CrownSensitivity::HIGH:
            px *= CROWN_SENSITIVITY_HIGH;
            break;
        default:
            break;
    }
    return px;
}

void Scrollable::UpdateCrownVelocity(const TimeStamp& timeStamp, double mainDelta, bool end)
{
    if (axis_ == Axis::VERTICAL) {
        accumulativeCrownPx_ += Offset(0, mainDelta);
    } else {
        accumulativeCrownPx_ += Offset(mainDelta, 0);
    }
    crownVelocityTracker_.UpdateTrackerPoint(accumulativeCrownPx_.GetX(), accumulativeCrownPx_.GetY(), timeStamp, end);
}

void Scrollable::HandleCrownEvent(const CrownEvent& event, const OffsetF& center)
{
    DimensionOffset centerDimension(center);
    Offset globalLocation(centerDimension.GetX().ConvertToPx(), centerDimension.GetY().ConvertToPx());

    GestureEvent info;
    info.SetSourceDevice(SourceType::CROWN);
    info.SetSourceTool(SourceTool::UNKNOWN);
    info.SetGlobalLocation(globalLocation);
    double mainDelta = GetCrownRotatePx(event);

    switch (event.action) {
        case CrownAction::BEGIN:
            crownEventNum_ = 0;
            TAG_LOGI(AceLogTag::ACE_SCROLLABLE, "-->BEGIN]");
            HandleCrownActionBegin(event.timeStamp, mainDelta, info);
            break;
        case CrownAction::UPDATE:
            TAG_LOGI(AceLogTag::ACE_SCROLLABLE, "-->UPDATE]");
            HandleCrownActionUpdate(event.timeStamp, mainDelta, info);
            break;
        case CrownAction::END:
            TAG_LOGI(AceLogTag::ACE_SCROLLABLE, "-->END]");
            HandleCrownActionEnd(event.timeStamp, mainDelta, info);
            break;
        default:
            HandleCrownActionCancel(info);
            break;
    }
}

void Scrollable::HandleCrownActionBegin(const TimeStamp& timeStamp, double mainDelta, GestureEvent& info)
{
    if (isDragging_) {
        return;
    }
    accumulativeCrownPx_.Reset();
    crownVelocityTracker_.Reset();
    UpdateCrownVelocity(timeStamp, mainDelta, false);
    info.SetMainDelta(mainDelta);
    info.SetMainVelocity(crownVelocityTracker_.GetMainAxisVelocity());
    isDragging_ = true;
    isCrownEventDragging_ = true;
    isCrownDragging_ = true;
    HandleDragStart(info);
}

void Scrollable::HandleCrownActionUpdate(const TimeStamp& timeStamp, double mainDelta, GestureEvent& info)
{
    if (!isCrownEventDragging_) {
        return;
    }
    UpdateCrownVelocity(timeStamp, mainDelta, false);
    info.SetMainDelta(mainDelta);
    info.SetMainVelocity(crownVelocityTracker_.GetMainAxisVelocity());
    HandleDragUpdate(info);
}

void Scrollable::HandleCrownActionEnd(const TimeStamp& timeStamp, double mainDelta, GestureEvent& info)
{
    if (!isCrownEventDragging_) {
        return;
    }
    if (NearZero(mainDelta)) {
        info.SetMainDelta(crownVelocityTracker_.GetMainAxisDeltaPos());
        info.SetMainVelocity(crownVelocityTracker_.GetMainAxisVelocity());
    } else {
        UpdateCrownVelocity(timeStamp, mainDelta, true);
        info.SetMainDelta(mainDelta);
        info.SetMainVelocity(crownVelocityTracker_.GetMainAxisVelocity());
    }
    HandleDragEnd(info);
    if (actionEnd_) {
        actionEnd_(info);
    }
    isDragging_ = false;
    isCrownDragging_ = false;
    auto context = context_.Upgrade();
    CHECK_NULL_VOID(context);
    auto taskExecutor = SingleTaskExecutor::Make(context->GetTaskExecutor(), TaskExecutor::TaskType::UI);
    crownTask_.Reset([weak = WeakClaim(this)] {
        auto scrollable = weak.Upgrade();
        if (scrollable) {
            scrollable->SetCrownEventDragging(false);
        }
    });
    taskExecutor.PostDelayedTask(crownTask_, CUSTOM_SPRING_ANIMATION_DURATION, "ArkUIUpdateCrownEventDrag");
}

void Scrollable::HandleCrownActionCancel(GestureEvent& info)
{
    if (!isDragging_ || !isCrownEventDragging_) {
        return;
    }

    if (dragCancelCallback_) {
        dragCancelCallback_();
    }
    info.SetMainDelta(0);
    info.SetMainVelocity(0);
    HandleDragEnd(info);
    if (actionEnd_) {
        actionEnd_(info);
    }
    isDragging_ = false;
    isCrownDragging_ = false;
}
#endif

void Scrollable::SetAxis(Axis axis)
{
    axis_ = axis;
    PanDirection panDirection;
    if (axis_ == Axis::NONE) {
        panDirection.type = PanDirection::NONE;
    } else if (axis_ == Axis::VERTICAL) {
        panDirection.type = PanDirection::VERTICAL;
    } else {
        panDirection.type = PanDirection::HORIZONTAL;
    }
    if (panRecognizerNG_) {
        panRecognizerNG_->SetDirection(panDirection);
    }
#ifdef SUPPORT_DIGITAL_CROWN
    crownVelocityTracker_.SetMainAxis(axis_);
#endif
}

void Scrollable::HandleTouchDown(bool fromcrown)
{
    if ((state_ != AnimationState::TRANSITION && state_ != AnimationState::IDLE) || itemSnapTakeoverPending_ ||
        isScrollBarDragging_ || isSmartGestureFling_) {
        isTouchStopAnimation_ = true;
    } else {
        isTouchStopAnimation_ = false;
    }
    if (!fromcrown) {
        isTouching_ = true;
    }
    // A new gesture takes over: cancel the item snap chain BEFORE stopping the animations —
    // stopping replaces the spring with a 0ms property animation which can deliver the removed
    // completion callback synchronously, and that must already see a stale token.
    CancelItemSnapChain();
    // If animation still runs, first stop it.
    ACE_SCOPED_TRACE("HandleTouchDown, panDirection:%u, id:%d, tag:%s", GetPanDirection(), nodeId_, nodeTag_.c_str());
    StopSpringAnimation();
    if (state_ == AnimationState::FRICTION) {
        StopFrictionAnimation();
    } else if (state_ == AnimationState::SNAP) {
        StopSnapAnimation();
    }
}

void Scrollable::CheckStopFlingInTouchUp()
{
    if (!isDragging_ && !isScrollBarDragging_ && !isSmartGestureFling_ && isTouchStopAnimation_) {
        isUserFling_ = false;
        if (onDidStopFlingCallback_) {
            onDidStopFlingCallback_();
        }
    }
    isTouchStopAnimation_ = false;
}

void Scrollable::HandleTouchUp()
{
    // Two fingers are alternately drag, one finger is released without triggering spring animation.
    ACE_SCOPED_TRACE("HandleTouchUp, isDragging_:%u, nestedScrolling_:%u id:%d, tag:%s", isDragging_, nestedScrolling_,
        nodeId_, nodeTag_.c_str());
    if (isDragging_) {
        isTouchStopAnimation_ = false;
        return;
    }
    isTouching_ = false;
    if (nestedScrolling_) {
        CheckStopFlingInTouchUp();
        return;
    }
    if (CanStayOverScroll()) {
        CheckStopFlingInTouchUp();
        return;
    }
    // outBoundaryCallback_ is only set in ScrollablePattern::SetEdgeEffect and when the edge effect is spring
    if (outBoundaryCallback_ && outBoundaryCallback_(false)) {
        if (state_ != AnimationState::SPRING && scrollOverCallback_ && !isScrollBarDragging_) {
            if (onScrollStartRec_) {
                onScrollStartRec_(static_cast<float>(axis_));
            }
            ProcessScrollOverCallback(0.0);
        }
        isTouchStopAnimation_ = false;
        return;
    }
    if (state_ != AnimationState::SNAP && startSnapAnimationCallback_) {
        SnapAnimationOptions snapAnimationOptions;
        if (startSnapAnimationCallback_(snapAnimationOptions)) {
            // Provider strategies only park here; without a flush the parked chain never runs
            // and this early return (which skips CheckStopFlingInTouchUp) would leave the
            // fling-stop lifecycle events permanently undelivered. The started chain's own
            // completion delivers them; for built-in align the flush is a no-op.
            FlushPendingItemSnapKickoff();
            isTouchStopAnimation_ = false;
            return;
        }
    }
    CheckStopFlingInTouchUp();
}

void Scrollable::HandleTouchCancel()
{
    if (isDragging_) {
        return;
    }
    isTouching_ = false;
    // Gesture takeover: see HandleTouchDown — cancel the in-flight item snap chain.
    CancelItemSnapChain();
    ACE_SCOPED_TRACE("HandleTouchCancel, id:%d, tag:%s", nodeId_, nodeTag_.c_str());
    if (outBoundaryCallback_ && outBoundaryCallback_(false)) {
        if (state_ != AnimationState::SPRING && scrollOverCallback_) {
            ProcessScrollOverCallback(0.0);
        }
    }
}

bool Scrollable::IsAnimationNotRunning() const
{
    return !isTouching_ && state_ == AnimationState::IDLE;
}

bool Scrollable::Idle() const
{
    return !isTouching_ && state_ == AnimationState::IDLE && !nestedScrolling_ && !isCrownDragging_;
}

bool Scrollable::IsStopped() const
{
    return state_ == AnimationState::IDLE;
}

bool Scrollable::IsAllAnimationStopped() const
{
    return !isTouching_ && state_ == AnimationState::IDLE && !nestedScrolling_
        && !isCrownDragging_ && springAnimationCount_ == 0 && updateSnapAnimationCount_ == 0;
}

bool Scrollable::IsSpringStopped() const
{
    return state_ != AnimationState::SPRING;
}

bool Scrollable::IsSnapStopped() const
{
    return state_ != AnimationState::SNAP;
}

void Scrollable::CancelItemSnapChain()
{
    // Kill any pending two-stage handoff (the posted task re-checks the generation) and drop a
    // pending fling-end settle. The axis steering state (snapDirection_/axisSnapDistance_) is
    // only reset when an item-snap-driven animation is in flight: legacy List/Scroll wheel
    // chaining relies on that state surviving across wheel ticks by design.
    itemSnapGeneration_++;
    // Invalidate the old friction completion before stopping/replacing its property animation.
    // Its generation check is opt-in, so legacy NONE/List completion paths retain their contract.
    ++frictionGeneration_;
    if (itemSnapTakeoverPending_) {
        itemSnapTakeoverPending_ = false;
        if (state_ == AnimationState::TRANSITION) {
            state_ = AnimationState::IDLE;
        }
    }
    needItemSnapSettle_ = false;
    itemSnapTakeoverDistance_ = 0.0f;
    itemSnapFlingRate_ = 0.0f;
    itemSnapMonotonicSpring_ = false;
    itemSnapTrackingCallback_ = nullptr;
    itemSnapTargetReady_ = false;
    itemSnapLookaheadDistance_ = 0.0f;
    pendingItemSnapKickoff_ = nullptr;
    if (itemSnapChainActive_) {
        itemSnapChainActive_ = false;
        snapDirection_ = SnapDirection::NONE;
        axisSnapDistance_ = 0.f;
    }
}

void Scrollable::StopScrollable()
{
    // Programmatic stop (ScrollBy / AnimateTo / StopAnimate chain): the item snap chain must die
    // with the animation, otherwise the posted handoff task would pass its generation check and
    // drag the new scroll away from its target. The settle-stop path inside
    // ProcessListSnapMotion does NOT come through here and stays unaffected.
    CancelItemSnapChain();
    if (state_ == AnimationState::FRICTION) {
        StopFrictionAnimation();
    }
    if (state_ == AnimationState::SPRING) {
        StopSpringAnimation();
    }
    if (state_ == AnimationState::SNAP) {
        StopSnapAnimation();
    }
}

void Scrollable::HandleScrollEnd(const std::optional<float>& velocity)
{
    // priority:
    //  1. onScrollEndRec_ (would internally call onScrollEnd)
    //  2. scrollEndCallback_
    if (onScrollEndRec_) {
        onScrollEndRec_(velocity);
        return;
    }
    if (scrollEndCallback_) {
        scrollEndCallback_();
    }
}

void Scrollable::HandleExtDragUpdate()
{
    HandleExtScroll();
}

void Scrollable::HandleDragStart(const OHOS::Ace::GestureEvent& info)
{
    if (info.GetSourceTool() == SourceTool::TOUCHPAD) {
        HandleTouchDown();
    }
#ifdef SUPPORT_DIGITAL_CROWN
    crownTask_.Cancel();
    if (info.GetSourceDevice() != SourceType::CROWN) {
        isCrownEventDragging_ = false;
    }
    if (isCrownEventDragging_) {
        HandleTouchDown(true);
    }
#endif
    currentVelocity_ = info.GetMainVelocity();
    auto isAxisEvent = IsMouseWheelScroll(info);
    if (!isAxisEvent) {
        ReportToDragFRCScene(currentVelocity_, NG::SceneStatus::START);
    }
    if (continuousDragStatus_) {
        IncreaseContinueDragCount();
        task_.Cancel();
    }
    SetDragStartPosition(GetMainOffset(Offset(info.GetGlobalPoint().GetX(), info.GetGlobalPoint().GetY())));
    const double dragPositionInMainAxis =
        axis_ == Axis::VERTICAL ? info.GetGlobalLocation().GetY() : info.GetGlobalLocation().GetX();
    TAG_LOGI(AceLogTag::ACE_SCROLLABLE, "Scroll drag start, id:%{public}d, tag:%{public}s", nodeId_, nodeTag_.c_str());
    skipRestartSpring_ = false; // reset flags. Extract method if more flags need to be reset
#ifdef OHOS_PLATFORM
    // Increase the cpu frequency when sliding start.
    auto currentTime = GetSysTimestamp();
    auto increaseCpuTime = currentTime - startIncreaseTime_;
    if (!moved_ || increaseCpuTime >= INCREASE_CPU_TIME_ONCE) {
        startIncreaseTime_ = currentTime;
        if (FrameReport::GetInstance().GetEnable()) {
            FrameReport::GetInstance().BeginListFling();
        }
    }
#endif
#ifndef CROSS_PLATFORM
    JankFrameReport::GetInstance().SetFrameJankFlag(JANK_RUNNING_SCROLL);
#endif
    ACE_SCOPED_TRACE("HandleDragStart, inputEventType:%d, sourceTool:%d, IsMouseWheelScroll:%u, "
                     "IsAxisAnimationRunning:%u, IsSnapAnimationRunning:%u, id:%d, tag:%s",
        info.GetInputEventType(), info.GetSourceTool(), isAxisEvent, IsAxisAnimationRunning(), IsSnapAnimationRunning(),
        nodeId_, nodeTag_.c_str());
    if (onWillStartDraggingCallback_) {
        onWillStartDraggingCallback_();
    }
    if (isAxisEvent && !CanStayOverScroll()) {
        if (!IsAxisAnimationRunning() && !IsSnapAnimationRunning()) {
            axisSnapDistance_ = currentPos_;
            snapDirection_ = SnapDirection::NONE;
        }
        return;
    } else if (IsAxisAnimationRunning()) {
        StopAxisAnimation();
    }
    isDragging_ = true;
    if (onScrollStartRec_) {
        onScrollStartRec_(static_cast<float>(dragPositionInMainAxis));
    }
    ResetDragUpdateDelta();
}

void Scrollable::HandleExtScroll()
{
    if (handleExtScrollCallback_ != nullptr) {
        // call NestableScrollContainer::HandleExtScroll
        handleExtScrollCallback_();
    }
}

ScrollResult Scrollable::HandleScroll(double offset, int32_t source, NestedState state)
{
    if (!handleScrollCallback_) {
        ExecuteScrollBegin(offset);
        canOverScroll_ = false;
        moved_ = UpdateScrollPosition(offset, source);
        return { 0, false };
    }
    // call NestableScrollContainer::HandleScroll
    return handleScrollCallback_(static_cast<float>(offset), source, state);
}

void Scrollable::HandleDragUpdate(const GestureEvent& info)
{
    // Keep the update delta until the layout pass consumes this frame's drag.
    dragUpdateDelta_ = dragUpdateDelta_.value_or(0.0) + info.GetMainDelta();
    currentVelocity_ = info.GetMainVelocity();
    ReportToDragFRCScene(currentVelocity_, NG::SceneStatus::RUNNING);
    if (!NearZero(info.GetMainVelocity()) && dragCount_ >= FIRST_THRESHOLD) {
        if (Negative(lastGestureVelocity_ / info.GetMainVelocity())) {
            ResetContinueDragCount();
        }
    }
    if (state_ != AnimationState::IDLE) {
        // If animation still runs, first stop it. A gesture taking over also cancels any
        // pending two-stage item snap handoff (the posted task checks the generation).
        isDragUpdateStop_ = true;
        CancelItemSnapChain();
        StopFrictionAnimation();
        StopSpringAnimation();
        if (!(IsMouseWheelScroll(info) && GetSnapType() != SnapType::NONE_SNAP)) {
            StopSnapAnimation();
            currentPos_ = 0.0;
        }
    }
#ifdef OHOS_PLATFORM
    // Handle the case where you keep sliding past limit time(4s).
    auto currentTime = GetSysTimestamp();
    auto increaseCpuTime = currentTime - startIncreaseTime_;
    if (increaseCpuTime >= INCREASE_CPU_TIME_ONCE) {
        startIncreaseTime_ = currentTime;
        if (FrameReport::GetInstance().GetEnable()) {
            FrameReport::GetInstance().BeginListFling();
        }
    }
#endif
    ProcessDragUpdateScroll(info);
}

void Scrollable::ProcessDragUpdateScroll(const GestureEvent& info)
{
    lastMainDelta_ = info.GetMainDelta();
    auto mainDelta = lastMainDelta_ + prevRemainDelta_;
    auto prevMainDelta = isReverseCallback_ && isReverseCallback_() ? -mainDelta : mainDelta;
    mainDelta = Round(prevMainDelta);
    prevRemainDelta_ = prevMainDelta - mainDelta;
#ifndef CROSS_PLATFORM
    JankFrameReport::GetInstance().RecordFrameUpdate();
#endif
    auto source = SCROLL_FROM_UPDATE;
    auto isAxisEvent = IsMouseWheelScroll(info);
    if (isAxisEvent) {
        source = SCROLL_FROM_AXIS;
    }
#ifdef SUPPORT_DIGITAL_CROWN
    if (isCrownEventDragging_) {
        source = SCROLL_FROM_CROWN;
    }
#endif
    ACE_SCOPED_TRACE(
        "HandleDragUpdate, mainDelta:%f, source:%d, id:%d, tag:%s", mainDelta, source, nodeId_, nodeTag_.c_str());
    if (isAxisEvent) {
        CHECK_EQUAL_VOID(CanStayOverScroll(), true);
        ProcessAxisUpdateEvent(mainDelta);
        return;
    }
    HandleScroll(mainDelta, source, NestedState::GESTURE);
}

void Scrollable::ProcessAxisUpdateEvent(float mainDelta, bool fromScrollBar)
{
    auto context = context_.Upgrade();
    CHECK_NULL_VOID(context);
    auto currentVsyncTime = context->GetVsyncTime();
    if (HandleAxisSnapUpdate(mainDelta, fromScrollBar)) {
        return;
    }
    lastAxisVsyncTime_ = currentVsyncTime;
    InitAxisAnimator();
    ACE_SCOPED_TRACE(
        "ProcessAxisUpdateEvent onAxis, IsAxisAnimationRunning:%u, mainDelta:%f, currentPos_:%f, id:%d, tag:%s",
        IsAxisAnimationRunning(), mainDelta, currentPos_, nodeId_, nodeTag_.c_str());
    axisAnimator_->OnAxis(mainDelta, currentPos_);
}

void Scrollable::LayoutDirectionEst(double gestureVelocity, double velocityScale, bool isScrollFromTouchPad)
{
    auto defaultVelocityScale = isSlow_ ? SLOW_VELOCITY_SCALE : velocityScale;
    double ret = SystemProperties::GetSrollableVelocityScale();
    velocityScale = !NearZero(ret) ? ret : defaultVelocityScale;
    velocityScale = isScrollFromTouchPad ? velocityScale * touchPadVelocityScaleRate_ : velocityScale;
    double gain = GetGain(GetDragOffset());
    bool isReverse = isReverseCallback_ && isReverseCallback_();
    if (GreatNotEqualCustomPrecision(gain, 1.0f, 0.01f)) {
        if ((isReverse && gestureVelocity < 0) || (!isReverse && gestureVelocity > 0)) {
            auto node = weakHost_.Upgrade();
            bool isBackToTop = backToTopCallback_ && backToTopCallback_();
            if (node && isBackToTop) {
                auto nodeIdStr = std::to_string(static_cast<uint64_t>(node->GetId()));
                auto nodeStr = node->GetTag() + nodeIdStr;
                TAG_LOGI(AceLogTag::ACE_SCROLLABLE,
                    "LayoutDirectionEst %{public}s, gain: %{public}f, velocity: %{public}f, reverse: %{public}d",
                    nodeStr.c_str(), gain, gestureVelocity, isReverse);
                OsContent::CallSendAction("ace_action", "scroll_gain", "{ \"nodeId\":" + nodeIdStr +
                    ", \"compId\": \"" + node->GetInspectorIdValue("") + "\"}");
            }
        }
    }
    if (isReverse) {
        currentVelocity_ = -gestureVelocity * velocityScale * gain;
    } else {
        currentVelocity_ = gestureVelocity * velocityScale * gain;
    }
    // Apply max fling velocity limit, it must be calculated after all fling velocity gain.
    currentVelocity_ = std::clamp(currentVelocity_, -maxFlingVelocity_ + slipFactor_, maxFlingVelocity_ - slipFactor_);
    slidInfo_.gestureVelocity = gestureVelocity;
    slidInfo_.velocityScale = velocityScale;
    slidInfo_.gain = gain;
    slidInfo_.maxFlingVelocity = maxFlingVelocity_;
    slidInfo_.slipFactor = slipFactor_;
}

void Scrollable::HandleDragEnd(const GestureEvent& info, bool isFromPanEnd)
{
    TAG_LOGI(AceLogTag::ACE_SCROLLABLE, "Scroll drag end, velocity is %{public}f id:%{public}d, tag:%{public}s, "
        "dragCnt:%{public}d", info.GetMainVelocity(), nodeId_, nodeTag_.c_str(), dragCount_);
    auto isAxisEvent = IsMouseWheelScroll(info);
    if (isAxisEvent) {
        ProcessAxisEndEvent();
        return;
    }
    if (CanStayOverScroll()) {
        SetCanStayOverScroll(false);
    }
    // avoid no render frame when drag end
    if (!isFromPanEnd) {
        // For a frame containing only an up event with zero offset, replay the last delta to trigger rendering.
        // Avoid synthesizing a second update when this frame already had one.
        if ((!dragUpdateDelta_.has_value() || NearZero(dragUpdateDelta_.value())) && NearZero(info.GetMainDelta())) {
            auto tempInfo = info;
            tempInfo.SetMainDelta(lastMainDelta_);
            HandleDragUpdate(tempInfo);
        } else {
            HandleDragUpdate(info);
        }
    }
    if (onWillStopDraggingCallback_) {
        onWillStopDraggingCallback_(info.GetMainVelocity());
    }
    ReportToDragFRCScene(info.GetMainVelocity(), NG::SceneStatus::END);
    auto isWillFling = ResolveDragEndFling(info, isAxisEvent);
    if (!isWillFling.has_value()) {
        return;
    }
    DispatchDragEndLifecycle(isWillFling.value());
    FinishDragEnd(info);
}

// Feat-20260806915268 cmetrics helper: drag-end gesture-state preamble plus the
// snap/overscroll/fling three-way dispatch, extracted verbatim from HandleDragEnd
// (zero behavior change). Returns nullopt where the original returned early.
std::optional<bool> Scrollable::ResolveDragEndFling(const GestureEvent& info, bool isAxisEvent)
{
    bool isScrollFromTouchPad = info.GetSourceTool() == SourceTool::TOUCHPAD;
    isDragUpdateStop_ = false;
    scrollPause_ = false;
    lastGestureVelocity_ = GetPanDirection() == Axis::NONE ? 0.0 : info.GetMainVelocity();
    isSlow_ = LessNotEqual(std::abs(lastGestureVelocity_), SLOW_FRICTION_THRESHOLD);
    SetDragEndPosition(GetMainOffset(Offset(info.GetGlobalPoint().GetX(), info.GetGlobalPoint().GetY())));
    lastPos_ = GetDragOffset();
#ifndef CROSS_PLATFORM
    JankFrameReport::GetInstance().ClearFrameJankFlag(JANK_RUNNING_SCROLL);
#endif
    double mainPosition = Round(GetMainOffset(Offset(info.GetGlobalPoint().GetX(), info.GetGlobalPoint().GetY())));
    bool isWillFling = false;
    if (!moved_ || isAxisEvent) {
        LayoutDirectionEst(lastGestureVelocity_, flingVelocityScale_, isScrollFromTouchPad);
        ResetContinueDragCount();
        if (GetSnapType() == SnapType::SCROLL_SNAP) {
            currentPos_ = mainPosition;
            SnapAnimationOptions snapAnimationOptions = {
                .animationVelocity = currentVelocity_,
                .source = isAxisEvent ? SCROLL_FROM_AXIS : SCROLL_FROM_NONE };
            if (startSnapAnimationCallback_ && startSnapAnimationCallback_(snapAnimationOptions)) {
                isTouching_ = false;
                return std::nullopt;
            }
        }
        HandleScrollEnd(currentVelocity_);
        currentVelocity_ = 0.f;
#ifdef OHOS_PLATFORM
        if (FrameReport::GetInstance().GetEnable()) {
            FrameReport::GetInstance().EndListFling();
        }
#endif
    } else if (canOverScroll_) {
        LayoutDirectionEst(lastGestureVelocity_, springVelocityScale_, isScrollFromTouchPad);
        CalcOverScrollVelocity();
        ResetContinueDragCount();
        isWillFling = HandleOverScroll(currentVelocity_);
    } else {
        LayoutDirectionEst(lastGestureVelocity_, flingVelocityScale_, isScrollFromTouchPad);
        // Feat-20260806915268: the provider kickoff parked by the snap callback must run only after the
        // gesture lifecycle events below (architect alignment: calculateApproachOffset runs
        // after onWillStartFling), so app-side state changes there are visible to it.
        SetDeferItemSnapKickoff(true);
        isWillFling = StartScrollAnimation(mainPosition, currentVelocity_, isScrollFromTouchPad);
        SetDeferItemSnapKickoff(false);
    }
    return isWillFling;
}

// Feat-20260806915268 cmetrics helper: the drag-end lifecycle tail of HandleDragEnd,
// extracted verbatim (zero behavior change).
void Scrollable::DispatchDragEndLifecycle(bool isWillFling)
{
    if (onDidStopDraggingCallback_) {
        onDidStopDraggingCallback_(isWillFling);
    }
    if (onWillStartFlingCallback_ && isWillFling && !isUserFling_) {
        onWillStartFlingCallback_();
    }
    if (onDidStopFlingCallback_ && isUserFling_ && !isWillFling) {
        onDidStopFlingCallback_();
    }
    isUserFling_ = isWillFling;
    FlushPendingItemSnapKickoff();
}

// Feat-20260806915268 cmetrics helper: the trace/report tail of HandleDragEnd,
// extracted verbatim (zero behavior change).
void Scrollable::FinishDragEnd(const GestureEvent& info)
{
    ACE_SCOPED_TRACE(
        "HandleDragEnd, mainPosition:%f, getureDelta:%lf, gestureVelocity:%f, currentVelocity:%f, moved_:%u "
        "canOverScroll_:%u, id:%d, tag:%s",
        Round(GetMainOffset(Offset(info.GetGlobalPoint().GetX(), info.GetGlobalPoint().GetY()))), info.GetMainDelta(),
        lastGestureVelocity_, currentVelocity_, moved_, canOverScroll_, nodeId_, nodeTag_.c_str());
    SetDelayedTask();
    if (dragEndCallback_) {
        dragEndCallback_();
    }
    if (info.GetSourceDevice() != SourceType::CROWN) {
        isTouching_ = false;
    }
    SetDragStartPosition(0.0);
    SetDragEndPosition(0.0);
}

void Scrollable::ProcessAxisScroll(float mainDelta)
{
    TAG_LOGD(AceLogTag::ACE_SCROLLABLE,
        "ProcessAxisScroll mainDelta:%{public}f, state:%{public}d, canStay:%{public}d, axisAnim:%{public}d, "
        "snapAnim:%{public}d, snapType:%{public}d, id:%{public}d",
        mainDelta, static_cast<int32_t>(state_), CanStayOverScroll(), IsAxisAnimationRunning(),
        IsSnapAnimationRunning(), static_cast<int32_t>(GetSnapType()), nodeId_);
    if (!CanStayOverScroll()) {
        if (!IsAxisAnimationRunning() && !IsSnapAnimationRunning()) {
            axisSnapDistance_ = currentPos_;
            snapDirection_ = SnapDirection::NONE;
        }
    } else if (IsAxisAnimationRunning()) {
        StopAxisAnimation();
    }
    if (state_ != AnimationState::IDLE) {
        isDragUpdateStop_ = true;
        StopFrictionAnimation();
        StopSpringAnimation();
        if (GetSnapType() == SnapType::NONE_SNAP) {
            StopSnapAnimation();
            currentPos_ = 0.0;
        }
    }
    lastMainDelta_ = mainDelta;
    mainDelta = lastMainDelta_ + prevRemainDelta_;
    auto prevMainDelta = isReverseCallback_ && isReverseCallback_() ? -mainDelta : mainDelta;
    mainDelta = Round(prevMainDelta);
    prevRemainDelta_ = prevMainDelta - mainDelta;

    if (CanStayOverScroll()) {
        HandleOverScroll(0);
        SetCanStayOverScroll(false);
    } else {
        ProcessAxisUpdateEvent(mainDelta);
    }
}

void Scrollable::ProcessAxisEndEvent()
{
    scrollPause_ = false;
    isTouching_ = false;
    isDragUpdateStop_ = false;
#ifndef CROSS_PLATFORM
    JankFrameReport::GetInstance().ClearFrameJankFlag(JANK_RUNNING_SCROLL);
#endif
    if (CanStayOverScroll()) {
        HandleOverScroll(0);
        SetCanStayOverScroll(false);
    }
    if (isUserFling_) {
        isUserFling_ = false;
        if (onDidStopFlingCallback_) {
            onDidStopFlingCallback_();
        }
    }
    if (onDidStopDraggingCallback_) {
        onDidStopDraggingCallback_(false);
    }
}

void Scrollable::ReportToDragFRCScene(double velocity, NG::SceneStatus sceneStatus)
{
    CHECK_NULL_VOID(dragFRCSceneCallback_);
    dragFRCSceneCallback_(velocity, sceneStatus);
}

void Scrollable::CalcOverScrollVelocity()
{
    auto gamma = 0.0f;
    if (overScrollOffsetCallback_ && continuousSlidingCallback_) {
        gamma = overScrollOffsetCallback_() / continuousSlidingCallback_();
    }
    gamma = GreatOrEqual(gamma, 1.0) ? 1.0f : gamma;
    currentVelocity_ = currentVelocity_ * exp(-ratio_ * gamma);
}

// Feat-20260806915268 cmetrics helper: the friction-parameter preamble of
// StartScrollAnimation, extracted verbatim (zero behavior change).
float Scrollable::PrepareFlingParams(float mainPosition, float& correctVelocity, bool isScrollFromTouchPad)
{
    double frictionTmp = friction_;
    if (NearEqual(friction_, -1.0)) {
        auto defaultFriction = isSlow_ ? SLOW_FRICTION : defaultFriction_;
        double ret = SystemProperties::GetSrollableFriction();
        frictionTmp = !NearZero(ret) ? ret : defaultFriction;
    }
    float friction = frictionTmp;
    slidInfo_.friction = friction;
    EventReport::ReportPageSlidInfo(slidInfo_);
    initVelocity_ = correctVelocity;
    finalPosition_ = mainPosition + correctVelocity / (friction * -FRICTION_SCALE);
    if (fixScrollParamCallback_) {
        fixScrollParamCallback_(mainPosition, initVelocity_, finalPosition_);
        correctVelocity = initVelocity_;
        currentVelocity_ = correctVelocity;
    }
    currentPos_ = mainPosition;
    return friction;
}

// Feat-20260806915268 cmetrics helper: the slow-release tail of StartScrollAnimation
// (below the fling threshold), extracted verbatim (zero behavior change).
bool Scrollable::HandleSlowReleaseOrFling(float mainPosition, float friction, float correctVelocity)
{
    float threshold = START_FRICTION_VELOCITY_THRESHOLD;
#ifdef SUPPORT_DIGITAL_CROWN
    threshold = isCrownEventDragging_ ? CROWN_START_FRICTION_VELOCITY_THRESHOLD : threshold;
#endif
    if (NearZero(correctVelocity, threshold)) {
        // Feat-20260806915268 item snap: a release too slow to start the friction fling never reaches
        // ProcessScrollMotionStop, so the pending settle must run here — otherwise a slow
        // release between anchors would rest unaligned with neither snap nor fling started.
        if (needItemSnapSettle_ && startSnapAnimationCallback_) {
            needItemSnapSettle_ = false;
            SnapAnimationOptions settleOptions = {
                .snapDelta = GetFinalPosition() - currentPos_,
                .animationVelocity = correctVelocity,
                .fromItemSnapFlingStop = true,
            };
            if (startSnapAnimationCallback_(settleOptions)) {
                return true;
            }
        }
        HandleScrollEnd(correctVelocity);
        currentVelocity_ = 0.0;
#ifdef OHOS_PLATFORM
        if (FrameReport::GetInstance().GetEnable()) {
            FrameReport::GetInstance().EndListFling();
        }
#endif
        return false;
    }
    TriggerFrictionAnimation(mainPosition, friction, correctVelocity);
    if (needItemSnapSettle_ && GreatNotEqual(itemSnapTakeoverDistance_, 0.0f)) {
        currentVelocity_ = initVelocity_;
        TryItemSnapTakeover();
    }
    return true;
}

bool Scrollable::StartScrollAnimation(float mainPosition, float correctVelocity, bool isScrollFromTouchPad)
{
    if (state_ == AnimationState::SPRING) {
        StopSpringAnimation();
    }
    if (!frictionOffsetProperty_) {
        GetFrictionProperty();
    }
    // Restarting a friction animation (e.g. nested-scroll velocity handoff): a pending item
    // snap handoff must not survive into the new fling.
    CancelItemSnapChain();
    StopSnapController();
    TAG_LOGD(AceLogTag::ACE_SCROLLABLE, "The position of scroll motion is %{public}f, velocity is %{public}f",
        mainPosition, correctVelocity);
    float friction = PrepareFlingParams(mainPosition, correctVelocity, isScrollFromTouchPad);
    SnapAnimationOptions snapAnimationOptions = {
        .snapDelta = GetFinalPosition() - mainPosition,
        .animationVelocity = correctVelocity,
        .dragDistance = GetDragOffset(),
        .snapDirection = SnapDirection::NONE,
    };
    if (startSnapAnimationCallback_ && startSnapAnimationCallback_(snapAnimationOptions)) {
        if (GetSnapType() == SnapType::LIST_SNAP) {
            currentVelocity_ = 0.0;
        }
        // Non-gesture callers (fling API, nested handoff) have no lifecycle events to wait
        // for: the parked provider kickoff runs right here. HandleDragEnd defers via
        // deferItemSnapKickoff_ and flushes after onWillStartFling instead.
        FlushPendingItemSnapKickoff();
        return true;
    }
    if (fixScrollParamCallback_) {
        fixScrollParamCallback_(mainPosition, initVelocity_, finalPosition_);
        correctVelocity = initVelocity_;
        currentVelocity_ = correctVelocity;
    }
    return HandleSlowReleaseOrFling(mainPosition, friction, correctVelocity);
}

void Scrollable::TriggerFrictionAnimation(float mainPosition, float friction, float correctVelocity)
{
    // change motion param when list item need to be center of screen on watch
    FixScrollMotion(mainPosition, correctVelocity);
    currentVelocity_ = 0.0;
    lastPosition_ = currentPos_;
    frictionVelocity_ = initVelocity_;
    if (itemSnapFlingRate_ > 0.0f) {
        const float rate = std::abs(friction * FRICTION_SCALE);
        // Only use the analytic derivative when the actual friction curve is exponential.
        const float expectedVelocity = rate * (finalPosition_ - mainPosition);
        const float tolerance = std::max(0.01f, std::abs(initVelocity_) * 0.0001f);
        itemSnapFlingRate_ =
            std::isfinite(rate) && std::abs(expectedVelocity - initVelocity_) <= tolerance ? rate : 0.0f;
    }
    frictionOffsetProperty_->Set(mainPosition);
    float response = fabs(2 * ACE_PI / (FRICTION_SCALE * friction));
    auto curve = AceType::MakeRefPtr<ResponsiveSpringMotion>(response, 1.0f, 0.0f);
    AnimationOption option;
    option.SetCurve(curve);
    option.SetDuration(CUSTOM_SPRING_ANIMATION_DURATION);
    option.SetFinishCallbackType(FinishCallbackType::LOGICALLY);
    frictionOffsetProperty_->SetThresholdType(ThresholdType::LAYOUT);
    frictionOffsetProperty_->SetPropertyUnit(PropertyUnit::PIXEL_POSITION);
    ACE_SCOPED_TRACE("Scrollable friction animation start, start:%f, end:%f, vel:%f, id:%d, tag:%s", mainPosition,
        finalPosition_, initVelocity_, nodeId_, nodeTag_.c_str());
    const bool itemSnapFling = needItemSnapSettle_ &&
        (GreatNotEqual(itemSnapTakeoverDistance_, 0.0f) || itemSnapTrackingCallback_);
    const auto generation = ++frictionGeneration_;
    frictionOffsetProperty_->AnimateWithVelocity(
        option, finalPosition_, initVelocity_,
        [weak = AceType::WeakClaim(this), id = Container::CurrentId(), generation, itemSnapFling]() {
            ContainerScope scope(id);
            auto scroll = weak.Upgrade();
            CHECK_NULL_VOID(scroll);
            if (itemSnapFling && scroll->frictionGeneration_ != generation) {
                return;
            }
            scroll->state_ = AnimationState::IDLE;
            ACE_SCOPED_TRACE(
                "Scrollable friction animation finish, id:%d, tag:%s", scroll->nodeId_, scroll->nodeTag_.c_str());
            scroll->ProcessScrollMotionStop(SCROLL_FROM_ANIMATION);
        });
    state_ = AnimationState::FRICTION;
    auto context = context_.Upgrade();
    CHECK_NULL_VOID(context);
    context->RequestFrame();
    lastVsyncTime_ = context->GetVsyncTime();
}

void Scrollable::SetDelayedTask()
{
    SetContinuousDragStatus(true);
    auto context = context_.Upgrade();
    CHECK_NULL_VOID(context);
    auto taskExecutor = SingleTaskExecutor::Make(context->GetTaskExecutor(), TaskExecutor::TaskType::UI);
    task_.Reset([weak = WeakClaim(this)] {
        auto drag = weak.Upgrade();
        if (drag) {
            drag->ResetContinueDragCount();
            drag->SetContinuousDragStatus(false);
        }
    });
    taskExecutor.PostDelayedTask(task_, DRAG_INTERVAL_TIME, "ArkUIScrollDragInterval");
}

void Scrollable::MarkNeedFlushAnimationStartTime()
{
    auto context = context_.Upgrade();
    CHECK_NULL_VOID(context);
    context->MarkNeedFlushAnimationStartTime();
}

double Scrollable::ComputeCap(int dragCount)
{
    if (dragCount < FIRST_THRESHOLD) {
        return 1.0;
    }
    auto cap = ComputeCap(dragCount - 1) + CAP_COEFFICIENT * (dragCount - 1);
    return cap;
}

double Scrollable::GetGain(double delta)
{
    auto cap = 1.0;
    auto gain = 1.0;
    if (std::abs(delta) < MULTI_FLING_DISTANCE) {
        ResetContinueDragCount();
        preGain_ = gain;
        return gain;
    }
    if (!continuousSlidingCallback_) {
        preGain_ = gain;
        return gain;
    }
    auto screenHeight = continuousSlidingCallback_();
    if (delta == 0 || screenHeight == 0) {
        preGain_ = gain;
        return gain;
    }
    if (dragCount_ >= FIRST_THRESHOLD && dragCount_ < SECOND_THRESHOLD) {
        if (Negative(lastPos_ / delta)) {
            ResetContinueDragCount();
            preGain_ = gain;
            return gain;
        }
        cap = CAP_COEFFICIENT * (dragCount_ - 1);
        gain = (LessNotEqual(cap, std::abs(delta) / screenHeight * (dragCount_ - 1)))
                   ? preGain_ + cap
                   : preGain_ + std::abs(delta) / screenHeight * (dragCount_ - 1);
    } else if (dragCount_ >= SECOND_THRESHOLD) {
        if (Negative(lastPos_ / delta)) {
            ResetContinueDragCount();
            preGain_ = gain;
            return gain;
        }
        cap = CAP_FIXED_VALUE;
        gain = (LessNotEqual(cap, preGain_ + std::abs(delta) / screenHeight * (dragCount_ - 1)))
                   ? cap
                   : preGain_ + std::abs(delta) / screenHeight * (dragCount_ - 1);
    }
    preGain_ = gain;
    return gain;
}

void Scrollable::ExecuteScrollBegin(double& mainDelta)
{
    auto context = context_.Upgrade();
    if (!scrollBeginCallback_ || !context) {
        return;
    }

    ScrollInfo scrollInfo;
    if (axis_ == Axis::VERTICAL) {
        scrollInfo = scrollBeginCallback_(0.0_vp, Dimension(mainDelta / context->GetDipScale(), DimensionUnit::VP));
        mainDelta = context->NormalizeToPx(scrollInfo.dy);
    } else if (axis_ == Axis::HORIZONTAL) {
        scrollInfo = scrollBeginCallback_(Dimension(mainDelta / context->GetDipScale(), DimensionUnit::VP), 0.0_vp);
        mainDelta = context->NormalizeToPx(scrollInfo.dx);
    }
}

float Scrollable::GetFrictionVelocityByFinalPosition(
    float final, float position, float friction, float signum, float threshold)
{
    return DEFAULT_THRESHOLD * threshold * signum - (final - position) * friction;
}

void Scrollable::FixScrollMotion(float position, float initVelocity)
{
#ifdef WEARABLE_PRODUCT
    float signum = 0.0;
    if (!NearZero(initVelocity)) {
        signum = GreatNotEqual(initVelocity, 0.0) ? 1.0 : -1.0;
    }
    if (frictionOffsetProperty_ && needCenterFix_ && watchFixCallback_) {
        float finalPosition = watchFixCallback_(GetFinalPosition(), position);
        if (!NearEqual(finalPosition, GetFinalPosition(), DISTANCE_EPSILON)) {
            float friction = friction_;
            float velocity = GetFrictionVelocityByFinalPosition(finalPosition, position, friction, signum);

            // fix again when velocity is less than velocity threshold
            if (!NearEqual(finalPosition, GetFinalPosition(), DISTANCE_EPSILON)) {
                velocity = GetFrictionVelocityByFinalPosition(finalPosition, position, friction, signum, 0.0f);
            }
            initVelocity_ = velocity;
            finalPosition_ = mainPosition + initVelocity_ / (friction * -FRICTION_SCALE);
        }
    }
#endif
}

void Scrollable::StartListSnapAnimation(float predictSnapOffset, float scrollSnapVelocity, bool fromScrollBar)
{
    endPos_ = currentPos_ + predictSnapOffset;
    finalPosition_ = endPos_;
    snapAnimationFromScrollBar_ = fromScrollBar;
    AnimationOption option;
    option.SetDuration(CUSTOM_SPRING_ANIMATION_DURATION);
    auto curve = AceType::MakeRefPtr<ResponsiveSpringMotion>(
        listSnapSpeed_ == ScrollSnapAnimationSpeed::NORMAL ? SNAP_SPRING_RESPONSE : SLOW_SPRING_RESPONSE,
        listSnapSpeed_ == ScrollSnapAnimationSpeed::NORMAL ? SNAP_SPRING_DAMP : SLOW_SPRING_DAMP, 0.0f);
    option.SetCurve(curve);
    if (!snapOffsetProperty_) {
        GetSnapProperty();
    }
    snapOffsetProperty_->Set(currentPos_);
    snapOffsetProperty_->SetPropertyUnit(PropertyUnit::PIXEL_POSITION);
    ACE_SCOPED_TRACE("List snap animation start, start:%f, end:%f, vel:%f, id:%d", currentPos_, endPos_,
        scrollSnapVelocity, nodeId_);
    updateSnapAnimationCount_++;
    snapOffsetProperty_->AnimateWithVelocity(
        option, endPos_, scrollSnapVelocity, [weak = AceType::WeakClaim(this), id = Container::CurrentId()]() {
            ContainerScope scope(id);
            auto scroll = weak.Upgrade();
            CHECK_NULL_VOID(scroll);
            ACE_SCOPED_TRACE("List snap animation finish, id:%d", scroll->nodeId_);
            scroll->updateSnapAnimationCount_--;
            if (scroll->updateSnapAnimationCount_ == 0) {
                scroll->state_ = AnimationState::IDLE;
                if (scroll->isUserFling_) {
                    if (scroll->onDidStopFlingCallback_) {
                        scroll->onDidStopFlingCallback_();
                    }
                    scroll->isUserFling_ = false;
                }
                scroll->axisSnapDistance_ = 0.f;
                scroll->ProcessScrollSnapStop();
            }
            scroll->snapAnimationFromScrollBar_ = false;
        });
    state_ = AnimationState::SNAP;
    auto context = context_.Upgrade();
    CHECK_NULL_VOID(context);
    lastVsyncTime_ = context->GetVsyncTime();
    MarkNeedFlushAnimationStartTime();
    context->RequestFrame();
}

void Scrollable::StartScrollSnapAnimation(
    float scrollSnapDelta, float scrollSnapVelocity, bool fromScrollBar, int32_t source, bool useSlowSpring)
{
    TAG_LOGD(AceLogTag::ACE_SCROLLABLE, "The snap delta of scroll motion is %{public}f, "
        "The snap velocity of scroll motion is %{public}f", scrollSnapDelta, scrollSnapVelocity);
    if (NearZero(scrollSnapDelta)) {
        return;
    }
    endPos_ = currentPos_ + scrollSnapDelta;
    finalPosition_ = endPos_;
    snapAnimationFromScrollBar_ = fromScrollBar;
    snapAnimationSource_ = source;
    ACE_SCOPED_TRACE("Scroll snap animation start, start:%f, end:%f, vel:%f, id:%d", currentPos_, endPos_,
        scrollSnapVelocity, nodeId_);
    AnimationOption option;
    option.SetDuration(CUSTOM_SPRING_ANIMATION_DURATION);
    option.SetCurve(CreateScrollSnapSpringCurve(scrollSnapDelta, useSlowSpring));
    // Tracked item snaps share List's slow curve and reject reverse/overshooting samples.
    itemSnapMonotonicSpring_ = useSlowSpring && itemSnapFlingRate_ > 0.0f;
    if (!snapOffsetProperty_) {
        GetSnapProperty();
    }
    snapOffsetProperty_->Set(currentPos_);
    snapOffsetProperty_->SetPropertyUnit(PropertyUnit::PIXEL_POSITION);
    updateSnapAnimationCount_++;
    snapOffsetProperty_->AnimateWithVelocity(
        option, endPos_, scrollSnapVelocity, [weak = AceType::WeakClaim(this), id = Container::CurrentId(), source]() {
            ContainerScope scope(id);
            auto scroll = weak.Upgrade();
            CHECK_NULL_VOID(scroll);
            scroll->updateSnapAnimationCount_--;
            if (scroll->updateSnapAnimationCount_ == 0) {
                scroll->state_ = AnimationState::IDLE;
                scroll->nextStep_.reset();
                scroll->axisSnapDistance_ = 0.f;
                scroll->snapDirection_ = SnapDirection::NONE;
                ACE_SCOPED_TRACE("Scroll snap animation finish, id:%d", scroll->nodeId_);
                scroll->ProcessScrollMotionStop(source);
            }
        });
    state_ = AnimationState::SNAP;
    auto context = context_.Upgrade();
    CHECK_NULL_VOID(context);
    lastVsyncTime_ = context->GetVsyncTime();
    MarkNeedFlushAnimationStartTime();
    context->RequestFrame();
}

void Scrollable::UpdateScrollSnapStartOffset(double offset)
{
    UpdateScrollSnapEndWithOffset(offset);
}

void Scrollable::ProcessListSnapMotion(double position)
{
    TAG_LOGD(AceLogTag::ACE_SCROLLABLE, "Current Pos is %{public}f, position is %{public}f", currentPos_, position);
    currentVelocity_ = snapVelocity_;
    auto source = snapAnimationFromScrollBar_ ? SCROLL_FROM_BAR_FLING : SCROLL_FROM_ANIMATION;
    if (NearEqual(currentPos_, position)) {
        UpdateScrollPosition(0.0, source);
    } else {
        auto mainDelta = position - currentPos_;
        HandleScroll(mainDelta, source, NestedState::GESTURE);
        if (!moved_ && state_ == AnimationState::SNAP) {
            StopSnapAnimation();
        }
    }
    if (LessOrEqual(std::abs(currentPos_ - position), 1)) {
        // trace stop at OnScrollStop
        AceAsyncTraceBeginCommercial(
            nodeId_, (TRAILING_ANIMATION + std::to_string(nodeId_) + std::string(" ") + nodeTag_).c_str());
    }
    currentPos_ = position;
    if (canOverScroll_ && state_ == AnimationState::SNAP) {
        if (!snapAnimationFromScrollBar_) {
            scrollPause_ = true;
            skipRestartSpring_ = true;
            MarkNeedFlushAnimationStartTime();
        }
        StopSnapAnimation();
    }
}

void Scrollable::ProcessScrollSnapStop()
{
    if (scrollPause_) {
        scrollPause_ = false;
        HandleOverScroll(currentVelocity_);
    } else {
        OnAnimateStop();
    }
}

void Scrollable::OnAnimateStop()
{
    HandleScrollEnd(std::nullopt);
    currentVelocity_ = 0.0;
    if (isTouching_ || isDragUpdateStop_) {
        return;
    }
    moved_ = false;
#ifdef OHOS_PLATFORM
    if (FrameReport::GetInstance().GetEnable()) {
        FrameReport::GetInstance().EndListFling();
    }
#endif
#if !defined(PREVIEW)
    LayoutInspector::SupportInspector();
#endif
}

void Scrollable::StartSpringMotion(
    double mainPosition, double mainVelocity, const ExtentPair& extent, const ExtentPair& initExtent)
{
    TAG_LOGD(AceLogTag::ACE_SCROLLABLE,
        "position is %{public}f, mainVelocity is %{public}f, minExtent is "
        "%{public}f, maxExtent is %{public}f, initMinExtent is %{public}f, initMaxExtent is %{public}f",
        mainPosition, mainVelocity, extent.Leading(), extent.Trailing(), initExtent.Leading(), initExtent.Trailing());
    if (state_ == AnimationState::SPRING || (skipRestartSpring_ && NearEqual(mainVelocity, 0.0f, 0.001f))) {
        OnAnimateStop();
        return;
    }
    currentPos_ = mainPosition;
    if (mainPosition > initExtent.Trailing() || NearEqual(mainPosition, initExtent.Trailing(), 0.01f)) {
        finalPosition_ = extent.Trailing();
    } else if (mainPosition < initExtent.Leading() || NearEqual(mainPosition, initExtent.Leading(), 0.01f)) {
        finalPosition_ = extent.Leading();
    } else {
        OnAnimateStop();
        EventReport::ReportScrollableErrorEvent("Scrollable", ScrollableErrorType::START_SPRING_MOTION_ERROR,
            "Edge spring animation not activated");
        return;
    }

    if (!springOffsetProperty_) {
        GetSpringProperty();
    }
    springAnimationCount_++;
    if (AnimationUtils::IsImplicitAnimationOpen()) {
        AnimationUtils::ExecuteWithoutAnimation([weak = AceType::WeakClaim(this), mainPosition]() {
            auto scrollable = weak.Upgrade();
            CHECK_NULL_VOID(scrollable);
            scrollable->springOffsetProperty_->Set(mainPosition);
        });
    } else {
        springOffsetProperty_->Set(mainPosition);
    }
    AnimationOption option;
    auto curve = AceType::MakeRefPtr<ResponsiveSpringMotion>(springResponse_, SNAP_SPRING_DAMP, 0.0f);
    option.SetCurve(curve);
    option.SetDuration(CUSTOM_SPRING_ANIMATION_DURATION);
    springOffsetProperty_->SetPropertyUnit(PropertyUnit::PIXEL_POSITION);
    ACE_SCOPED_TRACE("Scrollable spring animation start, start:%f, end:%f, vel:%f, id:%d, tag:%s", mainPosition,
        finalPosition_, mainVelocity, nodeId_, nodeTag_.c_str());
    lastVsyncTime_ = static_cast<uint64_t>(GetSysTimestamp());
    springOffsetProperty_->AnimateWithVelocity(
        option, finalPosition_, mainVelocity, [weak = AceType::WeakClaim(this), id = Container::CurrentId()]() {
            ContainerScope scope(id);
            auto scroll = weak.Upgrade();
            CHECK_NULL_VOID(scroll);
            scroll->springAnimationCount_--;
            ACE_SCOPED_TRACE(
                "Scrollable spring animation finish, id:%d, tag:%s", scroll->nodeId_, scroll->nodeTag_.c_str());
            // avoid current animation being interrupted by the prev animation's finish callback
            // and triggering onScrollStop when spring animation turns to friction animation.
            if (scroll->springAnimationCount_ > 0 || scroll->scrollPause_) {
                scroll->scrollPause_ = false;
                return;
            }
            scroll->state_ = AnimationState::IDLE;
            if (scroll->isUserFling_ && !scroll->isTouchStopAnimation_ && !scroll->isDragOuterScrollBarStopAnimation_) {
                if (scroll->onDidStopFlingCallback_) {
                    scroll->onDidStopFlingCallback_();
                }
                scroll->isUserFling_ = false;
            }
            scroll->currentVelocity_ = 0.0;
            scroll->OnAnimateStop();
        });
    state_ = AnimationState::SPRING;
    skipRestartSpring_ = false;
    auto context = context_.Upgrade();
    CHECK_NULL_VOID(context);
    context->RequestFrame();
}

void Scrollable::UpdateSpringMotion(double mainPosition, const ExtentPair& extent, const ExtentPair& initExtent)
{
    TAG_LOGD(AceLogTag::ACE_SCROLLABLE,
        "position is %{public}f, minExtent is "
        "%{public}f, maxExtent is %{public}f, initMinExtent is %{public}f, initMaxExtent is %{public}f",
        mainPosition, extent.Leading(), extent.Trailing(), initExtent.Leading(), initExtent.Trailing());
    if (state_ != AnimationState::SPRING || !springOffsetProperty_) {
        return;
    }
    float finalPosition = 0.0f;
    if (mainPosition > initExtent.Trailing() || NearEqual(mainPosition, initExtent.Trailing())) {
        finalPosition = extent.Trailing();
    } else if (mainPosition < initExtent.Leading() || NearEqual(mainPosition, initExtent.Leading())) {
        finalPosition = extent.Leading();
    } else {
        return;
    }

    finalPosition = finalPosition_ + (finalPosition - mainPosition) - (finalPosition_ - currentPos_);
    if (NearEqual(finalPosition, finalPosition_, SPRING_ACCURACY)) {
        return;
    }
    finalPosition_ = finalPosition;
    springAnimationCount_++;
    AnimationOption option;
    auto curve = AceType::MakeRefPtr<ResponsiveSpringMotion>(SNAP_SPRING_RESPONSE, SNAP_SPRING_DAMP, 0.0f);
    option.SetCurve(curve);
    option.SetDuration(CUSTOM_SPRING_ANIMATION_DURATION);
    springOffsetProperty_->SetPropertyUnit(PropertyUnit::PIXEL_POSITION);
    ACE_SCOPED_TRACE("Scrollable spring animation update, start:%f, end:%f, id:%d, tag:%s", mainPosition,
        finalPosition_, nodeId_, nodeTag_.c_str());
    AnimationUtils::StartAnimation(
        option,
        [weak = AceType::WeakClaim(this)]() {
            auto scroll = weak.Upgrade();
            CHECK_NULL_VOID(scroll);
            scroll->springOffsetProperty_->Set(scroll->finalPosition_);
            scroll->state_ = AnimationState::SPRING;
        },
        [weak = AceType::WeakClaim(this), id = Container::CurrentId()]() {
            ContainerScope scope(id);
            auto scroll = weak.Upgrade();
            CHECK_NULL_VOID(scroll);
            scroll->springAnimationCount_--;
            // avoid current animation being interrupted by the prev animation's finish callback
            if (scroll->springAnimationCount_ > 0) {
                return;
            }
            ACE_SCOPED_TRACE(
                "Scrollable updated spring animation finish, id:%d, tag:%s", scroll->nodeId_, scroll->nodeTag_.c_str());
            scroll->state_ = AnimationState::IDLE;
            if (scroll->isUserFling_) {
                if (scroll->onDidStopFlingCallback_) {
                    scroll->onDidStopFlingCallback_();
                }
                scroll->isUserFling_ = false;
            }
            scroll->currentVelocity_ = 0.0;
            scroll->OnAnimateStop();
        });
    state_ = AnimationState::SPRING;
    skipRestartSpring_ = false;
}

// Feat-20260806915268 item snap: marks that the running friction fling must settle to the nearest
// candidate at early handoff or natural stop (ProcessScrollMotionStop is the fallback).
void Scrollable::SetNeedItemSnapSettle(bool need)
{
    needItemSnapSettle_ = need;
}

// Feat-20260806915268 item snap: marks that an item-snap-driven animation is currently in flight, so
// CancelItemSnapChain knows the axis steering state belongs to an item snap chain (legacy
// List/Scroll wheel chaining keeps its steering state across ticks and must not be reset).
void Scrollable::MarkItemSnapChainActive()
{
    itemSnapChainActive_ = true;
}

// Feat-20260806915268 provider kickoff deferral: the pattern parks the provider-chain start here so
// the gesture lifecycle callbacks (onDidStopDragging / onWillStartFling) run BEFORE
// calculateApproachOffset executes (app-side state mutations in those callbacks are then
// visible to the provider). HandleDragEnd flushes after the events; every other caller of
// StartScrollAnimation (fling API, nested handoff, ...) has no gesture events around it
// and flushes automatically at the StartScrollAnimation exit.
void Scrollable::SetPendingItemSnapKickoff(std::function<void()>&& kickoff)
{
    pendingItemSnapKickoff_ = std::move(kickoff);
}

void Scrollable::FlushPendingItemSnapKickoff()
{
    if (deferItemSnapKickoff_ || !pendingItemSnapKickoff_) {
        return;
    }
    auto kickoff = std::move(pendingItemSnapKickoff_);
    pendingItemSnapKickoff_ = nullptr;
    kickoff();
}

void Scrollable::SetDeferItemSnapKickoff(bool defer)
{
    deferItemSnapKickoff_ = defer;
}

// Shared tail of the friction stop: re-enter the snap callback with the fling-end state so the
// settle targets the candidate nearest the actual landing point. The item-snap variant marks
// the invocation so the entry never re-defers (the residual velocity at the stop callback is
// not guaranteed to have decayed to ~0).
bool Scrollable::SettleSnapOnFrictionStop(bool itemSnapSettle)
{
    SnapAnimationOptions snapAnimationOptions = {
        .snapDelta = GetFinalPosition() - currentPos_,
        .animationVelocity = currentVelocity_,
        .fromItemSnapFlingStop = itemSnapSettle,
    };
    return startSnapAnimationCallback_(snapAnimationOptions);
}

void Scrollable::ProcessScrollMotionStop(int32_t source)
{
    if (needScrollSnapChange_ && startSnapAnimationCallback_ && frictionOffsetProperty_) {
        needScrollSnapChange_ = false;
        // Restores the pre-refactor semantics of the inline CHECK_NULL_VOID(!callback(...))
        // (upstream 9072922b2b7 "fix the bug of snapScroll"): when the legacy snapToSide
        // settle takes over, the stop events are delivered by ITS spring completion —
        // dispatching them here as well would double-fire.
        if (SettleSnapOnFrictionStop(false)) {
            return;
        }
    }
    // Feat-20260806915268 item snap (built-in align): the release deferred to the friction fling; now that
    // it naturally stopped, settle to the candidate nearest the landing point.
    if (needItemSnapSettle_ && startSnapAnimationCallback_ && frictionOffsetProperty_) {
        needItemSnapSettle_ = false;
        // When the settle spring takes over, the stop events (onDidStopFling / onScrollStop)
        // must wait for ITS completion — dispatching here would fire them while the spring is
        // still running and again at its end (double fire). "Not handled" (already on the
        // target, empty candidates) falls through so the stop chain still delivers them.
        bool tookOver = SettleSnapOnFrictionStop(true);
        if (tookOver) {
            // A mid-fling switch to a provider strategy parks the kickoff inside this settle
            // (the release path has already flushed); run it so the two-stage chain — and its
            // completion-driven stop events — actually start.
            FlushPendingItemSnapKickoff();
            return;
        }
    }
    // spring effect special process
    if (scrollPause_) {
        scrollPause_ = false;
        state_ = AnimationState::TRANSITION;
        HandleOverScroll(currentVelocity_);
        if (state_ == AnimationState::TRANSITION) {
            // didn't trigger spring animation
            state_ = AnimationState::IDLE;
            isUserFling_ = false;
            if (onDidStopFlingCallback_) {
                onDidStopFlingCallback_();
            }
        }
        return;
    }

    if (!isTouchStopAnimation_ && !isDragOuterScrollBarStopAnimation_ && source != SCROLL_FROM_AXIS &&
            source != SCROLL_FROM_LAYOUT) {
        if (onDidStopFlingCallback_) {
            onDidStopFlingCallback_();
        }
        isUserFling_ = false;
    }

    if (isDragUpdateStop_) {
        return;
    }
    moved_ = false;
    HandleScrollEnd(std::nullopt);
#ifdef OHOS_PLATFORM
    if (FrameReport::GetInstance().GetEnable()) {
        FrameReport::GetInstance().EndListFling();
    }
#endif
    currentVelocity_ = 0.0;
#if !defined(PREVIEW)
    LayoutInspector::SupportInspector();
#endif
}

void Scrollable::ProcessSpringMotion(double position)
{
    TAG_LOGD(AceLogTag::ACE_SCROLLABLE, "Current Pos is %{public}f, position is %{public}f", currentPos_, position);
    auto context = context_.Upgrade();
    CHECK_NULL_VOID(context);
    uint64_t currentVsync = context->GetVsyncTime();
    uint64_t diff = currentVsync - lastVsyncTime_;
    if (diff < MAX_VSYNC_DIFF_TIME && diff > MIN_DIFF_VSYNC) {
        currentVelocity_ = (position - currentPos_) / diff * MILLOS_PER_NANO_SECONDS;
    }
    lastVsyncTime_ = currentVsync;
    if (LessOrEqual(std::abs(currentPos_ - position), 1)) {
        // trace stop at OnScrollStop
        if (!isFadingAway_) {
            AceAsyncTraceBeginCommercial(
                nodeId_, (TRAILING_ANIMATION + std::to_string(nodeId_) + std::string(" ") + nodeTag_).c_str());
        } else {
            ACE_SCOPED_TRACE("Spring to same position");
        }
    }
    auto distance = currentPos_ - finalPosition_;
    auto nextDistance = position - finalPosition_;
    isFadingAway_ = GreatNotEqual(std::abs(nextDistance), std::abs(distance));
    auto delta = position - currentPos_;
    if (distance * nextDistance < 0) {
        double currentVelocity = currentVelocity_;
        scrollPause_ = true;
        MarkNeedFlushAnimationStartTime();
        StopSpringAnimation();
        ACE_SCOPED_TRACE("change direction in spring animation and start fling animation, distance:%f, "
                         "nextDistance:%f, nodeId:%d, tag:%s",
            distance, nextDistance, nodeId_, nodeTag_.c_str());
        // only handle offsets that are out of bounds
        delta = finalPosition_ - currentPos_;
        moved_ = UpdateScrollPosition(delta, SCROLL_FROM_ANIMATION_SPRING);
        // remainVelocityCallback_ will pass the velocity to the child component
        if (!remainVelocityCallback_ || !remainVelocityCallback_(currentVelocity)) {
            StartScrollAnimation(position, currentVelocity);
        }
    } else {
        moved_ = UpdateScrollPosition(delta, SCROLL_FROM_ANIMATION_SPRING);
    }
    if (!moved_) {
        StopSpringAnimation();
    }
    currentPos_ = position;
}

double Scrollable::CalcNextStep(double position, double mainDelta)
{
    auto finalDelta = finalPosition_ - currentPos_;
    if (GreatNotEqual(std::abs(finalDelta), SCROLL_SNAP_MIN_STEP_THRESHOLD)) {
        return mainDelta;
    }
    if (LessOrEqual(std::abs(finalDelta), SCROLL_SNAP_MIN_STEP)) {
        return finalDelta;
    }
    if (nextStep_.has_value()) {
        return nextStep_.value();
    }
    if (LessOrEqual(std::abs(mainDelta), SCROLL_SNAP_MIN_STEP)) {
        nextStep_ = Positive(finalDelta) ? SCROLL_SNAP_MIN_STEP : -SCROLL_SNAP_MIN_STEP;
        mainDelta = nextStep_.value();
    }
    return mainDelta;
}

void Scrollable::ProcessScrollMotion(double position, int32_t source)
{
    currentVelocity_ = state_ == AnimationState::SNAP ? snapVelocity_ : frictionVelocity_;
    auto mainDelta = position - currentPos_;
#ifdef SUPPORT_DIGITAL_CROWN
    if (state_ == AnimationState::SNAP && !itemSnapMonotonicSpring_) {
        mainDelta = CalcNextStep(position, mainDelta);
        position = currentPos_ + mainDelta;
    }
#endif
    if (needScrollSnapToSideCallback_) {
        needScrollSnapChange_ = needScrollSnapToSideCallback_(mainDelta);
    }
    TAG_LOGD(AceLogTag::ACE_SCROLLABLE, "position is %{public}f, currentVelocity_ is %{public}f, "
        "currentPos_ is %{public}f, needScrollSnapChange_ is %{public}u", position, currentVelocity_,
        currentPos_, needScrollSnapChange_);
    if (LessOrEqual(std::abs(mainDelta), 1)) {
        // trace stop at OnScrollStop
        AceAsyncTraceBeginCommercial(
            nodeId_, (TRAILING_ANIMATION + std::to_string(nodeId_) + std::string(" ") + nodeTag_).c_str());
    }
    // UpdateScrollPosition return false, means reach to scroll limit.
    source = snapAnimationSource_ == SCROLL_FROM_BAR && state_ == AnimationState::SNAP ? SCROLL_FROM_BAR_FLING : source;
    HandleScroll(mainDelta, source, NestedState::GESTURE);
    if (!moved_) {
        ResetContinueDragCount();
        StopFrictionAnimation();
        StopSnapAnimation();
    }
    currentPos_ = position;
#ifdef SUPPORT_DIGITAL_CROWN
    if (state_ == AnimationState::SNAP && NearEqual(currentPos_, finalPosition_)) {
        StopSnapAnimation();
    }
#endif
    // spring effect special process
    const bool isRefreshScroll = isRefreshScrollCallback_ && isRefreshScrollCallback_();
    if (((canOverScroll_ && source != SCROLL_FROM_AXIS) && !isRefreshScroll) || needScrollSnapChange_) {
        ACE_SCOPED_TRACE("scrollPause set true to stop ProcessScrollMotion, canOverScroll:%u, needScrollSnapChange:%u, "
                         "nodeId:%d, tag:%s",
            canOverScroll_, needScrollSnapChange_, nodeId_, nodeTag_.c_str());
        if (!snapAnimationFromScrollBar_) {
            scrollPause_ = true;
            skipRestartSpring_ = true;
            MarkNeedFlushAnimationStartTime();
        }
        ResetContinueDragCount();
        StopFrictionAnimation();
        StopSnapAnimation();
    }
}

bool Scrollable::UpdateScrollPosition(const double offset, int32_t source) const
{
    bool ret = true;
    if (callback_) {
        ret = callback_(offset, source);
    }
    return ret;
}

void Scrollable::ProcessScrollOverCallback(double velocity)
{
    if (outBoundaryCallback_ && !outBoundaryCallback_(true) && !canOverScroll_) {
        return;
    }
    // In the case of chain animation enabled, you need to switch the control point first,
    // and then correct the offset value in notification process
    if (notifyScrollOverCallback_) {
        notifyScrollOverCallback_(velocity);
    }
    // then use corrected offset to make scroll motion.
    if (scrollOverCallback_) {
        scrollOverCallback_(velocity);
    }
}

bool Scrollable::HandleOverScroll(double velocity)
{
    if (!overScrollCallback_) {
        if (edgeEffect_ == EdgeEffect::SPRING) {
            ProcessScrollOverCallback(velocity);
            return true;
        }
        if (scrollEndCallback_) {
            scrollEndCallback_();
        }
        return false;
    }
    // call NestableScrollContainer::HandleOverScroll
    return overScrollCallback_(velocity);
}

void Scrollable::SetSlipFactor(double SlipFactor)
{
    slipFactor_ = std::clamp(SlipFactor, -ADJUSTABLE_VELOCITY, ADJUSTABLE_VELOCITY);
}

void Scrollable::UpdateScrollSnapEndWithOffset(double offset)
{
    if (state_ == AnimationState::SNAP) {
        MarkNeedFlushAnimationStartTime();
        AnimationOption option;
        option.SetDuration(CUSTOM_SPRING_ANIMATION_DURATION);
        float response = SNAP_SPRING_RESPONSE;
        float damp = SNAP_SPRING_DAMP;
        if (GetSnapType() == SnapType::LIST_SNAP && listSnapSpeed_ == ScrollSnapAnimationSpeed::SLOW) {
            response = SLOW_SPRING_RESPONSE;
            damp = SLOW_SPRING_DAMP;
        }
        auto curve = AceType::MakeRefPtr<ResponsiveSpringMotion>(response, damp, 0.0f);
        option.SetCurve(curve);
        if (!snapOffsetProperty_) {
            GetSnapProperty();
        }
        updateSnapAnimationCount_++;
        endPos_ -= offset;
        finalPosition_ = endPos_;
        snapOffsetProperty_->SetPropertyUnit(PropertyUnit::PIXEL_POSITION);
        AnimationUtils::StartAnimation(
            option,
            [weak = AceType::WeakClaim(this)]() {
                auto scroll = weak.Upgrade();
                CHECK_NULL_VOID(scroll);
                scroll->snapOffsetProperty_->Set(scroll->endPos_);
                scroll->state_ = AnimationState::SNAP;
            },
            [weak = AceType::WeakClaim(this), id = Container::CurrentId()]() {
                ContainerScope scope(id);
                auto scroll = weak.Upgrade();
                CHECK_NULL_VOID(scroll);
                scroll->updateSnapAnimationCount_--;
                // avoid current animation being interrupted by the prev animation's finish callback
                if (scroll->updateSnapAnimationCount_ == 0) {
                    scroll->snapDirection_ = SnapDirection::NONE;
                    scroll->state_ = AnimationState::IDLE;
                    scroll->ProcessScrollSnapStop();
                }
            });
        state_ = AnimationState::SNAP;
    }
}

std::optional<float> Scrollable::GetPredictSnapOffset() const
{
    if (state_ == AnimationState::SNAP) {
        return endPos_ - currentPos_;
    }
    return std::nullopt;
}

void Scrollable::AttachAnimatableProperty(const RefPtr<NodeAnimatablePropertyFloat>& property)
{
    auto host = weakHost_.Upgrade();
    CHECK_NULL_VOID(host);
    auto renderContext = host->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    renderContext->AttachNodeAnimatableProperty(property);
}

void Scrollable::ProcessItemSnapFrictionFrame(float position)
{
    auto context = context_.Upgrade();
    CHECK_NULL_VOID(context);
    uint64_t currentVsync = context->GetVsyncTime();
    uint64_t diff = currentVsync - lastVsyncTime_;
    const bool velocityUpdated = diff < MAX_VSYNC_DIFF_TIME && diff > MIN_DIFF_VSYNC;
    if (std::isfinite(itemSnapFlingRate_) && itemSnapFlingRate_ > 0.0f) {
        // canOverScroll_ is conservative before the first layout; it must not disable this bound.
        if (itemSnapTrackingCallback_) {
            itemSnapTrackingCallback_();
            if (!itemSnapTargetReady_ && itemSnapLookaheadDistance_ > 0.0f) {
                const float direction = finalPosition_ > currentPos_ ? 1.0f : -1.0f;
                const float step = direction * (position - currentPos_);
                position = currentPos_ + direction * std::min(step, itemSnapLookaheadDistance_);
            }
        }
        if (!scrollPause_) {
            position = ClampItemSnapFrictionFrame(currentPos_, position,
                finalPosition_, itemSnapTakeoverDistance_);
        }
        // Position callbacks and vsync timestamps can refer to different frames after a stall.
        // The exponential friction derivative does not use that stale or coalesced time span.
        frictionVelocity_ = itemSnapFlingRate_ * (finalPosition_ - position);
    } else if (velocityUpdated) {
        frictionVelocity_ = (position - lastPosition_) / diff * MILLOS_PER_NANO_SECONDS;
    }
    lastVsyncTime_ = currentVsync;
    lastPosition_ = position;
    ProcessScrollMotion(position);
    if (state_ != AnimationState::FRICTION || TryItemSnapTakeover()) {
        return;
    }
    if (NearEqual(finalPosition_, position, 1.0) ||
        (velocityUpdated && NearZero(frictionVelocity_, FRICTION_VELOCITY_THRESHOLD))) {
        StopFrictionAnimation();
#ifndef CROSS_PLATFORM
        ResSchedReport::GetInstance().ResSchedDataReport("slide_off");
#endif
    }
}

RefPtr<NodeAnimatablePropertyFloat> Scrollable::GetFrictionProperty()
{
    auto propertyCallback = [weak = AceType::WeakClaim(this)](float position) {
        auto scroll = weak.Upgrade();
        CHECK_NULL_VOID(scroll);
        if (scroll->state_ != AnimationState::FRICTION || scroll->isTouching_) {
            return;
        }
        if (scroll->needItemSnapSettle_ &&
            (GreatNotEqual(scroll->itemSnapTakeoverDistance_, 0.0f) || scroll->itemSnapTrackingCallback_) &&
            scroll->GetSnapType() != SnapType::NONE_SNAP) {
            scroll->ProcessItemSnapFrictionFrame(position);
            return;
        }
        scroll->ProcessScrollMotion(position);
        if (NearEqual(scroll->finalPosition_, position, 1.0)) {
            scroll->StopFrictionAnimation();
        }
        auto context = scroll->context_.Upgrade();
        CHECK_NULL_VOID(context);
        uint64_t currentVsync = context->GetVsyncTime();
        uint64_t diff = currentVsync - scroll->lastVsyncTime_;
        if (diff < MAX_VSYNC_DIFF_TIME && diff > MIN_DIFF_VSYNC) {
            scroll->frictionVelocity_ = (position - scroll->lastPosition_) / diff * MILLOS_PER_NANO_SECONDS;
            if (NearZero(scroll->frictionVelocity_, FRICTION_VELOCITY_THRESHOLD)) {
                scroll->StopFrictionAnimation();
#ifndef CROSS_PLATFORM
                ResSchedReport::GetInstance().ResSchedDataReport("slide_off");
#endif
            }
        }
        scroll->lastVsyncTime_ = currentVsync;
        scroll->lastPosition_ = position;
    };
    frictionOffsetProperty_ = AceType::MakeRefPtr<NodeAnimatablePropertyFloat>(0.0, std::move(propertyCallback));
    AttachAnimatableProperty(frictionOffsetProperty_);
    return frictionOffsetProperty_;
}

RefPtr<NodeAnimatablePropertyFloat> Scrollable::GetSpringProperty()
{
    auto propertyCallback = [weak = AceType::WeakClaim(this)](float position) {
        auto scroll = weak.Upgrade();
        CHECK_NULL_VOID(scroll);
        if (scroll->state_ != AnimationState::SPRING) {
            return;
        }
        if (!NearEqual(scroll->finalPosition_, position, SPRING_ACCURACY)) {
            scroll->ProcessSpringMotion(position);
            return;
        }
        /*
         * In order to prevent accumulation errors, the current position is re obtained to ensure that
         * the last frame can accurately stop at the top and bottom positions.
         */
        if (scroll->currentPositionCallback_) {
            double currPos = scroll->currentPositionCallback_();
            if (NearEqual(currPos, scroll->currentPos_, 0.5)) {
                scroll->currentPos_ = currPos;
            }
        }
        scroll->StopSpringAnimation(true);
    };
    springOffsetProperty_ = AceType::MakeRefPtr<NodeAnimatablePropertyFloat>(0.0, std::move(propertyCallback));
    AttachAnimatableProperty(springOffsetProperty_);
    return springOffsetProperty_;
}

RefPtr<NodeAnimatablePropertyFloat> Scrollable::GetSnapProperty()
{
    auto propertyCallback = [weak = AceType::WeakClaim(this)](float position) {
        auto scroll = weak.Upgrade();
        CHECK_NULL_VOID(scroll);
        if (scroll->state_ != AnimationState::SNAP) {
            return;
        }
        auto context = scroll->context_.Upgrade();
        CHECK_NULL_VOID(context);
        if (scroll->itemSnapMonotonicSpring_) {
            // Roundoff or an obsolete sample must neither cross the target nor reverse the motion.
            const float current = static_cast<float>(scroll->currentPos_);
            if (!std::isfinite(position) || (!NearEqual(current, scroll->endPos_, SPRING_ACCURACY) &&
                (position - current) * (scroll->endPos_ - current) <= 0.0f)) {
                return;
            }
            position = std::clamp(position, std::min(current, scroll->endPos_), std::max(current, scroll->endPos_));
        }
        uint64_t currentVsync = context->GetVsyncTime();
        uint64_t diff = currentVsync - scroll->lastVsyncTime_;
        if (diff < MAX_VSYNC_DIFF_TIME && diff > MIN_DIFF_VSYNC) {
            scroll->snapVelocity_ = (position - scroll->currentPos_) / diff * MILLOS_PER_NANO_SECONDS;
        }
        scroll->lastVsyncTime_ = currentVsync;

        if (NearEqual(scroll->endPos_, position, SPRING_ACCURACY)) {
            if (scroll->GetSnapType() == SnapType::LIST_SNAP) {
                scroll->ProcessListSnapMotion(scroll->endPos_);
            } else if (scroll->GetSnapType() == SnapType::SCROLL_SNAP) {
                scroll->ProcessScrollMotion(scroll->endPos_);
            }
            scroll->StopSnapAnimation();
        } else {
            if (scroll->GetSnapType() == SnapType::LIST_SNAP) {
                scroll->ProcessListSnapMotion(position);
            } else if (scroll->GetSnapType() == SnapType::SCROLL_SNAP) {
                scroll->ProcessScrollMotion(position);
            }
        }
    };
    snapOffsetProperty_ = AceType::MakeRefPtr<NodeAnimatablePropertyFloat>(0.0, std::move(propertyCallback));
    AttachAnimatableProperty(snapOffsetProperty_);
    return snapOffsetProperty_;
}

void Scrollable::StopFrictionAnimation()
{
    if (state_ == AnimationState::FRICTION) {
        ACE_SCOPED_TRACE("StopFrictionAnimation, id:%d, tag:%s", nodeId_, nodeTag_.c_str());
        state_ = AnimationState::IDLE;
        CHECK_NULL_VOID(frictionOffsetProperty_);
        AnimationOption option;
        option.SetCurve(Curves::EASE);
        option.SetDuration(0);
        AnimationUtils::StartAnimation(
            option,
            [weak = AceType::WeakClaim(this)]() {
                auto scroll = weak.Upgrade();
                CHECK_NULL_VOID(scroll);
                scroll->frictionOffsetProperty_->Set(scroll->currentPos_);
            },
            nullptr);
    }
}

void Scrollable::StopSpringAnimation(bool reachFinalPosition)
{
    if (state_ == AnimationState::SPRING) {
        ACE_SCOPED_TRACE(
            "StopSpringAnimation, reachFinalPosition:%u, id:%d, tag:%s", reachFinalPosition, nodeId_, nodeTag_.c_str());
        state_ = AnimationState::IDLE;
        isFadingAway_ = false;
        CHECK_NULL_VOID(springOffsetProperty_);
        AnimationOption option;
        option.SetCurve(Curves::EASE);
        option.SetDuration(0);
        AnimationUtils::StartAnimation(
            option,
            [weak = AceType::WeakClaim(this), reachFinalPosition]() {
                auto scroll = weak.Upgrade();
                CHECK_NULL_VOID(scroll);
                if (reachFinalPosition) {
                    // ensure that the spring animation is restored to its final position.
                    scroll->ProcessSpringMotion(scroll->finalPosition_);
                    // use non final position to stop animation, otherwise the animation cannot be stoped.
                    scroll->springOffsetProperty_->Set(scroll->finalPosition_ - 1.f);
                } else {
                    // avoid top edge spring can not stop
                    scroll->springOffsetProperty_->Set(scroll->currentPos_);
                }
            },
            nullptr);
    }
    currentVelocity_ = 0.0;
}

void Scrollable::StopSnapAnimation()
{
    if (state_ == AnimationState::SNAP) {
        ACE_SCOPED_TRACE("StopSnapAnimation, animation state:%d, id:%d, tag:%s", state_, nodeId_, nodeTag_.c_str());
        state_ = AnimationState::IDLE;
        nextStep_.reset();
        CHECK_NULL_VOID(snapOffsetProperty_);
        AnimationOption option;
        option.SetCurve(Curves::EASE);
        option.SetDuration(0);
        AnimationUtils::StartAnimation(
            option,
            [weak = AceType::WeakClaim(this)]() {
                auto scroll = weak.Upgrade();
                CHECK_NULL_VOID(scroll);
                auto position = NearEqual(scroll->currentPos_, scroll->finalPosition_) ?
                    scroll->finalPosition_ - 1.f : scroll->currentPos_;
                scroll->snapOffsetProperty_->Set(position);
            },
            nullptr);
    }
}

void Scrollable::StopAxisAnimation()
{
    CHECK_NULL_VOID(axisAnimator_);
    axisAnimator_->StopAxisAnimation();
}

void Scrollable::OnCollectTouchTarget(TouchTestResult& result, const RefPtr<FrameNode>& frameNode,
    ResponseLinkResult& responseLinkResult)
{
    if (panRecognizerNG_) {
        panRecognizerNG_->SetNodeId(frameNode->GetId());
        panRecognizerNG_->AttachFrameNode(frameNode);
        panRecognizerNG_->SetIsSystemGesture(true);
        panRecognizerNG_->SetRecognizerType(GestureTypeName::PAN_GESTURE);
        result.emplace_back(panRecognizerNG_);
        responseLinkResult.emplace_back(panRecognizerNG_);
    }
}

void Scrollable::SetMaxFlingVelocity(double max)
{
    auto frameNode = weakHost_.Upgrade();
    FREE_NODE_CHECK(frameNode, SetMaxFlingVelocity, max);
    double density = PipelineBase::GetCurrentDensity();
    maxFlingVelocity_ = max * density;
}

void Scrollable::SetMaxFlingVelocityMultiThread(double max)
{
    auto frameNode = weakHost_.Upgrade();
    CHECK_NULL_VOID(frameNode);
    frameNode->PostAfterAttachMainTreeTask([weak = AceType::WeakClaim(this), max]() {
        auto scrollable = weak.Upgrade();
        CHECK_NULL_VOID(scrollable);
        double density = PipelineBase::GetCurrentDensity();
        scrollable->SetMaxFlingVelocityValue(max * density);
    });
}

void Scrollable::HandleScrollBarOnDidStopDragging(bool isWillFling)
{
    if (onDidStopDraggingCallback_) {
        onDidStopDraggingCallback_(isWillFling);
    }
    if (!isWillFling) {
        isScrollBarDragging_ = false;
        if (isUserFling_ && onDidStopFlingCallback_) {
            onDidStopFlingCallback_();
        }
    }
    isUserFling_ = isWillFling;
}

void Scrollable::HandleScrollBarOnWillStartFling()
{
    if (onWillStartFlingCallback_) {
        onWillStartFlingCallback_();
    }
    isUserFling_ = true;
}

void Scrollable::HandleScrollBarOnDidStopFling()
{
    if (isUserFling_ && !isTouchStopAnimation_ && !isDragOuterScrollBarStopAnimation_) {
        isUserFling_ = false;
        if (onDidStopFlingCallback_) {
            onDidStopFlingCallback_();
        }
    }
}

} // namespace OHOS::Ace::NG

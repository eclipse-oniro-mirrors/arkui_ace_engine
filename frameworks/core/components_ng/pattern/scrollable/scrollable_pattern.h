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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SCROLLABLE_SCROLLABLE_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SCROLLABLE_SCROLLABLE_PATTERN_H

#include <cstdint>
#include <functional>
#include <list>
#include <memory>
#include <optional>
#include <queue>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "base/geometry/axis.h"
#include "core/components/common/layout/constants.h"
#include "core/components_ng/base/frame_scene_status.h"
#include "core/components_ng/event/scrollable_event.h"
#include "core/components_ng/manager/content_change_manager/content_change_manager.h"
#include "core/components_ng/pattern/scroll_bar/proxy/scroll_bar_proxy.h"
#include "core/components_ng/pattern/scrollable/nestable_scroll_container.h"
#include "core/components_ng/pattern/scrollable/scrollable.h"
#include "core/components_ng/pattern/scrollable/scrollable_controller.h"
#include "core/components_ng/pattern/scrollable/scrollable_properties.h"
#include "core/components_ng/render/animation_utils.h"
#ifdef SUPPORT_DIGITAL_CROWN
#include "core/event/crown_event.h"
#endif
#include "core/components/scroll/scroll_controller_base.h"
#include "core/event/statusbar/statusbar_event_proxy.h"
#ifndef CROSS_PLATFORM
#include "core/common/recorder/event_recorder.h"
#endif

namespace OHOS::Ace {
class Animator;
class BezierVariableVelocityMotion;
class ScrollControllerBase;
class VelocityMotion;
// Item snap types are defined in scroll_snap_strategy.h and scroll_snap_utils.h;
// keep them incomplete here to avoid spreading their dependencies.
struct ScrollSnapStrategy;
struct SnapCandidate;
struct SnapSelectResult;
enum class ScrollAlign : int;
enum class ScrollEdgeType : size_t;
} // namespace OHOS::Ace

namespace OHOS::Ace::NG {
class InspectorFilter;
class GestureEventHub;
class InputEvent;
class InputEventHub;
class NavDestinationPatternBase;
class RefreshCoordination;
enum class RefreshCoordinationMode : char;
class Scrollable;
class ScrollableController;
class ScrollablePaintMethod;
class ScrollBar;
struct ScrollBarProperty;
class ScrollBarOverlayModifier;
class ScrollBarProxy;
class ScrollEdgeEffect;
class TouchEventImpl;
enum class DragEventType;
#ifndef WEARABLE_PRODUCT
constexpr double FRICTION = 0.6;
constexpr double API11_FRICTION = 0.7;
constexpr double API12_FRICTION = 0.75;
constexpr double SLOW_FRICTION_THRESHOLD = 3000.0;
constexpr double SLOW_FRICTION = 1.0;
constexpr double MAX_VELOCITY = 9000.0;
constexpr double VELOCITY_SCALE = 1.0;
constexpr double SLOW_VELOCITY_SCALE = 1.2;
constexpr double NEW_VELOCITY_SCALE = 1.5;
#else
constexpr double FRICTION = 0.9;
constexpr double VELOCITY_SCALE = 0.8;
constexpr double MAX_VELOCITY = 5000.0;
#endif
constexpr float SPRING_ACCURACY = 0.1f;
enum class ModalSheetCoordinationMode : char {
    UNKNOWN = 0,
    SHEET_SCROLL = 1,
    SCROLLABLE_SCROLL = 2,
};
enum class ScrollToDirection {
    NONE = 0,
    FORWARD,
    BACKWARD
};
enum class ScrollError {
    SCROLL_NO_ERROR,
    SCROLL_TOP_ERROR,
    SCROLL_BOTTOM_ERROR,
    SCROLL_NOT_SCROLLABLE_ERROR,
    SCROLL_ERROR_OTHER
};
enum class AccessibilityScrollSource {
    NONE,
    USER,
    API,
    ACCESSIBILITY,
    FOCUS,
};
struct ScrollOffsetAbility {
    std::function<bool(float)> scrollFunc = nullptr;
    Axis axis = Axis::VERTICAL;
    float contentStartOffset = 0.0f;
    float contentEndOffset = 0.0f;
};
struct ScrollOnInjectionEventInfo {
    bool isScrollByRatio = false;
    bool isScrollByOffset = false;
    bool isScrollByTargetId = false;
    int reportEventId = 0;
    int32_t targetId = -1;
    float ratio = 0.0f;
    double scrollOffset = 0.0;
};

struct ScrollToIndexTarget {
    int32_t index = -1;
    float extraOffset = 0.0f;
    ScrollAlign align = ScrollAlign::START;
    float targetOffset = 0.0f;
    float skipDistance = 0.0f;
    float viewportSize = 0.0f;
    FastScrollMode fastScrollMode = FastScrollMode::NONE;
    bool skipPending = false;
};

class ACE_FORCE_EXPORT ScrollablePattern : public NestableScrollContainer, public virtual StatusBarClickListener {
    DECLARE_ACE_TYPE(ScrollablePattern, NestableScrollContainer);

public:
    ScrollablePattern();
    ScrollablePattern(EdgeEffect edgeEffect, bool alwaysEnabled);

    ~ScrollablePattern() override;

    bool IsAtomicNode() const override
    {
        return false;
    }

    RefPtr<PaintProperty> CreatePaintProperty() override;

    void CreateAnalyzerOverlay(const RefPtr<FrameNode> node);

    void UpdateFadingEdge(const RefPtr<ScrollablePaintMethod>& paint);

    void ToJsonValue(std::unique_ptr<JsonValue>& json, const InspectorFilter& filter) const override;
    void OnWindowHide() override;

    // scrollable
    Axis GetAxis() const override
    {
        return axis_;
    }

    virtual bool IsReverse() const
    {
        return GetAxis() == Axis::HORIZONTAL && IsRTL();
    };

    bool IsRTL() const
    {
        auto host = GetHost();
        CHECK_NULL_RETURN(host, false);
        auto layoutProperty = host->GetLayoutProperty<LayoutProperty>();
        CHECK_NULL_RETURN(layoutProperty, false);
        auto layoutDirection = layoutProperty->GetNonAutoLayoutDirection();
        return (layoutDirection == TextDirection::RTL);
    };

    virtual bool ShouldDelayChildPressedState() const override
    {
        return true;
    }

    virtual bool ShouldPreventChildPressedState() const override
    {
        return isHitTestBlock_;
    }

    void RegisterScrollingListener(const RefPtr<ScrollingListener> listener) override;
    void FireAndCleanScrollingListener() override;
    void CleanScrollingListener() override;

    void SetAxis(Axis axis);
    virtual bool UpdateCurrentOffset(float delta, int32_t source) = 0;
    virtual bool IsScrollable() const
    {
        return false;
    }
    virtual bool IsAtTop() const = 0;
    virtual bool IsAtBottom(bool considerRepeat = false) const = 0;
    virtual bool IsAtTopWithDelta() const
    {
        return IsAtTop();
    }
    virtual bool IsAtBottomWithDelta() const
    {
        return IsAtBottom();
    }
    virtual bool IsFadingBottom() const
    {
        return !IsAtBottom();
    }
    virtual bool OutBoundaryCallback(bool useCurrentDelta = true)
    {
        return IsOutOfBoundary(useCurrentDelta);
    }

    virtual bool IsOutOfBoundary(bool useCurrentDelta = true)
    {
        return false;
    }

    virtual bool TryFreeScroll(double offset, Axis axis)
    {
        return false;
    }

    virtual bool FreeOverScrollWithDelta(Axis axis, double delta)
    {
        return false;
    }

    virtual bool CanOverScrollWithDelta(double delta, bool isNestScroller = false);

    virtual void OnTouchDown(const TouchEventInfo& info);

    virtual bool ShouldIgnoreTouchUpWithActiveFingers() const
    {
        return false;
    }

    void OnTouchpadInteraction(PointF point);

    virtual void ProcessFreeScrollOverDrag(const OffsetF velocity) {};

    void AddScrollEvent();
    RefPtr<ScrollableEvent> GetScrollableEvent()
    {
        return scrollableEvent_;
    }

    RefPtr<Scrollable> GetScrollable()
    {
        CHECK_NULL_RETURN(scrollableEvent_, nullptr);
        return scrollableEvent_->GetScrollable();
    }

    void SetScrollPanEscape(const std::unordered_set<int32_t>& fingerIds, bool toEntityManager = false)
    {
        auto scrollable = GetScrollable();
        CHECK_NULL_VOID(scrollable);
        scrollable->SetEscapeModeForScroll(fingerIds, toEntityManager);
    }

    // Number of fingers currently touching this scrollable. Drag hosts use it to
    // tell "one finger dragging + another finger scrolling" apart from a lone
    // dragging finger. Maintained by InitTouchEvent() from the raw touch stream.
    size_t GetActiveTouchFingerIdsSize() const
    {
        return activeTouchFingerIds_.size();
    }

    virtual bool OnScrollCallback(float offset, int32_t source);
    virtual void ReportBackToTop();
    virtual void OnScrollStartCallback();
    virtual void FireOnScrollStart(bool withPerfMonitor = true);
    virtual void FireOnReachStart(const OnReachEvent& onReachStart, const OnReachEvent& onJSFrameNodeReachStart) {}
    virtual void FireOnReachEnd(const OnReachEvent& onReachEnd, const OnReachEvent& onJSFrameNodeReachEnd) {}
    bool ScrollableIdle()
    {
        return !scrollableEvent_ || scrollableEvent_->Idle();
    }
    bool ScrollBarIdle()
    {
        return !scrollBarProxy_ || scrollBarProxy_->Idle();
    }
    bool InnerScrollBarIdle();
    void SetScrollEnabled(bool enabled)
    {
        CHECK_NULL_VOID(scrollableEvent_);
        bool bNest = false;
        if (scrollBarProxy_) {
            bNest = scrollBarProxy_->IsNestScroller();
        }

        if (enabled || bNest) {
            enabled = true;
            scrollableEvent_->SetAxis(axis_);
        } else {
            scrollableEvent_->SetAxis(Axis::NONE);
        }
        scrollableEvent_->SetEnabled(enabled);

        if (scrollBarProxy_) {
            scrollBarProxy_->SetScrollEnabled(enabled, AceType::WeakClaim(this));
        }
    }

    bool GetScrollEnabled() const
    {
        CHECK_NULL_RETURN(scrollableEvent_, false);
        return scrollableEvent_->GetEnabled();
    }

    RefPtr<GestureEventHub> GetGestureHub();
    RefPtr<InputEventHub> GetInputHub();

    // edgeEffect
    const RefPtr<ScrollEdgeEffect>& GetScrollEdgeEffect() const;
    bool CanFadeEffect(float offset, bool isAtTop, bool isAtBottom) const;
    bool HandleEdgeEffect(float offset, int32_t source, const SizeF& size);
    void HandleFadeEffect(float offset, int32_t source, const SizeF& size, bool isNotPositiveScrollableDistance);
    virtual void SetEdgeEffectCallback(const RefPtr<ScrollEdgeEffect>& scrollEffect) {}
    bool IsRestrictBoundary();
    // scrollBar
    virtual void UpdateScrollBarOffset() = 0;
    void SetScrollBar(const std::unique_ptr<ScrollBarProperty>& property);
    virtual RefPtr<ScrollBar> CreateScrollBar();
    void SetScrollBar(DisplayMode displayMode);
    void SetScrollBarProxy(const RefPtr<ScrollBarProxy>& scrollBarProxy);
    virtual RefPtr<ScrollBarOverlayModifier> CreateOverlayModifier();
    void CreateScrollBarOverlayModifier();
    virtual void AdjustOffset(float& delta, int32_t source) {}

    float GetScrollableDistance() const
    {
        return estimatedHeight_;
    }

    float GetBarOffset() const
    {
        return barOffset_;
    }

    float GetScrollBarOutBoundaryExtent() const
    {
        return scrollBarOutBoundaryExtent_;
    }

    void SetScrollBarOutBoundaryExtent(float scrollBarOutBoundaryExtent)
    {
        scrollBarOutBoundaryExtent_ = scrollBarOutBoundaryExtent;
    }

    void HandleScrollBarOutBoundary(float scrollBarOutBoundaryExtent);

    double GetMainSize(const SizeF& size) const
    {
        return axis_ == Axis::HORIZONTAL ? size.Width() : size.Height();
    }

    bool IsScrollableStopped() const
    {
        CHECK_NULL_RETURN(scrollableEvent_, true);
        auto scrollable = scrollableEvent_->GetScrollable();
        CHECK_NULL_RETURN(scrollable, true);
        return scrollable->IsStopped();
    }

    bool GetIsDragging() const
    {
        CHECK_NULL_RETURN(scrollableEvent_, false);
        auto scrollable = scrollableEvent_->GetScrollable();
        CHECK_NULL_RETURN(scrollable, false);
        return scrollable->GetIsDragging();
    }

    void StopScrollable()
    {
        CHECK_NULL_VOID(scrollableEvent_);
        auto scrollable = scrollableEvent_->GetScrollable();
        CHECK_NULL_VOID(scrollable);
        scrollable->StopScrollable();
    }

    virtual bool StartSnapAnimation(SnapAnimationOptions snapAnimationOptions)
    {
        return false;
    }

    bool IsScrollableSpringEffect() const;

    void SetCoordEventNeedSpringEffect(bool IsCoordEventNeedSpring)
    {
        isCoordEventNeedSpring_ = IsCoordEventNeedSpring;
    }

    void GetParentNavigation();
    void GetParentModalSheet();

    /**
     * @brief Return the portion of delta that's in overScroll range.
     *
     * @param delta incoming offset change.
     * @return the portion of delta in overScroll range. Both top overScroll and bottom overScroll.
     */
    virtual OverScrollOffset GetOverScrollOffset(double delta) const
    {
        return { 0, 0 };
    }

    void StartScrollBarAnimatorByProxy()
    {
        if (scrollBarProxy_) {
            scrollBarProxy_->StartScrollBarAnimator();
        }

        for (auto proxy : nestScrollBarProxy_) {
            auto scrollBarProxy = proxy.Upgrade();
            CHECK_NULL_CONTINUE(scrollBarProxy);
            scrollBarProxy->StartScrollBarAnimator();
        }
    }

    void StopScrollBarAnimatorByProxy()
    {
        if (scrollBarProxy_) {
            scrollBarProxy_->StopScrollBarAnimator();
        }

        for (auto proxy : nestScrollBarProxy_) {
            auto scrollBarProxy = proxy.Upgrade();
            CHECK_NULL_CONTINUE(scrollBarProxy);
            scrollBarProxy->StopScrollBarAnimator();
        }
    }

    virtual void SetFriction(double friction);
    void SetFrictionMultiThread(double friction);

    double GetFriction() const
    {
        return friction_;
    }

    void SetVelocityScale(double scale);

    void SetMaxFlingVelocity(double max);

    double GetMaxFlingVelocity() const;

    virtual void StopAnimate();
    bool AnimateRunning() const;
    bool AnimateStoped() const;

    void AbortScrollAnimator();
    bool GetScrollAbort() const
    {
        return scrollAbort_;
    }
    void SetScrollAbort(bool abort)
    {
        scrollAbort_ = abort;
    }
    void PlaySpringAnimation(float position, const SpringCurveOption& option, bool useTotalOffset = true,
        bool isFormUser = false);
    void PlayCurveAnimation(float position, float duration, const RefPtr<Curve>& curve, bool canOverScroll);
    virtual double GetTotalOffset() const
    {
        return 0.0f;
    }

    /* ============================= Free Scroll Enhancements ============================= */
    virtual Offset GetFreeScrollOffset() const
    {
        return {};
    }
    virtual bool FreeScrollBy(const OffsetF& delta, bool canOverScroll = false)
    {
        return false;
    }
    virtual bool FreeScrollPage(bool reverse, bool smooth)
    {
        return false;
    }
    virtual bool FreeScrollToEdge(ScrollEdgeType type, bool smooth, std::optional<float> velocity)
    {
        return false;
    }
    virtual void FreeScrollTo(const ScrollControllerBase::ScrollToParam& param) {}
    /* ============================================================================== */

    virtual float GetContentStartOffset() const
    {
        return 0.0f;
    }
    virtual float GetContentEndOffset() const
    {
        return 0.0f;
    }
    // main size of all children
    virtual float GetTotalHeight() const
    {
        return 0.0f;
    }
    virtual void OnAnimateStop();
    virtual void ScrollTo(float position);
    void SmartGesturePerformScroll(float position);
    // Preserve the virtual AnimateTo ABI; consume the request before entering callbacks.
    void AnimateToFromController(float position, float duration, const RefPtr<Curve>& curve,
        bool smooth, bool canOverScroll)
    {
        auto previous = capiScrollToRequest_;
        capiScrollToRequest_ = true;
        AnimateTo(position, duration, curve, smooth, canOverScroll);
        capiScrollToRequest_ = previous;
    }
    bool TakeCapiScrollToEventReportRequest()
    {
        auto requested = capiScrollToRequest_;
        capiScrollToRequest_ = false;
        return requested;
    }
    void UpdateCapiScrollEventSource(bool isScrollTo);
    void ScheduleCapiScrollCommand(bool isScrollTo, bool isScrollBy = false);
    void MarkCapiScrollByEventSource();
    virtual void AnimateTo(
        float position, float duration, const RefPtr<Curve>& curve, bool smooth, bool canOverScroll = false,
        bool useTotalOffset = true);
    virtual bool CanOverScroll(int32_t source)
    {
        auto canOverScroll =
            IsScrollableSpringEffect() && source != SCROLL_FROM_AXIS && source != SCROLL_FROM_BAR && IsScrollable();
        if (canOverScroll != lastCanOverScroll_) {
            lastCanOverScroll_ = canOverScroll;
            AddScrollableFrameInfo(source);
        }
        return canOverScroll;
    }
    bool CanOverScrollStart(int32_t source)
    {
        return (CanOverScroll(source) && GetEffectEdge() != EffectEdge::END) && (!ScrollableIdle() ||
            animateCanOverScroll_ || source == SCROLL_FROM_BAR_OVER_DRAG || animateOverScrollStart_);
    }
    bool CanOverScrollEnd(int32_t source)
    {
        return (CanOverScroll(source) && GetEffectEdge() != EffectEdge::START) && (!ScrollableIdle() ||
            animateCanOverScroll_ || source == SCROLL_FROM_BAR_OVER_DRAG || animateOverScrollEnd_);
    }
    void SetCanStayOverScroll(bool canStayOverScroll)
    {
        canStayOverScroll_ = canStayOverScroll;
    }
    bool GetCanStayOverScroll() const
    {
        return canStayOverScroll_;
    }
    void ChangeAnimateOverScroll()
    {
        if (GetScrollSource() == SCROLL_FROM_JUMP && GetCanStayOverScroll()) {
            SetAnimateCanOverScroll(false);
        }
    }
    void ChangeCanStayOverScroll()
    {
        if (GetIsOverScroll()) {
            if (!IsOutOfBoundary()) {
                SetIsOverScroll(false);
            }
        }
    }
    void HandleAnimateFromUserStop();

    // scrollSnap
    virtual std::optional<float> CalcPredictSnapOffset(
        float delta, float dragDistance, float velocity, SnapDirection snapDirection)
    {
        std::optional<float> predictSnapPosition;
        return predictSnapPosition;
    }

    virtual void SetLastSnapTargetIndex(int32_t lastSnapTargetIndex) {}

    virtual std::optional<int32_t> GetLastSnapTargetIndex()
    {
        return std::nullopt;
    }

    virtual void ResetLastSnapTargetIndex() {}

    void ResetScrollableSnapDirection()
    {
        auto scrollable = GetScrollable();
        CHECK_NULL_VOID(scrollable);
        scrollable->ResetSnapDirection();
    }

    void SetScrollableCurrentPos(float currentPos)
    {
        auto scrollable = GetScrollable();
        CHECK_NULL_VOID(scrollable);
        return scrollable->SetCurrentPos(currentPos);
    }

    virtual bool NeedScrollSnapToSide(float delta)
    {
        return false;
    }

    void SetScrollSource(int32_t scrollSource)
    {
        scrollSourceForQuery_ = scrollSource;      // Session-level: updated only by event/stop-chain entries
        SetScrollSourceInternal(scrollSource);
    }

    // Layout entry: updates only the frame-level scrollSource_, does not touch the session-level source
    void SetScrollSourceLayout(int32_t scrollSource)
    {
        SetScrollSourceInternal(scrollSource);
    }

    void SetScrollSourceInternal(int32_t scrollSource)
    {
        if (scrollSource == SCROLL_FROM_NONE) {
            if (lastScrollSource_ != scrollSource_) {
                AddScrollableFrameInfo(scrollSource_);
            }
            lastScrollSource_ = scrollSource_;
        }
        scrollSource_ = scrollSource;
    }

    // Direct write to the session-level source only (does not touch the frame-level scrollSource_)
    void SetScrollSourceForQuery(int32_t scrollSource)
    {
        scrollSourceForQuery_ = scrollSource;
    }

    void SetAccessibilityScrollSource(AccessibilityScrollSource source)
    {
        accessibilityScrollSource_ = source;
    }

    void ResetAccessibilityScrollSourceIfIdle();

    std::string GetAccessibilityScrollSource();

    void MarkUserScrollSource(int32_t source);

    int32_t GetScrollSource() const
    {
        return scrollSource_;
    }

    float GetCurrentVelocity() const
    {
        return currentVelocity_;
    }

    double GetScrollableCurrentVelocity() const
    {
        CHECK_NULL_RETURN(scrollableEvent_, 0.0);
        auto scrollable = scrollableEvent_->GetScrollable();
        CHECK_NULL_RETURN(scrollable, 0.0);
        return scrollable->GetCurrentVelocity();
    }

    ScrollState GetScrollState() const;

    // Query API: reads the session-level scrollSourceForQuery_, unaffected by layout-level resets.
    ScrollState GetScrollStateForQuery() const;

    static ScrollState GetScrollState(int32_t scrollSource);

    static ScrollSource ConvertScrollSource(int32_t source);

    struct ItemLazySyntax {
        bool arkoala = false;
        bool repeatV2 = false;
    };
    // Lazy syntax kinds that directly provide items of host.
    ACE_FORCE_EXPORT static ItemLazySyntax FindItemLazySyntax(const RefPtr<UINode>& host);

    static int32_t ScrollToTarget(
        RefPtr<FrameNode>& scrollable, RefPtr<FrameNode>& target, float targetOffset, ScrollAlign targetAlign);

    float CalculateFriction(float gamma);
    virtual float GetMainContentSize() const;

    virtual bool SupportScrollToIndex() const
    {
        return true;
    }

    virtual ScrollAlign GetDefaultScrollAlign() const
    {
        return ScrollAlign::START;
    }

    virtual void ScrollToIndex(int32_t index, bool smooth = false, ScrollAlign align = ScrollAlign::START,
        std::optional<float> extraOffset = std::nullopt)
    {}

    virtual void ScrollToIndexFastScroll(int32_t index, bool smooth, ScrollAlign align,
        std::optional<float> extraOffset, FastScrollMode mode)
    {
        ScrollToIndex(index, smooth, align, extraOffset);
    }

    void SetExtraOffset(std::optional<float> extraOffset)
    {
        extraOffset_ = extraOffset;
    }

    const std::optional<float>& GetExtraOffset() const
    {
        return extraOffset_;
    }

    void ResetExtraOffset()
    {
        extraOffset_.reset();
    }

    virtual void ScrollToEdge(ScrollEdgeType scrollEdgeType, bool smooth);

    virtual ScrollEdgeType GetScrollEdgeType() const
    {
        return ScrollEdgeType::SCROLL_NONE;
    }

    virtual void SetScrollEdgeType(ScrollEdgeType scrollEdgeType) {}

    virtual bool IsScrollEdgeFinish() const
    {
        return true;
    }

    virtual void Fling(double flingVelocity);

    void SetPositionController(RefPtr<ScrollableController> control)
    {
        positionController_ = control;
        if (control) {
            control->SetScrollPattern(AceType::WeakClaim(this));
        }
    }

    RefPtr<ScrollableController> GetOrCreatePositionController()
    {
        if (!positionController_) {
            auto controller = AceType::MakeRefPtr<NG::ScrollableController>();
            SetPositionController(controller);
        }
        return positionController_;
    }

    virtual Rect GetItemRect(int32_t index) const
    {
        return Rect();
    };

    virtual int32_t GetItemIndex(double x, double y) const
    {
        return -1;
    }

    void SetEdgeEffect(EdgeEffect edgeEffect, bool alwaysEnabled, EffectEdge effectEdge = EffectEdge::ALL)
    {
        edgeEffect_ = edgeEffect;
        edgeEffectAlwaysEnabled_ = alwaysEnabled;
        effectEdge_ = effectEdge;
    }

    EdgeEffect GetEdgeEffect()
    {
        return edgeEffect_;
    }

    bool GetAlwaysEnabled() const
    {
        return edgeEffectAlwaysEnabled_;
    }

    EffectEdge GetEffectEdge() const
    {
        return effectEdge_;
    }

    void SetAlwaysEnabled(bool alwaysEnabled)
    {
        edgeEffectAlwaysEnabled_ = alwaysEnabled;
    }

    bool IsScrollableAnimationNotRunning()
    {
        if (scrollableEvent_) {
            auto scrollable = scrollableEvent_->GetScrollable();
            if (scrollable) {
                return scrollable->IsAnimationNotRunning();
            }
            return false;
        }
        return false;
    }

    float GetFinalPosition() const
    {
        return finalPosition_;
    }
    void HandleOnDragStatusCallback(
        const DragEventType& dragEventType, const RefPtr<NotifyDragEvent>& notifyDragEvent) override;

    bool IsScrollableSpringMotionRunning()
    {
        CHECK_NULL_RETURN(scrollableEvent_, false);
        auto scrollable = scrollableEvent_->GetScrollable();
        CHECK_NULL_RETURN(scrollable, false);
        return scrollable->IsSpringMotionRunning();
    }

    virtual SnapType GetSnapType()
    {
        return SnapType::NONE_SNAP;
    }

    // Feat-20260806915268 item snap contract. Value types are forward-declared above;
    // defaults are out-of-line in scrollable_pattern.cpp
    // so this widely-included header stays lean. Overridden by List/Grid/WaterFlow properties.
    virtual ScrollSnapStrategy GetScrollSnapStrategy() const;
    // Snap candidates from the currently materialized (visible + cached) items only.
    virtual std::vector<SnapCandidate> BuildItemSnapCandidates(ScrollSnapAlign align);
    // Axis fallback anchor one row beyond the materialized window (details in the .cpp default).
    virtual std::optional<float> GetItemSnapNeighborAnchor(ScrollSnapAlign align, bool towardSmaller);
    // Legal scroll range bounds for clamping item snap targets (R-3/R-7; AC-2.4 leading gap).
    // Live content offset for snap selection and animation origins. Layouts that defer
    // applying motion deltas must include those deltas without changing GetTotalOffset().
    virtual double GetItemSnapCurrentOffset() const
    {
        return GetTotalOffset();
    }
    virtual float GetItemSnapMaxOffset() const;
    virtual float GetItemSnapCalibrationMargin() const
    {
        return 0.0f;
    }

    virtual float GetItemSnapMinOffset() const;
    virtual bool SupportsTrackedItemSnap() const
    {
        return false;
    }
    virtual void BeginTrackedItemSnap(float) {}
    virtual std::vector<SnapCandidate> BuildTrackedItemSnapCandidates(ScrollSnapAlign align);
    // Shared item snap execution for align strategies and custom providers; false = capability off.
    bool StartItemSnapAnimation(const SnapAnimationOptions& snapAnimationOptions);
    void RunPendingItemSnapProvider(const SnapAnimationOptions& snapAnimationOptions);

    virtual bool IsScrollSnap()
    {
        // When setting snap or enablePaging in scroll, the PARENT_FIRST in nestedScroll_ is invalid
        return false;
    }

    virtual bool IsEnablePagingValid()
    {
        return false;
    }

    void SetAnimateCanOverScroll(bool animateCanOverScroll)
    {
        bool isScrollable = !(IsAtBottom() && IsAtTop() && !GetAlwaysEnabled());
        animateCanOverScroll_ = isScrollable && animateCanOverScroll;
    }

    void InitScrollBarGestureEvent();
    void RegisterScrollBarInputEvents(const RefPtr<GestureEventHub>& gestureHub, const RefPtr<InputEventHub>& inputHub);
    void InitScrollBarCallbacks();
    void SetInBarRegionCallback();
    void SetBarCollectTargetCallback();
    void SetInBarRectRegionCallback();

    virtual void InitScrollBarClickEvent();
    void HandleClickEvent();
    void InitScrollBarMouseEvent();
    virtual void ScrollPage(
        bool reverse, bool smooth = false, AccessibilityScrollType scrollType = AccessibilityScrollType::SCROLL_FULL);
    void ScrollPageMultiThread(
        bool reverse, bool smooth = false, AccessibilityScrollType scrollType = AccessibilityScrollType::SCROLL_FULL);
    void PrintOffsetLog(AceLogTag tag, int32_t id, double finalOffset);

    ACE_FORCE_EXPORT void CheckRestartSpring(bool sizeDiminished, bool needNestedScrolling = true);

    Axis GetScrollablePanDirection()
    {
        CHECK_NULL_RETURN(scrollableEvent_, Axis::NONE);
        auto scrollable = scrollableEvent_->GetScrollable();
        CHECK_NULL_RETURN(scrollable, Axis::NONE);
        return scrollable->GetPanDirection();
    }

    void AddEventsFiredInfo(ScrollableEventType eventType);

    void AddScrollableFrameInfo(int32_t scrollSource);

    void GetEdgeEffectDumpInfo();
    void GetEdgeEffectDumpInfo(std::unique_ptr<JsonValue>& json);

    void GetAxisDumpInfo();
    void GetAxisDumpInfo(std::unique_ptr<JsonValue>& json);

    void GetPanDirectionDumpInfo();
    void GetPanDirectionDumpInfo(std::unique_ptr<JsonValue>& json);

    void GetPaintPropertyDumpInfo();
    void GetPaintPropertyDumpInfo(std::unique_ptr<JsonValue>& json);

    virtual void GetEventDumpInfo();
    virtual void GetEventDumpInfo(std::unique_ptr<JsonValue>& json);

    void DumpAdvanceInfo(std::unique_ptr<JsonValue>& json, const ParamConfig& config) override;
    void DumpAdvanceInfo() override;
    void DumpAdvanceInfo(std::unique_ptr<JsonValue>& json) override;

    void SetScrollToSafeAreaHelper(bool isScrollToSafeAreaHelper)
    {
        isScrollToSafeAreaHelper_ = isScrollToSafeAreaHelper;
    }

    bool IsScrollToSafeAreaHelper() const
    {
        return isScrollToSafeAreaHelper_;
    }

    virtual ScrollOffsetAbility GetScrollOffsetAbility(bool isAccessibility = false)
    {
        return { nullptr, Axis::NONE };
    }

    virtual std::function<bool(int32_t)> GetScrollIndexAbility()
    {
        return nullptr;
    }

    void ScrollAtFixedVelocity(float velocity);

    PositionMode GetPositionMode();

    ACE_FORCE_EXPORT void HandleMoveEventInComp(const PointF& point, bool needExpandHotZone = false);
    ACE_FORCE_EXPORT void HandleLeaveHotzoneEvent();
    void SetHotZoneScrollCallback(std::function<void(void)>&& func)
    {
        hotZoneScrollCallback_ = func;
    }

    // Registered by a drag host (List/Grid item drag manager) while an item floats or
    // is dragged. Fired on every real scroll frame, with the scroll source, so the drag
    // host can tell "another finger is scrolling the container" apart from "the content
    // moved for some other reason". The drag edge auto-scroll animator does not come
    // through here, it drives its own hotZoneScrollCallback_.
    void SetDragScrollCallback(std::function<void(int32_t)>&& func)
    {
        dragScrollCallback_ = func;
    }

    void FireDragScrollCallback(int32_t source)
    {
        if (!dragScrollCallback_) {
            return;
        }
        // Copy before invoking: the handler may end the drag session and clear this
        // very slot, which would otherwise destroy the running closure.
        auto callback = dragScrollCallback_;
        callback(source);
    }

    // True when the scroll was driven by a finger on this container: the drag itself,
    // the fling that follows it, or a scroll bar drag. Wheel/crown (SCROLL_FROM_AXIS,
    // SCROLL_FROM_CROWN) and programmatic scrolls (JUMP, LAYOUT, INDEXER, STATUSBAR,
    // ANIMATION_CONTROLLER) are not, and must not be mistaken for "another finger is
    // scrolling".
    static bool IsFingerDrivenScrollSource(int32_t source)
    {
        return source == SCROLL_FROM_UPDATE || source == SCROLL_FROM_ANIMATION ||
            source == SCROLL_FROM_BAR || source == SCROLL_FROM_BAR_FLING;
    }

    void SetIsOverScroll(bool val);
    bool GetIsOverScroll() const;

    void SetScrollBarShape(const ScrollBarShape& shape)
    {
        if (shape == ScrollBarShape::ARC) {
            isRoundScroll_ = true;
        } else {
            isRoundScroll_ = false;
        }
    }

#ifdef SUPPORT_DIGITAL_CROWN
    bool GetCrownEventDragging() const
    {
        CHECK_NULL_RETURN(scrollableEvent_, false);
        auto scrollable = scrollableEvent_->GetScrollable();
        CHECK_NULL_RETURN(scrollable, false);
        return scrollable->GetCrownEventDragging();
    }
#endif

    void OnCollectClickTarget(const OffsetF& coordinateOffset, const GetEventTargetImpl& getEventTargetImpl,
        TouchTestResult& result, const RefPtr<FrameNode>& frameNode,
        ResponseLinkResult& responseLinkResult);

    virtual void SetAccessibilityAction();
    RefPtr<NG::ScrollBarProxy> GetScrollBarProxy() const
    {
        return scrollBarProxy_;
    }

    virtual void OnAttachToMainTree() override;

    void AddNestScrollBarProxy(const WeakPtr<ScrollBarProxy>& scrollBarProxy);

    void SetParentNestedScroll(RefPtr<ScrollablePattern>& parentPattern);

    void SearchAndSetParentNestedScroll(const RefPtr<FrameNode>& node);

    void UnsetParentNestedScroll(RefPtr<ScrollablePattern>& parentPattern);

    void SearchAndUnsetParentNestedScroll(const RefPtr<FrameNode>& node);

    void DeleteNestScrollBarProxy(const WeakPtr<ScrollBarProxy>& scrollBarProxy);

    void SetUseTotalOffset(bool useTotalOffset)
    {
        useTotalOffset_ = useTotalOffset;
    }

    bool GetNestedScrolling() const
    {
        CHECK_NULL_RETURN(scrollableEvent_, false);
        auto scrollable = scrollableEvent_->GetScrollable();
        CHECK_NULL_RETURN(scrollable, false);
        return scrollable->GetNestedScrolling();
    }

    void SetIsScrolling(bool isScrolling)
    {
        isScrolling_ = isScrolling;
    }

    bool IsScrolling() const
    {
        return isScrolling_;
    }

    void OnColorConfigurationUpdate() override;

    virtual SizeF GetChildrenExpandedSize()
    {
        return SizeF();
    }

    SizeF GetViewSizeMinusPadding() const;

    void ScrollEndCallback(bool nestedScroll, float velocity);

    void StopScrollableAndAnimate();

    void SetBackToTop(bool backToTop);

    void ResetBackToTop();

    bool GetBackToTop() const
    {
        return backToTop_;
    }

    void UseDefaultBackToTop(bool useDefaultBackToTop)
    {
        useDefaultBackToTop_ = useDefaultBackToTop;
    }

    // Check if all parent components are active and visible
    bool CheckParentsActive() const;
    // Start back to top animation with performance optimization
    void StartBackToTopAnimation();

    void OnStatusBarClick() override;

    void SetIsAllowMouse(bool enableScrollWithMouse)
    {
        isAllowMouse_ = enableScrollWithMouse;
    }
    virtual bool GetIsAllowMouse() const;
    void ResetIsAllowMouse();
    // Whether the container supports mouse multi-select (e.g. multiSelectable Grid/List).
    virtual bool MultiSelectable() const;
    // Whether the container is in edit mode.
    virtual bool GetEnableEditMode() const;
    // Direction-aware default of enableScrollWithMouse: enabled only when scrolling horizontally.
    // FREE mode (free scroll) containers keep the default disabled.
    virtual bool IsMouseDragScrollDefaultEnabled() const;

    void SetIsAllowKeyboard(bool enableScrollWithKeyboard)
    {
        isAllowKeyboard_ = enableScrollWithKeyboard;
    }
    bool GetIsAllowKeyboard() const
    {
        return isAllowKeyboard_;
    }

    void GetRepeatCountInfo(
        RefPtr<UINode> node, int32_t& repeatDifference, int32_t& firstRepeatCount, int32_t& totalChildCount);

#ifdef SUPPORT_DIGITAL_CROWN
    void SetDigitalCrownSensitivity(CrownSensitivity sensitivity);
    CrownSensitivity GetDigitalCrownSensitivity() const
    {
        return crownSensitivity_;
    }
#endif
    virtual DisplayMode GetDefaultScrollBarDisplayMode() const
    {
        return DisplayMode::AUTO;
    }

    void MarkScrollBarProxyDirty();

    bool ChildPreMeasureHelperEnabled() override
    {
        return true;
    }
    bool PostponedTaskForIgnoreEnabled() override
    {
        return true;
    }

    bool NeedCustomizeSafeAreaPadding() override
    {
        return true;
    }

    PaddingPropertyF CustomizeSafeAreaPadding(PaddingPropertyF safeAreaPadding, bool needRotate) override;

    bool ChildTentativelyLayouted() override
    {
        return true;
    }

    bool AccumulatingTerminateHelper(RectF& adjustingRect, ExpandEdges& totalExpand, bool fromSelf = false,
        LayoutSafeAreaType ignoreType = NG::LAYOUT_SAFE_AREA_TYPE_SYSTEM) override;

    void SetNeedFullSafeArea(bool needFullSafeArea)
    {
        needFullSafeArea_ = needFullSafeArea;
    }

    RefPtr<ScrollBar> GetScrollBar() const;

    bool IsInitialized() const
    {
        return isInitialized_;
    }
    void ProcessScrollOverDrag(double velocity, bool isNestScroller);

    ACE_FORCE_EXPORT static double GetDefaultFriction();

    void SetCanOverScroll(bool val);

    void ContentChangeReport(const RefPtr<FrameNode>& keyNode, uint32_t type = ContentChangeManager::NONE);
    void ContentChangeReport(const RefPtr<FrameNode>& keyNode, uint32_t type, bool reportCapi);
    void ContentChangeOnTouchDown();
    void ContentChangeOnTouchEnd();

    ACE_FORCE_EXPORT void ContentChangeOnScrollStart(const RefPtr<FrameNode>& keyNode);

protected:
    float CalcSpringAdjustOffset(float offset) const;
    void StartFastScrollAnimation(float position);
    virtual void CancelFastScroll();
    const ScrollToIndexTarget* GetFastScrollRequest() const
    {
        return scrollTarget_ && IsFastScrollEnabled(scrollTarget_->fastScrollMode) ? &*scrollTarget_ : nullptr;
    }
    ScrollToIndexTarget* GetFastScrollRequest()
    {
        return scrollTarget_ && IsFastScrollEnabled(scrollTarget_->fastScrollMode) ? &*scrollTarget_ : nullptr;
    }
    std::optional<ScrollToIndexTarget> scrollTarget_;
    bool fastScrollSkip_ = false;
    bool startingFastScroll_ = false;
    uint64_t animationGeneration_ = 0;
    ACE_FORCE_EXPORT void SuggestOpIncGroup(bool flag);
    void OnAttachToFrameNode() override;
    void OnAttachToFrameNodeMultiThread();
    void OnAttachToMainTreeMultiThread();
    void OnDetachFromFrameNode(FrameNode* frameNode) override;
    void OnDetachFromFrameNodeMultiThread(FrameNode* frameNode);
    void OnDetachFromMainTree() override;
    void OnDetachFromMainTreeMultiThread();
    void ContentChangeByDetaching(PipelineContext* pipeline) override;
    void UpdateScrollBarRegion(float offset, float estimatedHeight, Size viewPort, Offset viewOffset);

    EdgeEffect GetEdgeEffect() const;
    void SetEdgeEffect()
    {
        SetEdgeEffect(edgeEffect_);
    }

    virtual void FireOnScroll(float finalOffset, OnScrollEvent& onScroll) const;

    void FireObserverOnTouch(const TouchEventInfo& info);
    void FireObserverOnReachStart();
    void FireObserverOnReachEnd();
    void FireObserverOnScrollStart();
    void FireObserverOnScrollStop();
    void ContentChangeOnScrollUpdate();
    void FireObserverOnDidScroll(float finalOffset);
    void FireObserverOnScrollerAreaChange(float finalOffset);
    float FireObserverOnWillScroll(float offset);

    virtual void OnScrollStop(const OnScrollStopEvent& onScrollStop, const OnScrollStopEvent& onJSFrameNodeScrollStop);
    void FireOnScrollStop(const OnScrollStopEvent& onScrollStop, const OnScrollStopEvent& onJSFrameNodeScrollStop);
    void FireAccessibilityScrollEndEvent();
    void FireObserverOnPanActionEnd(GestureEvent& info);

    float FireOnWillScroll(float offset) const;

    RefPtr<ScrollBarOverlayModifier> GetScrollBarOverlayModifier() const;

    void SetScrollBarOverlayModifier(RefPtr<ScrollBarOverlayModifier> scrollBarOverlayModifier);
    // just for hold ScrollableController
    RefPtr<ScrollableController> positionController_;

    bool scrollStop_ = false;
    bool animateOverScrollStart_ = false;
    bool animateOverScrollEnd_ = false;
    bool animateCanOverScroll_ = false;

    // for onReachStart of the first layout
    bool isInitialized_ = false;

    void Register2DragDropManager();

    void SetScrollOriginChild(const WeakPtr<NestableScrollContainer>& scrollOriginChild)
    {
        scrollOriginChild_ = scrollOriginChild;
    }

    RefPtr<NestableScrollContainer> GetScrollOriginChild()
    {
        return scrollOriginChild_.Upgrade();
    }

    bool GetCanOverScroll() const;
    bool lastCanOverScroll_ = false;
    bool lastScrollFromInjection_ = false;
    // Explicit value of enableScrollWithMouse set by the developer.
    // nullopt means not explicitly set: the direction-aware default applies.
    std::optional<bool> isAllowMouse_;
    bool isAllowKeyboard_ = true;

    void CheckScrollBarOff();

    bool IsBackToTopRunning() const
    {
        return isBackToTopRunning_;
    }

    std::string ParseCommand(const std::string& command, ScrollOnInjectionEventInfo& scrollOnInjectionEventInfo);
    void ReportScroll(bool isJump, ScrollError error, int32_t reportEventId);
    int32_t OnInjectionEventByRatio(const std::string& command);
    ScrollError ScrollByRatio(bool reverse, float ratio);
    void HandleScrollByRatio(bool isScrollByRatio, bool reverse, float ratio, int reportEventId);
    void HandleScrollByOffset(double scrollOffset, int reportEventId);
    void HandleScrollByTargetId(int32_t targetId, int reportEventId);

    virtual int32_t GetFirstIndex() const
    {
        return -1;
    }

    virtual void FillReportOnItemStopParams(std::unique_ptr<JsonValue>& params);
    void ReportOnItemScrollStop(const std::string& event);

    void SetLastScrollFromInjection(bool val)
    {
        lastScrollFromInjection_ = val;
    }

#ifdef SUPPORT_DIGITAL_CROWN
    void SetDigitalCrownEvent();
    CrownSensitivity crownSensitivity_ = CrownSensitivity::MEDIUM;
#endif

#ifndef CROSS_PLATFORM
    void RecordScrollEvent(Recorder::EventType eventType);
#endif

    RefPtr<Animator> GetOrCreateAnimator();
    void StopActiveScrollAnimation();
    uint64_t GetLastVsyncTime() const
    {
        return lastVsyncTime_;
    }
    void UpdateSelectScrollVsync(uint64_t currentVsync);

    float IsInHotZone(const PointF& point, bool needExpandHotZone = false);
    void HotZoneScroll(const float offset);
    void StopHotzoneScroll();

    virtual bool ScrollByArrowKey(KeyCode code, float delta);
    bool HandleArrowKeyScroll(const KeyEvent& keyEventInfo);

private:
    void GetEventDumpInfo(std::unique_ptr<JsonValue>& json, const ParamConfig& config);

    void DumpAdvanceScrollEventInfo(std::unique_ptr<JsonValue>& json, const ParamConfig& config);

    void DumpAdvancePhysicsInfo(std::unique_ptr<JsonValue>& json, const ParamConfig& config);

    void DumpAdvanceDiagnostics(std::unique_ptr<JsonValue>& json);
    void DumpAdvanceDiagnosticsDetails(std::unique_ptr<JsonValue>& json);
    void DumpAdvanceVisualProperties(std::unique_ptr<JsonValue>& json, const ParamConfig& config);
    void DumpAdvanceFunctionalProperties(std::unique_ptr<JsonValue>& json, const ParamConfig& config);


    // Pattern owns target selection; Scrollable owns the animation and queued-task generation.
    std::optional<float> predictedItemSnapTarget_;
    float itemSnapReleaseMotionVelocity_ = 0.0f;
    void RefreshTrackedItemSnap();
    void PrepareItemSnapFling(float motionVelocity, float motionDelta);
    SnapSelectResult SelectItemSnapForAxis(ScrollSnapAlign align, const std::vector<SnapCandidate>& candidates,
        float currentOffset, SnapDirection snapDirection);
    SnapSelectResult SelectItemSnapForAlign(const ScrollSnapStrategy& strategy,
        const SnapAnimationOptions& snapAnimationOptions, std::vector<SnapCandidate>& candidates,
        float currentOffset, bool& forward, bool tracked);

    virtual void OnScrollEndCallback() {};

    // Feat-20260806915268 item snap paths, split out of StartItemSnapAnimation (drag-frame targets).
    bool StartItemSnapWithProvider(const ScrollSnapStrategy& strategy, const SnapAnimationOptions& snapAnimationOptions,
        float currentOffset, float maxOffset, float motionVelocity, float motionDelta);
    bool StartItemSnapWithAlign(const ScrollSnapStrategy& strategy, const SnapAnimationOptions& snapAnimationOptions,
        float currentOffset, float maxOffset, float motionVelocity, float motionDelta);
    float ResolveItemSnapTarget(const ScrollSnapStrategy& strategy, float currentOffset, float minOffset,
        float maxOffset, float velocityPos, float predictDeltaPos);

    void RegisterScrollBarEventTask();
    void RegisterScrollBarOverDragEventTask();
    void RegisterScrollBarMarginCallback();
    std::pair<double, double> GetAutoAdjustAvoidOffset();
    void CalcPaddingAvoidDistByMainAxis(
        const std::unique_ptr<PaddingProperty>& property, std::pair<double, double>& offset, bool isRtl);
    void CalcBorderWidthAvoidDistByMainAxis(
        const std::unique_ptr<BorderWidthProperty>& property, std::pair<double, double>& offset, bool isRtl);
    bool OnScrollPosition(double& offset, int32_t source);
    void ProcessNavBarReactOnStart();
    float ProcessNavBarReactOnUpdate(float offset);
    void ProcessNavBarReactOnEnd();
    void InitSpringOffsetProperty();
    void InitCurveOffsetProperty();
    bool HandleCurveOffsetAnimateOverScroll();
    void OnAnimateFinish();
    void StopAnimation(std::shared_ptr<AnimationUtils::Animation> animation);
    void PauseAnimation(std::shared_ptr<AnimationUtils::Animation> animation);
    void InitOption(AnimationOption& option, float duration, const RefPtr<Curve>& curve);
    float GetScrollDelta(float offset, bool& stopAnimation);
    // Process spring offset animation callback
    void ProcessSpringOffsetCallback(float offset);

    void InitTouchEvent(const RefPtr<GestureEventHub>& gestureHub);
    void RegisterTouchpadInteractionCallback();
    void RegisterWindowStateChangedCallback();
    void OnTouchTestDone(const std::shared_ptr<BaseGestureEvent>& baseGestureEvent,
        const std::list<WeakPtr<NGGestureRecognizer>>& activeRecognizers);
    void OnTouchTestDoneHandlePreventRecognizer(
        const std::list<WeakPtr<NGGestureRecognizer>>& activeRecognizers, bool isHitTestBlock);
    bool IsNeedPreventRecognizer(const RefPtr<NGGestureRecognizer>& recognizer,
        bool isChild, bool isHitTestBlock) const;
    bool IsInComponent(PointF point);

    void UpdateBorderRadius();

    /******************************************************************************
     * NestableScrollContainer implementations
     */
    void HandleExtScroll(float velocity = 0.f);
    ScrollResult HandleScroll(
        float offset, int32_t source, NestedState state = NestedState::GESTURE, float velocity = 0.f) override;
    bool HandleScrollVelocity(float velocity, const RefPtr<NestableScrollContainer>& child = nullptr) override;

    void RemainVelocityToChild(float remainVelocity) override;
    bool NestedScrollOutOfBoundary() override
    {
        return OutBoundaryCallback();
    }
    void UpdateNestedScrollVelocity(float offset, NestedState state);
    float GetNestedScrollVelocity();

    void OnScrollEndRecursive(const std::optional<float>& velocity) override;
    void OnScrollEndRecursiveInner(const std::optional<float>& velocity);
    void OnScrollStartRecursive(WeakPtr<NestableScrollContainer> child, float position, float velocity = 0.f) override;
    void OnScrollStartRecursiveInner(WeakPtr<NestableScrollContainer> child, float position, float velocity = 0.f);
    void OnScrollDragEndRecursive() override;
    void StopScrollAnimation() override;

    ScrollResult HandleScrollParentFirst(float& offset, int32_t source, NestedState state);
    ScrollResult HandleScrollSelfFirst(float& offset, int32_t source, NestedState state);
    ScrollResult HandleScrollSelfOnly(float& offset, int32_t source, NestedState state);
    ScrollResult HandleScrollParallel(float& offset, int32_t source, NestedState state);
    /*
     *  End of NestableScrollContainer implementations
     *******************************************************************************/

    bool HandleOutBoundary(float& offset, int32_t source, NestedState state, ScrollResult& result);
    bool HasEdgeEffect(float offset, bool isWithRefresh = false) const;
    bool CanSpringOverScroll() const;
    bool HandleOverScroll(float velocity);
    bool HandleScrollableOverScroll(float velocity);

    void ExecuteScrollFrameBegin(float& mainDelta, ScrollState state);

    void OnScrollEnd();
    void ProcessSpringEffect(float velocity, bool needRestart = false);
    void SetEdgeEffect(EdgeEffect edgeEffect);
    void SetHandleScrollCallback(const RefPtr<Scrollable>& scrollable);
    void SetHandleExtScrollCallback(const RefPtr<Scrollable>& scrollable);
    void SetOverScrollCallback(const RefPtr<Scrollable>& scrollable);
    void SetIsReverseCallback(const RefPtr<Scrollable>& scrollable);
    void SetIsRefreshScrollCallback(const RefPtr<Scrollable>& scrollable);
    void SetOnScrollStartRec(const RefPtr<Scrollable>& scrollable);
    void SetOnScrollEndRec(const RefPtr<Scrollable>& scrollable);
    void SetScrollEndCallback(const RefPtr<Scrollable>& scrollable);
    void SetRemainVelocityCallback(const RefPtr<Scrollable>& scrollable);
    void SetDragEndCallback(const RefPtr<Scrollable>& scrollable);
    void SetStartSnapAnimationCallback(const RefPtr<Scrollable>& scrollable);
    void SetBackToTopCallback(const RefPtr<Scrollable>& scrollable);
    void SetNeedScrollSnapToSideCallback(const RefPtr<Scrollable>& scrollable);
    void SetDragFRCSceneCallback(const RefPtr<Scrollable>& scrollable);
    void SetOnContinuousSliding(const RefPtr<Scrollable>& scrollable);
    void SetGetSnapTypeCallback(const RefPtr<Scrollable>& scrollable);
    void SetPanActionEndEvent(const RefPtr<Scrollable>& scrollable);
    void SetOnWillStopDraggingCallback(const RefPtr<Scrollable>& scrollable);
    void SetOnWillStartDraggingCallback(const RefPtr<Scrollable>& scrollable);
    void SetOnDidStopDraggingCallback(const RefPtr<Scrollable>& scrollable);
    void SetOnWillStartFlingCallback(const RefPtr<Scrollable>& scrollable);
    void SetOnDidStopFlingCallback(const RefPtr<Scrollable>& scrollable);
    RefPtr<Scrollable> CreateScrollable();

    // Scrollable::UpdateScrollPosition
    bool HandleScrollImpl(float offset, int32_t source);
    void NotifyMoved(bool value);
    void CreateRefreshCoordination();
    float GetVelocity() const;
    bool NeedSplitScroll(OverScrollOffset& overOffsets, int32_t source);
    RefreshCoordinationMode CoordinateWithRefresh(double& offset, int32_t source, bool isAtTop);
    bool CoordinateWithNavigation(double& offset, int32_t source, bool isAtTop);
    void NotifyFRCSceneInfo(const std::string& scene, double velocity, SceneStatus sceneStatus);
    ModalSheetCoordinationMode CoordinateWithSheet(double& offset, int32_t source, bool isAtTop);
    bool NeedCoordinateScrollWithNavigation(double offset, int32_t source, const OverScrollOffset& overOffsets);
    void SetUiDvsyncSwitch(bool on);
    void SetUiDVSyncCommandTime(uint64_t time);
    void SetNestedScrolling(bool nestedScrolling);
    void InitRatio();
    void SetOnHiddenChangeForParent();
    virtual void ResetForExtScroll() {};
    void OnSyncGeometryNode(const DirtySwapConfig& config) override;
    void ReportOnItemStopEvent();

    bool IsArrowKeyMatchAxis(KeyCode code) const;
    int32_t GetMouseScrollRows();
    float CalculateScrollDelta(int32_t rows);

    Axis axis_ = Axis::VERTICAL;
    RefPtr<ScrollableEvent> scrollableEvent_;
    RefPtr<TouchEventImpl> touchEvent_;
    std::unordered_set<int32_t> activeTouchFingerIds_;
    RefPtr<ScrollEdgeEffect> scrollEffect_;
    RefPtr<RefreshCoordination> refreshCoordination_;
    int32_t scrollSource_ = SCROLL_FROM_NONE;
    int32_t scrollSourceForQuery_ = SCROLL_FROM_NONE;
    int32_t lastScrollSource_ = SCROLL_FROM_NONE;
    AccessibilityScrollSource accessibilityScrollSource_ = AccessibilityScrollSource::NONE;
    // scrollBar
    RefPtr<ScrollBar> scrollBar_;
    RefPtr<NG::ScrollBarProxy> scrollBarProxy_;
    std::list<WeakPtr<NG::ScrollBarProxy>> nestScrollBarProxy_;
    RefPtr<ScrollBarOverlayModifier> scrollBarOverlayModifier_;
    float barOffset_ = 0.0f;
    float estimatedHeight_ = 0.0f;
    bool isReactInParentMovement_ = false;
    bool isRefreshInReactive_ = false; // true if Refresh component is ready to receive scroll offset.
    bool isSheetInReactive_ = false;
    bool isCoordEventNeedSpring_ = true;
    bool isScrolling_ = false;
    float scrollBarOutBoundaryExtent_ = 0.f;
    std::optional<float> ratio_;
    double friction_ = -1.0;
    double velocityScale_ = 0.0;
    double maxFlingVelocity_ = MAX_VELOCITY;
    // scroller
    RefPtr<Animator> animator_;
    bool scrollAbort_ = false;
    bool capiScrollDetached_ = false;
    bool capiScrollStartPending_ = false;
    bool capiScrollStarted_ = false;
    bool capiSnapTakeover_ = false;
    bool capiBlockedCommandStart_ = false;
    bool capiBlockedCommandEnd_ = false;
    std::optional<double> capiCommandOffset_;
    uint32_t capiCommandSource_ = 0;
    bool capiScrollToRequest_ = false;
    bool isAnimateOverScroll_ = false;
    bool isScrollToOverAnimation_ = false;
    bool isScrollToSafeAreaHelper_ = true;
    bool inScrollingStatus_ = false;
    bool switchOnStatus_ = false;
    bool canStayOverScroll_ = false;

    float startPercent_ = 0.0f;
    float endPercent_ = 1.0f;
    void UpdateFadeInfo(
        bool isFadingTop, bool isFadingBottom, float fadeFrameSize, const RefPtr<ScrollablePaintMethod>& paint);

    RefPtr<NavDestinationPatternBase> navBarPattern_;
    RefPtr<Pattern> sheetPattern_;
    std::vector<RefPtr<ScrollingListener>> scrollingListener_;

    EdgeEffect edgeEffect_ = EdgeEffect::NONE;
    bool edgeEffectAlwaysEnabled_ = false;
    EffectEdge effectEdge_ = EffectEdge::ALL;

    RefPtr<NodeAnimatablePropertyFloat> springOffsetProperty_;
    RefPtr<NodeAnimatablePropertyFloat> curveOffsetProperty_;
    std::shared_ptr<AnimationUtils::Animation> springAnimation_;
    std::shared_ptr<AnimationUtils::Animation> curveAnimation_;
    uint64_t lastVsyncTime_ = 0;
    bool isAnimationStop_ = true; // graphic animation flag
    bool isBackToTopRunning_ = false;
    float currentVelocity_ = 0.0f;
    float lastPosition_ = 0.0f;
    float finalPosition_ = 0.0f;
    bool useTotalOffset_ = true;
    bool animateToTraceFlag_ = false;
    std::optional<float> extraOffset_;

    RefPtr<Animator> hotzoneAnimator_;
    float lastHonezoneOffsetPct_ = 0.0f;
    RefPtr<BezierVariableVelocityMotion> velocityMotion_;
    RefPtr<VelocityMotion> fixedVelocityMotion_;
    std::function<void(void)> hotZoneScrollCallback_;
    std::function<void(int32_t)> dragScrollCallback_;
    void UnRegister2DragDropManager(FrameNode* frameNode);
    void HandleHotZone(const DragEventType& dragEventType, const RefPtr<NotifyDragEvent>& notifyDragEvent);
    bool isVertical() const;
    void AddHotZoneSenceInterface(SceneStatus scene);
    float GetDVSyncOffset();
    Offset locationInfo_;
    WeakPtr<NestableScrollContainer> scrollOriginChild_;
    float nestedScrollVelocity_ = 0.0f;
    uint64_t nestedScrollTimestamp_ = 0;
    RefPtr<ClickRecognizer> clickRecognizer_;
    RefPtr<InputEvent> mouseEvent_;
    bool isMousePressed_ = false;
    bool preHasFadingEdge_ = false;
    float scrollStartOffset_ = 0.0f;

    bool isRoundScroll_ = false;

    // dump info
    std::list<ScrollableEventsFiredInfo> eventsFiredInfos_;
    std::list<ScrollableFrameInfo> scrollableFrameInfos_;

    bool backToTop_ = false;
    bool useDefaultBackToTop_ = true;
    bool isHitTestBlock_ = false;
    std::queue<std::pair<uint64_t, float>> offsets_;
    bool isExtScroll_ = false;
    bool isNeedCollectOffset_ = false;
    bool needFullSafeArea_ = false;
    ScrollToDirection scrollToDirection_ = ScrollToDirection::NONE;

    // Back to top performance optimization
    // Record the distance to skip during back to top animation
    // Mark whether the skip distance has been applied
    std::optional<float> backToTopSkipDistance_;
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SCROLL_SCROLL_PATTERN_H

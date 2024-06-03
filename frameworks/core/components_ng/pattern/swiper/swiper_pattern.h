/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SWIPER_SWIPER_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SWIPER_SWIPER_PATTERN_H

#include <functional>
#include <optional>
#include <vector>

#include "base/geometry/axis.h"
#include "base/geometry/ng/offset_t.h"
#include "base/memory/referenced.h"
#include "core/components/common/layout/constants.h"
#include "core/components/swiper/swiper_controller.h"
#include "core/components/swiper/swiper_indicator_theme.h"
#include "core/components_ng/base/frame_scene_status.h"
#include "core/components_ng/base/inspector_filter.h"
#include "core/components_ng/event/event_hub.h"
#include "core/components_ng/event/input_event.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/pattern/scrollable/nestable_scroll_container.h"
#include "core/components_ng/pattern/swiper/swiper_accessibility_property.h"
#include "core/components_ng/pattern/swiper/swiper_event_hub.h"
#include "core/components_ng/pattern/swiper/swiper_layout_algorithm.h"
#include "core/components_ng/pattern/swiper/swiper_layout_property.h"
#include "core/components_ng/pattern/swiper/swiper_model.h"
#include "core/components_ng/pattern/swiper/swiper_paint_method.h"
#include "core/components_ng/pattern/swiper/swiper_paint_property.h"
#include "core/components_ng/pattern/swiper/swiper_utils.h"
#include "core/components_ng/pattern/tabs/tab_content_transition_proxy.h"
#include "core/components_v2/inspector/utils.h"

namespace OHOS::Ace::NG {
class SwiperPattern : public NestableScrollContainer {
    DECLARE_ACE_TYPE(SwiperPattern, NestableScrollContainer);

public:
    using CustomContentTransitionPtr = std::shared_ptr<std::function<TabContentAnimatedTransition(int32_t, int32_t)>>;

    SwiperPattern();
    ~SwiperPattern() override = default;

    bool IsAtomicNode() const override
    {
        return false;
    }

    bool ShouldDelayChildPressedState() const override
    {
        return true;
    }

    void RegisterScrollingListener(const RefPtr<ScrollingListener> listener) override;
    void FireAndCleanScrollingListener() override;
    void CleanScrollingListener() override;

    bool UsResRegion() override
    {
        return false;
    }

    RefPtr<LayoutProperty> CreateLayoutProperty() override
    {
        return MakeRefPtr<SwiperLayoutProperty>();
    }

    RefPtr<PaintProperty> CreatePaintProperty() override
    {
        return MakeRefPtr<SwiperPaintProperty>();
    }

    RefPtr<AccessibilityProperty> CreateAccessibilityProperty() override
    {
        return MakeRefPtr<SwiperAccessibilityProperty>();
    }

    RefPtr<LayoutAlgorithm> CreateLayoutAlgorithm() override;

    RefPtr<NodePaintMethod> CreateNodePaintMethod() override
    {
        auto layoutProperty = GetLayoutProperty<SwiperLayoutProperty>();
        CHECK_NULL_RETURN(layoutProperty, nullptr);
        const auto& paddingProperty = layoutProperty->GetPaddingProperty();
        bool needClipPadding = paddingProperty != nullptr;
        bool needPaintFade = !IsLoop() && GetEdgeEffect() == EdgeEffect::FADE && !NearZero(fadeOffset_);
        auto paintMethod = MakeRefPtr<SwiperPaintMethod>(GetDirection(), fadeOffset_);
        paintMethod->SetNeedPaintFade(needPaintFade);
        paintMethod->SetNeedClipPadding(needClipPadding);
        return paintMethod;
    }

    RefPtr<EventHub> CreateEventHub() override
    {
        return MakeRefPtr<SwiperEventHub>();
    }

    void ToJsonValue(std::unique_ptr<JsonValue>& json, const InspectorFilter& filter) const override
    {
        Pattern::ToJsonValue(json, filter);
        json->PutExtAttr("currentIndex", currentIndex_, filter);
        json->PutExtAttr("currentOffset", currentOffset_, filter);
        json->PutExtAttr("uiCastJumpIndex", uiCastJumpIndex_.value_or(-1), filter);

        if (indicatorIsBoolean_) {
            return;
        }

        auto indicatorType = GetIndicatorType();
        if (indicatorType == SwiperIndicatorType::DOT) {
            json->PutExtAttr("indicator", GetDotIndicatorStyle().c_str(), filter);
        } else {
            json->PutExtAttr("indicator", GetDigitIndicatorStyle().c_str(), filter);
        }
    }

    void FromJson(const std::unique_ptr<JsonValue>& json) override
    {
        currentIndex_ = json->GetInt("currentIndex");
        auto currentOffset = json->GetDouble("currentOffset");
        auto jumpIndex = json->GetInt("uiCastJumpIndex");
        if (currentOffset != currentOffset_) {
            auto delta = currentOffset - currentOffset_;
            UpdateCurrentOffset(delta);
        } else if (jumpIndex >= 0) {
            jumpIndex_ = jumpIndex;
            MarkDirtyNodeSelf();
        }
        Pattern::FromJson(json);
    }

    std::string GetDotIndicatorStyle() const
    {
        auto swiperParameters = GetSwiperParameters();
        CHECK_NULL_RETURN(swiperParameters, "");
        auto jsonValue = JsonUtil::Create(true);
        jsonValue->Put("left", swiperParameters_->dimLeft.value_or(0.0_vp).ToString().c_str());
        jsonValue->Put("top", swiperParameters_->dimTop.value_or(0.0_vp).ToString().c_str());
        jsonValue->Put("right", swiperParameters_->dimRight.value_or(0.0_vp).ToString().c_str());
        jsonValue->Put("bottom", swiperParameters_->dimBottom.value_or(0.0_vp).ToString().c_str());
        jsonValue->Put("itemWidth", swiperParameters_->itemWidth.value_or(6.0_vp).ToString().c_str());
        jsonValue->Put("itemHeight", swiperParameters_->itemHeight.value_or(6.0_vp).ToString().c_str());
        jsonValue->Put("selectedItemWidth", swiperParameters_->selectedItemWidth.value_or(6.0_vp).ToString().c_str());
        jsonValue->Put("selectedItemHeight", swiperParameters_->selectedItemHeight.value_or(6.0_vp).ToString().c_str());
        jsonValue->Put("selectedColor",
            swiperParameters_->selectedColorVal.value_or(Color::FromString("#ff007dff")).ColorToString().c_str());
        jsonValue->Put(
            "color", swiperParameters_->colorVal.value_or(Color::FromString("#19182431")).ColorToString().c_str());
        jsonValue->Put("mask", swiperParameters_->maskValue ? "true" : "false");
        return jsonValue->ToString();
    }

    std::string GetDigitIndicatorStyle() const
    {
        auto swiperParameters = GetSwiperDigitalParameters();
        CHECK_NULL_RETURN(swiperParameters, "");
        auto jsonValue = JsonUtil::Create(true);
        auto pipelineContext = PipelineBase::GetCurrentContext();
        CHECK_NULL_RETURN(pipelineContext, "");
        auto swiperIndicatorTheme = pipelineContext->GetTheme<SwiperIndicatorTheme>();
        CHECK_NULL_RETURN(swiperIndicatorTheme, "");
        jsonValue->Put("left", swiperDigitalParameters_->dimLeft.value_or(0.0_vp).ToString().c_str());
        jsonValue->Put("top", swiperDigitalParameters_->dimTop.value_or(0.0_vp).ToString().c_str());
        jsonValue->Put("right", swiperDigitalParameters_->dimRight.value_or(0.0_vp).ToString().c_str());
        jsonValue->Put("bottom", swiperDigitalParameters_->dimBottom.value_or(0.0_vp).ToString().c_str());
        jsonValue->Put("fontSize", swiperDigitalParameters_->fontSize
                                       .value_or(swiperIndicatorTheme->GetDigitalIndicatorTextStyle().GetFontSize())
                                       .ToString()
                                       .c_str());
        jsonValue->Put("fontColor", swiperDigitalParameters_->fontColor
                                        .value_or(swiperIndicatorTheme->GetDigitalIndicatorTextStyle().GetTextColor())
                                        .ColorToString()
                                        .c_str());
        jsonValue->Put("fontWeight",
            V2::ConvertWrapFontWeightToStirng(swiperDigitalParameters_->fontWeight.value_or(FontWeight::NORMAL))
                .c_str());
        jsonValue->Put(
            "selectedFontSize", swiperDigitalParameters_->selectedFontSize
                                    .value_or(swiperIndicatorTheme->GetDigitalIndicatorTextStyle().GetFontSize())
                                    .ToString()
                                    .c_str());
        jsonValue->Put(
            "selectedFontColor", swiperDigitalParameters_->selectedFontColor
                                     .value_or(swiperIndicatorTheme->GetDigitalIndicatorTextStyle().GetTextColor())
                                     .ColorToString()
                                     .c_str());
        jsonValue->Put("selectedFontWeight",
            V2::ConvertWrapFontWeightToStirng(swiperDigitalParameters_->selectedFontWeight.value_or(FontWeight::NORMAL))
                .c_str());
        return jsonValue->ToString();
    }

    int32_t GetCurrentShownIndex() const
    {
        return IsLoop() ? currentIndex_ : GetLoopIndex(currentIndex_);
    }

    RefPtr<SwiperController> GetSwiperController() const
    {
        return swiperController_;
    }

    void SetSwiperController(const RefPtr<SwiperController>& swiperController)
    {
        swiperController_ = swiperController;
        InitSwiperController();
    }

    int32_t GetCurrentFirstIndex() const
    {
        return currentFirstIndex_;
    }

    int32_t GetCurrentIndex()
    {
        return GetLoopIndex(currentIndex_);
    }

    float GetTurnPageRate() const
    {
        return turnPageRate_;
    }

    GestureState GetGestureState()
    {
        auto gestureState = gestureState_;
        if (gestureState_ == GestureState::GESTURE_STATE_RELEASE_LEFT ||
            gestureState_ == GestureState::GESTURE_STATE_RELEASE_RIGHT) {
            gestureState_ = GestureState::GESTURE_STATE_NONE;
        }

        return gestureState;
    }

    TouchBottomTypeLoop GetTouchBottomTypeLoop() const
    {
        return touchBottomType_;
    }

    bool IsIndicatorAnimatorRunning() const
    {
        return indicatorAnimationIsRunning_;
    }

    void SetTurnPageRate(float turnPageRate)
    {
        turnPageRate_ = turnPageRate;
    }

    float GetTouchBottomRate() const
    {
        return touchBottomRate_;
    }

    void SetTouchBottomRate(float touchBottomRate)
    {
        touchBottomRate_ = touchBottomRate;
    }

    RefPtr<Animator> GetController()
    {
        return controller_;
    }

    void SetIndicatorDoingAnimation(bool indicatorDoingAnimation)
    {
        indicatorDoingAnimation_ = indicatorDoingAnimation;
    }

    void UpdateCurrentOffset(float offset);
    /**
     * @brief Checks if the given offset exceeds the bounds of the swiper container and triggers overScroll.
     *
     * @param offset The offset to check.
     * @return True if overScroll is triggered, false otherwise.
     */
    bool CheckOverScroll(float offset);

    /**
     * @brief Applies spring effect to the over-scrolling of the swiper.
     *
     * @param offset The offset of the swiper.
     * @return true if the spring effect is applied successfully, false otherwise.
     */
    bool SpringOverScroll(float offset);

    void CheckMarkDirtyNodeForRenderIndicator(float additionalOffset = 0.0f,
        std::optional<int32_t> nextIndex = std::nullopt);

    int32_t TotalCount() const;

    Axis GetDirection() const;

    FocusPattern GetFocusPattern() const override
    {
        return { FocusType::SCOPE, true };
    }

    ScopeFocusAlgorithm GetScopeFocusAlgorithm() override
    {
        return ScopeFocusAlgorithm(direction_ != Axis::HORIZONTAL, true, ScopeType::OTHERS,
            [wp = WeakClaim(this)](
                FocusStep step, const WeakPtr<FocusHub>& currFocusNode, WeakPtr<FocusHub>& nextFocusNode) {
                auto swiper = wp.Upgrade();
                if (swiper) {
                    nextFocusNode = swiper->GetNextFocusNode(step, currFocusNode);
                }
            });
    }

    void UpdateChangeEvent(ChangeEvent&& event)
    {
        if (!changeEvent_) {
            changeEvent_ = std::make_shared<ChangeEvent>(event);
            auto eventHub = GetEventHub<SwiperEventHub>();
            CHECK_NULL_VOID(eventHub);
            eventHub->AddOnChangeEvent(changeEvent_);
        } else {
            (*changeEvent_).swap(event);
        }
    }

    void UpdateOnChangeEvent(ChangeEvent&& event)
    {
        if (!onIndexChangeEvent_) {
            onIndexChangeEvent_ = std::make_shared<ChangeEvent>(event);
            auto eventHub = GetEventHub<SwiperEventHub>();
            CHECK_NULL_VOID(eventHub);
            eventHub->AddOnChangeEvent(onIndexChangeEvent_);
        } else {
            (*onIndexChangeEvent_).swap(event);
        }
    }

    void UpdateAnimationStartEvent(AnimationStartEvent&& event)
    {
        if (!animationStartEvent_) {
            animationStartEvent_ = std::make_shared<AnimationStartEvent>(event);
            auto eventHub = GetEventHub<SwiperEventHub>();
            CHECK_NULL_VOID(eventHub);
            eventHub->AddAnimationStartEvent(animationStartEvent_);
        } else {
            (*animationStartEvent_).swap(event);
        }
    }

    void UpdateAnimationEndEvent(AnimationEndEvent&& event)
    {
        if (!animationEndEvent_) {
            animationEndEvent_ = std::make_shared<AnimationEndEvent>(event);
            auto eventHub = GetEventHub<SwiperEventHub>();
            CHECK_NULL_VOID(eventHub);
            eventHub->AddAnimationEndEvent(animationEndEvent_);
        } else {
            (*animationEndEvent_).swap(event);
        }
    }

    void SetSwiperParameters(const SwiperParameters& swiperParameters)
    {
        swiperParameters_ = std::make_shared<SwiperParameters>(swiperParameters);
    }

    void SetSwiperDigitalParameters(const SwiperDigitalParameters& swiperDigitalParameters)
    {
        swiperDigitalParameters_ = std::make_shared<SwiperDigitalParameters>(swiperDigitalParameters);
    }

    void ShowNext();
    void ShowPrevious();
    void SwipeTo(int32_t index);
    void ChangeIndex(int32_t index, bool useAnimation);

    void OnVisibleChange(bool isVisible) override;

    int32_t GetStartIndex() const
    {
        return startIndex_;
    }

    int32_t GetEndIndex() const
    {
        return endIndex_;
    }

    bool HasIndicatorNode() const
    {
        return indicatorId_.has_value();
    }

    bool HasLeftButtonNode() const
    {
        return leftButtonId_.has_value();
    }

    bool HasRightButtonNode() const
    {
        return rightButtonId_.has_value();
    }

    int32_t GetIndicatorId()
    {
        if (!indicatorId_.has_value()) {
            indicatorId_ = ElementRegister::GetInstance()->MakeUniqueId();
        }
        return indicatorId_.value();
    }

    int32_t GetLeftButtonId()
    {
        if (!leftButtonId_.has_value()) {
            leftButtonId_ = ElementRegister::GetInstance()->MakeUniqueId();
        }
        return leftButtonId_.value();
    }

    int32_t GetRightButtonId()
    {
        if (!rightButtonId_.has_value()) {
            rightButtonId_ = ElementRegister::GetInstance()->MakeUniqueId();
        }
        return rightButtonId_.value();
    }

    void RemoveIndicatorNode()
    {
        CHECK_NULL_VOID(HasIndicatorNode());
        auto swiperNode = GetHost();
        CHECK_NULL_VOID(swiperNode);
        swiperNode->RemoveChildAtIndex(swiperNode->GetChildIndexById(GetIndicatorId()));
        indicatorId_ = std::nullopt;
    }

    void RemoveLeftButtonNode()
    {
        CHECK_NULL_VOID(HasLeftButtonNode());
        auto swiperNode = GetHost();
        CHECK_NULL_VOID(swiperNode);
        swiperNode->RemoveChildAtIndex(swiperNode->GetChildIndexById(GetLeftButtonId()));
        leftButtonId_ = std::nullopt;
    }

    void RemoveRightButtonNode()
    {
        CHECK_NULL_VOID(HasRightButtonNode());
        auto swiperNode = GetHost();
        CHECK_NULL_VOID(swiperNode);
        swiperNode->RemoveChildAtIndex(swiperNode->GetChildIndexById(GetRightButtonId()));
        rightButtonId_ = std::nullopt;
    }

    SwiperIndicatorType GetIndicatorType() const;

    bool IsIndicatorCustomSize() const
    {
        return IsCustomSize_;
    }

    void SetIsIndicatorCustomSize(bool IsCustomSize)
    {
        IsCustomSize_ = IsCustomSize;
    }

    void SetIndicatorIsBoolean(bool isBoolean)
    {
        indicatorIsBoolean_ = isBoolean;
    }

    bool GetIsAtHotRegion() const
    {
        return isAtHotRegion_;
    }

    bool HasSurfaceChangedCallback()
    {
        return surfaceChangedCallbackId_.has_value();
    }

    void UpdateSurfaceChangedCallbackId(int32_t id)
    {
        surfaceChangedCallbackId_ = id;
    }

    void SetIndicatorLongPress(bool isIndicatorLongPress)
    {
        isIndicatorLongPress_ = isIndicatorLongPress;
    }
    void SetCachedCount(int32_t cachedCount)
    {
        if (cachedCount_.has_value() && cachedCount_.value() != cachedCount) {
            SetLazyLoadFeature(true);
        }
        cachedCount_ = cachedCount;
    }

    void SetFinishCallbackType(FinishCallbackType finishCallbackType)
    {
        finishCallbackType_ = finishCallbackType;
    }

    FinishCallbackType GetFinishCallbackType() const
    {
        return finishCallbackType_;
    }

    void SetStopIndicatorAnimationCb(const std::function<void(bool)>& stopCallback)
    {
        stopIndicatorAnimationFunc_ = std::move(stopCallback);
    }

    std::shared_ptr<SwiperParameters> GetSwiperParameters() const;
    std::shared_ptr<SwiperDigitalParameters> GetSwiperDigitalParameters() const;

    void ArrowHover(bool hoverFlag);
    bool IsLoop() const;
    bool IsEnabled() const;
    void OnWindowShow() override;
    void OnWindowHide() override;
    std::string ProvideRestoreInfo() override;
    void OnRestoreInfo(const std::string& restoreInfo) override;
    bool IsAutoFill() const;
    void OnTouchTestHit(SourceType hitTestType) override;
    void SwipeToWithoutAnimation(int32_t index);
    void StopAutoPlay();
    void StartAutoPlay();
    void StopTranslateAnimation();
    void StopSpringAnimation();
    void DumpAdvanceInfo() override;
    int32_t GetLoopIndex(int32_t originalIndex) const;
    int32_t GetDuration() const;
    RefPtr<Curve> GetCurveIncludeMotion();
    void OnCustomContentTransition(int32_t toIndex);
    void OnCustomAnimationFinish(int32_t fromIndex, int32_t toIndex, bool hasOnChanged);
    void OnSwiperCustomAnimationFinish(std::pair<int32_t, SwiperItemInfo> item);

    void SetCustomAnimationToIndex(int32_t toIndex)
    {
        customAnimationToIndex_ = toIndex;
    }

    std::optional<int32_t> GetCustomAnimationToIndex() const
    {
        return customAnimationToIndex_;
    }

    void SetTabsCustomContentTransition(std::function<TabContentAnimatedTransition(int32_t, int32_t)>&& event)
    {
        onTabsCustomContentTransition_ =
            std::make_shared<std::function<TabContentAnimatedTransition(int32_t, int32_t)>>(event);
    }

    CustomContentTransitionPtr GetTabsCustomContentTransition() const
    {
        return onTabsCustomContentTransition_;
    }

    void SetSwiperCustomContentTransition(SwiperContentAnimatedTransition& transition)
    {
        onSwiperCustomContentTransition_ = std::make_shared<SwiperContentAnimatedTransition>(transition);
    }

    std::shared_ptr<SwiperContentAnimatedTransition> GetSwiperCustomContentTransition() const
    {
        return onSwiperCustomContentTransition_;
    }

    void SetOnContentDidScroll(ContentDidScrollEvent&& onContentDidScroll)
    {
        onContentDidScroll_ = std::make_shared<ContentDidScrollEvent>(onContentDidScroll);
    }

    std::shared_ptr<ContentDidScrollEvent> GetOnContentDidScroll() const
    {
        return onContentDidScroll_;
    }

    void SetSwiperEventCallback(bool disableSwipe);
    void UpdateSwiperPanEvent(bool disableSwipe);
    bool IsUseCustomAnimation() const;
    float GetMotionVelocity()
    {
        return motionVelocity_;
    }

    void SetTabsPaddingAndBorder(const PaddingPropertyF& tabsPaddingAndBorder)
    {
        tabsPaddingAndBorder_ = tabsPaddingAndBorder;
    }

    bool ContentWillChange(int32_t comingIndex);
    bool ContentWillChange(int32_t currentIndex, int32_t comingIndex);
    bool CheckSwiperPanEvent(const GestureEvent& info);
    void InitIndexCanChangeMap()
    {
        indexCanChangeMap_.clear();
    }

    int32_t RealTotalCount() const;
    bool IsSwipeByGroup() const;
    int32_t GetDisplayCount() const;
    int32_t GetCachedCount() const;

    int32_t GetNextValidIndex() const
    {
        return nextValidIndex_;
    }
    void UpdateNextValidIndex();

    void FireWillHideEvent(int32_t willHideIndex) const;
    void FireWillShowEvent(int32_t willShowIndex) const;
    void SetOnHiddenChangeForParent();

    void SetHasTabsAncestor(bool hasTabsAncestor)
    {
        hasTabsAncestor_ = hasTabsAncestor;
    }

    void SetIndicatorInteractive(bool isInteractive)
    {
        isIndicatorInteractive_ = isInteractive;
    }

private:
    void OnModifyDone() override;
    void OnAfterModifyDone() override;
    void OnAttachToFrameNode() override;
    void OnDetachFromFrameNode(FrameNode* node) override;
    void InitSurfaceChangedCallback();
    bool OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config) override;

    // Init pan recognizer to move items when drag update, play translate animation when drag end.
    void InitPanEvent(const RefPtr<GestureEventHub>& gestureHub);
    void AddPanEvent(const RefPtr<GestureEventHub>& gestureHub, GestureEventFunc && actionStart,
        GestureEventFunc && actionUpdate, GestureEventFunc && actionEnd, GestureEventNoParameter && actionCancel);

    // Init touch event, stop animation when touch down.
    void InitTouchEvent(const RefPtr<GestureEventHub>& gestureHub);
    void InitHoverMouseEvent();
    // Init on key event
    void InitOnFocusInternal(const RefPtr<FocusHub>& focusHub);
    void HandleFocusInternal();
    void InitOnKeyEvent(const RefPtr<FocusHub>& focusHub);
    bool OnKeyEvent(const KeyEvent& event);
    void FlushFocus(const RefPtr<FrameNode>& curShowFrame);
    WeakPtr<FocusHub> GetNextFocusNode(FocusStep step, const WeakPtr<FocusHub>& currentFocusNode);

    // Init controller of swiper, controller support showNext, showPrevious and finishAnimation interface.
    void InitSwiperController();

    // Init indicator
    void InitIndicator();
    void InitArrow();

    void HandleDragStart(const GestureEvent& info);
    void HandleDragUpdate(const GestureEvent& info);
    void HandleDragEnd(double dragVelocity);

    void HandleTouchEvent(const TouchEventInfo& info);
    void HandleTouchDown(const TouchLocationInfo& locationInfo);
    void HandleTouchUp();

    void HandleMouseEvent(const MouseInfo& info);
    void PlayTranslateAnimation(
        float startPos, float endPos, int32_t nextIndex, bool restartAutoPlay = false, float velocity = 0.0f);
    void OnTranslateAnimationFinish();
    void PlaySpringAnimation(double dragVelocity);
    void PlayFadeAnimation();

    // use property animation feature
    void PlayPropertyTranslateAnimation(
        float translate, int32_t nextIndex, float velocity = 0.0f, bool stopAutoPlay = false);
    void StopPropertyTranslateAnimation(bool isFinishAnimation, bool isBeforeCreateLayoutWrapper = false);
    void UpdateOffsetAfterPropertyAnimation(float offset);
    void OnPropertyTranslateAnimationFinish(const OffsetF& offset);
    void PlayIndicatorTranslateAnimation(float translate, std::optional<int32_t> nextIndex = std::nullopt);

    // Implement of swiper controller

    void FinishAnimation();
    void StopFadeAnimation();

    bool IsOutOfBoundary(float mainOffset = 0.0f) const;
    bool IsOutOfStart(float mainOffset = 0.0f) const;
    bool IsOutOfEnd(float mainOffset = 0.0f) const;
    bool AutoLinearIsOutOfBoundary(float mainOffset) const;
    float GetDistanceToEdge() const;
    float MainSize() const;
    float GetMainContentSize() const;
    void FireChangeEvent() const;
    void FireAnimationStartEvent(int32_t currentIndex, int32_t nextIndex, const AnimationCallbackInfo& info) const;
    void FireAnimationEndEvent(int32_t currentIndex, const AnimationCallbackInfo& info) const;
    void FireGestureSwipeEvent(int32_t currentIndex, const AnimationCallbackInfo& info) const;
    void FireSwiperCustomAnimationEvent();
    void FireContentDidScrollEvent();
    void HandleSwiperCustomAnimation(float offset);
    void CalculateAndUpdateItemInfo(float offset);
    void UpdateItemInfoInCustomAnimation(int32_t index, float startPos, float endPos);

    float GetItemSpace() const;
    float GetPrevMargin() const;
    float GetNextMargin() const;
    float CalculateVisibleSize() const;
    int32_t CurrentIndex() const;
    int32_t CalculateDisplayCount() const;
    int32_t CalculateCount(
        float contentWidth, float minSize, float margin, float gutter, float swiperPadding = 0.0f) const;
    int32_t GetInterval() const;
    RefPtr<Curve> GetCurve() const;
    EdgeEffect GetEdgeEffect() const;
    bool IsAutoPlay() const;
    bool IsDisableSwipe() const;
    bool IsShowIndicator() const;
    std::pair<int32_t, SwiperItemInfo> GetFirstItemInfoInVisibleArea() const;
    std::pair<int32_t, SwiperItemInfo> GetLastItemInfoInVisibleArea() const;
    std::pair<int32_t, SwiperItemInfo> GetSecondItemInfoInVisibleArea() const;
    void OnIndexChange();
    bool IsOutOfHotRegion(const PointF& dragPoint) const;
    void SaveDotIndicatorProperty(const RefPtr<FrameNode>& indicatorNode);
    void SaveDigitIndicatorProperty(const RefPtr<FrameNode>& indicatorNode);
    void PostTranslateTask(uint32_t delayTime);
    void RegisterVisibleAreaChange();
    bool NeedAutoPlay() const;
    void OnTranslateFinish(int32_t nextIndex, bool restartAutoPlay, bool isFinishAnimation, bool forceStop = false);
    bool IsShowArrow() const;
    void SaveArrowProperty(const RefPtr<FrameNode>& arrowNode);
    RefPtr<FocusHub> GetFocusHubChild(std::string childFrameName);
    WeakPtr<FocusHub> PreviousFocus(const RefPtr<FocusHub>& curFocusNode);
    WeakPtr<FocusHub> NextFocus(const RefPtr<FocusHub>& curFocusNode);
    void SetAccessibilityAction();
    bool NeedStartAutoPlay() const;
    void CheckAndSetArrowHoverState(const PointF& mousePoint);
    RectF GetArrowFrameRect(const int32_t index) const;
    float GetCustomPropertyOffset() const;
    void UpdateAnimationProperty(float velocity);
    void TriggerAnimationEndOnForceStop();
    void TriggerAnimationEndOnSwipeToLeft();
    void TriggerAnimationEndOnSwipeToRight();
    void TriggerEventOnFinish(int32_t nextIndex);
    bool IsVisibleChildrenSizeLessThanSwiper();
    void BeforeCreateLayoutWrapper() override;

    void SetLazyLoadFeature(bool useLazyLoad) const;
    void SetLazyForEachLongPredict(bool useLazyLoad) const;
    void SetLazyLoadIsLoop() const;
    int32_t ComputeNextIndexByVelocity(float velocity, bool onlyDistance = false) const;
    void UpdateCurrentIndex(int32_t index);
    void OnSpringAnimationStart(float velocity);
    void OnSpringAndFadeAnimationFinish();
    void OnFadeAnimationStart();
    int32_t TotalDisPlayCount() const;
    void StopAndResetSpringAnimation();
    void OnLoopChange();
    void StopSpringAnimationAndFlushImmediately();
    void UpdateItemRenderGroup(bool itemRenderGroup);
    void MarkDirtyNodeSelf();
    void ResetAndUpdateIndexOnAnimationEnd(int32_t nextIndex);
    int32_t GetLoopIndex(int32_t index, int32_t childrenSize) const;
    bool IsAutoLinear() const;
    bool AutoLinearAnimationNeedReset(float translate) const;
    void OnAnimationTranslateZero(int32_t nextIndex, bool stopAutoPlay);
    void UpdateDragFRCSceneInfo(float speed, SceneStatus sceneStatus);
    void AdjustCurrentIndexOnSwipePage(int32_t index);
    void TriggerCustomContentTransitionEvent(int32_t fromIndex, int32_t toIndex);
    /**
     * @brief Preprocess drag delta when received from DragUpdate event.
     *
     * Drag offset in Swiper can't go beyond a full page. Apply the restriction through this function.
     *
     * @param delta
     * @param mainSize content length along the main axis.
     * @param deltaSum accumulated delta in the current drag event.
     */
    static void ProcessDelta(float& delta, float mainSize, float deltaSum);

    /**
     * @brief Stops animations when the scroll starts.
     *
     * @param flushImmediately Whether to flush layout immediately.
     */
    void StopAnimationOnScrollStart(bool flushImmediately);
    /**
     * @brief Checks if the animation is currently running.
     *
     * @return true if the animation is running, false otherwise.
     */
    inline bool AnimationRunning() const;

    /**
     *  NestableScrollContainer implementations
     *  ============================================================
     */
    Axis GetAxis() const override
    {
        return GetDirection();
    }

    /**
     * @brief Closes gap to the edge, called before Swiper transfers extra offset to parent/child to ensure that Swiper
     * actually reaches the edge.
     *
     * @param offset The scroll offset from DragUpdate.
     */
    void CloseTheGap(float& offset);

    ScrollResult HandleScroll(
        float offset, int32_t source, NestedState state = NestedState::GESTURE, float velocity = 0.f) override;

    ScrollResult HandleScrollSelfFirst(float offset, int32_t source, NestedState state, float velocity = 0.f);

    ScrollResult HandleScrollParentFirst(float offset, int32_t source, NestedState state, float velocity = 0.f);

    bool HandleScrollVelocity(float velocity) override;

    void OnScrollStartRecursive(float position, float velocity = 0.f) override;
    void OnScrollEndRecursive(const std::optional<float>& velocity) override;

    /**
     * @brief Notifies the parent component that the scroll has started at the specified position.
     *
     * @param position The position where the scroll has started.
     */
    void NotifyParentScrollStart(float position);
    /**
     * @brief Notifies the parent NestableScrollContainer that the scroll has ended.
     */
    void NotifyParentScrollEnd();

    inline bool ChildFirst(NestedState state);
    void HandleTouchBottomLoop();
    void CalculateGestureState(float additionalOffset, float currentTurnPageRate, int32_t preFirstIndex);
    void StopIndicatorAnimation(bool ifImmediately = false);
    RefPtr<FrameNode> GetCurrentFrameNode(int32_t currentIndex) const;
    bool FadeOverScroll(float offset);
    int32_t ComputeSwipePageNextIndex(float velocity, bool onlyDistance = false) const;
    int32_t ComputeNextIndexInSinglePage(float velocity, bool onlyDistance) const;
    int32_t ComputePageIndex(int32_t index) const;
    void UpdateIndexOnAnimationStop();
    void UpdateIndexOnSwipePageStop(int32_t pauseTargetIndex);
    void AdjustCurrentFocusIndex();
    bool IsContentFocused();

    int32_t CheckTargetIndex(int32_t targetIndex, bool isForceBackward = false);

    void PreloadItems(const std::set<int32_t>& indexSet);
    void DoPreloadItems(const std::set<int32_t>& indexSet, int32_t errorCode);
    void FirePreloadFinishEvent(int32_t errorCode);
    // capture node start
    void InitCapture();
    int32_t GetLeftCaptureId()
    {
        if (!leftCaptureId_.has_value()) {
            leftCaptureId_ = ElementRegister::GetInstance()->MakeUniqueId();
        }
        return leftCaptureId_.value();
    }
    int32_t GetRightCaptureId()
    {
        if (!rightCaptureId_.has_value()) {
            rightCaptureId_ = ElementRegister::GetInstance()->MakeUniqueId();
        }
        return rightCaptureId_.value();
    }
    void RemoveAllCaptureNode()
    {
        auto swiperNode = GetHost();
        CHECK_NULL_VOID(swiperNode);
        swiperNode->RemoveChildAtIndex(swiperNode->GetChildIndexById(GetLeftCaptureId()));
        leftCaptureId_ = std::nullopt;
        swiperNode->RemoveChildAtIndex(swiperNode->GetChildIndexById(GetRightCaptureId()));
        rightCaptureId_ = std::nullopt;
    }
    RefPtr<FrameNode> GetLeftCaptureNode()
    {
        auto swiperNode = GetHost();
        CHECK_NULL_RETURN(swiperNode, nullptr);
        return DynamicCast<FrameNode>(swiperNode->GetChildAtIndex(swiperNode->GetChildIndexById(GetLeftCaptureId())));
    }
    RefPtr<FrameNode> GetRightCaptureNode()
    {
        auto swiperNode = GetHost();
        CHECK_NULL_RETURN(swiperNode, nullptr);
        return DynamicCast<FrameNode>(swiperNode->GetChildAtIndex(swiperNode->GetChildIndexById(GetRightCaptureId())));
    }
    bool IsCaptureNodeValid()
    {
        return hasCachedCapture_ && GetLeftCaptureNode() && GetRightCaptureNode();
    }
    void UpdateTargetCapture(bool forceUpdate);
    void CreateCaptureCallback(int32_t targetIndex, int32_t captureId, bool forceUpdate);
    void UpdateCaptureSource(std::shared_ptr<Media::PixelMap> pixelMap, int32_t captureId, int32_t targetIndex);

    bool SupportSwiperCustomAnimation()
    {
        auto swiperLayoutProperty = GetLayoutProperty<SwiperLayoutProperty>();
        return (onSwiperCustomContentTransition_ || onContentDidScroll_) &&
            !hasCachedCapture_ && SwiperUtils::IsStretch(swiperLayoutProperty);
    }

    bool NeedStartNewAnimation(const OffsetF& offset) const;
    void ResetOnForceMeasure();
    void UpdateTabBarIndicatorCurve();
    bool CheckDragOutOfBoundary(double dragVelocity);
    void UpdateCurrentFocus();

    void CreateSpringProperty();

    std::optional<RefPtr<UINode>> FindLazyForEachNode(RefPtr<UINode> baseNode, bool isSelfNode = true) const;

    RefPtr<PanEvent> panEvent_;
    RefPtr<TouchEventImpl> touchEvent_;
    RefPtr<InputEvent> hoverEvent_;

    // Control translate animation when drag end.
    RefPtr<Animator> controller_;

    // Control spring animation when drag beyond boundary and drag end.
    std::shared_ptr<AnimationUtils::Animation> springAnimation_;

    // Control fade animation when drag beyond boundary and drag end.
    std::shared_ptr<AnimationUtils::Animation> fadeAnimation_;

    // Control translate animation for indicator.
    std::shared_ptr<AnimationUtils::Animation> indicatorAnimation_;

    std::shared_ptr<AnimationUtils::Animation> translateAnimation_;

    bool indicatorAnimationIsRunning_ = false;
    bool translateAnimationIsRunning_ = false;

    // stop indicator animation callback
    std::function<void(bool)> stopIndicatorAnimationFunc_;

    RefPtr<SwiperController> swiperController_;
    RefPtr<InputEvent> mouseEvent_;

    bool isLastIndicatorFocused_ = false;
    int32_t startIndex_ = 0;
    int32_t endIndex_ = 0;
    int32_t currentIndex_ = 0;
    int32_t oldIndex_ = 0;
    int32_t nextIndex_ = 0;

    PanDirection panDirection_;

    float currentOffset_ = 0.0f;
    float fadeOffset_ = 0.0f;
    float turnPageRate_ = 0.0f;
    float translateAnimationEndPos_ = 0.0f;
    GestureState gestureState_ = GestureState::GESTURE_STATE_INIT;
    TouchBottomTypeLoop touchBottomType_ = TouchBottomTypeLoop::TOUCH_BOTTOM_TYPE_LOOP_NONE;
    float touchBottomRate_ = 1.0f;
    float currentIndexOffset_ = 0.0f;
    int32_t gestureSwipeIndex_ = 0;
    int32_t currentFirstIndex_ = 0;
    int32_t nextValidIndex_ = 0;
    int32_t currentFocusIndex_ = 0;

    bool moveDirection_ = false;
    bool indicatorDoingAnimation_ = false;
    bool isInit_ = true;
    bool hasVisibleChangeRegistered_ = false;
    bool isVisible_ = true;
    bool isVisibleArea_ = true;
    bool isWindowShow_ = true;
    bool IsCustomSize_ = false;
    bool indicatorIsBoolean_ = true;
    bool isAtHotRegion_ = false;
    bool isDragging_ = false;
    bool needTurn_ = false;
    bool isParentHiddenChange_ = false;
    /**
     * @brief Indicates whether the child NestableScrollContainer is currently scrolling and affecting Swiper.
     */
    bool childScrolling_ = false;
    bool isTouchDown_ = false;
    std::optional<bool> preLoop_;

    Axis direction_ = Axis::HORIZONTAL;

    ChangeEventPtr changeEvent_;
    ChangeEventPtr onIndexChangeEvent_;
    AnimationStartEventPtr animationStartEvent_;
    AnimationEndEventPtr animationEndEvent_;

    mutable std::shared_ptr<SwiperParameters> swiperParameters_;
    mutable std::shared_ptr<SwiperDigitalParameters> swiperDigitalParameters_;

    WeakPtr<FrameNode> lastWeakShowNode_;

    CancelableCallback<void()> translateTask_;

    std::optional<int32_t> indicatorId_;
    std::optional<int32_t> leftButtonId_;
    std::optional<int32_t> rightButtonId_;
    std::optional<int32_t> leftCaptureId_;
    std::optional<int32_t> rightCaptureId_;
    std::optional<SwiperIndicatorType> lastSwiperIndicatorType_;

    float startMainPos_ = 0.0f;
    float endMainPos_ = 0.0f;
    float contentMainSize_ = 0.0f;
    float contentCrossSize_ = 0.0f;
    bool crossMatchChild_ = false;

    std::optional<int32_t> uiCastJumpIndex_;
    std::optional<int32_t> jumpIndex_;
    std::optional<int32_t> targetIndex_;
    std::optional<int32_t> preTargetIndex_;
    std::optional<int32_t> pauseTargetIndex_;
    std::optional<int32_t> oldChildrenSize_;
    std::optional<float> placeItemWidth_;
    float currentDelta_ = 0.0f;
    // cumulated delta in a single drag event
    float mainDeltaSum_ = 0.0f;
    SwiperLayoutAlgorithm::PositionMap itemPosition_;
    std::optional<float> velocity_;
    float motionVelocity_ = 0.0f;
    bool isFinishAnimation_ = false;
    bool mainSizeIsMeasured_ = false;
    bool isNeedResetPrevMarginAndNextMargin_ = false;
    bool usePropertyAnimation_ = false;
    bool springAnimationIsRunning_ = false;
    bool isTouchDownSpringAnimation_ = false;
    int32_t propertyAnimationIndex_ = -1;
    bool isUserFinish_ = true;
    bool isVoluntarilyClear_ = false;
    bool isIndicatorLongPress_ = false;
    bool stopIndicatorAnimation_ = true;
    bool isTouchPad_ = false;
    bool fadeAnimationIsRunning_ = false;
    bool autoLinearReachBoundary = false;
    bool needAdjustIndex_ = false;
    bool hasTabsAncestor_ = false;
    bool isIndicatorInteractive_ = true;

    std::optional<int32_t> cachedCount_;

    std::optional<int32_t> surfaceChangedCallbackId_;
    SwiperLayoutAlgorithm::PositionMap itemPositionInAnimation_;

    WindowSizeChangeReason windowSizeChangeReason_ = WindowSizeChangeReason::UNDEFINED;
    std::vector<RefPtr<ScrollingListener>> scrollingListener_;
    FinishCallbackType finishCallbackType_ = FinishCallbackType::REMOVED;

    CustomContentTransitionPtr onTabsCustomContentTransition_;
    std::shared_ptr<SwiperContentAnimatedTransition> onSwiperCustomContentTransition_;
    std::shared_ptr<ContentDidScrollEvent> onContentDidScroll_;
    std::set<int32_t> indexsInAnimation_;
    std::set<int32_t> needUnmountIndexs_;
    std::optional<int32_t> customAnimationToIndex_;
    RefPtr<TabContentTransitionProxy> currentProxyInAnimation_;
    PaddingPropertyF tabsPaddingAndBorder_;
    std::map<int32_t, bool> indexCanChangeMap_;
    // capture
    std::optional<int32_t> leftCaptureIndex_;
    std::optional<int32_t> rightCaptureIndex_;
    bool hasCachedCapture_ = false;
    bool isCaptureReverse_ = false;
    OffsetF captureFinalOffset_;
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SWIPER_SWIPER_PATTERN_H

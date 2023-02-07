/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "core/components_ng/pattern/tabs/tab_bar_pattern.h"

#include "base/geometry/axis.h"
#include "base/geometry/dimension.h"
#include "base/geometry/ng/offset_t.h"
#include "base/geometry/ng/size_t.h"
#include "base/memory/ace_type.h"
#include "base/utils/utils.h"
#include "core/components/common/layout/constants.h"
#include "core/components/scroll/scrollable.h"
#include "core/components/tab_bar/tab_theme.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/swiper/swiper_pattern.h"
#include "core/components_ng/pattern/text/text_layout_property.h"
#include "core/components_ng/property/property.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {

void TabBarPattern::OnAttachToFrameNode()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->GetRenderContext()->SetClipToFrame(true);
}

void TabBarPattern::InitClick(const RefPtr<GestureEventHub>& gestureHub)
{
    if (clickEvent_) {
        return;
    }
    auto clickCallback = [weak = WeakClaim(this)](GestureEvent& info) {
        auto tabBar = weak.Upgrade();
        if (tabBar) {
            tabBar->HandleClick(info);
        }
    };
    clickEvent_ = AceType::MakeRefPtr<ClickEvent>(std::move(clickCallback));
    gestureHub->AddClickEvent(clickEvent_);
}

void TabBarPattern::InitScrollable(const RefPtr<GestureEventHub>& gestureHub)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto layoutProperty = host->GetLayoutProperty<TabBarLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);
    auto axis = layoutProperty->GetAxis().value_or(Axis::HORIZONTAL);
    if (axis_ == axis && scrollableEvent_) {
        LOGD("Direction not changed, need't resister scroll event again.");
        return;
    }

    axis_ = axis;
    auto task = [weak = WeakClaim(this)](double offset, int32_t source) {
        if (source != SCROLL_FROM_START) {
            auto pattern = weak.Upgrade();
            if (pattern) {
                pattern->UpdateCurrentOffset(static_cast<float>(offset));
            }
        }
        return true;
    };

    if (scrollableEvent_) {
        gestureHub->RemoveScrollableEvent(scrollableEvent_);
    }

    auto callback = [weak = WeakClaim(this)]() {
        auto tabBarPattern = weak.Upgrade();
        CHECK_NULL_VOID(tabBarPattern);
        auto scrollable = tabBarPattern->scrollableEvent_->GetScrollable();
        if (scrollable) {
            scrollable->StopScrollable();
        }
    };

    swiperController_->SetTabBarFinishCallback(std::move(callback));

    scrollableEvent_ = MakeRefPtr<ScrollableEvent>(axis);
    scrollableEvent_->SetScrollPositionCallback(std::move(task));
    gestureHub->AddScrollableEvent(scrollableEvent_);
}

void TabBarPattern::InitTouch(const RefPtr<GestureEventHub>& gestureHub)
{
    if (touchEvent_) {
        return;
    }
    auto touchCallback = [weak = WeakClaim(this)](const TouchEventInfo& info) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandleTouchEvent(info.GetTouches().front());
    };
    touchEvent_ = MakeRefPtr<TouchEventImpl>(std::move(touchCallback));
    gestureHub->AddTouchEvent(touchEvent_);
}

void TabBarPattern::InitHoverEvent()
{
    if (hoverEvent_) {
        return;
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto eventHub = GetHost()->GetEventHub<EventHub>();
    auto inputHub = eventHub->GetOrCreateInputEventHub();

    auto hoverTask = [weak = WeakClaim(this)](bool isHover) {
        auto pattern = weak.Upgrade();
        if (pattern) {
            pattern->HandleHoverEvent(isHover);
        }
    };
    hoverEvent_ = MakeRefPtr<InputEvent>(std::move(hoverTask));
    inputHub->AddOnHoverEvent(hoverEvent_);
}

void TabBarPattern::InitMouseEvent()
{
    if (mouseEvent_) {
        return;
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto eventHub = GetHost()->GetEventHub<EventHub>();
    auto inputHub = eventHub->GetOrCreateInputEventHub();
    auto mouseTask = [weak = WeakClaim(this)](const MouseInfo& info) {
        auto pattern = weak.Upgrade();
        if (pattern) {
            pattern->HandleMouseEvent(info);
        }
    };
    mouseEvent_ = MakeRefPtr<InputEvent>(std::move(mouseTask));
    inputHub->AddOnMouseEvent(mouseEvent_);
}

void TabBarPattern::HandleMouseEvent(const MouseInfo& info)
{
    if (IsContainsBuilder()) {
        return;
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto totalCount = host->TotalChildCount();
    auto index = CalculateSelectedIndex(info.GetLocalLocation());
    if (index < 0 || index >= totalCount) {
        if (hoverIndex_.has_value() && !touchingIndex_.has_value()) {
            HandleMoveAway(hoverIndex_.value());
        }
        hoverIndex_.reset();
        return;
    }
    auto mouseAction = info.GetAction();
    if (mouseAction == MouseAction::MOVE || mouseAction == MouseAction::WINDOW_ENTER) {
        if (touchingIndex_.has_value()) {
            hoverIndex_ = index;
            return;
        }
        if (!hoverIndex_.has_value()) {
            HandleHoverOnEvent(index);
            hoverIndex_ = index;
            return;
        }
        if (hoverIndex_.value() != index) {
            HandleMoveAway(hoverIndex_.value());
            HandleHoverOnEvent(index);
            hoverIndex_ = index;
            return;
        }
        return;
    }
    if (mouseAction == MouseAction::WINDOW_LEAVE) {
        HandleMoveAway(hoverIndex_.value());
        return;
    }
}

void TabBarPattern::HandleHoverEvent(bool isHover)
{
    if (IsContainsBuilder()) {
        return;
    }
    isHover_ = isHover;
    if (!isHover_ && hoverIndex_.has_value()) {
        if (!touchingIndex_.has_value()) {
            HandleMoveAway(hoverIndex_.value());
        }
        hoverIndex_.reset();
    }
}

void TabBarPattern::HandleHoverOnEvent(int32_t index)
{
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto tabTheme = pipelineContext->GetTheme<TabTheme>();
    CHECK_NULL_VOID(tabTheme);
    PlayPressAnimation(index, tabTheme->GetSubTabBarHoverColor(), AnimationType::HOVER);
}

void TabBarPattern::HandleMoveAway(int32_t index)
{
    PlayPressAnimation(index, Color::TRANSPARENT, AnimationType::HOVER);
}

void TabBarPattern::InitOnKeyEvent(const RefPtr<FocusHub>& focusHub)
{
    auto onKeyEvent = [wp = WeakClaim(this)](const KeyEvent& event) -> bool {
        auto pattern = wp.Upgrade();
        if (pattern) {
            return pattern->OnKeyEvent(event);
        }
        return false;
    };
    focusHub->SetOnKeyEventInternal(std::move(onKeyEvent));

    auto getInnerPaintRectCallback = [wp = WeakClaim(this)](RoundRect& paintRect) {
        auto pattern = wp.Upgrade();
        if (pattern) {
            pattern->GetInnerFocusPaintRect(paintRect);
        }
    };
    focusHub->SetInnerFocusPaintRectCallback(getInnerPaintRectCallback);
}

bool TabBarPattern::OnKeyEvent(const KeyEvent& event)
{
    if (event.action != KeyAction::DOWN) {
        return false;
    }
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    auto tabBarLayoutProperty = GetLayoutProperty<TabBarLayoutProperty>();
    auto indicator = tabBarLayoutProperty->GetIndicatorValue(0);

    if (event.code == (tabBarLayoutProperty->GetAxisValue(Axis::HORIZONTAL) == Axis::HORIZONTAL
                              ? KeyCode::KEY_DPAD_LEFT
                              : KeyCode::KEY_DPAD_UP) ||
        event.IsShiftWith(KeyCode::KEY_TAB)) {
        if (indicator <= 0) {
            return false;
        }
        indicator -= 1;
        FocusIndexChange(indicator);
        return true;
    }
    if (event.code == (tabBarLayoutProperty->GetAxisValue(Axis::HORIZONTAL) == Axis::HORIZONTAL
                              ? KeyCode::KEY_DPAD_RIGHT
                              : KeyCode::KEY_DPAD_DOWN) ||
        event.code == KeyCode::KEY_TAB) {
        if (indicator >= host->TotalChildCount() - 1) {
            return false;
        }
        indicator += 1;
        FocusIndexChange(indicator);
        return true;
    }
    return false;
}

void TabBarPattern::FocusIndexChange(int32_t index)
{
    auto tabBarLayoutProperty = GetLayoutProperty<TabBarLayoutProperty>();
    if (animationDuration_.has_value()) {
        swiperController_->SwipeTo(index);
    } else {
        swiperController_->SwipeToWithoutAnimation(index);
    }
    tabBarLayoutProperty->UpdateIndicator(index);
    PaintFocusState();
}

void TabBarPattern::GetInnerFocusPaintRect(RoundRect& paintRect)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto tabBarLayoutProperty = GetLayoutProperty<TabBarLayoutProperty>();
    CHECK_NULL_VOID(tabBarLayoutProperty);
    auto indicator = tabBarLayoutProperty->GetIndicatorValue(0);
    auto childNode = AceType::DynamicCast<FrameNode>(host->GetChildAtIndex(indicator));
    CHECK_NULL_VOID(childNode);
    auto renderContext = childNode->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    auto columnPaintRect = renderContext->GetPaintRectWithoutTransform();
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto tabTheme = pipeline->GetTheme<TabTheme>();
    CHECK_NULL_VOID(tabTheme);
    auto radius = tabTheme->GetFocusIndicatorRadius();
    auto outLineWidth = tabTheme->GetActiveIndicatorWidth();
    columnPaintRect.SetOffset(OffsetF((columnPaintRect.GetOffset().GetX() + outLineWidth.ConvertToPx() / 2),
        (columnPaintRect.GetOffset().GetY() + outLineWidth.ConvertToPx() / 2)));
    columnPaintRect.SetSize(SizeF((columnPaintRect.GetSize().Width() - outLineWidth.ConvertToPx()),
        (columnPaintRect.GetSize().Height() - outLineWidth.ConvertToPx())));

    paintRect.SetRect(columnPaintRect);
    paintRect.SetCornerRadius(RoundRect::CornerPos::TOP_LEFT_POS, static_cast<RSScalar>(radius.ConvertToPx()),
        static_cast<RSScalar>(radius.ConvertToPx()));
    paintRect.SetCornerRadius(RoundRect::CornerPos::TOP_RIGHT_POS, static_cast<RSScalar>(radius.ConvertToPx()),
        static_cast<RSScalar>(radius.ConvertToPx()));
    paintRect.SetCornerRadius(RoundRect::CornerPos::BOTTOM_LEFT_POS, static_cast<RSScalar>(radius.ConvertToPx()),
        static_cast<RSScalar>(radius.ConvertToPx()));
    paintRect.SetCornerRadius(RoundRect::CornerPos::BOTTOM_RIGHT_POS, static_cast<RSScalar>(radius.ConvertToPx()),
        static_cast<RSScalar>(radius.ConvertToPx()));
}

void TabBarPattern::PaintFocusState()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);

    RoundRect focusRect;
    GetInnerFocusPaintRect(focusRect);

    auto focusHub = host->GetFocusHub();
    CHECK_NULL_VOID(focusHub);
    focusHub->PaintInnerFocusState(focusRect);

    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
}

void TabBarPattern::OnModifyDone()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto hub = host->GetEventHub<EventHub>();
    CHECK_NULL_VOID(hub);
    auto gestureHub = hub->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(gestureHub);

    InitClick(gestureHub);
    auto layoutProperty = host->GetLayoutProperty<TabBarLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);
    if (layoutProperty->GetTabBarModeValue(TabBarMode::FIXED) == TabBarMode::SCROLLABLE) {
        InitScrollable(gestureHub);
    }
    InitTouch(gestureHub);
    InitHoverEvent();
    InitMouseEvent();
    auto focusHub = host->GetFocusHub();
    CHECK_NULL_VOID(focusHub);
    InitOnKeyEvent(focusHub);

    auto removeEventCallback = [weak = WeakClaim(this)]() {
        auto tabBarPattern = weak.Upgrade();
        CHECK_NULL_VOID(tabBarPattern);
        auto host = tabBarPattern->GetHost();
        CHECK_NULL_VOID(host);
        auto hub = host->GetEventHub<EventHub>();
        CHECK_NULL_VOID(hub);
        auto gestureHub = hub->GetOrCreateGestureEventHub();
        CHECK_NULL_VOID(gestureHub);
        auto layoutProperty = host->GetLayoutProperty<TabBarLayoutProperty>();
        CHECK_NULL_VOID(layoutProperty);
        gestureHub->RemoveClickEvent(tabBarPattern->clickEvent_);
        if (layoutProperty->GetTabBarModeValue(TabBarMode::FIXED) == TabBarMode::SCROLLABLE) {
            gestureHub->RemoveScrollableEvent(tabBarPattern->scrollableEvent_);
        }
        gestureHub->RemoveTouchEvent(tabBarPattern->touchEvent_);
    };
    swiperController_->SetRemoveTabBarEventCallback(std::move(removeEventCallback));

    auto addEventCallback = [weak = WeakClaim(this)]() {
        auto tabBarPattern = weak.Upgrade();
        CHECK_NULL_VOID(tabBarPattern);
        auto host = tabBarPattern->GetHost();
        CHECK_NULL_VOID(host);
        auto hub = host->GetEventHub<EventHub>();
        CHECK_NULL_VOID(hub);
        auto gestureHub = hub->GetOrCreateGestureEventHub();
        CHECK_NULL_VOID(gestureHub);
        auto layoutProperty = host->GetLayoutProperty<TabBarLayoutProperty>();
        CHECK_NULL_VOID(layoutProperty);
        gestureHub->AddClickEvent(tabBarPattern->clickEvent_);
        if (layoutProperty->GetTabBarModeValue(TabBarMode::FIXED) == TabBarMode::SCROLLABLE) {
            gestureHub->AddScrollableEvent(tabBarPattern->scrollableEvent_);
        }
        gestureHub->AddTouchEvent(tabBarPattern->touchEvent_);
    };
    swiperController_->SetAddTabBarEventCallback(std::move(addEventCallback));
}

bool TabBarPattern::OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config)
{
    if (config.skipMeasure && config.skipLayout) {
        return false;
    }
    auto layoutAlgorithmWrapper = DynamicCast<LayoutAlgorithmWrapper>(dirty->GetLayoutAlgorithm());
    CHECK_NULL_RETURN(layoutAlgorithmWrapper, false);
    auto tabBarLayoutAlgorithm = DynamicCast<TabBarLayoutAlgorithm>(layoutAlgorithmWrapper->GetLayoutAlgorithm());
    CHECK_NULL_RETURN(tabBarLayoutAlgorithm, false);
    tabItemOffsets_ = tabBarLayoutAlgorithm->GetTabItemOffset();
    currentOffset_ = tabBarLayoutAlgorithm->GetCurrentOffset();

    childrenMainSize_ = tabBarLayoutAlgorithm->GetChildrenMainSize();
    indicator_ = tabBarLayoutAlgorithm->GetIndicator();
    auto layoutProperty = DynamicCast<TabBarLayoutProperty>(dirty->GetLayoutProperty());
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    auto tabsFrameNode = AceType::DynamicCast<FrameNode>(host->GetParent());
    CHECK_NULL_RETURN(tabsFrameNode, false);
    auto swiperFrameNode = AceType::DynamicCast<FrameNode>(tabsFrameNode->GetChildren().back());
    CHECK_NULL_RETURN(swiperFrameNode, false);
    auto swiperPattern = swiperFrameNode->GetPattern<SwiperPattern>();
    CHECK_NULL_RETURN(swiperPattern, false);
    int32_t indicator = swiperPattern->GetCurrentIndex();
    int32_t totalCount = swiperPattern->TotalCount();
    if (indicator > totalCount - 1 || indicator < 0) {
        indicator = 0;
    }
    UpdateIndicator(indicator);
    return false;
}

void TabBarPattern::HandleClick(const GestureEvent& info)
{
    LOGI("Click event x is %{public}lf", info.GetLocalLocation().GetX());
    if (tabItemOffsets_.empty()) {
        LOGW("tabItemOffsets is empty");
        return;
    }

    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto layoutProperty = host->GetLayoutProperty<TabBarLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);
    auto totalCount = host->TotalChildCount();

    auto index = CalculateSelectedIndex(info.GetLocalLocation());
    if (index >= 0 && index < totalCount && swiperController_) {
        if (tabBarStyle_ == TabBarStyle::SUBTABBATSTYLE && layoutProperty->GetAxis() == Axis::HORIZONTAL) {
            auto host = GetHost();
            CHECK_NULL_VOID(host);
            auto tabsFrameNode = AceType::DynamicCast<FrameNode>(host->GetParent());
            CHECK_NULL_VOID(tabsFrameNode);
            auto swiperFrameNode = AceType::DynamicCast<FrameNode>(tabsFrameNode->GetChildren().back());
            CHECK_NULL_VOID(swiperFrameNode);
            auto swiperPattern = swiperFrameNode->GetPattern<SwiperPattern>();
            CHECK_NULL_VOID(swiperPattern);
            int32_t indicator = swiperPattern->GetCurrentIndex();
            if (std::abs(indicator - index) > 1) {
                swiperController_->SwipeToWithoutAnimation(index);
            } else {
                swiperController_->SwipeTo(index);
            }
            auto originalPaintRect = layoutProperty->GetIndicatorRect(indicator);
            auto targetPaintRect = layoutProperty->GetIndicatorRect(index);
            auto paintProperty = host->GetPaintProperty<TabBarPaintProperty>();
            CHECK_NULL_VOID(paintProperty);
            paintProperty->UpdateIndicator(targetPaintRect);
            PlayTranslateAnimation(originalPaintRect.GetX(), targetPaintRect.GetX());
            layoutProperty->UpdateIndicator(index);
            return;
        }
        if (animationDuration_.has_value()) {
            swiperController_->SwipeTo(index);
        } else {
            swiperController_->SwipeToWithoutAnimation(index);
        }
        layoutProperty->UpdateIndicator(index);
    }
}

void TabBarPattern::HandleTouchEvent(const TouchLocationInfo& info)
{
    if (IsContainsBuilder()) {
        return;
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto totalCount = host->TotalChildCount();
    auto touchType = info.GetTouchType();
    auto index = CalculateSelectedIndex(info.GetLocalLocation());
    if (touchType == TouchType::DOWN && index >= 0 && index < totalCount) {
        HandleTouchDown(index);
        touchingIndex_ = index;
        return;
    }
    if ((touchType == TouchType::UP || touchType == TouchType::CANCEL) && touchingIndex_.has_value()) {
        HandleTouchUp(index);
        touchingIndex_.reset();
    }
}

int32_t TabBarPattern::CalculateSelectedIndex(const Offset& info)
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, -1);
    auto geometryNode = host->GetGeometryNode();
    CHECK_NULL_RETURN(geometryNode, -1);
    auto frameSize = geometryNode->GetFrameSize();
    auto layoutProperty = host->GetLayoutProperty<TabBarLayoutProperty>();
    CHECK_NULL_RETURN(layoutProperty, -1);
    auto axis = layoutProperty->GetAxis().value_or(Axis::HORIZONTAL);
    auto local = OffsetF(info.GetX(), info.GetY());
    if (axis == Axis::VERTICAL) {
        auto clickRange = std::make_pair(tabItemOffsets_[0].GetY(), tabItemOffsets_[tabItemOffsets_.size() - 1].GetY());
        if (LessNotEqual(local.GetY(), clickRange.first) || GreatNotEqual(local.GetY(), clickRange.second)) {
            LOGW("clicked (%{public}lf) position out of range [%{public}lf, %{public}lf]", local.GetY(),
                clickRange.first, clickRange.second);
            return -1;
        }
    } else {
        auto clickRange = std::make_pair(tabItemOffsets_[0].GetX(), tabItemOffsets_[tabItemOffsets_.size() - 1].GetX());
        if (!isRTL_) {
            if (LessNotEqual(local.GetX(), clickRange.first) || GreatNotEqual(local.GetX(), clickRange.second)) {
                LOGW("clicked (%{public}lf) position out of range [%{public}lf, %{public}lf]", local.GetX(),
                    clickRange.first, clickRange.second);
                return -1;
            }
        } else {
            if (GreatNotEqual(local.GetX(), frameSize.MainSize(axis)) ||
                LessNotEqual(local.GetX(), clickRange.second)) {
                LOGW("clicked (%{public}lf) position out of range [%{public}lf, %{public}lf]", local.GetX(),
                    clickRange.first, clickRange.second);
                return -1;
            }
        }
    }
    auto pos = std::lower_bound(tabItemOffsets_.begin(), tabItemOffsets_.end(), local,
        [axis, isRTL = isRTL_](const OffsetF& a, const OffsetF& b) {
            return isRTL
                       ? GreatNotEqual(a.GetX(), b.GetX())
                       : (axis == Axis::VERTICAL ? LessNotEqual(a.GetY(), b.GetY()) : LessNotEqual(a.GetX(), b.GetX()));
        });

    if (pos == tabItemOffsets_.end()) {
        return -1;
    }
    return isRTL_ ? std::distance(tabItemOffsets_.begin(), pos) : std::distance(tabItemOffsets_.begin(), pos) - 1;
}

void TabBarPattern::HandleTouchDown(int32_t index)
{
    const auto& removeSwiperEventCallback = swiperController_->GetRemoveSwiperEventCallback();
    if (removeSwiperEventCallback) {
        removeSwiperEventCallback();
    }
    SetTouching(true);
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto tabTheme = pipelineContext->GetTheme<TabTheme>();
    CHECK_NULL_VOID(tabTheme);
    PlayPressAnimation(index, tabTheme->GetSubTabBarPressedColor(), AnimationType::PRESS);
}

void TabBarPattern::HandleTouchUp(int32_t index)
{
    const auto& addSwiperEventCallback = swiperController_->GetAddSwiperEventCallback();
    if (addSwiperEventCallback) {
        addSwiperEventCallback();
    }
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto tabTheme = pipelineContext->GetTheme<TabTheme>();
    CHECK_NULL_VOID(tabTheme);
    if (IsTouching()) {
        SetTouching(false);
        if (hoverIndex_.has_value() && touchingIndex_.value_or(-1) == index) {
            PlayPressAnimation(index, tabTheme->GetSubTabBarHoverColor(), AnimationType::HOVERTOPRESS);
            return;
        }
        PlayPressAnimation(touchingIndex_.value_or(-1), Color::TRANSPARENT, AnimationType::PRESS);
        if (hoverIndex_.has_value()) {
            PlayPressAnimation(hoverIndex_.value(), tabTheme->GetSubTabBarHoverColor(), AnimationType::HOVER);
        }
    }
}

void TabBarPattern::PlayPressAnimation(int32_t index, const Color& pressColor, AnimationType animationType)
{
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto tabTheme = pipelineContext->GetTheme<TabTheme>();
    CHECK_NULL_VOID(tabTheme);
    AnimationOption option = AnimationOption();
    option.SetDuration(animationType == AnimationType::HOVERTOPRESS
                           ? static_cast<int32_t>(tabTheme->GetSubTabBarHoverToPressDuration())
                           : static_cast<int32_t>(tabTheme->GetSubTabBarHoverDuration()));
    option.SetDelay(0);

    option.SetCurve(animationType == AnimationType::PRESS   ? AceType::MakeRefPtr<CubicCurve>(0.2f, 0.0f, 0.1f, 1.0f)
                    : animationType == AnimationType::HOVER ? Curves::FRICTION
                                                            : Curves::SHARP);
    option.SetFillMode(FillMode::FORWARDS);
    AnimationUtils::Animate(option, [weak = AceType::WeakClaim(this), selectedIndex = index, color = pressColor]() {
        auto tabBar = weak.Upgrade();
        if (tabBar) {
            auto host = tabBar->GetHost();
            CHECK_NULL_VOID(host);
            auto columnNode = AceType::DynamicCast<FrameNode>(host->GetChildAtIndex(selectedIndex));
            CHECK_NULL_VOID(columnNode);
            auto renderContext = columnNode->GetRenderContext();
            CHECK_NULL_VOID(renderContext);
            BorderRadiusProperty borderRadiusProperty;
            auto pipelineContext = PipelineContext::GetCurrentContext();
            CHECK_NULL_VOID(pipelineContext);
            auto tabTheme = pipelineContext->GetTheme<TabTheme>();
            CHECK_NULL_VOID(tabTheme);
            borderRadiusProperty.SetRadius(tabTheme->GetFocusIndicatorRadius());
            renderContext->UpdateBackgroundColor(color);
            renderContext->UpdateBorderRadius(borderRadiusProperty);
            columnNode->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
        }
    });
}

void TabBarPattern::UpdateCurrentOffset(float offset)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    currentOffset_ = currentOffset_ + offset;
    UpdateIndicator(indicator_);
    host->MarkDirtyNode(PROPERTY_UPDATE_LAYOUT);
}

void TabBarPattern::UpdateIndicator(int32_t indicator)
{
    auto layoutProperty = GetLayoutProperty<TabBarLayoutProperty>();
    layoutProperty->UpdateIndicator(indicator);

    auto tabBarNode = GetHost();
    CHECK_NULL_VOID(tabBarNode);
    auto tabBarPattern = tabBarNode->GetPattern<TabBarPattern>();
    CHECK_NULL_VOID(tabBarPattern);
    if (tabBarPattern->IsContainsBuilder() || layoutProperty->GetAxis() == Axis::VERTICAL ||
        tabBarStyle_ == TabBarStyle::BOTTOMTABBATSTYLE) {
        return;
    }

    RectF rect = layoutProperty->GetIndicatorRect(indicator);
    auto paintProperty = GetPaintProperty<TabBarPaintProperty>();
    paintProperty->UpdateIndicator(rect);
    currentIndicatorOffset_ = rect.GetX();
    tabBarNode->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
}

void TabBarPattern::UpdateTextColor(int32_t indicator)
{
    auto tabBarNode = GetHost();
    CHECK_NULL_VOID(tabBarNode);
    auto tabBarPattern = tabBarNode->GetPattern<TabBarPattern>();
    CHECK_NULL_VOID(tabBarPattern);
    if (tabBarPattern->IsContainsBuilder()) {
        return;
    }
    auto columnNode = DynamicCast<FrameNode>(tabBarNode->GetChildAtIndex(indicator));
    CHECK_NULL_VOID(columnNode);
    auto selectedColumnId = columnNode->GetId();
    CHECK_NULL_VOID(tabBarNode);
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto tabTheme = pipelineContext->GetTheme<TabTheme>();
    CHECK_NULL_VOID(tabTheme);
    for (const auto& columnNode : tabBarNode->GetChildren()) {
        CHECK_NULL_VOID(columnNode);
        auto textNode = AceType::DynamicCast<FrameNode>(columnNode->GetChildren().back());
        CHECK_NULL_VOID(textNode);
        auto textLayoutProperty = textNode->GetLayoutProperty<TextLayoutProperty>();
        CHECK_NULL_VOID(textLayoutProperty);
        if (columnNode->GetId() == selectedColumnId) {
            textLayoutProperty->UpdateTextColor(tabTheme->GetActiveIndicatorColor());
        } else {
            textLayoutProperty->UpdateTextColor(tabTheme->GetSubTabTextOffColor());
        }
        textNode->MarkModifyDone();
        textNode->MarkDirtyNode();
    }
}

bool TabBarPattern::IsContainsBuilder()
{
    return std::any_of(tabBarType_.begin(), tabBarType_.end(), [](const auto& isBuilder) { return isBuilder.second; });
}

void TabBarPattern::PlayTranslateAnimation(float startPos, float endPos)
{
    LOGI("Play translate animation startPos: %{public}lf, endPos: %{public}lf", startPos, endPos);
    auto curve = MakeRefPtr<CubicCurve>(0.2f, 0.0f, 0.1f, 1.0f);

    // If animation is still running, stop it before play new animation.
    StopTranslateAnimation();

    auto translate = AceType::MakeRefPtr<CurveAnimation<double>>(startPos, endPos, curve);
    auto weak = AceType::WeakClaim(this);
    translate->AddListener(Animation<double>::ValueCallback([weak, startPos, endPos](double value) {
        auto tabBarPattern = weak.Upgrade();
        CHECK_NULL_VOID(tabBarPattern);
        if (!NearEqual(value, startPos) && !NearEqual(value, endPos) && !NearEqual(startPos, endPos)) {
            float moveRate =
                Curves::EASE_OUT->MoveInternal(static_cast<float>((value - startPos) / (endPos - startPos)));
            value = startPos + (endPos - startPos) * moveRate;
        }
        tabBarPattern->UpdateIndicatorCurrentOffset(static_cast<float>(value - tabBarPattern->currentIndicatorOffset_));
    }));

    if (!controller_) {
        auto pipeline = PipelineBase::GetCurrentContext();
        CHECK_NULL_VOID(pipeline);
        controller_ = AceType::MakeRefPtr<Animator>(pipeline);
    }
    controller_->ClearStopListeners();
    controller_->ClearInterpolators();
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto tabTheme = pipelineContext->GetTheme<TabTheme>();
    CHECK_NULL_VOID(tabTheme);
    controller_->SetDuration(static_cast<int32_t>(tabTheme->GetTabContentAnimationDuration()));
    controller_->AddInterpolator(translate);
    controller_->Play();
}

void TabBarPattern::StopTranslateAnimation()
{
    if (controller_ && !controller_->IsStopped()) {
        controller_->Stop();
    }
}

void TabBarPattern::UpdateIndicatorCurrentOffset(float offset)
{
    currentIndicatorOffset_ = currentIndicatorOffset_ + offset;
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
}

} // namespace OHOS::Ace::NG

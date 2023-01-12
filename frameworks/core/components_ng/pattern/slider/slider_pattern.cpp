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

#include "core/components_ng/pattern/slider/slider_pattern.h"

#include <valarray>

#include "base/geometry/offset.h"
#include "base/utils/utils.h"
#include "core/components_ng/pattern/slider/slider_accessibility_property.h"
#include "core/components_ng/pattern/slider/tip_pattern.h"
#include "core/components_ng/pattern/text/text_pattern.h"
#include "core/pipeline/pipeline_base.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
namespace {
constexpr float HALF = 0.5;
constexpr Dimension FOCUS_DISTANCE = 4.0_vp;
} // namespace

void SliderPattern::OnModifyDone()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto hub = host->GetEventHub<EventHub>();
    CHECK_NULL_VOID(hub);
    auto gestureHub = hub->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(gestureHub);
    auto inputEventHub = hub->GetOrCreateInputEventHub();
    CHECK_NULL_VOID(inputEventHub);
    auto layoutProperty = host->GetLayoutProperty<SliderLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);
    layoutProperty->UpdateAlignment(Alignment::CENTER);
    auto sliderPaintProperty = host->GetPaintProperty<SliderPaintProperty>();
    CHECK_NULL_VOID(sliderPaintProperty);
    showTips_ = layoutProperty->GetShowTips().value_or(false);
    value_ = sliderPaintProperty->GetValue().value_or(0.0f);
    float min = sliderPaintProperty->GetMin().value_or(0.0f);
    float max = sliderPaintProperty->GetMax().value_or(100.0f);
    float step = sliderPaintProperty->GetStep().value_or(1.0f);
    valueRatio_ = (value_ - min) / (max - min);
    stepRatio_ = step / (max - min);
    InitTouchEvent(gestureHub);
    InitPanEvent(gestureHub);
    InitMouseEvent(inputEventHub);
    auto focusHub = hub->GetFocusHub();
    CHECK_NULL_VOID_NOLOG(focusHub);
    InitOnKeyEvent(focusHub);
}

bool SliderPattern::OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, bool skipMeasure, bool /*skipLayout*/)
{
    if (skipMeasure || dirty->SkipMeasureContent()) {
        return false;
    }

    auto layoutAlgorithmWrapper = DynamicCast<LayoutAlgorithmWrapper>(dirty->GetLayoutAlgorithm());
    CHECK_NULL_RETURN(layoutAlgorithmWrapper, false);
    auto sliderLayoutAlgorithm = DynamicCast<SliderLayoutAlgorithm>(layoutAlgorithmWrapper->GetLayoutAlgorithm());
    CHECK_NULL_RETURN(sliderLayoutAlgorithm, false);
    trackThickness_ = sliderLayoutAlgorithm->GetTrackThickness();
    blockDiameter_ = sliderLayoutAlgorithm->GetBlockDiameter();
    blockHotSize_ = sliderLayoutAlgorithm->GetBlockHotSize();

    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    auto sliderLayoutProperty = host->GetLayoutProperty<SliderLayoutProperty>();
    CHECK_NULL_RETURN(sliderLayoutProperty, false);
    std::optional<SizeF> contentSize = GetHostContentSize();
    CHECK_NULL_RETURN(contentSize.has_value(), false);
    float length = sliderLayoutProperty->GetDirection().value_or(Axis::HORIZONTAL) == Axis::HORIZONTAL
                       ? contentSize.value().Width()
                       : contentSize.value().Height();

    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, false);
    auto theme = pipeline->GetTheme<SliderTheme>();
    CHECK_NULL_RETURN(theme, false);
    Dimension hotBlockShadowWidth = sliderLayoutProperty->GetSliderMode().value_or(SliderModel::SliderMode::OUTSET) ==
                                            SliderModel::SliderMode::OUTSET
                                        ? theme->GetOutsetHotBlockShadowWidth()
                                        : theme->GetInsetHotBlockShadowWidth();

    hotBlockShadowWidth_ = static_cast<float>(hotBlockShadowWidth.ConvertToPx());
    borderBlank_ = std::max(trackThickness_, blockDiameter_ + hotBlockShadowWidth_ / HALF);
    sliderLength_ = length >= borderBlank_ ? length - borderBlank_ : 1;
    borderBlank_ = (length - sliderLength_) * HALF;

    return true;
}

void SliderPattern::InitTouchEvent(const RefPtr<GestureEventHub>& gestureHub)
{
    if (touchEvent_) {
        return;
    }
    auto touchTask = [weak = WeakClaim(this)](const TouchEventInfo& info) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID_NOLOG(pattern);
        pattern->HandleTouchEvent(info);
    };
    gestureHub->RemoveTouchEvent(touchEvent_);
    touchEvent_ = MakeRefPtr<TouchEventImpl>(std::move(touchTask));
    gestureHub->AddTouchEvent(touchEvent_);
}

void SliderPattern::HandleTouchEvent(const TouchEventInfo& info)
{
    auto touchType = info.GetTouches().front().GetTouchType();
    if (touchType == TouchType::DOWN) {
        hotFlag_ = true;
        UpdateValueByLocalLocation(info.GetChangedTouches().front().GetLocalLocation());
        if (showTips_) {
            bubbleFlag_ = true;
            CreateAndSetBubbleNode();
        }
        FireChangeEvent(SliderChangeMode::Click);
    } else if (touchType == TouchType::UP) {
        hotFlag_ = false;
        if (bubbleFlag_) {
            RemoveBubbleNode();
            bubbleFlag_ = false;
        }
    }
    UpdateMarkDirtyNode(showTips_ ? PROPERTY_UPDATE_MEASURE : PROPERTY_UPDATE_RENDER);
}

void SliderPattern::CreateAndSetBubbleNode()
{
    CHECK_NULL_VOID(!HasBubbleNode());
    auto frameNode = GetHost();
    CHECK_NULL_VOID(frameNode);
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto sliderTheme = pipeline->GetTheme<SliderTheme>();
    CHECK_NULL_VOID(sliderTheme);

    std::string content = std::to_string(static_cast<int>(std::round(valueRatio_ * 100.0f))) + '%';

    auto tipNode = FrameNode::GetOrCreateFrameNode(
        V2::POPUP_ETS_TAG, GetBubbleId(), []() { return AceType::MakeRefPtr<TipPattern>(); });
    tipNode->GetLayoutProperty<TipLayoutProperty>()->UpdateDirection(direction_);
    tipNode->GetLayoutProperty<TipLayoutProperty>()->UpdatePadding(sliderTheme->GetTipTextPadding());
    tipNode->GetPaintProperty<TipPaintProperty>()->UpdateDirection(direction_);
    tipNode->GetPaintProperty<TipPaintProperty>()->UpdateColor(sliderTheme->GetTipColor());
    tipNode->MountToParent(frameNode);

    auto textNode = FrameNode::GetOrCreateFrameNode(V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        []() { return AceType::MakeRefPtr<TextPattern>(); });
    textNode->GetRenderContext()->UpdateBackgroundColor(sliderTheme->GetTipColor());
    textNode->GetLayoutProperty<TextLayoutProperty>()->UpdateContent(content);
    textNode->GetLayoutProperty<TextLayoutProperty>()->UpdateTextColor(sliderTheme->GetTipTextColor());
    textNode->GetLayoutProperty<TextLayoutProperty>()->UpdateFontSize(sliderTheme->GetTipFontSize());
    textNode->MarkModifyDone();
    textNode->MountToParent(tipNode);
    UpdateBubbleNode();
}

void SliderPattern::RemoveBubbleNode()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->RemoveChildAtIndex(0);
    UpdateMarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    bubbleId_ = std::nullopt;
}

void SliderPattern::HandlingGestureEvent(const GestureEvent& info)
{
    if (info.GetInputEventType() == InputEventType::AXIS) {
        info.GetMainDelta() > 0.0 ? MoveStep(-1) : MoveStep(1);
    } else {
        UpdateValueByLocalLocation(info.GetLocalLocation());
        UpdateBubbleNode();
    }
    UpdateMarkDirtyNode(PROPERTY_UPDATE_RENDER);
}

void SliderPattern::HandledGestureEvent()
{
    hotFlag_ = false;
    UpdateMarkDirtyNode(PROPERTY_UPDATE_RENDER);
}

void SliderPattern::UpdateValueByLocalLocation(const std::optional<Offset>& localLocation)
{
    CHECK_NULL_VOID(localLocation.has_value());
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto sliderLayoutProperty = host->GetLayoutProperty<SliderLayoutProperty>();
    CHECK_NULL_VOID(sliderLayoutProperty);
    auto sliderPaintProperty = host->GetPaintProperty<SliderPaintProperty>();
    float length = sliderLayoutProperty->GetDirection().value_or(Axis::HORIZONTAL) == Axis::HORIZONTAL
                       ? static_cast<float>(localLocation->GetX())
                       : static_cast<float>(localLocation->GetY());
    float touchLength = sliderPaintProperty->GetReverse().value_or(false) ? borderBlank_ + sliderLength_ - length
                                                                          : length - borderBlank_;
    float min = sliderPaintProperty->GetMin().value_or(0.0f);
    float max = sliderPaintProperty->GetMax().value_or(100.0f);
    touchLength = std::clamp(touchLength, 0.0f, sliderLength_);
    CHECK_NULL_VOID(sliderLength_ != 0);
    valueRatio_ = touchLength / sliderLength_;
    CHECK_NULL_VOID(stepRatio_ != 0);
    valueRatio_ = NearEqual(valueRatio_, 1) ? 1 : std::round(valueRatio_ / stepRatio_) * stepRatio_;
    float oldValue = value_;
    value_ = valueRatio_ * (max - min) + min;
    valueChangeFlag_ = !NearEqual(oldValue, value_);
}

void SliderPattern::UpdateTipsValue()
{
    CHECK_NULL_VOID_NOLOG(valueChangeFlag_);
    CHECK_NULL_VOID_NOLOG(showTips_);
    CHECK_NULL_VOID(HasBubbleNode());
    auto frameNode = GetHost();
    CHECK_NULL_VOID(frameNode);
    auto bubbleNode = frameNode->GetChildAtIndex(0);
    CHECK_NULL_VOID(bubbleNode);
    auto textNode = DynamicCast<FrameNode>(bubbleNode->GetChildAtIndex(0));
    CHECK_NULL_VOID(textNode);
    std::string content = std::to_string(static_cast<int>(std::round(valueRatio_ * 100.0f))) + '%';
    textNode->GetLayoutProperty<TextLayoutProperty>()->UpdateContent(content);
    textNode->MarkModifyDone();
}

void SliderPattern::UpdateCircleCenterOffset()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto frameSize = GetHostFrameSize();
    CHECK_NULL_VOID(frameSize.has_value());
    auto sliderPaintProperty = host->GetPaintProperty<SliderPaintProperty>();
    CHECK_NULL_VOID(sliderPaintProperty);
    auto touchLength = valueRatio_ * sliderLength_;
    auto touchOffset = sliderPaintProperty->GetReverse().value_or(false) ? sliderLength_ - touchLength + borderBlank_
                                                                         : touchLength + borderBlank_;
    if (sliderPaintProperty->GetDirection().value_or(Axis::HORIZONTAL) == Axis::HORIZONTAL) {
        circleCenter_.SetX(touchOffset);
        circleCenter_.SetY(frameSize->Height() * HALF);
    } else {
        circleCenter_.SetX(frameSize->Width() * HALF);
        circleCenter_.SetY(touchOffset);
    }
}

void SliderPattern::UpdateBubbleNode()
{
    CHECK_NULL_VOID_NOLOG(showTips_);
    UpdateTipsValue();
    UpdateCircleCenterOffset();
    UpdateMarkDirtyNode(PROPERTY_UPDATE_MEASURE);
}

void SliderPattern::InitPanEvent(const RefPtr<GestureEventHub>& gestureHub)
{
    if (direction_ == GetDirection() && panEvent_) {
        return;
    }
    direction_ = GetDirection();
    auto actionStartTask = [weak = WeakClaim(this)](const GestureEvent& info) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID_NOLOG(pattern);
        pattern->HandlingGestureEvent(info);
        pattern->FireChangeEvent(SliderChangeMode::Begin);
    };
    auto actionUpdateTask = [weak = WeakClaim(this)](const GestureEvent& info) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID_NOLOG(pattern);
        pattern->HandlingGestureEvent(info);
        pattern->FireChangeEvent(SliderChangeMode::Moving);
    };
    auto actionEndTask = [weak = WeakClaim(this)](const GestureEvent& /*info*/) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID_NOLOG(pattern);
        pattern->HandledGestureEvent();
        pattern->FireChangeEvent(SliderChangeMode::End);
    };
    auto actionCancelTask = [weak = WeakClaim(this)]() {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID_NOLOG(pattern);
        pattern->HandledGestureEvent();
        pattern->FireChangeEvent(SliderChangeMode::End);
    };
    if (panEvent_) {
        gestureHub->RemovePanEvent(panEvent_);
    }
    panEvent_ = MakeRefPtr<PanEvent>(
        std::move(actionStartTask), std::move(actionUpdateTask), std::move(actionEndTask), std::move(actionCancelTask));

    PanDirection panDirection;
    panDirection.type = direction_ == Axis::VERTICAL ? PanDirection::VERTICAL : PanDirection::HORIZONTAL;
    float distance = static_cast<float>(Dimension(DEFAULT_PAN_DISTANCE, DimensionUnit::VP).ConvertToPx());
    gestureHub->AddPanEvent(panEvent_, panDirection, 1, distance);
}

void SliderPattern::InitOnKeyEvent(const RefPtr<FocusHub>& focusHub)
{
    auto getInnerPaintRectCallback = [wp = WeakClaim(this)](RoundRect& paintRect) {
        auto pattern = wp.Upgrade();
        CHECK_NULL_VOID_NOLOG(pattern);
        pattern->GetInnerFocusPaintRect(paintRect);
    };
    focusHub->SetInnerFocusPaintRectCallback(getInnerPaintRectCallback);

    auto onKeyEvent = [wp = WeakClaim(this)](const KeyEvent& event) -> bool {
        auto pattern = wp.Upgrade();
        CHECK_NULL_RETURN_NOLOG(pattern, false);
        return pattern->OnKeyEvent(event);
    };
    focusHub->SetOnKeyEventInternal(std::move(onKeyEvent));
}

void SliderPattern::GetInnerFocusPaintRect(RoundRect& paintRect)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto sliderLayoutProperty = host->GetLayoutProperty<SliderLayoutProperty>();
    auto sliderMode = sliderLayoutProperty->GetSliderMode().value_or(SliderModel::SliderMode::OUTSET);
    if (sliderMode == SliderModel::SliderMode::OUTSET) {
        GetOutsetInnerFocusPaintRect(paintRect);
    } else {
        GetInsetInnerFocusPaintRect(paintRect);
    }
}

void SliderPattern::GetOutsetInnerFocusPaintRect(RoundRect& paintRect)
{
    UpdateCircleCenterOffset();
    auto focusRadius = blockDiameter_ * HALF + static_cast<float>(FOCUS_DISTANCE.ConvertToPx());
    paintRect.SetRect(RectF(circleCenter_.GetX() - focusRadius, circleCenter_.GetY() - focusRadius, focusRadius / HALF,
        focusRadius / HALF));
    paintRect.SetCornerRadius(focusRadius);
}
void SliderPattern::GetInsetInnerFocusPaintRect(RoundRect& paintRect)
{
    auto frameSize = GetHostFrameSize();
    CHECK_NULL_VOID(frameSize);
    float offsetX = 0;
    float offsetY = 0;
    float width = frameSize->Width();
    float height = frameSize->Height();
    float focusRadius = trackThickness_ + static_cast<float>(FOCUS_DISTANCE.ConvertToPx()) / HALF;
    if (direction_ == Axis::HORIZONTAL) {
        offsetX = borderBlank_ - trackThickness_ * HALF - static_cast<float>(FOCUS_DISTANCE.ConvertToPx());
        offsetY = (frameSize->Height() - trackThickness_) * HALF - static_cast<float>(FOCUS_DISTANCE.ConvertToPx());
        width = sliderLength_ + trackThickness_ + static_cast<float>(FOCUS_DISTANCE.ConvertToPx()) / HALF;
        height = trackThickness_ + static_cast<float>(FOCUS_DISTANCE.ConvertToPx()) / HALF;
    } else {
        offsetX = (frameSize->Width() - trackThickness_) * HALF - static_cast<float>(FOCUS_DISTANCE.ConvertToPx());
        offsetY = borderBlank_ - trackThickness_ * HALF - static_cast<float>(FOCUS_DISTANCE.ConvertToPx());
        width = trackThickness_ + static_cast<float>(FOCUS_DISTANCE.ConvertToPx()) / HALF;
        height = sliderLength_ + trackThickness_ + static_cast<float>(FOCUS_DISTANCE.ConvertToPx()) / HALF;
    }
    paintRect.SetRect(RectF(offsetX, offsetY, width, height));
    paintRect.SetCornerRadius(focusRadius);
}

void SliderPattern::PaintFocusState()
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

bool SliderPattern::OnKeyEvent(const KeyEvent& event)
{
    if (event.action != KeyAction::DOWN) {
        return false;
    }
    if ((direction_ == Axis::HORIZONTAL && event.code == KeyCode::KEY_DPAD_LEFT) ||
        (direction_ == Axis::VERTICAL && event.code == KeyCode::KEY_DPAD_UP)) {
        MoveStep(-1);
        PaintFocusState();
    }
    if ((direction_ == Axis::HORIZONTAL && event.code == KeyCode::KEY_DPAD_RIGHT) ||
        (direction_ == Axis::VERTICAL && event.code == KeyCode::KEY_DPAD_DOWN)) {
        MoveStep(1);
        PaintFocusState();
    }
    return false;
}

bool SliderPattern::MoveStep(int32_t stepCount)
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    auto sliderPaintProperty = host->GetPaintProperty<SliderPaintProperty>();
    CHECK_NULL_RETURN(sliderPaintProperty, false);
    float step = sliderPaintProperty->GetStep().value_or(1.0f);
    float min = sliderPaintProperty->GetMin().value_or(0.0f);
    float max = sliderPaintProperty->GetMax().value_or(100.0f);
    if (NearZero(step)) {
        return false;
    }
    float nextValue = -1.0;
    nextValue = value_ + static_cast<float>(stepCount) * step;
    if (NearEqual(nextValue, -1.0)) {
        return false;
    }
    nextValue = std::clamp(nextValue, min, max);
    nextValue = std::round(nextValue / step) * step;
    if (nextValue == value_) {
        return false;
    }
    value_ = nextValue;
    valueRatio_ = (value_ - min) / (max - min);
    LOGD("Move %{public}d steps, Value change to %{public}f", stepCount, value_);
    FireChangeEvent(SliderChangeMode::Click);
    UpdateMarkDirtyNode(PROPERTY_UPDATE_RENDER);
    return true;
}

void SliderPattern::InitMouseEvent(const RefPtr<InputEventHub>& inputEventHub)
{
    auto hoverEvent = [weak = WeakClaim(this)](bool isHover) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID_NOLOG(pattern);
        pattern->HandleHoverEvent(isHover);
    };
    if (hoverEvent_) {
        inputEventHub->RemoveOnHoverEvent(hoverEvent_);
    }
    hoverEvent_ = MakeRefPtr<InputEvent>(std::move(hoverEvent));
    inputEventHub->AddOnHoverEvent(hoverEvent_);

    auto mouseEvent = [weak = WeakClaim(this)](MouseInfo& info) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID_NOLOG(pattern);
        pattern->HandleMouseEvent(info);
    };
    if (mouseEvent_) {
        inputEventHub->RemoveOnMouseEvent(mouseEvent_);
    }
    mouseEvent_ = MakeRefPtr<InputEvent>(std::move(mouseEvent));
    inputEventHub->AddOnMouseEvent(mouseEvent_);
}

void SliderPattern::HandleHoverEvent(bool isHover)
{
    mouseHoverFlag_ = mouseHoverFlag_ && isHover;
    UpdateMarkDirtyNode(PROPERTY_UPDATE_RENDER);
}

void SliderPattern::HandleMouseEvent(const MouseInfo& info)
{
    UpdateCircleCenterOffset();
    auto mouseToCenterDistanceX = static_cast<float>(std::abs(info.GetLocalLocation().GetX() - circleCenter_.GetX()));
    auto mouseToCenterDistanceY = static_cast<float>(std::abs(info.GetLocalLocation().GetY() - circleCenter_.GetY()));
    float mouseToCenterDistance = std::max(mouseToCenterDistanceX, mouseToCenterDistanceY);
    mouseHoverFlag_ = LessOrEqual(mouseToCenterDistance, blockHotSize_ * HALF);
    mousePressedFlag_ = info.GetButton() == MouseButton::LEFT_BUTTON;
    UpdateMarkDirtyNode(PROPERTY_UPDATE_RENDER);
}

void SliderPattern::FireChangeEvent(int32_t mode)
{
    if (mode != SliderChangeMode::End) {
        CHECK_NULL_VOID(valueChangeFlag_);
    }
    auto sliderEventHub = GetEventHub<SliderEventHub>();
    CHECK_NULL_VOID(sliderEventHub);
    sliderEventHub->FireChangeEvent(static_cast<float>(value_), mode);
    valueChangeFlag_ = false;
}

void SliderPattern::UpdateMarkDirtyNode(const PropertyChangeFlag& Flag)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->MarkDirtyNode(Flag);
}

Axis SliderPattern::GetDirection() const
{
    auto sliderLayoutProperty = GetLayoutProperty<SliderLayoutProperty>();
    CHECK_NULL_RETURN(sliderLayoutProperty, Axis::HORIZONTAL);
    return sliderLayoutProperty->GetDirection().value_or(Axis::HORIZONTAL);
}

RefPtr<AccessibilityProperty> SliderPattern::CreateAccessibilityProperty()
{
    return MakeRefPtr<SliderAccessibilityProperty>();
}
} // namespace OHOS::Ace::NG
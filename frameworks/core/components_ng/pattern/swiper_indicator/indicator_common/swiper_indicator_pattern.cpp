/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "core/components_ng/pattern/swiper_indicator/indicator_common/swiper_indicator_pattern.h"

#include <cmath>

#include "base/log/dump_log.h"
#include "base/utils/utils.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/swiper/swiper_pattern.h"
#include "core/components_ng/pattern/swiper/swiper_utils.h"
#include "core/event/ace_events.h"
#include "core/event/mouse_event.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
namespace {
constexpr Dimension INDICATOR_ITEM_SPACE = 8.0_vp;
constexpr uint32_t INDICATOR_HAS_CHILD = 2;
constexpr Dimension INDICATOR_DRAG_MIN_DISTANCE = 4.0_vp;
constexpr Dimension INDICATOR_DRAG_MAX_DISTANCE = 18.0_vp;
constexpr Dimension INDICATOR_TOUCH_BOTTOM_MAX_DISTANCE = 80.0_vp;
constexpr int32_t LONG_PRESS_DELAY = 300;
constexpr double QUARTER_CIRCLE_ANGLE = 90.0;
constexpr double HALF_CIRCLE_ANGLE = 180.0;
constexpr double THREE_QUARTER_CIRCLE_ANGLE = 270.0;
constexpr double FULL_CIRCLE_ANGLE = 360.0;
constexpr float ITEM_PADDING = 5.0f;
constexpr float ACTIVE_ITEM_ANGLE = 4.0f;
constexpr float ADD_HOT_REG_ANGLE = 8.0f;
constexpr Dimension CONTAINER_BORDER_WIDTH = 24.0_vp;
constexpr Dimension CIRCLE_DIAMETER_OFFSET = 16.0_vp;
constexpr float INDICATOR_DRAG_MIN_ANGLE = 6.0;
constexpr float INDICATOR_DRAG_MAX_ANGLE = 23.0;
constexpr float INDICATOR_TOUCH_BOTTOM_MAX_ANGLE = 120.0;
} // namespace

void SwiperIndicatorPattern::OnAttachToFrameNode()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->GetRenderContext()->SetClipToBounds(true);
}

void SwiperIndicatorPattern::OnModifyDone()
{
    Pattern::OnModifyDone();
    auto host = GetHost();
    CHECK_NULL_VOID(host);

    auto swiperNode = GetSwiperNode();
    CHECK_NULL_VOID(swiperNode);
    auto swiperPattern = swiperNode->GetPattern<SwiperPattern>();
    CHECK_NULL_VOID(swiperPattern);
    swiperIndicatorType_ = swiperPattern->GetIndicatorType();
    if (swiperIndicatorType_ == SwiperIndicatorType::DIGIT) {
        RefPtr<FrameNode> firstTextNode;
        RefPtr<FrameNode> lastTextNode;
        auto layoutProperty = host->GetLayoutProperty<SwiperIndicatorLayoutProperty>();
        CHECK_NULL_VOID(layoutProperty);
        if (host->GetChildren().size() == INDICATOR_HAS_CHILD) {
            firstTextNode = DynamicCast<FrameNode>(host->GetFirstChild());
            lastTextNode = DynamicCast<FrameNode>(host->GetLastChild());
        } else {
            host->Clean();
            firstTextNode = FrameNode::CreateFrameNode(
                V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
            lastTextNode = FrameNode::CreateFrameNode(
                V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
        }
        UpdateTextContent(layoutProperty, firstTextNode, lastTextNode);
        host->AddChild(firstTextNode);
        host->AddChild(lastTextNode);
    } else {
        host->Clean();
    }

    if (dotIndicatorModifier_) {
        dotIndicatorModifier_->StopAnimation();
    }

    auto swiperEventHub = swiperPattern->GetEventHub<SwiperEventHub>();
    CHECK_NULL_VOID(swiperEventHub);

    swiperEventHub->SetIndicatorOnChange(
        [weak = AceType::WeakClaim(RawPtr(host)), context = AceType::WeakClaim(this)]() {
            auto pipeline = PipelineContext::GetCurrentContextSafely();
            CHECK_NULL_VOID(pipeline);
            pipeline->AddAfterLayoutTask([weak, context]() {
                auto indicator = weak.Upgrade();
                CHECK_NULL_VOID(indicator);
                auto textContext = context.Upgrade();
                CHECK_NULL_VOID(textContext);
                if (textContext->swiperIndicatorType_ == SwiperIndicatorType::DIGIT) {
                    RefPtr<FrameNode> firstTextNode;
                    RefPtr<FrameNode> lastTextNode;
                    auto layoutProperty = indicator->GetLayoutProperty<SwiperIndicatorLayoutProperty>();
                    firstTextNode = DynamicCast<FrameNode>(indicator->GetFirstChild());
                    lastTextNode = DynamicCast<FrameNode>(indicator->GetLastChild());
                    textContext->UpdateTextContent(layoutProperty, firstTextNode, lastTextNode);
                }
                indicator->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
            });
            pipeline->RequestFrame();
        });
    auto swiperLayoutProperty = swiperPattern->GetLayoutProperty<SwiperLayoutProperty>();
    CHECK_NULL_VOID(swiperLayoutProperty);
    if (swiperLayoutProperty->GetIndicatorTypeValue(SwiperIndicatorType::DOT) == SwiperIndicatorType::DOT) {
        auto gestureHub = host->GetOrCreateGestureEventHub();
        CHECK_NULL_VOID(gestureHub);
        InitClickEvent(gestureHub);
        InitHoverMouseEvent();
        InitTouchEvent(gestureHub);
        InitLongPressEvent(gestureHub);
        InitFocusEvent();
    }
    if (swiperLayoutProperty->GetIndicatorTypeValue(SwiperIndicatorType::DOT) == SwiperIndicatorType::ARC_DOT) {
        auto gestureHub = host->GetOrCreateGestureEventHub();
        CHECK_NULL_VOID(gestureHub);
        InitTouchEvent(gestureHub);
        InitLongPressEvent(gestureHub);
    }
}

void SwiperIndicatorPattern::InitFocusEvent()
{
    if (focusEventInitialized_) {
        return;
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto focusHub = host->GetOrCreateFocusHub();
    CHECK_NULL_VOID(focusHub);
    auto focusTask = [weak = WeakClaim(this)]() {
        auto pattern = weak.Upgrade();
        if (pattern) {
            pattern->HandleFocusEvent();
        }
    };
    focusHub->SetOnFocusInternal(focusTask);
    auto blurTask = [weak = WeakClaim(this)]() {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandleBlurEvent();
    };
    focusHub->SetOnBlurInternal(blurTask);
    focusEventInitialized_ = true;
}

void SwiperIndicatorPattern::HandleFocusEvent()
{
    CHECK_NULL_VOID(dotIndicatorModifier_);
    AddIsFocusActiveUpdateEvent();
    OnIsFocusActiveUpdate(true);
}

void SwiperIndicatorPattern::HandleBlurEvent()
{
    CHECK_NULL_VOID(dotIndicatorModifier_);
    RemoveIsFocusActiveUpdateEvent();
    OnIsFocusActiveUpdate(false);
}

void SwiperIndicatorPattern::AddIsFocusActiveUpdateEvent()
{
    if (!isFocusActiveUpdateEvent_) {
        isFocusActiveUpdateEvent_ = [weak = WeakClaim(this)](bool isFocusAcitve) {
            auto pattern = weak.Upgrade();
            CHECK_NULL_VOID(pattern);
            pattern->OnIsFocusActiveUpdate(isFocusAcitve);
        };
    }

    auto pipline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipline);
    pipline->AddIsFocusActiveUpdateEvent(GetHost(), isFocusActiveUpdateEvent_);
}

void SwiperIndicatorPattern::RemoveIsFocusActiveUpdateEvent()
{
    auto pipline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipline);
    pipline->RemoveIsFocusActiveUpdateEvent(GetHost());
}

void SwiperIndicatorPattern::OnIsFocusActiveUpdate(bool isFocusAcitve)
{
    CHECK_NULL_VOID(dotIndicatorModifier_);
    dotIndicatorModifier_->SetIsFocused(isFocusAcitve);
}

bool SwiperIndicatorPattern::OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config)
{
    if (swiperIndicatorType_ == SwiperIndicatorType::ARC_DOT) {
        return SetArcIndicatorHotRegion(dirty, config);
    }
    CHECK_NULL_RETURN(config.frameSizeChange, false);
    return true;
}

void SwiperIndicatorPattern::InitClickEvent(const RefPtr<GestureEventHub>& gestureHub)
{
    CHECK_NULL_VOID(!clickEvent_);
    auto clickTask = [weak = WeakClaim(this)](const GestureEvent& info) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandleClick(info);
    };
    clickEvent_ = MakeRefPtr<ClickEvent>(std::move(clickTask));
    gestureHub->AddClickEvent(clickEvent_);
}

void SwiperIndicatorPattern::HandleClick(const GestureEvent& info)
{
    if (info.GetSourceDevice() == SourceType::KEYBOARD) {
        return;
    }

    if (info.GetSourceDevice() == SourceType::MOUSE) {
        isClicked_ = true;
        HandleMouseClick(info);
    } else {
        HandleTouchClick(info);
    }
}

void SwiperIndicatorPattern::HandleMouseClick(const GestureEvent& /* info */)
{
    if (isRepeatClicked_) {
        return;
    }
    GetMouseClickIndex();
    CHECK_NULL_VOID(mouseClickIndex_);
    auto swiperNode = GetSwiperNode();
    CHECK_NULL_VOID(swiperNode);
    auto swiperPattern = swiperNode->GetPattern<SwiperPattern>();
    CHECK_NULL_VOID(swiperPattern);

    if (swiperPattern->IsSwipeByGroup()) {
        auto clickPageIndex = SwiperUtils::ComputePageIndex(mouseClickIndex_.value(), swiperPattern->GetDisplayCount());
        if (clickPageIndex == swiperPattern->GetCurrentIndex()) {
            mouseClickIndex_ = std::nullopt;
            return;
        }

        mouseClickIndex_ = clickPageIndex;
    }

    swiperPattern->SwipeTo(mouseClickIndex_.value());
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
}

void SwiperIndicatorPattern::HandleTouchClick(const GestureEvent& info)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto paintProperty = host->GetPaintProperty<DotIndicatorPaintProperty>();
    CHECK_NULL_VOID(paintProperty);
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto theme = pipeline->GetTheme<SwiperIndicatorTheme>();
    CHECK_NULL_VOID(theme);
    auto itemWidth = paintProperty->GetItemWidthValue(theme->GetSize()).ConvertToPx();
    auto selectedItemWidth = paintProperty->GetSelectedItemWidthValue(theme->GetSize()).ConvertToPx();
    if (Negative(itemWidth) || Negative(selectedItemWidth)) {
        itemWidth = theme->GetSize().ConvertToPx();
        selectedItemWidth = theme->GetSize().ConvertToPx();
    }
    auto swiperNode = GetSwiperNode();
    CHECK_NULL_VOID(swiperNode);
    auto swiperPattern = swiperNode->GetPattern<SwiperPattern>();
    CHECK_NULL_VOID(swiperPattern);
    auto currentIndex = swiperPattern->GetCurrentIndex();
    auto margin = HandleTouchClickMargin();
    Dimension paddingSide = theme->GetIndicatorPaddingDot();
    auto lengthBeforeCurrentIndex = margin + paddingSide.ConvertToPx() +
                                    (INDICATOR_ITEM_SPACE.ConvertToPx() + itemWidth) * currentIndex;
    auto lengthWithCurrentIndex = lengthBeforeCurrentIndex + selectedItemWidth;
    auto axis = swiperPattern->GetDirection();
    auto mainClickOffset = axis == Axis::HORIZONTAL ? info.GetLocalLocation().GetX() : info.GetLocalLocation().GetY();
    if (mainClickOffset < lengthBeforeCurrentIndex) {
        swiperPattern->ShowPrevious();
    } else if (mainClickOffset > lengthWithCurrentIndex) {
        swiperPattern->ShowNext();
    }
}

void SwiperIndicatorPattern::InitHoverMouseEvent()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto eventHub = host->GetEventHub<EventHub>();
    CHECK_NULL_VOID(eventHub);
    auto inputHub = eventHub->GetOrCreateInputEventHub();
    CHECK_NULL_VOID(inputHub);

    auto hoverTask = [weak = WeakClaim(this)](bool isHover, HoverInfo& info) {
        auto pattern = weak.Upgrade();
        if (pattern && info.GetSourceDevice() != SourceType::TOUCH) {
            pattern->HandleHoverEvent(isHover);
        }
    };

    if (!hoverEvent_) {
        hoverEvent_ = MakeRefPtr<InputEvent>(std::move(hoverTask));
        inputHub->AddOnHoverEvent(hoverEvent_);
    }

    auto mouseEvent = [weak = WeakClaim(this)](MouseInfo& info) {
        auto pattern = weak.Upgrade();
        if (pattern) {
            pattern->HandleMouseEvent(info);
        }
    };
    if (mouseEvent_) {
        inputHub->RemoveOnMouseEvent(mouseEvent_);
    }
    mouseEvent_ = MakeRefPtr<InputEvent>(std::move(mouseEvent));
    inputHub->AddOnMouseEvent(mouseEvent_);
}

void SwiperIndicatorPattern::HandleMouseEvent(const MouseInfo& info)
{
    if (info.GetSourceDevice() == SourceType::TOUCH) {
        return;
    }
    auto mouseOffsetX = static_cast<float>(info.GetLocalLocation().GetX());
    auto mouseOffsetY = static_cast<float>(info.GetLocalLocation().GetY());
    auto mouseAction = info.GetAction();
    if ((mouseAction == MouseAction::PRESS || mouseAction == MouseAction::RELEASE) &&
        isClicked_ && NearEqual(hoverPoint_, PointF(mouseOffsetX, mouseOffsetY))) {
        isRepeatClicked_ = true;
        return;
    }
    isClicked_ = false;
    isRepeatClicked_ = false;
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    hoverPoint_.SetX(mouseOffsetX);
    hoverPoint_.SetY(mouseOffsetY);

    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
}

void SwiperIndicatorPattern::HandleHoverEvent(bool isHover)
{
    if (isHover_ == isHover) {
        return;
    }

    isHover_ = isHover;
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto swiperNode = GetSwiperNode();
    CHECK_NULL_VOID(swiperNode);
    auto swiperPattern = swiperNode->GetPattern<SwiperPattern>();
    CHECK_NULL_VOID(swiperPattern);
    auto swiperLayoutProperty = swiperPattern->GetLayoutProperty<SwiperLayoutProperty>();
    CHECK_NULL_VOID(swiperLayoutProperty);
    if (swiperLayoutProperty->GetHoverShowValue(false) && !swiperPattern->GetIsAtHotRegion()) {
        swiperPattern->ArrowHover(isHover_);
    }
    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
}

void SwiperIndicatorPattern::InitTouchEvent(const RefPtr<GestureEventHub>& gestureHub)
{
    if (touchEvent_) {
        return;
    }

    auto touchTask = [weak = WeakClaim(this)](const TouchEventInfo& info) {
        auto pattern = weak.Upgrade();
        if (pattern) {
            pattern->HandleTouchEvent(info);
        }
    };

    if (touchEvent_) {
        gestureHub->RemoveTouchEvent(touchEvent_);
    }
    touchEvent_ = MakeRefPtr<TouchEventImpl>(std::move(touchTask));
    gestureHub->AddTouchEvent(touchEvent_);

    auto swiperNode = GetSwiperNode();
    CHECK_NULL_VOID(swiperNode);
    auto swiperPattern = swiperNode->GetPattern<SwiperPattern>();
    CHECK_NULL_VOID(swiperPattern);
    auto stopAnimationCb = [weak = WeakClaim(this)](bool ifImmediately) {
        auto pattern = weak.Upgrade();
        if (pattern) {
            if (pattern->dotIndicatorModifier_) {
                pattern->dotIndicatorModifier_->StopAnimation(ifImmediately);
            }
        }
    };
    swiperPattern->SetStopIndicatorAnimationCb(stopAnimationCb);
}

void SwiperIndicatorPattern::HandleTouchEvent(const TouchEventInfo& info)
{
    auto touchType = info.GetTouches().front().GetTouchType();
    if (touchType == TouchType::UP) {
        HandleTouchUp();
        HandleDragEnd(0);
        isPressed_ = false;
    } else if (touchType == TouchType::CANCEL) {
        HandleTouchUp();
        HandleDragEnd(0);
        isPressed_ = false;
    }
    if (isPressed_) {
        HandleLongDragUpdate(info.GetTouches().front());
    }
}

void SwiperIndicatorPattern::HandleTouchDown()
{
    isPressed_ = true;
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
}

void SwiperIndicatorPattern::HandleTouchUp()
{
    isPressed_ = false;
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
}

void SwiperIndicatorPattern::GetMouseClickIndex()
{
    auto pipelineContext = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto swiperTheme = pipelineContext->GetTheme<SwiperIndicatorTheme>();
    CHECK_NULL_VOID(swiperTheme);
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto paintProperty = host->GetPaintProperty<DotIndicatorPaintProperty>();
    CHECK_NULL_VOID(paintProperty);
    auto swiperNode = GetSwiperNode();
    CHECK_NULL_VOID(swiperNode);
    auto swiperPattern = swiperNode->GetPattern<SwiperPattern>();
    CHECK_NULL_VOID(swiperPattern);
    float itemWidthValue = static_cast<float>(paintProperty->GetItemWidthValue(swiperTheme->GetSize()).ConvertToPx());
    float itemHeightValue = static_cast<float>(paintProperty->GetItemHeightValue(swiperTheme->GetSize()).ConvertToPx());
    float selectedItemWidthValue =
        static_cast<float>(paintProperty->GetSelectedItemWidthValue(swiperTheme->GetSize()).ConvertToPx() * 2);
    paintProperty->GetIsCustomSizeValue(false) ? selectedItemWidthValue *= 0.5f : selectedItemWidthValue;
    // diameter calculation
    double scaleIndicator = swiperTheme->GetScaleSwiper();
    float itemWidth = itemWidthValue * scaleIndicator;
    float itemHeight = itemHeightValue * scaleIndicator;
    float selectedItemWidth = selectedItemWidthValue * scaleIndicator;
    float space = static_cast<float>(INDICATOR_ITEM_SPACE.ConvertToPx());
    int32_t currentIndex = swiperPattern->GetCurrentShownIndex();
    int32_t itemCount = swiperPattern->RealTotalCount();
    int32_t loopCount = itemCount == 0 ? 0 : std::abs(currentIndex / itemCount);
    auto frameSize = host->GetGeometryNode()->GetFrameSize();
    auto axis = swiperPattern->GetDirection();
    float centerX = static_cast<float>((swiperTheme->GetIndicatorPaddingDot()).ConvertToPx());
    float centerY = ((axis == Axis::HORIZONTAL ? frameSize.Height() : frameSize.Width()) - itemHeight) * 0.5f;
    PointF hoverPoint = axis == Axis::HORIZONTAL ? hoverPoint_ : PointF(hoverPoint_.GetY(), hoverPoint_.GetX());
    int start = currentIndex >= 0 ? loopCount * itemCount : -(loopCount + 1) * itemCount;
    int end = currentIndex >= 0 ? (loopCount + 1) * itemCount : -loopCount * itemCount;
    if (swiperPattern->IsHorizontalAndRightToLeft()) {
        end = currentIndex >= 0 ? loopCount * itemCount - 1 : -(loopCount + 1) * itemCount - 1;
        start = currentIndex >= 0 ? (loopCount + 1) * itemCount - 1 : -loopCount * itemCount - 1;
    }
    for (int32_t i = start; i != end; start > end ? --i : ++i) {
        if (i != currentIndex) {
            if (hoverPoint.GetX() >= centerX && hoverPoint.GetX() <= centerX + itemWidth &&
                hoverPoint.GetY() >= centerY && hoverPoint.GetY() <= centerY + itemHeight) {
                mouseClickIndex_ = i;
                break;
            }
            centerX += itemWidth + space;
        } else {
            centerX += selectedItemWidth + space;
        }
    }
}

void SwiperIndicatorPattern::UpdateTextContent(const RefPtr<SwiperIndicatorLayoutProperty>& layoutProperty,
    const RefPtr<FrameNode>& firstTextNode, const RefPtr<FrameNode>& lastTextNode)
{
    CHECK_NULL_VOID(layoutProperty);
    CHECK_NULL_VOID(firstTextNode);
    CHECK_NULL_VOID(lastTextNode);
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto theme = pipeline->GetTheme<SwiperIndicatorTheme>();
    firstTextNode->SetInternal();
    lastTextNode->SetInternal();
    auto firstTextLayoutProperty = firstTextNode->GetLayoutProperty<TextLayoutProperty>();
    CHECK_NULL_VOID(firstTextLayoutProperty);
    auto selectedFontColor =
        layoutProperty->GetSelectedFontColorValue(theme->GetDigitalIndicatorTextStyle().GetTextColor());
    auto selectedFontSize =
        layoutProperty->GetSelectedFontSizeValue(theme->GetDigitalIndicatorTextStyle().GetFontSize());
    if (!selectedFontSize.IsValid()) {
        selectedFontSize = theme->GetDigitalIndicatorTextStyle().GetFontSize();
    }
    auto selectedFontWeight =
        layoutProperty->GetSelectedFontWeightValue(theme->GetDigitalIndicatorTextStyle().GetFontWeight());
    firstTextLayoutProperty->UpdateTextColor(selectedFontColor);
    firstTextLayoutProperty->UpdateFontSize(selectedFontSize);
    firstTextLayoutProperty->UpdateFontWeight(selectedFontWeight);
    UpdateTextContentSub(layoutProperty, firstTextNode, lastTextNode);
}

int32_t SwiperIndicatorPattern::GetCurrentIndex() const
{
    auto swiperNode = GetSwiperNode();
    CHECK_NULL_RETURN(swiperNode, 0);
    auto swiperPattern = swiperNode->GetPattern<SwiperPattern>();
    CHECK_NULL_RETURN(swiperPattern, 0);
    auto swiperLayoutProperty = swiperPattern->GetLayoutProperty<SwiperLayoutProperty>();
    CHECK_NULL_RETURN(swiperLayoutProperty, 0);
    auto currentIndex = swiperLayoutProperty->GetIndex().value_or(0);
    if (swiperPattern->IsSwipeByGroup()) {
        currentIndex = SwiperUtils::ComputePageIndex(currentIndex, swiperPattern->GetDisplayCount());
    }

    return currentIndex;
}

void SwiperIndicatorPattern::UpdateTextContentSub(const RefPtr<SwiperIndicatorLayoutProperty>& layoutProperty,
    const RefPtr<FrameNode>& firstTextNode, const RefPtr<FrameNode>& lastTextNode)
{
    CHECK_NULL_VOID(layoutProperty);
    CHECK_NULL_VOID(firstTextNode);
    CHECK_NULL_VOID(lastTextNode);
    auto swiperNode = GetSwiperNode();
    CHECK_NULL_VOID(swiperNode);
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto theme = pipeline->GetTheme<SwiperIndicatorTheme>();
    auto firstTextLayoutProperty = firstTextNode->GetLayoutProperty<TextLayoutProperty>();
    auto swiperPattern = swiperNode->GetPattern<SwiperPattern>();
    CHECK_NULL_VOID(swiperPattern);
    auto swiperLayoutProperty = swiperPattern->GetLayoutProperty<SwiperLayoutProperty>();
    CHECK_NULL_VOID(swiperLayoutProperty);
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto currentIndex = swiperPattern->GetCurrentFirstIndex() + 1;
    if (currentIndex > swiperPattern->RealTotalCount()) {
        currentIndex = 1;
    } else if (swiperLayoutProperty->HasIndex()) {
        currentIndex = GetCurrentIndex() + 1;
        if (currentIndex > swiperPattern->RealTotalCount()) {
            currentIndex = 1;
        }
    }
    auto lastTextLayoutProperty = lastTextNode->GetLayoutProperty<TextLayoutProperty>();
    CHECK_NULL_VOID(lastTextLayoutProperty);
    lastTextLayoutProperty->UpdateLayoutDirection(swiperPattern->GetNonAutoLayoutDirection());
    bool isRtl = swiperPattern->GetNonAutoLayoutDirection() == TextDirection::RTL;
    std::string firstContent = isRtl ? std::to_string(swiperPattern->RealTotalCount()) : std::to_string(currentIndex);
    std::string lastContent = isRtl ? std::to_string(currentIndex) + "\\" :
        "/" + std::to_string(swiperPattern->RealTotalCount());

    firstTextLayoutProperty->UpdateContent(firstContent);
    lastTextLayoutProperty = lastTextNode->GetLayoutProperty<TextLayoutProperty>();
    CHECK_NULL_VOID(lastTextLayoutProperty);
    auto fontColor = layoutProperty->GetFontColorValue(theme->GetDigitalIndicatorTextStyle().GetTextColor());
    auto fontSize = layoutProperty->GetFontSizeValue(theme->GetDigitalIndicatorTextStyle().GetFontSize());
    if (!fontSize.IsValid()) {
        fontSize = theme->GetDigitalIndicatorTextStyle().GetFontSize();
    }
    auto fontWeight = layoutProperty->GetFontWeightValue(theme->GetDigitalIndicatorTextStyle().GetFontWeight());
    lastTextLayoutProperty->UpdateTextColor(fontColor);
    lastTextLayoutProperty->UpdateFontSize(fontSize);
    lastTextLayoutProperty->UpdateFontWeight(fontWeight);
    lastTextLayoutProperty->UpdateContent(lastContent);
    firstTextNode->MarkModifyDone();
    firstTextNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF_AND_PARENT);
    lastTextNode->MarkModifyDone();
    lastTextNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF_AND_PARENT);
}

void SwiperIndicatorPattern::HandleDragStart(const GestureEvent& info)
{
    dragStartPoint_ =
        PointF(static_cast<float>(info.GetLocalLocation().GetX()), static_cast<float>(info.GetLocalLocation().GetY()));
}

void SwiperIndicatorPattern::HandleDragEnd(double dragVelocity)
{
    auto swiperNode = GetSwiperNode();
    CHECK_NULL_VOID(swiperNode);
    auto swiperPattern = swiperNode->GetPattern<SwiperPattern>();
    CHECK_NULL_VOID(swiperPattern);
    swiperPattern->SetTurnPageRate(0.0f);
    auto swiperPaintProperty = swiperPattern->GetPaintProperty<SwiperPaintProperty>();
    CHECK_NULL_VOID(swiperPaintProperty);
    auto autoPlay = swiperPaintProperty->GetAutoPlay().value_or(false);
    if (autoPlay) {
        swiperPattern->SetIndicatorLongPress(false);
        swiperPattern->StartAutoPlay();
    }
    if (swiperPattern->GetIndicatorType() == SwiperIndicatorType::ARC_DOT) {
        isLongPressed_ = false;
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    touchBottomType_ = TouchBottomType::NONE;
    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
}

void SwiperIndicatorPattern::SetIndicatorInteractive(bool isInteractive)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto eventHub = host->GetEventHub<EventHub>();
    CHECK_NULL_VOID(eventHub);
    if (isInteractive) {
        eventHub->SetEnabled(true);
    } else {
        eventHub->SetEnabled(false);
    }
}

bool SwiperIndicatorPattern::CheckIsTouchBottom(const GestureEvent& info)
{
    auto swiperNode = GetSwiperNode();
    CHECK_NULL_RETURN(swiperNode, false);
    auto swiperPattern = swiperNode->GetPattern<SwiperPattern>();
    CHECK_NULL_RETURN(swiperPattern, false);
    auto currentIndex = swiperPattern->GetCurrentIndex();
    auto childrenSize = swiperPattern->RealTotalCount();

    auto swiperLayoutProperty = swiperNode->GetLayoutProperty<SwiperLayoutProperty>();
    CHECK_NULL_RETURN(swiperLayoutProperty, false);
    auto displayCount = swiperLayoutProperty->GetDisplayCount().value_or(1);
    auto isLoop = swiperLayoutProperty->GetLoop().value_or(true);
    auto dragPoint =
        PointF(static_cast<float>(info.GetLocalLocation().GetX()), static_cast<float>(info.GetLocalLocation().GetY()));
    auto offset = dragPoint - dragStartPoint_;
    float touchBottomRate = 0.0;
    float touchOffset = 0.0;
    if (swiperIndicatorType_ == SwiperIndicatorType::ARC_DOT) {
        auto center = GetCenterPointF();
        float startAngle = GetAngleWithPoint(center, dragStartPoint_);
        float endAngle = GetAngleWithPoint(center, dragPoint);
        touchOffset = startAngle - endAngle;
        touchBottomRate = LessOrEqual(std::abs(touchOffset), INDICATOR_TOUCH_BOTTOM_MAX_ANGLE)
                                ? touchOffset / INDICATOR_TOUCH_BOTTOM_MAX_ANGLE : 1;
    } else {
        touchOffset = swiperPattern->GetDirection() == Axis::HORIZONTAL ? offset.GetX() : offset.GetY();
        touchBottomRate = LessOrEqual(std::abs(touchOffset), INDICATOR_TOUCH_BOTTOM_MAX_DISTANCE.ConvertToPx())
                                ? touchOffset / INDICATOR_TOUCH_BOTTOM_MAX_DISTANCE.ConvertToPx() : 1;
    }
    swiperPattern->SetTurnPageRate(0);
    swiperPattern->SetTouchBottomRate(std::abs(touchBottomRate));
    TouchBottomType touchBottomType = TouchBottomType::NONE;
    if ((currentIndex <= 0) && !isLoop) {
        if (Negative(info.GetMainDelta()) || NonPositive(touchOffset)) {
            touchBottomType = TouchBottomType::START;
        }
    }
    if ((currentIndex >= childrenSize - displayCount) && !isLoop) {
        if (Positive(info.GetMainDelta()) || NonNegative(touchOffset)) {
            touchBottomType = TouchBottomType::END;
        }
    }

    touchBottomType_ = touchBottomType;
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);

    return touchBottomType == TouchBottomType::NONE ? false : true;
}

bool SwiperIndicatorPattern::CheckIsTouchBottom(const TouchLocationInfo& info)
{
    auto swiperNode = GetSwiperNode();
    CHECK_NULL_RETURN(swiperNode, false);
    auto swiperPattern = swiperNode->GetPattern<SwiperPattern>();
    CHECK_NULL_RETURN(swiperPattern, false);
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    auto currentIndex = swiperPattern->GetCurrentIndex();
    auto childrenSize = swiperPattern->RealTotalCount();

    auto swiperLayoutProperty = swiperNode->GetLayoutProperty<SwiperLayoutProperty>();
    CHECK_NULL_RETURN(swiperLayoutProperty, false);
    auto displayCount = swiperLayoutProperty->GetDisplayCount().value_or(1);

    auto dragPoint =
        PointF(static_cast<float>(info.GetLocalLocation().GetX()), static_cast<float>(info.GetLocalLocation().GetY()));
    float touchBottomRate = 0.0;
    float touchOffset = 0.0;
    if (swiperIndicatorType_ == SwiperIndicatorType::ARC_DOT) {
        auto center = GetCenterPointF();
        float startAngle = GetAngleWithPoint(center, dragStartPoint_);
        float endAngle = GetAngleWithPoint(center, dragPoint);
        touchOffset = startAngle - endAngle;
        touchBottomRate = LessOrEqual(std::abs(touchOffset), INDICATOR_TOUCH_BOTTOM_MAX_ANGLE)
                                ? touchOffset / INDICATOR_TOUCH_BOTTOM_MAX_ANGLE : 1;
    } else {
        auto offset = dragPoint - dragStartPoint_;
        touchOffset = swiperPattern->GetDirection() == Axis::HORIZONTAL ? offset.GetX() : offset.GetY();
        touchBottomRate = LessOrEqual(std::abs(touchOffset), INDICATOR_TOUCH_BOTTOM_MAX_DISTANCE.ConvertToPx())
                                ? touchOffset / INDICATOR_TOUCH_BOTTOM_MAX_DISTANCE.ConvertToPx() : 1;
    }
    swiperPattern->SetTurnPageRate(0);
    swiperPattern->SetTouchBottomRate(std::abs(touchBottomRate));
    TouchBottomType touchBottomType = TouchBottomType::NONE;

    if (currentIndex <= 0) {
        if (swiperPattern->IsHorizontalAndRightToLeft()) {
            if (Positive(touchOffset)) {
                touchBottomType = TouchBottomType::END;
            }
        } else {
            if (NonPositive(touchOffset)) {
                touchBottomType = TouchBottomType::START;
            }
        }
    }
    if (currentIndex >= childrenSize - displayCount) {
        if (swiperPattern->IsHorizontalAndRightToLeft()) {
            if (NonPositive(touchOffset)) {
                touchBottomType = TouchBottomType::START;
            }
        } else {
            if (Positive(touchOffset)) {
                touchBottomType = TouchBottomType::END;
            }
        }
    }
    touchBottomType_ = touchBottomType;
    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);

    return touchBottomType == TouchBottomType::NONE ? false : true;
}

void SwiperIndicatorPattern::InitLongPressEvent(const RefPtr<GestureEventHub>& gestureHub)
{
    CHECK_NULL_VOID(!longPressEvent_);
    auto longPressCallback = [weak = WeakClaim(this)](GestureEvent& info) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandleLongPress(info);
    };
    longPressEvent_ = MakeRefPtr<LongPressEvent>(std::move(longPressCallback));

    gestureHub->SetLongPressEvent(longPressEvent_, false, false, LONG_PRESS_DELAY);
}

void SwiperIndicatorPattern::HandleLongPress(GestureEvent& info)
{
    HandleTouchDown();
    HandleDragStart(info);

    auto swiperNode = GetSwiperNode();
    CHECK_NULL_VOID(swiperNode);
    auto swiperPattern = swiperNode->GetPattern<SwiperPattern>();
    CHECK_NULL_VOID(swiperPattern);
    auto swiperPaintProperty = swiperPattern->GetPaintProperty<SwiperPaintProperty>();
    CHECK_NULL_VOID(swiperPaintProperty);
    auto autoPlay = swiperPaintProperty->GetAutoPlay().value_or(false);
    if (autoPlay) {
        swiperPattern->SetIndicatorLongPress(true);
        swiperPattern->StopTranslateAnimation();
        swiperPattern->StopSpringAnimation();
        swiperPattern->StopAutoPlay();
    }
    if (swiperPattern->GetIndicatorType() == SwiperIndicatorType::ARC_DOT) {
        isLongPressed_ = true;
    }
}

PointF SwiperIndicatorPattern::GetCenterPointF()
{
    auto center = PointF(0.0, 0.0);
    auto swiperNode = GetSwiperNode();
    CHECK_NULL_RETURN(swiperNode, center);
    auto swiperPattern = swiperNode->GetPattern<SwiperPattern>();
    CHECK_NULL_RETURN(swiperPattern, center);

    const auto& geometryNode = swiperNode->GetGeometryNode();
    const auto& contentSize = geometryNode->GetFrameSize();
    float centerY_ = (swiperPattern->GetDirection() == Axis::HORIZONTAL ?
        contentSize.Height() : contentSize.Width()) * 0.5;
    float centerX_ = (swiperPattern->GetDirection() == Axis::HORIZONTAL ?
        contentSize.Width() : contentSize.Height()) * 0.5;
    center = PointF(centerX_, centerY_);
    return center;
}

float SwiperIndicatorPattern::ConvertAngleWithArcDirection(SwiperArcDirection arcDirection, const float& angle)
{
    float result = 0.0;
    if (arcDirection == SwiperArcDirection::SIX_CLOCK_DIRECTION) {
        result = angle;
    } else if (arcDirection == SwiperArcDirection::THREE_CLOCK_DIRECTION) {
        if (angle > QUARTER_CIRCLE_ANGLE) {
            result = -THREE_QUARTER_CIRCLE_ANGLE + angle;
        } else {
            result  = angle + QUARTER_CIRCLE_ANGLE;
        }
    } else {
        if (angle < -QUARTER_CIRCLE_ANGLE) {
            result = THREE_QUARTER_CIRCLE_ANGLE + angle;
        } else {
            result = angle - QUARTER_CIRCLE_ANGLE;
        }
    }

    return result;
}

float SwiperIndicatorPattern::GetAngleWithPoint(const PointF& conter, const PointF& point)
{
    float angle = 0.0;
    auto swiperNode = GetSwiperNode();
    CHECK_NULL_RETURN(swiperNode, angle);
    auto swiperPattern = swiperNode->GetPattern<SwiperPattern>();
    CHECK_NULL_RETURN(swiperPattern, angle);

    float centerX = swiperPattern->GetDirection() == Axis::HORIZONTAL ? conter.GetX() : conter.GetY();
    float centerY = swiperPattern->GetDirection() == Axis::HORIZONTAL ? conter.GetY() : conter.GetX();
    float pointX = swiperPattern->GetDirection() == Axis::HORIZONTAL ? point.GetX() : point.GetY();
    float pointY = swiperPattern->GetDirection() == Axis::HORIZONTAL ? point.GetY() : point.GetX();
    const auto& arcDotParameters = swiperPattern->GetSwiperArcDotParameters();
    CHECK_NULL_RETURN(arcDotParameters, angle);
    std::optional<SwiperArcDirection> swiperArcDirection = arcDotParameters->arcDirection;
    CHECK_NULL_RETURN(swiperArcDirection.has_value(), angle);
    auto arcDirection = swiperArcDirection.value();
    if (NearEqual(centerY, pointY) && LessOrEqual(centerX, pointX)) {
        angle = -QUARTER_CIRCLE_ANGLE;
    } else if (NearEqual(centerX, pointX) && pointY > centerY) {
        angle = 0.0;
    } else if (NearEqual(centerY, pointY) && pointX < centerX) {
        angle = QUARTER_CIRCLE_ANGLE;
    } else if (NearEqual(centerX, pointX) && pointY < centerY) {
        angle = HALF_CIRCLE_ANGLE;
    } else if (pointX > centerX && pointY > centerY) {
        angle = atan((pointY - centerY) / (pointX - centerX)) * HALF_CIRCLE_ANGLE / M_PI - QUARTER_CIRCLE_ANGLE;
    } else if (pointX < centerX && pointY > centerY) {
        angle = QUARTER_CIRCLE_ANGLE - atan((pointY - centerY) / (centerX - pointX)) * HALF_CIRCLE_ANGLE / M_PI;
    } else if (pointX < centerX && pointY < centerY) {
        angle = QUARTER_CIRCLE_ANGLE + atan((centerY - pointY) / (centerX - pointX)) * HALF_CIRCLE_ANGLE / M_PI;
    } else {
        angle = -QUARTER_CIRCLE_ANGLE - atan((centerY - pointY) / (pointX - centerX)) * HALF_CIRCLE_ANGLE / M_PI;
    }

    return ConvertAngleWithArcDirection(arcDirection, angle);
}

void SwiperIndicatorPattern::HandleLongDragUpdate(const TouchLocationInfo& info)
{
    auto swiperNode = GetSwiperNode();
    CHECK_NULL_VOID(swiperNode);
    auto swiperPattern = swiperNode->GetPattern<SwiperPattern>();
    CHECK_NULL_VOID(swiperPattern);
    if (swiperPattern->IsIndicatorAnimatorRunning()) {
        return;
    }
    auto swiperLayoutProperty = swiperNode->GetLayoutProperty<SwiperLayoutProperty>();
    CHECK_NULL_VOID(swiperLayoutProperty);
    auto displayCount = swiperLayoutProperty->GetDisplayCount().value_or(1);
    if (swiperPattern->RealTotalCount() == displayCount) {
        return;
    }
    if (CheckIsTouchBottom(info)) {
        return;
    }
    float turnPageRate = 0.0;
    float turnPageRateOffset = 0.0;
    auto dragPoint =
        PointF(static_cast<float>(info.GetLocalLocation().GetX()), static_cast<float>(info.GetLocalLocation().GetY()));
    if (swiperIndicatorType_ == SwiperIndicatorType::ARC_DOT) {
        auto center = GetCenterPointF();
        float startAngle = GetAngleWithPoint(center, dragStartPoint_);
        float endAngle = GetAngleWithPoint(center, dragPoint);
        turnPageRateOffset = startAngle - endAngle;
        if (LessNotEqual(std::abs(turnPageRateOffset), INDICATOR_DRAG_MIN_ANGLE)) {
            return;
        }
        turnPageRate = -(turnPageRateOffset / INDICATOR_DRAG_MAX_ANGLE);
    } else {
        auto offset = dragPoint - dragStartPoint_;
        turnPageRateOffset = swiperPattern->GetDirection() == Axis::HORIZONTAL ? offset.GetX() : offset.GetY();
        if (LessNotEqual(std::abs(turnPageRateOffset), INDICATOR_DRAG_MIN_DISTANCE.ConvertToPx())) {
            return;
        }
        if (swiperPattern->IsHorizontalAndRightToLeft()) {
            turnPageRateOffset = -turnPageRateOffset;
        }
        turnPageRate = -(turnPageRateOffset / INDICATOR_DRAG_MAX_DISTANCE.ConvertToPx());
    }
    swiperPattern->SetTurnPageRate(turnPageRate);
    if (std::abs(turnPageRate) >= 1) {
        if (Positive(turnPageRateOffset)) {
            swiperPattern->SwipeToWithoutAnimation(swiperPattern->GetCurrentIndex() + 1);
        }
        if (NonPositive(turnPageRateOffset)) {
            swiperPattern->SwipeToWithoutAnimation(swiperPattern->GetCurrentIndex() - 1);
        }

        dragStartPoint_ = dragPoint;
    }
}

float SwiperIndicatorPattern::HandleTouchClickMargin()
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, 0.0f);
    auto paintProperty = host->GetPaintProperty<DotIndicatorPaintProperty>();
    CHECK_NULL_RETURN(paintProperty, 0.0f);
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, 0.0f);
    auto theme = pipeline->GetTheme<SwiperIndicatorTheme>();
    CHECK_NULL_RETURN(theme, 0.0f);
    auto itemWidth = paintProperty->GetItemWidthValue(theme->GetSize()).ConvertToPx();
    auto selectedItemWidth = paintProperty->GetSelectedItemWidthValue(theme->GetSize()).ConvertToPx();
    if (Negative(itemWidth) || Negative(selectedItemWidth)) {
        itemWidth = theme->GetSize().ConvertToPx();
        selectedItemWidth = theme->GetSize().ConvertToPx();
    }
    auto swiperNode = GetSwiperNode();
    CHECK_NULL_RETURN(swiperNode, 0.0f);
    auto swiperPattern = swiperNode->GetPattern<SwiperPattern>();
    int32_t itemCount = swiperPattern->RealTotalCount();
    auto allPointDiameterSum = itemWidth * static_cast<float>(itemCount - 1) + selectedItemWidth;
    auto allPointSpaceSum = static_cast<float>(INDICATOR_ITEM_SPACE.ConvertToPx() * (itemCount - 1));
    Dimension paddingSide = theme->GetIndicatorPaddingDot();
    auto indicatorPadding = static_cast<float>(paddingSide.ConvertToPx());
    auto contentWidth = indicatorPadding + allPointDiameterSum + allPointSpaceSum + indicatorPadding;
    auto geometryNode = host->GetGeometryNode();
    CHECK_NULL_RETURN(geometryNode, 0.0f);
    auto frameSize = geometryNode->GetFrameSize();
    auto axis = swiperPattern->GetDirection();
    return ((axis == Axis::HORIZONTAL ? frameSize.Width() : frameSize.Height()) - contentWidth) * 0.5f;
}

void SwiperIndicatorPattern::DumpAdvanceInfo()
{
    isHover_ ? DumpLog::GetInstance().AddDesc("isHover:true") : DumpLog::GetInstance().AddDesc("isHover:false");
    isPressed_ ? DumpLog::GetInstance().AddDesc("isPressed:true") : DumpLog::GetInstance().AddDesc("isPressed:false");
    isClicked_ ? DumpLog::GetInstance().AddDesc("isClicked:true") : DumpLog::GetInstance().AddDesc("isClicked:false");
    isRepeatClicked_ ? DumpLog::GetInstance().AddDesc("isRepeatClicked:true")
                     : DumpLog::GetInstance().AddDesc("isRepeatClicked:false");
    switch (swiperIndicatorType_) {
        case SwiperIndicatorType::DOT: {
            DumpLog::GetInstance().AddDesc("SwiperIndicatorType:DOT");
            break;
        }
        case SwiperIndicatorType::DIGIT: {
            DumpLog::GetInstance().AddDesc("SwiperIndicatorType:DIGIT");
            break;
        }
        case SwiperIndicatorType::ARC_DOT: {
            DumpLog::GetInstance().AddDesc("SwiperIndicatorType:ARC_DOT");
            break;
        }
        default: {
            break;
        }
    }
}

bool SwiperIndicatorPattern::SetArcIndicatorHotRegion(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config)
{
    if (config.skipMeasure && config.skipLayout) {
        return false;
    }

    const auto& indicatorGeometryNode = dirty->GetGeometryNode();
    CHECK_NULL_RETURN(indicatorGeometryNode, false);
    const auto& frameRect = indicatorGeometryNode->GetFrameRect();
    if (NonPositive(frameRect.Width()) || NonPositive(frameRect.Height())) {
        return false;
    }
    return CalculateArcIndicatorHotRegion(frameRect, indicatorGeometryNode->GetContentOffset());
}

bool SwiperIndicatorPattern::CalculateArcIndicatorHotRegion(const RectF& frameRect, const OffsetF& contentOffset)
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    auto gestureHub = host->GetOrCreateGestureEventHub();
    CHECK_NULL_RETURN(gestureHub, false);
    auto swiperNode = DynamicCast<FrameNode>(host->GetParent());
    CHECK_NULL_RETURN(swiperNode, false);
    auto swiperPattern = swiperNode->GetPattern<SwiperPattern>();
    CHECK_NULL_RETURN(swiperPattern, false);
    int32_t itemCount = swiperPattern->RealTotalCount();
    if (itemCount <= 0) {
        return false;
    }
    auto allPointArcAngle = (itemCount - 1) * ITEM_PADDING + ACTIVE_ITEM_ANGLE + ADD_HOT_REG_ANGLE;
    auto centerX = frameRect.Width() * 0.5 + contentOffset.GetX();
    auto centerY = frameRect.Height() * 0.5 + contentOffset.GetY();
    auto minEdgeLength = LessNotEqual(frameRect.Width(), frameRect.Height()) ? frameRect.Width() : frameRect.Height();
    if (LessOrEqual(minEdgeLength, CIRCLE_DIAMETER_OFFSET.ConvertToPx())) {
        return false;
    }
    auto radius = (minEdgeLength - CIRCLE_DIAMETER_OFFSET.ConvertToPx()) * 0.5;
    const auto& parameter = swiperPattern->GetSwiperArcDotParameters();
    CHECK_NULL_RETURN(parameter, false);
    SwiperArcDirection arcDirection = parameter->arcDirection.value_or(SwiperArcDirection::SIX_CLOCK_DIRECTION);
    std::vector<DimensionRect> responseRegion;
    int32_t allAngleCount = static_cast<int32_t>(allPointArcAngle);
    for (int32_t i = 0; i <= allAngleCount; i++) {
        double angle = 0.0;
        // The number 0.5 represents equal division
        if (arcDirection == SwiperArcDirection::THREE_CLOCK_DIRECTION) {
            angle = - allPointArcAngle * 0.5 + i;
        } else if (arcDirection == SwiperArcDirection::NINE_CLOCK_DIRECTION) {
            angle = HALF_CIRCLE_ANGLE - allPointArcAngle * 0.5 + i;
        } else {
            angle = QUARTER_CIRCLE_ANGLE - allPointArcAngle * 0.5 + i;
        }
        if (LessNotEqual(angle, 0.0)) {
            angle = FULL_CIRCLE_ANGLE + angle;
        }

        OffsetF angleOffset = CalculateAngleOffset(centerX, centerY, radius, angle);
        Dimension width;
        Dimension height;
        OffsetF hotRegionOffset = CalculateRectLayout(angle, radius, angleOffset, width, height);

        DimensionRect responseRect(width, height, DimensionOffset(hotRegionOffset));
        responseRegion.emplace_back(responseRect);
    }

    gestureHub->SetResponseRegion(responseRegion);
    return true;
}

OffsetF SwiperIndicatorPattern::CalculateAngleOffset(float centerX, float centerY, float radius, double angle)
{
    double radians = 0.0;
    OffsetF angleOffset;
    if (GreatOrEqual(angle, 0.0) && LessNotEqual(angle, QUARTER_CIRCLE_ANGLE)) {
        radians = std::abs(angle) * M_PI / HALF_CIRCLE_ANGLE;
        angleOffset.SetX(centerX + cos(radians) * radius);
        angleOffset.SetY(centerY + sin(radians) * radius);
    } else if (GreatOrEqual(angle, QUARTER_CIRCLE_ANGLE) && LessNotEqual(angle, HALF_CIRCLE_ANGLE)) {
        radians = std::abs(HALF_CIRCLE_ANGLE - angle) * M_PI / HALF_CIRCLE_ANGLE;
        angleOffset.SetX(centerX - cos(radians) * radius);
        angleOffset.SetY(centerY + sin(radians) * radius);
    } else if (GreatOrEqual(angle, HALF_CIRCLE_ANGLE) && LessNotEqual(angle, THREE_QUARTER_CIRCLE_ANGLE)) {
        radians = std::abs(angle - HALF_CIRCLE_ANGLE) * M_PI / HALF_CIRCLE_ANGLE;
        angleOffset.SetX(centerX - cos(radians) * radius);
        angleOffset.SetY(centerY - sin(radians) * radius);
    } else if (GreatOrEqual(angle, THREE_QUARTER_CIRCLE_ANGLE) && LessNotEqual(angle, FULL_CIRCLE_ANGLE)) {
        radians = std::abs(FULL_CIRCLE_ANGLE - angle) * M_PI / HALF_CIRCLE_ANGLE;
        angleOffset.SetX(centerX + cos(radians) * radius);
        angleOffset.SetY(centerY - sin(radians) * radius);
    }
    return angleOffset;
}

OffsetF SwiperIndicatorPattern::CalculateRectLayout(
    double angle, float radius, OffsetF angleOffset, Dimension& width, Dimension& height)
{
    OffsetF hotRegionOffset = angleOffset;
    Dimension oneAngleLength = Dimension(sin(M_PI / HALF_CIRCLE_ANGLE) * radius, DimensionUnit::PX);
    // The number 0.5 represents equal division
    if ((GreatOrEqual(angle, QUARTER_CIRCLE_ANGLE * 0.5) &&
            LessNotEqual(angle, QUARTER_CIRCLE_ANGLE * 0.5 + QUARTER_CIRCLE_ANGLE)) ||
        (GreatOrEqual(angle, QUARTER_CIRCLE_ANGLE * 0.5 + HALF_CIRCLE_ANGLE) &&
            LessNotEqual(angle, QUARTER_CIRCLE_ANGLE * 0.5 + THREE_QUARTER_CIRCLE_ANGLE))) {
        hotRegionOffset.SetY(angleOffset.GetY() - CONTAINER_BORDER_WIDTH.ConvertToPx() * 0.5);
        width = oneAngleLength;
        height = CONTAINER_BORDER_WIDTH;
    } else {
        hotRegionOffset.SetX(angleOffset.GetX() - CONTAINER_BORDER_WIDTH.ConvertToPx() * 0.5);
        width = CONTAINER_BORDER_WIDTH;
        height = oneAngleLength;
    }
    return hotRegionOffset;
}
} // namespace OHOS::Ace::NG

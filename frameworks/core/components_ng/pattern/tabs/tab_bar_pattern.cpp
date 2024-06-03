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

#include "core/components_ng/pattern/tabs/tab_bar_pattern.h"

#include <optional>

#include "base/geometry/axis.h"
#include "base/geometry/dimension.h"
#include "base/geometry/ng/offset_t.h"
#include "base/geometry/ng/size_t.h"
#include "base/log/dump_log.h"
#include "base/memory/ace_type.h"
#include "base/utils/utils.h"
#include "core/components/common/layout/constants.h"
#include "core/components_ng/pattern/scrollable/scrollable.h"
#include "core/components/tab_bar/tab_theme.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/inspector_filter.h"
#include "core/components_ng/pattern/image/image_layout_property.h"
#include "core/components_ng/pattern/image/image_pattern.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/pattern/scroll/scroll_spring_effect.h"
#include "core/components_ng/pattern/swiper/swiper_event_hub.h"
#include "core/components_ng/pattern/swiper/swiper_model.h"
#include "core/components_ng/pattern/swiper/swiper_pattern.h"
#include "core/components_ng/pattern/tabs/tabs_layout_property.h"
#include "core/components_ng/pattern/tabs/tabs_node.h"
#include "core/components_ng/pattern/tabs/tabs_pattern.h"
#include "core/components_ng/pattern/tabs/tab_content_model_ng.h"
#include "core/components_ng/pattern/text/text_layout_property.h"
#include "core/components_ng/pattern/symbol/constants.h"
#include "core/components_ng/pattern/symbol/symbol_effect_options.h"
#include "core/components_ng/property/property.h"
#include "core/components_ng/property/safe_area_insets.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/pipeline_ng/pipeline_context.h"
#include "base/perfmonitor/perf_constants.h"
#include "base/perfmonitor/perf_monitor.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"
#include "core/components_ng/pattern/text/text_pattern.h"
#include "core/components/toast/toast_theme.h"
#include "core/components/text_field/textfield_theme.h"
#include "core/components_ng/pattern/app_bar/app_bar_theme.h"
namespace OHOS::Ace::NG {
namespace {
constexpr int8_t LEFT_GRADIENT = 0;
constexpr int8_t RIGHT_GRADIENT = 1;
constexpr int8_t TOP_GRADIENT = 2;
constexpr int8_t BOTTOM_GRADIENT = 3;
constexpr float HALF_PROGRESS = 0.5f;
constexpr float FULL_PROGRESS = 1.0f;
constexpr float HALF_MASK_RADIUS_RATIO = 0.717f;
constexpr float FULL_MASK_RADIUS_RATIO = 1.414f;
constexpr float INVALID_RATIO = -1.0f;
constexpr uint16_t MASK_ANIMATION_DURATION = 200;
constexpr int8_t MASK_COUNT = 2;
constexpr float FULL_OPACITY = 1.0f;
constexpr float NEAR_FULL_OPACITY = 0.99f;
constexpr float NO_OPACITY = 0.0f;
constexpr float TEXT_COLOR_THREDHOLD = 0.673f;
constexpr int8_t HALF_OF_WIDTH = 2;

constexpr float BIG_FONT_SIZE_SCALE = 1.75f;
constexpr float LARGE_FONT_SIZE_SCALE = 2.0f;
constexpr float MAX_FONT_SIZE_SCALE = 3.2f;
constexpr double BIG_DIALOG_WIDTH = 216.0;
constexpr double MAX_DIALOG_WIDTH = 256.0;
constexpr int8_t GRIDCOUNT = 2;
constexpr int8_t MAXLINES = 6;
constexpr float MAX_FLING_VELOCITY = 4200.0f;
const auto DurationCubicCurve = AceType::MakeRefPtr<CubicCurve>(0.2f, 0.0f, 0.1f, 1.0f);
} // namespace

void FindTextAndImageNode(
    const RefPtr<FrameNode>& columnNode, RefPtr<FrameNode>& textNode, RefPtr<FrameNode>& imageNode)
{
    if (columnNode->GetTag() == V2::TEXT_ETS_TAG) {
        textNode = columnNode;
    } else if (columnNode->GetTag() == V2::IMAGE_ETS_TAG) {
        imageNode = columnNode;
    } else {
        std::list<RefPtr<UINode>> children = columnNode->GetChildren();
        for (auto child : children) {
            FindTextAndImageNode(AceType::DynamicCast<FrameNode>(child), textNode, imageNode);
        }
    }
}

void CreateDialogIconNode(const RefPtr<FrameNode> &columnNode, const RefPtr<PipelineBase> &pipelineContext,
    const RefPtr<FrameNode>& imageNode)
{
    auto imageLayoutProperty = imageNode->GetLayoutProperty<ImageLayoutProperty>();
    CHECK_NULL_VOID(imageLayoutProperty);
    auto imageSourceInfo = imageLayoutProperty->GetImageSourceInfo().value_or(ImageSourceInfo());
    auto tabTheme = pipelineContext->GetTheme<TabTheme>();
    CHECK_NULL_VOID(tabTheme);
    auto color = tabTheme->GetDialogIconColor();
    imageSourceInfo.SetFillColor(Color(color.GetValue()));
    auto iconNode = FrameNode::GetOrCreateFrameNode(V2::IMAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        []() { return AceType::MakeRefPtr<ImagePattern>(); });
    auto iconProps = iconNode->GetLayoutProperty<ImageLayoutProperty>();
    CHECK_NULL_VOID(iconProps);
    iconProps->UpdateUserDefinedIdealSize(CalcSize(CalcLength(64.0_vp), CalcLength(64.0_vp)));
    iconProps->UpdateImageFit(ImageFit::FILL);
    iconProps->UpdateImageSourceInfo(imageSourceInfo);
    // add image margin
    MarginProperty margin = {
        .top = CalcLength(48.0_vp),
        .bottom = CalcLength(16.0_vp),
    };
    iconProps->UpdateMargin(margin);
    iconNode->MountToParent(columnNode);
    iconNode->MarkModifyDone();
}

void CreateDialogTextNode(const RefPtr<FrameNode>& columnNode, const RefPtr<PipelineBase> &pipelineContext,
    const RefPtr<FrameNode>& textNode)
{
    auto textLayoutProperty = textNode->GetLayoutProperty<TextLayoutProperty>();
    CHECK_NULL_VOID(textLayoutProperty);
    auto textValue = textLayoutProperty->GetContent();
    
    auto dialogTextNode = FrameNode::CreateFrameNode(
        V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
    CHECK_NULL_VOID(dialogTextNode);
    auto titleProp = AceType::DynamicCast<TextLayoutProperty>(dialogTextNode->GetLayoutProperty());
    CHECK_NULL_VOID(titleProp);
    titleProp->UpdateContent(textValue.value_or(""));
    titleProp->UpdateTextOverflow(TextOverflow::ELLIPSIS);
    titleProp->UpdateMaxLines(MAXLINES);
    MarginProperty margin = {
        .left = CalcLength(8.0_vp),
        .right = CalcLength(8.0_vp),
        .bottom = CalcLength(24.0_vp),
    };
    titleProp->UpdateMargin(margin);
    auto tabTheme = pipelineContext->GetTheme<TabTheme>();
    auto color = tabTheme->GetDialogFontColor();
    titleProp->UpdateTextColor(Color(color.GetValue()));
    columnNode->AddChild(dialogTextNode);
}

void TabBarPattern::OnAttachToFrameNode()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto renderContext = host->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    renderContext->SetClipToFrame(true);
    host->GetLayoutProperty()->UpdateSafeAreaExpandOpts(
        SafeAreaExpandOpts { .type = SAFE_AREA_TYPE_SYSTEM, .edges = SAFE_AREA_EDGE_BOTTOM });
    swiperController_->SetTabBarFinishCallback([weak = WeakClaim(this)]() {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        // always swipe with physical curve, ignore animationDuration
        pattern->SetSwiperCurve(TabBarPhysicalCurve);

        CHECK_NULL_VOID(pattern && pattern->scrollableEvent_);
        auto scrollable = pattern->scrollableEvent_->GetScrollable();
        if (scrollable) {
            scrollable->StopScrollable();
        }
    });
    InitSurfaceChangedCallback();
}

void TabBarPattern::InitSurfaceChangedCallback()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto pipeline = host->GetContextRefPtr();
    CHECK_NULL_VOID(pipeline);
    if (!HasSurfaceChangedCallback()) {
        auto callbackId = pipeline->RegisterSurfaceChangedCallback(
            [weak = WeakClaim(this)](int32_t newWidth, int32_t newHeight, int32_t prevWidth, int32_t prevHeight,
                WindowSizeChangeReason type) {
                auto pattern = weak.Upgrade();
                if (!pattern) {
                    return;
                }

                pattern->windowSizeChangeReason_ = type;
                pattern->prevRootSize_ = std::make_pair(prevWidth, prevHeight);

                if (type == WindowSizeChangeReason::ROTATION) {
                    pattern->windowSizeChangeReason_ = type;
                    pattern->StopTranslateAnimation();
                }
            });
        UpdateSurfaceChangedCallbackId(callbackId);
    }
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

void TabBarPattern::InitLongPressEvent(const RefPtr<GestureEventHub>& gestureHub)
{
    if (longPressEvent_) {
        return;
    }

    auto longPressTask = [weak = WeakClaim(this)](GestureEvent& info) {
        auto tabBar = weak.Upgrade();
        if (tabBar) {
            tabBar->HandleLongPressEvent(info);
        }
    };
    longPressEvent_ = AceType::MakeRefPtr<LongPressEvent>(std::move(longPressTask));
    gestureHub->SetLongPressEvent(longPressEvent_);
}

void TabBarPattern::InitDragEvent(const RefPtr<GestureEventHub>& gestureHub)
{
    CHECK_NULL_VOID(!dragEvent_);
    auto actionUpdateTask = [weak = WeakClaim(this)](const GestureEvent& info) {
        auto tabBar = weak.Upgrade();
        auto index = tabBar->CalculateSelectedIndex(info.GetLocalLocation());
        auto host = tabBar->GetHost();
        CHECK_NULL_VOID(host);
        auto totalCount = host->TotalChildCount() - MASK_COUNT;
        if (tabBar && tabBar->dialogNode_ && index >= 0 && index < totalCount) {
            if (!tabBar->moveIndex_.has_value()) {
                tabBar->moveIndex_ = index;
            }

            if (tabBar->moveIndex_ != index) {
                tabBar->CloseDialog();
                tabBar->moveIndex_ = index;
                tabBar->ShowDialogWithNode(index);
            }
        }
    };
    dragEvent_ = MakeRefPtr<DragEvent>(nullptr, std::move(actionUpdateTask), nullptr, nullptr);
    PanDirection panDirection = { .type = PanDirection::ALL };
    gestureHub->SetDragEvent(dragEvent_, panDirection, DEFAULT_PAN_FINGER, DEFAULT_PAN_DISTANCE);
}

void TabBarPattern::InitScrollable(const RefPtr<GestureEventHub>& gestureHub)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto layoutProperty = host->GetLayoutProperty<TabBarLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);
    auto axis = layoutProperty->GetAxis().value_or(Axis::HORIZONTAL);
    if (axis_ == axis && scrollableEvent_) {
        return;
    }

    axis_ = axis;
    auto task = [weak = WeakClaim(this)](double offset, int32_t source) {
        if (source == SCROLL_FROM_START) {
            return true;
        }
        auto pattern = weak.Upgrade();
        CHECK_NULL_RETURN(pattern, false);

        if (pattern->IsOutOfBoundary()) {
            // over scroll in drag update from normal to over scroll or during over scroll.
            auto scrollable = pattern->scrollableEvent_->GetScrollable();
            if (scrollable) {
                scrollable->SetCanOverScroll(true);
            }
            if (pattern->tabItemOffsets_.empty()) {
                return false;
            }

            auto host = pattern->GetHost();
            CHECK_NULL_RETURN(host, false);
            auto geometryNode = host->GetGeometryNode();
            CHECK_NULL_RETURN(geometryNode, false);
            auto overScrollInfo = pattern->GetOverScrollInfo(geometryNode->GetPaddingSize());
            if (source == SCROLL_FROM_UPDATE) {
                // adjust offset.
                if (overScrollInfo.second != 0.0f) {
                    auto friction = CalculateFriction(std::abs(overScrollInfo.first) / overScrollInfo.second);
                    pattern->UpdateCurrentOffset(static_cast<float>(offset * friction));
                }
                return true;
            }
        }
        if (source == SCROLL_FROM_AXIS) {
            pattern->AdjustOffset(offset);
        }
        pattern->UpdateCurrentOffset(static_cast<float>(offset));
        return true;
    };

    if (scrollableEvent_) {
        gestureHub->RemoveScrollableEvent(scrollableEvent_);
    }

    scrollableEvent_ = MakeRefPtr<ScrollableEvent>(axis);
    auto scrollable = MakeRefPtr<Scrollable>(task, axis);
    scrollable->SetNodeId(host->GetAccessibilityId());
    scrollable->Initialize(host->GetContextRefPtr());
    scrollable->SetMaxFlingVelocity(MAX_FLING_VELOCITY);
    auto renderContext = host->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    auto springProperty = scrollable->GetSpringProperty();
    renderContext->AttachNodeAnimatableProperty(springProperty);
    auto frictionProperty = scrollable->GetFrictionProperty();
    renderContext->AttachNodeAnimatableProperty(frictionProperty);
    scrollableEvent_->SetScrollable(scrollable);
    gestureHub->AddScrollableEvent(scrollableEvent_);
    scrollableEvent_->GetScrollable()->SetEdgeEffect(EdgeEffect::SPRING);
}

std::pair<float, float> TabBarPattern::GetOverScrollInfo(const SizeF& size)
{
    auto overScroll = 0.0f;
    auto mainSize = 0.0f;
    if (axis_ == Axis::HORIZONTAL) {
        auto startPos = tabItemOffsets_.front().GetX() - scrollMargin_ - GetLeftPadding();
        mainSize = size.Width();
        if (Positive(startPos)) {
            overScroll = startPos;
        } else {
            overScroll = mainSize + GetLeftPadding() - tabItemOffsets_.back().GetX() - scrollMargin_;
        }
    } else {
        auto startPos = tabItemOffsets_.front().GetY();
        mainSize = size.Height();
        if (Positive(startPos)) {
            overScroll = startPos;
        } else {
            overScroll = mainSize - tabItemOffsets_.back().GetY();
        }
    }
    return std::make_pair(overScroll, mainSize);
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
    auto totalCount = host->TotalChildCount() - MASK_COUNT;
    if (totalCount < 0) {
        return;
    }
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
        if (hoverIndex_.has_value()) {
            HandleMoveAway(hoverIndex_.value());
        }
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
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, false);
    if (!pipeline->GetIsFocusActive()) {
        return false;
    }
    isFirstFocus_ = false;
    if (event.action != KeyAction::DOWN) {
        return false;
    }
    if (tabBarStyle_ == TabBarStyle::BOTTOMTABBATSTYLE || tabBarStyle_ == TabBarStyle::SUBTABBATSTYLE) {
        return OnKeyEventWithoutClick(event);
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
        if (indicator >= host->TotalChildCount() - MASK_COUNT - 1) {
            return false;
        }
        indicator += 1;
        FocusIndexChange(indicator);
        return true;
    }
    if (event.code == KeyCode::KEY_MOVE_HOME) {
        indicator = 0;
        FocusIndexChange(indicator);
        return true;
    }
    if (event.code == KeyCode::KEY_MOVE_END) {
        indicator = host->TotalChildCount() - MASK_COUNT - 1;
        FocusIndexChange(indicator);
        return true;
    }
    return false;
}

bool TabBarPattern::OnKeyEventWithoutClick(const KeyEvent& event)
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    auto tabBarLayoutProperty = GetLayoutProperty<TabBarLayoutProperty>();
    if (event.code == (tabBarLayoutProperty->GetAxisValue(Axis::HORIZONTAL) == Axis::HORIZONTAL
                              ? KeyCode::KEY_DPAD_LEFT
                              : KeyCode::KEY_DPAD_UP) ||
        event.IsShiftWith(KeyCode::KEY_TAB)) {
        if (focusIndicator_ <= 0) {
            return false;
        }
        if (!ContentWillChange(focusIndicator_ - 1)) {
            return true;
        }
        focusIndicator_ -= 1;
        PaintFocusState();
        return true;
    }
    if (event.code == (tabBarLayoutProperty->GetAxisValue(Axis::HORIZONTAL) == Axis::HORIZONTAL
                              ? KeyCode::KEY_DPAD_RIGHT
                              : KeyCode::KEY_DPAD_DOWN) ||
        event.code == KeyCode::KEY_TAB) {
        if (focusIndicator_ >= host->TotalChildCount() - MASK_COUNT - 1) {
            return false;
        }
        if (!ContentWillChange(focusIndicator_ + 1)) {
            return true;
        }
        focusIndicator_ += 1;
        PaintFocusState();
        return true;
    }
    return OnKeyEventWithoutClick(host, event);
}

bool TabBarPattern::OnKeyEventWithoutClick(const RefPtr<FrameNode>& host, const KeyEvent& event)
{
    if (event.code == KeyCode::KEY_MOVE_HOME) {
        if (!ContentWillChange(0)) {
            return true;
        }
        focusIndicator_ = 0;
        PaintFocusState();
        return true;
    }
    if (event.code == KeyCode::KEY_MOVE_END) {
        if (!ContentWillChange(host->TotalChildCount() - MASK_COUNT - 1)) {
            return true;
        }
        focusIndicator_ = host->TotalChildCount() - MASK_COUNT - 1;
        PaintFocusState();
        return true;
    }
    if (event.code == KeyCode::KEY_SPACE || event.code == KeyCode::KEY_ENTER) {
        TabBarClickEvent(focusIndicator_);
        FocusIndexChange(focusIndicator_);
        return true;
    }
    return false;
}

void TabBarPattern::FocusIndexChange(int32_t index)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto tabsNode = AceType::DynamicCast<TabsNode>(host->GetParent());
    CHECK_NULL_VOID(tabsNode);
    auto tabsPattern = tabsNode->GetPattern<TabsPattern>();
    CHECK_NULL_VOID(tabsPattern);
    auto tabBarLayoutProperty = GetLayoutProperty<TabBarLayoutProperty>();
    CHECK_NULL_VOID(tabBarLayoutProperty);
    if (!ContentWillChange(indicator_, index)) {
        return;
    }
    if (tabsPattern->GetIsCustomAnimation()) {
        OnCustomContentTransition(indicator_, index);
        tabBarLayoutProperty->UpdateIndicator(index);
        PaintFocusState(false);
    } else {
        if (GetAnimationDuration().has_value()
            && tabsPattern->GetAnimateMode() != TabAnimateMode::NO_ANIMATION) {
            swiperController_->SwipeTo(index);
        } else {
            swiperController_->SwipeToWithoutAnimation(index);
        }

        tabBarLayoutProperty->UpdateIndicator(index);
        PaintFocusState();
    }

    UpdateTextColorAndFontWeight(index);
}

void TabBarPattern::GetInnerFocusPaintRect(RoundRect& paintRect)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto tabBarLayoutProperty = GetLayoutProperty<TabBarLayoutProperty>();
    CHECK_NULL_VOID(tabBarLayoutProperty);
    auto indicator = tabBarLayoutProperty->GetIndicatorValue(0);
    if (tabBarStyle_ == TabBarStyle::BOTTOMTABBATSTYLE || tabBarStyle_ == TabBarStyle::SUBTABBATSTYLE) {
        if (isFirstFocus_) {
            focusIndicator_ = indicator;
        } else {
            indicator = focusIndicator_;
        }
    }
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

void TabBarPattern::PaintFocusState(bool needMarkDirty)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);

    RoundRect focusRect;
    GetInnerFocusPaintRect(focusRect);

    auto focusHub = host->GetFocusHub();
    CHECK_NULL_VOID(focusHub);
    focusHub->PaintInnerFocusState(focusRect);

    if (needMarkDirty) {
        host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
    }
}

void TabBarPattern::OnModifyDone()
{
    Pattern::OnModifyDone();
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto hub = host->GetEventHub<EventHub>();
    CHECK_NULL_VOID(hub);
    auto gestureHub = hub->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(gestureHub);

    InitClick(gestureHub);
    InitTurnPageRateEvent();
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    if (Container::GreatOrEqualAPITargetVersion(PlatformVersion::VERSION_TWELVE)) {
        auto tabBarPaintProperty = host->GetPaintProperty<TabBarPaintProperty>();
        CHECK_NULL_VOID(tabBarPaintProperty);
        auto theme = pipelineContext->GetTheme<TabTheme>();
        CHECK_NULL_VOID(theme);
        auto defaultBlurStyle = static_cast<BlurStyle>(theme->GetBottomTabBackgroundBlurStyle());
        tabBarPaintProperty->UpdateTabBarBlurStyle(defaultBlurStyle);
    }
    auto layoutProperty = host->GetLayoutProperty<TabBarLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);
    if (layoutProperty->GetTabBarModeValue(TabBarMode::FIXED) == TabBarMode::SCROLLABLE) {
        InitScrollable(gestureHub);
        SetEdgeEffect(gestureHub);
    }
    InitTouch(gestureHub);
    InitHoverEvent();
    InitMouseEvent();
    auto focusHub = host->GetFocusHub();
    CHECK_NULL_VOID(focusHub);
    InitOnKeyEvent(focusHub);
    SetAccessibilityAction();
    UpdateSubTabBoard();
    needSetCentered_ = true;

    CHECK_NULL_VOID(swiperController_);
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
        tabBarPattern->isTouchingSwiper_ = true;
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

    auto surfaceChangeCallback = [weak = WeakClaim(this)]() {
        auto tabBarPattern = weak.Upgrade();
        CHECK_NULL_VOID(tabBarPattern);
        tabBarPattern->isTouchingSwiper_ = false;
    };
    swiperController_->SetSurfaceChangeCallback(std::move(surfaceChangeCallback));
}

void TabBarPattern::UpdatePaintIndicator(int32_t indicator, bool needMarkDirty)
{
    auto tabBarNode = GetHost();
    CHECK_NULL_VOID(tabBarNode);
    auto tabBarPattern = tabBarNode->GetPattern<TabBarPattern>();
    CHECK_NULL_VOID(tabBarPattern);
    auto paintProperty = GetPaintProperty<TabBarPaintProperty>();
    if (indicator_ >= static_cast<int32_t>(tabBarStyles_.size())) {
        return;
    }

    auto layoutProperty = GetLayoutProperty<TabBarLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);
    if (tabBarPattern->IsContainsBuilder() || layoutProperty->GetAxis() == Axis::VERTICAL ||
        tabBarStyles_[indicator] == TabBarStyle::BOTTOMTABBATSTYLE) {
        paintProperty->UpdateIndicator({});

        if (needMarkDirty) {
            tabBarNode->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
        }

        return;
    }

    RectF rect = layoutProperty->GetIndicatorRect(indicator);
    paintProperty->UpdateIndicator(rect);
    if (!isTouchingSwiper_ || tabBarStyles_[indicator] != TabBarStyle::SUBTABBATSTYLE) {
        currentIndicatorOffset_ = rect.GetX() + rect.Width() / 2;

        if (needMarkDirty) {
            tabBarNode->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
        }
    }
    if (tabBarStyles_[indicator] == TabBarStyle::SUBTABBATSTYLE) {
        UpdateSubTabBoard();
    }
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
    scrollMargin_ = tabBarLayoutAlgorithm->GetScrollMargin();
    auto layoutProperty = DynamicCast<TabBarLayoutProperty>(dirty->GetLayoutProperty());
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    auto tabsFrameNode = AceType::DynamicCast<TabsNode>(host->GetParent());
    CHECK_NULL_RETURN(tabsFrameNode, false);
    auto swiperFrameNode = AceType::DynamicCast<FrameNode>(tabsFrameNode->GetTabs());
    CHECK_NULL_RETURN(swiperFrameNode, false);
    auto swiperPattern = swiperFrameNode->GetPattern<SwiperPattern>();
    CHECK_NULL_RETURN(swiperPattern, false);
    int32_t indicator = swiperPattern->GetCurrentIndex();
    int32_t totalCount = swiperPattern->TotalCount();
    if (indicator > totalCount - 1 || indicator < 0) {
        indicator = 0;
    }

    if (swiperPattern->IsUseCustomAnimation()) {
        UpdatePaintIndicator(indicator, false);
    }

    if ((!swiperPattern->IsUseCustomAnimation() || isFirstLayout_) && !isAnimating_ && !IsMaskAnimationExecuted()) {
        UpdateIndicator(indicator);
    }
    isFirstLayout_ = false;

    if (windowSizeChangeReason_) {
        if (*windowSizeChangeReason_ == WindowSizeChangeReason::ROTATION && animationTargetIndex_.has_value() &&
            animationTargetIndex_ != indicator) {
            swiperController_->SwipeToWithoutAnimation(animationTargetIndex_.value());
            animationTargetIndex_.reset();
            windowSizeChangeReason_.reset();
        } else if (prevRootSize_.first != PipelineContext::GetCurrentRootWidth() ||
            prevRootSize_.second != PipelineContext::GetCurrentRootHeight()) {
            // UNDEFINED currently implies window change on foldable
            PlayTabBarTranslateAnimation(indicator_);
            UpdateIndicator(indicator_);
            windowSizeChangeReason_.reset();
        }
    }
    UpdateGradientRegions(!swiperPattern->IsUseCustomAnimation());
    if (!swiperPattern->IsUseCustomAnimation() && isTouchingSwiper_ &&
        layoutProperty->GetTabBarModeValue(TabBarMode::FIXED) == TabBarMode::SCROLLABLE) {
        ApplyTurnPageRateToIndicator(turnPageRate_);
    }
    return false;
}

void TabBarPattern::InitLongPressAndDragEvent()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto hub = host->GetEventHub<EventHub>();
    CHECK_NULL_VOID(hub);
    auto gestureHub = hub->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(gestureHub);
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    float scale = pipelineContext->GetFontScale();
    if (tabBarStyle_ == TabBarStyle::BOTTOMTABBATSTYLE && scale >= BIG_FONT_SIZE_SCALE) {
        InitLongPressEvent(gestureHub);
        InitDragEvent(gestureHub);
    } else {
        gestureHub->RemoveDragEvent();
        gestureHub->SetLongPressEvent(nullptr);
        longPressEvent_ = nullptr;
        dragEvent_ = nullptr;
    }
}

void TabBarPattern::HandleLongPressEvent(const GestureEvent& info)
{
    auto index = CalculateSelectedIndex(info.GetLocalLocation());
    HandleClick(info);
    ShowDialogWithNode(index);
}

void TabBarPattern::ShowDialogWithNode(int32_t index)
{
    auto tabBarNode = GetHost();
    CHECK_NULL_VOID(tabBarNode);
    auto columnNode = AceType::DynamicCast<FrameNode>(tabBarNode->GetChildAtIndex(index));
    CHECK_NULL_VOID(columnNode);
    RefPtr<FrameNode> imageNode = nullptr;
    RefPtr<FrameNode> textNode = nullptr;
    FindTextAndImageNode(columnNode, textNode, imageNode);
    CHECK_NULL_VOID(imageNode);
    CHECK_NULL_VOID(textNode);

    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto context = AceType::DynamicCast<NG::PipelineContext>(pipelineContext);
    CHECK_NULL_VOID(context);
    auto overlayManager = context->GetOverlayManager();
    CHECK_NULL_VOID(overlayManager);
    RefPtr<FrameNode> dialogColumnNode = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<LinearLayoutPattern>(true));
    CHECK_NULL_VOID(dialogColumnNode);
    CreateDialogIconNode(dialogColumnNode, pipelineContext, imageNode);
    CreateDialogTextNode(dialogColumnNode, pipelineContext, textNode);

    DialogProperties dialogProperties;
    dialogProperties.alignment = DialogAlignment::CENTER;
    dialogProperties.gridCount = GRIDCOUNT;
    dialogProperties.isModal = false;
    dialogProperties.backgroundColor = Color::TRANSPARENT;
    dialogProperties.shadow = Shadow::CreateShadow(ShadowStyle::OuterDefaultLG);
    BlurStyleOption styleOption;
    styleOption.blurStyle = static_cast<BlurStyle>(
        dialogProperties.backgroundBlurStyle.value_or(static_cast<int>(BlurStyle::COMPONENT_ULTRA_THICK)));
    auto renderContext = dialogColumnNode->GetRenderContext();
    renderContext->UpdateBackBlurStyle(styleOption);
    float scale = pipelineContext->GetFontScale();
    if (scale == BIG_FONT_SIZE_SCALE || scale == LARGE_FONT_SIZE_SCALE) {
        dialogProperties.width = CalcDimension(BIG_DIALOG_WIDTH, DimensionUnit::VP);
    } else if (scale >= MAX_FONT_SIZE_SCALE) {
        dialogProperties.width = CalcDimension(MAX_DIALOG_WIDTH, DimensionUnit::VP);
    }
    auto layoutProperty = dialogColumnNode->GetLayoutProperty();
    layoutProperty->UpdateMeasureType(MeasureType::MATCH_PARENT_CROSS_AXIS);
    dialogNode_ = overlayManager->ShowDialogWithNode(dialogProperties, dialogColumnNode, false);
}

void TabBarPattern::CloseDialog()
{
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto context = AceType::DynamicCast<NG::PipelineContext>(pipelineContext);
    CHECK_NULL_VOID(context);
    auto overlayManager = context->GetOverlayManager();
    CHECK_NULL_VOID(overlayManager);
    overlayManager->CloseDialog(dialogNode_);
    dialogNode_.Reset();
}

void TabBarPattern::HandleClick(const GestureEvent& info)
{
    PerfMonitor::GetPerfMonitor()->Start(PerfConstants::APP_TAB_SWITCH, PerfActionType::LAST_UP, "");
    if (info.GetSourceDevice() == SourceType::KEYBOARD) {
        return;
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto layoutProperty = host->GetLayoutProperty<TabBarLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);
    if (layoutProperty->GetTabBarModeValue(TabBarMode::FIXED) == TabBarMode::SCROLLABLE) {
        auto scrollable = scrollableEvent_->GetScrollable();
        if (scrollable && !scrollable->IsSpringStopped()) {
            if (IsOutOfBoundary()) {
                return;
            }
            scrollable->StopScrollable();
        }
    }
    if (tabItemOffsets_.empty()) {
        return;
    }

    auto totalCount = host->TotalChildCount() - MASK_COUNT;
    if (totalCount < 0) {
        return;
    }

    auto index = CalculateSelectedIndex(info.GetLocalLocation());
    TAG_LOGI(AceLogTag::ACE_TABS, "Clicked tabBarIndex: %{public}d, Clicked tabBarLocation: %{public}s", index,
        info.GetLocalLocation().ToString().c_str());
    if (index < 0 || index >= totalCount || !swiperController_ ||
        indicator_ >= static_cast<int32_t>(tabBarStyles_.size())) {
        return;
    }
    SetSwiperCurve(DurationCubicCurve);

    TabBarClickEvent(index);
    if (tabBarStyles_[indicator_] == TabBarStyle::SUBTABBATSTYLE &&
        tabBarStyles_[index] == TabBarStyle::SUBTABBATSTYLE &&
        layoutProperty->GetAxisValue(Axis::HORIZONTAL) == Axis::HORIZONTAL) {
        HandleSubTabBarClick(layoutProperty, index);
        return;
    }

    if (!ContentWillChange(index)) {
        return;
    }
    ClickTo(host, index);
    layoutProperty->UpdateIndicator(index);
}

void TabBarPattern::ClickTo(const RefPtr<FrameNode>& host, int32_t index)
{
    auto tabsNode = AceType::DynamicCast<TabsNode>(host->GetParent());
    CHECK_NULL_VOID(tabsNode);
    auto tabsPattern = tabsNode->GetPattern<TabsPattern>();
    CHECK_NULL_VOID(tabsPattern);
    if (tabsPattern->GetIsCustomAnimation()) {
        OnCustomContentTransition(indicator_, index);
    } else {
        if (GetAnimationDuration().has_value()
            && tabsPattern->GetAnimateMode() != TabAnimateMode::NO_ANIMATION) {
            swiperController_->SwipeTo(index);
            animationTargetIndex_ = index;
        } else {
            swiperController_->SwipeToWithoutAnimation(index);
        }
    }
}

void TabBarPattern::HandleBottomTabBarChange(int32_t index)
{
    AnimationUtils::CloseImplicitAnimation();
    auto preIndex = GetImageColorOnIndex().value_or(indicator_);
    UpdateImageColor(index);
    UpdateSymbolStats(index, preIndex);
    if (preIndex != index && (tabBarStyles_[preIndex] == TabBarStyle::BOTTOMTABBATSTYLE ||
                                   tabBarStyles_[index] == TabBarStyle::BOTTOMTABBATSTYLE)) {
        int32_t selectedIndex = -1;
        int32_t unselectedIndex = -1;
        if (tabBarStyles_[preIndex] == TabBarStyle::BOTTOMTABBATSTYLE && CheckSvg(preIndex)) {
            unselectedIndex = preIndex;
        }
        if (tabBarStyles_[index] == TabBarStyle::BOTTOMTABBATSTYLE && CheckSvg(index)) {
            selectedIndex = index;
        }
        HandleBottomTabBarClick(selectedIndex, unselectedIndex);
    }
}

bool TabBarPattern::CheckSvg(int32_t index) const
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    auto columnNode = AceType::DynamicCast<FrameNode>(host->GetChildAtIndex(index));
    CHECK_NULL_RETURN(columnNode, false);
    auto imageNode = AceType::DynamicCast<FrameNode>(columnNode->GetChildren().front());
    CHECK_NULL_RETURN(imageNode, false);
    auto imageLayoutProperty = imageNode->GetLayoutProperty<ImageLayoutProperty>();
    CHECK_NULL_RETURN(imageLayoutProperty, false);
    ImageSourceInfo info;
    auto imageSourceInfo = imageLayoutProperty->GetImageSourceInfo().value_or(info);
    return imageSourceInfo.IsSvg();
}

void TabBarPattern::HandleBottomTabBarClick(int32_t selectedIndex, int32_t unselectedIndex)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto layoutProperty = host->GetLayoutProperty<TabBarLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);

    std::vector<int32_t> selectedIndexes = {selectedIndex, unselectedIndex};
    OffsetF originalSelectedMaskOffset, originalUnselectedMaskOffset;
    float selectedImageSize = 0.0f, unselectedImageSize = 0.0f;
    for (int32_t maskIndex = 0; maskIndex < MASK_COUNT; maskIndex++) {
        if (maskIndex == 0) {
            layoutProperty->UpdateSelectedMask(selectedIndex);
        } else {
            layoutProperty->UpdateUnselectedMask(unselectedIndex);
        }
        if (selectedIndexes[maskIndex] < 0) {
            continue;
        }
        GetBottomTabBarImageSizeAndOffset(selectedIndexes, maskIndex, selectedImageSize, unselectedImageSize,
            originalSelectedMaskOffset, originalUnselectedMaskOffset);
    }
    ChangeMask(selectedIndex, selectedImageSize, originalSelectedMaskOffset, NO_OPACITY, HALF_MASK_RADIUS_RATIO, true);
    ChangeMask(unselectedIndex, unselectedImageSize, originalUnselectedMaskOffset, FULL_OPACITY,
        FULL_MASK_RADIUS_RATIO, false);

    host->MarkDirtyNode(PROPERTY_UPDATE_LAYOUT);
    PlayMaskAnimation(selectedImageSize, originalSelectedMaskOffset, selectedIndex, unselectedImageSize,
        originalUnselectedMaskOffset, unselectedIndex);
}

void TabBarPattern::GetBottomTabBarImageSizeAndOffset(const std::vector<int32_t>& selectedIndexes, int32_t maskIndex,
    float& selectedImageSize, float& unselectedImageSize, OffsetF& originalSelectedMaskOffset,
    OffsetF& originalUnselectedMaskOffset)
{
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto tabTheme = pipelineContext->GetTheme<TabTheme>();
    CHECK_NULL_VOID(tabTheme);

    auto host = GetHost();
    CHECK_NULL_VOID(host);

    auto columnNode = AceType::DynamicCast<FrameNode>(host->GetChildAtIndex(selectedIndexes[maskIndex]));
    CHECK_NULL_VOID(columnNode);
    auto imageNode = AceType::DynamicCast<FrameNode>(columnNode->GetChildren().front());
    CHECK_NULL_VOID(imageNode);
    auto imageGeometryNode = imageNode->GetGeometryNode();
    CHECK_NULL_VOID(imageGeometryNode);
    auto imageOffset = imageGeometryNode->GetFrameOffset();
    auto imageSize = imageGeometryNode->GetFrameSize().Width();
    if (maskIndex == 0) {
        selectedImageSize = imageSize;
    } else {
        unselectedImageSize = imageSize;
    }
    auto imageLayoutProperty = imageNode->GetLayoutProperty<ImageLayoutProperty>();
    CHECK_NULL_VOID(imageLayoutProperty);
    ImageSourceInfo info;
    auto imageSourceInfo = imageLayoutProperty->GetImageSourceInfo().value_or(info);

    auto maskPosition = host->GetChildren().size() - MASK_COUNT;
    if (maskPosition < 0) {
        return;
    }
    auto selectedMaskNode = AceType::DynamicCast<FrameNode>(host->GetChildAtIndex(maskPosition + maskIndex));
    CHECK_NULL_VOID(selectedMaskNode);
    if (maskIndex == 0) {
        originalSelectedMaskOffset = imageOffset;
    } else {
        originalUnselectedMaskOffset = imageOffset;
    }
    auto selectedImageNode = AceType::DynamicCast<FrameNode>(selectedMaskNode->GetChildren().front());
    CHECK_NULL_VOID(selectedImageNode);

    auto selectedImageLayoutProperty = selectedImageNode->GetLayoutProperty<ImageLayoutProperty>();
    CHECK_NULL_VOID(selectedImageLayoutProperty);
    UpdateBottomTabBarImageColor(selectedIndexes, maskIndex);
    selectedImageLayoutProperty->UpdateImageSourceInfo(imageSourceInfo);
    imageLayoutProperty->UpdateImageSourceInfo(imageSourceInfo);

    selectedImageNode->MarkModifyDone();
    selectedImageNode->MarkDirtyNode();
    imageNode->MarkModifyDone();
    imageNode->MarkDirtyNode();
}

void TabBarPattern::UpdateBottomTabBarImageColor(const std::vector<int32_t>& selectedIndexes, int32_t maskIndex)
{
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto tabTheme = pipelineContext->GetTheme<TabTheme>();
    CHECK_NULL_VOID(tabTheme);

    auto host = GetHost();
    CHECK_NULL_VOID(host);

    auto columnNode = AceType::DynamicCast<FrameNode>(host->GetChildAtIndex(selectedIndexes[maskIndex]));
    CHECK_NULL_VOID(columnNode);
    auto imageNode = AceType::DynamicCast<FrameNode>(columnNode->GetChildren().front());
    CHECK_NULL_VOID(imageNode);

    auto maskPosition = host->GetChildren().size() - MASK_COUNT;
    if (maskPosition < 0) {
        return;
    }
    auto selectedMaskNode = AceType::DynamicCast<FrameNode>(host->GetChildAtIndex(maskPosition + maskIndex));
    CHECK_NULL_VOID(selectedMaskNode);
    auto selectedImageNode = AceType::DynamicCast<FrameNode>(selectedMaskNode->GetChildren().front());
    CHECK_NULL_VOID(selectedImageNode);

    auto selectedImagePaintProperty = selectedImageNode->GetPaintProperty<ImageRenderProperty>();
    CHECK_NULL_VOID(selectedImagePaintProperty);
    auto unselectedImagePaintProperty = imageNode->GetPaintProperty<ImageRenderProperty>();
    CHECK_NULL_VOID(unselectedImagePaintProperty);
    if (selectedIndexes[maskIndex] >= 0 && selectedIndexes[maskIndex] < static_cast<int32_t>(iconStyles_.size())) {
        if (iconStyles_[selectedIndexes[maskIndex]].selectedColor.has_value()) {
            selectedImagePaintProperty->UpdateSvgFillColor(
                iconStyles_[selectedIndexes[maskIndex]].selectedColor.value());
        } else {
            selectedImagePaintProperty->UpdateSvgFillColor(tabTheme->GetBottomTabIconOn());
        }

        if (iconStyles_[selectedIndexes[maskIndex]].unselectedColor.has_value()) {
            unselectedImagePaintProperty->UpdateSvgFillColor(
                iconStyles_[selectedIndexes[maskIndex]].unselectedColor.value());
        } else {
            unselectedImagePaintProperty->UpdateSvgFillColor(tabTheme->GetBottomTabIconOff());
        }
    }
}

void TabBarPattern::PlayMaskAnimation(float selectedImageSize,
    const OffsetF& originalSelectedMaskOffset, int32_t selectedIndex, float unselectedImageSize,
    const OffsetF& originalUnselectedMaskOffset, int32_t unselectedIndex)
{
    auto curve = AceType::MakeRefPtr<CubicCurve>(0.4f, 0.0f, 0.2f, 1.0f);
    AnimationOption option;
    option.SetDuration(MASK_ANIMATION_DURATION);
    option.SetCurve(curve);

    AnimationUtils::OpenImplicitAnimation(option, option.GetCurve(), [weak = AceType::WeakClaim(this),
        selectedIndex, unselectedIndex]() {
        auto tabBar = weak.Upgrade();
        if (tabBar) {
            auto host = tabBar->GetHost();
            CHECK_NULL_VOID(host);
            MaskAnimationFinish(host, selectedIndex, true);
            MaskAnimationFinish(host, unselectedIndex, false);
        }
    });

    AnimationUtils::AddKeyFrame(HALF_PROGRESS, [weak = AceType::WeakClaim(this), selectedIndex, unselectedIndex,
        selectedImageSize, originalSelectedMaskOffset, unselectedImageSize, originalUnselectedMaskOffset]() {
        auto tabBar = weak.Upgrade();
        if (tabBar) {
            tabBar->ChangeMask(selectedIndex, selectedImageSize, originalSelectedMaskOffset, FULL_OPACITY,
                INVALID_RATIO, true);
            tabBar->ChangeMask(unselectedIndex, unselectedImageSize, originalUnselectedMaskOffset, NEAR_FULL_OPACITY,
                INVALID_RATIO, false);
        }
    });

    AnimationUtils::AddKeyFrame(FULL_PROGRESS, [weak = AceType::WeakClaim(this), selectedIndex, unselectedIndex,
        selectedImageSize, originalSelectedMaskOffset, unselectedImageSize, originalUnselectedMaskOffset]() {
        auto tabBar = weak.Upgrade();
        if (tabBar) {
            tabBar->ChangeMask(selectedIndex, selectedImageSize, originalSelectedMaskOffset, FULL_OPACITY,
                FULL_MASK_RADIUS_RATIO, true);
            tabBar->ChangeMask(unselectedIndex, unselectedImageSize, originalUnselectedMaskOffset, NO_OPACITY,
                HALF_MASK_RADIUS_RATIO, false);
        }
    });

    AnimationUtils::CloseImplicitAnimation();
}

void TabBarPattern::MaskAnimationFinish(const RefPtr<FrameNode>& host, int32_t selectedIndex,
    bool isSelected)
{
    if (selectedIndex < 0) {
        return;
    }
    auto tabBarLayoutProperty = host->GetLayoutProperty<TabBarLayoutProperty>();
    CHECK_NULL_VOID(tabBarLayoutProperty);
    if (isSelected) {
        tabBarLayoutProperty->UpdateSelectedMask(-1);
    } else {
        tabBarLayoutProperty->UpdateUnselectedMask(-1);
    }

    auto columnNode = AceType::DynamicCast<FrameNode>(host->GetChildAtIndex(selectedIndex));
    CHECK_NULL_VOID(columnNode);
    auto imageNode = AceType::DynamicCast<FrameNode>(columnNode->GetChildren().front());
    CHECK_NULL_VOID(imageNode);

    auto imageLayoutProperty = imageNode->GetLayoutProperty<ImageLayoutProperty>();
    CHECK_NULL_VOID(imageLayoutProperty);
    auto imagePaintProperty = imageNode->GetPaintProperty<ImageRenderProperty>();
    CHECK_NULL_VOID(imagePaintProperty);
    ImageSourceInfo info;
    auto imageSourceInfo = imageLayoutProperty->GetImageSourceInfo().value_or(info);

    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto tabTheme = pipelineContext->GetTheme<TabTheme>();
    CHECK_NULL_VOID(tabTheme);
    auto tabBarPattern = host->GetPattern<TabBarPattern>();
    CHECK_NULL_VOID(tabBarPattern);
    auto iconStyles = tabBarPattern->GetIconStyle();
    if (selectedIndex >= 0 && selectedIndex < static_cast<int32_t>(iconStyles.size())) {
        if (isSelected) {
            if (iconStyles[selectedIndex].selectedColor.has_value()) {
                imagePaintProperty->UpdateSvgFillColor(iconStyles[selectedIndex].selectedColor.value());
            } else {
                imagePaintProperty->UpdateSvgFillColor(tabTheme->GetBottomTabIconOn());
            }
        } else {
            if (iconStyles[selectedIndex].unselectedColor.has_value()) {
                imagePaintProperty->UpdateSvgFillColor(iconStyles[selectedIndex].unselectedColor.value());
            } else {
                imagePaintProperty->UpdateSvgFillColor(tabTheme->GetBottomTabIconOff());
            }
        }
    }
    imageLayoutProperty->UpdateImageSourceInfo(imageSourceInfo);

    host->MarkDirtyNode();
    imageNode->MarkModifyDone();
    imageNode->MarkDirtyNode();
}

void TabBarPattern::ChangeMask(int32_t index, float imageSize, const OffsetF& originalMaskOffset, float opacity,
    float radiusRatio, bool isSelected)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto maskPosition = host->GetChildren().size() - MASK_COUNT;
    if (index < 0 || NearZero(imageSize) || maskPosition < 0) {
        return;
    }

    auto maskNode = AceType::DynamicCast<FrameNode>(host->GetChildAtIndex(maskPosition + !isSelected));
    CHECK_NULL_VOID(maskNode);
    auto maskImageNode = AceType::DynamicCast<FrameNode>(maskNode->GetChildren().front());
    CHECK_NULL_VOID(maskImageNode);
    auto maskImageRenderContext = maskImageNode->GetRenderContext();
    CHECK_NULL_VOID(maskImageRenderContext);

    if (NonNegative(radiusRatio)) {
        auto maskRenderContext = maskNode->GetRenderContext();
        CHECK_NULL_VOID(maskRenderContext);
        auto maskGeometryNode = maskNode->GetGeometryNode();
        CHECK_NULL_VOID(maskGeometryNode);
        auto tabBarNode = AceType::DynamicCast<FrameNode>(host->GetChildAtIndex(index));
        CHECK_NULL_VOID(tabBarNode);
        auto tabBarGeometryNode = tabBarNode->GetGeometryNode();
        CHECK_NULL_VOID(tabBarGeometryNode);

        OffsetF maskOffset = originalMaskOffset;
        maskOffset.AddX(-imageSize * radiusRatio);
        maskOffset.AddY(imageSize * (1.0f - radiusRatio));
        auto tabBarOffset = tabBarGeometryNode->GetMarginFrameOffset();
        maskGeometryNode->SetMarginFrameOffset(maskOffset + tabBarOffset);
        maskGeometryNode->SetFrameSize(SizeF(imageSize * radiusRatio * 2.0f, imageSize * radiusRatio * 2.0f));
        maskRenderContext->SavePaintRect();
        maskRenderContext->SyncGeometryProperties(nullptr);
        BorderRadiusProperty borderRadiusProperty;
        borderRadiusProperty.SetRadius(Dimension(imageSize * radiusRatio));
        maskRenderContext->UpdateBorderRadius(borderRadiusProperty);
        maskImageRenderContext->UpdateOffset(OffsetT<Dimension>(Dimension(imageSize * radiusRatio),
            Dimension(imageSize * (radiusRatio - 1.0f))));
        auto maskImageGeometryNode = maskImageNode->GetGeometryNode();
        CHECK_NULL_VOID(maskImageGeometryNode);
        maskImageGeometryNode->SetFrameSize(SizeF(imageSize, imageSize));
        auto maskImageProperty = maskImageNode->GetLayoutProperty<ImageLayoutProperty>();
        CHECK_NULL_VOID(maskImageProperty);
        maskImageProperty->UpdateUserDefinedIdealSize(
            CalcSize(NG::CalcLength(Dimension(imageSize)), NG::CalcLength(Dimension(imageSize))));
        maskImageRenderContext->SetVisible(false);
        maskImageRenderContext->SavePaintRect();
        maskImageRenderContext->SyncGeometryProperties(nullptr);
    }
    maskImageRenderContext->UpdateOpacity(opacity);
}

void TabBarPattern::HandleSubTabBarClick(const RefPtr<TabBarLayoutProperty>& layoutProperty, int32_t index)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto tabsFrameNode = AceType::DynamicCast<TabsNode>(host->GetParent());
    CHECK_NULL_VOID(tabsFrameNode);
    auto swiperFrameNode = AceType::DynamicCast<FrameNode>(tabsFrameNode->GetTabs());
    CHECK_NULL_VOID(swiperFrameNode);
    auto swiperPattern = swiperFrameNode->GetPattern<SwiperPattern>();
    CHECK_NULL_VOID(swiperPattern);
    CHECK_NULL_VOID(swiperController_);
    swiperController_->FinishAnimation();
    int32_t indicator = swiperPattern->GetCurrentIndex();
    auto tabsPattern = tabsFrameNode->GetPattern<TabsPattern>();
    CHECK_NULL_VOID(tabsPattern);
    if (!tabsPattern->GetIsCustomAnimation() && indicator == index) {
        return;
    }
    changeByClick_ = true;

    if (tabsPattern->GetIsCustomAnimation()) {
        OnCustomContentTransition(indicator, index);
        TriggerTranslateAnimation(layoutProperty, index, swiperPattern->GetCurrentIndex());
    } else {
        TriggerTranslateAnimation(layoutProperty, index, indicator);
        if (tabsPattern->GetAnimateMode() != TabAnimateMode::NO_ANIMATION) {
            swiperController_->SwipeTo(index);
        } else {
            swiperController_->SwipeToWithoutAnimation(index);
        }
    }

    layoutProperty->UpdateIndicator(index);
}

void TabBarPattern::HandleTouchEvent(const TouchLocationInfo& info)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto touchType = info.GetTouchType();
    auto index = CalculateSelectedIndex(info.GetLocalLocation());
    if ((touchType == TouchType::UP || touchType == TouchType::CANCEL) && dialogNode_) {
        TabBarClickEvent(index);
        ClickTo(host, index);
        CloseDialog();
    }

    if (IsContainsBuilder()) {
        return;
    }

    auto totalCount = host->TotalChildCount() - MASK_COUNT;
    if (totalCount < 0) {
        return;
    }
   
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
    if (tabItemOffsets_.empty()) {
        return -1;
    }
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
            return -1;
        }
    } else {
        auto clickRange = std::make_pair(tabItemOffsets_[0].GetX(), tabItemOffsets_[tabItemOffsets_.size() - 1].GetX());
        if (!isRTL_) {
            if (LessNotEqual(local.GetX(), clickRange.first) || GreatNotEqual(local.GetX(), clickRange.second)) {
                return -1;
            }
        } else {
            if (GreatNotEqual(local.GetX(), frameSize.MainSize(axis)) ||
                LessNotEqual(local.GetX(), clickRange.second)) {
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
    if (Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_TWELVE) &&
        tabBarStyle_ == TabBarStyle::BOTTOMTABBATSTYLE) {
        return;
    }
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto tabTheme = pipelineContext->GetTheme<TabTheme>();
    CHECK_NULL_VOID(tabTheme);
    AnimationOption option = AnimationOption();
    option.SetDuration(animationType == AnimationType::HOVERTOPRESS
                           ? static_cast<int32_t>(tabTheme->GetSubTabBarHoverToPressDuration())
                           : static_cast<int32_t>(tabTheme->GetSubTabBarHoverDuration()));
    option.SetDelay(0);

    option.SetCurve(animationType == AnimationType::PRESS   ? DurationCubicCurve
                    : animationType == AnimationType::HOVER ? Curves::FRICTION
                                                            : Curves::SHARP);
    option.SetFillMode(FillMode::FORWARDS);
    Color color = pressColor;
    auto layoutProperty = GetLayoutProperty<TabBarLayoutProperty>();
    if (color == Color::TRANSPARENT && tabBarStyles_[index] == TabBarStyle::SUBTABBATSTYLE && index == indicator_ &&
        selectedModes_[index] == SelectedMode::BOARD &&
        layoutProperty->GetAxis().value_or(Axis::HORIZONTAL) == Axis::HORIZONTAL) {
        color = indicatorStyles_[index].color;
    }
    AnimationUtils::Animate(option, [weak = AceType::WeakClaim(this), selectedIndex = index, color = color]() {
        auto tabBar = weak.Upgrade();
        if (tabBar) {
            auto host = tabBar->GetHost();
            CHECK_NULL_VOID(host);
            auto columnNode = AceType::DynamicCast<FrameNode>(host->GetChildAtIndex(selectedIndex));
            CHECK_NULL_VOID(columnNode);
            auto renderContext = columnNode->GetRenderContext();
            CHECK_NULL_VOID(renderContext);
            if (tabBar->tabBarStyles_[selectedIndex] != TabBarStyle::SUBTABBATSTYLE) {
                BorderRadiusProperty borderRadiusProperty;
                auto pipelineContext = PipelineContext::GetCurrentContext();
                auto tabTheme = pipelineContext->GetTheme<TabTheme>();
                borderRadiusProperty.SetRadius(tabTheme->GetFocusIndicatorRadius());
                renderContext->UpdateBorderRadius(borderRadiusProperty);
            }
            renderContext->UpdateBackgroundColor(color);
            columnNode->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
        }
    }, [weak = AceType::WeakClaim(this), selectedIndex = index]() {
        auto tabBar = weak.Upgrade();
        if (tabBar) {
            if (tabBar->tabBarStyles_[selectedIndex] != TabBarStyle::SUBTABBATSTYLE) {
                auto host = tabBar->GetHost();
                auto columnNode = AceType::DynamicCast<FrameNode>(host->GetChildAtIndex(selectedIndex));
                auto renderContext = columnNode->GetRenderContext();
                renderContext->ResetBorderRadius();
                columnNode->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
            }
        }
    });
}

void TabBarPattern::OnTabBarIndexChange(int32_t index)
{
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    pipeline->AddAfterRenderTask([weak = WeakClaim(this), index]() {
        auto tabBarPattern = weak.Upgrade();
        CHECK_NULL_VOID(tabBarPattern);
        auto tabBarNode = tabBarPattern->GetHost();
        CHECK_NULL_VOID(tabBarNode);
        tabBarPattern->ResetIndicatorAnimationState();
        auto tabBarLayoutProperty = tabBarPattern->GetLayoutProperty<TabBarLayoutProperty>();
        CHECK_NULL_VOID(tabBarLayoutProperty);
        if (!tabBarPattern->IsMaskAnimationByCreate()) {
            tabBarPattern->HandleBottomTabBarChange(index);
        }
        tabBarPattern->SetMaskAnimationByCreate(false);
        tabBarPattern->SetIndicator(index);
        tabBarPattern->UpdateIndicator(index);
        tabBarPattern->UpdateTextColorAndFontWeight(index);
        if (tabBarLayoutProperty->GetTabBarMode().value_or(TabBarMode::FIXED) == TabBarMode::SCROLLABLE) {
            if (tabBarPattern->GetTabBarStyle() == TabBarStyle::SUBTABBATSTYLE &&
                tabBarLayoutProperty->GetAxisValue(Axis::HORIZONTAL) == Axis::HORIZONTAL) {
                if (!tabBarPattern->GetChangeByClick()) {
                    tabBarPattern->PlayTabBarTranslateAnimation(index);
                    tabBarNode->MarkDirtyNode(PROPERTY_UPDATE_LAYOUT);
                } else {
                    tabBarPattern->SetChangeByClick(false);
                }
            } else {
                tabBarPattern->needSetCentered_ = true;
                tabBarNode->MarkDirtyNode(PROPERTY_UPDATE_LAYOUT);
            }
        }
    });
    pipeline->RequestFrame();
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
    CHECK_NULL_VOID(layoutProperty);
    layoutProperty->UpdateIndicator(indicator);

    UpdatePaintIndicator(indicator, true);
}

void TabBarPattern::UpdateGradientRegions(bool needMarkDirty)
{
    auto layoutProperty = GetLayoutProperty<TabBarLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);
    auto barMode = layoutProperty->GetTabBarMode().value_or(TabBarMode::FIXED);
    auto axis = layoutProperty->GetAxis().value_or(Axis::HORIZONTAL);
    auto tarBarNode = GetHost();
    CHECK_NULL_VOID(tarBarNode);
    auto geometryNode = tarBarNode->GetGeometryNode();
    CHECK_NULL_VOID(geometryNode);
    auto frameRect = geometryNode->GetFrameRect();

    std::fill(gradientRegions_.begin(), gradientRegions_.end(), false);
    if (barMode == TabBarMode::SCROLLABLE && !tabItemOffsets_.empty()) {
        if (axis == Axis::HORIZONTAL) {
            if (LessNotEqual(tabItemOffsets_.front().GetX() - GetLeftPadding(), scrollMargin_)) {
                gradientRegions_[LEFT_GRADIENT] = true;
            }
            if (GreatNotEqual(tabItemOffsets_.back().GetX() + scrollMargin_, frameRect.Width() - GetLeftPadding())) {
                gradientRegions_[RIGHT_GRADIENT] = true;
            }
        } else if (axis == Axis::VERTICAL) {
            if (LessNotEqual(tabItemOffsets_.front().GetY(), 0.0f)) {
                gradientRegions_[TOP_GRADIENT] = true;
            }
            if (GreatNotEqual(tabItemOffsets_.front().GetY() + childrenMainSize_, frameRect.Height())) {
                gradientRegions_[BOTTOM_GRADIENT] = true;
            }
        }
    }

    if (needMarkDirty) {
        tarBarNode->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
    }
}

void TabBarPattern::UpdateTextColorAndFontWeight(int32_t indicator)
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
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto tabTheme = pipelineContext->GetTheme<TabTheme>();
    CHECK_NULL_VOID(tabTheme);
    int32_t index = 0;
    for (const auto& columnNode : tabBarNode->GetChildren()) {
        CHECK_NULL_VOID(columnNode);
        auto textNode = AceType::DynamicCast<FrameNode>(columnNode->GetChildren().back());
        CHECK_NULL_VOID(textNode);
        auto textLayoutProperty = textNode->GetLayoutProperty<TextLayoutProperty>();
        CHECK_NULL_VOID(textLayoutProperty);
        auto isSelected = columnNode->GetId() == selectedColumnId;
        if (index >= 0 && index < static_cast<int32_t>(labelStyles_.size())) {
            if (isSelected) {
                textLayoutProperty->UpdateTextColor(labelStyles_[index].selectedColor.has_value() ?
                    labelStyles_[index].selectedColor.value() : tabTheme->GetSubTabTextOnColor());
            } else {
                textLayoutProperty->UpdateTextColor(labelStyles_[index].unselectedColor.has_value() ?
                    labelStyles_[index].unselectedColor.value() : tabTheme->GetSubTabTextOffColor());
            }
        }
        if (IsNeedUpdateFontWeight(index)) {
            textLayoutProperty->UpdateFontWeight(isSelected ? FontWeight::MEDIUM : FontWeight::NORMAL);
        }
        textNode->MarkModifyDone();
        textNode->MarkDirtyNode();
        index++;
    }
}

bool TabBarPattern::IsNeedUpdateFontWeight(int32_t index)
{
    if (index < 0 || index >= static_cast<int32_t>(tabBarStyles_.size()) ||
        tabBarStyles_[index] != TabBarStyle::SUBTABBATSTYLE) {
        return false;
    }
    if (index >= static_cast<int32_t>(labelStyles_.size()) || labelStyles_[index].fontWeight.has_value()) {
        return false;
    }
    return true;
}

void TabBarPattern::UpdateImageColor(int32_t indicator)
{
    auto tabBarNode = GetHost();
    CHECK_NULL_VOID(tabBarNode);
    auto tabBarPattern = tabBarNode->GetPattern<TabBarPattern>();
    CHECK_NULL_VOID(tabBarPattern);
    if (tabBarPattern->IsContainsBuilder()) {
        return;
    }
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto tabTheme = pipelineContext->GetTheme<TabTheme>();
    CHECK_NULL_VOID(tabTheme);
    int32_t index = 0;
    for (const auto& columnNode : tabBarNode->GetChildren()) {
        CHECK_NULL_VOID(columnNode);
        auto imageNode = AceType::DynamicCast<FrameNode>(columnNode->GetChildren().front());
        CHECK_NULL_VOID(imageNode);
        if (imageNode->GetTag() != V2::IMAGE_ETS_TAG) {
            if (indicator == index) {
                UpdateSymbolStats(index, -1);
            } else {
                UpdateSymbolStats(-1, index);
            }
            index++;
            continue;
        }
        auto imageLayoutProperty = imageNode->GetLayoutProperty<ImageLayoutProperty>();
        CHECK_NULL_VOID(imageLayoutProperty);
        auto imagePaintProperty = imageNode->GetPaintProperty<ImageRenderProperty>();
        CHECK_NULL_VOID(imagePaintProperty);
        ImageSourceInfo info;
        auto imageSourceInfo = imageLayoutProperty->GetImageSourceInfo().value_or(info);
        if (index >= 0 && index < static_cast<int32_t>(iconStyles_.size())) {
            if (indicator == index) {
                imagePaintProperty->UpdateSvgFillColor(iconStyles_[index].selectedColor.has_value() ?
                    iconStyles_[index].selectedColor.value() : tabTheme->GetBottomTabIconOn());
            } else {
                imagePaintProperty->UpdateSvgFillColor(iconStyles_[index].unselectedColor.has_value() ?
                    iconStyles_[index].unselectedColor.value() : tabTheme->GetBottomTabIconOff());
            }
        }
        imageLayoutProperty->UpdateImageSourceInfo(imageSourceInfo);
        imageNode->MarkModifyDone();
        imageNode->MarkDirtyNode();
        index++;
    }
    SetImageColorOnIndex(indicator);
}

void TabBarPattern::UpdateSymbolStats(int32_t index, int32_t preIndex)
{
    auto tabBarNode = GetHost();
    CHECK_NULL_VOID(tabBarNode);
    auto tabBarPattern = tabBarNode->GetPattern<TabBarPattern>();
    CHECK_NULL_VOID(tabBarPattern);
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto tabTheme = pipelineContext->GetTheme<TabTheme>();
    CHECK_NULL_VOID(tabTheme);
    if (tabBarPattern->IsContainsBuilder()) {
        return;
    }
    std::vector<int32_t> indexes = {index, preIndex};
    for (uint32_t i = 0; i < indexes.size(); i++) {
        if (indexes[i] < 0 || indexes[i] >= static_cast<int32_t>(symbolArray_.size())) {
            continue;
        }
        auto columnNode = DynamicCast<FrameNode>(tabBarNode->GetChildAtIndex(indexes[i]));
        CHECK_NULL_VOID(columnNode);
        auto symbolNode = AceType::DynamicCast<FrameNode>(columnNode->GetChildren().front());
        CHECK_NULL_VOID(symbolNode);
        if (symbolNode->GetTag() != V2::SYMBOL_ETS_TAG) {
            continue;
        }
        auto symbolLayoutProperty = symbolNode->GetLayoutProperty<TextLayoutProperty>();
        CHECK_NULL_VOID(symbolLayoutProperty);
        TabContentModelNG::UpdateDefaultSymbol(tabTheme, symbolLayoutProperty);
        if (i == 0) {
            symbolLayoutProperty->UpdateSymbolColorList({tabTheme->GetBottomTabIconOn()});
            auto modifierOnApply = symbolArray_[indexes[i]].onApply;
            UpdateSymbolApply(symbolNode, symbolLayoutProperty, indexes[i], "selected");
            if (preIndex != -1) {
                TabContentModelNG::UpdateSymbolEffect(symbolLayoutProperty, true);
            }
        } else {
            symbolLayoutProperty->UpdateSymbolColorList({tabTheme->GetBottomTabSymbolOff()});
            UpdateSymbolApply(symbolNode, symbolLayoutProperty, indexes[i], "normal");
        }
        symbolNode->MarkModifyDone();
        symbolNode->MarkDirtyNode();
    }
}

void TabBarPattern::UpdateSymbolApply(const RefPtr<NG::FrameNode>& symbolNode,
    RefPtr<TextLayoutProperty>& symbolProperty, int32_t index, std::string type)
{
    auto modifierOnApply = symbolArray_[index].onApply;
    if (type == "selected" && !symbolArray_[index].selectedFlag) {
        return;
    }
    if (modifierOnApply != nullptr) {
        modifierOnApply(AccessibilityManager::WeakClaim(AccessibilityManager::RawPtr(symbolNode)), type);
        TabContentModelNG::UpdateSymbolEffect(symbolProperty, false);
    }
}

void TabBarPattern::UpdateSymbolEffect(int32_t index)
{
    if (index != GetImageColorOnIndex().value_or(indicator_)) {
        return;
    }
    auto tabBarNode = GetHost();
    CHECK_NULL_VOID(tabBarNode);
    auto columnNode = DynamicCast<FrameNode>(tabBarNode->GetChildAtIndex(index));
    CHECK_NULL_VOID(columnNode);
    auto symbolNode = AceType::DynamicCast<FrameNode>(columnNode->GetChildren().front());
    CHECK_NULL_VOID(symbolNode);
    if (symbolNode->GetTag() == V2::SYMBOL_ETS_TAG) {
        auto symbolLayoutProperty = symbolNode->GetLayoutProperty<TextLayoutProperty>();
        CHECK_NULL_VOID(symbolLayoutProperty);
        auto symbolEffectOptions = symbolLayoutProperty->GetSymbolEffectOptionsValue(SymbolEffectOptions());
        symbolEffectOptions.SetIsTxtActive(false);
        symbolLayoutProperty->UpdateSymbolEffectOptions(symbolEffectOptions);
    }
}

void TabBarPattern::UpdateSubTabBoard()
{
    auto layoutProperty = GetLayoutProperty<TabBarLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);
    auto axis = layoutProperty->GetAxis().value_or(Axis::HORIZONTAL);

    if (indicator_ >= static_cast<int32_t>(indicatorStyles_.size()) ||
        indicator_ >= static_cast<int32_t>(selectedModes_.size())) {
        return;
    }
    auto tabBarNode = GetHost();
    CHECK_NULL_VOID(tabBarNode);
    auto paintProperty = GetPaintProperty<TabBarPaintProperty>();
    CHECK_NULL_VOID(paintProperty);
    auto columnNode = DynamicCast<FrameNode>(tabBarNode->GetChildAtIndex(indicator_));
    CHECK_NULL_VOID(columnNode);
    auto selectedColumnId = columnNode->GetId();

    for (const auto& columnNode : tabBarNode->GetChildren()) {
        CHECK_NULL_VOID(columnNode);
        auto columnFrameNode = AceType::DynamicCast<FrameNode>(columnNode);
        auto renderContext = columnFrameNode->GetRenderContext();
        CHECK_NULL_VOID(renderContext);
        if (tabBarStyles_[indicator_] == TabBarStyle::SUBTABBATSTYLE) {
            if (selectedModes_[indicator_] == SelectedMode::BOARD && columnFrameNode->GetId() == selectedColumnId &&
                axis == Axis::HORIZONTAL) {
                renderContext->UpdateBackgroundColor(indicatorStyles_[indicator_].color);
            } else {
                renderContext->UpdateBackgroundColor(Color::BLACK.BlendOpacity(0.0f));
            }
            columnFrameNode->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
        }
    }
}

SelectedMode TabBarPattern::GetSelectedMode() const
{
    if (indicator_ >= static_cast<int32_t>(selectedModes_.size())) {
        return SelectedMode::INDICATOR;
    } else {
        return selectedModes_[indicator_];
    }
}

bool TabBarPattern::IsContainsBuilder()
{
    return std::any_of(tabBarType_.begin(), tabBarType_.end(), [](const auto& isBuilder) { return isBuilder.second; });
}

void TabBarPattern::TriggerTranslateAnimation(
    const RefPtr<TabBarLayoutProperty>& layoutProperty, int32_t index, int32_t indicator)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto originalPaintRect = layoutProperty->GetIndicatorRect(indicator);
    auto targetPaintRect = layoutProperty->GetIndicatorRect(index);
    auto paintProperty = host->GetPaintProperty<TabBarPaintProperty>();
    CHECK_NULL_VOID(paintProperty);
    paintProperty->UpdateIndicator(targetPaintRect);
    float targetOffset = 0.0f;
    if (host->GetGeometryNode()->GetPaddingSize().Width() < childrenMainSize_ &&
        layoutProperty->GetTabBarModeValue(TabBarMode::FIXED) == TabBarMode::SCROLLABLE) {
        auto space = GetSpace(index);
        float frontChildrenMainSize = CalculateFrontChildrenMainSize(index);
        float backChildrenMainSize = CalculateBackChildrenMainSize(index);
        targetOffset = space < 0.0f                    ? -frontChildrenMainSize
                       : frontChildrenMainSize < space ? 0.0f
                       : backChildrenMainSize < space
                           ? host->GetGeometryNode()->GetPaddingSize().Width() - childrenMainSize_
                           : space - frontChildrenMainSize;
        if (tabItemOffsets_.empty()) {
            return;
        }
        PlayTranslateAnimation(originalPaintRect.GetX() + originalPaintRect.Width() / 2,
            targetPaintRect.GetX() + targetPaintRect.Width() / 2 - tabItemOffsets_.front().GetX() + scrollMargin_ +
                targetOffset + GetLeftPadding(),
            targetOffset);
    } else {
        PlayTranslateAnimation(originalPaintRect.GetX() + originalPaintRect.Width() / 2,
            targetPaintRect.GetX() + targetPaintRect.Width() / 2, targetOffset);
    }
    swiperStartIndex_ = indicator;
    animationTargetIndex_ = index;
    UpdateTextColorAndFontWeight(index);
}

void TabBarPattern::PlayTranslateAnimation(float startPos, float endPos, float targetCurrentOffset)
{
    auto curve = DurationCubicCurve;
    isAnimating_ = true;
    StopTranslateAnimation();
    SetSwiperCurve(curve);
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto tabTheme = pipelineContext->GetTheme<TabTheme>();
    CHECK_NULL_VOID(tabTheme);
    AnimationOption option = AnimationOption();
    option.SetDuration(static_cast<int32_t>(GetAnimationDuration().value_or(
        tabTheme->GetTabContentAnimationDuration())));
    option.SetCurve(curve);
    option.SetFillMode(FillMode::FORWARDS);

    auto weak = AceType::WeakClaim(this);
    const auto& pattern = weak.Upgrade();
    auto host = pattern->GetHost();
    indicatorStartPos_ = startPos;
    indicatorEndPos_ = endPos;
    turnPageRate_ = 0.0f;

    host->CreateAnimatablePropertyFloat("tabbarindicator", 0, [weak](float value) {
        auto tabBarPattern = weak.Upgrade();
        CHECK_NULL_VOID(tabBarPattern);
        if (!tabBarPattern->isAnimating_ ||
            NearZero(tabBarPattern->indicatorEndPos_ - tabBarPattern->indicatorStartPos_)) {
            return;
        }
        tabBarPattern->turnPageRate_ = (value - tabBarPattern->indicatorStartPos_) /
            (tabBarPattern->indicatorEndPos_ - tabBarPattern->indicatorStartPos_);
        tabBarPattern->UpdateIndicatorCurrentOffset(static_cast<float>(value - tabBarPattern->currentIndicatorOffset_));
    });
    host->UpdateAnimatablePropertyFloat("tabbarindicator", startPos);
    auto delta = endPos;
    indicatorAnimationIsRunning_ = true;
    tabbarIndicatorAnimation_ = AnimationUtils::StartAnimation(option,
        [host, delta]() {
            host->UpdateAnimatablePropertyFloat("tabbarindicator", delta);
        },
        [weak]() {
            auto tabBarPattern = weak.Upgrade();
            CHECK_NULL_VOID(tabBarPattern);
            tabBarPattern->indicatorAnimationIsRunning_ = false;
        });

    auto startCurrentOffset = currentOffset_;
    host->CreateAnimatablePropertyFloat("tabbar", 0, [weak](float value) {
        auto tabBarPattern = weak.Upgrade();
        CHECK_NULL_VOID(tabBarPattern);
        tabBarPattern->currentOffset_ = value;
        auto host = tabBarPattern->GetHost();
        host->MarkDirtyNode(PROPERTY_UPDATE_LAYOUT);
    });
    host->UpdateAnimatablePropertyFloat("tabbar", startCurrentOffset);
    delta = targetCurrentOffset;
    translateAnimationIsRunning_ = true;
    translateAnimation_ = AnimationUtils::StartAnimation(option,
        [host, delta]() {
            host->UpdateAnimatablePropertyFloat("tabbar", delta);
        },
        [weak]() {
            auto tabBarPattern = weak.Upgrade();
            CHECK_NULL_VOID(tabBarPattern);
            tabBarPattern->translateAnimationIsRunning_ = false;
        });
}

void TabBarPattern::StopTranslateAnimation()
{
    if (translateAnimation_)
        AnimationUtils::StopAnimation(translateAnimation_);

    if (tabbarIndicatorAnimation_)
        AnimationUtils::StopAnimation(tabbarIndicatorAnimation_);

    if (indicatorAnimationIsRunning_)
        indicatorAnimationIsRunning_ = false;

    if (translateAnimationIsRunning_)
        translateAnimationIsRunning_ = false;
}

void TabBarPattern::PlayTabBarTranslateAnimation(int32_t targetIndex)
{
    StopTabBarTranslateAnimation();
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    if (host->GetGeometryNode()->GetPaddingSize().Width() >= childrenMainSize_) {
        return;
    }
    auto space = GetSpace(targetIndex);
    float frontChildrenMainSize = CalculateFrontChildrenMainSize(targetIndex);
    float backChildrenMainSize = CalculateBackChildrenMainSize(targetIndex);
    auto targetOffset = space < 0.0f                    ? -frontChildrenMainSize
                        : frontChildrenMainSize < space ? 0.0f
                        : backChildrenMainSize < space
                            ? host->GetGeometryNode()->GetPaddingSize().Width() - childrenMainSize_
                            : space - frontChildrenMainSize;
    auto startOffset = currentOffset_;

    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto tabTheme = pipelineContext->GetTheme<TabTheme>();
    CHECK_NULL_VOID(tabTheme);
    auto curve = DurationCubicCurve;
    AnimationOption option = AnimationOption();
    option.SetDuration(static_cast<int32_t>(GetAnimationDuration().value_or(
        tabTheme->GetTabContentAnimationDuration())));
    option.SetCurve(curve);

    auto weak = AceType::WeakClaim(this);
    host->CreateAnimatablePropertyFloat("tabbar", 0, [weak, startOffset, targetOffset](float value) {
        auto tabBarPattern = weak.Upgrade();
        CHECK_NULL_VOID(tabBarPattern);
        tabBarPattern->currentOffset_ = value;
        auto host = tabBarPattern->GetHost();
        host->MarkDirtyNode(PROPERTY_UPDATE_LAYOUT);
    });
    host->UpdateAnimatablePropertyFloat("tabbar", startOffset);
    auto delta = targetOffset;
    tabBarTranslateAnimationIsRunning_ = true;
    tabBarTranslateAnimation_ = AnimationUtils::StartAnimation(option,
        [host, delta]() {
            host->UpdateAnimatablePropertyFloat("tabbar", delta);
        },
        [weak]() {
            auto tabBarPattern = weak.Upgrade();
            CHECK_NULL_VOID(tabBarPattern);
            tabBarPattern->tabBarTranslateAnimationIsRunning_ = false;
        });
}

void TabBarPattern::StopTabBarTranslateAnimation()
{
    if (tabBarTranslateAnimation_)
        AnimationUtils::StopAnimation(tabBarTranslateAnimation_);

    if (tabBarTranslateAnimationIsRunning_)
        tabBarTranslateAnimationIsRunning_ = false;
}

void TabBarPattern::UpdateIndicatorCurrentOffset(float offset)
{
    currentIndicatorOffset_ = currentIndicatorOffset_ + offset;
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
}

RefPtr<NodePaintMethod> TabBarPattern::CreateNodePaintMethod()
{
    if (indicator_ < 0 || indicator_ >= static_cast<int32_t>(indicatorStyles_.size()) ||
        indicator_ >= static_cast<int32_t>(selectedModes_.size())) {
        return nullptr;
    }

    if (!tabBarModifier_) {
        tabBarModifier_ = AceType::MakeRefPtr<TabBarModifier>();
    }

    Color bgColor = GetTabBarBackgroundColor();
    RectF tabBarItemRect;
    auto paintProperty = GetPaintProperty<TabBarPaintProperty>();
    if (paintProperty) {
        RectF rect;
        tabBarItemRect = paintProperty->GetIndicator().value_or(rect);
    }
    IndicatorStyle indicatorStyle;
    OffsetF indicatorOffset = { currentIndicatorOffset_, tabBarItemRect.GetY() };
    GetIndicatorStyle(indicatorStyle, indicatorOffset);
    indicatorOffset.AddX(-indicatorStyle.width.ConvertToPx() / HALF_OF_WIDTH);
    auto hasIndicator = selectedModes_[indicator_] == SelectedMode::INDICATOR && !NearZero(tabBarItemRect.Height());
    return MakeRefPtr<TabBarPaintMethod>(tabBarModifier_, gradientRegions_, bgColor, indicatorStyle,
        indicatorOffset, hasIndicator);
}

Color TabBarPattern::GetTabBarBackgroundColor() const
{
    Color bgColor = Color::WHITE;
    auto tabBarNode = GetHost();
    CHECK_NULL_RETURN(tabBarNode, bgColor);
    auto tabBarCtx = tabBarNode->GetRenderContext();
    CHECK_NULL_RETURN(tabBarCtx, bgColor);
    if (tabBarCtx->GetBackgroundColor()) {
        bgColor = *tabBarCtx->GetBackgroundColor();
    } else {
        auto tabsNode = AceType::DynamicCast<FrameNode>(tabBarNode->GetParent());
        CHECK_NULL_RETURN(tabsNode, bgColor);
        auto tabsCtx = tabsNode->GetRenderContext();
        CHECK_NULL_RETURN(tabsCtx, bgColor);
        if (tabsCtx->GetBackgroundColor()) {
            bgColor = *tabsCtx->GetBackgroundColor();
        } else {
            auto pipeline = PipelineContext::GetCurrentContext();
            CHECK_NULL_RETURN(pipeline, bgColor);
            auto tabTheme = pipeline->GetTheme<TabTheme>();
            CHECK_NULL_RETURN(tabTheme, bgColor);
            bgColor = tabTheme->GetBackgroundColor().ChangeAlpha(0xff);
        }
    }
    return bgColor;
}

void TabBarPattern::GetIndicatorStyle(IndicatorStyle& indicatorStyle, OffsetF& indicatorOffset)
{
    if (indicator_ < 0 || indicator_ >= static_cast<int32_t>(indicatorStyles_.size())) {
        return;
    }
    indicatorStyle = indicatorStyles_[indicator_];

    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto layoutProperty = host->GetLayoutProperty<TabBarLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);

    if (NonPositive(indicatorStyle.width.Value())) {
        indicatorStyle.width = Dimension(layoutProperty->GetIndicatorRect(indicator_).Width());
    }

    if ((!isTouchingSwiper_ && !isAnimating_) || axis_ != Axis::HORIZONTAL) {
        return;
    }

    if (LessOrEqual(turnPageRate_, 0.0f)) {
        turnPageRate_ = 0.0f;
    }
    if (GreatOrEqual(turnPageRate_, 1.0f)) {
        turnPageRate_ = 1.0f;
    }

    if (swiperStartIndex_ < 0 || swiperStartIndex_ >= static_cast<int32_t>(tabBarStyles_.size()) ||
        tabBarStyles_[swiperStartIndex_] != TabBarStyle::SUBTABBATSTYLE ||
        swiperStartIndex_ >= static_cast<int32_t>(selectedModes_.size()) ||
        selectedModes_[swiperStartIndex_] != SelectedMode::INDICATOR ||
        swiperStartIndex_ >= static_cast<int32_t>(indicatorStyles_.size())) {
        return;
    }

    auto nextIndex = isTouchingSwiper_ ? swiperStartIndex_ + 1 : animationTargetIndex_.value_or(-1);
    if (nextIndex < 0 || nextIndex >= static_cast<int32_t>(tabBarStyles_.size()) ||
        tabBarStyles_[nextIndex] != TabBarStyle::SUBTABBATSTYLE ||
        nextIndex >= static_cast<int32_t>(selectedModes_.size()) ||
        selectedModes_[nextIndex] != SelectedMode::INDICATOR ||
        nextIndex >= static_cast<int32_t>(indicatorStyles_.size())) {
        return;
    }

    indicatorStyle = indicatorStyles_[swiperStartIndex_];
    auto startItemRect = layoutProperty->GetIndicatorRect(swiperStartIndex_);
    if (NonPositive(indicatorStyle.width.Value())) {
        indicatorStyle.width = Dimension(startItemRect.Width());
    }
    IndicatorStyle nextIndicatorStyle = indicatorStyles_[nextIndex];
    auto nextItemRect = layoutProperty->GetIndicatorRect(nextIndex);
    if (NonPositive(nextIndicatorStyle.width.Value())) {
        nextIndicatorStyle.width = Dimension(nextItemRect.Width());
    }
    indicatorStyle.width =
        Dimension(indicatorStyle.width.ConvertToPx() +
                  (nextIndicatorStyle.width.ConvertToPx() - indicatorStyle.width.ConvertToPx()) * turnPageRate_);
    indicatorStyle.marginTop = Dimension(
        indicatorStyle.marginTop.ConvertToPx() +
        (nextIndicatorStyle.marginTop.ConvertToPx() - indicatorStyle.marginTop.ConvertToPx()) * turnPageRate_);
    indicatorStyle.height =
        Dimension(indicatorStyle.height.ConvertToPx() +
                  (nextIndicatorStyle.height.ConvertToPx() - indicatorStyle.height.ConvertToPx()) * turnPageRate_);
    LinearColor color = LinearColor(indicatorStyle.color) +
                        (LinearColor(nextIndicatorStyle.color) - LinearColor(indicatorStyle.color)) * turnPageRate_;
    indicatorStyle.color = color.ToColor();
    indicatorOffset.SetY(startItemRect.GetY() + (nextItemRect.GetY() - startItemRect.GetY()) * turnPageRate_);
}

float TabBarPattern::GetSpace(int32_t indicator)
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, 0.0f);
    auto geometryNode = host->GetGeometryNode();
    auto childFrameNode = AceType::DynamicCast<FrameNode>(host->GetChildAtIndex(indicator));
    CHECK_NULL_RETURN(childFrameNode, 0.0f);
    auto childGeometryNode = childFrameNode->GetGeometryNode();

    return (geometryNode->GetPaddingSize().MainSize(axis_) - childGeometryNode->GetMarginFrameSize().MainSize(axis_)) /
           2;
}

float TabBarPattern::CalculateFrontChildrenMainSize(int32_t indicator)
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, 0.0f);
    float frontChildrenMainSize = scrollMargin_;
    for (int32_t index = 0; index < indicator; ++index) {
        auto childFrameNode = AceType::DynamicCast<FrameNode>(host->GetChildAtIndex(index));
        CHECK_NULL_RETURN(childFrameNode, 0.0f);
        auto childGeometryNode = childFrameNode->GetGeometryNode();
        auto childFrameSize = childGeometryNode->GetMarginFrameSize();
        frontChildrenMainSize += childFrameSize.MainSize(axis_);
    }
    return indicator == 0 ? 0.0f : frontChildrenMainSize;
}

float TabBarPattern::CalculateBackChildrenMainSize(int32_t indicator)
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, 0.0f);
    float backChildrenMainSize = scrollMargin_;
    auto childCount = host->GetChildren().size() - MASK_COUNT;
    for (uint32_t index = static_cast<uint32_t>(indicator) + 1; index < childCount; ++index) {
        auto childFrameNode = AceType::DynamicCast<FrameNode>(host->GetChildAtIndex(index));
        CHECK_NULL_RETURN(childFrameNode, 0.0f);
        auto childGeometryNode = childFrameNode->GetGeometryNode();
        auto childFrameSize = childGeometryNode->GetMarginFrameSize();
        backChildrenMainSize += childFrameSize.MainSize(axis_);
    }
    return indicator == static_cast<int32_t>(childCount - 1) ? 0.0f : backChildrenMainSize;
}

void TabBarPattern::SetEdgeEffect(const RefPtr<GestureEventHub>& gestureHub)
{
    CHECK_NULL_VOID(gestureHub);
    if (scrollEffect_) {
        gestureHub->RemoveScrollEdgeEffect(scrollEffect_);
        scrollEffect_.Reset();
    }
    if (!scrollEffect_) {
        auto springEffect = AceType::MakeRefPtr<ScrollSpringEffect>();
        CHECK_NULL_VOID(springEffect);
        springEffect->SetOutBoundaryCallback([weak = AceType::WeakClaim(this)]() {
            auto pattern = weak.Upgrade();
            CHECK_NULL_RETURN(pattern, false);
            return pattern->IsAtTop() || pattern->IsAtBottom();
        });
        // add callback to springEdgeEffect
        SetEdgeEffectCallback(springEffect);
        scrollEffect_ = springEffect;
        gestureHub->AddScrollEdgeEffect(axis_, scrollEffect_);
    }
}

void TabBarPattern::SetEdgeEffectCallback(const RefPtr<ScrollEdgeEffect>& scrollEffect)
{
    scrollEffect->SetCurrentPositionCallback([weak = AceType::WeakClaim(this)]() -> double {
        auto tabBar = weak.Upgrade();
        CHECK_NULL_RETURN(tabBar, 0.0);
        return tabBar->currentOffset_;
    });
    auto leadingCallback = [weak = AceType::WeakClaim(this)]() -> double {
        auto tabBar = weak.Upgrade();
        CHECK_NULL_RETURN(tabBar, 0.0);
        auto host = tabBar->GetHost();
        CHECK_NULL_RETURN(host, 0.0);
        auto geometryNode = host->GetGeometryNode();
        CHECK_NULL_RETURN(geometryNode, 0.0);
        if (tabBar->axis_ == Axis::HORIZONTAL) {
            return geometryNode->GetPaddingSize().Width() - tabBar->childrenMainSize_;
        } else {
            return geometryNode->GetPaddingSize().Height() - tabBar->childrenMainSize_;
        }
    };
    scrollEffect->SetLeadingCallback(leadingCallback);
    scrollEffect->SetTrailingCallback([]() -> double { return 0.0; });
    scrollEffect->SetInitLeadingCallback(leadingCallback);
    scrollEffect->SetInitTrailingCallback([]() -> double { return 0.0; });
}

bool TabBarPattern::IsAtTop() const
{
    return NonNegative(currentOffset_);
}

bool TabBarPattern::IsAtBottom() const
{
    if (tabItemOffsets_.empty()) {
        return false;
    }
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    auto geometryNode = host->GetGeometryNode();
    CHECK_NULL_RETURN(geometryNode, false);

    if (axis_ == Axis::HORIZONTAL) {
        return LessOrEqual(tabItemOffsets_.back().GetX() + scrollMargin_,
            geometryNode->GetPaddingSize().Width() + GetLeftPadding());
    } else {
        return LessOrEqual(tabItemOffsets_.back().GetY(), geometryNode->GetPaddingSize().Height());
    }
}

bool TabBarPattern::IsOutOfBoundary()
{
    if (tabItemOffsets_.empty()) {
        return false;
    }
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    auto geometryNode = host->GetGeometryNode();
    CHECK_NULL_RETURN(geometryNode, false);

    if (axis_ == Axis::HORIZONTAL) {
        auto mainSize = geometryNode->GetPaddingSize().Width();
        bool outOfStart = Positive(tabItemOffsets_.front().GetX() - scrollMargin_ - GetLeftPadding()) &&
            GreatNotEqual(tabItemOffsets_.back().GetX() + scrollMargin_, mainSize + GetLeftPadding());
        bool outOfEnd = LessNotEqual(tabItemOffsets_.back().GetX() + scrollMargin_, mainSize + GetLeftPadding()) &&
            Negative(tabItemOffsets_.front().GetX() - scrollMargin_ - GetLeftPadding());
        return outOfStart || outOfEnd;
    } else {
        auto mainSize = geometryNode->GetPaddingSize().Height();
        bool outOfStart = Positive(tabItemOffsets_.front().GetY()) &&
            GreatNotEqual(tabItemOffsets_.back().GetY(), mainSize);
        bool outOfEnd = LessNotEqual(tabItemOffsets_.back().GetY(), mainSize) &&
            Negative(tabItemOffsets_.front().GetY());
        return outOfStart || outOfEnd;
    }
}

void TabBarPattern::SetAccessibilityAction()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto accessibilityProperty = host->GetAccessibilityProperty<AccessibilityProperty>();
    CHECK_NULL_VOID(accessibilityProperty);
    accessibilityProperty->SetActionScrollForward([weakPtr = WeakClaim(this)]() {
        const auto& pattern = weakPtr.Upgrade();
        CHECK_NULL_VOID(pattern);
        auto tabBarLayoutProperty = pattern->GetLayoutProperty<TabBarLayoutProperty>();
        CHECK_NULL_VOID(tabBarLayoutProperty);
        auto frameNode = pattern->GetHost();
        CHECK_NULL_VOID(frameNode);
        if (tabBarLayoutProperty->GetTabBarMode().value_or(TabBarMode::FIXED) == TabBarMode::SCROLLABLE &&
            frameNode->TotalChildCount() - MASK_COUNT > 1) {
            auto index = pattern->GetIndicator() + 1;
            pattern->FocusIndexChange(index);
            // AccessibilityEventType::SCROLL_END
        }
    });

    accessibilityProperty->SetActionScrollBackward([weakPtr = WeakClaim(this)]() {
        const auto& pattern = weakPtr.Upgrade();
        CHECK_NULL_VOID(pattern);
        auto tabBarLayoutProperty = pattern->GetLayoutProperty<TabBarLayoutProperty>();
        CHECK_NULL_VOID(tabBarLayoutProperty);
        auto frameNode = pattern->GetHost();
        CHECK_NULL_VOID(frameNode);
        if (tabBarLayoutProperty->GetTabBarMode().value_or(TabBarMode::FIXED) == TabBarMode::SCROLLABLE &&
           frameNode->TotalChildCount() - MASK_COUNT > 1) {
            auto index = pattern->GetIndicator() - 1;
            pattern->FocusIndexChange(index);
            // AccessibilityEventType::SCROLL_END
        }
    });
}

std::string TabBarPattern::ProvideRestoreInfo()
{
    auto jsonObj = JsonUtil::Create(true);
    auto tabBarLayoutProperty = GetLayoutProperty<TabBarLayoutProperty>();
    CHECK_NULL_RETURN(tabBarLayoutProperty, "");
    jsonObj->Put("Index", tabBarLayoutProperty->GetIndicator().value_or(0));
    return jsonObj->ToString();
}

void TabBarPattern::OnRestoreInfo(const std::string& restoreInfo)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto tabBarLayoutProperty = GetLayoutProperty<TabBarLayoutProperty>();
    CHECK_NULL_VOID(tabBarLayoutProperty);
    auto info = JsonUtil::ParseJsonString(restoreInfo);
    if (!info->IsValid() || !info->IsObject()) {
        return;
    }
    auto jsonIsOn = info->GetValue("Index");
    auto index = jsonIsOn->GetInt();
    auto totalCount = host->TotalChildCount();
    if (index < 0 || index >= totalCount || !swiperController_ ||
        indicator_ >= static_cast<int32_t>(tabBarStyles_.size())) {
        return;
    }
    auto tabsFrameNode = AceType::DynamicCast<TabsNode>(host->GetParent());
    CHECK_NULL_VOID(tabsFrameNode);
    auto tabsPattern = tabsFrameNode->GetPattern<TabsPattern>();
    tabBarLayoutProperty->UpdateIndicator(index);
    if (GetAnimationDuration().has_value()
        && (!tabsPattern || tabsPattern->GetAnimateMode() != TabAnimateMode::NO_ANIMATION)) {
        swiperController_->SwipeTo(index);
    } else {
        swiperController_->SwipeToWithoutAnimation(index);
    }
}

void TabBarPattern::ToJsonValue(std::unique_ptr<JsonValue>& json, const InspectorFilter& filter) const
{
    Pattern::ToJsonValue(json, filter);
    auto selectedModes = JsonUtil::CreateArray(true);
    for (const auto& selectedMode : selectedModes_) {
        auto mode = JsonUtil::Create(true);
        mode->Put("mode", selectedMode == SelectedMode::INDICATOR ? "INDICATOR" : "BOARD");
        selectedModes->Put(mode);
    }
    json->PutExtAttr("selectedModes", selectedModes->ToString().c_str(), filter);

    auto indicatorStyles = JsonUtil::CreateArray(true);
    for (const auto& indicatorStyle : indicatorStyles_) {
        auto indicator = JsonUtil::Create(true);
        indicator->Put("color", indicatorStyle.color.ColorToString().c_str());
        indicator->Put("height", indicatorStyle.height.ToString().c_str());
        indicator->Put("width", indicatorStyle.width.ToString().c_str());
        indicator->Put("borderRadius", indicatorStyle.borderRadius.ToString().c_str());
        indicator->Put("marginTop", indicatorStyle.marginTop.ToString().c_str());
        indicatorStyles->Put(indicator);
    }
    json->PutExtAttr("indicatorStyles", indicatorStyles->ToString().c_str(), filter);

    auto tabBarStyles = JsonUtil::CreateArray(true);
    for (const auto& tabBarStyle : tabBarStyles_) {
        auto style = JsonUtil::Create(true);
        style->Put("style", tabBarStyle == TabBarStyle::NOSTYLE          ? "NOSTYLE"
                            : tabBarStyle == TabBarStyle::SUBTABBATSTYLE ? "SUBTABBATSTYLE"
                                                                         : "BOTTOMTABBATSTYLE");
        tabBarStyles->Put(style);
    }
    json->PutExtAttr("tabBarStyles", tabBarStyles->ToString().c_str(), filter);
}

void TabBarPattern::FromJson(const std::unique_ptr<JsonValue>& json)
{
    auto selectedModes = JsonUtil::ParseJsonString(json->GetString("selectedModes"));
    for (int32_t i = 0; i < selectedModes->GetArraySize(); i++) {
        auto selectedMode = selectedModes->GetArrayItem(i);
        auto mode = selectedMode->GetString("mode");
        SetSelectedMode(mode == "INDICATOR" ? SelectedMode::INDICATOR : SelectedMode::BOARD, i);
    }

    auto indicatorStyles = JsonUtil::ParseJsonString(json->GetString("indicatorStyles"));
    for (int32_t i = 0; i < indicatorStyles->GetArraySize(); i++) {
        auto indicatorStyle = indicatorStyles->GetArrayItem(i);
        IndicatorStyle style;
        style.color = Color::ColorFromString(indicatorStyle->GetString("color"));
        style.height = Dimension::FromString(indicatorStyle->GetString("height"));
        style.width = Dimension::FromString(indicatorStyle->GetString("width"));
        style.borderRadius = Dimension::FromString(indicatorStyle->GetString("borderRadius"));
        style.marginTop = Dimension::FromString(indicatorStyle->GetString("marginTop"));
        SetIndicatorStyle(style, i);
    }

    auto tabBarStyles = JsonUtil::ParseJsonString(json->GetString("tabBarStyles"));
    for (int32_t i = 0; i < tabBarStyles->GetArraySize(); i++) {
        auto tabBarStyle = tabBarStyles->GetArrayItem(i);
        auto style = tabBarStyle->GetString("style");
        SetTabBarStyle(style == "NOSTYLE"          ? TabBarStyle::NOSTYLE
                       : style == "SUBTABBATSTYLE" ? TabBarStyle::SUBTABBATSTYLE
                                                   : TabBarStyle::BOTTOMTABBATSTYLE,
            i);
    }

    auto layoutProperty = GetLayoutProperty<TabBarLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);
    auto indicatorValue = layoutProperty->GetIndicatorValue(0);
    UpdateIndicator(indicatorValue);
    Pattern::FromJson(json);
}

void TabBarPattern::AdjustFocusPosition()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto layoutProperty = host->GetLayoutProperty<TabBarLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);
    if (focusIndicator_ < 0 || static_cast<uint32_t>(focusIndicator_ + 1) >= tabItemOffsets_.size() ||
        layoutProperty->GetTabBarModeValue(TabBarMode::FIXED) != TabBarMode::SCROLLABLE) {
        return;
    }
    if (axis_ == Axis::HORIZONTAL) {
        auto mainSize = host->GetGeometryNode()->GetPaddingSize().Width();
        if (LessNotEqual(tabItemOffsets_[focusIndicator_].GetX(), GetLeftPadding())) {
            currentOffset_ -= tabItemOffsets_[focusIndicator_].GetX() - GetLeftPadding();
            host->MarkDirtyNode(PROPERTY_UPDATE_LAYOUT);
        } else if (GreatNotEqual(tabItemOffsets_[focusIndicator_ + 1].GetX(), mainSize + GetLeftPadding())) {
            currentOffset_ += mainSize + GetLeftPadding() - tabItemOffsets_[focusIndicator_ + 1].GetX();
            host->MarkDirtyNode(PROPERTY_UPDATE_LAYOUT);
        }
    } else {
        auto mainSize = host->GetGeometryNode()->GetPaddingSize().Height();
        if (LessNotEqual(tabItemOffsets_[focusIndicator_].GetY(), 0.0f)) {
            currentOffset_ -= tabItemOffsets_[focusIndicator_].GetY();
            host->MarkDirtyNode(PROPERTY_UPDATE_LAYOUT);
        } else if (GreatNotEqual(tabItemOffsets_[focusIndicator_ + 1].GetY(), mainSize)) {
            currentOffset_ += mainSize - tabItemOffsets_[focusIndicator_ + 1].GetY();
            host->MarkDirtyNode(PROPERTY_UPDATE_LAYOUT);
        }
    }
}

void TabBarPattern::TabBarClickEvent(int32_t index) const
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto tabsNode = AceType::DynamicCast<TabsNode>(host->GetParent());
    CHECK_NULL_VOID(tabsNode);
    auto tabsPattern = tabsNode->GetPattern<TabsPattern>();
    CHECK_NULL_VOID(tabsPattern);
    auto tabBarClickEvent = tabsPattern->GetTabBarClickEvent();
    CHECK_NULL_VOID(tabBarClickEvent);
    auto event = *tabBarClickEvent;
    event(index);
}


void TabBarPattern::OnCustomContentTransition(int32_t fromIndex, int32_t toIndex)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto tabsNode = AceType::DynamicCast<TabsNode>(host->GetParent());
    CHECK_NULL_VOID(tabsNode);
    auto tabsPattern = tabsNode->GetPattern<TabsPattern>();
    CHECK_NULL_VOID(tabsPattern);
    auto swiperNode = AceType::DynamicCast<FrameNode>(tabsNode->GetTabs());
    CHECK_NULL_VOID(swiperNode);
    auto swiperPattern = swiperNode->GetPattern<SwiperPattern>();
    CHECK_NULL_VOID(swiperPattern);

    swiperPattern->OnCustomContentTransition(toIndex);
}

bool TabBarPattern::CheckSwiperDisable() const
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, true);
    auto tabsNode = AceType::DynamicCast<TabsNode>(host->GetParent());
    CHECK_NULL_RETURN(tabsNode, true);
    auto swiperNode = AceType::DynamicCast<FrameNode>(tabsNode->GetTabs());
    CHECK_NULL_RETURN(swiperNode, true);
    auto props = swiperNode->GetLayoutProperty<SwiperLayoutProperty>();
    CHECK_NULL_RETURN(props, true);
    return props->GetDisableSwipe().value_or(false);
}

void TabBarPattern::SetSwiperCurve(const RefPtr<Curve>& curve) const
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto tabsNode = AceType::DynamicCast<TabsNode>(host->GetParent());
    CHECK_NULL_VOID(tabsNode);
    auto swiperNode = AceType::DynamicCast<FrameNode>(tabsNode->GetTabs());
    CHECK_NULL_VOID(swiperNode);
    auto swiperPaintProperty = swiperNode->GetPaintProperty<SwiperPaintProperty>();
    CHECK_NULL_VOID(swiperPaintProperty);
    swiperPaintProperty->UpdateCurve(curve);
}

void TabBarPattern::ApplyTurnPageRateToIndicator(float turnPageRate)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto layoutProperty = host->GetLayoutProperty<TabBarLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);
    if (swiperStartIndex_ < 0 || swiperStartIndex_ >= static_cast<int32_t>(tabBarStyles_.size()) ||
        tabBarStyles_[swiperStartIndex_] != TabBarStyle::SUBTABBATSTYLE ||
        swiperStartIndex_ >= static_cast<int32_t>(selectedModes_.size()) ||
        selectedModes_[swiperStartIndex_] != SelectedMode::INDICATOR) {
        return;
    }

    auto index = swiperStartIndex_ + 1;
    if (index >= static_cast<int32_t>(tabBarStyles_.size())) {
        swiperStartIndex_--;
        index--;
        turnPageRate += 1.0f;
    }
    if (Negative(turnPageRate)) {
        turnPageRate = 0.0f;
    }
    if (index < 0 || index >= static_cast<int32_t>(tabBarStyles_.size()) ||
        tabBarStyles_[index] != TabBarStyle::SUBTABBATSTYLE || index >= static_cast<int32_t>(selectedModes_.size()) ||
        selectedModes_[index] != SelectedMode::INDICATOR) {
        return;
    }

    if (GreatOrEqual(turnPageRate, 1.0f)) {
        turnPageRate_ = 1.0f;
    } else if (LessOrEqual(turnPageRate, 0.0f)) {
        turnPageRate_ = 0.0f;
    } else {
        if (turnPageRate_ <= TEXT_COLOR_THREDHOLD && turnPageRate > TEXT_COLOR_THREDHOLD) {
            UpdateTextColorAndFontWeight(index);
        } else if (turnPageRate <= 1.0f - TEXT_COLOR_THREDHOLD && turnPageRate_ > 1.0f - TEXT_COLOR_THREDHOLD) {
            UpdateTextColorAndFontWeight(swiperStartIndex_);
        }
        turnPageRate_ = turnPageRate;
    }

    auto originalPaintRect = layoutProperty->GetIndicatorRect(swiperStartIndex_);
    auto targetPaintRect = layoutProperty->GetIndicatorRect(index);
    auto paintRectDiff = std::abs(targetPaintRect.GetX() + targetPaintRect.Width() / 2 - originalPaintRect.GetX() -
                                  originalPaintRect.Width() / 2);

    currentIndicatorOffset_ = originalPaintRect.GetX() + originalPaintRect.Width() / 2 + paintRectDiff * turnPageRate_;
    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
}

void TabBarPattern::AdjustOffset(double& offset) const
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto geometryNode = host->GetGeometryNode();
    CHECK_NULL_VOID(geometryNode);
    auto mainSize = 0.0f;
    if (axis_ == Axis::HORIZONTAL) {
        mainSize = geometryNode->GetPaddingSize().Width();
    } else {
        mainSize = geometryNode->GetPaddingSize().Height();
    }
    if (GreatNotEqual(currentOffset_ + offset, 0.0f)) {
        offset = -currentOffset_;
    } else if (LessNotEqual(childrenMainSize_ + currentOffset_ + offset, mainSize)) {
        offset = mainSize - childrenMainSize_ - currentOffset_;
    }
}

void TabBarPattern::InitTurnPageRateEvent()
{
    auto turnPageRateCallback = [weak = WeakClaim(this)](int32_t swipingIndex, float turnPageRate) {
        auto pattern = weak.Upgrade();
        if (pattern) {
            if (!pattern->CheckSwiperDisable() && pattern->axis_ == Axis::HORIZONTAL && pattern->isTouchingSwiper_) {
                pattern->swiperStartIndex_ = swipingIndex;
                pattern->ApplyTurnPageRateToIndicator(turnPageRate);
            } else if (!pattern->isAnimating_) {
                pattern->turnPageRate_ = 0.0f;
            }
        }
    };
    swiperController_->SetTurnPageRateCallback(std::move(turnPageRateCallback));

    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto tabsNode = AceType::DynamicCast<TabsNode>(host->GetParent());
    CHECK_NULL_VOID(tabsNode);
    auto swiperNode = AceType::DynamicCast<FrameNode>(tabsNode->GetTabs());
    CHECK_NULL_VOID(swiperNode);
    auto eventHub = swiperNode->GetEventHub<SwiperEventHub>();
    CHECK_NULL_VOID(eventHub);
    if (!animationStartEvent_) {
        AnimationStartEvent animationStartEvent =
            [weak = WeakClaim(this)](int32_t index, int32_t targetIndex, const AnimationCallbackInfo& info) {
                auto pattern = weak.Upgrade();
                if (pattern) {
                    pattern->HandleBottomTabBarAnimation(targetIndex);
                }
            };
        animationStartEvent_ = std::make_shared<AnimationStartEvent>(std::move(animationStartEvent));
        eventHub->AddAnimationStartEvent(animationStartEvent_);
    }
    if (!animationEndEvent_) {
        AnimationEndEvent animationEndEvent =
            [weak = WeakClaim(this)](int32_t index, const AnimationCallbackInfo& info) {
                PerfMonitor::GetPerfMonitor()->End(PerfConstants::APP_TAB_SWITCH, true);
                auto pattern = weak.Upgrade();
                if (pattern && (NearZero(pattern->turnPageRate_) || NearEqual(pattern->turnPageRate_, 1.0f))) {
                    pattern->isTouchingSwiper_ = false;
                }
                pattern->SetMaskAnimationExecuted(false);
            };
        animationEndEvent_ = std::make_shared<AnimationEndEvent>(std::move(animationEndEvent));
        eventHub->AddAnimationEndEvent(animationEndEvent_);
    }
}

void TabBarPattern::HandleBottomTabBarAnimation(int32_t index)
{
    auto preIndex = GetImageColorOnIndex().value_or(indicator_);
    if (preIndex < 0 || preIndex >= static_cast<int32_t>(tabBarStyles_.size())
        || index < 0 || index >= static_cast<int32_t>(tabBarStyles_.size())) {
        return;
    }
    if (tabBarStyles_[preIndex] != TabBarStyle::BOTTOMTABBATSTYLE &&
        tabBarStyles_[index] != TabBarStyle::BOTTOMTABBATSTYLE) {
        return;
    }
    if (preIndex != index) {
        auto host = GetHost();
        CHECK_NULL_VOID(host);
        auto tabsNode = AceType::DynamicCast<TabsNode>(host->GetParent());
        CHECK_NULL_VOID(tabsNode);
        auto tabsPattern = tabsNode->GetPattern<TabsPattern>();
        CHECK_NULL_VOID(tabsPattern);
        auto onChangeEvent = tabsPattern->GetChangeEvent();
        if (onChangeEvent) {
            (*onChangeEvent)(index);
        }
        auto onIndexChangeEvent = tabsPattern->GetIndexChangeEvent();
        if (onIndexChangeEvent) {
            (*onIndexChangeEvent)(index);
        }
    }
    SetMaskAnimationExecuted(true);
}

float TabBarPattern::GetLeftPadding() const
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, 0.0f);
    auto geometryNode = host->GetGeometryNode();
    CHECK_NULL_RETURN(geometryNode, 0.0f);
    if (!geometryNode->GetPadding()) {
        return 0.0f;
    }
    return geometryNode->GetPadding()->left.value_or(0.0f);
}

std::optional<int32_t> TabBarPattern::GetAnimationDuration()
{
    if (animationDuration_.has_value() && animationDuration_.value() >= 0) {
        return animationDuration_;
    }
    if (!animationDuration_.has_value() && Container::LessThanAPIVersion(PlatformVersion::VERSION_ELEVEN)) {
        return animationDuration_;
    }

    std::optional<int32_t> duration;
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipelineContext, duration);
    auto tabTheme = pipelineContext->GetTheme<TabTheme>();
    CHECK_NULL_RETURN(tabTheme, duration);
    auto host = GetHost();
    CHECK_NULL_RETURN(host, duration);
    auto tabsNode = AceType::DynamicCast<TabsNode>(host->GetParent());
    CHECK_NULL_RETURN(tabsNode, duration);
    auto swiperNode = AceType::DynamicCast<FrameNode>(tabsNode->GetTabs());
    CHECK_NULL_RETURN(swiperNode, duration);
    auto swiperPaintProperty = swiperNode->GetPaintProperty<SwiperPaintProperty>();
    CHECK_NULL_RETURN(swiperPaintProperty, duration);
    duration = static_cast<int32_t>(tabTheme->GetTabContentAnimationDuration());
    if (Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_ELEVEN) &&
        std::count(tabBarStyles_.begin(), tabBarStyles_.end(), TabBarStyle::BOTTOMTABBATSTYLE)) {
        duration = 0;
    }
    SetAnimationDuration(duration.value());
    swiperPaintProperty->UpdateDuration(duration.value());
    return duration;
}

void TabBarPattern::DumpAdvanceInfo()
{
    isRTL_ ? DumpLog::GetInstance().AddDesc("isRTL:true") : DumpLog::GetInstance().AddDesc("isRTL:false");
    touching_ ? DumpLog::GetInstance().AddDesc("touching:true") : DumpLog::GetInstance().AddDesc("touching:false");
    isMaskAnimationByCreate_ ? DumpLog::GetInstance().AddDesc("isMaskAnimationByCreate:true")
                             : DumpLog::GetInstance().AddDesc("isMaskAnimationByCreate:false");
    animationDuration_.has_value()
        ? DumpLog::GetInstance().AddDesc("animationDuration:" + std::to_string(animationDuration_.value()))
        : DumpLog::GetInstance().AddDesc("animationDuration:null");
    isFirstFocus_ ? DumpLog::GetInstance().AddDesc("isFirstFocus:true")
                  : DumpLog::GetInstance().AddDesc("isFirstFocus:false");
    isTouchingSwiper_ ? DumpLog::GetInstance().AddDesc("isTouchingSwiper:true")
                      : DumpLog::GetInstance().AddDesc("isTouchingSwiper:false");
    isAnimating_ ? DumpLog::GetInstance().AddDesc("isAnimating:true")
                 : DumpLog::GetInstance().AddDesc("isAnimating:false");
    changeByClick_ ? DumpLog::GetInstance().AddDesc("changeByClick:true")
                   : DumpLog::GetInstance().AddDesc("changeByClick:false");
    needSetCentered_ ? DumpLog::GetInstance().AddDesc("needSetCentered:true")
                     : DumpLog::GetInstance().AddDesc("needSetCentered:false");
    DumpLog::GetInstance().AddDesc("childrenMainSize:" + std::to_string(childrenMainSize_));
    DumpLog::GetInstance().AddDesc("indicator:" + std::to_string(indicator_));
    DumpLog::GetInstance().AddDesc("swiperStartIndex:" + std::to_string(swiperStartIndex_));
    DumpLog::GetInstance().AddDesc("scrollMargin:" + std::to_string(scrollMargin_));
    std::string regionString = std::string("region:");
    for (auto item : gradientRegions_) {
        item ? regionString.append("true ") : regionString.append("false ");
    }
    DumpLog::GetInstance().AddDesc(regionString);
    switch (axis_) {
        case Axis::NONE: {
            DumpLog::GetInstance().AddDesc("Axis:NONE");
            break;
        }
        case Axis::HORIZONTAL: {
            DumpLog::GetInstance().AddDesc("Axis:HORIZONTAL");
            break;
        }
        case Axis::FREE: {
            DumpLog::GetInstance().AddDesc("Axis:FREE");
            break;
        }
        case Axis::VERTICAL: {
            DumpLog::GetInstance().AddDesc("Axis:VERTICAL");
            break;
        }
        default: {
            break;
        }
    }
}

bool TabBarPattern::ContentWillChange(int32_t comingIndex)
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, true);
    auto tabsNode = AceType::DynamicCast<TabsNode>(host->GetParent());
    CHECK_NULL_RETURN(tabsNode, true);
    auto swiperNode = AceType::DynamicCast<FrameNode>(tabsNode->GetTabs());
    CHECK_NULL_RETURN(swiperNode, true);
    auto swiperPattern = swiperNode->GetPattern<SwiperPattern>();
    CHECK_NULL_RETURN(swiperPattern, true);
    int32_t currentIndex = swiperPattern->GetCurrentIndex();
    return ContentWillChange(currentIndex, comingIndex);
}

bool TabBarPattern::ContentWillChange(int32_t currentIndex, int32_t comingIndex)
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, true);
    auto tabsNode = AceType::DynamicCast<TabsNode>(host->GetParent());
    CHECK_NULL_RETURN(tabsNode, true);
    auto tabsPattern = tabsNode->GetPattern<TabsPattern>();
    CHECK_NULL_RETURN(tabsPattern, true);
    if (tabsPattern->GetInterceptStatus()) {
        auto ret = tabsPattern->OnContentWillChange(currentIndex, comingIndex);
        return ret.has_value() ? ret.value() : true;
    }
    return true;
}
} // namespace OHOS::Ace::NG

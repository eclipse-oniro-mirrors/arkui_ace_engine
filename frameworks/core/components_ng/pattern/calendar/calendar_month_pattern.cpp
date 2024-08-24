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

#include "core/components_ng/pattern/calendar/calendar_month_pattern.h"

#include "base/geometry/offset.h"
#include "base/i18n/localization.h"
#include "base/utils/utils.h"
#include "core/common/ace_application_info.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/button/button_layout_property.h"
#include "core/components_ng/pattern/button/button_pattern.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"
#include "core/components_ng/pattern/calendar_picker/calendar_dialog_view.h"
#include "core/components_ng/pattern/swiper/swiper_event_hub.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
namespace {
constexpr int32_t CALENDAR_WEEK_DAYS = 7;
constexpr int32_t DAILY_FOUR_ROWSPACE = 4;
constexpr int32_t DAILY_FIVE_ROWSPACE = 5;
constexpr Dimension CALENDAR_DISTANCE_ADJUST_FOCUSED_EVENT = 4.0_vp;
} // namespace
void CalendarMonthPattern::OnAttachToFrameNode()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->GetRenderContext()->SetClipToFrame(true);
}

bool CalendarMonthPattern::OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config)
{
    if (IsCalendarDialog()) {
        SetColRowSpace();
    }

    return !(config.skipMeasure || dirty->SkipMeasureContent());
}

Dimension CalendarMonthPattern::GetDaySize(const RefPtr<CalendarTheme>& theme)
{
    auto pipeline = GetHost()->GetContext();
    CHECK_NULL_RETURN(pipeline, theme->GetCalendarPickerDayWidthOrHeight());
    auto fontSizeScale = pipeline->GetFontScale();
    if (fontSizeScale < theme->GetCalendarPickerLargeScale() || CalendarDialogView::CheckOrientationChange()) {
        return theme->GetCalendarPickerDayWidthOrHeight();
    } else {
        return theme->GetCalendarPickerDayLargeWidthOrHeight();
    }
}

void CalendarMonthPattern::SetColRowSpace()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto contentConstraint = host->GetLayoutProperty()->GetLayoutConstraint();
    if (!contentConstraint.has_value()) {
        return;
    }
    auto constraint = contentConstraint.value();

    auto dataSize = GetMonthData().days.size();
    if (dataSize <= 0) {
        return;
    }

    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    RefPtr<CalendarTheme> theme = pipelineContext->GetTheme<CalendarTheme>();
    CHECK_NULL_VOID(theme);
    auto width = constraint.selfIdealSize.Width().value() - CALENDAR_DISTANCE_ADJUST_FOCUSED_EVENT.ConvertToPx() * 2;
    auto height = constraint.selfIdealSize.Height().value() - CALENDAR_DISTANCE_ADJUST_FOCUSED_EVENT.ConvertToPx();
    auto paintProperty = GetPaintProperty<CalendarPaintProperty>();
    CHECK_NULL_VOID(paintProperty);

    auto calendarDaySize = GetDaySize(theme);
    auto space = (width - calendarDaySize.ConvertToPx() * CALENDAR_WEEK_DAYS) / (CALENDAR_WEEK_DAYS - 1);
    if (Positive(space)) {
        Dimension colSpace = 0.0_px;
        colSpace.SetValue(space);
        paintProperty->UpdateColSpace(colSpace);
    }

    auto rowCount = dataSize / CALENDAR_WEEK_DAYS;
    space = (height - calendarDaySize.ConvertToPx() * (rowCount + 1)) / rowCount;
    if (!Positive(space)) {
        return;
    }
    Dimension rowSpace = 0.0_px;
    rowSpace.SetValue(space);
    paintProperty->UpdateWeekAndDayRowSpace(rowSpace);
    switch (rowCount) {
        case 4:
            paintProperty->UpdateDailyFourRowSpace(rowSpace);
            break;
        case 6:
            paintProperty->UpdateDailySixRowSpace(rowSpace);
            break;
        case 5:
        default:
            paintProperty->UpdateDailyFiveRowSpace(rowSpace);
            break;
    }
}

void CalendarMonthPattern::OnModifyDone()
{
    Pattern::OnModifyDone();
    InitClickEvent();
    InitTouchEvent();
    InitHoverEvent();
    InitAccessibilityHoverEvent();
    FireModifyAccessibilityVirtualNode(obtainedMonth_);
}

void CalendarMonthPattern::SetVirtualNodeUserSelected(int32_t index)
{
    if (accessibilityPropertyVec_.size() < 1) {
        return;
    }
    for (int i = 0; i < static_cast<int>(accessibilityPropertyVec_.size()); i++) {
        if (i == index &&
            obtainedMonth_.days[i].month.month == obtainedMonth_.month &&
            obtainedMonth_.days[i].month.year == obtainedMonth_.year) {
            accessibilityPropertyVec_[i]->SetUserSelected(true);
            continue;
        }
        accessibilityPropertyVec_[i]->SetUserSelected(false);
    }
    if (!obtainedMonth_.days.empty()) {
        for (auto& day : obtainedMonth_.days) {
            day.focused = false;
        }
        auto calendarEventHub = GetEventHub<CalendarEventHub>();
        CHECK_NULL_VOID(calendarEventHub);
        if (index >= 0 && index < static_cast<int32_t>(obtainedMonth_.days.size())) {
            obtainedMonth_.days[index].focused = true;
            auto json = JsonUtil::Create(true);
            json->Put("day", obtainedMonth_.days[index].day);
            json->Put("month", obtainedMonth_.days[index].month.month);
            json->Put("year", obtainedMonth_.days[index].month.year);
            calendarEventHub->UpdateSelectedChangeEvent(json->ToString());
        }
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF_AND_CHILD);
}

void CalendarMonthPattern::InitVirtualButtonClickEvent(RefPtr<FrameNode> frameNode, int32_t index)
{
    CHECK_NULL_VOID(frameNode);
    auto gesture = frameNode->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(gesture);
    auto clickCallback = [weak = WeakClaim(this), index](GestureEvent& info) {
        auto calendarPattern = weak.Upgrade();
        CHECK_NULL_VOID(calendarPattern);
        calendarPattern->SetVirtualNodeUserSelected(index);
    };
    auto clickListener = MakeRefPtr<ClickEvent>(std::move(clickCallback));
    gesture->AddClickAfterEvent(clickListener);
}

void CalendarMonthPattern::InitClickEvent()
{
    if (clickListener_) {
        return;
    }

    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto gesture = host->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(gesture);
    auto obtainedMonth = obtainedMonth_;
    auto clickCallback = [weak = WeakClaim(this), obtainedMonth](GestureEvent& info) {
        auto calendarPattern = weak.Upgrade();
        CHECK_NULL_VOID(calendarPattern);
        auto localLocation = info.GetFingerList().begin()->localLocation_;
        calendarPattern->OnClick(localLocation, calendarPattern->obtainedMonth_);
    };
    clickListener_ = MakeRefPtr<ClickEvent>(std::move(clickCallback));
    gesture->AddClickEvent(clickListener_);
}

void CalendarMonthPattern::InitTouchEvent()
{
    if (touchListener_) {
        return;
    }

    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto gesture = host->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(gesture);
    auto touchCallback = [weak = WeakClaim(this)](const TouchEventInfo& info) {
        auto calendarPattern = weak.Upgrade();
        CHECK_NULL_VOID(calendarPattern);
        if (info.GetTouches().front().GetTouchType() == TouchType::DOWN) {
            calendarPattern->OnTouchEvent(info.GetTouches().front().GetLocalLocation(), true);
        }
        if (info.GetTouches().front().GetTouchType() == TouchType::UP ||
            info.GetTouches().front().GetTouchType() == TouchType::CANCEL) {
            calendarPattern->OnTouchEvent(info.GetTouches().front().GetLocalLocation(), false);
        }
    };
    touchListener_ = MakeRefPtr<TouchEventImpl>(std::move(touchCallback));
    gesture->AddTouchEvent(touchListener_);
}

void CalendarMonthPattern::InitHoverEvent()
{
    if (hoverListener_) {
        return;
    }

    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto eventHub = GetEventHub<CalendarEventHub>();
    CHECK_NULL_VOID(eventHub);
    auto inputHub = eventHub->GetOrCreateInputEventHub();
    CHECK_NULL_VOID(inputHub);
    auto hoverCallback = [weak = WeakClaim(this)](bool state) {
        auto calendarPattern = weak.Upgrade();
        CHECK_NULL_VOID(calendarPattern);
        calendarPattern->SetHoverState(state);
        if (!state) {
            Offset localLocation;
            calendarPattern->OnHoverEvent(localLocation, false);
        }
    };
    hoverListener_ = MakeRefPtr<InputEvent>(std::move(hoverCallback));
    inputHub->AddOnHoverEvent(hoverListener_);
    auto mouseCallback = [weak = WeakClaim(this)](MouseInfo& info) {
        auto calendarPattern = weak.Upgrade();
        CHECK_NULL_VOID(calendarPattern);
        calendarPattern->OnHoverEvent(info.GetLocalLocation(), calendarPattern->GetHoverState());
    };
    inputHub->SetMouseEvent(std::move(mouseCallback));
}

void CalendarMonthPattern::OnClick(Offset& localLocation, const ObtainedMonth& obtainedMonth)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto pattern = host->GetPattern<CalendarMonthPattern>();
    CHECK_NULL_VOID(pattern);
    auto index = JudgeArea(localLocation);
    pattern->obtainedMonth_ = obtainedMonth;
    if (!obtainedMonth_.days.empty()) {
        for (auto& day : pattern->obtainedMonth_.days) {
            day.focused = false;
        }
        auto calendarEventHub = GetEventHub<CalendarEventHub>();
        CHECK_NULL_VOID(calendarEventHub);
        if (index >= 0 && index < static_cast<int32_t>(obtainedMonth.days.size())) {
            pattern->obtainedMonth_.days[index].focused = true;
            auto json = JsonUtil::Create(true);
            json->Put("day", obtainedMonth.days[index].day);
            json->Put("month", obtainedMonth.days[index].month.month);
            json->Put("year", obtainedMonth.days[index].month.year);
            calendarEventHub->UpdateSelectedChangeEvent(json->ToString());
        }
        host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
    }
}

void CalendarMonthPattern::OnTouchEvent(const Offset& localLocation, bool isPressed)
{
    if (!isCalendarDialog_ || obtainedMonth_.days.empty()) {
        return;
    }
    auto index = JudgeArea(localLocation);
    if (index < 0 || index >= static_cast<int32_t>(obtainedMonth_.days.size())) {
        return;
    }
    if (isPressed) {
        obtainedMonth_.days[index].isPressing = true;
    } else {
        for (auto& day : obtainedMonth_.days) {
            day.isPressing = false;
        }
    }

    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
}

void CalendarMonthPattern::OnHoverEvent(const Offset& localLocation, bool state)
{
    if (!isCalendarDialog_ || obtainedMonth_.days.empty()) {
        return;
    }
    int32_t index = JudgeArea(localLocation);
    if (index < 0 || index >= static_cast<int32_t>(obtainedMonth_.days.size())) {
        return;
    }
    for (auto& day : obtainedMonth_.days) {
        day.isHovering = false;
    }
    if (state) {
        obtainedMonth_.days[index].isHovering = true;
    }

    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
}

int32_t CalendarMonthPattern::JudgeArea(const Offset& offset)
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    auto paintProperty = host->GetPaintProperty<CalendarPaintProperty>();
    CHECK_NULL_RETURN(paintProperty, false);
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipelineContext, false);
    RefPtr<CalendarTheme> theme = pipelineContext->GetTheme<CalendarTheme>();
    CHECK_NULL_RETURN(theme, false);
    auto topPadding = isCalendarDialog_ ? 0.0 : theme->GetCalendarTheme().topPadding.ConvertToPx();
    auto weekHeight = paintProperty->GetWeekHeight().value_or(theme->GetCalendarTheme().weekHeight).ConvertToPx();
    auto weekAndDayRowSpace =
        paintProperty->GetWeekAndDayRowSpace().value_or(theme->GetCalendarTheme().weekAndDayRowSpace).ConvertToPx();
    auto dayHeight = paintProperty->GetDayHeight().value_or(theme->GetCalendarTheme().dayHeight).ConvertToPx();
    auto dayWidth = paintProperty->GetDayWidth().value_or(theme->GetCalendarTheme().dayWidth).ConvertToPx();
    const static int32_t columnsOfData = 7;
    auto colSpace = paintProperty->GetColSpaceValue({}).ConvertToPx() <= 0
                        ? theme->GetCalendarTheme().colSpace.ConvertToPx()
                        : paintProperty->GetColSpaceValue({}).ConvertToPx();
    auto dailyFourRowSpace = NonPositive(paintProperty->GetDailyFourRowSpaceValue({}).ConvertToPx())
                                 ? theme->GetCalendarTheme().dailySixRowSpace.ConvertToPx()
                                 : paintProperty->GetDailyFourRowSpaceValue({}).ConvertToPx();
    auto dailyFiveRowSpace = paintProperty->GetDailyFiveRowSpaceValue({}).ConvertToPx() <= 0
                                 ? theme->GetCalendarTheme().dailyFiveRowSpace.ConvertToPx()
                                 : paintProperty->GetDailyFiveRowSpaceValue({}).ConvertToPx();
    auto dailySixRowSpace = paintProperty->GetDailySixRowSpaceValue({}).ConvertToPx() <= 0
                                ? theme->GetCalendarTheme().dailySixRowSpace.ConvertToPx()
                                : paintProperty->GetDailySixRowSpaceValue({}).ConvertToPx();
    auto rows = (static_cast<int32_t>(obtainedMonth_.days.size()) / columnsOfData);
    auto rowSpace = dailySixRowSpace;
    switch (rows) {
        case 4: {
            rowSpace = dailyFourRowSpace;
            break;
        }
        case 5: {
            rowSpace = dailyFiveRowSpace;
            break;
        }
        default:
            break;
    }
    auto browHeight = weekHeight + topPadding + weekAndDayRowSpace;
    auto maxHeight = host->GetGeometryNode()->GetFrameSize().Height();
    auto maxWidth = host->GetGeometryNode()->GetFrameSize().Width();
    if ((offset.GetX() < 0) || (offset.GetX() > maxWidth) || (offset.GetY() < browHeight) ||
        (offset.GetY() > maxHeight) || LessOrEqual(dayHeight, 0.0) || LessOrEqual(dayWidth, 0.0)) {
        return -1;
    }
    auto height = offset.GetY() - browHeight;
    int32_t y =
        height < (dayHeight + rowSpace / 2) ? 0 : (height - dayHeight - rowSpace / 2) / (dayHeight + rowSpace) + 1;
    int32_t x = offset.GetX() < (dayWidth + colSpace / 2)
                    ? 0
                    : (offset.GetX() - dayWidth - colSpace / 2) / (dayWidth + colSpace) + 1;
    auto textDirection = host->GetLayoutProperty()->GetNonAutoLayoutDirection();
    if (textDirection == TextDirection::RTL) {
        x = columnsOfData - x - 1;
    }
    return (y * columnsOfData + x);
}

bool CalendarMonthPattern::InitCalendarVirtualNode()
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    if (buttonAccessibilityNodeVec_.size() > 0) {
        FireModifyAccessibilityVirtualNode(obtainedMonth_);
        return true;
    }

    auto lineNode = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<LinearLayoutPattern>(true));
    for (auto& day : obtainedMonth_.days) {
        auto buttonNode = AddButtonNodeIntoVirtual(day);
        lineNode->AddChild(buttonNode);
        buttonAccessibilityNodeVec_.emplace_back(buttonNode);
        ChangeVirtualNodeContent(day);
    }
    auto accessibilityProperty = host->GetAccessibilityProperty<AccessibilityProperty>();
    accessibilityProperty->SetAccessibilityText(" ");
    auto virtualFrameNode = AceType::DynamicCast<NG::FrameNode>(lineNode);
    CHECK_NULL_RETURN(virtualFrameNode, false);
    virtualFrameNode->SetAccessibilityNodeVirtual();
    virtualFrameNode->SetAccessibilityVirtualNodeParent(AceType::DynamicCast<NG::UINode>(host));
    virtualFrameNode->SetFirstAccessibilityVirtualNode();
    FrameNode::ProcessOffscreenNode(virtualFrameNode);
    accessibilityProperty->SaveAccessibilityVirtualNode(lineNode);
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF_AND_CHILD);
    return true;
}

std::pair<int32_t, int32_t> GetXYByIndex(const int32_t index)
{
    if (index < CALENDAR_WEEK_DAYS - 1) {
        return {index, 0};
    } else {
        return {(index % CALENDAR_WEEK_DAYS), (index / CALENDAR_WEEK_DAYS)};
    }
}

RefPtr<FrameNode> CalendarMonthPattern::AddButtonNodeIntoVirtual(const CalendarDay& calendarDay)
{
    auto buttonNode = FrameNode::CreateFrameNode(
        V2::BUTTON_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<ButtonPattern>());
    auto buttonAccessibilityProperty = buttonNode->GetAccessibilityProperty<AccessibilityProperty>();
    CHECK_NULL_RETURN(buttonAccessibilityProperty, nullptr);
    UpdateAccessibilityButtonNode(buttonNode, calendarDay.index);
    accessibilityPropertyVec_.emplace_back(buttonAccessibilityProperty);
    InitVirtualButtonClickEvent(buttonNode, calendarDay.index);
    buttonAccessibilityProperty->SetUserDisabled(calendarDay.month.month != obtainedMonth_.month ? true : false);
    buttonAccessibilityProperty->SetOnAccessibilityFocusCallback([weak = WeakClaim(this)](bool focus) {
        if (focus) {
            auto calendar = weak.Upgrade();
            CHECK_NULL_VOID(calendar);
            if (calendar->isOnHover_) {
                calendar->SetCalendarAccessibilityLevel(AccessibilityProperty::Level::NO_STR);
            }
        }
    });
    return buttonNode;
}

float GetRowSpace(RefPtr<CalendarPaintProperty> paintProperty, RefPtr<CalendarTheme> theme, size_t daySize)
{
    auto dailyFourRowSpace = NonPositive(paintProperty->GetDailyFourRowSpaceValue({}).ConvertToPx())
                                 ? theme->GetCalendarTheme().dailySixRowSpace.ConvertToPx()
                                 : paintProperty->GetDailyFourRowSpaceValue({}).ConvertToPx();
    auto dailyFiveRowSpace = paintProperty->GetDailyFiveRowSpaceValue({}).ConvertToPx() <= 0
                                 ? theme->GetCalendarTheme().dailyFiveRowSpace.ConvertToPx()
                                 : paintProperty->GetDailyFiveRowSpaceValue({}).ConvertToPx();
    auto dailySixRowSpace = paintProperty->GetDailySixRowSpaceValue({}).ConvertToPx() <= 0
                                ? theme->GetCalendarTheme().dailySixRowSpace.ConvertToPx()
                                : paintProperty->GetDailySixRowSpaceValue({}).ConvertToPx();
    auto rows = (static_cast<int32_t>(daySize) / CALENDAR_WEEK_DAYS);
    auto rowSpace = dailySixRowSpace;
    switch (rows) {
        case DAILY_FOUR_ROWSPACE: {
            rowSpace = dailyFourRowSpace;
            break;
        }
        case DAILY_FIVE_ROWSPACE: {
            rowSpace = dailyFiveRowSpace;
            break;
        }
        default:
            break;
    }
    return rowSpace;
}

void CalendarMonthPattern::SetCalendarAccessibilityLevel(const std::string& level)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto accessibilityProperty = host->GetAccessibilityProperty<AccessibilityProperty>();
    CHECK_NULL_VOID(accessibilityProperty);
    accessibilityProperty->SetAccessibilityLevel(level);
    auto parent = host->GetParent();
    while (parent && parent->GetTag() != V2::COLUMN_ETS_TAG) {
        parent = parent->GetParent();
    }
    auto parentNode = AceType::DynamicCast<NG::FrameNode>(parent);
    CHECK_NULL_VOID(parentNode);
    auto parentAccessibilityProperty = parentNode->GetAccessibilityProperty<AccessibilityProperty>();
    CHECK_NULL_VOID(parentAccessibilityProperty);
    parentAccessibilityProperty->SetAccessibilityLevel(level);
}

void CalendarMonthPattern::InitAccessibilityHoverEvent()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto eventHub = host->GetOrCreateInputEventHub();
    CHECK_NULL_VOID(eventHub);
    eventHub->SetAccessibilityHoverEvent([weak = WeakClaim(this)](bool isHover, AccessibilityHoverInfo& info) {
        auto calendar = weak.Upgrade();
        CHECK_NULL_VOID(calendar);
        calendar->HandleAccessibilityHoverEvent(isHover, info);
    });
}

void CalendarMonthPattern::HandleAccessibilityHoverEvent(bool isHover, AccessibilityHoverInfo& info)
{
    if (isHover) {
        isOnHover_ = true;
    } else if (!isHover) {
        isOnHover_ = false;
        SetCalendarAccessibilityLevel(AccessibilityProperty::Level::YES_STR);
    }
}

void CalendarMonthPattern::UpdateAccessibilityButtonNode(RefPtr<FrameNode> frameNode, int32_t index)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto paintProperty = host->GetPaintProperty<CalendarPaintProperty>();
    CHECK_NULL_VOID(paintProperty);
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    RefPtr<CalendarTheme> theme = pipelineContext->GetTheme<CalendarTheme>();
    CHECK_NULL_VOID(theme);
    auto dayHeight = paintProperty->GetDayHeight().value_or(theme->GetCalendarTheme().dayHeight).ConvertToPx();
    auto dayWidth = paintProperty->GetDayWidth().value_or(theme->GetCalendarTheme().dayWidth).ConvertToPx();
    CHECK_NULL_VOID(frameNode);
    auto buttonLayoutProperty = frameNode->GetLayoutProperty<ButtonLayoutProperty>();
    CHECK_NULL_VOID(buttonLayoutProperty);
    buttonLayoutProperty->UpdateUserDefinedIdealSize(CalcSize(CalcLength(dayWidth), CalcLength(dayHeight)));
    auto renderContext = frameNode->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    auto pos = GetXYByIndex(index);
    auto colSpace = paintProperty->GetColSpaceValue({}).ConvertToPx() <= 0
                    ? theme->GetCalendarTheme().colSpace.ConvertToPx()
                    : paintProperty->GetColSpaceValue({}).ConvertToPx();
    Dimension buttonOffsetX = Dimension(colSpace / 2 + (colSpace + dayWidth) * pos.first);
    auto topPadding = isCalendarDialog_ ? 0.0 : theme->GetCalendarTheme().topPadding.ConvertToPx();
    auto weekHeight = paintProperty->GetWeekHeight().value_or(theme->GetCalendarTheme().weekHeight).ConvertToPx();
    auto rowSpace = GetRowSpace(paintProperty, theme, obtainedMonth_.days.size());
    auto weekAndDayRowSpace =
        paintProperty->GetWeekAndDayRowSpace().value_or(theme->GetCalendarTheme().weekAndDayRowSpace).ConvertToPx();
    auto browHeight = weekHeight + topPadding + weekAndDayRowSpace;
    Dimension buttonOffsetY = Dimension(browHeight + (dayHeight + rowSpace) * pos.second - 1);
    renderContext->UpdatePosition(OffsetT(buttonOffsetX, buttonOffsetY));
    DimensionRect hotZoneRegion;
    hotZoneRegion.SetSize(DimensionSize(Dimension(dayWidth + colSpace), Dimension(dayHeight + rowSpace)));
    hotZoneRegion.SetOffset(DimensionOffset(Dimension(buttonOffsetX), Dimension(buttonOffsetY)));
    auto gestureHub = host->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(gestureHub);
    std::vector<DimensionRect> hotZoneRegions;
    hotZoneRegions.emplace_back(hotZoneRegion);
    gestureHub->SetResponseRegion(hotZoneRegions);
    frameNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
}

void CalendarMonthPattern::ChangeVirtualNodeContent(const CalendarDay& calendarDay)
{
    std::string message;
    if (calendarDay.month.year == calendarDay_.month.year && calendarDay.month.month == calendarDay_.month.month &&
                      calendarDay.day == calendarDay_.day) {
        message += Localization::GetInstance()->GetEntryLetters("calendar.today");
    }
    std::string month = Localization::GetInstance()->GetEntryLetters("calendar.month");
    message += std::to_string(calendarDay.month.month) + month;
    std::string day = Localization::GetInstance()->GetEntryLetters("calendar.day");
    message += std::to_string(calendarDay.day) + day;
    std::string week = Localization::GetInstance()->GetEntryLetters("calendar.week");
    message += week + (calendarDay.index % CALENDAR_WEEK_DAYS == 0 ? day :
        std::to_string(calendarDay.index % CALENDAR_WEEK_DAYS));
    auto node = buttonAccessibilityNodeVec_[calendarDay.index];
    CHECK_NULL_VOID(node);
    auto buttonAccessibilityProperty = node->GetAccessibilityProperty<AccessibilityProperty>();
    CHECK_NULL_VOID(buttonAccessibilityProperty);
    buttonAccessibilityProperty->SetAccessibilityText(message);
}

void CalendarMonthPattern::FireModifyAccessibilityVirtualNode(const ObtainedMonth& currentData)
{
    if (isInitVirtualNode_) {
        auto pipeline = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(pipeline);
        pipeline->AddAfterRenderTask([weak = WeakClaim(this), currentData]() {
            auto calendarMonthPattern = weak.Upgrade();
            CHECK_NULL_VOID(calendarMonthPattern);
            calendarMonthPattern->ModifyAccessibilityVirtualNode(currentData);
        });
    }
}

void CalendarMonthPattern::ModifyAccessibilityVirtualNode(const ObtainedMonth& currentData)
{
    if (buttonAccessibilityNodeVec_.size() < 1) {
        return;
    }
    for (auto& day : currentData.days) {
        ChangeVirtualNodeContent(day);
    }
}
} // namespace OHOS::Ace::NG

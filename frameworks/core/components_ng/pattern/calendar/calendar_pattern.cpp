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

#include "core/components_ng/pattern/calendar/calendar_pattern.h"

#include "base/geometry/offset.h"
#include "base/utils/utils.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/calendar/calendar_month_pattern.h"
#include "core/components_ng/pattern/swiper/swiper_event_hub.h"
#include "core/components_ng/pattern/swiper/swiper_layout_property.h"
#include "core/components_ng/pattern/swiper/swiper_pattern.h"

namespace OHOS::Ace::NG {
void CalendarPattern::OnAttachToFrameNode()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->GetRenderContext()->SetClipToFrame(true);
}

bool CalendarPattern::OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, bool skipMeasure, bool skipLayout)
{
    return true;
}

void CalendarPattern::OnModifyDone()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto swiperNode = host->GetChildren().front();
    CHECK_NULL_VOID(swiperNode);
    auto swiperFrameNode = DynamicCast<FrameNode>(swiperNode);
    CHECK_NULL_VOID(swiperFrameNode);
    auto swiperEventHub = swiperFrameNode->GetEventHub<SwiperEventHub>();
    CHECK_NULL_VOID(swiperEventHub);
    auto calendarEventHub = host->GetEventHub<CalendarEventHub>();
    CHECK_NULL_VOID(calendarEventHub);
    auto preFrameNode = AceType::DynamicCast<FrameNode>(swiperFrameNode->GetChildren().front());
    CHECK_NULL_VOID(preFrameNode);
    auto prePattern = preFrameNode->GetPattern<CalendarMonthPattern>();
    CHECK_NULL_VOID(prePattern);
    auto iterator = swiperFrameNode->GetChildren().begin();
    auto currentFrameNode = AceType::DynamicCast<FrameNode>(*(++iterator));
    CHECK_NULL_VOID(currentFrameNode);
    auto currentPattern = currentFrameNode->GetPattern<CalendarMonthPattern>();
    CHECK_NULL_VOID(currentPattern);
    auto nextFrameNode = AceType::DynamicCast<FrameNode>(swiperFrameNode->GetChildren().back());
    CHECK_NULL_VOID(nextFrameNode);
    auto nextPattern = nextFrameNode->GetPattern<CalendarMonthPattern>();
    CHECK_NULL_VOID(nextPattern);
    if (initialize) {
        prePattern->SetMonthData(preMonth_, MonthState::PRE_MONTH);
        prePattern->SetCalendarDay(calendarDay_);
        currentPattern->SetMonthData(currentMonth_, MonthState::CUR_MONTH);
        currentPattern->SetCalendarDay(calendarDay_);
        nextPattern->SetMonthData(nextMonth_, MonthState::NEXT_MONTH);
        nextPattern->SetCalendarDay(calendarDay_);
        if (currentMonth_.days.empty() && calendarEventHub->GetOnRequestDataEvent()) {
            FireFirstRequestData();
        } else {
            initialize = false;
        }
    } else {
        auto swiperPattern = swiperFrameNode->GetPattern<SwiperPattern>();
        CHECK_NULL_VOID(swiperPattern);
        auto currentIndex = swiperPattern->GetCurrentIndex();
        switch (currentIndex) {
            case 0: {
                if (currentPattern->GetMonthData().month != nextMonth_.month) {
                    currentPattern->SetMonthData(nextMonth_, MonthState::NEXT_MONTH);
                    currentPattern->SetCalendarDay(calendarDay_);
                    currentFrameNode->MarkModifyDone();
                    currentFrameNode->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
                }
                if (prePattern->GetMonthData().month != currentMonth_.month) {
                    prePattern->SetMonthData(currentMonth_, MonthState::CUR_MONTH);
                    prePattern->SetCalendarDay(calendarDay_);
                    preFrameNode->MarkModifyDone();
                    preFrameNode->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
                }
                if (nextPattern->GetMonthData().month != preMonth_.month) {
                    nextPattern->SetMonthData(preMonth_, MonthState::PRE_MONTH);
                    nextPattern->SetCalendarDay(calendarDay_);
                    nextFrameNode->MarkModifyDone();
                    nextFrameNode->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
                }
                return;
            }
            case 1: {
                if (currentPattern->GetMonthData().month != currentMonth_.month) {
                    currentPattern->SetMonthData(currentMonth_, MonthState::CUR_MONTH);
                    currentPattern->SetCalendarDay(calendarDay_);
                    currentFrameNode->MarkModifyDone();
                    currentFrameNode->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
                }
                if (prePattern->GetMonthData().month != preMonth_.month) {
                    prePattern->SetMonthData(preMonth_, MonthState::PRE_MONTH);
                    prePattern->SetCalendarDay(calendarDay_);
                    preFrameNode->MarkModifyDone();
                    preFrameNode->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
                }
                if (nextPattern->GetMonthData().month != nextMonth_.month) {
                    nextPattern->SetMonthData(nextMonth_, MonthState::NEXT_MONTH);
                    nextPattern->SetCalendarDay(calendarDay_);
                    nextFrameNode->MarkModifyDone();
                    nextFrameNode->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
                }
                return;
            }
            case 2: {
                if (currentPattern->GetMonthData().month != preMonth_.month) {
                    currentPattern->SetMonthData(preMonth_, MonthState::PRE_MONTH);
                    currentPattern->SetCalendarDay(calendarDay_);
                    currentFrameNode->MarkModifyDone();
                    currentFrameNode->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
                }
                if (prePattern->GetMonthData().month != nextMonth_.month) {
                    prePattern->SetMonthData(nextMonth_, MonthState::NEXT_MONTH);
                    prePattern->SetCalendarDay(calendarDay_);
                    preFrameNode->MarkModifyDone();
                    preFrameNode->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
                }
                if (nextPattern->GetMonthData().month != currentMonth_.month) {
                    nextPattern->SetMonthData(currentMonth_, MonthState::CUR_MONTH);
                    nextPattern->SetCalendarDay(calendarDay_);
                    nextFrameNode->MarkModifyDone();
                    nextFrameNode->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
                }
                return;
            }
            default:
                return;
        }
    }
    auto requestDataCallBack = [weak = WeakClaim(this), swiperEventHub = swiperEventHub]() {
        auto pattern = weak.Upgrade();
        auto direction = swiperEventHub->GetDirection();
        if (direction == NEXT) {
            pattern->FireRequestData(MonthState::NEXT_MONTH);
            pattern->SetMoveDirection(NG::Direction::NEXT);
        } else {
            pattern->FireRequestData(MonthState::PRE_MONTH);
            pattern->SetMoveDirection(NG::Direction::PRE);
        }
    };

    swiperEventHub->SetChangeDoneEvent(requestDataCallBack);

    for (const auto& calendarMonthNode : swiperNode->GetChildren()) {
        auto calenderMonthFrameNode = AceType::DynamicCast<FrameNode>(calendarMonthNode);
        CHECK_NULL_VOID(calenderMonthFrameNode);
        calenderMonthFrameNode->MarkModifyDone();
        calenderMonthFrameNode->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
    }
}

void CalendarPattern::FireFirstRequestData()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto eventHub = GetEventHub<CalendarEventHub>();
    CHECK_NULL_VOID(eventHub);
    auto json = JsonUtil::Create(true);
    auto currentMonth = calendarDay_.month;
    json->Put("currentYear", currentMonth.year);
    json->Put("currentMonth", currentMonth.month);
    json->Put("year", currentMonth.year);
    json->Put("month", currentMonth.month);
    json->Put("MonthState", 0);
    eventHub->UpdateRequestDataEvent(json->ToString());
}

void CalendarPattern::FireRequestData(MonthState monthState)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto eventHub = GetEventHub<CalendarEventHub>();
    CHECK_NULL_VOID(eventHub);
    auto onRequest = eventHub->GetOnRequestDataEvent();
    auto json = JsonUtil::Create(true);
    if (monthState == MonthState::PRE_MONTH) {
        json->Put("MonthState", 1);
    } else if (monthState == MonthState::NEXT_MONTH) {
        json->Put("MonthState", 2);
    }
    json->Put("currentYear", calendarDay_.month.year);
    json->Put("currentMonth", calendarDay_.month.year);
    json->Put("year", currentMonth_.year);
    json->Put("month", currentMonth_.month);
    onRequest(json->ToString());
}

} // namespace OHOS::Ace::NG
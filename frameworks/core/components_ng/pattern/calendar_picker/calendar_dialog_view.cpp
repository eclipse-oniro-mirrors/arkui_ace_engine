/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "core/components_ng/pattern/calendar_picker/calendar_dialog_view.h"

#include <utility>

#include "base/memory/ace_type.h"
#include "base/utils/utils.h"
#include "core/components/common/properties/shadow_config.h"
#include "core/components/theme/icon_theme.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/button/button_pattern.h"
#include "core/components_ng/pattern/calendar/calendar_month_pattern.h"
#include "core/components_ng/pattern/calendar/calendar_paint_property.h"
#include "core/components_ng/pattern/calendar/calendar_pattern.h"
#include "core/components_ng/pattern/calendar_picker/calendar_dialog_pattern.h"
#include "core/components_ng/pattern/dialog/dialog_view.h"
#include "core/components_ng/pattern/divider/divider_pattern.h"
#include "core/components_ng/pattern/image/image_pattern.h"
#include "core/components_ng/pattern/picker/date_time_animation_controller.h"
#include "core/components_ng/pattern/picker/datepicker_pattern.h"
#include "core/components_ng/pattern/picker/datepicker_row_layout_property.h"
#include "core/components_ng/pattern/stack/stack_pattern.h"
#include "core/components_ng/pattern/swiper/swiper_pattern.h"
#include "core/components_ng/property/measure_property.h"
#include "core/components_v2/inspector/inspector_constants.h"

namespace OHOS::Ace::NG {
namespace {
constexpr int32_t SWIPER_MONTHS_COUNT = 3;
constexpr int32_t CURRENT_MONTH_INDEX = 1;
constexpr Dimension DIALOG_WIDTH = 320.0_vp;
} // namespace
RefPtr<FrameNode> CalendarDialogView::Show(const DialogProperties& dialogProperties,
    const CalendarSettingData& settingData, std::map<std::string, NG::DialogEvent> dialogEvent,
    std::map<std::string, NG::DialogGestureEvent> dialogCancelEvent)
{
    auto contentColumn = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<CalendarDialogPattern>());
    auto pattern = contentColumn->GetPattern<CalendarDialogPattern>();
    CHECK_NULL_RETURN(pattern, nullptr);
    pattern->SetEntryNode(settingData.entryNode);
    pattern->SetDialogOffset(OffsetF(dialogProperties.offset.GetX().Value(), dialogProperties.offset.GetY().Value()));
    auto layoutProperty = contentColumn->GetLayoutProperty();
    CHECK_NULL_RETURN(layoutProperty, nullptr);
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipelineContext, nullptr);
    RefPtr<CalendarTheme> theme = pipelineContext->GetTheme<CalendarTheme>();
    RefPtr<DialogTheme> dialogTheme = pipelineContext->GetTheme<DialogTheme>();
    CHECK_NULL_RETURN(theme, nullptr);
    PaddingProperty padding;
    padding.top = CalcLength(theme->GetCalendarTitleRowTopPadding());
    padding.bottom = CalcLength(theme->GetCalendarActionRowBottomLeftRightPadding());
    layoutProperty->UpdatePadding(padding);
    auto renderContext = contentColumn->GetRenderContext();
    CHECK_NULL_RETURN(renderContext, nullptr);
    if (dialogProperties.customStyle) {
        layoutProperty->UpdateUserDefinedIdealSize(CalcSize(NG::CalcLength(DIALOG_WIDTH), std::nullopt));
        BorderRadiusProperty radius;
        radius.SetRadius(dialogTheme->GetRadius().GetX());
        renderContext->UpdateBorderRadius(radius);
    }
    renderContext->UpdateBackShadow(ShadowConfig::DefaultShadowS);
    renderContext->UpdateBackgroundColor(theme->GetDialogBackgroundColor());

    auto calendarNode = CreateCalendarNode(contentColumn, settingData, dialogEvent);
    CHECK_NULL_RETURN(calendarNode, nullptr);
    auto titleNode = CreateTitleNode(calendarNode);
    CHECK_NULL_RETURN(titleNode, nullptr);
    titleNode->MountToParent(contentColumn);
    calendarNode->MountToParent(contentColumn);

    auto dialogNode = DialogView::CreateDialogNode(dialogProperties, contentColumn);
    CHECK_NULL_RETURN(dialogNode, nullptr);

    if (!settingData.entryNode) {
        auto contentRow = CreateOptionsNode(dialogNode, calendarNode, dialogEvent, std::move(dialogCancelEvent));
        contentRow->MountToParent(contentColumn);
    }

    contentColumn->MarkModifyDone();
    calendarNode->MarkModifyDone();
    dialogNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
    return dialogNode;
}

RefPtr<FrameNode> CalendarDialogView::CreateTitleNode(const RefPtr<FrameNode>& calendarNode)
{
    auto titleRow = FrameNode::CreateFrameNode(V2::ROW_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<LinearLayoutPattern>(false));
    CHECK_NULL_RETURN(titleRow, nullptr);
    auto layoutProps = titleRow->GetLayoutProperty<LinearLayoutProperty>();
    CHECK_NULL_RETURN(layoutProps, nullptr);
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipelineContext, nullptr);
    RefPtr<CalendarTheme> theme = pipelineContext->GetTheme<CalendarTheme>();
    CHECK_NULL_RETURN(theme, nullptr);
    layoutProps->UpdateMainAxisAlign(FlexAlign::AUTO);
    layoutProps->UpdateCrossAxisAlign(FlexAlign::CENTER);
    layoutProps->UpdateMeasureType(MeasureType::MATCH_PARENT_MAIN_AXIS);
    MarginProperty margin;
    margin.left = CalcLength(theme->GetCalendarTitleRowLeftRightPadding());
    margin.right = CalcLength(theme->GetCalendarTitleRowLeftRightPadding());
    layoutProps->UpdateMargin(margin);
    layoutProps->UpdateUserDefinedIdealSize(CalcSize(std::nullopt, CalcLength(theme->GetCalendarTitleRowHeight())));

    // left year arrow
    auto leftYearArrowNode =
        CreateTitleImageNode(calendarNode, InternalResource::ResourceId::IC_PUBLIC_DOUBLE_ARROW_LEFT_SVG);
    leftYearArrowNode->MountToParent(titleRow);

    // left day arrow
    auto leftDayArrowNode = CreateTitleImageNode(calendarNode, InternalResource::ResourceId::IC_PUBLIC_ARROW_LEFT_SVG);
    leftDayArrowNode->MountToParent(titleRow);

    // text
    auto calendarPattern = calendarNode->GetPattern<CalendarPattern>();
    CHECK_NULL_RETURN(calendarPattern, nullptr);
    auto textTitleNode =
        FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, calendarPattern->GetTitleId(), AceType::MakeRefPtr<TextPattern>());
    CHECK_NULL_RETURN(textTitleNode, nullptr);
    auto textLayoutProperty = textTitleNode->GetLayoutProperty<TextLayoutProperty>();
    CHECK_NULL_RETURN(textLayoutProperty, nullptr);
    textLayoutProperty->UpdateContent("");
    MarginProperty textMargin;
    textMargin.left = CalcLength(theme->GetCalendarTitleTextPadding());
    textMargin.right = CalcLength(theme->GetCalendarTitleTextPadding());
    textLayoutProperty->UpdateMargin(textMargin);
    textLayoutProperty->UpdateFontSize(theme->GetCalendarTitleFontSize());
    textLayoutProperty->UpdateTextColor(theme->GetCalendarTitleFontColor());
    textLayoutProperty->UpdateFontWeight(FontWeight::MEDIUM);
    textLayoutProperty->UpdateMaxLines(1);
    textLayoutProperty->UpdateLayoutWeight(1);
    textLayoutProperty->UpdateTextAlign(TextAlign::CENTER);
    textTitleNode->MarkModifyDone();
    textTitleNode->MountToParent(titleRow);

    // right day arrow
    auto rightDayArrowNode =
        CreateTitleImageNode(calendarNode, InternalResource::ResourceId::IC_PUBLIC_ARROW_RIGHT_SVG);
    rightDayArrowNode->MountToParent(titleRow);

    // right year arrow
    auto rightYearArrowNode =
        CreateTitleImageNode(calendarNode, InternalResource::ResourceId::IC_PUBLIC_DOUBLE_ARROW_RIGHT_SVG);
    rightYearArrowNode->MountToParent(titleRow);

    return titleRow;
}

RefPtr<FrameNode> CalendarDialogView::CreateTitleImageNode(
    const RefPtr<FrameNode>& calendarNode, const InternalResource::ResourceId& resourceId)
{
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipelineContext, nullptr);
    RefPtr<CalendarTheme> theme = pipelineContext->GetTheme<CalendarTheme>();
    CHECK_NULL_RETURN(theme, nullptr);
    CalcSize idealSize = { CalcLength(theme->GetCalendarImageWidthHeight()),
        CalcLength(theme->GetCalendarImageWidthHeight()) };
    MeasureProperty layoutConstraint = { .selfIdealSize = idealSize };
    BorderRadiusProperty borderRadius;
    Dimension radius =
        Dimension((theme->GetCalendarImageWidthHeight().Value()) / 2, (theme->GetCalendarImageWidthHeight()).Unit());
    borderRadius.SetRadius(radius);

    // Create button node
    auto buttonNode = FrameNode::GetOrCreateFrameNode(V2::BUTTON_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<ButtonPattern>(); });
    CHECK_NULL_RETURN(buttonNode, nullptr);
    auto buttonLayoutProperty = buttonNode->GetLayoutProperty<ButtonLayoutProperty>();
    CHECK_NULL_RETURN(buttonLayoutProperty, nullptr);
    buttonLayoutProperty->UpdateType(ButtonType::CIRCLE);
    auto buttonRenderContext = buttonNode->GetRenderContext();
    CHECK_NULL_RETURN(buttonRenderContext, nullptr);
    buttonRenderContext->UpdateBackgroundColor(Color::TRANSPARENT);
    buttonRenderContext->UpdateBorderRadius(borderRadius);
    MarginProperty margin;
    if (resourceId == InternalResource::ResourceId::IC_PUBLIC_DOUBLE_ARROW_LEFT_SVG) {
        margin.right = CalcLength(theme->GetCalendarTitleImagePadding());
    } else if (resourceId == InternalResource::ResourceId::IC_PUBLIC_DOUBLE_ARROW_RIGHT_SVG) {
        margin.left = CalcLength(theme->GetCalendarTitleImagePadding());
    }
    buttonLayoutProperty->UpdateMargin(margin);

    // Create image node
    auto imageNode = FrameNode::CreateFrameNode(
        V2::IMAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<ImagePattern>());
    CHECK_NULL_RETURN(imageNode, nullptr);
    auto imageRenderContext = imageNode->GetRenderContext();
    CHECK_NULL_RETURN(imageRenderContext, nullptr);
    imageRenderContext->UpdateBorderRadius(borderRadius);
    auto imageLayoutProperty = imageNode->GetLayoutProperty<ImageLayoutProperty>();
    CHECK_NULL_RETURN(imageLayoutProperty, nullptr);
    ImageSourceInfo imageSourceInfo;
    imageSourceInfo.SetResourceId(resourceId, theme->GetEntryArrowColor());
    imageLayoutProperty->UpdateImageSourceInfo(imageSourceInfo);
    imageLayoutProperty->UpdateCalcLayoutProperty(layoutConstraint);
    imageNode->MarkModifyDone();
    imageNode->MountToParent(buttonNode);

    buttonNode->MarkModifyDone();
    return buttonNode;
}

RefPtr<FrameNode> CalendarDialogView::CreateCalendarNode(const RefPtr<FrameNode>& calendarDialogNode,
    const CalendarSettingData& settingData, std::map<std::string, NG::DialogEvent> dialogEvent)
{
    int32_t calendarNodeId = ElementRegister::GetInstance()->MakeUniqueId();
    auto calendarNode = FrameNode::GetOrCreateFrameNode(
        V2::CALENDAR_ETS_TAG, calendarNodeId, []() { return AceType::MakeRefPtr<CalendarPattern>(); });
    CHECK_NULL_RETURN(calendarNode, nullptr);

    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipelineContext, nullptr);
    auto calendarLayoutProperty = calendarNode->GetLayoutProperty();
    CHECK_NULL_RETURN(calendarLayoutProperty, nullptr);
    RefPtr<CalendarTheme> theme = pipelineContext->GetTheme<CalendarTheme>();
    CHECK_NULL_RETURN(theme, nullptr);
    MarginProperty margin = {
        .left = CalcLength(theme->GetDistanceBetweenContainterAndDate()),
        .right = CalcLength(theme->GetDistanceBetweenContainterAndDate()),
        .top = CalcLength(theme->GetDistanceBetweenTitleAndDate()),
    };
    calendarLayoutProperty->UpdateMargin(margin);
    calendarLayoutProperty->UpdateUserDefinedIdealSize(
        CalcSize(std::nullopt, CalcLength(theme->GetCalendarContainerHeight())));

    auto swiperNode = CreateCalendarSwiperNode();
    CHECK_NULL_RETURN(swiperNode, nullptr);
    swiperNode->MountToParent(calendarNode);

    InitOnRequestDataEvent(calendarDialogNode, calendarNode);
    auto calendarPattern = calendarNode->GetPattern<CalendarPattern>();
    CHECK_NULL_RETURN(calendarPattern, nullptr);
    PickerDate date = settingData.selectedDate;
    calendarPattern->SetSelectedDay(date);
    CalendarMonth currentMonth { .year = date.GetYear(), .month = date.GetMonth() };
    UpdateCalendarMonthData(calendarDialogNode, calendarNode, currentMonth);

    CalendarDay calendarDay;
    PickerDate today = PickerDate::Current();
    calendarDay.month.year = static_cast<int32_t>(today.GetYear());
    calendarDay.month.month = static_cast<int32_t>(today.GetMonth());
    calendarDay.day = static_cast<int32_t>(today.GetDay());
    calendarPattern->SetCalendarDay(calendarDay);

    auto changeEvent = dialogEvent["changeId"];
    for (int32_t i = 0; i < SWIPER_MONTHS_COUNT; i++) {
        auto monthFrameNode = CreateCalendarMonthNode(calendarNodeId, settingData, changeEvent);
        monthFrameNode->MountToParent(swiperNode);
        monthFrameNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
    }

    swiperNode->MarkModifyDone();
    return calendarNode;
}

RefPtr<FrameNode> CalendarDialogView::CreateCalendarSwiperNode()
{
    auto swiperNode = FrameNode::GetOrCreateFrameNode(V2::SWIPER_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<SwiperPattern>(); });
    CHECK_NULL_RETURN(swiperNode, nullptr);
    auto swiperPaintProperty = swiperNode->GetPaintProperty<SwiperPaintProperty>();
    CHECK_NULL_RETURN(swiperPaintProperty, nullptr);
    swiperPaintProperty->UpdateDisableSwipe(false);
    auto swiperLayoutProperty = swiperNode->GetLayoutProperty<SwiperLayoutProperty>();
    CHECK_NULL_RETURN(swiperLayoutProperty, nullptr);
    swiperLayoutProperty->UpdateIndex(CURRENT_MONTH_INDEX);
    swiperLayoutProperty->UpdateShowIndicator(false);
    swiperLayoutProperty->UpdateMeasureType(MeasureType::MATCH_PARENT);
    swiperLayoutProperty->UpdateLoop(true);
    return swiperNode;
}

RefPtr<FrameNode> CalendarDialogView::CreateCalendarMonthNode(int32_t calendarNodeId,
    const CalendarSettingData& settingData, DialogEvent& changeEvent)
{
    auto monthFrameNode = FrameNode::GetOrCreateFrameNode(V2::CALENDAR_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<CalendarMonthPattern>(); });
    CHECK_NULL_RETURN(monthFrameNode, nullptr);
    ViewStackProcessor::GetInstance()->Push(monthFrameNode);
    SetCalendarPaintProperties(settingData);
    ViewStackProcessor::GetInstance()->Finish();
    auto monthPattern = monthFrameNode->GetPattern<CalendarMonthPattern>();
    CHECK_NULL_RETURN(monthPattern, nullptr);
    monthPattern->SetCalendarDialogFlag(true);
    auto calendarEventHub = monthPattern->GetEventHub<CalendarEventHub>();
    CHECK_NULL_RETURN(calendarEventHub, nullptr);
    auto selectedChangeEvent = [calendarNodeId, changeEvent](const std::string& callbackInfo) {
        OnSelectedChangeEvent(calendarNodeId, callbackInfo, std::move(changeEvent));
    };
    calendarEventHub->SetSelectedChangeEvent(std::move(selectedChangeEvent));

    auto monthLayoutProperty = monthFrameNode->GetLayoutProperty<LayoutProperty>();
    CHECK_NULL_RETURN(monthLayoutProperty, nullptr);
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipelineContext, nullptr);
    RefPtr<CalendarTheme> theme = pipelineContext->GetTheme<CalendarTheme>();
    CHECK_NULL_RETURN(theme, nullptr);
    monthLayoutProperty->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(1, DimensionUnit::PERCENT), CalcLength(theme->GetCalendarContainerHeight())));
    return monthFrameNode;
}

void CalendarDialogView::UpdateCalendarMonthData(const RefPtr<FrameNode>& calendarDialogNode,
    const RefPtr<FrameNode>& calendarNode, const CalendarMonth& currentMonth)
{
    CHECK_NULL_VOID(calendarNode);
    CHECK_NULL_VOID(calendarDialogNode);
    auto calendarPattern = calendarNode->GetPattern<CalendarPattern>();
    CHECK_NULL_VOID(calendarPattern);
    auto calendarDialogPattern = calendarDialogNode->GetPattern<CalendarDialogPattern>();
    CHECK_NULL_VOID(calendarDialogPattern);

    CalendarData calendarData;
    calendarDialogPattern->GetCalendarMonthData(currentMonth.year, currentMonth.month, calendarData.currentData);
    calendarPattern->SetCurrentMonthData(calendarData.currentData);
    calendarDialogPattern->GetCalendarMonthData(calendarDialogPattern->GetLastMonth(currentMonth).year,
        calendarDialogPattern->GetLastMonth(currentMonth).month, calendarData.preData);
    calendarPattern->SetPreMonthData(calendarData.preData);
    calendarDialogPattern->GetCalendarMonthData(calendarDialogPattern->GetNextMonth(currentMonth).year,
        calendarDialogPattern->GetNextMonth(currentMonth).month, calendarData.nextData);
    calendarPattern->SetNextMonthData(calendarData.nextData);
}

void CalendarDialogView::SetDialogChange(const RefPtr<FrameNode>& frameNode, DialogEvent&& onChange)
{
    CHECK_NULL_VOID(frameNode);
    auto eventHub = frameNode->GetEventHub<CalendarEventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->SetDialogChange(std::move(onChange));
}

void CalendarDialogView::SetDialogAcceptEvent(const RefPtr<FrameNode>& frameNode, DialogEvent&& onAccept)
{
    CHECK_NULL_VOID(frameNode);
    auto eventHub = frameNode->GetEventHub<CalendarEventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->SetDialogAcceptEvent(std::move(onAccept));
}

RefPtr<FrameNode> CalendarDialogView::CreateButtonNode(bool isConfirm)
{
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, nullptr);
    auto dialogTheme = pipeline->GetTheme<DialogTheme>();
    auto pickerTheme = pipeline->GetTheme<PickerTheme>();
    auto buttonNode = FrameNode::GetOrCreateFrameNode(V2::BUTTON_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<ButtonPattern>(); });
    CHECK_NULL_RETURN(buttonNode, nullptr);
    auto textNode = FrameNode::CreateFrameNode(
        V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
    CHECK_NULL_RETURN(textNode, nullptr);
    auto textLayoutProperty = textNode->GetLayoutProperty<TextLayoutProperty>();
    CHECK_NULL_RETURN(textLayoutProperty, nullptr);
    textLayoutProperty->UpdateContent(
        Localization::GetInstance()->GetEntryLetters(isConfirm ? "common.ok" : "common.cancel"));
    textLayoutProperty->UpdateTextColor(pickerTheme->GetOptionStyle(true, false).GetTextColor());
    textLayoutProperty->UpdateFontSize(pickerTheme->GetOptionStyle(false, false).GetFontSize());
    textLayoutProperty->UpdateFontWeight(pickerTheme->GetOptionStyle(true, false).GetFontWeight());

    auto buttonLayoutProperty = buttonNode->GetLayoutProperty<ButtonLayoutProperty>();
    CHECK_NULL_RETURN(buttonLayoutProperty, nullptr);
    buttonLayoutProperty->UpdateMeasureType(MeasureType::MATCH_PARENT_MAIN_AXIS);
    buttonLayoutProperty->UpdateType(ButtonType::CAPSULE);
    buttonLayoutProperty->UpdateFlexShrink(1.0);
    buttonLayoutProperty->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(pickerTheme->GetButtonWidth()), CalcLength(pickerTheme->GetButtonHeight())));
    buttonNode->GetRenderContext()->UpdateBackgroundColor(Color::TRANSPARENT);

    MarginProperty margin;
    if (isConfirm) {
        margin.right = CalcLength(dialogTheme->GetDividerPadding().Right());
    } else {
        margin.left = CalcLength(dialogTheme->GetDividerPadding().Left());
    }
    buttonNode->GetLayoutProperty()->UpdateMargin(margin);

    auto buttonEventHub = buttonNode->GetEventHub<ButtonEventHub>();
    CHECK_NULL_RETURN(buttonEventHub, nullptr);
    buttonEventHub->SetStateEffect(true);

    textNode->MountToParent(buttonNode);
    buttonNode->MarkModifyDone();
    return buttonNode;
}

RefPtr<FrameNode> CalendarDialogView::CreateConfirmNode(const RefPtr<FrameNode>& calendarNode, DialogEvent& acceptEvent)
{
    CHECK_NULL_RETURN(calendarNode, nullptr);
    auto buttonConfirmNode = CreateButtonNode(true);

    auto eventConfirmHub = buttonConfirmNode->GetOrCreateGestureEventHub();
    CHECK_NULL_RETURN(eventConfirmHub, nullptr);
    SetDialogAcceptEvent(calendarNode, std::move(acceptEvent));
    auto clickCallback = [calendarNode](const GestureEvent& /* info */) {
        auto calendarPattern = calendarNode->GetPattern<CalendarPattern>();
        CHECK_NULL_VOID(calendarPattern);
        auto str = calendarPattern->GetSelectDate();
        auto calendarEventHub = calendarNode->GetEventHub<CalendarEventHub>();
        CHECK_NULL_VOID(calendarEventHub);
        calendarEventHub->FireDialogAcceptEvent(str);
    };
    eventConfirmHub->AddClickEvent(AceType::MakeRefPtr<NG::ClickEvent>(std::move(clickCallback)));
    return buttonConfirmNode;
}

RefPtr<FrameNode> CalendarDialogView::CreateCancelNode(NG::DialogGestureEvent& cancelEvent)
{
    auto buttonCancelNode = CreateButtonNode(false);

    auto eventCancelHub = buttonCancelNode->GetOrCreateGestureEventHub();
    CHECK_NULL_RETURN(eventCancelHub, nullptr);
    eventCancelHub->AddClickEvent(AceType::MakeRefPtr<NG::ClickEvent>(std::move(cancelEvent)));
    return buttonCancelNode;
}

RefPtr<FrameNode> CalendarDialogView::CreateDividerNode()
{
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, nullptr);
    auto dialogTheme = pipeline->GetTheme<DialogTheme>();
    RefPtr<CalendarTheme> theme = pipeline->GetTheme<CalendarTheme>();
    auto dividerNode = FrameNode::GetOrCreateFrameNode(V2::DIVIDER_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<DividerPattern>(); });
    auto dividerRenderContext = dividerNode->GetRenderContext();
    CHECK_NULL_RETURN(dividerRenderContext, nullptr);
    dividerRenderContext->UpdateBackgroundColor(theme->GetDialogDividerColor());

    MarginProperty margin;
    dividerNode->GetLayoutProperty()->UpdateMargin(margin);
    dividerNode->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(dialogTheme->GetDividerWidth()), CalcLength(dialogTheme->GetDividerHeight())));

    return dividerNode;
}

RefPtr<FrameNode> CalendarDialogView::CreateOptionsNode(
    const RefPtr<FrameNode>& dialogNode, const RefPtr<FrameNode>& dateNode,
    std::map<std::string, NG::DialogEvent> dialogEvent,
    std::map<std::string, NG::DialogGestureEvent> dialogCancelEvent)
{
    auto acceptEvent = dialogEvent["acceptId"];
    auto cancelEvent = dialogCancelEvent["cancelId"];
    auto contentRow = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<LinearLayoutPattern>(false));
    CHECK_NULL_RETURN(contentRow, nullptr);
    auto layoutProps = contentRow->GetLayoutProperty<LinearLayoutProperty>();
    CHECK_NULL_RETURN(layoutProps, nullptr);
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipelineContext, nullptr);
    RefPtr<CalendarTheme> theme = pipelineContext->GetTheme<CalendarTheme>();
    CHECK_NULL_RETURN(theme, nullptr);
    layoutProps->UpdateMainAxisAlign(FlexAlign::SPACE_AROUND);
    layoutProps->UpdateMeasureType(MeasureType::MATCH_PARENT_MAIN_AXIS);
    MarginProperty margin;
    margin.top = CalcLength(theme->GetCalendarActionRowTopPadding());
    margin.left = CalcLength(theme->GetCalendarActionRowBottomLeftRightPadding());
    margin.right = CalcLength(theme->GetCalendarActionRowBottomLeftRightPadding());
    layoutProps->UpdateMargin(margin);
    layoutProps->UpdateUserDefinedIdealSize(CalcSize(std::nullopt, CalcLength(theme->GetCalendarActionRowHeight())));

    auto buttonCancelNode = CreateCancelNode(cancelEvent);
    auto buttonConfirmNode = CreateConfirmNode(dateNode, acceptEvent);

    buttonCancelNode->MountToParent(contentRow);
    buttonConfirmNode->MountToParent(contentRow);

    auto event = [dialogNode, pipelineContext](const GestureEvent& /* info */) {
        auto overlayManager = pipelineContext->GetOverlayManager();
        CHECK_NULL_VOID(overlayManager);
        overlayManager->CloseDialog(dialogNode);
    };
    for (const auto& child : contentRow->GetChildren()) {
        auto firstChild = AceType::DynamicCast<FrameNode>(child);
        auto gesturHub = firstChild->GetOrCreateGestureEventHub();
        auto onClick = AceType::MakeRefPtr<NG::ClickEvent>(event);
        gesturHub->AddClickEvent(onClick);
    }
    contentRow->AddChild(CreateDividerNode(), 1);
    return contentRow;
}

void CalendarDialogView::SetCalendarPaintProperties(const CalendarSettingData& settingData)
{
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    RefPtr<CalendarTheme> theme = pipelineContext->GetTheme<CalendarTheme>();
    CHECK_NULL_VOID(theme);
    ACE_UPDATE_PAINT_PROPERTY(CalendarPaintProperty, DayHeight, theme->GetCalendarPickerDayWidthOrHeight());
    ACE_UPDATE_PAINT_PROPERTY(CalendarPaintProperty, DayWidth, theme->GetCalendarPickerDayWidthOrHeight());
    ACE_UPDATE_PAINT_PROPERTY(CalendarPaintProperty, DayFontSize, theme->GetCalendarDayFontSize());
    ACE_UPDATE_PAINT_PROPERTY(CalendarPaintProperty, WeekHeight, theme->GetCalendarPickerDayWidthOrHeight());
    ACE_UPDATE_PAINT_PROPERTY(CalendarPaintProperty, WeekWidth, theme->GetCalendarPickerDayWidthOrHeight());
    ACE_UPDATE_PAINT_PROPERTY(CalendarPaintProperty, WeekFontSize, theme->GetCalendarDayFontSize());
    ACE_UPDATE_PAINT_PROPERTY(CalendarPaintProperty, WeekColor, theme->GetTextCurrentMonthColor());
    if (settingData.dayRadius.has_value()) {
        ACE_UPDATE_PAINT_PROPERTY(CalendarPaintProperty, DayRadius, settingData.dayRadius.value());
    }
}

void CalendarDialogView::InitOnRequestDataEvent(
    const RefPtr<FrameNode>& calendarDialogNode, const RefPtr<FrameNode>& calendarNode)
{
    auto callback = [calendarDialogNode, calendarNode](const std::string& info) {
        CHECK_NULL_VOID(calendarNode);
        auto jsonInfo = JsonUtil::ParseJsonString(info);
        CalendarMonth currentMonth { .year = jsonInfo->GetInt("year"), .month = jsonInfo->GetInt("month") };
        UpdateCalendarMonthData(calendarDialogNode, calendarNode, currentMonth);

        calendarNode->MarkModifyDone();
        calendarNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF_AND_CHILD);
    };
    auto eventHub = calendarNode->GetEventHub<CalendarEventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->SetOnRequestDataEvent(std::move(callback));
}

void CalendarDialogView::OnSelectedChangeEvent(
    int32_t calendarNodeId, const std::string& callbackInfo, const DialogEvent&& onChange)
{
    auto calendarNode = FrameNode::GetOrCreateFrameNode(
        V2::CALENDAR_ETS_TAG, calendarNodeId, []() { return AceType::MakeRefPtr<CalendarPattern>(); });
    CHECK_NULL_VOID(calendarNode);
    auto calendarPattern = calendarNode->GetPattern<CalendarPattern>();
    CHECK_NULL_VOID(calendarPattern);
    auto currentMonthData = calendarPattern->GetCurrentMonthData();
    auto jsonInfo = JsonUtil::ParseJsonString(callbackInfo);
    CalendarMonth selectedMonth { .year = jsonInfo->GetInt("year"), .month = jsonInfo->GetInt("month") };
    if (currentMonthData.year != selectedMonth.year || currentMonthData.month != selectedMonth.month) {
        return;
    }

    if (onChange) {
        onChange(callbackInfo);
    }
    PickerDate selectedDay(selectedMonth.year, selectedMonth.month, jsonInfo->GetInt("day"));
    calendarPattern->SetSelectedDay(selectedDay);
    calendarNode->MarkModifyDone();
}
} // namespace OHOS::Ace::NG

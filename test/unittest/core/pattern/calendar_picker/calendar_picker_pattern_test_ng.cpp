/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <cstddef>
#include <optional>
#include <string>
#include <utility>

#include "gtest/gtest.h"

#define private public
#define protected public

#include "test/mock/base/mock_task_executor.h"
#include "test/mock/core/common/mock_theme_manager.h"
#include "test/mock/core/pipeline/mock_pipeline_context.h"
#include "test/mock/core/render/mock_render_context.h"

#include "base/geometry/axis.h"
#include "base/geometry/dimension.h"
#include "base/geometry/ng/size_t.h"
#include "base/geometry/offset.h"
#include "base/json/json_util.h"
#include "base/memory/ace_type.h"
#include "base/utils/utils.h"
#include "core/components/calendar/calendar_theme.h"
#include "core/components/common/properties/shadow_config.h"
#include "core/components/theme/icon_theme.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/event/focus_hub.h"
#include "core/components_ng/pattern/button/button_pattern.h"
#include "core/components_ng/pattern/calendar/calendar_month_pattern.h"
#include "core/components_ng/pattern/calendar/calendar_paint_property.h"
#include "core/components_ng/pattern/calendar/calendar_pattern.h"
#include "core/components_ng/pattern/calendar_picker/calendar_dialog_pattern.h"
#include "core/components_ng/pattern/calendar_picker/calendar_dialog_view.h"
#include "core/components_ng/pattern/calendar_picker/calendar_picker_event_hub.h"
#include "core/components_ng/pattern/calendar_picker/calendar_picker_layout_algorithm.h"
#include "core/components_ng/pattern/calendar_picker/calendar_picker_layout_property.h"
#include "core/components_ng/pattern/calendar_picker/calendar_picker_model_ng.h"
#include "core/components_ng/pattern/dialog/dialog_view.h"
#include "core/components_ng/pattern/divider/divider_pattern.h"
#include "core/components_ng/pattern/flex/flex_layout_pattern.h"
#include "core/components_ng/pattern/flex/flex_layout_property.h"
#include "core/components_ng/pattern/image/image_pattern.h"
#include "core/components_ng/pattern/picker/date_time_animation_controller.h"
#include "core/components_ng/pattern/picker/datepicker_pattern.h"
#include "core/components_ng/pattern/picker/datepicker_row_layout_property.h"
#include "core/components_ng/pattern/stack/stack_pattern.h"
#include "core/components_ng/pattern/swiper/swiper_pattern.h"
#include "core/components_ng/pattern/text/text_layout_property.h"
#include "core/components_ng/pattern/text/text_pattern.h"
#include "core/components_ng/pattern/text_field/text_field_pattern.h"
#include "core/components_ng/property/measure_property.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/pipeline/base/element_register.h"
#include "core/pipeline_ng/ui_task_scheduler.h"

#undef private
#undef protected

using namespace testing;
using namespace testing::ext;
namespace OHOS::Ace::NG {
namespace {
const InspectorFilter filter;
constexpr Dimension TEST_SETTING_RADIUS = Dimension(10.0, DimensionUnit::VP);
} // namespace
class CalendarPickerPatternTestNg : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void MockPipelineContextGetTheme();

protected:
    static void CreateCalendarPicker();
    static RefPtr<FrameNode> CalendarDialogShow(RefPtr<FrameNode> entryNode);
};

void CalendarPickerPatternTestNg::SetUpTestCase()
{
    MockPipelineContext::SetUp();
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly([](ThemeType type) -> RefPtr<Theme> {
        if (type == CalendarTheme::TypeId()) {
            return AceType::MakeRefPtr<CalendarTheme>();
        } else if (type == IconTheme::TypeId()) {
            return AceType::MakeRefPtr<IconTheme>();
        } else if (type == DialogTheme::TypeId()) {
            return AceType::MakeRefPtr<DialogTheme>();
        } else {
            return AceType::MakeRefPtr<PickerTheme>();
        }
    });
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
}

void CalendarPickerPatternTestNg::TearDownTestCase()
{
    MockPipelineContext::GetCurrent()->SetThemeManager(nullptr);
    MockPipelineContext::TearDown();
}

void CalendarPickerPatternTestNg::CreateCalendarPicker()
{
    CalendarSettingData settingData;
    CalendarPickerModelNG calendarPickerModel;

    calendarPickerModel.Create(settingData);
    DimensionOffset offset;
    calendarPickerModel.SetEdgeAlign(CalendarEdgeAlign::EDGE_ALIGN_START, offset);

    PickerTextStyle textStyle;
    calendarPickerModel.SetTextStyle(textStyle);
    auto onChange = [](const std::string& /* info */) {};
    calendarPickerModel.SetOnChange(onChange);
    calendarPickerModel.SetChangeEvent(onChange);
}

RefPtr<FrameNode> CalendarPickerPatternTestNg::CalendarDialogShow(RefPtr<FrameNode> entryNode)
{
    CalendarSettingData settingData;
    DialogProperties properties;
    properties.alignment = DialogAlignment::BOTTOM;
    properties.customStyle = false;
    properties.offset = DimensionOffset(Offset(0, -1.0f));
    auto selectedDate = PickerDate(2000, 1, 1);
    settingData.selectedDate = selectedDate;
    settingData.dayRadius = TEST_SETTING_RADIUS;
    settingData.entryNode = entryNode;
    std::map<std::string, NG::DialogEvent> dialogEvent;
    std::map<std::string, NG::DialogGestureEvent> dialogCancelEvent;
    std::vector<ButtonInfo> buttonInfos;
    auto dialogNode = CalendarDialogView::Show(properties, settingData, buttonInfos, dialogEvent, dialogCancelEvent);
    return dialogNode;
}

/**
 * @tc.name: HandleHoverEvent001
 * @tc.desc: HandleHoverEvent Function Test
 * @tc.type: FUNC
 */
HWTEST_F(CalendarPickerPatternTestNg, HandleHoverEvent001, TestSize.Level1)
{
    CreateCalendarPicker();

    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish();
    EXPECT_EQ(element->GetTag(), V2::CALENDAR_PICKER_ETS_TAG);

    auto frameNode = AceType::DynamicCast<FrameNode>(element);
    ASSERT_NE(frameNode, nullptr);

    auto pickerPattern = frameNode->GetPattern<CalendarPickerPattern>();
    ASSERT_NE(pickerPattern, nullptr);

    auto json = JsonUtil::Create(true);
    pickerPattern->isKeyWaiting_ = true;
    json->Put("year", 800);
    json->Put("month", 2);
    json->Put("day", 29);
    pickerPattern->SetDate(json->ToString());

    Offset globalLocation(2, 1);
    pickerPattern->HandleHoverEvent(true, globalLocation);
    EXPECT_EQ(pickerPattern->CheckRegion(globalLocation), CalendarPickerSelectedType::OTHER);
}

/**
 * @tc.name: HandleTouchEvent001
 * @tc.desc: HandleTouchEvent Function Test
 * @tc.type: FUNC
 */
HWTEST_F(CalendarPickerPatternTestNg, HandleTouchEvent001, TestSize.Level1)
{
    CreateCalendarPicker();

    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish();
    EXPECT_EQ(element->GetTag(), V2::CALENDAR_PICKER_ETS_TAG);

    auto frameNode = AceType::DynamicCast<FrameNode>(element);
    ASSERT_NE(frameNode, nullptr);

    auto pickerPattern = frameNode->GetPattern<CalendarPickerPattern>();
    ASSERT_NE(pickerPattern, nullptr);

    auto json = JsonUtil::Create(true);
    pickerPattern->isKeyWaiting_ = true;
    json->Put("year", 800);
    json->Put("month", 2);
    json->Put("day", 29);
    pickerPattern->SetDate(json->ToString());

    Offset globalLocation(800, 0);
    pickerPattern->HandleTouchEvent(true, globalLocation);
    pickerPattern->HandleTouchEvent(false, globalLocation);
    EXPECT_EQ(pickerPattern->CheckRegion(globalLocation), CalendarPickerSelectedType::OTHER);
}

/**
 * @tc.name: HandleClickEvent001
 * @tc.desc: HandleClickEvent Function Test
 * @tc.type: FUNC
 */
HWTEST_F(CalendarPickerPatternTestNg, HandleClickEvent001, TestSize.Level1)
{
    CreateCalendarPicker();

    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish();
    EXPECT_EQ(element->GetTag(), V2::CALENDAR_PICKER_ETS_TAG);

    auto frameNode = AceType::DynamicCast<FrameNode>(element);
    ASSERT_NE(frameNode, nullptr);

    auto pickerPattern = frameNode->GetPattern<CalendarPickerPattern>();
    ASSERT_NE(pickerPattern, nullptr);

    Offset globalLocation(800, 0);
    pickerPattern->SetDialogShow(true);
    pickerPattern->HandleClickEvent(globalLocation);
}

/**
 * @tc.name: HandleKeyEvent001
 * @tc.desc: HandleKeyEvent Function Test
 * @tc.type: FUNC
 */
HWTEST_F(CalendarPickerPatternTestNg, HandleKeyEvent001, TestSize.Level1)
{
    CreateCalendarPicker();

    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish();
    EXPECT_EQ(element->GetTag(), V2::CALENDAR_PICKER_ETS_TAG);

    auto frameNode = AceType::DynamicCast<FrameNode>(element);
    ASSERT_NE(frameNode, nullptr);

    auto pickerPattern = frameNode->GetPattern<CalendarPickerPattern>();
    ASSERT_NE(pickerPattern, nullptr);

    KeyEvent event;
    event.action = KeyAction::DOWN;
    event.code = KeyCode::KEY_TAB;
    pickerPattern->isFirtFocus_ = true;
    pickerPattern->HandleKeyEvent(event);

    event.action = KeyAction::DOWN;
    event.code = KeyCode::KEY_TAB;
    pickerPattern->isFirtFocus_ = false;
    pickerPattern->HandleKeyEvent(event);

    event.action = KeyAction::DOWN;
    event.code = KeyCode::KEY_SHIFT_LEFT;
    pickerPattern->isFirtFocus_ = true;
    pickerPattern->HandleKeyEvent(event);

    event.action = KeyAction::DOWN;
    event.code = KeyCode::KEY_SHIFT_LEFT;
    pickerPattern->isFirtFocus_ = false;
    pickerPattern->HandleKeyEvent(event);

    event.action = KeyAction::UP;
    event.code = KeyCode::KEY_TAB;
    pickerPattern->isFirtFocus_ = true;
    pickerPattern->HandleKeyEvent(event);

    event.action = KeyAction::UP;
    event.code = KeyCode::KEY_TAB;
    pickerPattern->isFirtFocus_ = false;
    pickerPattern->HandleKeyEvent(event);

    event.action = KeyAction::UP;
    event.code = KeyCode::KEY_SHIFT_LEFT;
    pickerPattern->isFirtFocus_ = true;
    pickerPattern->HandleKeyEvent(event);

    event.action = KeyAction::UP;
    event.code = KeyCode::KEY_SHIFT_LEFT;
    pickerPattern->isFirtFocus_ = false;
    bool res = pickerPattern->HandleKeyEvent(event);

    EXPECT_FALSE(res);
}

/**
 * @tc.name: HandleFocusEvent001
 * @tc.desc: HandleFocusEvent Function Test
 * @tc.type: FUNC
 */
HWTEST_F(CalendarPickerPatternTestNg, HandleFocusEvent001, TestSize.Level1)
{
    CreateCalendarPicker();

    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish();
    EXPECT_EQ(element->GetTag(), V2::CALENDAR_PICKER_ETS_TAG);

    auto frameNode = AceType::DynamicCast<FrameNode>(element);
    ASSERT_NE(frameNode, nullptr);

    auto pickerPattern = frameNode->GetPattern<CalendarPickerPattern>();
    ASSERT_NE(pickerPattern, nullptr);

    KeyEvent event;
    event.code = KeyCode::KEY_DPAD_UP;
    pickerPattern->isFirtFocus_ = true;
    pickerPattern->selected_ = CalendarPickerSelectedType::OTHER;
    pickerPattern->HandleFocusEvent(event);

    pickerPattern->isFirtFocus_ = false;
    pickerPattern->selected_ = CalendarPickerSelectedType::OTHER;
    pickerPattern->HandleFocusEvent(event);

    pickerPattern->isFirtFocus_ = true;
    pickerPattern->selected_ = CalendarPickerSelectedType::SUBBTN;
    pickerPattern->HandleFocusEvent(event);

    pickerPattern->isFirtFocus_ = false;
    pickerPattern->selected_ = CalendarPickerSelectedType::SUBBTN;
    pickerPattern->HandleFocusEvent(event);

    event.code = KeyCode::KEY_DPAD_DOWN;
    pickerPattern->isFirtFocus_ = true;
    pickerPattern->selected_ = CalendarPickerSelectedType::OTHER;
    pickerPattern->HandleFocusEvent(event);

    pickerPattern->isFirtFocus_ = false;
    pickerPattern->selected_ = CalendarPickerSelectedType::OTHER;
    pickerPattern->HandleFocusEvent(event);

    pickerPattern->isFirtFocus_ = true;
    pickerPattern->selected_ = CalendarPickerSelectedType::SUBBTN;
    pickerPattern->HandleFocusEvent(event);

    pickerPattern->isFirtFocus_ = false;
    pickerPattern->selected_ = CalendarPickerSelectedType::SUBBTN;
    bool res = pickerPattern->HandleFocusEvent(event);

    EXPECT_TRUE(res);
}

/**
 * @tc.name: HandleYearKeyWaitingEvent001
 * @tc.desc: HandleYearKeyWaitingEvent Function Test
 * @tc.type: FUNC
 */
HWTEST_F(CalendarPickerPatternTestNg, HandleYearKeyWaitingEvent001, TestSize.Level1)
{
    CreateCalendarPicker();

    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish();
    EXPECT_EQ(element->GetTag(), V2::CALENDAR_PICKER_ETS_TAG);

    auto frameNode = AceType::DynamicCast<FrameNode>(element);
    ASSERT_NE(frameNode, nullptr);

    auto pickerPattern = frameNode->GetPattern<CalendarPickerPattern>();
    ASSERT_NE(pickerPattern, nullptr);

    std::function<void()> task = []() {};

    std::function<void()> zeroStartTask = []() {};
    pickerPattern->yearPrefixZeroCount_ = 2;
    pickerPattern->yearEnterCount_ = 3;
    pickerPattern->HandleYearKeyWaitingEvent(0, task, zeroStartTask);

    pickerPattern->yearPrefixZeroCount_ = 3;
    pickerPattern->yearEnterCount_ = 4;
    bool res = pickerPattern->HandleYearKeyWaitingEvent(0, task, zeroStartTask);
    EXPECT_FALSE(res);
}

/**
 * @tc.name: HandleYearKeyEvent001
 * @tc.desc: HandleYearKeyEvent Function Test
 * @tc.type: FUNC
 */
HWTEST_F(CalendarPickerPatternTestNg, HandleYearKeyEvent001, TestSize.Level1)
{
    CreateCalendarPicker();

    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish();
    EXPECT_EQ(element->GetTag(), V2::CALENDAR_PICKER_ETS_TAG);

    auto frameNode = AceType::DynamicCast<FrameNode>(element);
    ASSERT_NE(frameNode, nullptr);

    auto pickerPattern = frameNode->GetPattern<CalendarPickerPattern>();
    ASSERT_NE(pickerPattern, nullptr);

    pickerPattern->yearEnterCount_ = 5;
    bool res = pickerPattern->HandleYearKeyEvent(0);
    EXPECT_FALSE(res);
}

/**
 * @tc.name: HandleMonthKeyEvent001
 * @tc.desc: HandleMonthKeyEvent Function Test
 * @tc.type: FUNC
 */
HWTEST_F(CalendarPickerPatternTestNg, HandleMonthKeyEvent001, TestSize.Level1)
{
    CreateCalendarPicker();

    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish();
    EXPECT_EQ(element->GetTag(), V2::CALENDAR_PICKER_ETS_TAG);

    auto frameNode = AceType::DynamicCast<FrameNode>(element);
    ASSERT_NE(frameNode, nullptr);

    auto pickerPattern = frameNode->GetPattern<CalendarPickerPattern>();
    ASSERT_NE(pickerPattern, nullptr);

    pickerPattern->isKeyWaiting_ = true;

    pickerPattern->monthPrefixZeroCount_ = 1;
    pickerPattern->HandleMonthKeyEvent(0);
    pickerPattern->monthPrefixZeroCount_ = 0;
    pickerPattern->HandleMonthKeyEvent(0);

    pickerPattern->monthPrefixZeroCount_ = 1;
    pickerPattern->HandleMonthKeyEvent(1);
    pickerPattern->monthPrefixZeroCount_ = 0;
    bool res = pickerPattern->HandleMonthKeyEvent(1);
    EXPECT_TRUE(res);
}

/**
 * @tc.name: HandleDayKeyEvent001
 * @tc.desc: HandleDayKeyEvent Function Test
 * @tc.type: FUNC
 */
HWTEST_F(CalendarPickerPatternTestNg, HandleDayKeyEvent001, TestSize.Level1)
{
    CreateCalendarPicker();

    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish();
    EXPECT_EQ(element->GetTag(), V2::CALENDAR_PICKER_ETS_TAG);

    auto frameNode = AceType::DynamicCast<FrameNode>(element);
    ASSERT_NE(frameNode, nullptr);

    auto pickerPattern = frameNode->GetPattern<CalendarPickerPattern>();
    ASSERT_NE(pickerPattern, nullptr);

    pickerPattern->isKeyWaiting_ = true;

    pickerPattern->dayPrefixZeroCount_ = 1;
    pickerPattern->HandleDayKeyEvent(0);
    pickerPattern->dayPrefixZeroCount_ = 0;
    pickerPattern->HandleDayKeyEvent(0);

    pickerPattern->dayPrefixZeroCount_ = 1;
    pickerPattern->HandleDayKeyEvent(1);
    pickerPattern->dayPrefixZeroCount_ = 0;
    bool res = pickerPattern->HandleDayKeyEvent(1);
    EXPECT_TRUE(res);
}

/**
 * @tc.name: CalculateDialogOffset001
 * @tc.desc: CalculateDialogOffset Function Test
 * @tc.type: FUNC
 */
HWTEST_F(CalendarPickerPatternTestNg, CalculateDialogOffset001, TestSize.Level1)
{
    CreateCalendarPicker();

    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish();
    EXPECT_EQ(element->GetTag(), V2::CALENDAR_PICKER_ETS_TAG);

    auto frameNode = AceType::DynamicCast<FrameNode>(element);
    ASSERT_NE(frameNode, nullptr);

    auto pickerPattern = frameNode->GetPattern<CalendarPickerPattern>();
    ASSERT_NE(pickerPattern, nullptr);

    auto host = pickerPattern->GetHost();
    ASSERT_NE(host, nullptr);
    auto layoutProperty = host->GetLayoutProperty<CalendarPickerLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    layoutProperty->UpdateLayoutDirection(TextDirection::LTR);
    layoutProperty->UpdateDialogAlignType(CalendarEdgeAlign::EDGE_ALIGN_CENTER);
    OffsetF f = pickerPattern->CalculateDialogOffset();

    layoutProperty->UpdateDialogAlignType(CalendarEdgeAlign::EDGE_ALIGN_END);
    f = pickerPattern->CalculateDialogOffset();

    auto pipelineContext = PipelineContext::GetCurrentContext();
    ASSERT_NE(pipelineContext, nullptr);
    auto windowManager = pipelineContext->GetWindowManager();

    auto WindownMode = []() -> WindowMode { return WindowMode::WINDOW_MODE_FLOATING; };
    windowManager->SetWindowGetModeCallBack(WindownMode);
    pipelineContext->SetWindowModal(WindowModal::CONTAINER_MODAL);

    f = pickerPattern->CalculateDialogOffset();

    EXPECT_FALSE(f.GetX() > 0);
}

/**
 * @tc.name: CalendarDialogPatternTest029
 * @tc.desc: HandleKeyEvent Function Test
 * @tc.type: FUNC
 */
HWTEST_F(CalendarPickerPatternTestNg, CalendarDialogPatternTest029, TestSize.Level1)
{
    CreateCalendarPicker();
    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish();
    auto dialogNode = CalendarDialogShow(AceType::DynamicCast<FrameNode>(element));
    auto contentWrapper = dialogNode->GetChildAtIndex(0);
    auto calendarDialogNode = AceType::DynamicCast<FrameNode>(contentWrapper->GetChildAtIndex(0));
    auto dialogPattern = calendarDialogNode->GetPattern<CalendarDialogPattern>();
    auto calendarNode = dialogPattern->GetCalendarFrameNode();
    const auto& calendarLayoutProperty = calendarNode->GetLayoutProperty();
    calendarLayoutProperty->UpdateLayoutDirection(TextDirection::RTL);
    KeyEvent keyEventOne(KeyCode::KEY_DPAD_LEFT, KeyAction::DOWN);
    auto result = dialogPattern->HandleKeyEvent(keyEventOne);
    EXPECT_TRUE(result);

    dialogPattern->focusAreaID_ = 0;
    dialogPattern->focusAreaChildID_ = 1;
    result = dialogPattern->HandleKeyEvent(keyEventOne);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: CalendarDialogPatternTest030
 * @tc.desc: HandleKeyEvent Function Test
 * @tc.type: FUNC
 */
HWTEST_F(CalendarPickerPatternTestNg, CalendarDialogPatternTest030, TestSize.Level1)
{
    CreateCalendarPicker();
    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish();
    auto dialogNode = CalendarDialogShow(AceType::DynamicCast<FrameNode>(element));
    auto contentWrapper = dialogNode->GetChildAtIndex(0);
    auto calendarDialogNode = AceType::DynamicCast<FrameNode>(contentWrapper->GetChildAtIndex(0));
    auto dialogPattern = calendarDialogNode->GetPattern<CalendarDialogPattern>();
    auto calendarNode = dialogPattern->GetCalendarFrameNode();
    const auto& calendarLayoutProperty = calendarNode->GetLayoutProperty();
    calendarLayoutProperty->UpdateLayoutDirection(TextDirection::RTL);
    KeyEvent keyEventOne(KeyCode::KEY_DPAD_LEFT, KeyAction::DOWN);
    dialogPattern->focusAreaID_ = 1;
    dialogPattern->focusAreaChildID_ = 1;
    auto result = dialogPattern->HandleKeyEvent(keyEventOne);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: CalendarDialogPatternTest031
 * @tc.desc: HandleKeyEvent Function Test
 * @tc.type: FUNC
 */
HWTEST_F(CalendarPickerPatternTestNg, CalendarDialogPatternTest31, TestSize.Level1)
{
    CreateCalendarPicker();
    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish();
    auto dialogNode = CalendarDialogShow(AceType::DynamicCast<FrameNode>(element));
    auto contentWrapper = dialogNode->GetChildAtIndex(0);
    ASSERT_NE(contentWrapper, nullptr);
    auto calendarDialogNode = AceType::DynamicCast<FrameNode>(contentWrapper->GetChildAtIndex(0));
    auto dialogPattern = calendarDialogNode->GetPattern<CalendarDialogPattern>();
    auto calendarNode = dialogPattern->GetCalendarFrameNode();
    const auto& calendarLayoutProperty = calendarNode->GetLayoutProperty();
    calendarLayoutProperty->UpdateLayoutDirection(TextDirection::RTL);
    KeyEvent keyEventOne(KeyCode::KEY_DPAD_RIGHT, KeyAction::DOWN);
    auto result = dialogPattern->HandleKeyEvent(keyEventOne);
    EXPECT_TRUE(result);

    dialogPattern->focusAreaID_ = 0;
    dialogPattern->focusAreaChildID_ = 1;
    result = dialogPattern->HandleKeyEvent(keyEventOne);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: CalendarDialogPatternTest032
 * @tc.desc: HandleKeyEvent Function Test
 * @tc.type: FUNC
 */
HWTEST_F(CalendarPickerPatternTestNg, CalendarDialogPatternTest032, TestSize.Level1)
{
    CreateCalendarPicker();
    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish();
    auto dialogNode = CalendarDialogShow(AceType::DynamicCast<FrameNode>(element));
    auto contentWrapper = dialogNode->GetChildAtIndex(0);
    ASSERT_NE(contentWrapper, nullptr);
    auto calendarDialogNode = AceType::DynamicCast<FrameNode>(contentWrapper->GetChildAtIndex(0));
    auto dialogPattern = calendarDialogNode->GetPattern<CalendarDialogPattern>();
    auto calendarNode = dialogPattern->GetCalendarFrameNode();
    const auto& calendarLayoutProperty = calendarNode->GetLayoutProperty();
    calendarLayoutProperty->UpdateLayoutDirection(TextDirection::RTL);
    KeyEvent keyEventOne(KeyCode::KEY_DPAD_RIGHT, KeyAction::DOWN);
    dialogPattern->focusAreaID_ = 1;
    dialogPattern->focusAreaChildID_ = 1;
    auto result = dialogPattern->HandleKeyEvent(keyEventOne);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: CalendarDialogPatternTest034
 * @tc.desc: OnEnterKeyEvent Function Test
 * @tc.type: FUNC
 */
HWTEST_F(CalendarPickerPatternTestNg, CalendarDialogPatternTest033, TestSize.Level1)
{
    CreateCalendarPicker();
    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish();
    auto dialogNode = CalendarDialogShow(AceType::DynamicCast<FrameNode>(element));
    auto contentWrapper = dialogNode->GetChildAtIndex(0);
    ASSERT_NE(contentWrapper, nullptr);
    auto calendarDialogNode = AceType::DynamicCast<FrameNode>(contentWrapper->GetChildAtIndex(0));
    auto dialogPattern = calendarDialogNode->GetPattern<CalendarDialogPattern>();
    KeyEvent keyEventOne(KeyCode::KEY_NUMPAD_ENTER, KeyAction::DOWN);
    dialogPattern->OnEnterKeyEvent(keyEventOne);
    EXPECT_EQ(dialogNode->GetChildAtIndex(2), nullptr);
}

/**
 * @tc.name: CalendarDialogPatternTest034
 * @tc.desc: OnLanguageConfigurationUpdate Function Test
 * @tc.type: FUNC
 */
HWTEST_F(CalendarPickerPatternTestNg, CalendarDialogPatternTest034, TestSize.Level1)
{
    CreateCalendarPicker();
    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish();
    auto dialogNode = CalendarDialogShow(AceType::DynamicCast<FrameNode>(element));
    auto contentWrapper = dialogNode->GetChildAtIndex(0);
    ASSERT_NE(contentWrapper, nullptr);
    auto calendarDialogNode = AceType::DynamicCast<FrameNode>(contentWrapper->GetChildAtIndex(0));
    auto dialogPattern = calendarDialogNode->GetPattern<CalendarDialogPattern>();
    dialogPattern->OnLanguageConfigurationUpdate();
    EXPECT_NE(AceApplicationInfo::GetInstance().GetLanguage(), "zh");
    AceApplicationInfo::GetInstance().language_ = "zh";
    dialogPattern->OnLanguageConfigurationUpdate();
    auto pipelineContext = dialogPattern->GetContext();
    auto fontSizeScale = pipelineContext->GetFontScale();
    RefPtr<CalendarTheme> theme = pipelineContext->GetTheme<CalendarTheme>();
    EXPECT_FALSE(fontSizeScale < theme->GetCalendarPickerLargeScale());
}

/**
 * @tc.name: CalendarDialogPatternTest035
 * @tc.desc: HandleSurfaceChanged Function Test
 * @tc.type: FUNC
 */
HWTEST_F(CalendarPickerPatternTestNg, CalendarDialogPatternTest035, TestSize.Level1)
{
    CreateCalendarPicker();
    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish();
    auto dialogNode = CalendarDialogShow(AceType::DynamicCast<FrameNode>(element));
    auto contentWrapper = dialogNode->GetChildAtIndex(0);
    ASSERT_NE(contentWrapper, nullptr);
    auto calendarDialogNode = AceType::DynamicCast<FrameNode>(contentWrapper->GetChildAtIndex(0));
    auto dialogPattern = calendarDialogNode->GetPattern<CalendarDialogPattern>();
    dialogPattern->HandleSurfaceChanged(1, 1, 2, 2);
    EXPECT_EQ(CalendarDialogView::previousOrientation_, SystemProperties::GetDeviceOrientation());
}
} // namespace OHOS::Ace::NG

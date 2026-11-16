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

#include <cstddef>
#include <optional>
#include <string>
#include <utility>

#include "gtest/gtest.h"

#define private public
#define protected public

#include "test/mock/frameworks/base/thread/mock_task_executor.h"
#include "test/mock/frameworks/core/common/mock_theme_manager.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"
#include "test/mock/frameworks/core/components_ng/render/mock_render_context.h"

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
#include "core/components_ng/pattern/calendar_picker/calendar_picker_i18n.h"
#include "core/components_ng/pattern/calendar_picker/calendar_picker_pattern.h"
#include "core/components_ng/pattern/dialog/dialog_view.h"
#include "core/components_ng/pattern/divider/divider_pattern.h"
#include "core/components_ng/pattern/flex/flex_layout_pattern.h"
#include "core/components_ng/pattern/flex/flex_layout_property.h"
#include "core/components_ng/pattern/image/image_pattern.h"
#include "core/components_ng/pattern/date_picker/date_time_animation_controller.h"
#include "core/components_ng/pattern/date_picker/datepicker_pattern.h"
#include "core/components_ng/pattern/date_picker/datepicker_row_layout_property.h"
#include "core/components_ng/pattern/stack/stack_pattern.h"
#include "core/components_ng/pattern/swiper/swiper_pattern.h"
#include "core/components_ng/pattern/text/text_layout_property.h"
#include "core/components_ng/pattern/text/text_pattern.h"
#include "core/components_ng/pattern/text_field/text_field_pattern.h"
#include "core/components_ng/property/measure_property.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/pipeline/base/element_register.h"
#include "core/pipeline_ng/ui_task_scheduler.h"
#include "core/components/button/button_theme.h"

#undef private
#undef protected

using namespace testing;
using namespace testing::ext;
namespace OHOS::Ace::NG {
namespace {
const InspectorFilter filter;
constexpr Dimension TEST_SETTING_RADIUS = Dimension(10.0, DimensionUnit::VP);
} // namespace
class CalendarPickerTestUpdate : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void MockPipelineContextGetTheme();

protected:
    static void CreateCalendarPicker();
    static RefPtr<FrameNode> CalendarDialogShow(RefPtr<FrameNode> entryNode);
};

void CalendarPickerTestUpdate::SetUpTestCase()
{
    MockPipelineContext::SetUp();
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    auto getThemeByType = [](ThemeType type) -> RefPtr<Theme> {
        if (type == CalendarTheme::TypeId()) {
            return AceType::MakeRefPtr<CalendarTheme>();
        } else if (type == IconTheme::TypeId()) {
            return AceType::MakeRefPtr<IconTheme>();
        } else if (type == DialogTheme::TypeId()) {
            return AceType::MakeRefPtr<DialogTheme>();
        } else {
            return AceType::MakeRefPtr<PickerTheme>();
        }
    };
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Invoke(getThemeByType));
    EXPECT_CALL(*themeManager, GetTheme(_, _))
        .WillRepeatedly([getThemeByType](ThemeType type, int32_t themeScopeId) -> RefPtr<Theme> {
            (void)themeScopeId;
            return getThemeByType(type);
        });
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
}

void CalendarPickerTestUpdate::TearDownTestCase()
{
    MockPipelineContext::GetCurrent()->SetThemeManager(nullptr);
    MockPipelineContext::TearDown();
}

void CalendarPickerTestUpdate::CreateCalendarPicker()
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

RefPtr<FrameNode> CalendarPickerTestUpdate::CalendarDialogShow(RefPtr<FrameNode> entryNode)
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
 * @tc.name: CalendarPickerUpdateButtonStyles002
 * @tc.desc: Test UpdateButtonStyle.
 * @tc.type: FUNC
 */
HWTEST_F(CalendarPickerTestUpdate, CalendarPickerUpdateButtonStyles002, TestSize.Level1)
{
    std::vector<ButtonInfo> buttonInfos;
    ButtonInfo info1;
    ButtonInfo info2;
    info1.fontWeight = FontWeight::W100;
    info2.fontWeight = FontWeight::W200;
    
    buttonInfos.push_back(info1);
    buttonInfos.push_back(info2);
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    CreateCalendarPicker();
    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish();
    EXPECT_EQ(element->GetTag(), V2::CALENDAR_PICKER_ETS_TAG);
    auto frameNode = AceType::DynamicCast<FrameNode>(element);
    ASSERT_NE(frameNode, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<CalendarPickerLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    auto buttonNode = FrameNode::GetOrCreateFrameNode(
        V2::BUTTON_ETS_TAG, stack->ClaimNodeId(), []() { return AceType::MakeRefPtr<ButtonPattern>(); });
    ASSERT_NE(buttonNode, nullptr);
    auto buttonLayoutProperty = buttonNode->GetLayoutProperty<ButtonLayoutProperty>();
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto renderContext = buttonNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    ASSERT_NE(themeManager, nullptr);
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    auto buttonTheme = AceType::MakeRefPtr<ButtonTheme>();
    ASSERT_NE(buttonTheme, nullptr);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(buttonTheme));
    EXPECT_CALL(*themeManager, GetTheme(_, _))
        .WillRepeatedly([buttonTheme](ThemeType type, int32_t themeScopeId) -> RefPtr<Theme> {
            (void)type;
            (void)themeScopeId;
            return buttonTheme;
        });
    
    size_t sizet = 0;
    CalendarDialogView::UpdateButtonStyles(buttonInfos, sizet, buttonLayoutProperty, renderContext);
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto testval = buttonLayoutProperty->GetFontWeightValue();
    EXPECT_EQ(testval, FontWeight::W100);

    sizet++;
    CalendarDialogView::UpdateButtonStyles(buttonInfos, sizet, buttonLayoutProperty, renderContext);
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto testval2 = buttonLayoutProperty->GetFontWeightValue();
    EXPECT_EQ(testval2, FontWeight::W200);
}
/**
 * @tc.name: CalendarPickerUpdateButtonStyles003
 * @tc.desc: Test UpdateButtonStyle.
 * @tc.type: FUNC
 */
HWTEST_F(CalendarPickerTestUpdate, CalendarPickerUpdateButtonStyles003, TestSize.Level1)
{
    std::vector<ButtonInfo> buttonInfos;
    ButtonInfo info1;
    info1.fontSize = Dimension(100);
    buttonInfos.push_back(info1);

    size_t sizet = 0;

    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);

    CreateCalendarPicker();
    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish();
    EXPECT_EQ(element->GetTag(), V2::CALENDAR_PICKER_ETS_TAG);

    auto frameNode = AceType::DynamicCast<FrameNode>(element);
    ASSERT_NE(frameNode, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<CalendarPickerLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    auto buttonNode = FrameNode::GetOrCreateFrameNode(
        V2::BUTTON_ETS_TAG, stack->ClaimNodeId(), []() { return AceType::MakeRefPtr<ButtonPattern>(); });
    ASSERT_NE(buttonNode, nullptr);
    auto buttonLayoutProperty = buttonNode->GetLayoutProperty<ButtonLayoutProperty>();
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto renderContext = buttonNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    ASSERT_NE(themeManager, nullptr);
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    auto buttonTheme = AceType::MakeRefPtr<ButtonTheme>();
    ASSERT_NE(buttonTheme, nullptr);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(buttonTheme));
    EXPECT_CALL(*themeManager, GetTheme(_, _))
        .WillRepeatedly([buttonTheme](ThemeType type, int32_t themeScopeId) -> RefPtr<Theme> {
            (void)type;
            (void)themeScopeId;
            return buttonTheme;
        });
    CalendarDialogView::UpdateButtonStyles(buttonInfos, sizet, buttonLayoutProperty, renderContext);
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto testval = buttonLayoutProperty->GetFontSizeValue();
    EXPECT_EQ(testval, Dimension(100));
}
/**
 * @tc.name: CalendarPickerUpdateButtonStyles004
 * @tc.desc: Test UpdateButtonStyle.
 * @tc.type: FUNC
 */
HWTEST_F(CalendarPickerTestUpdate, CalendarPickerUpdateButtonStyles004, TestSize.Level1)
{
    std::vector<ButtonInfo> buttonInfos;
    ButtonInfo info1;
    info1.fontColor = std::make_optional<Color>(Color::BLACK);
    buttonInfos.push_back(info1);

    size_t sizet = 0;

    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);

    CreateCalendarPicker();
    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish();
    EXPECT_EQ(element->GetTag(), V2::CALENDAR_PICKER_ETS_TAG);

    auto frameNode = AceType::DynamicCast<FrameNode>(element);
    ASSERT_NE(frameNode, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<CalendarPickerLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    auto buttonNode = FrameNode::GetOrCreateFrameNode(
        V2::BUTTON_ETS_TAG, stack->ClaimNodeId(), []() { return AceType::MakeRefPtr<ButtonPattern>(); });
    ASSERT_NE(buttonNode, nullptr);
    auto buttonLayoutProperty = buttonNode->GetLayoutProperty<ButtonLayoutProperty>();
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto renderContext = buttonNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    ASSERT_NE(themeManager, nullptr);
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    auto buttonTheme = AceType::MakeRefPtr<ButtonTheme>();
    ASSERT_NE(buttonTheme, nullptr);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(buttonTheme));
    EXPECT_CALL(*themeManager, GetTheme(_, _))
        .WillRepeatedly([buttonTheme](ThemeType type, int32_t themeScopeId) -> RefPtr<Theme> {
            (void)type;
            (void)themeScopeId;
            return buttonTheme;
        });
    CalendarDialogView::UpdateButtonStyles(buttonInfos, sizet, buttonLayoutProperty, renderContext);
    ASSERT_NE(buttonLayoutProperty, nullptr);

    auto testval = buttonLayoutProperty->GetFontColorValue();
    EXPECT_EQ(testval, Color::BLACK);
}
/**
 * @tc.name: CalendarPickerUpdateButtonStyles005
 * @tc.desc: Test UpdateButtonStyle.
 * @tc.type: FUNC
 */
HWTEST_F(CalendarPickerTestUpdate, CalendarPickerUpdateButtonStyles005, TestSize.Level1)
{
    std::vector<ButtonInfo> buttonInfos;
    ButtonInfo info1;
    info1.fontStyle = Ace::FontStyle::ITALIC;
    buttonInfos.push_back(info1);

    size_t sizet = 0;

    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);

    CreateCalendarPicker();
    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish();
    EXPECT_EQ(element->GetTag(), V2::CALENDAR_PICKER_ETS_TAG);

    auto frameNode = AceType::DynamicCast<FrameNode>(element);
    ASSERT_NE(frameNode, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<CalendarPickerLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    auto buttonNode = FrameNode::GetOrCreateFrameNode(
        V2::BUTTON_ETS_TAG, stack->ClaimNodeId(), []() { return AceType::MakeRefPtr<ButtonPattern>(); });
    ASSERT_NE(buttonNode, nullptr);
    auto buttonLayoutProperty = buttonNode->GetLayoutProperty<ButtonLayoutProperty>();
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto renderContext = buttonNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    ASSERT_NE(themeManager, nullptr);
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    auto buttonTheme = AceType::MakeRefPtr<ButtonTheme>();
    ASSERT_NE(buttonTheme, nullptr);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(buttonTheme));
    EXPECT_CALL(*themeManager, GetTheme(_, _))
        .WillRepeatedly([buttonTheme](ThemeType type, int32_t themeScopeId) -> RefPtr<Theme> {
            (void)type;
            (void)themeScopeId;
            return buttonTheme;
        });
    CalendarDialogView::UpdateButtonStyles(buttonInfos, sizet, buttonLayoutProperty, renderContext);
    ASSERT_NE(buttonLayoutProperty, nullptr);
    
    auto testval = buttonLayoutProperty->GetFontStyleValue();
    EXPECT_EQ(testval, Ace::FontStyle::ITALIC);
}

/**
 * @tc.name: CalendarPickerUpdateButtonStyles006
 * @tc.desc: Test UpdateButtonStyle.
 * @tc.type: FUNC
 */
HWTEST_F(CalendarPickerTestUpdate, CalendarPickerUpdateButtonStyles006, TestSize.Level1)
{
    std::vector<ButtonInfo> buttonInfos;
    ButtonInfo info1;
    info1.type = std::make_optional<ButtonType>(ButtonType::CIRCLE);
    buttonInfos.push_back(info1);

    size_t sizet = 0;

    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);

    CreateCalendarPicker();
    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish();
    EXPECT_EQ(element->GetTag(), V2::CALENDAR_PICKER_ETS_TAG);

    auto frameNode = AceType::DynamicCast<FrameNode>(element);
    ASSERT_NE(frameNode, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<CalendarPickerLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    auto buttonNode = FrameNode::GetOrCreateFrameNode(
        V2::BUTTON_ETS_TAG, stack->ClaimNodeId(), []() { return AceType::MakeRefPtr<ButtonPattern>(); });
    ASSERT_NE(buttonNode, nullptr);
    auto buttonLayoutProperty = buttonNode->GetLayoutProperty<ButtonLayoutProperty>();
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto renderContext = buttonNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    ASSERT_NE(themeManager, nullptr);
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    auto buttonTheme = AceType::MakeRefPtr<ButtonTheme>();
    ASSERT_NE(buttonTheme, nullptr);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(buttonTheme));
    EXPECT_CALL(*themeManager, GetTheme(_, _))
        .WillRepeatedly([buttonTheme](ThemeType type, int32_t themeScopeId) -> RefPtr<Theme> {
            (void)type;
            (void)themeScopeId;
            return buttonTheme;
        });
    CalendarDialogView::UpdateButtonStyles(buttonInfos, sizet, buttonLayoutProperty, renderContext);
    ASSERT_NE(buttonLayoutProperty, nullptr);
    
    auto testval = buttonLayoutProperty->GetTypeValue();
    EXPECT_EQ(testval, ButtonType::CIRCLE);
}
/**
 * @tc.name: CalendarPickerUpdateButtonStyles007
 * @tc.desc: Test UpdateButtonStyle.
 * @tc.type: FUNC
 */
HWTEST_F(CalendarPickerTestUpdate, CalendarPickerUpdateButtonStyles007, TestSize.Level1)
{
    std::vector<ButtonInfo> buttonInfos;
    ButtonInfo info1;
    info1.buttonStyle = std::make_optional<ButtonStyleMode>(ButtonStyleMode::EMPHASIZE);
    buttonInfos.push_back(info1);

    size_t sizet = 0;

    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);

    CreateCalendarPicker();
    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish();
    EXPECT_EQ(element->GetTag(), V2::CALENDAR_PICKER_ETS_TAG);

    auto frameNode = AceType::DynamicCast<FrameNode>(element);
    ASSERT_NE(frameNode, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<CalendarPickerLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    auto buttonNode = FrameNode::GetOrCreateFrameNode(
        V2::BUTTON_ETS_TAG, stack->ClaimNodeId(), []() { return AceType::MakeRefPtr<ButtonPattern>(); });
    ASSERT_NE(buttonNode, nullptr);
    auto buttonLayoutProperty = buttonNode->GetLayoutProperty<ButtonLayoutProperty>();
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto renderContext = buttonNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    ASSERT_NE(themeManager, nullptr);
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    auto buttonTheme = AceType::MakeRefPtr<ButtonTheme>();
    ASSERT_NE(buttonTheme, nullptr);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(buttonTheme));
    EXPECT_CALL(*themeManager, GetTheme(_, _))
        .WillRepeatedly([buttonTheme](ThemeType type, int32_t themeScopeId) -> RefPtr<Theme> {
            (void)type;
            (void)themeScopeId;
            return buttonTheme;
        });
    CalendarDialogView::UpdateButtonStyles(buttonInfos, sizet, buttonLayoutProperty, renderContext);
    ASSERT_NE(buttonLayoutProperty, nullptr);
    
    auto testval = buttonLayoutProperty->GetButtonStyleValue();
    EXPECT_EQ(testval, ButtonStyleMode::EMPHASIZE);
}
/**
 * @tc.name: CalendarPickerUpdateButtonStyles008
 * @tc.desc: Test UpdateButtonStyle.
 * @tc.type: FUNC
 */
HWTEST_F(CalendarPickerTestUpdate, CalendarPickerUpdateButtonStyles008, TestSize.Level1)
{
    std::vector<ButtonInfo> buttonInfos;
    ButtonInfo info1;
    info1.role = std::make_optional<ButtonRole>(ButtonRole::ERROR);
    buttonInfos.push_back(info1);

    size_t sizet = 0;

    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);

    CreateCalendarPicker();
    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish();
    EXPECT_EQ(element->GetTag(), V2::CALENDAR_PICKER_ETS_TAG);

    auto frameNode = AceType::DynamicCast<FrameNode>(element);
    ASSERT_NE(frameNode, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<CalendarPickerLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    auto buttonNode = FrameNode::GetOrCreateFrameNode(
        V2::BUTTON_ETS_TAG, stack->ClaimNodeId(), []() { return AceType::MakeRefPtr<ButtonPattern>(); });
    ASSERT_NE(buttonNode, nullptr);
    auto buttonLayoutProperty = buttonNode->GetLayoutProperty<ButtonLayoutProperty>();
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto renderContext = buttonNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    ASSERT_NE(themeManager, nullptr);
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    auto buttonTheme = AceType::MakeRefPtr<ButtonTheme>();
    ASSERT_NE(buttonTheme, nullptr);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(buttonTheme));
    EXPECT_CALL(*themeManager, GetTheme(_, _))
        .WillRepeatedly([buttonTheme](ThemeType type, int32_t themeScopeId) -> RefPtr<Theme> {
            (void)type;
            (void)themeScopeId;
            return buttonTheme;
        });
    CalendarDialogView::UpdateButtonStyles(buttonInfos, sizet, buttonLayoutProperty, renderContext);
    ASSERT_NE(buttonLayoutProperty, nullptr);
    
    auto testval = buttonLayoutProperty->GetButtonRoleValue();
    EXPECT_EQ(testval, ButtonRole::ERROR);
}
/**
 * @tc.name: CalendarPickerUpdateButtonStyles009
 * @tc.desc: Test UpdateButtonStyle.
 * @tc.type: FUNC
 */
HWTEST_F(CalendarPickerTestUpdate, CalendarPickerUpdateButtonStyles009, TestSize.Level1)
{
    std::vector<ButtonInfo> buttonInfos;
    ButtonInfo info1;
    ButtonInfo info2;
    info1.role = std::make_optional<ButtonRole>(ButtonRole::ERROR);
    info2.role = std::make_optional<ButtonRole>(ButtonRole::NORMAL);
    buttonInfos.push_back(info1);
    buttonInfos.push_back(info2);
    size_t sizet = 0;

    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);

    CreateCalendarPicker();
    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish();
    EXPECT_EQ(element->GetTag(), V2::CALENDAR_PICKER_ETS_TAG);

    auto frameNode = AceType::DynamicCast<FrameNode>(element);
    ASSERT_NE(frameNode, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<CalendarPickerLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    auto buttonNode = FrameNode::GetOrCreateFrameNode(
        V2::BUTTON_ETS_TAG, stack->ClaimNodeId(), []() { return AceType::MakeRefPtr<ButtonPattern>(); });
    ASSERT_NE(buttonNode, nullptr);
    auto buttonLayoutProperty = buttonNode->GetLayoutProperty<ButtonLayoutProperty>();
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto renderContext = buttonNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    ASSERT_NE(themeManager, nullptr);
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    auto buttonTheme = AceType::MakeRefPtr<ButtonTheme>();
    ASSERT_NE(buttonTheme, nullptr);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(buttonTheme));
    EXPECT_CALL(*themeManager, GetTheme(_, _))
        .WillRepeatedly([buttonTheme](ThemeType type, int32_t themeScopeId) -> RefPtr<Theme> {
            (void)type;
            (void)themeScopeId;
            return buttonTheme;
        });
    CalendarDialogView::UpdateButtonStyles(buttonInfos, sizet, buttonLayoutProperty, renderContext);
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto testval = buttonLayoutProperty->GetButtonRoleValue();
    EXPECT_EQ(testval, ButtonRole::ERROR);

    sizet++;
    CalendarDialogView::UpdateButtonStyles(buttonInfos, sizet, buttonLayoutProperty, renderContext);
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto testval2 = buttonLayoutProperty->GetButtonRoleValue();
    EXPECT_EQ(testval2, ButtonRole::NORMAL);
}
/**
 * @tc.name: CalendarPickerUpdateButtonStyles010
 * @tc.desc: Test UpdateButtonStyle.
 * @tc.type: FUNC
 */
HWTEST_F(CalendarPickerTestUpdate, CalendarPickerUpdateButtonStyles010, TestSize.Level1)
{
    std::vector<ButtonInfo> buttonInfos;
    ButtonInfo info1;
    ButtonInfo info2;
    info1.buttonStyle = std::make_optional<ButtonStyleMode>(ButtonStyleMode::EMPHASIZE);
    info2.buttonStyle = ButtonStyleMode::TEXT;
    buttonInfos.push_back(info1);
    buttonInfos.push_back(info2);
    size_t sizet = 0;

    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);

    CreateCalendarPicker();
    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish();
    EXPECT_EQ(element->GetTag(), V2::CALENDAR_PICKER_ETS_TAG);

    auto frameNode = AceType::DynamicCast<FrameNode>(element);
    ASSERT_NE(frameNode, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<CalendarPickerLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    auto buttonNode = FrameNode::GetOrCreateFrameNode(
        V2::BUTTON_ETS_TAG, stack->ClaimNodeId(), []() { return AceType::MakeRefPtr<ButtonPattern>(); });
    ASSERT_NE(buttonNode, nullptr);
    auto buttonLayoutProperty = buttonNode->GetLayoutProperty<ButtonLayoutProperty>();
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto renderContext = buttonNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    ASSERT_NE(themeManager, nullptr);
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    auto buttonTheme = AceType::MakeRefPtr<ButtonTheme>();
    ASSERT_NE(buttonTheme, nullptr);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(buttonTheme));
    EXPECT_CALL(*themeManager, GetTheme(_, _))
        .WillRepeatedly([buttonTheme](ThemeType type, int32_t themeScopeId) -> RefPtr<Theme> {
            (void)type;
            (void)themeScopeId;
            return buttonTheme;
        });
    CalendarDialogView::UpdateButtonStyles(buttonInfos, sizet, buttonLayoutProperty, renderContext);
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto testval = buttonLayoutProperty->GetButtonStyleValue();
    EXPECT_EQ(testval, ButtonStyleMode::EMPHASIZE);

    sizet++;
    CalendarDialogView::UpdateButtonStyles(buttonInfos, sizet, buttonLayoutProperty, renderContext);
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto testval2 = buttonLayoutProperty->GetButtonStyleValue();
    EXPECT_EQ(testval2, ButtonStyleMode::TEXT);
}
/**
 * @tc.name: CalendarPickerUpdateButtonStyles011
 * @tc.desc: Test UpdateButtonStyle.
 * @tc.type: FUNC
 */
HWTEST_F(CalendarPickerTestUpdate, CalendarPickerUpdateButtonStyles011, TestSize.Level1)
{
    std::vector<ButtonInfo> buttonInfos;
    ButtonInfo info1;
    ButtonInfo info2;
    info1.type = std::make_optional<ButtonType>(ButtonType::CIRCLE);
    info2.type = std::make_optional<ButtonType>(ButtonType::CAPSULE);
    buttonInfos.push_back(info1);
    buttonInfos.push_back(info2);
    size_t sizet = 0;

    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);

    CreateCalendarPicker();
    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish();
    EXPECT_EQ(element->GetTag(), V2::CALENDAR_PICKER_ETS_TAG);

    auto frameNode = AceType::DynamicCast<FrameNode>(element);
    ASSERT_NE(frameNode, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<CalendarPickerLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    auto buttonNode = FrameNode::GetOrCreateFrameNode(
        V2::BUTTON_ETS_TAG, stack->ClaimNodeId(), []() { return AceType::MakeRefPtr<ButtonPattern>(); });
    ASSERT_NE(buttonNode, nullptr);
    auto buttonLayoutProperty = buttonNode->GetLayoutProperty<ButtonLayoutProperty>();
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto renderContext = buttonNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    ASSERT_NE(themeManager, nullptr);
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    auto buttonTheme = AceType::MakeRefPtr<ButtonTheme>();
    ASSERT_NE(buttonTheme, nullptr);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(buttonTheme));
    EXPECT_CALL(*themeManager, GetTheme(_, _))
        .WillRepeatedly([buttonTheme](ThemeType type, int32_t themeScopeId) -> RefPtr<Theme> {
            (void)type;
            (void)themeScopeId;
            return buttonTheme;
        });
    CalendarDialogView::UpdateButtonStyles(buttonInfos, sizet, buttonLayoutProperty, renderContext);
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto testval = buttonLayoutProperty->GetTypeValue();
    EXPECT_EQ(testval, ButtonType::CIRCLE);

    sizet++;
    CalendarDialogView::UpdateButtonStyles(buttonInfos, sizet, buttonLayoutProperty, renderContext);
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto testval2 = buttonLayoutProperty->GetTypeValue();
    EXPECT_EQ(testval2, ButtonType::CAPSULE);
}
/**
 * @tc.name: CalendarPickerUpdateButtonStyles012
 * @tc.desc: Test UpdateButtonStyle.
 * @tc.type: FUNC
 */
HWTEST_F(CalendarPickerTestUpdate, CalendarPickerUpdateButtonStyles012, TestSize.Level1)
{
    std::vector<ButtonInfo> buttonInfos;
    ButtonInfo info1;
    ButtonInfo info2;
    info1.fontStyle = Ace::FontStyle::ITALIC;
    info2.fontStyle = Ace::FontStyle::NONE;
    buttonInfos.push_back(info1);
    buttonInfos.push_back(info2);
    size_t sizet = 0;

    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);

    CreateCalendarPicker();
    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish();
    EXPECT_EQ(element->GetTag(), V2::CALENDAR_PICKER_ETS_TAG);

    auto frameNode = AceType::DynamicCast<FrameNode>(element);
    ASSERT_NE(frameNode, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<CalendarPickerLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    auto buttonNode = FrameNode::GetOrCreateFrameNode(
        V2::BUTTON_ETS_TAG, stack->ClaimNodeId(), []() { return AceType::MakeRefPtr<ButtonPattern>(); });
    ASSERT_NE(buttonNode, nullptr);
    auto buttonLayoutProperty = buttonNode->GetLayoutProperty<ButtonLayoutProperty>();
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto renderContext = buttonNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    ASSERT_NE(themeManager, nullptr);
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    auto buttonTheme = AceType::MakeRefPtr<ButtonTheme>();
    ASSERT_NE(buttonTheme, nullptr);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(buttonTheme));
    EXPECT_CALL(*themeManager, GetTheme(_, _))
        .WillRepeatedly([buttonTheme](ThemeType type, int32_t themeScopeId) -> RefPtr<Theme> {
            (void)type;
            (void)themeScopeId;
            return buttonTheme;
        });
    CalendarDialogView::UpdateButtonStyles(buttonInfos, sizet, buttonLayoutProperty, renderContext);
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto testval = buttonLayoutProperty->GetFontStyleValue();
    EXPECT_EQ(testval, Ace::FontStyle::ITALIC);

    sizet++;
    CalendarDialogView::UpdateButtonStyles(buttonInfos, sizet, buttonLayoutProperty, renderContext);
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto testval2 = buttonLayoutProperty->GetFontStyleValue();
    EXPECT_EQ(testval2, Ace::FontStyle::NONE);
}
/**
 * @tc.name: CalendarPickerUpdateButtonStyles013
 * @tc.desc: Test UpdateButtonStyle.
 * @tc.type: FUNC
 */
HWTEST_F(CalendarPickerTestUpdate, CalendarPickerUpdateButtonStyles013, TestSize.Level1)
{
    std::vector<ButtonInfo> buttonInfos;
    ButtonInfo info1;
    info1.fontStyle = Ace::FontStyle::ITALIC;
    info1.fontWeight = FontWeight::W500;
    info1.fontColor = Color::BLACK;
    buttonInfos.push_back(info1);
    size_t sizet = 0;

    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);

    CreateCalendarPicker();
    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish();
    EXPECT_EQ(element->GetTag(), V2::CALENDAR_PICKER_ETS_TAG);

    auto frameNode = AceType::DynamicCast<FrameNode>(element);
    ASSERT_NE(frameNode, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<CalendarPickerLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    auto buttonNode = FrameNode::GetOrCreateFrameNode(
        V2::BUTTON_ETS_TAG, stack->ClaimNodeId(), []() { return AceType::MakeRefPtr<ButtonPattern>(); });
    ASSERT_NE(buttonNode, nullptr);
    auto buttonLayoutProperty = buttonNode->GetLayoutProperty<ButtonLayoutProperty>();
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto renderContext = buttonNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    ASSERT_NE(themeManager, nullptr);
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    auto buttonTheme = AceType::MakeRefPtr<ButtonTheme>();
    ASSERT_NE(buttonTheme, nullptr);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(buttonTheme));
    EXPECT_CALL(*themeManager, GetTheme(_, _))
        .WillRepeatedly([buttonTheme](ThemeType type, int32_t themeScopeId) -> RefPtr<Theme> {
            (void)type;
            (void)themeScopeId;
            return buttonTheme;
        });
    CalendarDialogView::UpdateButtonStyles(buttonInfos, sizet, buttonLayoutProperty, renderContext);
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto testval = buttonLayoutProperty->GetFontStyleValue();
    EXPECT_EQ(testval, Ace::FontStyle::ITALIC);

    auto testval2 = buttonLayoutProperty->GetFontWeightValue();
    EXPECT_EQ(testval2, FontWeight::W500);

    auto testval3 = buttonLayoutProperty->GetFontColorValue();
    EXPECT_EQ(testval3, Color::BLACK);
}
// /**
//  * @tc.name: CalendarPickerUpdateButtonStyles014
//  * @tc.desc: Test UpdateButtonStyle.
//  * @tc.type: FUNC
//  */
HWTEST_F(CalendarPickerTestUpdate, CalendarPickerUpdateButtonStyles014, TestSize.Level1)
{
    std::vector<ButtonInfo> buttonInfos;
    ButtonInfo info1;
    info1.type = std::make_optional<ButtonType>(ButtonType::CIRCLE);
    info1.buttonStyle = std::make_optional<ButtonStyleMode>(ButtonStyleMode::EMPHASIZE);
    info1.role = std::make_optional<ButtonRole>(ButtonRole::NORMAL);
    buttonInfos.push_back(info1);
    size_t sizet = 0;

    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);

    CreateCalendarPicker();
    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish();
    EXPECT_EQ(element->GetTag(), V2::CALENDAR_PICKER_ETS_TAG);

    auto frameNode = AceType::DynamicCast<FrameNode>(element);
    ASSERT_NE(frameNode, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<CalendarPickerLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    auto buttonNode = FrameNode::GetOrCreateFrameNode(
        V2::BUTTON_ETS_TAG, stack->ClaimNodeId(), []() { return AceType::MakeRefPtr<ButtonPattern>(); });
    ASSERT_NE(buttonNode, nullptr);
    auto buttonLayoutProperty = buttonNode->GetLayoutProperty<ButtonLayoutProperty>();
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto renderContext = buttonNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    ASSERT_NE(themeManager, nullptr);
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    auto buttonTheme = AceType::MakeRefPtr<ButtonTheme>();
    ASSERT_NE(buttonTheme, nullptr);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(buttonTheme));
    EXPECT_CALL(*themeManager, GetTheme(_, _))
        .WillRepeatedly([buttonTheme](ThemeType type, int32_t themeScopeId) -> RefPtr<Theme> {
            (void)type;
            (void)themeScopeId;
            return buttonTheme;
        });
    CalendarDialogView::UpdateButtonStyles(buttonInfos, sizet, buttonLayoutProperty, renderContext);
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto testval = buttonLayoutProperty->GetTypeValue();
    EXPECT_EQ(testval, ButtonType::CIRCLE);

    auto testval2 = buttonLayoutProperty->GetButtonStyleValue();
    EXPECT_EQ(testval2, ButtonStyleMode::EMPHASIZE);

    auto testval3 = buttonLayoutProperty->GetButtonRoleValue();
    EXPECT_EQ(testval3, ButtonRole::NORMAL);
}
// /**
//  * @tc.name: CalendarPickerUpdateButtonStyles015
//  * @tc.desc: Test UpdateButtonStyle.
//  * @tc.type: FUNC
//  */
HWTEST_F(CalendarPickerTestUpdate, CalendarPickerUpdateButtonStyles015, TestSize.Level1)
{
    std::vector<ButtonInfo> buttonInfos;
    ButtonInfo info1;
    info1.fontSize = Dimension(0);
    buttonInfos.push_back(info1);

    size_t sizet = 0;

    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);

    CreateCalendarPicker();
    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish();
    EXPECT_EQ(element->GetTag(), V2::CALENDAR_PICKER_ETS_TAG);

    auto frameNode = AceType::DynamicCast<FrameNode>(element);
    ASSERT_NE(frameNode, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<CalendarPickerLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    auto buttonNode = FrameNode::GetOrCreateFrameNode(
        V2::BUTTON_ETS_TAG, stack->ClaimNodeId(), []() { return AceType::MakeRefPtr<ButtonPattern>(); });
    ASSERT_NE(buttonNode, nullptr);
    auto buttonLayoutProperty = buttonNode->GetLayoutProperty<ButtonLayoutProperty>();
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto renderContext = buttonNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    ASSERT_NE(themeManager, nullptr);
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    auto buttonTheme = AceType::MakeRefPtr<ButtonTheme>();
    ASSERT_NE(buttonTheme, nullptr);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(buttonTheme));
    EXPECT_CALL(*themeManager, GetTheme(_, _))
        .WillRepeatedly([buttonTheme](ThemeType type, int32_t themeScopeId) -> RefPtr<Theme> {
            (void)type;
            (void)themeScopeId;
            return buttonTheme;
        });

    CalendarDialogView::UpdateButtonStyles(buttonInfos, sizet, buttonLayoutProperty, renderContext);
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto testval = buttonLayoutProperty->GetFontSizeValue();
    EXPECT_EQ(testval, Dimension(0));
}
// /**
//  * @tc.name: CalendarPickerUpdateButtonStyles016
//  * @tc.desc: Test UpdateButtonStyle.
//  * @tc.type: FUNC
//  */
HWTEST_F(CalendarPickerTestUpdate, CalendarPickerUpdateButtonStyles016, TestSize.Level1)
{
    std::vector<ButtonInfo> buttonInfos;
    ButtonInfo info1;
    info1.fontSize = Dimension(100000);
    buttonInfos.push_back(info1);

    size_t sizet = 0;

    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);

    CreateCalendarPicker();
    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish();
    EXPECT_EQ(element->GetTag(), V2::CALENDAR_PICKER_ETS_TAG);

    auto frameNode = AceType::DynamicCast<FrameNode>(element);
    ASSERT_NE(frameNode, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<CalendarPickerLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    auto buttonNode = FrameNode::GetOrCreateFrameNode(
        V2::BUTTON_ETS_TAG, stack->ClaimNodeId(), []() { return AceType::MakeRefPtr<ButtonPattern>(); });
    ASSERT_NE(buttonNode, nullptr);
    auto buttonLayoutProperty = buttonNode->GetLayoutProperty<ButtonLayoutProperty>();
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto renderContext = buttonNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    ASSERT_NE(themeManager, nullptr);
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    auto buttonTheme = AceType::MakeRefPtr<ButtonTheme>();
    ASSERT_NE(buttonTheme, nullptr);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(buttonTheme));
    EXPECT_CALL(*themeManager, GetTheme(_, _))
        .WillRepeatedly([buttonTheme](ThemeType type, int32_t themeScopeId) -> RefPtr<Theme> {
            (void)type;
            (void)themeScopeId;
            return buttonTheme;
        });

    CalendarDialogView::UpdateButtonStyles(buttonInfos, sizet, buttonLayoutProperty, renderContext);
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto testval = buttonLayoutProperty->GetFontSizeValue();
    EXPECT_EQ(testval, Dimension(100000));
}

// /**
//  * @tc.name: CalendarPickerUpdateButtonStyles017
//  * @tc.desc: Test UpdateButtonStyle.
//  * @tc.type: FUNC
//  */
HWTEST_F(CalendarPickerTestUpdate, CalendarPickerUpdateButtonStyles017, TestSize.Level1)
{
    std::vector<ButtonInfo> buttonInfos;
    ButtonInfo info1;
    info1.fontSize = Dimension(1);
    buttonInfos.push_back(info1);

    size_t sizet = 0;

    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);

    CreateCalendarPicker();
    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish();
    EXPECT_EQ(element->GetTag(), V2::CALENDAR_PICKER_ETS_TAG);

    auto frameNode = AceType::DynamicCast<FrameNode>(element);
    ASSERT_NE(frameNode, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<CalendarPickerLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    auto buttonNode = FrameNode::GetOrCreateFrameNode(
        V2::BUTTON_ETS_TAG, stack->ClaimNodeId(), []() { return AceType::MakeRefPtr<ButtonPattern>(); });
    ASSERT_NE(buttonNode, nullptr);
    auto buttonLayoutProperty = buttonNode->GetLayoutProperty<ButtonLayoutProperty>();
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto renderContext = buttonNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    ASSERT_NE(themeManager, nullptr);
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    auto buttonTheme = AceType::MakeRefPtr<ButtonTheme>();
    ASSERT_NE(buttonTheme, nullptr);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(buttonTheme));
    EXPECT_CALL(*themeManager, GetTheme(_, _))
        .WillRepeatedly([buttonTheme](ThemeType type, int32_t themeScopeId) -> RefPtr<Theme> {
            (void)type;
            (void)themeScopeId;
            return buttonTheme;
        });

    CalendarDialogView::UpdateButtonStyles(buttonInfos, sizet, buttonLayoutProperty, renderContext);
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto testval = buttonLayoutProperty->GetFontSizeValue();
    EXPECT_EQ(testval, Dimension(1));
}

// /**
//  * @tc.name: CalendarPickerUpdateButtonStyles018
//  * @tc.desc: Test UpdateButtonStyle.
//  * @tc.type: FUNC
//  */
HWTEST_F(CalendarPickerTestUpdate, CalendarPickerUpdateButtonStyles018, TestSize.Level1)
{
    std::vector<ButtonInfo> buttonInfos;
    ButtonInfo info1;
    ButtonInfo info2;
    info1.fontSize = Dimension(1);
    buttonInfos.push_back(info1);
    info2.fontSize = Dimension(0);
    buttonInfos.push_back(info2);

    size_t sizet = 0;

    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);

    CreateCalendarPicker();
    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish();
    EXPECT_EQ(element->GetTag(), V2::CALENDAR_PICKER_ETS_TAG);

    auto frameNode = AceType::DynamicCast<FrameNode>(element);
    ASSERT_NE(frameNode, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<CalendarPickerLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    auto buttonNode = FrameNode::GetOrCreateFrameNode(
        V2::BUTTON_ETS_TAG, stack->ClaimNodeId(), []() { return AceType::MakeRefPtr<ButtonPattern>(); });
    ASSERT_NE(buttonNode, nullptr);
    auto buttonLayoutProperty = buttonNode->GetLayoutProperty<ButtonLayoutProperty>();
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto renderContext = buttonNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    ASSERT_NE(themeManager, nullptr);
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    auto buttonTheme = AceType::MakeRefPtr<ButtonTheme>();
    ASSERT_NE(buttonTheme, nullptr);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(buttonTheme));
    EXPECT_CALL(*themeManager, GetTheme(_, _))
        .WillRepeatedly([buttonTheme](ThemeType type, int32_t themeScopeId) -> RefPtr<Theme> {
            (void)type;
            (void)themeScopeId;
            return buttonTheme;
        });

    CalendarDialogView::UpdateButtonStyles(buttonInfos, sizet, buttonLayoutProperty, renderContext);
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto testval = buttonLayoutProperty->GetFontSizeValue();
    EXPECT_EQ(testval, Dimension(1));

    sizet++;
    CalendarDialogView::UpdateButtonStyles(buttonInfos, sizet, buttonLayoutProperty, renderContext);
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto testval2 = buttonLayoutProperty->GetFontSizeValue();
    EXPECT_EQ(testval2, Dimension(0));
}

// /**
//  * @tc.name: CalendarPickerUpdateButtonStyles019
//  * @tc.desc: Test UpdateButtonStyle.
//  * @tc.type: FUNC
//  */
HWTEST_F(CalendarPickerTestUpdate, CalendarPickerUpdateButtonStyles019, TestSize.Level1)
{
    std::vector<ButtonInfo> buttonInfos;
    ButtonInfo info1;
    ButtonInfo info2;
    ButtonInfo info3;
    info1.fontSize = Dimension(0);
    buttonInfos.push_back(info1);
    info2.fontSize = Dimension(100);
    buttonInfos.push_back(info2);
    info2.fontSize = Dimension(1);
    buttonInfos.push_back(info2);

    size_t sizet = 0;

    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);

    CreateCalendarPicker();
    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish();
    EXPECT_EQ(element->GetTag(), V2::CALENDAR_PICKER_ETS_TAG);

    auto frameNode = AceType::DynamicCast<FrameNode>(element);
    ASSERT_NE(frameNode, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<CalendarPickerLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    auto buttonNode = FrameNode::GetOrCreateFrameNode(
        V2::BUTTON_ETS_TAG, stack->ClaimNodeId(), []() { return AceType::MakeRefPtr<ButtonPattern>(); });
    ASSERT_NE(buttonNode, nullptr);
    auto buttonLayoutProperty = buttonNode->GetLayoutProperty<ButtonLayoutProperty>();
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto renderContext = buttonNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    ASSERT_NE(themeManager, nullptr);
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    auto buttonTheme = AceType::MakeRefPtr<ButtonTheme>();
    ASSERT_NE(buttonTheme, nullptr);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(buttonTheme));
    EXPECT_CALL(*themeManager, GetTheme(_, _))
        .WillRepeatedly([buttonTheme](ThemeType type, int32_t themeScopeId) -> RefPtr<Theme> {
            (void)type;
            (void)themeScopeId;
            return buttonTheme;
        });

    CalendarDialogView::UpdateButtonStyles(buttonInfos, sizet, buttonLayoutProperty, renderContext);
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto testval = buttonLayoutProperty->GetFontSizeValue();
    EXPECT_EQ(testval, Dimension(0));

    sizet++;
    CalendarDialogView::UpdateButtonStyles(buttonInfos, sizet, buttonLayoutProperty, renderContext);
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto testval2 = buttonLayoutProperty->GetFontSizeValue();
    EXPECT_EQ(testval2, Dimension(100));

    sizet++;
    CalendarDialogView::UpdateButtonStyles(buttonInfos, sizet, buttonLayoutProperty, renderContext);
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto testval3 = buttonLayoutProperty->GetFontSizeValue();
    EXPECT_EQ(testval3, Dimension(1));
}

// /**
//  * @tc.name: CalendarPickerUpdateButtonStyles020
//  * @tc.desc: Test UpdateButtonStyle.
//  * @tc.type: FUNC
//  */
HWTEST_F(CalendarPickerTestUpdate, CalendarPickerUpdateButtonStyles020, TestSize.Level1)
{
    std::vector<ButtonInfo> buttonInfos;
    ButtonInfo info1;
    info1.fontSize = Dimension(0);
    info1.fontColor = Color::BLACK;
    buttonInfos.push_back(info1);

    size_t sizet = 0;

    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);

    CreateCalendarPicker();
    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish();
    EXPECT_EQ(element->GetTag(), V2::CALENDAR_PICKER_ETS_TAG);

    auto frameNode = AceType::DynamicCast<FrameNode>(element);
    ASSERT_NE(frameNode, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<CalendarPickerLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    auto buttonNode = FrameNode::GetOrCreateFrameNode(
        V2::BUTTON_ETS_TAG, stack->ClaimNodeId(), []() { return AceType::MakeRefPtr<ButtonPattern>(); });
    ASSERT_NE(buttonNode, nullptr);
    auto buttonLayoutProperty = buttonNode->GetLayoutProperty<ButtonLayoutProperty>();
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto renderContext = buttonNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    ASSERT_NE(themeManager, nullptr);
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    auto buttonTheme = AceType::MakeRefPtr<ButtonTheme>();
    ASSERT_NE(buttonTheme, nullptr);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(buttonTheme));
    EXPECT_CALL(*themeManager, GetTheme(_, _))
        .WillRepeatedly([buttonTheme](ThemeType type, int32_t themeScopeId) -> RefPtr<Theme> {
            (void)type;
            (void)themeScopeId;
            return buttonTheme;
        });

    CalendarDialogView::UpdateButtonStyles(buttonInfos, sizet, buttonLayoutProperty, renderContext);
    ASSERT_NE(buttonLayoutProperty, nullptr);

    auto testval = buttonLayoutProperty->GetFontSizeValue();
    EXPECT_EQ(testval, Dimension(0));

    auto testval2 = buttonLayoutProperty->GetFontColorValue();
    EXPECT_EQ(testval2, Color::BLACK);
}

// /**
//  * @tc.name: CalendarPickerUpdateButtonStyles021
//  * @tc.desc: Test UpdateButtonStyle.
//  * @tc.type: FUNC
//  */
HWTEST_F(CalendarPickerTestUpdate, CalendarPickerUpdateButtonStyles021, TestSize.Level1)
{
    std::vector<ButtonInfo> buttonInfos;
    ButtonInfo info1;
    info1.fontSize = Dimension(1);
    info1.fontColor = Color::FOREGROUND;
    buttonInfos.push_back(info1);

    size_t sizet = 0;

    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);

    CreateCalendarPicker();
    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish();
    EXPECT_EQ(element->GetTag(), V2::CALENDAR_PICKER_ETS_TAG);

    auto frameNode = AceType::DynamicCast<FrameNode>(element);
    ASSERT_NE(frameNode, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<CalendarPickerLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    auto buttonNode = FrameNode::GetOrCreateFrameNode(
        V2::BUTTON_ETS_TAG, stack->ClaimNodeId(), []() { return AceType::MakeRefPtr<ButtonPattern>(); });
    ASSERT_NE(buttonNode, nullptr);
    auto buttonLayoutProperty = buttonNode->GetLayoutProperty<ButtonLayoutProperty>();
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto renderContext = buttonNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    ASSERT_NE(themeManager, nullptr);
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    auto buttonTheme = AceType::MakeRefPtr<ButtonTheme>();
    ASSERT_NE(buttonTheme, nullptr);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(buttonTheme));
    EXPECT_CALL(*themeManager, GetTheme(_, _))
        .WillRepeatedly([buttonTheme](ThemeType type, int32_t themeScopeId) -> RefPtr<Theme> {
            (void)type;
            (void)themeScopeId;
            return buttonTheme;
        });

    CalendarDialogView::UpdateButtonStyles(buttonInfos, sizet, buttonLayoutProperty, renderContext);
    ASSERT_NE(buttonLayoutProperty, nullptr);

    auto testval = buttonLayoutProperty->GetFontSizeValue();
    EXPECT_EQ(testval, Dimension(1));

    auto testval2 = buttonLayoutProperty->GetFontColorValue();
    EXPECT_EQ(testval2, Color::FOREGROUND);
}

// /**
//  * @tc.name: CalendarPickerUpdateButtonStyles022
//  * @tc.desc: Test UpdateButtonStyle.
//  * @tc.type: FUNC
//  */
HWTEST_F(CalendarPickerTestUpdate, CalendarPickerUpdateButtonStyles022, TestSize.Level1)
{
    std::vector<ButtonInfo> buttonInfos;
    ButtonInfo info1;
    info1.fontSize = Dimension(100);
    info1.fontColor = Color::GREEN;
    buttonInfos.push_back(info1);

    size_t sizet = 0;

    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);

    CreateCalendarPicker();
    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish();
    EXPECT_EQ(element->GetTag(), V2::CALENDAR_PICKER_ETS_TAG);

    auto frameNode = AceType::DynamicCast<FrameNode>(element);
    ASSERT_NE(frameNode, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<CalendarPickerLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    auto buttonNode = FrameNode::GetOrCreateFrameNode(
        V2::BUTTON_ETS_TAG, stack->ClaimNodeId(), []() { return AceType::MakeRefPtr<ButtonPattern>(); });
    ASSERT_NE(buttonNode, nullptr);
    auto buttonLayoutProperty = buttonNode->GetLayoutProperty<ButtonLayoutProperty>();
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto renderContext = buttonNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    ASSERT_NE(themeManager, nullptr);
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    auto buttonTheme = AceType::MakeRefPtr<ButtonTheme>();
    ASSERT_NE(buttonTheme, nullptr);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(buttonTheme));
    EXPECT_CALL(*themeManager, GetTheme(_, _))
        .WillRepeatedly([buttonTheme](ThemeType type, int32_t themeScopeId) -> RefPtr<Theme> {
            (void)type;
            (void)themeScopeId;
            return buttonTheme;
        });

    CalendarDialogView::UpdateButtonStyles(buttonInfos, sizet, buttonLayoutProperty, renderContext);
    ASSERT_NE(buttonLayoutProperty, nullptr);

    auto testval = buttonLayoutProperty->GetFontSizeValue();
    EXPECT_EQ(testval, Dimension(100));

    auto testval2 = buttonLayoutProperty->GetFontColorValue();
    EXPECT_EQ(testval2, Color::GREEN);
}

// /**
//  * @tc.name: CalendarPickerUpdateButtonStyles023
//  * @tc.desc: Test UpdateButtonStyle.
//  * @tc.type: FUNC
//  */
HWTEST_F(CalendarPickerTestUpdate, CalendarPickerUpdateButtonStyles023, TestSize.Level1)
{
    std::vector<ButtonInfo> buttonInfos;
    ButtonInfo info1;
    ButtonInfo info2;
    ButtonInfo info3;
    info1.fontSize = Dimension(100);
    info2.fontSize = Dimension(0);
    info3.fontSize = Dimension(1);
    buttonInfos.push_back(info1);
    buttonInfos.push_back(info2);
    buttonInfos.push_back(info3);
    size_t sizet = 0;
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    CreateCalendarPicker();
    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish();
    EXPECT_EQ(element->GetTag(), V2::CALENDAR_PICKER_ETS_TAG);
    auto frameNode = AceType::DynamicCast<FrameNode>(element);
    ASSERT_NE(frameNode, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<CalendarPickerLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    auto buttonNode = FrameNode::GetOrCreateFrameNode(
        V2::BUTTON_ETS_TAG, stack->ClaimNodeId(), []() { return AceType::MakeRefPtr<ButtonPattern>(); });
    ASSERT_NE(buttonNode, nullptr);
    auto buttonLayoutProperty = buttonNode->GetLayoutProperty<ButtonLayoutProperty>();
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto renderContext = buttonNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    ASSERT_NE(themeManager, nullptr);
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    auto buttonTheme = AceType::MakeRefPtr<ButtonTheme>();
    ASSERT_NE(buttonTheme, nullptr);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(buttonTheme));
    EXPECT_CALL(*themeManager, GetTheme(_, _))
        .WillRepeatedly([buttonTheme](ThemeType type, int32_t themeScopeId) -> RefPtr<Theme> {
            (void)type;
            (void)themeScopeId;
            return buttonTheme;
        });
    CalendarDialogView::UpdateButtonStyles(buttonInfos, sizet, buttonLayoutProperty, renderContext);
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto testval = buttonLayoutProperty->GetFontSizeValue();
    EXPECT_EQ(testval, Dimension(100));
    sizet++;
    CalendarDialogView::UpdateButtonStyles(buttonInfos, sizet, buttonLayoutProperty, renderContext);
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto testval2 = buttonLayoutProperty->GetFontSizeValue();
    EXPECT_EQ(testval2, Dimension(0));
    sizet++;
    CalendarDialogView::UpdateButtonStyles(buttonInfos, sizet, buttonLayoutProperty, renderContext);
    ASSERT_NE(buttonLayoutProperty, nullptr);
    auto testval3 = buttonLayoutProperty->GetFontSizeValue();
    EXPECT_EQ(testval3, Dimension(1));
}

/**
 * @tc.name: CalendarPickerPatternTest050
 * @tc.desc: InitDateIndex Function Test
 * @tc.type: FUNC
 */
HWTEST_F(CalendarPickerTestUpdate, CalendarPickerPatternTest050, TestSize.Level1)
{
    CreateCalendarPicker();

    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish();
    EXPECT_EQ(element->GetTag(), V2::CALENDAR_PICKER_ETS_TAG);

    auto frameNode = AceType::DynamicCast<FrameNode>(element);
    ASSERT_NE(frameNode, nullptr);
    auto pickerPattern = frameNode->GetPattern<CalendarPickerPattern>();
    ASSERT_NE(pickerPattern, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<CalendarPickerLayoutProperty>();
    layoutProperty->UpdateLayoutDirection(TextDirection::RTL);

    pickerPattern->SetCalendarEdgeAlign(CalendarEdgeAlign::EDGE_ALIGN_START);
    pickerPattern->UpdateEdgeAlign();
    EXPECT_EQ(layoutProperty->GetDialogAlignType(), CalendarEdgeAlign::EDGE_ALIGN_END);

    pickerPattern->SetCalendarEdgeAlign(CalendarEdgeAlign::EDGE_ALIGN_END);
    pickerPattern->UpdateEdgeAlign();
    EXPECT_EQ(layoutProperty->GetDialogAlignType(), CalendarEdgeAlign::EDGE_ALIGN_START);

    pickerPattern->SetCalendarEdgeAlign(CalendarEdgeAlign::EDGE_ALIGN_CENTER);
    pickerPattern->UpdateEdgeAlign();
    EXPECT_EQ(layoutProperty->GetDialogAlignType(), CalendarEdgeAlign::EDGE_ALIGN_CENTER);
}

/**
 * @tc.name: CalendarPickerPatternTest051
 * @tc.desc: HandleYearKeyEvent Function Test
 * @tc.type: FUNC
 */
HWTEST_F(CalendarPickerTestUpdate, CalendarPickerPatternTest051, TestSize.Level1)
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
    json->Put("year", 999);
    json->Put("month", 1);
    json->Put("day", 1);
    pickerPattern->SetDate(json->ToString());
    pickerPattern->yearPrefixZeroCount_ = 1;
    pickerPattern->yearEnterCount_ = 2;
    EXPECT_TRUE(pickerPattern->HandleYearKeyEvent(0));

    pickerPattern->isKeyWaiting_ = true;
    json->Put("year", 999);
    json->Put("month", 1);
    json->Put("day", 1);
    pickerPattern->SetDate(json->ToString());
    pickerPattern->yearPrefixZeroCount_ = 0;
    pickerPattern->yearEnterCount_ = 2;
    EXPECT_TRUE(pickerPattern->HandleYearKeyEvent(1));

    pickerPattern->isKeyWaiting_ = true;
    json->Put("year", 999);
    json->Put("month", 1);
    json->Put("day", 1);
    pickerPattern->SetDate(json->ToString());
    pickerPattern->yearPrefixZeroCount_ = 3;
    pickerPattern->yearEnterCount_ = 2;
    EXPECT_FALSE(pickerPattern->HandleYearKeyEvent(0));
}

/**
 * @tc.name: CalendarPickerPatternTest052
 * @tc.desc: HandleYearKeyEvent Function Test
 * @tc.type: FUNC
 */
HWTEST_F(CalendarPickerTestUpdate, CalendarPickerPatternTest052, TestSize.Level1)
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
    json->Put("year", 0);
    json->Put("month", 1);
    json->Put("day", 1);
    pickerPattern->SetDate(json->ToString());
    pickerPattern->yearPrefixZeroCount_ = 1;
    pickerPattern->yearEnterCount_ = 2;
    EXPECT_TRUE(pickerPattern->HandleYearKeyEvent(1));

    pickerPattern->isKeyWaiting_ = true;
    json->Put("year", 0);
    json->Put("month", 1);
    json->Put("day", 1);
    pickerPattern->SetDate(json->ToString());
    pickerPattern->yearPrefixZeroCount_ = 0;
    pickerPattern->yearEnterCount_ = 2;
    EXPECT_TRUE(pickerPattern->HandleYearKeyEvent(1));

    pickerPattern->isKeyWaiting_ = true;
    json->Put("year", 0);
    json->Put("month", 1);
    json->Put("day", 1);
    pickerPattern->SetDate(json->ToString());
    pickerPattern->yearPrefixZeroCount_ = 1;
    pickerPattern->yearEnterCount_ = 3;
    EXPECT_TRUE(pickerPattern->HandleYearKeyEvent(1));
}

/**
 * @tc.name: CalendarPickerPatternTest053
 * @tc.desc: HandleYearKeyEvent Function Test
 * @tc.type: FUNC
 */
HWTEST_F(CalendarPickerTestUpdate, CalendarPickerPatternTest053, TestSize.Level1)
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
    json->Put("year", 0);
    json->Put("month", 1);
    json->Put("day", 1);
    pickerPattern->SetDate(json->ToString());
    pickerPattern->yearPrefixZeroCount_ = 1;
    pickerPattern->yearEnterCount_ = 3;
    EXPECT_TRUE(pickerPattern->HandleYearKeyEvent(0));

    pickerPattern->isKeyWaiting_ = true;
    json->Put("year", 0);
    json->Put("month", 1);
    json->Put("day", 1);
    pickerPattern->SetDate(json->ToString());
    pickerPattern->yearPrefixZeroCount_ = 0;
    pickerPattern->yearEnterCount_ = 1;
    EXPECT_TRUE(pickerPattern->HandleYearKeyEvent(0));

    pickerPattern->isKeyWaiting_ = true;
    json->Put("year", 0);
    json->Put("month", 1);
    json->Put("day", 1);
    pickerPattern->SetDate(json->ToString());
    pickerPattern->yearPrefixZeroCount_ = 0;
    pickerPattern->yearEnterCount_ = 2;
    EXPECT_TRUE(pickerPattern->HandleYearKeyEvent(0));
}

/**
 * @tc.name: CalendarPickerPatternTest054
 * @tc.desc: HandleZeroStartTaskCallback Function Test
 * @tc.type: FUNC
 */
HWTEST_F(CalendarPickerTestUpdate, CalendarPickerPatternTest054, TestSize.Level1)
{
    CreateCalendarPicker();

    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish();
    EXPECT_EQ(element->GetTag(), V2::CALENDAR_PICKER_ETS_TAG);

    auto frameNode = AceType::DynamicCast<FrameNode>(element);
    ASSERT_NE(frameNode, nullptr);
    auto pickerPattern = frameNode->GetPattern<CalendarPickerPattern>();
    ASSERT_NE(pickerPattern, nullptr);

    pickerPattern->taskCount_ = 2;
    pickerPattern->HandleZeroStartTaskCallback();
    EXPECT_EQ(pickerPattern->taskCount_, 1);

    pickerPattern->taskCount_ = 0;
    pickerPattern->isKeyWaiting_ = true;
    pickerPattern->HandleZeroStartTaskCallback();
    EXPECT_EQ(pickerPattern->taskCount_, 0);
    EXPECT_FALSE(pickerPattern->isKeyWaiting_);

    pickerPattern->taskCount_ = 0;
    pickerPattern->isKeyWaiting_ = false;
    pickerPattern->HandleZeroStartTaskCallback();
    EXPECT_EQ(pickerPattern->taskCount_, 0);
    EXPECT_FALSE(pickerPattern->isKeyWaiting_);
}

/**
 * @tc.name: CalendarPickerCreateAndSetMarkTodayTest
 * @tc.desc: CalendarPickerCreateAndSetMarkTodayTest function test.
 * @tc.type: FUNC
 */
HWTEST_F(CalendarPickerTestUpdate, CalendarPickerCreateAndSetMarkTodayTest, TestSize.Level1)
{
    CalendarSettingData settingData;
    CalendarPickerModelNG::GetInstance()->Create(settingData);
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);

    CalendarPickerModelNG::SetMarkToday(frameNode, false);
    EXPECT_EQ(CalendarPickerModelNG::GetMarkToday(frameNode), false);
}

/**
 * @tc.name: CalendarPickerI18nGetDateSeparators001
 * @tc.desc: Test GetDateSeparators returns exactly two separators.
 * @tc.type: FUNC
 */
HWTEST_F(CalendarPickerTestUpdate, CalendarPickerI18nGetDateSeparators001, TestSize.Level1)
{
    CalendarPickerI18n::ClearCache();
    auto separators = CalendarPickerI18n::GetDateSeparators("en");
    EXPECT_EQ(separators.size(), 2);
}

/**
 * @tc.name: CalendarPickerI18nGetDateSeparators002
 * @tc.desc: Test GetDateSeparators cache hit returns same result for same locale.
 * @tc.type: FUNC
 */
HWTEST_F(CalendarPickerTestUpdate, CalendarPickerI18nGetDateSeparators002, TestSize.Level1)
{
    CalendarPickerI18n::ClearCache();
    auto sep1 = CalendarPickerI18n::GetDateSeparators("en");
    auto sep2 = CalendarPickerI18n::GetDateSeparators("en");
    EXPECT_EQ(sep1, sep2);
}

/**
 * @tc.name: CalendarPickerI18nGetDateSeparators003
 * @tc.desc: Test GetDateSeparators returns fallback for empty locale.
 * @tc.type: FUNC
 */
HWTEST_F(CalendarPickerTestUpdate, CalendarPickerI18nGetDateSeparators003, TestSize.Level1)
{
    CalendarPickerI18n::ClearCache();
    auto separators = CalendarPickerI18n::GetDateSeparators("");
    EXPECT_EQ(separators.size(), 2);
    EXPECT_EQ(separators[0], "/");
    EXPECT_EQ(separators[1], "/");
}

/**
 * @tc.name: CalendarPickerI18nGetDateSeparators004
 * @tc.desc: Test GetDateSeparators with different locales does not crash.
 * @tc.type: FUNC
 */
HWTEST_F(CalendarPickerTestUpdate, CalendarPickerI18nGetDateSeparators004, TestSize.Level1)
{
    CalendarPickerI18n::ClearCache();
    auto sepEn = CalendarPickerI18n::GetDateSeparators("en");
    EXPECT_EQ(sepEn.size(), 2);
    auto sepZh = CalendarPickerI18n::GetDateSeparators("zh");
    EXPECT_EQ(sepZh.size(), 2);
}

/**
 * @tc.name: CalendarPickerI18nLocalizeDateNumber001
 * @tc.desc: Test LocalizeDateNumber pads year to four digits.
 * @tc.type: FUNC
 */
HWTEST_F(CalendarPickerTestUpdate, CalendarPickerI18nLocalizeDateNumber001, TestSize.Level1)
{
    CalendarPickerI18n::ClearCache();
    auto result = CalendarPickerI18n::LocalizeDateNumber(2024, 4, "en");
    EXPECT_GE(result.length(), 4);
    EXPECT_NE(result.find("2024"), std::string::npos);
}

/**
 * @tc.name: CalendarPickerI18nLocalizeDateNumber002
 * @tc.desc: Test LocalizeDateNumber pads month to two digits.
 * @tc.type: FUNC
 */
HWTEST_F(CalendarPickerTestUpdate, CalendarPickerI18nLocalizeDateNumber002, TestSize.Level1)
{
    CalendarPickerI18n::ClearCache();
    auto result = CalendarPickerI18n::LocalizeDateNumber(3, 2, "en");
    EXPECT_GE(result.length(), 2);
}

/**
 * @tc.name: CalendarPickerI18nLocalizeDateNumber003
 * @tc.desc: Test LocalizeDateNumber pads day to two digits.
 * @tc.type: FUNC
 */
HWTEST_F(CalendarPickerTestUpdate, CalendarPickerI18nLocalizeDateNumber003, TestSize.Level1)
{
    CalendarPickerI18n::ClearCache();
    auto result = CalendarPickerI18n::LocalizeDateNumber(7, 2, "en");
    EXPECT_GE(result.length(), 2);
}

/**
 * @tc.name: CalendarPickerI18nLocalizeDateNumber004
 * @tc.desc: Test LocalizeDateNumber cache hit returns same result for same input.
 * @tc.type: FUNC
 */
HWTEST_F(CalendarPickerTestUpdate, CalendarPickerI18nLocalizeDateNumber004, TestSize.Level1)
{
    CalendarPickerI18n::ClearCache();
    auto r1 = CalendarPickerI18n::LocalizeDateNumber(15, 2, "en");
    auto r2 = CalendarPickerI18n::LocalizeDateNumber(15, 2, "en");
    EXPECT_EQ(r1, r2);
}

/**
 * @tc.name: CalendarPickerI18nLocalizeDateNumber005
 * @tc.desc: Test LocalizeDateNumber uses separate caches for different minDigits.
 * @tc.type: FUNC
 */
HWTEST_F(CalendarPickerTestUpdate, CalendarPickerI18nLocalizeDateNumber005, TestSize.Level1)
{
    CalendarPickerI18n::ClearCache();
    auto r2 = CalendarPickerI18n::LocalizeDateNumber(12, 2, "en");
    auto r4 = CalendarPickerI18n::LocalizeDateNumber(12, 4, "en");
    EXPECT_GE(r2.length(), 2);
    EXPECT_GE(r4.length(), 4);
}

/**
 * @tc.name: CalendarPickerI18nLocalizeDateNumber006
 * @tc.desc: Test LocalizeDateNumber cache invalidation on locale change.
 * @tc.type: FUNC
 */
HWTEST_F(CalendarPickerTestUpdate, CalendarPickerI18nLocalizeDateNumber006, TestSize.Level1)
{
    CalendarPickerI18n::ClearCache();
    auto rEn = CalendarPickerI18n::LocalizeDateNumber(10, 2, "en");
    auto rZh = CalendarPickerI18n::LocalizeDateNumber(10, 2, "zh");
    EXPECT_FALSE(rEn.empty());
    EXPECT_FALSE(rZh.empty());
}

/**
 * @tc.name: CalendarPickerI18nGetFirstDayOfWeek001
 * @tc.desc: Test GetFirstDayOfWeek returns a valid value in [0, 6].
 * @tc.type: FUNC
 */
HWTEST_F(CalendarPickerTestUpdate, CalendarPickerI18nGetFirstDayOfWeek001, TestSize.Level1)
{
    CalendarPickerI18n::ClearCache();
    auto firstDay = CalendarPickerI18n::GetFirstDayOfWeek("en");
    EXPECT_GE(firstDay, 0);
    EXPECT_LE(firstDay, 6);
}

/**
 * @tc.name: CalendarPickerI18nGetFirstDayOfWeek002
 * @tc.desc: Test GetFirstDayOfWeek cache hit returns same result.
 * @tc.type: FUNC
 */
HWTEST_F(CalendarPickerTestUpdate, CalendarPickerI18nGetFirstDayOfWeek002, TestSize.Level1)
{
    CalendarPickerI18n::ClearCache();
    auto d1 = CalendarPickerI18n::GetFirstDayOfWeek("en");
    auto d2 = CalendarPickerI18n::GetFirstDayOfWeek("en");
    EXPECT_EQ(d1, d2);
}

/**
 * @tc.name: CalendarPickerI18nClearCache001
 * @tc.desc: Test ClearCache forces recompute and result remains correct.
 * @tc.type: FUNC
 */
HWTEST_F(CalendarPickerTestUpdate, CalendarPickerI18nClearCache001, TestSize.Level1)
{
    CalendarPickerI18n::ClearCache();
    auto sepBefore = CalendarPickerI18n::GetDateSeparators("en");
    CalendarPickerI18n::ClearCache();
    auto sepAfter = CalendarPickerI18n::GetDateSeparators("en");
    EXPECT_EQ(sepBefore, sepAfter);

    CalendarPickerI18n::ClearCache();
    auto numBefore = CalendarPickerI18n::LocalizeDateNumber(2024, 4, "en");
    CalendarPickerI18n::ClearCache();
    auto numAfter = CalendarPickerI18n::LocalizeDateNumber(2024, 4, "en");
    EXPECT_EQ(numBefore, numAfter);
}

/**
 * @tc.name: CalendarPickerI18nBatchCall001
 * @tc.desc: Test 100 batch calls with same locale return consistent results.
 * @tc.type: FUNC
 */
HWTEST_F(CalendarPickerTestUpdate, CalendarPickerI18nBatchCall001, TestSize.Level1)
{
    CalendarPickerI18n::ClearCache();
    const std::string locale = "en";
    auto sepFirst = CalendarPickerI18n::GetDateSeparators(locale);
    auto yearFirst = CalendarPickerI18n::LocalizeDateNumber(2024, 4, locale);
    auto monthFirst = CalendarPickerI18n::LocalizeDateNumber(1, 2, locale);
    auto dayFirst = CalendarPickerI18n::LocalizeDateNumber(1, 2, locale);
    auto fdowFirst = CalendarPickerI18n::GetFirstDayOfWeek(locale);

    for (int32_t i = 0; i < 100; ++i) {
        EXPECT_EQ(CalendarPickerI18n::GetDateSeparators(locale), sepFirst);
        EXPECT_EQ(CalendarPickerI18n::LocalizeDateNumber(2024, 4, locale), yearFirst);
        EXPECT_EQ(CalendarPickerI18n::LocalizeDateNumber(1, 2, locale), monthFirst);
        EXPECT_EQ(CalendarPickerI18n::LocalizeDateNumber(1, 2, locale), dayFirst);
        EXPECT_EQ(CalendarPickerI18n::GetFirstDayOfWeek(locale), fdowFirst);
    }
}

/**
 * @tc.name: CalendarPickerI18nClearCache002
 * @tc.desc: Test ClearCache followed by locale switch works correctly.
 * @tc.type: FUNC
 */
HWTEST_F(CalendarPickerTestUpdate, CalendarPickerI18nClearCache002, TestSize.Level1)
{
    CalendarPickerI18n::ClearCache();
    auto sepEn = CalendarPickerI18n::GetDateSeparators("en");
    EXPECT_EQ(sepEn.size(), 2);

    CalendarPickerI18n::ClearCache();
    auto sepZh = CalendarPickerI18n::GetDateSeparators("zh");
    EXPECT_EQ(sepZh.size(), 2);

    CalendarPickerI18n::ClearCache();
    auto fdowEn = CalendarPickerI18n::GetFirstDayOfWeek("en");
    EXPECT_GE(fdowEn, 0);
    EXPECT_LE(fdowEn, 6);
}
} // namespace OHOS::Ace::NG
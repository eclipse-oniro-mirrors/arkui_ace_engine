/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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


#include "test/unittest/core/base/frame_node_test_ng.h"
#include "core/components_ng/pattern/command/command_validation.h"
#include "core/components_ng/pattern/button/button_pattern.h"
#include "core/components_ng/pattern/button/toggle_button_pattern.h"
#include "core/components_ng/pattern/toggle/switch_pattern.h"
#include "core/components_ng/pattern/checkbox/toggle_checkbox_pattern.h"
#include "core/components_ng/pattern/checkbox/checkbox_pattern.h"
#include "core/components_ng/pattern/checkboxgroup/checkboxgroup_pattern.h"
#include "core/components_ng/pattern/slider/slider_pattern.h"
#include "core/components_ng/pattern/select/select_pattern.h"
#include "core/components_ng/pattern/indexer/indexer_pattern.h"
#include "core/components_ng/pattern/text/text_pattern.h"
#include "core/components_ng/pattern/text_field/text_field_pattern.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_pattern.h"
#include "core/components_ng/pattern/search/search_pattern.h"
#include "core/components_ng/pattern/counter/counter_pattern.h"
#include "core/components_ng/pattern/list/list_pattern.h"
#include "core/components_ng/pattern/waterflow/water_flow_pattern.h"
#include "core/components_ng/pattern/scroll/scroll_pattern.h"
#include "core/components_ng/pattern/date_picker/datepicker_pattern.h"
#include "core/components_ng/pattern/time_picker/timepicker_row_pattern.h"
#include "core/components_ng/pattern/time_picker/timepicker_model_ng.h"
#include "test/mock/frameworks/core/common/mock_theme_manager.h"
#include "test/mock/frameworks/core/common/mock_theme_default.h"
#include "core/components_ng/pattern/dialog/dialog_pattern.h"
#include "core/components_ng/pattern/overlay/modal_presentation_pattern.h"
#include "core/components_ng/pattern/sheet/sheet_presentation_pattern.h"
#include "core/components_ng/pattern/hyperlink/hyperlink_pattern.h"
#ifdef VIDEO_SUPPORTED
#include "core/components_ng/pattern/video/video_pattern.h"
#include "core/components_ng/pattern/video/video_state_machine_pattern.h"
#endif
#ifdef WEB_SUPPORTED
#include "core/components_ng/pattern/web/web_pattern.h"
#endif

using namespace testing;
using namespace testing::ext;
namespace OHOS::Ace::NG {
// Keep conflicting legacy component header aliases in separate translation units.
RefPtr<Pattern> CreateSendCommandRatingTestPattern();
RefPtr<Pattern> CreateSendCommandRadioTestPattern();
RefPtr<Pattern> CreateSendCommandTabsTestPattern();
RefPtr<Pattern> CreateSendCommandSwiperTestPattern();
RefPtr<Pattern> CreateSendCommandGridTestPattern();
RefPtr<Pattern> CreateSendCommandCalendarTestPattern();

namespace {
struct CommandErrorTestItem {
    const char* name;
    RefPtr<Pattern> pattern;
    const char* command;
};

std::vector<CommandErrorTestItem> CreateBasicCommandErrorCases()
{
    return {
        { "ButtonPattern", AceType::MakeRefPtr<ButtonPattern>(), R"({"cmd":"onButtonClick"})" },
        { "ToggleButtonPattern", AceType::MakeRefPtr<ToggleButtonPattern>(),
            R"({"cmd":"onToggleChange","params":{"isOn":true}})" },
        { "SwitchPattern", AceType::MakeRefPtr<SwitchPattern>(), R"({"cmd":"onToggleChange","params":{"isOn":true}})" },
        { "ToggleCheckBoxPattern", AceType::MakeRefPtr<ToggleCheckBoxPattern>(),
            R"({"cmd":"onToggleChange","params":{"isOn":true}})" },
        { "CheckBoxPattern", AceType::MakeRefPtr<CheckBoxPattern>(),
            R"({"cmd":"selectCheckBox","selectStatus":true})" },
        { "CheckBoxGroupPattern", AceType::MakeRefPtr<CheckBoxGroupPattern>(),
            R"({"cmd":"selectCheckBoxGroup","selectStatus":true})" },
        { "RadioPattern", CreateSendCommandRadioTestPattern(), R"({"cmd":"checked"})" },
        { "RatingPattern", CreateSendCommandRatingTestPattern(), R"({"cmd":"onRatingChange","params":{"value":1}})" },
        { "SliderPattern", AceType::MakeRefPtr<SliderPattern>(), R"({"cmd":"onSliderChange","params":{"value":1}})" },
        { "SelectPattern", AceType::MakeRefPtr<SelectPattern>(), R"({"cmd":"onSelect","params":{"index":0}})" },
        { "IndexerPattern", AceType::MakeRefPtr<IndexerPattern>(),
            R"({"cmd":"setAlphabetIndexer","params":{"value":0}})" },
        { "TextPattern", AceType::MakeRefPtr<TextPattern>(),
            R"({"cmd":"selectText","selectionStart":0,"selectionEnd":1})" },
        { "TextFieldPattern", AceType::MakeRefPtr<TextFieldPattern>(),
            R"({"cmd":"setText","params":{"value":"test"}})" },
        { "RichEditorPattern", AceType::MakeRefPtr<RichEditorPattern>(),
            R"({"cmd":"setText","params":{"value":"test"}})" },
        { "SearchPattern", AceType::MakeRefPtr<SearchPattern>(),
            R"({"cmd":"setSearchText","params":{"value":"test"}})" },
        { "CounterPattern", AceType::MakeRefPtr<CounterPattern>(), R"({"cmd":"setCounterOnInc"})" },
    };
}

std::vector<CommandErrorTestItem> CreateContainerCommandErrorCases()
{
    return {
        { "TabsPattern", CreateSendCommandTabsTestPattern(), R"({"cmd":"changeIndex","params":{"index":"0"}})" },
        { "SwiperPattern", CreateSendCommandSwiperTestPattern(), R"({"cmd":"change","params":{"type":"forward"}})" },
        { "ListPattern", AceType::MakeRefPtr<ListPattern>(), R"({"cmd":"scrollByOffset","offset":1})" },
        { "GridPattern", CreateSendCommandGridTestPattern(), R"({"cmd":"scrollByOffset","offset":1})" },
        { "WaterFlowPattern", AceType::MakeRefPtr<WaterFlowPattern>(), R"({"cmd":"scrollByOffset","offset":1})" },
        { "ScrollPattern", AceType::MakeRefPtr<ScrollPattern>(), R"({"cmd":"scrollByOffset","offset":1})" },
        { "DatePickerPattern", AceType::MakeRefPtr<DatePickerPattern>(),
            R"({"cmd":"setDatePickerTime","params":{"year":2026,"month":1,"day":1}})" },
        { "TimePickerRowPattern", AceType::MakeRefPtr<TimePickerRowPattern>(),
            R"({"cmd":"setTimePickerTime","params":{"hour":1,"minute":0,"second":0}})" },
        { "CalendarDialogPattern", CreateSendCommandCalendarTestPattern(),
            R"({"cmd":"setCalendarPickerDialogTime","params":{"year":2026,"month":1,"day":1}})" },
        { "DialogPattern", AceType::MakeRefPtr<DialogPattern>(nullptr, nullptr),
            R"({"cmd":"alertDialogButtonClick","params":{"buttonIndex":0}})" },
        { "ModalPresentationPattern",
            AceType::MakeRefPtr<ModalPresentationPattern>(0, ModalTransition::DEFAULT, nullptr),
            R"({"cmd":"CloseContentCover"})" },
        { "SheetPresentationPattern", AceType::MakeRefPtr<SheetPresentationPattern>(0, "target", nullptr),
            R"({"cmd":"CloseSheet"})" },
        { "HyperlinkPattern", AceType::MakeRefPtr<HyperlinkPattern>(), R"({"cmd":"click"})" },
#ifdef VIDEO_SUPPORTED
        { "VideoPattern", AceType::MakeRefPtr<VideoPattern>(nullptr),
            R"({"cmd":"setVideoPlayerStatus","value":"play"})" },
        { "VideoStateMachinePattern", AceType::MakeRefPtr<VideoStateMachinePattern>(nullptr),
            R"({"cmd":"setVideoPlayerStatus","value":"play"})" },
#endif
    };
}
} // namespace

HWTEST_F(FrameNodeTestNg, SendCommandComponentStructuredErrorMatrix, TestSize.Level1)
{
    for (const auto& items : { CreateBasicCommandErrorCases(), CreateContainerCommandErrorCases() }) {
        for (const auto& item : items) {
            SCOPED_TRACE(item.name);
            ASSERT_NE(item.pattern, nullptr);
            EXPECT_EQ(item.pattern->OnInjectionEventWithResult("{").errorType, CommandErrorType::INVALID_PARAM);
            EXPECT_EQ(item.pattern->OnInjectionEventWithResult(R"({"cmd":"unsupported"})").errorType,
                CommandErrorType::INVALID_COMMAND);
            EXPECT_EQ(item.pattern->OnInjectionEventWithResult(item.command).errorType, CommandErrorType::EXECUTE_FAIL);
        }
    }
#ifdef WEB_SUPPORTED
    auto web = AceType::MakeRefPtr<WebPattern>();
    EXPECT_EQ(web->OnInjectionEventWithResult("{").errorType, CommandErrorType::INVALID_PARAM);
    EXPECT_EQ(web->OnInjectionEventWithResult(R"({"event_type":"unsupported"})").errorType,
        CommandErrorType::INVALID_COMMAND);
    EXPECT_EQ(web->OnInjectionEventWithResult(R"({"event_type":"click"})").errorType, CommandErrorType::EXECUTE_FAIL);
#endif
}

HWTEST_F(FrameNodeTestNg, SendCommandComponentSuccessAndDisabledMatrix, TestSize.Level1)
{
    struct Item {
        const char* tag;
        RefPtr<Pattern> pattern;
        const char* command;
    };
    const std::vector<Item> items {
        { "Button", AceType::MakeRefPtr<ButtonPattern>(), R"({"cmd":"onButtonClick"})" },
        { "Toggle", AceType::MakeRefPtr<ToggleButtonPattern>(),
            R"({"cmd":"onToggleChange","params":{"isOn":true}})" },
        { "Toggle", AceType::MakeRefPtr<SwitchPattern>(),
            R"({"cmd":"onToggleChange","params":{"isOn":true}})" },
        { "Toggle", AceType::MakeRefPtr<ToggleCheckBoxPattern>(),
            R"({"cmd":"onToggleChange","params":{"isOn":true}})" },
        { "Checkbox", AceType::MakeRefPtr<CheckBoxPattern>(), R"({"cmd":"selectCheckBox","selectStatus":true})" },
        { "Radio", CreateSendCommandRadioTestPattern(), R"({"cmd":"checked","params":{"value":true}})" },
        { "Rating", CreateSendCommandRatingTestPattern(), R"({"cmd":"onRatingChange","params":{"value":1}})" },
        { "Slider", AceType::MakeRefPtr<SliderPattern>(), R"({"cmd":"onSliderChange","params":{"value":1}})" },
    };
    for (const auto& item : items) {
        SCOPED_TRACE(item.tag);
        auto node = FrameNode::CreateFrameNode(item.tag, ElementRegister::GetInstance()->MakeUniqueId(), item.pattern);
        ASSERT_NE(node, nullptr);
        node->GetOrCreateGestureEventHub()->SetUserOnClick([](GestureEvent&) {});
        EXPECT_TRUE(node->OnRecvCommandWithResult(item.command).success);
        node->GetEventHub<EventHub>()->SetEnabled(false);
        EXPECT_EQ(node->OnRecvCommandWithResult(item.command).errorType, CommandErrorType::EXECUTE_FAIL);
    }
}

HWTEST_F(FrameNodeTestNg, SendCommandClickExecutesExactlyOnceAndHonorsDisabledState, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<ButtonPattern>();
    auto node = FrameNode::CreateFrameNode("Button", ElementRegister::GetInstance()->MakeUniqueId(), pattern);
    ASSERT_NE(node, nullptr);
    int clicks = 0;
    node->GetOrCreateGestureEventHub()->SetUserOnClick([&clicks](GestureEvent&) { ++clicks; });
    EXPECT_TRUE(node->OnRecvCommandWithResult(R"({"cmd":"click"})").success);
    EXPECT_EQ(clicks, 1);
    node->GetEventHub<EventHub>()->SetEnabled(false);
    EXPECT_EQ(node->OnRecvCommandWithResult(R"({"cmd":"click"})").errorType, CommandErrorType::EXECUTE_FAIL);
    EXPECT_EQ(clicks, 1);
}

HWTEST_F(FrameNodeTestNg, SendCommandRejectsWrongParameterTypesBeforeExecuting, TestSize.Level1)
{
    for (const auto& command : {
        R"({"cmd":"setText","params":{"value":123}})",
        R"({"cmd":"selectText","selectionStart":"0","selectionEnd":1})",
        R"({"cmd":"onToggleChange","params":{"isOn":"true"}})",
        R"({"cmd":"setVideoPlaybackSpeed","value":999})",
        R"({"cmd":"changeIndex","params":{"index":"garbage"}})",
        R"({"cmd":"onRatingChange","params":{"value":"5"}})" }) {
        auto json = JsonUtil::ParseJsonString(command);
        EXPECT_FALSE(ValidateCommandParameters(json, json->GetString("cmd")));
    }
}

HWTEST_F(FrameNodeTestNg, SendCommandRichEditorDeniedCopyDoesNotReportSuccess, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<RichEditorPattern>();
    auto node = FrameNode::CreateFrameNode("RichEditor", ElementRegister::GetInstance()->MakeUniqueId(), pattern);
    ASSERT_NE(node, nullptr);
    pattern->copyOption_ = CopyOptions::None;
    for (const auto* command : { R"({"cmd":"copy"})", R"({"cmd":"cut"})" }) {
        EXPECT_EQ(pattern->OnInjectionEvent(command), RET_SUCCESS);
        const auto result = node->OnRecvCommandWithResult(command);
        EXPECT_FALSE(result.success);
        EXPECT_EQ(result.errorType, CommandErrorType::EXECUTE_FAIL);
    }
}

HWTEST_F(FrameNodeTestNg, SendCommandTextInputAndPickerSuccessMatrix, TestSize.Level1)
{
    struct Item { const char* tag; RefPtr<Pattern> pattern; const char* command; };
    const std::vector<Item> items {
        { "CheckboxGroup", AceType::MakeRefPtr<CheckBoxGroupPattern>(),
            R"({"cmd":"selectCheckBoxGroup","selectStatus":false})" },
        { "Text", AceType::MakeRefPtr<TextPattern>(),
            R"({"cmd":"selectText","selectionStart":0,"selectionEnd":0})" },
        { "TextInput", AceType::MakeRefPtr<TextFieldPattern>(),
            R"({"cmd":"setText","params":{"value":"hello"}})" },
        { "RichEditor", AceType::MakeRefPtr<RichEditorPattern>(),
            R"({"cmd":"setText","params":{"value":"hello"}})" },
        { "CalendarPickerDialog", CreateSendCommandCalendarTestPattern(),
            R"({"cmd":"setCalendarPickerDialogTime","params":{"year":2026,"month":1,"day":1}})" },
        { "DatePicker", AceType::MakeRefPtr<DatePickerPattern>(),
            R"({"cmd":"setDatePickerTime","params":{"year":2026,"month":1,"day":1}})" },
    };
    for (const auto& item : items) {
        SCOPED_TRACE(item.tag);
        auto node = FrameNode::CreateFrameNode(item.tag, ElementRegister::GetInstance()->MakeUniqueId(), item.pattern);
        ASSERT_NE(node, nullptr);
        EXPECT_EQ(node->OnRecvCommandWithResult("{").errorType, CommandErrorType::INVALID_PARAM);
        EXPECT_EQ(node->OnRecvCommandWithResult(R"({"cmd":"unsupported"})").errorType,
            CommandErrorType::INVALID_COMMAND);
        node->GetEventHub<EventHub>()->SetEnabled(false);
        EXPECT_EQ(node->OnRecvCommandWithResult(item.command).errorType, CommandErrorType::EXECUTE_FAIL);
        node->GetEventHub<EventHub>()->SetEnabled(true);
        EXPECT_TRUE(node->OnRecvCommandWithResult(item.command).success);
    }
}

HWTEST_F(FrameNodeTestNg, SendCommandSearchUsesItsTextFieldAndRejectsBadValue, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<SearchPattern>();
    auto node = FrameNode::CreateFrameNode("Search", ElementRegister::GetInstance()->MakeUniqueId(), pattern);
    auto field = FrameNode::CreateFrameNode("SearchField", ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<TextFieldPattern>());
    field->MountToParent(node);
    EXPECT_EQ(node->OnRecvCommandWithResult(R"({"cmd":"setSearchText","params":{"value":12}})").errorType,
        CommandErrorType::INVALID_PARAM);
    EXPECT_TRUE(node->OnRecvCommandWithResult(R"({"cmd":"setSearchText","params":{"value":"abc"}})").success);
    node->RemoveChild(field);
    EXPECT_EQ(node->OnRecvCommandWithResult(R"({"cmd":"setSearchText","params":{"value":"abc"}})").errorType,
        CommandErrorType::EXECUTE_FAIL);
}

HWTEST_F(FrameNodeTestNg, SendCommandCounterAndDialogDispatchExactlyOnce, TestSize.Level1)
{
    auto counter = AceType::MakeRefPtr<CounterPattern>();
    auto node = FrameNode::CreateFrameNode("Counter", ElementRegister::GetInstance()->MakeUniqueId(), counter);
    auto add = FrameNode::CreateFrameNode("Button", counter->GetAddId(), AceType::MakeRefPtr<ButtonPattern>());
    add->MountToParent(node);
    int count = 0;
    add->GetOrCreateGestureEventHub()->SetUserOnClick([&count](GestureEvent&) { ++count; });
    EXPECT_TRUE(node->OnRecvCommandWithResult(R"({"cmd":"setCounterOnInc"})").success);
    EXPECT_EQ(count, 1);
    add->GetEventHub<EventHub>()->SetEnabled(false);
    EXPECT_EQ(node->OnRecvCommandWithResult(R"({"cmd":"setCounterOnInc"})").errorType,
        CommandErrorType::EXECUTE_FAIL);
    EXPECT_EQ(count, 1);

    auto dialog = AceType::MakeRefPtr<DialogPattern>(nullptr, nullptr);
    auto dialogNode = FrameNode::CreateFrameNode("Dialog", ElementRegister::GetInstance()->MakeUniqueId(), dialog);
    dialog->dialogProperties_.type = DialogType::ALERT_DIALOG;
    auto container = FrameNode::CreateFrameNode("Row", ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<Pattern>());
    dialog->buttonContainer_ = container;
    auto button = FrameNode::CreateFrameNode("Button", ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<ButtonPattern>());
    button->MountToParent(container);
    button->GetOrCreateGestureEventHub()->SetUserOnClick([&count](GestureEvent&) { ++count; });
    EXPECT_TRUE(dialogNode->OnRecvCommandWithResult(
        R"({"cmd":"alertDialogButtonClick","params":{"buttonIndex":0}})").success);
    EXPECT_EQ(count, 2);
    EXPECT_EQ(dialogNode->OnRecvCommandWithResult(
        R"({"cmd":"alertDialogButtonClick","params":{"buttonIndex":5}})").errorType,
        CommandErrorType::INVALID_PARAM);
    EXPECT_EQ(count, 2);
}

HWTEST_F(FrameNodeTestNg, SendCommandIndexerAndHyperlinkReachSuccess, TestSize.Level1)
{
    auto indexer = AceType::MakeRefPtr<IndexerPattern>();
    auto node = FrameNode::CreateFrameNode("AlphabetIndexer", ElementRegister::GetInstance()->MakeUniqueId(), indexer);
    indexer->itemCount_ = 1;
    indexer->selected_ = 0;
    EXPECT_TRUE(node->OnRecvCommandWithResult(R"({"cmd":"setAlphabetIndexer","params":{"value":0}})").success);
    EXPECT_EQ(node->OnRecvCommandWithResult(R"({"cmd":"setAlphabetIndexer","params":{"value":2}})").errorType,
        CommandErrorType::INVALID_PARAM);
    EXPECT_EQ(indexer->selected_, 0);

    auto link = FrameNode::CreateFrameNode("Hyperlink", ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<HyperlinkPattern>());
    int clicks = 0;
    link->GetOrCreateGestureEventHub()->SetUserOnClick([&clicks](GestureEvent&) { ++clicks; });
    EXPECT_TRUE(link->OnRecvCommandWithResult(R"({"cmd":"click"})").success);
    EXPECT_EQ(clicks, 1);
}

HWTEST_F(FrameNodeTestNg, SendCommandRejectsFailedFocusRequest, TestSize.Level1)
{
    auto field = FrameNode::CreateFrameNode("TextInput", ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<TextFieldPattern>());
    field->GetOrCreateFocusHub()->SetFocusable(false);
    EXPECT_EQ(field->OnRecvCommandWithResult(R"({"cmd":"requestKeyboard"})").errorType,
        CommandErrorType::EXECUTE_FAIL);
    EXPECT_FALSE(field->GetOrCreateFocusHub()->IsCurrentFocus());
    auto search = FrameNode::CreateFrameNode("Search", ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<SearchPattern>());
    field->MountToParent(search);
    EXPECT_EQ(search->OnRecvCommandWithResult(R"({"cmd":"requestKeyboard"})").errorType,
        CommandErrorType::EXECUTE_FAIL);
    EXPECT_FALSE(field->GetOrCreateFocusHub()->IsCurrentFocus());
}


HWTEST_F(FrameNodeTestNg, SendCommandTimePickerUsesInitializedColumns, TestSize.Level1)
{
    auto context = MockPipelineContext::GetCurrent();
    ASSERT_NE(context, nullptr);
    struct RestoreTheme {
        RefPtr<PipelineContext> context;
        RefPtr<ThemeManager> theme;
        ~RestoreTheme() { context->SetThemeManager(theme); }
    } restore { context, context->GetThemeManager() };
    auto manager = AceType::MakeRefPtr<NiceMock<MockThemeManager>>();
    auto theme = MockThemeDefault::GetPickerTheme();
    ON_CALL(*manager, GetTheme(_)).WillByDefault([theme](ThemeType type) -> RefPtr<Theme> {
        return type == PickerTheme::TypeId() ? theme : nullptr;
    });
    ON_CALL(*manager, GetTheme(_, _)).WillByDefault([theme](ThemeType type, int32_t) -> RefPtr<Theme> {
        return type == PickerTheme::TypeId() ? theme : nullptr;
    });
    context->SetThemeManager(manager);
    auto node = TimePickerModelNG::CreateFrameNode(ElementRegister::GetInstance()->MakeUniqueId());
    ASSERT_NE(node, nullptr);
    auto pattern = node->GetPattern<TimePickerRowPattern>();
    ASSERT_NE(pattern, nullptr);
    node->MarkModifyDone();
    EXPECT_EQ(node->OnRecvCommandWithResult(
        R"({"cmd":"setTimePickerTime","params":{"hour":25,"minute":0,"second":0}})").errorType,
        CommandErrorType::INVALID_PARAM);
    EXPECT_TRUE(node->OnRecvCommandWithResult(
        R"({"cmd":"setTimePickerTime","params":{"hour":1,"minute":0,"second":0}})").success);
    EXPECT_EQ(pattern->GetSelectedTime().GetHour(), 1u);
}

} // namespace OHOS::Ace::NG

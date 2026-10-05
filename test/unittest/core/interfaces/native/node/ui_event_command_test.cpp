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
 *
 * Framework-level test for InjectCompositeCommandImpl (ui_event_command_processor.cpp).
 * Calls the impl directly (not via the table); uses real ElementRegister/FrameNode +
 * mock PipelineContext/TaskExecutor. Verifies return codes + backend error messages
 * (SET_ERROR_CODE_AND_MESSAGE_IN_BACKEND -> ErrorMessageManager).
 */

#include <cstring>
#include <functional>
#include <queue>
#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "base/json/json_util.h"

#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_model_ng.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_pattern.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_theme.h"
#include "core/components_ng/pattern/swiper/swiper_layout_property.h"
#include "core/components_ng/pattern/swiper/swiper_model_ng.h"
#include "core/components_ng/pattern/swiper/swiper_pattern.h"
#include "core/components_ng/pattern/tabs/tab_content_model_ng.h"
#include "core/components_ng/pattern/tabs/tabs_model_ng.h"
#include "core/components_ng/pattern/tabs/tabs_node.h"
#include "core/components_ng/pattern/text/text_pattern.h"
#include "core/components_ng/pattern/text/paragraph_manager.h"
#include "core/components_ng/pattern/text_field/text_field_model_ng.h"
#include "core/components_ng/pattern/text_field/text_field_pattern.h"
#include "core/components/scroll/scroll_bar_theme.h"
#include "core/components_ng/pattern/swiper/swiper_theme.h"
#include "core/components/tab_bar/tab_theme.h"
#include "core/components/text/text_theme.h"
#include "core/components/text_field/textfield_theme.h"
#include "core/interfaces/native/node/ui_event_command_processor.h"
#include "interfaces/native/native_type.h"
#include "interfaces/native/ui_event_injection.h"
#include "core/interfaces/native/utility/error_message_manager.h"
#include "core/pipeline/base/element_register.h"
#include "core/pipeline_ng/pipeline_context.h"
#include "test/mock/frameworks/base/thread/mock_task_executor.h"
#include "test/mock/frameworks/core/common/mock_container.h"
#include "test/mock/frameworks/core/common/mock_theme_manager.h"
#include "test/mock/frameworks/core/components_ng/render/mock_paragraph.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {
constexpr const char* V2_TEXTINPUT_TAG = "textInput";

using InjectionResult = OH_ArkUI_NativeModule_UIEventInjection_ResultCode;
constexpr auto INJECTION_SUCCESS = OH_ARKUI_NATIVE_MODULE_UI_EVENT_INJECTION_RESULT_SUCCESS;
constexpr auto TARGET_NOT_FOUND = OH_ARKUI_NATIVE_MODULE_UI_EVENT_INJECTION_RESULT_TARGET_NOT_FOUND;
constexpr auto COMMAND_NOT_SUPPORTED = OH_ARKUI_NATIVE_MODULE_UI_EVENT_INJECTION_RESULT_COMMAND_NOT_SUPPORTED;

struct CallbackState {
    int32_t count = 0;
    InjectionResult result = INJECTION_SUCCESS;
};

struct CommandCase {
    const char* input;
    const char* expected;
};

std::vector<CommandCase> GetOptionalAndBoundaryCommandCases()
{
    return {
        { R"({"type":"setText","action_info":{"value":""}})",
            R"({"cmd":"setText","params":{"value":""}})" },
        { R"({"type":"addText","action_info":{"value":""}})",
            R"({"cmd":"addText","params":{"value":""}})" },
        { R"({"type":"addText","action_info":{"value":"x","offset":-1}})",
            R"({"cmd":"addText","params":{"value":"x","offset":-1}})" },
        { R"({"type":"deleteText","action_info":{}})",
            R"({"cmd":"deleteText","params":{"start":0,"end":0}})" },
        { R"({"type":"deleteText","action_info":{"start":5,"end":2}})",
            R"({"cmd":"deleteText","params":{"start":5,"end":2}})" },
        { R"({"type":"deleteText","action_info":{"start":-1,"end":4}})",
            R"({"cmd":"deleteText","params":{"start":0,"end":4}})" },
        { R"({"type":"selectText","action_info":{"selectionStart":5,"selectionEnd":2}})",
            R"({"cmd":"selectText","selectionStart":5,"selectionEnd":2})" },
        { R"({"type":"copy","action_info":{}})", R"({"cmd":"copy"})" },
        { R"({"type":"swiperSwitch","action_info":{"direction":"forward"}})",
            R"({"cmd":"change","params":{"type":"forward"}})" },
        { R"({"type":"swiperSwitch","action_info":{"direction":"backward","index":"unused"}})",
            R"({"cmd":"change","params":{"type":"backward"}})" },
        { R"({"type":"swiperSwitch","action_info":{"direction":"index","index":-1}})",
            R"({"cmd":"change","params":{"type":"index","index":"-1"}})" },
        { R"({"type":"tabsSwitch","action_info":{"index":-1}})",
            R"({"cmd":"changeIndex","params":{"index":"-1"}})" },
    };
}

using ModifierCallback = void (*)(ArkUI_Int32, void*);

ModifierCallback ToModifierCallback(OH_ArkUI_NativeModule_UIEventInjectionCallback callback)
{
    return reinterpret_cast<ModifierCallback>(callback);
}

void RecordCompletion(InjectionResult result, void* data)
{
    if (data != nullptr) {
        auto& state = *static_cast<CallbackState*>(data);
        state.result = result;
        ++state.count;
    }
}

// Compare the command protocol by fields and types, independently of JSON serialization order.
void ExpectCommandJson(const JsonValue& actual, const JsonValue& expected)
{
    if (expected.IsObject()) {
        ASSERT_TRUE(actual.IsObject());
        int32_t expectedFields = 0;
        for (auto field = expected.GetChild(); field && field->IsValid(); field = field->GetNext()) {
            ++expectedFields;
            ASSERT_TRUE(actual.Contains(field->GetKey())) << field->GetKey();
            auto value = actual.GetValue(field->GetKey());
            ASSERT_NE(value, nullptr);
            ExpectCommandJson(*value, *field);
        }
        int32_t actualFields = 0;
        for (auto field = actual.GetChild(); field && field->IsValid(); field = field->GetNext()) {
            ++actualFields;
        }
        EXPECT_EQ(actualFields, expectedFields);
    } else if (expected.IsString()) {
        ASSERT_TRUE(actual.IsString());
        EXPECT_EQ(actual.GetString(), expected.GetString());
    } else {
        ASSERT_TRUE(expected.IsNumber());
        ASSERT_TRUE(actual.IsNumber());
        EXPECT_DOUBLE_EQ(actual.GetDouble(), expected.GetDouble());
    }
}

class TestPattern : public Pattern {
public:
    explicit TestPattern(int32_t recvResult) : recvResult_(recvResult) {}
    int32_t OnInjectionEvent(const std::string& command) override
    {
        receivedCommand_ = command;
        ++commandCount_;
        return recvResult_;
    }
    const std::string& GetReceivedCommand() const { return receivedCommand_; }
    int32_t GetCommandCount() const { return commandCount_; }

private:
    std::string receivedCommand_;
    int32_t commandCount_ = 0;
    int32_t recvResult_ = 0;
};

class SelectionParagraph : public MockParagraph {
public:
    using MockParagraph::CalcCaretMetricsByPosition;
    bool CalcCaretMetricsByPosition(int32_t extent, CaretMetricsF& metrics, TextAffinity, bool) override
    {
        // Supply glyph geometry only; TextPattern still computes the actual command selection.
        metrics = CaretMetricsF(OffsetF(extent * 10.0f, 0.0f), 20.0f);
        return true;
    }
};

class DeferTaskExecutor : public MockTaskExecutor {
public:
    bool OnPostTask(Task&& task, TaskType, uint32_t delayTime, const std::string& name,
        PriorityType = PriorityType::LOW,
        VsyncBarrierOption = VsyncBarrierOption::NO_BARRIER) const override
    {
        if (skipDelayed_ && delayTime > 0) {
            return true; // Component timers (e.g. caret blinking) are outside the command path.
        }
        return TryEnqueueTask(std::move(task), name);
    }
    void Drain()
    {
        while (!queue_.empty()) {
            auto t = std::move(queue_.front());
            queue_.pop();
            if (t) { t(); }
        }
    }
    bool RunNext()
    {
        if (queue_.empty()) {
            return false;
        }
        auto task = std::move(queue_.front());
        queue_.pop();
        task();
        return true;
    }
    bool rejectCompletion_ = false;
    bool skipDelayed_ = false;
private:
    bool TryEnqueueTask(Task&& task, const std::string& name) const
    {
        if (rejectCompletion_ && name == "UIEventInjection.CompleteCompositeCommand") {
            return false;
        }
        if (task) {
            queue_.push(std::move(task));
        }
        return true;
    }

    mutable std::queue<std::function<void()>> queue_;
};

class FailPostTaskExecutor : public MockTaskExecutor {
public:
    bool OnPostTask(Task&&, TaskType, uint32_t, const std::string&,
        PriorityType = PriorityType::LOW,
        VsyncBarrierOption = VsyncBarrierOption::NO_BARRIER) const override
    {
        return false;
    }
};

bool ErrorMessageContains(const std::string& needle)
{
    const char* msg = ErrorMessageManager::GetInstance().GetErrorMessage();
    return msg != nullptr && std::string(msg).find(needle) != std::string::npos;
}

void ResetErrorStore()
{
    ErrorMessageManager::GetInstance().SetErrorCodeAndMessage(0, "");
}
} // namespace

class UIEventCommandTest : public testing::Test {
public:
    static void SetUpTestSuite()
    {
        MockPipelineContext::SetUp();
        MockContainer::SetUp(MockPipelineContext::GetCurrent());
        auto taskExecutor = AceType::MakeRefPtr<MockTaskExecutor>();
        MockPipelineContext::GetCurrent()->SetTaskExecutor(taskExecutor);
    }
    static void TearDownTestSuite()
    {
        MockPipelineContext::GetCurrent()->SetTaskExecutor(nullptr);
        MockContainer::TearDown();
        MockPipelineContext::TearDown();
    }
    void SetUp() override
    {
        ResetErrorStore();
        // fresh synchronous executor per case (isolates the global CAS flag)
        MockPipelineContext::GetCurrent()->SetTaskExecutor(AceType::MakeRefPtr<MockTaskExecutor>());
        testInstanceId_ = MockContainer::Current()->GetInstanceId();
    }
    void TearDown() override
    {
        for (const auto& node : nodes_) {
            if (auto parent = node->GetParent()) {
                parent->RemoveChild(node);
            }
        }
        nodes_.clear();
        MockContainer::Current()->SetTaskExecutor(nullptr);
        MockPipelineContext::GetCurrent()->SetThemeManager(nullptr);
        MockPipelineContext::GetCurrent()->SetUseFlushUITasks(false);
    }
protected:
    RefPtr<DeferTaskExecutor> SetUpComponents()
    {
        MockContainer::Current()->SetUseNewPipeline();
        auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
        auto getTheme = [](ThemeType type) -> RefPtr<Theme> {
            if (type == ScrollBarTheme::TypeId()) {
                return ScrollBarTheme::Builder().Build(nullptr);
            }
            if (type == TextFieldTheme::TypeId()) {
                return TextFieldTheme::Builder().Build(nullptr);
            }
            if (type == TextTheme::TypeId()) {
                return TextTheme::Builder().Build(nullptr);
            }
            if (type == RichEditorTheme::TypeId()) {
                return RichEditorTheme::Builder().Build(nullptr);
            }
            if (type == SwiperTheme::TypeId()) {
                return SwiperTheme::Builder().Build(nullptr);
            }
            if (type == TabTheme::TypeId()) {
                return TabTheme::Builder().Build(nullptr);
            }
            return nullptr;
        };
        EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(getTheme);
        EXPECT_CALL(*themeManager, GetTheme(_, _))
            .WillRepeatedly([getTheme](ThemeType type, int32_t) { return getTheme(type); });
        MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
        auto executor = AceType::MakeRefPtr<DeferTaskExecutor>();
        executor->skipDelayed_ = true;
        MockPipelineContext::GetCurrent()->SetTaskExecutor(executor);
        MockContainer::Current()->SetTaskExecutor(executor);
        return executor;
    }

    void RunComponentCommand(const RefPtr<FrameNode>& node, const RefPtr<DeferTaskExecutor>& executor,
        const std::string& type, const std::string& action)
    {
        auto json = "{\"schemaVersion\":1,\"cmd\":{\"type\":\"" + type + "\",\"action_info\":" + action + "}}";
        CallbackState result;
        EXPECT_EQ(RunInject(node->GetId(), json, &result), ARKUI_ERROR_CODE_NO_ERROR);
        executor->Drain();
        EXPECT_EQ(result.count, 1);
        EXPECT_EQ(result.result, INJECTION_SUCCESS);
    }

    void RunTextFieldCommand(const RefPtr<FrameNode>& node, const RefPtr<DeferTaskExecutor>& executor,
        const std::string& type, const std::string& action)
    {
        const auto json = "{\"schemaVersion\":1,\"cmd\":{\"type\":\"" + type +
            "\",\"action_info\":" + action + "}}";
        CallbackState state;
        EXPECT_EQ(RunInject(node->GetId(), json, &state), ARKUI_ERROR_CODE_NO_ERROR);
        EXPECT_EQ(state.count, 0);
        EXPECT_TRUE(executor->RunNext());
        // Text effects and callback results are checked separately; neither must precede the other.
        node->GetPattern()->BeforeCreateLayoutWrapper();
        executor->Drain();
        EXPECT_EQ(state.count, 1);
        EXPECT_EQ(state.result, INJECTION_SUCCESS);
    }

    void LayoutComponent(const RefPtr<FrameNode>& node)
    {
        auto pipeline = MockPipelineContext::GetCurrent();
        pipeline->SetUseFlushUITasks(true);
        if (!node->GetParent()) {
            auto stage = pipeline->GetRootElement()->GetChildAtIndex(0);
            node->MountToParent(stage);
        }
        node->SetActive();
        node->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
        pipeline->FlushUITasks();
    }

    uint32_t CreateNode(int32_t recvResult = 10)
    {
        auto uniqueId = ElementRegister::GetInstance()->MakeUniqueId();
        auto node = FrameNode::CreateFrameNode(V2_TEXTINPUT_TAG, uniqueId,
            AceType::MakeRefPtr<TestPattern>(recvResult));
        nodes_.push_back(node);
        return uniqueId;
    }
    ArkUI_Int32 RunInject(uint32_t uniqueId, const std::string& json,
        CallbackState* slot = nullptr)
    {
        return InjectCompositeCommandImpl(testInstanceId_, uniqueId, json.c_str(),
            static_cast<ArkUI_Uint32>(json.size()),
            ToModifierCallback(RecordCompletion),
            slot);
    }
    int32_t testInstanceId_ = 0;
    std::vector<RefPtr<UINode>> nodes_;
};

static const char* ValidJson()
{
    return "{\"schemaVersion\":1,\"cmd\":{\"type\":\"setText\",\"action_info\":{\"value\":\"hi\"}}}";
}

HWTEST_F(UIEventCommandTest, T0CrossInstance001, TestSize.Level1)
{
    // Bogus instanceId: the mock container still resolves the pipeline, so the path reaches
    // the T0 probe where the node's GetInstanceId() (testInstanceId_) != 999999 -> rejected.
    auto uid = CreateNode();
    CallbackState slot;
    auto rc = InjectCompositeCommandImpl(999999, uid, ValidJson(),
        static_cast<ArkUI_Uint32>(strlen(ValidJson())),
        ToModifierCallback(RecordCompletion),
        &slot);
    EXPECT_EQ(rc, static_cast<ArkUI_Int32>(ARKUI_ERROR_CODE_NODE_NOT_FOUND));
    EXPECT_TRUE(ErrorMessageContains("node instance id does not match the specified uiContext instance"));
    EXPECT_EQ(slot.count, 0);
}

HWTEST_F(UIEventCommandTest, TaskExecutorNull001, TestSize.Level1)
{
    MockPipelineContext::GetCurrent()->SetTaskExecutor(nullptr);
    auto uid = CreateNode();
    CallbackState slot;
    auto rc = RunInject(uid, ValidJson(), &slot);
    EXPECT_EQ(rc, static_cast<ArkUI_Int32>(ARKUI_ERROR_CODE_PARAM_INVALID));
    EXPECT_EQ(slot.count, 0);
    EXPECT_TRUE(ErrorMessageContains("task executor not available"));
    MockPipelineContext::GetCurrent()->SetTaskExecutor(AceType::MakeRefPtr<MockTaskExecutor>());
    EXPECT_EQ(RunInject(uid, ValidJson(), &slot), static_cast<ArkUI_Int32>(ARKUI_ERROR_CODE_NO_ERROR));
    EXPECT_EQ(slot.count, 1);
    EXPECT_EQ(slot.result, INJECTION_SUCCESS);
}

HWTEST_F(UIEventCommandTest, InvalidJson001, TestSize.Level1)
{
    auto uid = CreateNode();
    auto rc = RunInject(uid, "not json");
    EXPECT_EQ(rc, static_cast<ArkUI_Int32>(ARKUI_ERROR_CODE_PARAM_INVALID));
    EXPECT_TRUE(ErrorMessageContains("invalid json payload"));
}

HWTEST_F(UIEventCommandTest, MissingCmdObject001, TestSize.Level1)
{
    auto uid = CreateNode();
    auto rc = RunInject(uid, R"({"action_info":{}})");
    EXPECT_EQ(rc, static_cast<ArkUI_Int32>(ARKUI_ERROR_CODE_PARAM_INVALID));
    EXPECT_TRUE(ErrorMessageContains("missing cmd object"));
}

HWTEST_F(UIEventCommandTest, EmptyCommandType001, TestSize.Level1)
{
    auto uid = CreateNode();
    auto rc = RunInject(uid, R"({"cmd":{"type":"","action_info":{}}})");
    EXPECT_EQ(rc, static_cast<ArkUI_Int32>(ARKUI_ERROR_CODE_PARAM_INVALID));
    EXPECT_TRUE(ErrorMessageContains("missing cmd.type"));
}

HWTEST_F(UIEventCommandTest, MissingActionInfo001, TestSize.Level1)
{
    auto uid = CreateNode();
    auto rc = RunInject(uid, R"({"cmd":{"type":"setText"}})");
    EXPECT_EQ(rc, static_cast<ArkUI_Int32>(ARKUI_ERROR_CODE_PARAM_INVALID));
    EXPECT_TRUE(ErrorMessageContains("missing action_info object"));
}

HWTEST_F(UIEventCommandTest, UnknownType001, TestSize.Level1)
{
    auto uid = CreateNode();
    auto rc = RunInject(uid, "{\"cmd\":{\"type\":\"nope\",\"action_info\":{}}}");
    EXPECT_EQ(rc, static_cast<ArkUI_Int32>(ARKUI_ERROR_CODE_PARAM_INVALID));
    EXPECT_TRUE(ErrorMessageContains("unknown command type"));
}

HWTEST_F(UIEventCommandTest, InvalidCommandFieldsRejectedBeforeQueue001, TestSize.Level1)
{
    auto executor = AceType::MakeRefPtr<DeferTaskExecutor>();
    MockPipelineContext::GetCurrent()->SetTaskExecutor(executor);
    const auto uid = CreateNode();
    const std::vector<std::string> commands = {
        R"({"type":"setText","action_info":{}})",
        R"({"type":"setText","action_info":{"value":1}})",
        R"({"type":"addText","action_info":{"value":null}})",
        R"({"type":"addText","action_info":{"value":"x","offset":"1"}})",
        R"({"type":"addText","action_info":{"value":"x","offset":1.5}})",
        R"({"type":"deleteText","action_info":{"start":false}})",
        R"({"type":"deleteText","action_info":{"end":[]}})",
        R"({"type":"selectText","action_info":{"selectionStart":0}})",
        R"({"type":"selectText","action_info":{"selectionStart":"0","selectionEnd":1}})",
        R"({"type":"selectText","action_info":{"selectionStart":0,"selectionEnd":null}})",
        R"({"type":"swiperSwitch","action_info":{"direction":1}})",
        R"({"type":"swiperSwitch","action_info":{"direction":"sideways"}})",
        R"({"type":"swiperSwitch","action_info":{"direction":"index"}})",
        R"({"type":"swiperSwitch","action_info":{"direction":"index","index":"1"}})",
        R"({"type":"tabsSwitch","action_info":{}})",
        R"({"type":"tabsSwitch","action_info":{"index":true}})",
        R"({"type":"tabsSwitch","action_info":{"index":2147483648}})",
        R"({"type":"tabsSwitch","action_info":{"index":-2147483649}})",
    };
    for (const auto& command : commands) {
        SCOPED_TRACE(command);
        const auto json = "{\"schemaVersion\":1,\"cmd\":" + command + "}";
        CallbackState result;
        EXPECT_EQ(RunInject(uid, json, &result), ARKUI_ERROR_CODE_PARAM_INVALID);
        EXPECT_FALSE(executor->RunNext()); // A rejected command must not execute or post a callback.
        executor->Drain();
        EXPECT_EQ(result.count, 0);
    }
    EXPECT_EQ(RunInject(uid, ValidJson()), ARKUI_ERROR_CODE_NO_ERROR);
    executor->Drain(); // Invalid input must not leave the process occupied.
}

// Validate only the new converter/dispatch protocol; TestPattern does not execute component branches.
HWTEST_F(UIEventCommandTest, OptionalFieldsAndComponentBoundariesAccepted001, TestSize.Level1)
{
    auto executor = AceType::MakeRefPtr<DeferTaskExecutor>();
    MockPipelineContext::GetCurrent()->SetTaskExecutor(executor);
    const auto uid = ElementRegister::GetInstance()->MakeUniqueId();
    auto pattern = AceType::MakeRefPtr<TestPattern>(10);
    auto node = FrameNode::CreateFrameNode(V2_TEXTINPUT_TAG, uid, pattern);
    ASSERT_NE(node, nullptr);
    nodes_.push_back(node);
    const auto commands = GetOptionalAndBoundaryCommandCases();
    int32_t dispatched = 0;
    for (const auto& command : commands) {
        SCOPED_TRACE(command.input);
        const auto json = std::string("{\"schemaVersion\":1,\"cmd\":") + command.input + "}";
        CallbackState state;
        EXPECT_EQ(RunInject(uid, json, &state), ARKUI_ERROR_CODE_NO_ERROR);
        EXPECT_EQ(state.count, 0);
        EXPECT_EQ(pattern->GetCommandCount(), dispatched);
        executor->Drain();
        ++dispatched;
        EXPECT_EQ(pattern->GetCommandCount(), dispatched);
        EXPECT_EQ(state.count, 1);
        EXPECT_EQ(state.result, INJECTION_SUCCESS);
        auto actual = JsonUtil::ParseJsonString(pattern->GetReceivedCommand());
        auto expected = JsonUtil::ParseJsonString(command.expected);
        ASSERT_NE(actual, nullptr);
        ASSERT_NE(expected, nullptr);
        ASSERT_TRUE(actual->IsValid());
        ASSERT_TRUE(expected->IsValid());
        ExpectCommandJson(*actual, *expected);
    }
}

HWTEST_F(UIEventCommandTest, CommandPayloadSnapshot001, TestSize.Level1)
{
    auto executor = AceType::MakeRefPtr<DeferTaskExecutor>();
    MockPipelineContext::GetCurrent()->SetTaskExecutor(executor);
    const auto uid = ElementRegister::GetInstance()->MakeUniqueId();
    auto pattern = AceType::MakeRefPtr<TestPattern>(10);
    auto node = FrameNode::CreateFrameNode(V2_TEXTINPUT_TAG, uid, pattern);
    ASSERT_NE(node, nullptr);
    nodes_.push_back(node);
    CallbackState state;
    {
        std::string input = R"({"schemaVersion":1,"cmd":{"type":"setText","action_info":{"value":"original"}}})";
        EXPECT_EQ(RunInject(uid, input, &state), ARKUI_ERROR_CODE_NO_ERROR);
        input.assign(input.size(), 'x'); // Caller storage is changed, then destroyed before dispatch.
    }
    EXPECT_EQ(pattern->GetCommandCount(), 0);
    EXPECT_EQ(state.count, 0);
    EXPECT_TRUE(executor->RunNext());
    EXPECT_EQ(pattern->GetCommandCount(), 1);
    EXPECT_EQ(state.count, 0);
    EXPECT_TRUE(executor->RunNext());
    EXPECT_EQ(state.count, 1);
    EXPECT_EQ(state.result, INJECTION_SUCCESS);
    EXPECT_FALSE(executor->RunNext());
    auto actual = JsonUtil::ParseJsonString(pattern->GetReceivedCommand());
    auto expected = JsonUtil::ParseJsonString(R"({"cmd":"setText","params":{"value":"original"}})");
    ASSERT_NE(actual, nullptr);
    ASSERT_NE(expected, nullptr);
    ExpectCommandJson(*actual, *expected);
}

HWTEST_F(UIEventCommandTest, T0NodeNotFound001, TestSize.Level1)
{
    auto rc = RunInject(999999, ValidJson());
    EXPECT_EQ(rc, static_cast<ArkUI_Int32>(ARKUI_ERROR_CODE_NODE_NOT_FOUND));
    EXPECT_TRUE(ErrorMessageContains("uniqueId does not resolve to a node"));
}

HWTEST_F(UIEventCommandTest, T0HitQueued001, TestSize.Level1)
{
    auto uid = CreateNode();
    auto rc = RunInject(uid, ValidJson());
    EXPECT_EQ(rc, static_cast<ArkUI_Int32>(ARKUI_ERROR_CODE_NO_ERROR));
}

HWTEST_F(UIEventCommandTest, CasReject001, TestSize.Level1)
{
    auto defer = AceType::MakeRefPtr<DeferTaskExecutor>();
    MockPipelineContext::GetCurrent()->SetTaskExecutor(defer);
    auto uid = CreateNode();
    CallbackState slot1;
    CallbackState slot2;
    auto r1 = RunInject(uid, ValidJson(), &slot1);
    EXPECT_EQ(r1, static_cast<ArkUI_Int32>(ARKUI_ERROR_CODE_NO_ERROR));
    auto r2 = RunInject(uid, ValidJson(), &slot2);
    EXPECT_EQ(r2, static_cast<ArkUI_Int32>(ARKUI_ERROR_CODE_COMMAND_UNFINISHED));
    EXPECT_TRUE(ErrorMessageContains("a previous composite command is still in-flight"));
    EXPECT_EQ(slot1.count, 0);
    EXPECT_EQ(slot2.count, 0);
    defer->Drain();
    EXPECT_EQ(slot1.count, 1);
    EXPECT_EQ(slot1.result, INJECTION_SUCCESS);
    EXPECT_EQ(slot2.count, 0);
}

HWTEST_F(UIEventCommandTest, T3NodeDestroyed001, TestSize.Level1)
{
    auto defer = AceType::MakeRefPtr<DeferTaskExecutor>();
    MockPipelineContext::GetCurrent()->SetTaskExecutor(defer);
    auto uid = CreateNode();
    CallbackState slot;
    auto r = RunInject(uid, ValidJson(), &slot);
    EXPECT_EQ(r, static_cast<ArkUI_Int32>(ARKUI_ERROR_CODE_NO_ERROR));
    nodes_.clear(); // drop strong ref -> ElementRegister weak ref expires at T3
    defer->Drain();
    EXPECT_EQ(slot.count, 1);
    EXPECT_EQ(slot.result, TARGET_NOT_FOUND);
    EXPECT_TRUE(ErrorMessageContains("node was destroyed after dispatch"));
}

HWTEST_F(UIEventCommandTest, NodeInstanceIdMismatchAfterDispatch001, TestSize.Level1)
{
    auto defer = AceType::MakeRefPtr<DeferTaskExecutor>();
    MockPipelineContext::GetCurrent()->SetTaskExecutor(defer);
    auto uid = CreateNode();
    CallbackState slot;
    auto r = RunInject(uid, ValidJson(), &slot);
    EXPECT_EQ(r, static_cast<ArkUI_Int32>(ARKUI_ERROR_CODE_NO_ERROR));
    ASSERT_FALSE(nodes_.empty());
    auto otherPipeline = AceType::MakeRefPtr<MockPipelineContext>();
    otherPipeline->SetThisInstanceId(999);
    otherPipeline->SetTaskExecutor(defer);
    nodes_[0]->AttachContext(AceType::RawPtr(otherPipeline));
    defer->Drain();
    EXPECT_EQ(slot.count, 1);
    EXPECT_EQ(slot.result, TARGET_NOT_FOUND);
    EXPECT_TRUE(ErrorMessageContains("node instance id changed after dispatch"));
}

HWTEST_F(UIEventCommandTest, OnRecvCommandSuccess001, TestSize.Level1)
{
    auto uid = CreateNode(10); // RET_SUCCESS -> callback SUCCESS.
    CallbackState slot;
    auto r = RunInject(uid, ValidJson(), &slot);
    EXPECT_EQ(r, static_cast<ArkUI_Int32>(ARKUI_ERROR_CODE_NO_ERROR));
    EXPECT_EQ(slot.count, 1);
    EXPECT_EQ(slot.result, INJECTION_SUCCESS);
}

HWTEST_F(UIEventCommandTest, OnRecvCommandNotSupported001, TestSize.Level1)
{
    auto uid = CreateNode(11); // RET_FAILED -> callback COMMAND_NOT_SUPPORTED.
    CallbackState slot;
    auto r = RunInject(uid, ValidJson(), &slot);
    EXPECT_EQ(r, static_cast<ArkUI_Int32>(ARKUI_ERROR_CODE_NO_ERROR));
    EXPECT_EQ(slot.count, 1);
    EXPECT_EQ(slot.result, COMMAND_NOT_SUPPORTED);
    EXPECT_TRUE(ErrorMessageContains("command not supported by the node"));
}

HWTEST_F(UIEventCommandTest, OnRecvCommandInternalError001, TestSize.Level1)
{
    auto uid = CreateNode(99); // Other execution failures also map to callback COMMAND_NOT_SUPPORTED.
    CallbackState slot;
    auto r = RunInject(uid, ValidJson(), &slot);
    EXPECT_EQ(r, static_cast<ArkUI_Int32>(ARKUI_ERROR_CODE_NO_ERROR));
    EXPECT_EQ(slot.count, 1);
    EXPECT_EQ(slot.result, COMMAND_NOT_SUPPORTED);
    EXPECT_TRUE(ErrorMessageContains("OnRecvCommand returned an internal error"));
}

HWTEST_F(UIEventCommandTest, NullCallbackRejected001, TestSize.Level1)
{
    auto uid = CreateNode(10);
    auto r = InjectCompositeCommandImpl(testInstanceId_, uid, ValidJson(),
        static_cast<ArkUI_Uint32>(strlen(ValidJson())), nullptr, nullptr);
    EXPECT_EQ(r, static_cast<ArkUI_Int32>(ARKUI_ERROR_CODE_PARAM_INVALID));
    EXPECT_TRUE(ErrorMessageContains("callback must not be null"));
}

HWTEST_F(UIEventCommandTest, PostTaskFail001, TestSize.Level1)
{
    MockPipelineContext::GetCurrent()->SetTaskExecutor(
        AceType::MakeRefPtr<FailPostTaskExecutor>());
    auto uid = CreateNode();
    CallbackState slot;
    auto rc = RunInject(uid, ValidJson(), &slot);
    EXPECT_EQ(rc, static_cast<ArkUI_Int32>(ARKUI_ERROR_CODE_PARAM_INVALID));
    EXPECT_EQ(slot.count, 0);
    EXPECT_TRUE(ErrorMessageContains("failed to post composite command to UI thread"));
    MockPipelineContext::GetCurrent()->SetTaskExecutor(AceType::MakeRefPtr<MockTaskExecutor>());
    EXPECT_EQ(RunInject(uid, ValidJson(), &slot), static_cast<ArkUI_Int32>(ARKUI_ERROR_CODE_NO_ERROR));
    EXPECT_EQ(slot.count, 1);
    EXPECT_EQ(slot.result, INJECTION_SUCCESS);
}

HWTEST_F(UIEventCommandTest, CompletionQueueAndReentry001, TestSize.Level1)
{
    auto executor = AceType::MakeRefPtr<DeferTaskExecutor>();
    MockPipelineContext::GetCurrent()->SetTaskExecutor(executor);
    auto uid = CreateNode();
    struct CompletionState {
        int32_t instanceId;
        uint32_t uniqueId;
        int32_t callbacks = 0;
        int32_t nextResult = -1;
        CallbackState nextCompletion {};
    } state { testInstanceId_, uid };
    auto rc = InjectCompositeCommandImpl(testInstanceId_, uid, ValidJson(), strlen(ValidJson()),
        ToModifierCallback([](InjectionResult code, void* data) {
            auto& state = *static_cast<CompletionState*>(data);
            EXPECT_EQ(static_cast<int>(code), static_cast<int>(INJECTION_SUCCESS));
            ++state.callbacks;
            state.nextResult = InjectCompositeCommandImpl(state.instanceId, state.uniqueId, ValidJson(),
                strlen(ValidJson()), ToModifierCallback(RecordCompletion), &state.nextCompletion);
        }), &state);
    EXPECT_EQ(rc, ARKUI_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(state.callbacks, 0);
    EXPECT_TRUE(executor->RunNext()); // Execute command; completion must remain queued.
    EXPECT_EQ(state.callbacks, 0);
    EXPECT_EQ(RunInject(uid, ValidJson()), ARKUI_ERROR_CODE_COMMAND_UNFINISHED);
    EXPECT_TRUE(executor->RunNext()); // Release ownership before entering callback.
    EXPECT_EQ(state.callbacks, 1);
    EXPECT_EQ(state.nextResult, ARKUI_ERROR_CODE_NO_ERROR);
    // Returning from the old callback must not release the new command's ownership.
    EXPECT_EQ(RunInject(uid, ValidJson()), ARKUI_ERROR_CODE_COMMAND_UNFINISHED);
    executor->Drain();
    EXPECT_EQ(state.nextCompletion.count, 1);
    EXPECT_EQ(state.nextCompletion.result, INJECTION_SUCCESS);
    EXPECT_EQ(RunInject(uid, ValidJson()), ARKUI_ERROR_CODE_NO_ERROR);
    executor->Drain();
}

HWTEST_F(UIEventCommandTest, ContextLostBeforeExecution001, TestSize.Level1)
{
    auto executor = AceType::MakeRefPtr<DeferTaskExecutor>();
    auto pipeline = MockPipelineContext::GetCurrent();
    pipeline->SetTaskExecutor(executor);
    auto uid = CreateNode();
    CallbackState result;
    EXPECT_EQ(RunInject(uid, ValidJson(), &result), ARKUI_ERROR_CODE_NO_ERROR);
    MockPipelineContext::pipeline_ = nullptr;
    EXPECT_TRUE(executor->RunNext());
    EXPECT_EQ(result.count, 0); // Error completion must also be posted.
    MockPipelineContext::pipeline_ = pipeline;
    EXPECT_TRUE(executor->RunNext());
    EXPECT_EQ(result.count, 1);
    EXPECT_EQ(result.result, TARGET_NOT_FOUND);
    EXPECT_EQ(RunInject(uid, ValidJson()), ARKUI_ERROR_CODE_NO_ERROR);
    executor->Drain();
}

HWTEST_F(UIEventCommandTest, CompletionPostFailureAndReentry001, TestSize.Level1)
{
    auto executor = AceType::MakeRefPtr<DeferTaskExecutor>();
    executor->rejectCompletion_ = true;
    MockPipelineContext::GetCurrent()->SetTaskExecutor(executor);
    auto uid = CreateNode();
    int callbacks = 0;
    CallbackState nextResult;
    std::function<void(ArkUI_Int32)> callback = [&](ArkUI_Int32 result) {
        ++callbacks;
        EXPECT_EQ(static_cast<int>(result), static_cast<int>(INJECTION_SUCCESS));
        EXPECT_EQ(RunInject(uid, ValidJson(), &nextResult), ARKUI_ERROR_CODE_NO_ERROR);
    };
    EXPECT_EQ(InjectCompositeCommandImpl(testInstanceId_, uid, ValidJson(), strlen(ValidJson()),
        ToModifierCallback([](InjectionResult result, void* data) {
            (*static_cast<std::function<void(ArkUI_Int32)>*>(data))(result);
        }), &callback), ARKUI_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(callbacks, 0);
    EXPECT_TRUE(executor->RunNext()); // Rejected completion runs on this call stack.
    EXPECT_EQ(callbacks, 1);
    EXPECT_EQ(RunInject(uid, ValidJson()), ARKUI_ERROR_CODE_COMMAND_UNFINISHED);
    executor->Drain();
    EXPECT_EQ(callbacks, 1);
    EXPECT_EQ(nextResult.count, 1);
    EXPECT_EQ(nextResult.result, INJECTION_SUCCESS);
    EXPECT_FALSE(executor->RunNext());
}

HWTEST_F(UIEventCommandTest, TextFieldCommandEffects001, TestSize.Level1)
{
    auto executor = SetUpComponents();
    for (bool multiline : { false, true }) {
        auto id = ElementRegister::GetInstance()->MakeUniqueId();
        auto node = multiline ? TextFieldModelNG::CreateTextAreaNode(id, u"", u"original") :
            TextFieldModelNG::CreateTextInputNode(id, u"", u"original");
        ASSERT_NE(node, nullptr);
        nodes_.push_back(node);
        auto pattern = node->GetPattern<TextFieldPattern>();
        ASSERT_NE(pattern, nullptr);
        node->MarkModifyDone();
        executor->Drain();
        // The command uses real selection logic; the headless test supplies glyph geometry.
        pattern->GetTextSelectController()->UpdateParagraph(AceType::MakeRefPtr<SelectionParagraph>());
        pattern->GetTextSelectController()->UpdateContentRect(RectF(0, 0, 300, 100));
        auto run = [&](const std::string& type, const std::string& action) {
            RunTextFieldCommand(node, executor, type, action);
        };
        run("setText", R"({"value":"abcdef"})");
        EXPECT_EQ(pattern->GetTextUtf16Value(), u"abcdef");
        run("addText", R"({"value":"XY","offset":2})");
        EXPECT_EQ(pattern->GetTextUtf16Value(), u"abXYcdef");
        run("deleteText", R"({"start":2,"end":4})");
        EXPECT_EQ(pattern->GetTextUtf16Value(), u"abcdef");
        run("selectText", R"({"selectionStart":1,"selectionEnd":4})");
        EXPECT_EQ(pattern->GetTextSelectController()->GetStartIndex(), 1);
        EXPECT_EQ(pattern->GetTextSelectController()->GetEndIndex(), 4);
        run("copy", "{}");
        std::string copied;
        auto clipboard = pattern->GetClipboard();
        ASSERT_NE(clipboard, nullptr);
        clipboard->GetData([&copied](const std::string& text) { copied = text; }, true);
        EXPECT_EQ(copied, "bcd");
    }
}

HWTEST_F(UIEventCommandTest, TextSelectionEffects001, TestSize.Level1)
{
    auto executor = SetUpComponents();
    auto id = ElementRegister::GetInstance()->MakeUniqueId();
    auto node = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, id, AceType::MakeRefPtr<TextPattern>());
    nodes_.push_back(node);
    auto property = node->GetLayoutProperty<TextLayoutProperty>();
    property->UpdateContent(u"abcdef");
    property->UpdateCopyOption(CopyOptions::InApp);
    node->GetGeometryNode()->SetFrameSize(SizeF(300, 100));
    node->MarkModifyDone();
    executor->Drain();
    auto pattern = node->GetPattern<TextPattern>();
    auto paragraph = AceType::MakeRefPtr<SelectionParagraph>();
    ASSERT_NE(pattern->GetParagraphManager(), nullptr);
    pattern->GetParagraphManager()->AddParagraph({ .paragraph = paragraph, .start = 0, .end = 6 });
    RunComponentCommand(node, executor, "selectText", R"({"selectionStart":1,"selectionEnd":4})");
    EXPECT_EQ(pattern->GetTextSelector().GetTextStart(), 1);
    EXPECT_EQ(pattern->GetTextSelector().GetTextEnd(), 4);
}

HWTEST_F(UIEventCommandTest, RichEditorCommandEffects001, TestSize.Level1)
{
    auto executor = SetUpComponents();
    RichEditorModelNG model;
    ViewStackProcessor::GetInstance()->StartGetAccessRecordingFor(
        ElementRegister::GetInstance()->MakeUniqueId());
    model.Create();
    auto node = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ViewStackProcessor::GetInstance()->StopGetAccessRecording();
    ASSERT_NE(node, nullptr);
    nodes_.push_back(node);
    node->MarkModifyDone();
    executor->Drain();
    RunComponentCommand(node, executor, "addText", R"({"value":"abcdef","offset":0})");
    auto pattern = node->GetPattern<RichEditorPattern>();
    EXPECT_EQ(pattern->GetTextContentLength(), 6);
    RunComponentCommand(node, executor, "deleteText", R"({"start":2,"end":4})");
    EXPECT_EQ(pattern->GetTextContentLength(), 4);
    std::u16string content;
    for (const auto& span : pattern->GetSpansInfoByRange(0, 4).GetSelection().resultObjects) {
        content += span.valueString;
    }
    EXPECT_EQ(content, u"abef");
}

HWTEST_F(UIEventCommandTest, SwiperSwitchEffects001, TestSize.Level1)
{
    auto executor = SetUpComponents();
    auto node = SwiperModelNG::CreateFrameNode(ElementRegister::GetInstance()->MakeUniqueId());
    nodes_.push_back(node);
    SwiperModelNG::SetDuration(AceType::RawPtr(node), 0);
    SwiperModelNG::SetShowIndicator(AceType::RawPtr(node), false);
    node->GetLayoutProperty()->UpdateUserDefinedIdealSize(CalcSize(CalcLength(300), CalcLength(200)));
    for (int i = 0; i < 3; ++i) {
        auto page = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG,
            ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
        page->MountToParent(node);
    }
    node->MarkModifyDone();
    LayoutComponent(node);
    executor->Drain();
    ASSERT_EQ(node->GetPattern<SwiperPattern>()->TotalCount(), 3);
    ASSERT_GT(node->GetGeometryNode()->GetFrameSize().Width(), 0);
    RunComponentCommand(node, executor, "swiperSwitch", R"({"direction":"index","index":1})");
    LayoutComponent(node);
    EXPECT_EQ(node->GetPattern<SwiperPattern>()->GetCurrentIndex(), 1);
}

HWTEST_F(UIEventCommandTest, TabsSwitchEffects001, TestSize.Level1)
{
    auto executor = SetUpComponents();
    TabsModelNG model;
    ViewStackProcessor::GetInstance()->StartGetAccessRecordingFor(
        ElementRegister::GetInstance()->MakeUniqueId());
    model.Create(BarPosition::START, 0, nullptr);
    ViewStackProcessor::GetInstance()->StopGetAccessRecording();
    model.SetAnimationDuration(0);
    for (int i = 0; i < 3; ++i) {
        TabContentModelNG content;
        ViewStackProcessor::GetInstance()->StartGetAccessRecordingFor(
            ElementRegister::GetInstance()->MakeUniqueId());
        content.Create(nullptr);
        ViewStackProcessor::GetInstance()->Pop();
        ViewStackProcessor::GetInstance()->StopGetAccessRecording();
    }
    model.Pop();
    auto node = AceType::DynamicCast<TabsNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(node, nullptr);
    nodes_.push_back(node);
    node->GetLayoutProperty()->UpdateUserDefinedIdealSize(CalcSize(CalcLength(300), CalcLength(200)));
    node->MarkModifyDone();
    LayoutComponent(node);
    executor->Drain();
    RunComponentCommand(node, executor, "tabsSwitch", R"({"index":1})");
    LayoutComponent(node);
    auto swiper = AceType::DynamicCast<FrameNode>(node->GetTabs());
    ASSERT_NE(swiper, nullptr);
    EXPECT_EQ(swiper->GetPattern<SwiperPattern>()->GetCurrentIndex(), 1);
}

// AC-10: Callback timing — async dispatch, single callback, no re-entrant duplicate.
HWTEST_F(UIEventCommandTest, CallbackTimingAsyncSingle001, TestSize.Level1)
{
    auto executor = AceType::MakeRefPtr<DeferTaskExecutor>();
    MockPipelineContext::GetCurrent()->SetTaskExecutor(executor);
    auto uid = CreateNode();
    CallbackState state;
    EXPECT_EQ(RunInject(uid, ValidJson(), &state), ARKUI_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(state.count, 0);
    EXPECT_TRUE(executor->RunNext()); // Command execution only posts the completion task.
    EXPECT_EQ(state.count, 0);
    EXPECT_TRUE(executor->RunNext()); // Completion is delivered by a separate UI task.
    EXPECT_EQ(state.count, 1);
    EXPECT_EQ(state.result, INJECTION_SUCCESS);
    EXPECT_FALSE(executor->RunNext());
    EXPECT_EQ(state.count, 1);
}

// AC-10: Multiple sequential commands — each gets its own callback, IDs independent.
HWTEST_F(UIEventCommandTest, SequentialCommandsIndependent001, TestSize.Level1)
{
    auto uid = CreateNode();
    for (int i = 0; i < 3; ++i) {
        CallbackState result;
        EXPECT_EQ(RunInject(uid, ValidJson(), &result), static_cast<ArkUI_Int32>(ARKUI_ERROR_CODE_NO_ERROR));
        EXPECT_EQ(result.count, 1);
        EXPECT_EQ(result.result, INJECTION_SUCCESS);
    }
}

// AC-14: setText replaces entire content; addText at offset inserts.
HWTEST_F(UIEventCommandTest, TextFieldSetAndAddText001, TestSize.Level1)
{
    auto executor = SetUpComponents();
    auto id = ElementRegister::GetInstance()->MakeUniqueId();
    auto node = TextFieldModelNG::CreateTextInputNode(id, u"", u"hello");
    nodes_.push_back(node);
    auto pattern = node->GetPattern<TextFieldPattern>();
    node->MarkModifyDone();
    executor->Drain();
    pattern->GetTextSelectController()->UpdateParagraph(AceType::MakeRefPtr<SelectionParagraph>());
    pattern->GetTextSelectController()->UpdateContentRect(RectF(0, 0, 300, 100));
    RunTextFieldCommand(node, executor, "setText", R"({"value":"world"})");
    EXPECT_EQ(pattern->GetTextUtf16Value(), u"world");
    RunTextFieldCommand(node, executor, "addText", R"({"value":"XY","offset":1})");
    EXPECT_EQ(pattern->GetTextUtf16Value(), u"wXYorld");
}

// AC-14: deleteText with start>end swaps endpoints (D-16).
HWTEST_F(UIEventCommandTest, DeleteTextSwappedEndpoints001, TestSize.Level1)
{
    auto executor = SetUpComponents();
    auto id = ElementRegister::GetInstance()->MakeUniqueId();
    auto node = TextFieldModelNG::CreateTextInputNode(id, u"", u"abcdef");
    nodes_.push_back(node);
    auto pattern = node->GetPattern<TextFieldPattern>();
    node->MarkModifyDone();
    executor->Drain();
    pattern->GetTextSelectController()->UpdateParagraph(AceType::MakeRefPtr<SelectionParagraph>());
    pattern->GetTextSelectController()->UpdateContentRect(RectF(0, 0, 300, 100));
    RunTextFieldCommand(node, executor, "deleteText", R"({"start":4,"end":2})");
    EXPECT_EQ(pattern->GetTextUtf16Value(), u"abef");
}

// AC-14: addText with negative offset inserts at beginning (D-15).
HWTEST_F(UIEventCommandTest, AddTextNegativeOffset001, TestSize.Level1)
{
    auto executor = SetUpComponents();
    auto id = ElementRegister::GetInstance()->MakeUniqueId();
    auto node = TextFieldModelNG::CreateTextInputNode(id, u"", u"abc");
    nodes_.push_back(node);
    auto pattern = node->GetPattern<TextFieldPattern>();
    node->MarkModifyDone();
    executor->Drain();
    pattern->GetTextSelectController()->UpdateParagraph(AceType::MakeRefPtr<SelectionParagraph>());
    pattern->GetTextSelectController()->UpdateContentRect(RectF(0, 0, 300, 100));
    RunTextFieldCommand(node, executor, "addText", R"({"value":"XY","offset":-1})");
    EXPECT_EQ(pattern->GetTextUtf16Value(), u"XYabc");
}

// AC-15: Non-looping Swiper at its last page reports COMMAND_NOT_SUPPORTED (D-13/D-20).
HWTEST_F(UIEventCommandTest, SwiperForwardBoundary001, TestSize.Level1)
{
    auto executor = SetUpComponents();
    auto node = SwiperModelNG::CreateFrameNode(ElementRegister::GetInstance()->MakeUniqueId());
    ASSERT_NE(node, nullptr);
    nodes_.push_back(node);
    SwiperModelNG::SetLoop(AceType::RawPtr(node), false);
    SwiperModelNG::SetDisplayCount(AceType::RawPtr(node), 1);
    SwiperModelNG::SetDisplayMode(AceType::RawPtr(node), SwiperDisplayMode::STRETCH);
    SwiperModelNG::SetDisableSwipe(AceType::RawPtr(node), false);
    SwiperModelNG::SetDuration(AceType::RawPtr(node), 0);
    SwiperModelNG::SetShowIndicator(AceType::RawPtr(node), false);
    node->GetLayoutProperty()->UpdateUserDefinedIdealSize(CalcSize(CalcLength(300), CalcLength(200)));
    for (int i = 0; i < 2; ++i) {
        auto page = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG,
            ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
        page->MountToParent(node);
    }
    node->MarkModifyDone();
    LayoutComponent(node);
    executor->Drain();
    auto pattern = node->GetPattern<SwiperPattern>();
    ASSERT_NE(pattern, nullptr);
    ASSERT_EQ(pattern->TotalCount(), 2);
    ASSERT_FALSE(pattern->IsLoop());
    ASSERT_FALSE(pattern->IsAutoLinear());
    auto layoutProperty = node->GetLayoutProperty<SwiperLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    ASSERT_FALSE(layoutProperty->GetDisableSwipeValue(false));
    ASSERT_EQ(layoutProperty->GetDisplayCountValue(1), 1);
    RunComponentCommand(node, executor, "swiperSwitch", R"({"direction":"index","index":1})");
    LayoutComponent(node);
    ASSERT_EQ(pattern->GetCurrentIndex(), 1);
    CallbackState result;
    auto json = "{\"schemaVersion\":1,\"cmd\":{\"type\":\"swiperSwitch\",\"action_info\":{\"direction\":\"forward\"}}}";
    EXPECT_EQ(RunInject(node->GetId(), json, &result), ARKUI_ERROR_CODE_NO_ERROR);
    executor->Drain();
    EXPECT_EQ(result.count, 1);
    EXPECT_EQ(result.result, COMMAND_NOT_SUPPORTED);
    LayoutComponent(node);
    EXPECT_EQ(pattern->GetCurrentIndex(), 1);
}

// AC-15: Tabs out-of-range index clamps to 0.
HWTEST_F(UIEventCommandTest, TabsOutOfRangeIndex001, TestSize.Level1)
{
    auto executor = SetUpComponents();
    TabsModelNG model;
    ViewStackProcessor::GetInstance()->StartGetAccessRecordingFor(
        ElementRegister::GetInstance()->MakeUniqueId());
    model.Create(BarPosition::START, 0, nullptr);
    ViewStackProcessor::GetInstance()->StopGetAccessRecording();
    model.SetAnimationDuration(0);
    for (int i = 0; i < 3; ++i) {
        TabContentModelNG content;
        ViewStackProcessor::GetInstance()->StartGetAccessRecordingFor(
            ElementRegister::GetInstance()->MakeUniqueId());
        content.Create(nullptr);
        ViewStackProcessor::GetInstance()->Pop();
        ViewStackProcessor::GetInstance()->StopGetAccessRecording();
    }
    model.Pop();
    auto node = AceType::DynamicCast<TabsNode>(ViewStackProcessor::GetInstance()->Finish());
    nodes_.push_back(node);
    node->GetLayoutProperty()->UpdateUserDefinedIdealSize(CalcSize(CalcLength(300), CalcLength(200)));
    node->MarkModifyDone();
    LayoutComponent(node);
    executor->Drain();
    RunComponentCommand(node, executor, "tabsSwitch", R"({"index":99})");
    LayoutComponent(node);
    auto swiper = AceType::DynamicCast<FrameNode>(node->GetTabs());
    ASSERT_NE(swiper, nullptr);
    EXPECT_EQ(swiper->GetPattern<SwiperPattern>()->GetCurrentIndex(), 0);
}

// AC-16: Context destroyed after accept — callback receives TARGET_NOT_FOUND.
HWTEST_F(UIEventCommandTest, ContextDestroyedAfterAccept001, TestSize.Level1)
{
    auto executor = AceType::MakeRefPtr<DeferTaskExecutor>();
    auto pipeline = MockPipelineContext::GetCurrent();
    pipeline->SetTaskExecutor(executor);
    auto uid = CreateNode();
    CallbackState result;
    EXPECT_EQ(RunInject(uid, ValidJson(), &result), static_cast<ArkUI_Int32>(ARKUI_ERROR_CODE_NO_ERROR));
    EXPECT_EQ(result.count, 0);
    MockPipelineContext::pipeline_ = nullptr;
    executor->Drain();
    EXPECT_EQ(result.count, 1);
    EXPECT_EQ(result.result, TARGET_NOT_FOUND);
    MockPipelineContext::pipeline_ = pipeline;
}

// AC-16: Context destroyed after accept — new command rejected with CONTEXT_INVALID.
HWTEST_F(UIEventCommandTest, ContextDestroyedNewCommandRejected001, TestSize.Level1)
{
    auto executor = AceType::MakeRefPtr<DeferTaskExecutor>();
    auto pipeline = MockPipelineContext::GetCurrent();
    pipeline->SetTaskExecutor(executor);
    auto uid = CreateNode();
    MockPipelineContext::pipeline_ = nullptr;
    auto rc = RunInject(uid, ValidJson());
    EXPECT_EQ(rc, static_cast<ArkUI_Int32>(ARKUI_ERROR_CODE_UI_CONTEXT_INVALID));
    EXPECT_TRUE(ErrorMessageContains("pipeline context not found for instance"));
    MockPipelineContext::pipeline_ = pipeline;
}

// AC-22: Only the new injection diagnostic is checked; platform logs/trace/dump are outside this test.
HWTEST_F(UIEventCommandTest, DfxNoSensitiveContent001, TestSize.Level1)
{
    auto uid = CreateNode(11); // Deterministically reject in the test Pattern, without a component failure branch.
    const auto json = R"({"schemaVersion":1,"cmd":{"type":"setText","action_info":)"
        R"({"value":"synthetic-private-text"}}})";
    CallbackState state;
    EXPECT_EQ(RunInject(uid, json, &state), ARKUI_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(state.count, 1);
    EXPECT_EQ(state.result, COMMAND_NOT_SUPPORTED);
    const char* msg = ErrorMessageManager::GetInstance().GetErrorMessage();
    ASSERT_NE(msg, nullptr);
    const std::string message(msg);
    EXPECT_NE(message.find("command not supported by the node"), std::string::npos);
    EXPECT_NE(message.find(std::to_string(ARKUI_ERROR_CODE_ATTRIBUTE_OR_EVENT_NOT_SUPPORTED)), std::string::npos);
    EXPECT_EQ(message.find("synthetic-private-text"), std::string::npos);
}

// AC-22: The supplied private markers must not be echoed by the new field-validation rejection.
HWTEST_F(UIEventCommandTest, DfxNoLeakOnInvalid001, TestSize.Level1)
{
    auto executor = AceType::MakeRefPtr<DeferTaskExecutor>();
    MockPipelineContext::GetCurrent()->SetTaskExecutor(executor);
    auto uid = CreateNode();
    const auto json = R"({"schemaVersion":1,"cmd":{"type":"addText","action_info":)"
        R"({"value":"synthetic-private-text","offset":"synthetic-private-offset"}}})";
    CallbackState state;
    EXPECT_EQ(RunInject(uid, json, &state), ARKUI_ERROR_CODE_PARAM_INVALID);
    EXPECT_FALSE(executor->RunNext());
    EXPECT_EQ(state.count, 0);
    const char* msg = ErrorMessageManager::GetInstance().GetErrorMessage();
    ASSERT_NE(msg, nullptr);
    const std::string message(msg);
    EXPECT_NE(message.find("missing or invalid field for command type"), std::string::npos);
    EXPECT_NE(message.find(std::to_string(ARKUI_ERROR_CODE_PARAM_INVALID)), std::string::npos);
    EXPECT_EQ(message.find("synthetic-private-text"), std::string::npos);
    EXPECT_EQ(message.find("synthetic-private-offset"), std::string::npos);
}

} // namespace OHOS::Ace::NG

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

#include "gtest/gtest.h"

#include <chrono>
#include <functional>
#include <thread>
#include <utility>
#include <vector>

#include "base/json/json_util.h"
#include "base/memory/referenced.h"
#include "core/common/event_manager.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/relaxed_interaction/executor_generator.h"
#include "core/components_ng/relaxed_interaction/executors/mouse_click_executor.h"
#include "core/components_ng/relaxed_interaction/executors/press_and_move_executor.h"
#include "core/components_ng/relaxed_interaction/executors/press_key_executor.h"
#include "core/components_ng/relaxed_interaction/executors/simulate_touch_executor.h"
#include "core/components_ng/relaxed_interaction/executors/strict_click_executor.h"
#include "core/components_ng/relaxed_interaction/executors/wheel_scroll_executor.h"
#include "core/components_ng/relaxed_interaction/parsers/press_and_move_command_parser.h"
#include "core/components_ng/relaxed_interaction/parsers/press_key_command_parser.h"
#include "core/components_ng/relaxed_interaction/parsers/wheel_scroll_command_parser.h"
#include "core/components_ng/relaxed_interaction/relaxed_interaction_manager.h"
#include "core/components_ng/relaxed_interaction/utils/relaxed_event_factory.h"
#include "core/pipeline_ng/pipeline_context.h"
#include "core/components_ng/pattern/command/ui_command_security_gate.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "test/mock/adapter/ohos/osal/mock_input_manager.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {
constexpr char PRESS_KEY_EXECUTOR[] = "press_key";
constexpr char WHEEL_SCROLL_EXECUTOR[] = "wheel_scroll";
constexpr char PRESS_AND_MOVE_EXECUTOR[] = "press_and_move";

std::unique_ptr<JsonValue> Parse(const std::string& json)
{
    return JsonUtil::ParseJsonString(json);
}
} // namespace

class PcInputInjectionParserTest : public testing::Test {
protected:
    WeakPtr<PipelineContext> Context() const
    {
        return WeakPtr<PipelineContext>();
    }
};

HWTEST_F(PcInputInjectionParserTest, PressKeyParser_ParsesKeySequenceAndHoldTime, TestSize.Level1)
{
    PressKeyCommandParser parser(Context());
    auto executors = parser.Parse(Parse(R"({"action_info":{
        "key": ["ctrl", "a"],
        "hold_ms": 100
    }})"));

    ASSERT_EQ(executors.size(), 1U);
    EXPECT_EQ(executors.front()->GetType(), PRESS_KEY_EXECUTOR);
}

HWTEST_F(PcInputInjectionParserTest, PressKeyParser_RejectsInvalidHoldTime, TestSize.Level1)
{
    PressKeyCommandParser parser(Context());
    EXPECT_TRUE(parser.Parse(Parse(R"({"action_info":{"key":"enter", "hold_ms":100.5}})")).empty());
    EXPECT_TRUE(parser.Parse(Parse(R"({"action_info":{"key":"enter", "hold_ms":5001}})")).empty());
}

HWTEST_F(PcInputInjectionParserTest, PressKeyParser_ParsesActionInfoContractForm, TestSize.Level1)
{
    PressKeyCommandParser parser(Context());
    for (const auto& json : {
        R"({"action_info":{"key":"A", "hold_ms":500}})",
        R"({"action_info":{"keys":["ctrl", "a"], "hold_ms":100}})",
        R"({"action_info":{"key":"ctrl+a"}})",
        R"({"action_info":{"key":"CTRL+SHIFT+A"}})",
    }) {
        auto executors = parser.Parse(Parse(json));
        ASSERT_EQ(executors.size(), 1U) << json;
        EXPECT_EQ(executors.front()->GetType(), PRESS_KEY_EXECUTOR) << json;
    }
}

HWTEST_F(PcInputInjectionParserTest, PressKeyParser_RejectsMalformedActionInfoAndCombos, TestSize.Level1)
{
    PressKeyCommandParser parser(Context());
    for (const auto& json : {
        R"({"action_info":123})",
        R"({"action_info":"ctrl+a"})",
        R"({"action_info":{}})",
        R"({"action_info":{"key":123}})",
        R"({"action_info":{"key":"ctrl+"}})",
        R"({"action_info":{"key":"+a"}})",
        R"({"action_info":{"key":"a++b"}})",
        R"({"action_info":{"key":"ctrl+ctrl"}})",
        R"({"action_info":{"keys":["a", "a"]}})",
        R"({"action_info":{"key":"a", "hold_ms":100.5}})",
    }) {
        EXPECT_TRUE(parser.Parse(Parse(json)).empty()) << json;
    }
}

HWTEST_F(PcInputInjectionParserTest, WheelScrollParser_ParsesDeltaAndModifiers, TestSize.Level1)
{
    WheelScrollCommandParser parser(Context());
    auto executors = parser.Parse(Parse(R"({"action_info":{
        "delta": -120,
        "modifier": ["ctrl", "shift"],
        "coordinates": {"x1": 20, "y1": 30}
    }})"));

    ASSERT_EQ(executors.size(), 1U);
    EXPECT_EQ(executors.front()->GetType(), WHEEL_SCROLL_EXECUTOR);
}

HWTEST_F(PcInputInjectionParserTest, WheelScrollParser_RejectsUnknownModifier, TestSize.Level1)
{
    WheelScrollCommandParser parser(Context());
    EXPECT_TRUE(parser.Parse(Parse(R"({"action_info":{"delta":1, "modifier":"meta"}})")).empty());
}

HWTEST_F(PcInputInjectionParserTest, PressAndMoveParser_ParsesTouchAndMouseForms, TestSize.Level1)
{
    PressAndMoveCommandParser parser(Context());
    auto touch = parser.Parse(Parse(R"({"action_info":{
        "from": {"x": 10, "y": 20},
        "to": {"x": 100, "y": 200}
    }})"));
    auto mouse = parser.Parse(Parse(R"({"action_info":{
        "from": {"x": 10, "y": 20},
        "to": {"x": 100, "y": 200},
        "input_type": "mouse"
    }})"));

    ASSERT_EQ(touch.size(), 1U);
    ASSERT_EQ(mouse.size(), 1U);
    EXPECT_EQ(touch.front()->GetType(), PRESS_AND_MOVE_EXECUTOR);
    EXPECT_EQ(mouse.front()->GetType(), PRESS_AND_MOVE_EXECUTOR);
}

HWTEST_F(PcInputInjectionParserTest, PressAndMoveParser_RejectsIncompleteCoordinates, TestSize.Level1)
{
    PressAndMoveCommandParser parser(Context());
    EXPECT_TRUE(parser.Parse(Parse(R"({"action_info":{"from":{"x":10, "y":20}}})")).empty());
}

HWTEST_F(PcInputInjectionParserTest, ExecutorGenerator_RegistersPcInputCommands, TestSize.Level1)
{
    ExecutorGenerator generator(Context());
    auto pressKey = generator.ParseCommand(Parse(R"({
        "cmd": {"type": "press_key", "action_info": {"key": "enter"}}
    })"));
    auto wheelScroll = generator.ParseCommand(Parse(R"({
        "cmd": {"type": "wheel_scroll", "action_info": {"delta": 120}}
    })"));
    auto pressAndMove = generator.ParseCommand(Parse(R"({
        "cmd": {"type": "press_and_move", "action_info": {"from": {"x": 0, "y": 0},
            "to": {"x": 10, "y": 10}}}
    })"));

    ASSERT_EQ(pressKey.size(), 1U);
    ASSERT_EQ(wheelScroll.size(), 1U);
    ASSERT_EQ(pressAndMove.size(), 1U);
    EXPECT_EQ(pressKey.front()->GetType(), PRESS_KEY_EXECUTOR);
    EXPECT_EQ(wheelScroll.front()->GetType(), WHEEL_SCROLL_EXECUTOR);
    EXPECT_EQ(pressAndMove.front()->GetType(), PRESS_AND_MOVE_EXECUTOR);
}

HWTEST_F(PcInputInjectionParserTest, Manager_PreemptsQueuedPressKeyForUserAction, TestSize.Level1)
{
    auto manager = AceType::MakeRefPtr<RelaxedInteractionManager>(WeakPtr<PipelineContext>());
    ASSERT_NE(manager, nullptr);

    NG::ExecutionState observed = NG::ExecutionState::NONE;
    auto result = manager->ProcessCommand(R"({
        "cmd": {"type": "press_key", "action_info": {"key": "enter", "hold_ms": 100}}
    })", [&observed](NG::ExecutionState state) { observed = state; }, true);

    EXPECT_EQ(result, ProcessResult::SUCCESS);
    EXPECT_TRUE(manager->HasPendingWorkflow());
    EXPECT_TRUE(manager->IsKeyClassInjectionRunning());
    EXPECT_FALSE(manager->IsPointerClassInjectionRunning());

    manager->Clear(NG::ExecutionState::INTERRUPTED_BY_USER);
    manager->ExecuteNextStep();

    EXPECT_FALSE(manager->HasPendingWorkflow());
    EXPECT_EQ(observed, NG::ExecutionState::INTERRUPTED_BY_USER);
}

HWTEST_F(PcInputInjectionParserTest, PressKeyParser_ParsesKeysAliasAndCaseInsensitiveNames, TestSize.Level1)
{
    PressKeyCommandParser parser(Context());
    auto aliases = parser.Parse(Parse(R"({"action_info":{"keys":["RETURN", "esc", "Space", "tab"]}})"));
    ASSERT_EQ(aliases.size(), 1U);
    EXPECT_EQ(aliases.front()->GetType(), PRESS_KEY_EXECUTOR);
}

HWTEST_F(PcInputInjectionParserTest, PressKeyParser_ParsesLetterDigitAndNamedKeys, TestSize.Level1)
{
    PressKeyCommandParser parser(Context());
    for (const auto& key : {"a", "Z", "0", "9", "control", "shift", "alt", "home", "end", "up", "down",
                              "left", "right", "backspace", "delete"}) {
        std::string json = std::string("{\"action_info\":{\"key\":\"") + key + "\"}}";
        auto executors = parser.Parse(Parse(json));
        ASSERT_EQ(executors.size(), 1U) << key;
    }
}

HWTEST_F(PcInputInjectionParserTest, PressKeyParser_RejectsEmptyOrUnknownKeys, TestSize.Level1)
{
    PressKeyCommandParser parser(Context());
    EXPECT_TRUE(parser.Parse(Parse(R"({"action_info":{"key":[]}})")).empty());
    EXPECT_TRUE(parser.Parse(Parse(R"({"action_info":{"key":"not-a-key"}})")).empty());
    EXPECT_TRUE(parser.Parse(Parse(R"({"action_info":{"key":["enter", "not-a-key"]}})")).empty());
}

HWTEST_F(PcInputInjectionParserTest, PressKeyParser_RejectsNonStringKeyValues, TestSize.Level1)
{
    PressKeyCommandParser parser(Context());
    EXPECT_TRUE(parser.Parse(Parse(R"({"action_info":{"key":123}})")).empty());
    EXPECT_TRUE(parser.Parse(Parse(R"({"action_info":{"key":["enter",1]}})")).empty());
    EXPECT_TRUE(parser.Parse(Parse(R"({"action_info":{"key":null}})")).empty());
}

HWTEST_F(PcInputInjectionParserTest, PressKeyParser_AcceptsHoldTimeBoundaries, TestSize.Level1)
{
    PressKeyCommandParser parser(Context());
    EXPECT_EQ(parser.Parse(Parse(R"({"action_info":{"key":"enter", "hold_ms":0}})")).size(), 1U);
    EXPECT_EQ(parser.Parse(Parse(R"({"action_info":{"key":"enter", "hold_ms":5000}})")).size(), 1U);
}

HWTEST_F(PcInputInjectionParserTest, PressKeyParser_RejectsInvalidHoldTimeTypesAndRange, TestSize.Level1)
{
    PressKeyCommandParser parser(Context());
    EXPECT_TRUE(parser.Parse(Parse(R"({"action_info":{"key":"enter", "hold_ms":-1}})")).empty());
    EXPECT_TRUE(parser.Parse(Parse(R"({"action_info":{"key":"enter", "hold_ms":5001}})")).empty());
    EXPECT_TRUE(parser.Parse(Parse(R"({"action_info":{"key":"enter", "hold_ms":"100"}})")).empty());
    EXPECT_TRUE(parser.Parse(Parse(R"({"action_info":{"key":"enter", "hold_ms":null}})")).empty());
}

HWTEST_F(PcInputInjectionParserTest, PressKeyParser_RejectsNonObjectAndMissingKey, TestSize.Level1)
{
    PressKeyCommandParser parser(Context());
    EXPECT_TRUE(parser.Parse(Parse(R"([])")).empty());
    EXPECT_TRUE(parser.Parse(Parse(R"(null)")).empty());
    EXPECT_TRUE(parser.Parse(Parse(R"({"hold_ms":100})")).empty());
}

HWTEST_F(PcInputInjectionParserTest, WheelScrollParser_ParsesActionInfoAndModifiersAlias, TestSize.Level1)
{
    WheelScrollCommandParser parser(Context());
    auto executors = parser.Parse(Parse(R"({
        "action_info": {"delta":120, "modifiers": ["ctrl", "shift"]}
    })"));
    ASSERT_EQ(executors.size(), 1U);
    EXPECT_EQ(executors.front()->GetType(), WHEEL_SCROLL_EXECUTOR);
}

HWTEST_F(PcInputInjectionParserTest, WheelScrollParser_ParsesTopLevelCoordinates, TestSize.Level1)
{
    WheelScrollCommandParser parser(Context());
    auto executors = parser.Parse(Parse(R"({"action_info":{"delta":-1, "x":12.5, "y":30}})"));
    ASSERT_EQ(executors.size(), 1U);
    EXPECT_EQ(executors.front()->GetType(), WHEEL_SCROLL_EXECUTOR);
}

HWTEST_F(PcInputInjectionParserTest, WheelScrollParser_ParsesActionInfoCoordinates, TestSize.Level1)
{
    WheelScrollCommandParser parser(Context());
    auto executors = parser.Parse(Parse(R"({
        "action_info": {"delta": 1, "coordinates": {"x1": 20, "y1": 30}}
    })"));
    ASSERT_EQ(executors.size(), 1U);
}

HWTEST_F(PcInputInjectionParserTest, WheelScrollParser_AcceptsModifierAliasesAndEmptyList, TestSize.Level1)
{
    WheelScrollCommandParser parser(Context());
    EXPECT_EQ(parser.Parse(Parse(R"({"action_info":{"delta":1, "modifier":"alt"}})")).size(), 1U);
    EXPECT_EQ(parser.Parse(Parse(R"({"action_info":{"delta":1, "modifiers":[]}})")).size(), 1U);
}

HWTEST_F(PcInputInjectionParserTest, WheelScrollParser_RejectsMissingOrFractionalDelta, TestSize.Level1)
{
    WheelScrollCommandParser parser(Context());
    EXPECT_TRUE(parser.Parse(Parse(R"({})")).empty());
    EXPECT_TRUE(parser.Parse(Parse(R"({"action_info":{"delta":1.5}})")).empty());
    EXPECT_TRUE(parser.Parse(Parse(R"({"action_info":{"delta":"1"}})")).empty());
}

HWTEST_F(PcInputInjectionParserTest, WheelScrollParser_RejectsInvalidModifierTypesAndNames, TestSize.Level1)
{
    WheelScrollCommandParser parser(Context());
    EXPECT_TRUE(parser.Parse(Parse(R"({"action_info":{"delta":1, "modifier":1}})")).empty());
    EXPECT_TRUE(parser.Parse(Parse(R"({"action_info":{"delta":1, "modifier":"meta"}})")).empty());
    EXPECT_TRUE(parser.Parse(Parse(R"({"action_info":{"delta":1, "modifiers":["ctrl",2]}})")).empty());
    EXPECT_TRUE(parser.Parse(Parse(R"({"action_info":{"delta":1, "modifiers":["META"]}})")).empty());
}

HWTEST_F(PcInputInjectionParserTest, WheelScrollParser_RejectsInvalidOptionalCoordinateTypes, TestSize.Level1)
{
    WheelScrollCommandParser parser(Context());
    // SDD AC-3.2 requires invalid explicit coordinates to fail instead of using a default point.
    EXPECT_TRUE(parser.Parse(Parse(R"({"action_info":{"delta":1, "coordinates":{"x1":"1", "y1":2}}})")).empty());
    EXPECT_TRUE(parser.Parse(Parse(R"({"action_info":{"delta":1, "coordinates":{"x1":"1", "y1":2}}})")).empty());
}

HWTEST_F(PcInputInjectionParserTest, PressAndMoveParser_ParsesActionInfoFromToAndFlatCoordinates, TestSize.Level1)
{
    PressAndMoveCommandParser parser(Context());
    EXPECT_EQ(parser.Parse(Parse(R"({"action_info":{"from":{"x":1, "y":2}, "to":{"x":3, "y":4}}})")).size(), 1U);
    EXPECT_TRUE(parser.Parse(Parse(R"({"action_info":{"from_x":1, "from_y":2, "to_x":3, "to_y":4}})")).empty());
}

HWTEST_F(PcInputInjectionParserTest, PressAndMoveParser_AcceptsExplicitTouchInput, TestSize.Level1)
{
    PressAndMoveCommandParser parser(Context());
    auto executors = parser.Parse(Parse(R"({"action_info":{
        "from":{"x":1, "y":2}, "to":{"x":3, "y":4}, "input_type":"touch"
    }})"));
    ASSERT_EQ(executors.size(), 1U);
    EXPECT_EQ(executors.front()->GetType(), PRESS_AND_MOVE_EXECUTOR);
}

HWTEST_F(PcInputInjectionParserTest, PressAndMoveParser_RejectsInvalidInputType, TestSize.Level1)
{
    PressAndMoveCommandParser parser(Context());
    EXPECT_TRUE(parser.Parse(
        Parse(R"({"action_info":{"from":{"x":1, "y":2}, "to":{"x":3, "y":4}, "input_type":"pen"}})")).empty());
    EXPECT_TRUE(parser.Parse(
        Parse(R"({"action_info":{"from":{"x":1, "y":2}, "to":{"x":3, "y":4}, "input_type":1}})")).empty());
}

HWTEST_F(PcInputInjectionParserTest, PressAndMoveParser_RejectsInvalidCoordinateTypesAndShapes, TestSize.Level1)
{
    PressAndMoveCommandParser parser(Context());
    EXPECT_TRUE(parser.Parse(Parse(R"({"action_info":{"from":{"x":"1", "y":2}, "to":{"x":3, "y":4}}})")).empty());
    EXPECT_TRUE(parser.Parse(Parse(R"({"coordinates":{"x1":1, "y1":2, "x2":3}})")).empty());
    EXPECT_TRUE(parser.Parse(Parse(R"({"action_info":{"from_x":1, "from_y":2, "to_x":3}})")).empty());
}

HWTEST_F(PcInputInjectionParserTest, PressAndMoveParser_RejectsNonObjectAndMissingCoordinates, TestSize.Level1)
{
    PressAndMoveCommandParser parser(Context());
    EXPECT_TRUE(parser.Parse(Parse(R"([])")).empty());
    EXPECT_TRUE(parser.Parse(Parse(R"(null)")).empty());
    EXPECT_TRUE(parser.Parse(Parse(R"({"input_type":"mouse"})")).empty());
}

HWTEST_F(PcInputInjectionParserTest, ExecutorGenerator_RejectsMissingAndMalformedCommands, TestSize.Level1)
{
    ExecutorGenerator generator(Context());
    EXPECT_TRUE(generator.ParseCommand(Parse(R"({})")).empty());
    EXPECT_TRUE(generator.ParseCommand(Parse(R"({"cmd":null})")).empty());
    EXPECT_TRUE(generator.ParseCommand(Parse(R"({"cmd":[]})")).empty());
    EXPECT_TRUE(generator.ParseCommand(Parse(R"({"cmd":{"type":1}})")).empty());
    EXPECT_TRUE(generator.ParseCommand(Parse(R"({"cmd":{"type":"unknown"}})")).empty());
}

HWTEST_F(PcInputInjectionParserTest, ExecutorGenerator_ParsesEachPcCommandRoute, TestSize.Level1)
{
    ExecutorGenerator generator(Context());
    const std::vector<std::pair<std::string, std::string>> commands = {
        {R"({"cmd":{"type":"press_key", "action_info":{"keys":["a", "b"]}}})", PRESS_KEY_EXECUTOR},
        {R"({"cmd":{"type":"wheel_scroll", "action_info":{"delta":-120, "modifier":"alt"}}})", WHEEL_SCROLL_EXECUTOR},
        {R"({"cmd":{"type":"press_and_move", "action_info":{"from":{"x":0, "y":0}, "to":{"x":1, "y":1}}}})",
            PRESS_AND_MOVE_EXECUTOR},
    };
    for (const auto& command : commands) {
        auto executors = generator.ParseCommand(Parse(command.first));
        ASSERT_EQ(executors.size(), 1U);
        EXPECT_EQ(executors.front()->GetType(), command.second);
    }
}

HWTEST_F(PcInputInjectionParserTest, Manager_ReturnsParseAndValidationErrors, TestSize.Level1)
{
    auto manager = AceType::MakeRefPtr<RelaxedInteractionManager>(WeakPtr<PipelineContext>());
    auto callback = [](ExecutionState) {};
    EXPECT_EQ(manager->ProcessCommand("{", callback), ProcessResult::PARSE_ERROR);
    EXPECT_EQ(manager->ProcessCommand(R"({"cmd":{"type":"unknown"}})", callback), ProcessResult::NO_VALID_CMD);
    EXPECT_EQ(manager->ProcessCommand(R"({"cmd":{"type":"wheel_scroll"}})", callback),
        ProcessResult::VALIDATION_ERROR);
}

HWTEST_F(PcInputInjectionParserTest, Manager_TracksPointerInjectionAndClear, TestSize.Level1)
{
    auto manager = AceType::MakeRefPtr<RelaxedInteractionManager>(WeakPtr<PipelineContext>());
    EXPECT_EQ(manager->ProcessCommand(R"({"cmd":{"type":"wheel_scroll", "action_info":{"delta":1}}})",
        [](ExecutionState) {}, true),
        ProcessResult::SUCCESS);
    EXPECT_TRUE(manager->HasPendingWorkflow());
    EXPECT_TRUE(manager->IsPointerClassInjectionRunning());
    EXPECT_FALSE(manager->IsKeyClassInjectionRunning());
    manager->Clear();
    EXPECT_FALSE(manager->HasPendingWorkflow());
    EXPECT_FALSE(manager->IsPointerClassInjectionRunning());
}

HWTEST_F(PcInputInjectionParserTest, Manager_KeepsQueueingCommandsBeyondSixteenPending, TestSize.Level1)
{
    auto manager = AceType::MakeRefPtr<RelaxedInteractionManager>(WeakPtr<PipelineContext>());
    std::vector<NG::ExecutionState> states;
    auto callback = [&states](NG::ExecutionState state) { states.emplace_back(state); };
    // The pending queue has no bound: commands keep queuing past any depth and finish serially.
    for (size_t index = 0; index < 20; ++index) {
        EXPECT_EQ(manager->ProcessCommand(
            R"({"cmd":{"type":"wheel_scroll", "action_info":{"delta":1}}})", callback, true), ProcessResult::SUCCESS)
            << index;
    }
    EXPECT_TRUE(manager->HasPendingWorkflow());
    manager->Clear(ExecutionState::INTERRUPTED_BY_USER);
    manager->ExecuteNextStep();
    ASSERT_EQ(states.size(), 20U);
    EXPECT_EQ(states.front(), ExecutionState::INTERRUPTED_BY_USER);
    EXPECT_EQ(states.back(), ExecutionState::INTERRUPTED_BY_USER);
    EXPECT_FALSE(manager->HasPendingWorkflow());
}

HWTEST_F(PcInputInjectionParserTest, Manager_CompletesInterruptedWorkflowsWithUserPriority, TestSize.Level1)
{
    auto manager = AceType::MakeRefPtr<RelaxedInteractionManager>(WeakPtr<PipelineContext>());
    std::vector<NG::ExecutionState> states;
    auto callback = [&states](NG::ExecutionState state) { states.emplace_back(state); };
    EXPECT_EQ(manager->ProcessCommand(
        R"({"cmd":{"type":"wheel_scroll", "action_info":{"delta":1}}})", callback, true), ProcessResult::SUCCESS);
    EXPECT_EQ(manager->ProcessCommand(
        R"({"cmd":{"type":"press_key", "action_info":{"key":"a"}}})", callback, true), ProcessResult::SUCCESS);
    manager->Clear(ExecutionState::INTERRUPTED_BY_USER);
    manager->ExecuteNextStep();
    ASSERT_EQ(states.size(), 2U);
    EXPECT_EQ(states[0], ExecutionState::INTERRUPTED_BY_USER);
    EXPECT_EQ(states[1], ExecutionState::INTERRUPTED_BY_USER);
    EXPECT_FALSE(manager->HasPendingWorkflow());
}

HWTEST_F(PcInputInjectionParserTest, Manager_CompletesWorkflowWithTimeout, TestSize.Level1)
{
    auto context = AceType::MakeRefPtr<MockPipelineContext>();
    auto manager = AceType::MakeRefPtr<RelaxedInteractionManager>(WeakPtr<PipelineContext>(context));
    manager->SetCommandTimeoutMs(1);
    NG::ExecutionState observed = ExecutionState::NONE;
    size_t callbackCount = 0;
    ASSERT_EQ(manager->ProcessCommand(
        R"({"cmd":{"type":"press_key", "action_info":{"key":"a", "hold_ms":100}}})",
        [&observed, &callbackCount](NG::ExecutionState state) {
            observed = state;
            ++callbackCount;
        }, true), ProcessResult::SUCCESS);
    ASSERT_EQ(manager->ExecuteNextStep(), ExecutionState::RUNNING);
    // hold_ms=100 extends the effective timeout to hold+margin(500)=600ms; judge past it.
    std::this_thread::sleep_for(std::chrono::milliseconds(700));
    EXPECT_EQ(manager->ExecuteNextStep(), ExecutionState::TIMEOUT);
    EXPECT_EQ(observed, ExecutionState::TIMEOUT);
    EXPECT_FALSE(manager->HasPendingWorkflow());
    EXPECT_EQ(callbackCount, 1U);
    EXPECT_EQ(manager->ExecuteNextStep(), ExecutionState::FAILED);
    EXPECT_EQ(callbackCount, 1U);
}

HWTEST_F(PcInputInjectionParserTest, Manager_ReportsDenyAsControlDenied, TestSize.Level1)
{
    auto context = AceType::MakeRefPtr<MockPipelineContext>();
    auto manager = AceType::MakeRefPtr<RelaxedInteractionManager>(WeakPtr<PipelineContext>(context));
    manager->SetDenyCallback([](const PointF&, const RelaxedCommandIdentity&) { return true; });
    NG::ExecutionState observed = ExecutionState::NONE;
    EXPECT_EQ(manager->ProcessCommand(
        R"({"cmd":{"type":"wheel_scroll", "action_info":{"delta":1, "coordinates":{"x1":0, "y1":0}}}})",
        [&observed](NG::ExecutionState state) { observed = state; }, true), ProcessResult::SUCCESS);
    EXPECT_EQ(manager->ExecuteNextStep(), ExecutionState::CONTROL_DENIED);
    EXPECT_EQ(observed, ExecutionState::CONTROL_DENIED);
}

HWTEST_F(PcInputInjectionParserTest, ClickParser_SupportsMouseButtonAndLegacyAlias, TestSize.Level1)
{
    ExecutorGenerator generator(Context());
    for (const auto& button : {"left", "right", "middle", "forward", "back"}) {
        for (const auto& field : {"mouse_button", "button"}) {
            const std::string json = std::string(R"({"cmd":{"type":"click", "action_info":{")") +
                field + R"(":")" + button + R"(", "coordinates":{"x1":10, "y1":20}}}})";
            auto executors = generator.ParseCommand(Parse(json));
            ASSERT_EQ(executors.size(), 1U) << field << ':' << button;
            EXPECT_EQ(executors.front()->GetType(), "mouse_click");
        }
    }
    auto legacyTouch = generator.ParseCommand(Parse(R"({"cmd":{"type":"click", "mode":"simulating",
        "action_info":{"coordinates":{"x1":10, "y1":20}}}})"));
    ASSERT_EQ(legacyTouch.size(), 1U);
    EXPECT_EQ(legacyTouch.front()->GetType(), "simulate_touch");
}

HWTEST_F(PcInputInjectionParserTest, ClickParser_RejectsInvalidOrConflictingButtonAliases, TestSize.Level1)
{
    ExecutorGenerator generator(Context());
    const std::vector<std::string> invalid = {
        R"({"mouse_button":"unknown"})", R"({"mouse_button":1})", R"({"mouse_button":null})",
        R"({"mouse_button":"right", "button":"left"})",
        R"({"mouse_button":"right", "button":1})", R"({"button":"unknown"})",
        R"({"click_count":1.5})",
    };
    for (const auto& options : invalid) {
        const std::string json = std::string(R"({"cmd":{"type":"click", "action_info":{
            "coordinates":{"x1":10, "y1":20},)") + options.substr(1) + "}}";
        auto parsed = Parse(json);
        ASSERT_NE(parsed, nullptr);
        ASSERT_TRUE(parsed->IsValid()) << json;
        EXPECT_TRUE(generator.ParseCommand(parsed).empty()) << options;
    }
    auto matchingAliases = generator.ParseCommand(Parse(R"({"cmd":{"type":"click", "action_info":{
        "coordinates":{"x1":10, "y1":20}, "mouse_button":"right", "button":"right"}}})"));
    ASSERT_EQ(matchingAliases.size(), 1U);
    EXPECT_EQ(matchingAliases.front()->GetType(), "mouse_click");
}

HWTEST_F(PcInputInjectionParserTest, ClickParser_CountAndModifiersAloneKeepTouchPath, TestSize.Level1)
{
    ExecutorGenerator generator(Context());
    // click_count / modifier without an explicit button must stay on the touch-compatible
    // path (auto mode creates the strict-click + simulate-touch pair), never mouse_click.
    for (const auto& options : {
        R"("click_count":2)",
        R"("modifier":["ctrl"])",
        R"("click_count":3, "modifier":"shift")",
    }) {
        const std::string json = std::string(R"({"cmd":{"type":"click", "action_info":{
            "coordinates":{"x1":10, "y1":20},)") + options + "}}}";
        auto executors = generator.ParseCommand(Parse(json));
        ASSERT_EQ(executors.size(), 2U) << options;
        EXPECT_NE(executors.front()->GetType(), "mouse_click") << options;
        EXPECT_NE(executors.back()->GetType(), "mouse_click") << options;
    }
    // An invalid count without a button is still rejected outright.
    EXPECT_TRUE(generator.ParseCommand(Parse(R"({"cmd":{"type":"click", "action_info":{
        "coordinates":{"x1":10, "y1":20}, "click_count":1.5}}})")).empty());
}

HWTEST_F(PcInputInjectionParserTest, WheelScrollParser_AcceptsDirectionsAndNestedDeltaFallback, TestSize.Level1)
{
    WheelScrollCommandParser parser(Context());
    for (const auto& json : {
        R"({"action_info":{"delta":120}})",
        R"({"action_info":{"delta":-120, "direction":"horizontal"}})",
        R"({"action_info":{"delta":120, "direction":"vertical"}})",
        R"({"action_info":{"delta":120, "direction":"horizontal"}})",
    }) {
        EXPECT_EQ(parser.Parse(Parse(json)).size(), 1U) << json;
    }
}

HWTEST_F(PcInputInjectionParserTest, WheelScrollParser_RejectsDirectionAndPartialCoordinateErrors, TestSize.Level1)
{
    WheelScrollCommandParser parser(Context());
    for (const auto& json : {
        R"({"delta":120})", R"({"action_info":123})",
        R"({"action_info":{"delta":120, "direction":"diagonal"}})", R"({"action_info":{"delta":120, "direction":1}})",
        R"({"action_info":{"delta":120, "direction":null}})", R"({"action_info":{"delta":"120"}})",
        R"({"action_info":{"delta":1.5}})", R"({"action_info":{"delta":120, "coordinates":{}}})",
        R"({"action_info":{"delta":120, "coordinates":{"x1":10}}})",
        R"({"action_info":{"delta":120, "coordinates":null}})",
        R"({"action_info":{"delta":120, "coordinates":{"x1":"bad", "y1":10}}})",
        R"({"action_info":{"delta":120, "direction":"invalid"}})",
    }) {
        EXPECT_TRUE(parser.Parse(Parse(json)).empty()) << json;
    }
}

HWTEST_F(PcInputInjectionParserTest, WheelFactory_MapsDirectionAndWindowOffset, TestSize.Level1)
{
    const std::vector<KeyCode> modifiers = { KeyCode::KEY_CTRL_LEFT };
    auto horizontal = RelaxedEventFactory::CreateAxisEvent(
        PointF(10.0f, 20.0f), Offset(100.0, 200.0), -120.0, modifiers, WheelScrollDirection::HORIZONTAL);
    EXPECT_DOUBLE_EQ(horizontal.horizontalAxis, -120.0);
    EXPECT_DOUBLE_EQ(horizontal.verticalAxis, 0.0);
    EXPECT_EQ(horizontal.axes, 1U << static_cast<uint32_t>(AxisType::HORIZONTAL_AXIS));
    EXPECT_FLOAT_EQ(horizontal.x, 10.0f);
    EXPECT_FLOAT_EQ(horizontal.y, 20.0f);
    EXPECT_DOUBLE_EQ(horizontal.screenX, 110.0);
    EXPECT_DOUBLE_EQ(horizontal.screenY, 220.0);
    EXPECT_DOUBLE_EQ(horizontal.globalDisplayX, 110.0);
    EXPECT_DOUBLE_EQ(horizontal.globalDisplayY, 220.0);
    EXPECT_EQ(horizontal.pressedCodes, modifiers);
    EXPECT_TRUE(horizontal.isInjected);

    auto vertical = RelaxedEventFactory::CreateAxisEvent(
        PointF(10.0f, 20.0f), Offset(-30.0, -40.0), 120.0, {}, WheelScrollDirection::VERTICAL);
    EXPECT_DOUBLE_EQ(vertical.verticalAxis, 120.0);
    EXPECT_DOUBLE_EQ(vertical.horizontalAxis, 0.0);
    EXPECT_EQ(vertical.axes, 1U << static_cast<uint32_t>(AxisType::VERTICAL_AXIS));
    EXPECT_DOUBLE_EQ(vertical.screenX, -20.0);
    EXPECT_DOUBLE_EQ(vertical.screenY, -20.0);
}

HWTEST_F(PcInputInjectionParserTest, WheelFactory_PreservesLegacyVerticalOverload, TestSize.Level1)
{
    auto event = RelaxedEventFactory::CreateAxisEvent(PointF(10.0f, 20.0f), 120.0, {});
    EXPECT_DOUBLE_EQ(event.verticalAxis, 120.0);
    EXPECT_DOUBLE_EQ(event.horizontalAxis, 0.0);
    EXPECT_DOUBLE_EQ(event.screenX, 10.0);
    EXPECT_DOUBLE_EQ(event.screenY, 20.0);
}

HWTEST_F(PcInputInjectionParserTest, KeyFactory_AlignsFieldsWithRealKeyboardPipeline, TestSize.Level1)
{
    // msg must stay empty: KeyEvent::msg doubles as the IME paste payload, and ctrl+v would
    // otherwise type the marker string into a focused text box. Injection is marked by isInjected.
    auto esc = RelaxedEventFactory::CreateKeyEvent(KeyCode::KEY_ESCAPE, KeyAction::DOWN, { KeyCode::KEY_ESCAPE });
    EXPECT_TRUE(esc.isInjected);
    EXPECT_TRUE(esc.msg.empty());
    EXPECT_EQ(esc.keyIntention, KeyIntention::INTENTION_ESCAPE);
    EXPECT_EQ(esc.key, "KEYCODE_ESCAPE");
    EXPECT_EQ(esc.unicode, 0U);

    auto letter = RelaxedEventFactory::CreateKeyEvent(KeyCode::KEY_A, KeyAction::DOWN, { KeyCode::KEY_A });
    EXPECT_TRUE(letter.msg.empty());
    EXPECT_EQ(letter.keyIntention, KeyIntention::INTENTION_UNKNOWN);
    EXPECT_EQ(letter.unicode, static_cast<uint32_t>('a'));

    auto digit = RelaxedEventFactory::CreateKeyEvent(KeyCode::KEY_5, KeyAction::DOWN, { KeyCode::KEY_5 });
    EXPECT_EQ(digit.unicode, static_cast<uint32_t>('5'));
}

HWTEST_F(PcInputInjectionParserTest, MouseFactory_FillsPressedTimeAndButtonsArray, TestSize.Level1)
{
    auto press = RelaxedEventFactory::CreateMouseEvent(PointF(1.0f, 2.0f), MouseAction::PRESS,
        MouseButton::LEFT_BUTTON, static_cast<int32_t>(MouseButton::LEFT_BUTTON), {});
    EXPECT_NE(press.pressedTime.time_since_epoch().count(), 0);
    ASSERT_EQ(press.pressedButtonsArray.size(), 1U);
    EXPECT_EQ(press.pressedButtonsArray.front(), MouseButton::LEFT_BUTTON);

    // RELEASE carries an empty pressed set: the button is no longer held.
    auto release = RelaxedEventFactory::CreateMouseEvent(PointF(1.0f, 2.0f), MouseAction::RELEASE,
        MouseButton::LEFT_BUTTON, 0, {});
    EXPECT_NE(release.pressedTime.time_since_epoch().count(), 0);
    EXPECT_TRUE(release.pressedButtonsArray.empty());
}

class PcWheelCoordinateTest : public testing::Test {
protected:
    void SetUp() override
    {
        MockPipelineContext::SetUp();
        context_ = MockPipelineContext::GetCurrent();
        ASSERT_NE(context_, nullptr);
        context_->SetupRootElement();
        ASSERT_NE(context_->GetRootElement(), nullptr);
        auto rootGeometry = context_->GetRootElement()->GetGeometryNode();
        ASSERT_NE(rootGeometry, nullptr);
        rootGeometry->SetFrameOffset(OffsetF(10.0f, 20.0f));
        rootGeometry->SetFrameSize(SizeF(600.0f, 400.0f));
    }

    void TearDown() override
    {
        context_.Reset();
        MockPipelineContext::TearDown();
    }

    RefPtr<MockPipelineContext> context_;
};

HWTEST_F(PcWheelCoordinateTest, WheelExecutor_UsesRootCenterOnlyWhenCoordinatesOmitted, TestSize.Level1)
{
    WheelScrollCommandParser parser { WeakPtr<PipelineContext>(context_) };
    auto implicit = parser.Parse(Parse(R"({"action_info":{"delta":120}})"));
    ASSERT_EQ(implicit.size(), 1U);
    PointF observed;
    size_t checked = 0;
    auto deny = [&observed, &checked](const PointF& point, const RelaxedCommandIdentity&) {
        observed = point;
        ++checked;
        return true;
    };
    implicit.front()->SetDenyCallback(deny, {});
    EXPECT_EQ(implicit.front()->ExecuteStep(), ExecutorResult::FAILED);
    EXPECT_EQ(implicit.front()->GetFailureState(), ExecutionState::CONTROL_DENIED);
    EXPECT_EQ(checked, 1U);
    EXPECT_FLOAT_EQ(observed.GetX(), 310.0f);
    EXPECT_FLOAT_EQ(observed.GetY(), 220.0f);

    auto explicitPoint = parser.Parse(Parse(R"({"action_info":{"delta":120, "coordinates":{"x1":0, "y1":0}}})"));
    ASSERT_EQ(explicitPoint.size(), 1U);
    explicitPoint.front()->SetDenyCallback(deny, {});
    EXPECT_EQ(explicitPoint.front()->ExecuteStep(), ExecutorResult::FAILED);
    EXPECT_EQ(checked, 2U);
    // Off-window coordinates are clipped into the root rect (root spans [10, 610] x [20, 420]).
    EXPECT_FLOAT_EQ(observed.GetX(), 10.0f);
    EXPECT_FLOAT_EQ(observed.GetY(), 20.0f);

    auto mixedPoint = parser.Parse(Parse(
        R"({"action_info":{"delta":120, "coordinates":{"x1":42, "y1":64}}})"));
    ASSERT_EQ(mixedPoint.size(), 1U);
    mixedPoint.front()->SetDenyCallback(deny, {});
    EXPECT_EQ(mixedPoint.front()->ExecuteStep(), ExecutorResult::FAILED);
    EXPECT_EQ(mixedPoint.front()->GetFailureState(), ExecutionState::CONTROL_DENIED);
    EXPECT_EQ(checked, 3U);
    EXPECT_FLOAT_EQ(observed.GetX(), 42.0f);
    EXPECT_FLOAT_EQ(observed.GetY(), 64.0f);
}

HWTEST_F(PcWheelCoordinateTest, WheelExecutor_RejectsMissingTargetForImplicitPoint, TestSize.Level1)
{
    context_->GetRootElement()->GetGeometryNode()->SetFrameSize(SizeF(0.0f, 0.0f));
    WheelInput input;
    input.delta = 120;
    WheelScrollExecutor executor(WeakPtr<PipelineContext>(context_), input);
    size_t checked = 0;
    executor.SetDenyCallback([&checked](const PointF&, const RelaxedCommandIdentity&) {
        ++checked;
        return false;
    }, {});
    EXPECT_EQ(executor.ExecuteStep(), ExecutorResult::FAILED);
    EXPECT_EQ(executor.GetFailureState(), ExecutionState::TARGET_NOT_FOUND);
    EXPECT_EQ(checked, 0U);
}

HWTEST_F(PcWheelCoordinateTest, MouseClick_ClipsOffWindowCoordinatesIntoWindowRect, TestSize.Level1)
{
    ClickInput input;
    input.coordinates = PointF(-5.0f, -5.0f);
    MouseClickExecutor executor(WeakPtr<PipelineContext>(context_), input);
    PointF observed;
    size_t checked = 0;
    executor.SetDenyCallback([&observed, &checked](const PointF& point, const RelaxedCommandIdentity&) {
        observed = point;
        ++checked;
        return true;
    }, {});
    EXPECT_EQ(executor.ExecuteStep(), ExecutorResult::FAILED);
    EXPECT_EQ(executor.GetFailureState(), ExecutionState::CONTROL_DENIED);
    EXPECT_EQ(checked, 1U);
    // The injected press lands on the nearest in-window point, not the raw off-window one.
    EXPECT_FLOAT_EQ(observed.GetX(), 10.0f);
    EXPECT_FLOAT_EQ(observed.GetY(), 20.0f);
}

HWTEST_F(PcInputInjectionParserTest, WheelExecutor_DistinguishesMissingContext, TestSize.Level1)
{
    WheelInput input;
    input.delta = 120;
    WheelScrollExecutor executor(Context(), input);
    EXPECT_EQ(executor.ExecuteStep(), ExecutorResult::FAILED);
    EXPECT_EQ(executor.GetFailureState(), ExecutionState::CONTEXT_UNAVAILABLE);
}

namespace {
class PcRecoveryContext : public MockPipelineContext {
    DECLARE_ACE_TYPE(PcRecoveryContext, MockPipelineContext);

public:
    PcRecoveryContext()
    {
        eventManager_ = AceType::MakeRefPtr<EventManager>();
    }

    void OnTouchEvent(const TouchEvent& event, bool isSubPipe = false) override
    {
        touches.emplace_back(event);
        if (onTouch) {
            onTouch(event);
        }
    }

    void OnMouseEvent(const MouseEvent& event) override
    {
        mice.emplace_back(event);
        if (onMouse) {
            onMouse(event);
        }
    }

    void OnAxisEvent(const AxisEvent& event) override
    {
        axes.emplace_back(event);
        if (onAxis) {
            onAxis(event);
        }
    }

    bool OnNonPointerEvent(const NonPointerEvent& event) override
    {
        const auto& key = static_cast<const KeyEvent&>(event);
        keys.emplace_back(key);
        if (onKey) {
            onKey(key);
        }
        return keyHandled;
    }

    std::vector<TouchEvent> touches;
    std::vector<MouseEvent> mice;
    std::vector<AxisEvent> axes;
    std::vector<KeyEvent> keys;
    // Toggle to emulate a pipeline that swallows every key event (no focus / custom dispatcher).
    bool keyHandled = true;
    std::function<void(const TouchEvent&)> onTouch;
    std::function<void(const MouseEvent&)> onMouse;
    std::function<void(const AxisEvent&)> onAxis;
    std::function<void(const KeyEvent&)> onKey;
};

MoveInput RecoveryMoveInput(MoveInputType type)
{
    MoveInput input;
    input.coordinates = {10.0f, 20.0f, 90.0f, 180.0f};
    input.inputType = type;
    return input;
}
} // namespace

HWTEST_F(PcInputInjectionParserTest, MouseClick_CancelBeforePressEmitsNothing, TestSize.Level1)
{
    auto context = AceType::MakeRefPtr<PcRecoveryContext>();
    MouseClickExecutor executor(context, ClickInput {});
    executor.Cancel();
    executor.Cancel();
    EXPECT_EQ(executor.ExecuteStep(), ExecutorResult::SUCCESS);
    EXPECT_TRUE(context->mice.empty());
}

HWTEST_F(PcInputInjectionParserTest, MouseClick_CancelReleasesOnlyHeldButtonOnce, TestSize.Level1)
{
    auto context = AceType::MakeRefPtr<PcRecoveryContext>();
    {
        ClickInput input;
        input.coordinates = PointF(31.0f, 47.0f);
        input.button = "right";
        input.clickCount = 2;
        MouseClickExecutor executor(context, input);
        ASSERT_EQ(executor.ExecuteStep(), ExecutorResult::PENDING);
        executor.Cancel();
        executor.Cancel();
        EXPECT_EQ(executor.ExecuteStep(), ExecutorResult::SUCCESS);
    }
    ASSERT_EQ(context->mice.size(), 2U);
    EXPECT_EQ(context->mice[0].action, MouseAction::PRESS);
    EXPECT_EQ(context->mice[1].action, MouseAction::CANCEL);
    EXPECT_EQ(context->mice[1].button, MouseButton::RIGHT_BUTTON);
    EXPECT_EQ(context->mice[1].pressedButtons, 0);
    EXPECT_TRUE(context->mice[1].isInjected);
    EXPECT_FLOAT_EQ(context->mice[1].x, context->mice[0].x);
    EXPECT_FLOAT_EQ(context->mice[1].y, context->mice[0].y);
}

HWTEST_F(PcInputInjectionParserTest, MouseClick_CancelBetweenClicksDoesNotCancelReleasedButton, TestSize.Level1)
{
    auto context = AceType::MakeRefPtr<PcRecoveryContext>();
    ClickInput input;
    input.clickCount = 2;
    MouseClickExecutor executor(context, input);
    ASSERT_EQ(executor.ExecuteStep(), ExecutorResult::PENDING);
    ASSERT_EQ(executor.ExecuteStep(), ExecutorResult::PENDING);
    executor.Cancel();
    ASSERT_EQ(context->mice.size(), 2U);
    EXPECT_EQ(context->mice.back().action, MouseAction::RELEASE);
}

HWTEST_F(PcInputInjectionParserTest, MouseClick_ReentrantCancelDoesNotResumeInjection, TestSize.Level1)
{
    auto context = AceType::MakeRefPtr<PcRecoveryContext>();
    MouseClickExecutor executor(context, ClickInput {});
    context->onMouse = [&executor](const MouseEvent& event) {
        if (event.action == MouseAction::PRESS) {
            executor.Cancel();
        }
    };
    executor.ExecuteStep();
    executor.ExecuteStep();
    ASSERT_EQ(context->mice.size(), 2U);
    EXPECT_EQ(context->mice.back().action, MouseAction::CANCEL);
}

HWTEST_F(PcInputInjectionParserTest, MouseClick_NormalCompletionDoesNotCompensateOnDestruction, TestSize.Level1)
{
    auto context = AceType::MakeRefPtr<PcRecoveryContext>();
    {
        MouseClickExecutor executor(context, ClickInput {});
        EXPECT_EQ(executor.ExecuteStep(), ExecutorResult::PENDING);
        EXPECT_EQ(executor.ExecuteStep(), ExecutorResult::SUCCESS);
        executor.Cancel();
    }
    ASSERT_EQ(context->mice.size(), 2U);
    EXPECT_EQ(context->mice.back().action, MouseAction::RELEASE);
}

HWTEST_F(PcInputInjectionParserTest, TouchMove_CancelUsesLastInjectedPointAndRestoresDrag, TestSize.Level1)
{
    auto context = AceType::MakeRefPtr<PcRecoveryContext>();
    {
        PressAndMoveExecutor executor(context, RecoveryMoveInput(MoveInputType::TOUCH));
        ASSERT_EQ(executor.ExecuteStep(), ExecutorResult::PENDING);
        ASSERT_EQ(executor.ExecuteStep(), ExecutorResult::PENDING);
        ASSERT_TRUE(context->GetEventManager()->IsRelaxedInteractionDragSuppressed());
        executor.Cancel();
        executor.Cancel();
        EXPECT_EQ(executor.ExecuteStep(), ExecutorResult::SUCCESS);
    }
    ASSERT_EQ(context->touches.size(), 3U);
    EXPECT_EQ(context->touches.back().type, TouchType::CANCEL);
    EXPECT_FLOAT_EQ(context->touches.back().x, context->touches[1].x);
    EXPECT_FLOAT_EQ(context->touches.back().y, context->touches[1].y);
    ASSERT_FALSE(context->touches.back().pointers.empty());
    EXPECT_FALSE(context->touches.back().pointers.front().isPressed);
    EXPECT_FALSE(context->GetEventManager()->IsRelaxedInteractionDragSuppressed());
}

HWTEST_F(PcInputInjectionParserTest, MouseMove_DestructionCancelsAtLastInjectedPoint, TestSize.Level1)
{
    auto context = AceType::MakeRefPtr<PcRecoveryContext>();
    {
        PressAndMoveExecutor executor(context, RecoveryMoveInput(MoveInputType::MOUSE));
        ASSERT_EQ(executor.ExecuteStep(), ExecutorResult::PENDING);
        ASSERT_EQ(executor.ExecuteStep(), ExecutorResult::PENDING);
    }
    ASSERT_EQ(context->mice.size(), 3U);
    EXPECT_EQ(context->mice.back().action, MouseAction::CANCEL);
    EXPECT_EQ(context->mice.back().button, MouseButton::LEFT_BUTTON);
    EXPECT_EQ(context->mice.back().pressedButtons, 0);
    EXPECT_FLOAT_EQ(context->mice.back().x, context->mice[1].x);
    EXPECT_FLOAT_EQ(context->mice.back().y, context->mice[1].y);
    EXPECT_FALSE(context->GetEventManager()->IsRelaxedInteractionDragSuppressed());
}

HWTEST_F(PcInputInjectionParserTest, Move_CancelBeforeStartEmitsNothing, TestSize.Level1)
{
    for (auto type : {MoveInputType::TOUCH, MoveInputType::MOUSE}) {
        auto context = AceType::MakeRefPtr<PcRecoveryContext>();
        PressAndMoveExecutor executor(context, RecoveryMoveInput(type));
        executor.Cancel();
        executor.ExecuteStep();
        EXPECT_TRUE(context->touches.empty());
        EXPECT_TRUE(context->mice.empty());
        EXPECT_FALSE(context->GetEventManager()->IsRelaxedInteractionDragSuppressed());
    }
}

HWTEST_F(PcInputInjectionParserTest, Move_NormalCompletionRestoresDragWithoutCancel, TestSize.Level1)
{
    for (auto type : {MoveInputType::TOUCH, MoveInputType::MOUSE}) {
        auto context = AceType::MakeRefPtr<PcRecoveryContext>();
        bool releasedWhileSuppressed = false;
        context->onTouch = [&context, &releasedWhileSuppressed](const TouchEvent& event) {
            if (event.type == TouchType::UP) {
                releasedWhileSuppressed = context->GetEventManager()->IsRelaxedInteractionDragSuppressed();
            }
        };
        context->onMouse = [&context, &releasedWhileSuppressed](const MouseEvent& event) {
            if (event.action == MouseAction::RELEASE) {
                releasedWhileSuppressed = context->GetEventManager()->IsRelaxedInteractionDragSuppressed();
            }
        };
        {
            PressAndMoveExecutor executor(context, RecoveryMoveInput(type));
            for (size_t index = 0; index < 10U; ++index) {
                auto result = executor.ExecuteStep();
                EXPECT_EQ(result, index == 9U ? ExecutorResult::SUCCESS : ExecutorResult::PENDING);
            }
            EXPECT_FALSE(context->GetEventManager()->IsRelaxedInteractionDragSuppressed());
            executor.Cancel();
        }
        EXPECT_TRUE(releasedWhileSuppressed);
        if (type == MoveInputType::TOUCH) {
            ASSERT_EQ(context->touches.size(), 10U);
            EXPECT_EQ(context->touches.back().type, TouchType::UP);
        } else {
            ASSERT_EQ(context->mice.size(), 10U);
            EXPECT_EQ(context->mice.back().action, MouseAction::RELEASE);
        }
    }
}

HWTEST_F(PcInputInjectionParserTest, Move_DenyHasNoPartialInputOrDragSuppression, TestSize.Level1)
{
    auto context = AceType::MakeRefPtr<PcRecoveryContext>();
    PressAndMoveExecutor executor(context, RecoveryMoveInput(MoveInputType::TOUCH));
    executor.SetDenyCallback([](const PointF&, const RelaxedCommandIdentity&) { return true; }, {});
    EXPECT_EQ(executor.ExecuteStep(), ExecutorResult::FAILED);
    EXPECT_EQ(executor.GetFailureState(), ExecutionState::CONTROL_DENIED);
    executor.Cancel();
    EXPECT_TRUE(context->touches.empty());
    EXPECT_FALSE(context->GetEventManager()->IsRelaxedInteractionDragSuppressed());
}

HWTEST_F(PcInputInjectionParserTest, PressKey_CancelReleasesOnlyActuallyPressedKeysOnce, TestSize.Level1)
{
    auto context = AceType::MakeRefPtr<PcRecoveryContext>();
    {
        KeyInput input {{KeyCode::KEY_CTRL_LEFT, KeyCode::KEY_A}, 1000};
        PressKeyExecutor executor(context, input);
        ASSERT_EQ(executor.ExecuteStep(), ExecutorResult::PENDING);
        executor.Cancel();
        executor.Cancel();
        executor.ExecuteStep();
    }
    ASSERT_EQ(context->keys.size(), 2U);
    EXPECT_EQ(context->keys[0].action, KeyAction::DOWN);
    EXPECT_EQ(context->keys[1].action, KeyAction::UP);
    EXPECT_EQ(context->keys[1].code, KeyCode::KEY_CTRL_LEFT);
    EXPECT_TRUE(context->keys[1].isInjected);
    const std::vector<KeyCode> ctrlOnly {KeyCode::KEY_CTRL_LEFT};
    EXPECT_EQ(context->keys[0].pressedCodes, ctrlOnly);
    EXPECT_TRUE(context->keys[1].pressedCodes.empty());
    context->GetEventManager()->SetPressedKeyCodes(context->keys[1].pressedCodes);
    EXPECT_FALSE(context->GetEventManager()->IsKeyInPressed(KeyCode::KEY_CTRL_LEFT));
    EXPECT_FALSE(context->GetEventManager()->IsKeyInPressed(KeyCode::KEY_A));
}

HWTEST_F(PcInputInjectionParserTest, PressKey_DestructionReleasesHeldCombinationInReverseOrder, TestSize.Level1)
{
    auto context = AceType::MakeRefPtr<PcRecoveryContext>();
    {
        KeyInput input {{KeyCode::KEY_CTRL_LEFT, KeyCode::KEY_A}, 1000};
        PressKeyExecutor executor(context, input);
        ASSERT_EQ(executor.ExecuteStep(), ExecutorResult::PENDING);
        ASSERT_EQ(executor.ExecuteStep(), ExecutorResult::PENDING);
    }
    ASSERT_EQ(context->keys.size(), 4U);
    EXPECT_EQ(context->keys[2].action, KeyAction::UP);
    EXPECT_EQ(context->keys[2].code, KeyCode::KEY_A);
    EXPECT_EQ(context->keys[3].action, KeyAction::UP);
    EXPECT_EQ(context->keys[3].code, KeyCode::KEY_CTRL_LEFT);
    const std::vector<KeyCode> ctrlOnly {KeyCode::KEY_CTRL_LEFT};
    const std::vector<KeyCode> combination {KeyCode::KEY_CTRL_LEFT, KeyCode::KEY_A};
    EXPECT_EQ(context->keys[0].pressedCodes, ctrlOnly);
    EXPECT_EQ(context->keys[1].pressedCodes, combination);
    EXPECT_EQ(context->keys[2].pressedCodes, ctrlOnly);
    EXPECT_TRUE(context->keys[3].pressedCodes.empty());
}

HWTEST_F(PcInputInjectionParserTest, PressKey_ReentrantCancelOnFirstDownReleasesAndStops, TestSize.Level1)
{
    auto context = AceType::MakeRefPtr<PcRecoveryContext>();
    KeyInput input {{KeyCode::KEY_CTRL_LEFT, KeyCode::KEY_A}, 0};
    PressKeyExecutor executor(context, input);
    context->onKey = [&executor](const KeyEvent& event) {
        if (event.action == KeyAction::DOWN) {
            executor.Cancel();
        }
    };
    executor.ExecuteStep();
    executor.Cancel();
    ASSERT_EQ(context->keys.size(), 2U);
    EXPECT_EQ(context->keys.back().action, KeyAction::UP);
    EXPECT_EQ(context->keys.back().code, KeyCode::KEY_CTRL_LEFT);
    const std::vector<KeyCode> ctrlOnly {KeyCode::KEY_CTRL_LEFT};
    EXPECT_EQ(context->keys.front().pressedCodes, ctrlOnly);
    EXPECT_TRUE(context->keys.back().pressedCodes.empty());
}

HWTEST_F(PcInputInjectionParserTest, PressKey_NormalCompletionClearsPressedCodes, TestSize.Level1)
{
    auto context = AceType::MakeRefPtr<PcRecoveryContext>();
    auto eventManager = context->GetEventManager();
    context->onKey = [&eventManager](const KeyEvent& event) {
        eventManager->SetPressedKeyCodes(event.pressedCodes);
    };
    {
        KeyInput input {{KeyCode::KEY_CTRL_LEFT, KeyCode::KEY_A}, 0};
        PressKeyExecutor executor(context, input);
        EXPECT_EQ(executor.ExecuteStep(), ExecutorResult::SUCCESS);
        executor.Cancel();
    }
    ASSERT_EQ(context->keys.size(), 4U);
    const std::vector<KeyCode> ctrlOnly {KeyCode::KEY_CTRL_LEFT};
    const std::vector<KeyCode> combination {KeyCode::KEY_CTRL_LEFT, KeyCode::KEY_A};
    EXPECT_EQ(context->keys[0].pressedCodes, ctrlOnly);
    EXPECT_EQ(context->keys[1].pressedCodes, combination);
    EXPECT_EQ(context->keys[2].pressedCodes, ctrlOnly);
    EXPECT_TRUE(context->keys[3].pressedCodes.empty());
    EXPECT_FALSE(eventManager->IsKeyInPressed(KeyCode::KEY_CTRL_LEFT));
    EXPECT_FALSE(eventManager->IsKeyInPressed(KeyCode::KEY_A));
}

HWTEST_F(PcInputInjectionParserTest, PressKey_ReportsFailureWhenNoEventHandled, TestSize.Level1)
{
    // Emulate a pipeline that swallows every key event: the sequence must report failure
    // instead of a silent success (key_executor uses the same handled-based semantics).
    auto context = AceType::MakeRefPtr<PcRecoveryContext>();
    context->keyHandled = false;
    KeyInput input {{KeyCode::KEY_A}, 0};
    PressKeyExecutor executor(context, input);
    EXPECT_EQ(executor.ExecuteStep(), ExecutorResult::FAILED);
    EXPECT_EQ(executor.GetFailureState(), ExecutionState::FAILED);
    ASSERT_EQ(context->keys.size(), 2U);
    EXPECT_EQ(context->keys.front().action, KeyAction::DOWN);
    EXPECT_TRUE(context->keys.front().msg.empty());
}

HWTEST_F(PcInputInjectionParserTest, PressKey_ReentrantCancelDuringUpReleasesRemainingKeysOnce, TestSize.Level1)
{
    auto context = AceType::MakeRefPtr<PcRecoveryContext>();
    auto eventManager = context->GetEventManager();
    KeyInput input {{KeyCode::KEY_CTRL_LEFT, KeyCode::KEY_SHIFT_LEFT, KeyCode::KEY_A}, 0};
    PressKeyExecutor executor(context, input);
    context->onKey = [&executor, &eventManager](const KeyEvent& event) {
        eventManager->SetPressedKeyCodes(event.pressedCodes);
        if (event.action == KeyAction::UP) {
            executor.Cancel();
        }
    };
    EXPECT_EQ(executor.ExecuteStep(), ExecutorResult::SUCCESS);
    executor.Cancel();
    ASSERT_EQ(context->keys.size(), 6U);
    EXPECT_EQ(context->keys[3].code, KeyCode::KEY_A);
    EXPECT_EQ(context->keys[4].code, KeyCode::KEY_SHIFT_LEFT);
    EXPECT_EQ(context->keys[5].code, KeyCode::KEY_CTRL_LEFT);
    const std::vector<KeyCode> modifiers {KeyCode::KEY_CTRL_LEFT, KeyCode::KEY_SHIFT_LEFT};
    const std::vector<KeyCode> ctrlOnly {KeyCode::KEY_CTRL_LEFT};
    EXPECT_EQ(context->keys[3].pressedCodes, modifiers);
    EXPECT_EQ(context->keys[4].pressedCodes, ctrlOnly);
    EXPECT_TRUE(context->keys[5].pressedCodes.empty());
    EXPECT_FALSE(eventManager->IsKeyInPressed(KeyCode::KEY_CTRL_LEFT));
    EXPECT_FALSE(eventManager->IsKeyInPressed(KeyCode::KEY_SHIFT_LEFT));
    EXPECT_FALSE(eventManager->IsKeyInPressed(KeyCode::KEY_A));
}

HWTEST_F(PcInputInjectionParserTest, Click_DenyProtectsStrictAndSimulatedFallback, TestSize.Level1)
{
    auto context = AceType::MakeRefPtr<PcRecoveryContext>();
    auto deny = [](const PointF&, const RelaxedCommandIdentity&) { return true; };
    StrictClickExecutor strict(context, PointF(10.0f, 20.0f));
    strict.SetDenyCallback(deny, {});
    EXPECT_EQ(strict.ExecuteStep(), ExecutorResult::FAILED);
    EXPECT_EQ(strict.GetFailureState(), ExecutionState::CONTROL_DENIED);
    SimulateTouchExecutor simulated(context, PointF(10.0f, 20.0f));
    simulated.SetDenyCallback(deny, {});
    EXPECT_EQ(simulated.ExecuteStep(), ExecutorResult::FAILED);
    EXPECT_EQ(simulated.GetFailureState(), ExecutionState::CONTROL_DENIED);
    EXPECT_TRUE(context->touches.empty());
}

HWTEST_F(PcInputInjectionParserTest, PcExecutors_ReportUnavailableContextWithoutChangingLegacyClick, TestSize.Level1)
{
    MouseClickExecutor mouse(Context(), ClickInput {});
    PressAndMoveExecutor move(Context(), RecoveryMoveInput(MoveInputType::TOUCH));
    PressKeyExecutor key(Context(), KeyInput {{KeyCode::KEY_A}, 0});
    for (auto executor : std::vector<BaseExecutor*> {&mouse, &move, &key}) {
        EXPECT_EQ(executor->ExecuteStep(), ExecutorResult::FAILED);
        EXPECT_EQ(executor->GetFailureState(), ExecutionState::CONTEXT_UNAVAILABLE);
    }
    StrictClickExecutor strict(Context(), PointF());
    SimulateTouchExecutor simulated(Context(), PointF());
    EXPECT_EQ(strict.ExecuteStep(), ExecutorResult::FAILED);
    EXPECT_EQ(strict.GetFailureState(), ExecutionState::TARGET_NOT_FOUND);
    EXPECT_EQ(simulated.ExecuteStep(), ExecutorResult::FAILED);
    EXPECT_EQ(simulated.GetFailureState(), ExecutionState::TARGET_NOT_FOUND);
}
namespace {
constexpr size_t TOUCH_INPUT = 0;
constexpr size_t AXIS_INPUT = 2;
constexpr int32_t REAL_TOUCH_EVENT_ID = 23;

const std::vector<std::string> PC_ENTRY_COMMANDS = {
    R"({"cmd":{"type":"click", "action_info":{"mouse_button":"left", "coordinates":{"x1":10, "y1":20}}}})",
    R"({"cmd":{"type":"wheel_scroll", "action_info":{"delta":120, "coordinates":{"x1":10, "y1":20}}}})",
    R"({"cmd":{"type":"press_and_move", "action_info":{"from":{"x":10, "y":20}, "to":{"x":90, "y":180}}}})",
    R"({"cmd":{"type":"press_key", "action_info":{"key":"a", "hold_ms":1000}}})",
};
} // namespace

class PcCommandEntryTest : public testing::Test {
protected:
    void SetUp() override
    {
        Reset();
    }

    void TearDown() override
    {
        events_->ClearRelaxedInteraction(ExecutionState::FAILED);
    }

    void Reset()
    {
        if (events_) {
            events_->relaxedInteractionManager_->Clear();
        }
        context_ = AceType::MakeRefPtr<PcRecoveryContext>();
        events_ = context_->GetEventManager();
        manager_ = AceType::MakeRefPtr<RelaxedInteractionManager>(WeakPtr<PipelineContext>(context_));
        events_->relaxedInteractionManager_ = manager_;
        results_.clear();
        frames_ = 0;
    }

    void Submit(const std::string& command)
    {
        events_->ProcessCommandWithResult(command, [this]() { ++frames_; },
            [this](const RelaxedCommandResult& result) { results_.push_back(result); });
    }

    void Flush()
    {
        events_->FlushRelaxedInteraction([this]() { ++frames_; });
    }

    void RealInput(size_t input, bool injected = false, bool end = false, int64_t device = 1)
    {
        if (input == TOUCH_INPUT) {
            TouchEvent event;
            event.id = REAL_TOUCH_EVENT_ID;
            event.deviceId = device;
            event.type = end ? TouchType::UP : TouchType::DOWN;
            event.isInjected = injected;
            events_->HandleRelaxedInteractionTouch(event);
        } else if (input == 1) {
            MouseEvent event;
            event.deviceId = device;
            event.action = end ? MouseAction::RELEASE : MouseAction::PRESS;
            event.button = MouseButton::LEFT_BUTTON;
            event.isInjected = injected;
            events_->HandleRelaxedInteractionMouse(event);
        } else if (input == AXIS_INPUT) {
            AxisEvent event;
            event.deviceId = device;
            event.action = end ? AxisAction::END : AxisAction::BEGIN;
            event.isInjected = injected;
            events_->HandleRelaxedInteractionAxis(event);
        } else {
            KeyEvent event(KeyCode::KEY_B, end ? KeyAction::UP : KeyAction::DOWN);
            event.deviceId = device;
            event.isInjected = injected;
            events_->HandleRelaxedInteractionKey(event);
        }
    }

    RefPtr<PcRecoveryContext> context_;
    RefPtr<EventManager> events_;
    RefPtr<RelaxedInteractionManager> manager_;
    std::vector<RelaxedCommandResult> results_;
    size_t frames_ = 0;
};

HWTEST_F(PcCommandEntryTest, StandardEntry_AppliesConflictMatrixAndCompletesSynchronously, TestSize.Level1)
{
    for (size_t command = 0; command < PC_ENTRY_COMMANDS.size(); ++command) {
        for (size_t input = 0; input < 4; ++input) {
            Reset();
            Submit(PC_ENTRY_COMMANDS[command]);
            ASSERT_TRUE(results_.empty());
            Flush();
            ASSERT_TRUE(results_.empty());
            RealInput(input);
            const bool conflicts = command == 3 ? input == 3 : input != 3;
            if (conflicts) {
                ASSERT_EQ(results_.size(), 1U) << command << ':' << input;
                EXPECT_EQ(results_.front().errorType, RelaxedCommandErrorType::USER_ACTION_PRIORITY);
                EXPECT_EQ(results_.front().errorMessage, "interrupted by user action");
                EXPECT_FALSE(manager_->HasPendingWorkflow());
                Flush();
                EXPECT_EQ(results_.size(), 1U);
            } else {
                EXPECT_TRUE(results_.empty()) << command << ':' << input;
                EXPECT_TRUE(manager_->HasPendingWorkflow());
            }
        }
    }
}

HWTEST_F(PcCommandEntryTest, StandardEntry_SplitsWheelScrollAcrossTwoFrames, TestSize.Level1)
{
    const std::string command = R"({"cmd":{"type":"wheel_scroll", "action_info":{"delta":-120,)"
        R"("direction":"horizontal", "coordinates":{"x1":10, "y1":20}}}})";
    Submit(command);
    Flush();
    // Frame 1 opens the axis session and carries the delta; END must wait for a later frame.
    ASSERT_EQ(context_->axes.size(), 2U);
    EXPECT_EQ(context_->axes[0].action, AxisAction::BEGIN);
    EXPECT_DOUBLE_EQ(context_->axes[0].horizontalAxis, 0.0);
    EXPECT_EQ(context_->axes[1].action, AxisAction::UPDATE);
    EXPECT_DOUBLE_EQ(context_->axes[1].horizontalAxis, -120.0);
    EXPECT_TRUE(context_->axes[0].isInjected);
    EXPECT_TRUE(results_.empty());
    EXPECT_TRUE(manager_->HasPendingWorkflow());
    Flush();
    ASSERT_EQ(context_->axes.size(), 3U);
    EXPECT_EQ(context_->axes[2].action, AxisAction::END);
    EXPECT_DOUBLE_EQ(context_->axes[2].horizontalAxis, 0.0);
    ASSERT_EQ(results_.size(), 1U);
    EXPECT_TRUE(results_.front().success);
    EXPECT_FALSE(manager_->HasPendingWorkflow());
}

HWTEST_F(PcCommandEntryTest, StandardEntry_ClosesAxisSessionWhenWheelScrollIsInterrupted, TestSize.Level1)
{
    Submit(PC_ENTRY_COMMANDS[1]);
    Flush();
    ASSERT_EQ(context_->axes.size(), 2U);
    RealInput(0); // a real touch preempts the wheel between the two frames
    ASSERT_EQ(results_.size(), 1U);
    EXPECT_EQ(results_.front().errorType, RelaxedCommandErrorType::USER_ACTION_PRIORITY);
    ASSERT_EQ(context_->axes.size(), 3U);
    EXPECT_EQ(context_->axes.back().action, AxisAction::END);
    EXPECT_FALSE(manager_->HasPendingWorkflow());
}

HWTEST_F(PcCommandEntryTest, StandardEntry_RejectsEveryCommandWhileRealOperationIsActive, TestSize.Level1)
{
    for (size_t input = 0; input < 4; ++input) {
        for (const auto& command : PC_ENTRY_COMMANDS) {
            Reset();
            // Track the DOWN even when no relaxed manager has been created yet.
            events_->relaxedInteractionManager_.Reset();
            RealInput(input);
            events_->relaxedInteractionManager_ = manager_;
            Submit(command);
            ASSERT_EQ(results_.size(), 1U);
            EXPECT_EQ(results_.front().errorType, RelaxedCommandErrorType::USER_ACTION_PRIORITY);
            EXPECT_EQ(frames_, 0U);
            EXPECT_FALSE(manager_->HasPendingWorkflow());
            EXPECT_TRUE(context_->touches.empty());
            EXPECT_TRUE(context_->mice.empty());
            EXPECT_TRUE(context_->keys.empty());
            RealInput(input, false, true);
            Submit(command);
            EXPECT_EQ(results_.size(), 1U);
            EXPECT_TRUE(manager_->HasPendingWorkflow());
            EXPECT_EQ(frames_, 1U);
        }
    }
}

HWTEST_F(PcCommandEntryTest, StandardEntry_IgnoresInjectedInputAndHover, TestSize.Level1)
{
    Submit(PC_ENTRY_COMMANDS[2]);
    Flush();
    for (size_t input = 0; input < 4; ++input) {
        RealInput(input, true);
    }
    MouseEvent hover;
    hover.action = MouseAction::MOVE;
    events_->HandleRelaxedInteractionMouse(hover);
    hover.action = MouseAction::HOVER;
    events_->HandleRelaxedInteractionMouse(hover);
    NonPointerEvent nonKey;
    events_->HandleRelaxedInteractionKey(nonKey);
    EXPECT_TRUE(results_.empty());
    EXPECT_TRUE(manager_->HasPendingWorkflow());
    manager_->Clear();
    Submit(PC_ENTRY_COMMANDS[3]);
    EXPECT_TRUE(results_.empty());
    EXPECT_TRUE(manager_->HasPendingWorkflow());
}

HWTEST_F(PcCommandEntryTest, StandardEntry_IgnoresAgentControllerInput, TestSize.Level1)
{
    Submit(PC_ENTRY_COMMANDS[2]);
    Flush();
    TouchEvent agentTouch;
    agentTouch.id = REAL_TOUCH_EVENT_ID;
    agentTouch.type = TouchType::DOWN;
    agentTouch.isAgent = true;
    events_->HandleRelaxedInteractionTouch(agentTouch);
    MouseEvent agentMouse;
    agentMouse.action = MouseAction::PRESS;
    agentMouse.button = MouseButton::LEFT_BUTTON;
    agentMouse.isAgent = true;
    events_->HandleRelaxedInteractionMouse(agentMouse);
    // Agent controller input neither interrupts the running command...
    EXPECT_TRUE(results_.empty());
    EXPECT_TRUE(manager_->HasPendingWorkflow());
    manager_->Clear();
    // ...nor is tracked as an active real operation for admission.
    agentTouch.type = TouchType::UP;
    events_->HandleRelaxedInteractionTouch(agentTouch);
    agentMouse.action = MouseAction::RELEASE;
    events_->HandleRelaxedInteractionMouse(agentMouse);
    Submit(PC_ENTRY_COMMANDS[0]);
    EXPECT_TRUE(results_.empty());
    EXPECT_TRUE(manager_->HasPendingWorkflow());
    manager_->Clear();
}

HWTEST_F(PcCommandEntryTest, StandardEntry_TracksIndependentDevicesUntilAllInputsEnd, TestSize.Level1)
{
    for (size_t input = 0; input < 4; ++input) {
        Reset();
        RealInput(input, false, false, 1);
        RealInput(input, false, false, 2);
        RealInput(input, false, true, 1);
        Submit(PC_ENTRY_COMMANDS[0]);
        ASSERT_EQ(results_.size(), 1U);
        EXPECT_EQ(results_.front().errorType, RelaxedCommandErrorType::USER_ACTION_PRIORITY);
        RealInput(input, false, true, 2);
        Submit(PC_ENTRY_COMMANDS[0]);
        EXPECT_EQ(results_.size(), 1U);
        EXPECT_TRUE(manager_->HasPendingWorkflow());
    }
}

HWTEST_F(PcCommandEntryTest, StandardEntry_MouseCancelWithoutButtonClearsHeldState, TestSize.Level1)
{
    RealInput(1);
    MouseEvent cancel;
    cancel.deviceId = 1;
    cancel.action = MouseAction::CANCEL;
    events_->HandleRelaxedInteractionMouse(cancel);
    Submit(PC_ENTRY_COMMANDS[0]);
    EXPECT_TRUE(results_.empty());
    EXPECT_TRUE(manager_->HasPendingWorkflow());
}

HWTEST_F(PcCommandEntryTest, StandardEntry_CompensatesBeforeCallbackAndRejectsReentrantAdmission, TestSize.Level1)
{
    size_t completed = 0;
    events_->ProcessCommandWithResult(PC_ENTRY_COMMANDS[2], []() {},
        [this, &completed](const RelaxedCommandResult& result) {
            ++completed;
            EXPECT_EQ(result.errorType, RelaxedCommandErrorType::USER_ACTION_PRIORITY);
            ASSERT_FALSE(context_->touches.empty());
            EXPECT_EQ(context_->touches.back().type, TouchType::CANCEL);
            EXPECT_FALSE(events_->IsRelaxedInteractionDragSuppressed());
            Submit(PC_ENTRY_COMMANDS[0]);
        });
    Flush();
    Flush();
    ASSERT_EQ(context_->touches.size(), 2U);
    RealInput(0);
    EXPECT_EQ(completed, 1U);
    ASSERT_EQ(results_.size(), 1U);
    EXPECT_EQ(results_.front().errorType, RelaxedCommandErrorType::USER_ACTION_PRIORITY);
    EXPECT_FALSE(manager_->HasPendingWorkflow());
    EXPECT_FLOAT_EQ(context_->touches.back().x, context_->touches[1].x);
    RealInput(0, false, true);
    Flush();
    EXPECT_EQ(completed, 1U);
}

HWTEST_F(PcCommandEntryTest, StandardEntry_DenyIsTerminalBeforeClickFallback, TestSize.Level1)
{
    size_t checks = 0;
    manager_->SetDenyCallback([&checks](const PointF&, const RelaxedCommandIdentity&) {
        ++checks;
        return true;
    });
    Submit(R"({"cmd":{"type":"click", "action_info":{"coordinates":{"x1":10, "y1":20}}}})");
    Flush();
    ASSERT_EQ(results_.size(), 1U);
    EXPECT_EQ(results_.front().errorType, RelaxedCommandErrorType::CONTROL_DENIED);
    EXPECT_EQ(results_.front().errorMessage, "control denied by policy");
    EXPECT_EQ(checks, 1U);
    EXPECT_FALSE(manager_->HasPendingWorkflow());
    Flush();
    EXPECT_EQ(results_.size(), 1U);
    EXPECT_EQ(checks, 1U);
    EXPECT_TRUE(context_->touches.empty());
}

HWTEST_F(PcCommandEntryTest, StandardEntry_DistinguishesContextAndTargetErrors, TestSize.Level1)
{
    for (const auto& command : PC_ENTRY_COMMANDS) {
        Reset();
        manager_->context_.Reset();
        Submit(command);
        Flush();
        ASSERT_EQ(results_.size(), 1U);
        EXPECT_EQ(results_.front().errorType, RelaxedCommandErrorType::CONTEXT_UNAVAILABLE);
        EXPECT_EQ(results_.front().errorMessage, "UI context is unavailable");
        EXPECT_TRUE(context_->touches.empty());
        EXPECT_TRUE(context_->mice.empty());
        EXPECT_TRUE(context_->keys.empty());
    }
    Reset();
    Submit(R"({"cmd":{"type":"wheel_scroll", "action_info":{"delta":120}}})");
    Flush();
    ASSERT_EQ(results_.size(), 1U);
    EXPECT_EQ(results_.front().errorType, RelaxedCommandErrorType::TARGET_UNAVAILABLE);
    EXPECT_EQ(results_.front().errorMessage, "target is unavailable");
}

HWTEST_F(PcCommandEntryTest, StandardEntry_ContextClearCompletesOnceWithoutChangingLegacyResult, TestSize.Level1)
{
    Submit(PC_ENTRY_COMMANDS[2]);
    Flush();
    manager_->context_.Reset();
    events_->ClearRelaxedInteraction(ExecutionState::TARGET_NOT_FOUND);
    events_->ClearRelaxedInteraction(ExecutionState::TARGET_NOT_FOUND);
    ASSERT_EQ(results_.size(), 1U);
    EXPECT_EQ(results_.front().errorType, RelaxedCommandErrorType::CONTEXT_UNAVAILABLE);
    EXPECT_FALSE(events_->IsRelaxedInteractionDragSuppressed());
    EXPECT_EQ(context_->touches.back().type, TouchType::CANCEL);
    Reset();
    Submit(R"({"cmd":{"type":"not_supported"}})");
    ASSERT_EQ(results_.size(), 1U);
    EXPECT_EQ(results_.front().errorType, RelaxedCommandErrorType::INVALID_COMMAND);
    EXPECT_TRUE(results_.front().errorMessage.empty());
}

HWTEST_F(PcCommandEntryTest, StandardEntry_RespectsSystemDragEnteringWithoutLocalDown, TestSize.Level1)
{
    TouchEvent touch;
    touch.type = TouchType::PULL_IN_WINDOW;
    events_->HandleRelaxedInteractionTouch(touch);
    Submit(PC_ENTRY_COMMANDS[2]);
    ASSERT_EQ(results_.size(), 1U);
    EXPECT_EQ(results_.front().errorType, RelaxedCommandErrorType::USER_ACTION_PRIORITY);
    touch.type = TouchType::PULL_OUT_WINDOW;
    events_->HandleRelaxedInteractionTouch(touch);
    Submit(PC_ENTRY_COMMANDS[2]);
    EXPECT_EQ(results_.size(), 1U);
    EXPECT_TRUE(manager_->HasPendingWorkflow());
    Reset();
    MouseEvent mouse;
    mouse.action = MouseAction::PULL_MOVE;
    events_->HandleRelaxedInteractionMouse(mouse);
    Submit(PC_ENTRY_COMMANDS[2]);
    ASSERT_EQ(results_.size(), 1U);
    EXPECT_EQ(results_.front().errorType, RelaxedCommandErrorType::USER_ACTION_PRIORITY);
    mouse.action = MouseAction::CANCEL;
    events_->HandleRelaxedInteractionMouse(mouse);
    Submit(PC_ENTRY_COMMANDS[2]);
    EXPECT_EQ(results_.size(), 1U);
    EXPECT_TRUE(manager_->HasPendingWorkflow());
}

HWTEST_F(PcCommandEntryTest, StandardEntry_DoesNotTreatGeneratedCancellationAsRealInput, TestSize.Level1)
{
    Submit(PC_ENTRY_COMMANDS[2]);
    TouchEvent touch;
    touch.type = TouchType::DOWN;
    touch.isGenerate = true;
    events_->HandleRelaxedInteractionTouch(touch);
    touch.isGenerate = false;
    touch.isFalsified = true;
    events_->HandleRelaxedInteractionTouch(touch);
    MouseEvent mouse;
    mouse.action = MouseAction::PRESS;
    mouse.button = MouseButton::LEFT_BUTTON;
    mouse.isFalsifyCancel = true;
    events_->HandleRelaxedInteractionMouse(mouse);
    AxisEvent axis;
    axis.action = AxisAction::BEGIN;
    axis.isFalsifyCancel = true;
    events_->HandleRelaxedInteractionAxis(axis);
    KeyEvent key(KeyCode::KEY_B, KeyAction::DOWN);
    key.isFalsifyCancel = true;
    events_->HandleRelaxedInteractionKey(key);
    EXPECT_TRUE(results_.empty());
    EXPECT_TRUE(manager_->HasPendingWorkflow());
    manager_->Clear();
    Submit(PC_ENTRY_COMMANDS[3]);
    EXPECT_TRUE(results_.empty());
    EXPECT_TRUE(manager_->HasPendingWorkflow());
}

HWTEST_F(PcCommandEntryTest, LegacyEntry_PreservesDeferredCompletionAndNoCallbackBehavior, TestSize.Level1)
{
    size_t completed = 0;
    events_->ProcessCommand(PC_ENTRY_COMMANDS[3], []() {}, [&completed](ExecutionState state) {
        ++completed;
        EXPECT_EQ(state, ExecutionState::INTERRUPTED_BY_USER);
    });
    NonPointerEvent nonKey;
    events_->HandleRelaxedInteractionKey(nonKey);
    EXPECT_EQ(completed, 0U);
    EXPECT_TRUE(manager_->HasPendingWorkflow());
    Flush();
    EXPECT_EQ(completed, 1U);
    EXPECT_FALSE(manager_->HasPendingWorkflow());
    events_->ProcessCommand(PC_ENTRY_COMMANDS[3], []() {});
    RealInput(0);
    EXPECT_TRUE(manager_->HasPendingWorkflow());
    EXPECT_EQ(completed, 1U);
    manager_->Clear();
}

HWTEST_F(PcCommandEntryTest, StandardEntry_TimeoutAndMalformedArgumentsReturnFixedMessages, TestSize.Level1)
{
    Submit(R"({"cmd":{"type":"wheel_scroll", "action_info":{"delta":120, "coordinates":{"x1":10}}}})");
    ASSERT_EQ(results_.size(), 1U);
    EXPECT_EQ(results_.front().errorType, RelaxedCommandErrorType::INVALID_PARAM);
    EXPECT_EQ(results_.front().errorMessage, "invalid command argument");
    EXPECT_EQ(frames_, 0U);
    Reset();
    manager_->SetCommandTimeoutMs(1);
    Submit(PC_ENTRY_COMMANDS[3]);
    Flush();
    // hold_ms=1000 extends the effective timeout to hold+margin(500)=1500ms; judge past it.
    std::this_thread::sleep_for(std::chrono::milliseconds(1600));
    Flush();
    ASSERT_EQ(results_.size(), 1U);
    EXPECT_EQ(results_.front().errorType, RelaxedCommandErrorType::TIMEOUT);
    EXPECT_EQ(results_.front().errorMessage, "command execution timed out");
    ASSERT_EQ(context_->keys.size(), 2U);
    EXPECT_EQ(context_->keys.back().action, KeyAction::UP);
}

HWTEST_F(PcInputInjectionParserTest, PressKeyParser_HomeMapsToKeyboardMoveHome, TestSize.Level1)
{
    // Keyboard Home is KEY_MOVE_HOME: KEY_HOME (1) is the system Home key that the
    // focus/caret/shortcut paths never treat as a keyboard Home press.
    auto context = AceType::MakeRefPtr<PcRecoveryContext>();
    PressKeyCommandParser parser { WeakPtr<PipelineContext>(context) };
    auto executors = parser.Parse(Parse(R"({"action_info":{"key":"home"}})"));
    ASSERT_EQ(executors.size(), 1U);
    EXPECT_EQ(executors.front()->ExecuteStep(), ExecutorResult::SUCCESS);
    ASSERT_EQ(context->keys.size(), 2U);
    EXPECT_EQ(context->keys.front().code, KeyCode::KEY_MOVE_HOME);
    EXPECT_EQ(context->keys.front().keyIntention, KeyIntention::INTENTION_HOME);
}

HWTEST_F(PcInputInjectionParserTest, SimulateTouchExecutor_RepeatsClickPerClickCount, TestSize.Level1)
{
    auto context = AceType::MakeRefPtr<PcRecoveryContext>();
    SimulateTouchExecutor executor(context, PointF(10.0f, 20.0f), 2);
    EXPECT_EQ(executor.ExecuteStep(), ExecutorResult::PENDING);
    ASSERT_EQ(context->touches.size(), 2U);
    EXPECT_EQ(context->touches[0].type, TouchType::DOWN);
    EXPECT_EQ(context->touches[1].type, TouchType::UP);
    EXPECT_EQ(executor.ExecuteStep(), ExecutorResult::SUCCESS);
    ASSERT_EQ(context->touches.size(), 4U);
    EXPECT_EQ(context->touches[2].type, TouchType::DOWN);
    EXPECT_EQ(context->touches[3].type, TouchType::UP);
    EXPECT_EQ(executor.ExecuteStep(), ExecutorResult::SUCCESS);
    EXPECT_EQ(context->touches.size(), 4U);
}

HWTEST_F(PcInputInjectionParserTest, Manager_BoundaryHoldCompletesInsteadOfTimeout, TestSize.Level1)
{
    // hold_ms == command timeout: the declared hold extends the effective timeout, so the
    // boundary hold completes with SUCCESS instead of being judged TIMEOUT.
    auto context = AceType::MakeRefPtr<PcRecoveryContext>();
    auto manager = AceType::MakeRefPtr<RelaxedInteractionManager>(WeakPtr<PipelineContext>(context));
    manager->SetCommandTimeoutMs(1000);
    NG::ExecutionState observed = NG::ExecutionState::NONE;
    EXPECT_EQ(manager->ProcessCommand(
        R"({"cmd":{"type":"press_key","action_info":{"key":"a","hold_ms":1000}}})",
        [&observed](NG::ExecutionState state) { observed = state; }, true), ProcessResult::SUCCESS);
    auto result = NG::ExecutionState::FAILED;
    for (int32_t attempt = 0; attempt < 40 && result != NG::ExecutionState::SUCCESS; ++attempt) {
        result = manager->ExecuteNextStep();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    EXPECT_EQ(result, NG::ExecutionState::SUCCESS);
    EXPECT_EQ(observed, NG::ExecutionState::SUCCESS);
}

HWTEST_F(PcInputInjectionParserTest, SpecialComponent_NodeHitProducesContractPayload, TestSize.Level1)
{
    auto frameNode = FrameNode::CreateFrameNode(
        V2::UI_EXTENSION_COMPONENT_ETS_TAG, 1234, AceType::MakeRefPtr<Pattern>());
    SimulateTouchExecutor executor(WeakPtr<PipelineContext>(), PointF(0.0f, 0.0f));
    EXPECT_TRUE(executor.HandleSpecialComponentHit(frameNode, 100.0, 200.0));
    EXPECT_EQ(executor.GetFailureState(), NG::ExecutionState::TARGET_NOT_FOUND);
    NG::SpecialComponentHitInfo info;
    NG::BaseExecutor::ConsumeSpecialComponentHit(info);
    EXPECT_TRUE(info.hit);
    EXPECT_EQ(info.nodeId, 1234);
    // Default-matrix node keeps the input coordinates after the transform.
    EXPECT_FLOAT_EQ(info.offsetX, 100.0f);
    EXPECT_FLOAT_EQ(info.offsetY, 200.0f);
    NG::BaseExecutor::ClearSpecialComponentHit();
}

HWTEST_F(PcInputInjectionParserTest, SpecialComponent_DeniedNodeReportsSecurityWithoutPayload, TestSize.Level1)
{
    // needSecurityAuth + DENY node + control permission: the security gate precedes the
    // special-component contract (SECURITY_DENIED, audited, and no hit payload is carried).
    const bool savedGranted = MockInputManager::controlComponentPermissionGranted;
    MockInputManager::controlComponentPermissionGranted = true;
    NG::UiCommandSecurityGate::SetContext(NG::CommandSecurityContext { true, 234 });
    auto frameNode = FrameNode::CreateFrameNode(
        V2::UI_EXTENSION_COMPONENT_ETS_TAG, 5678, AceType::MakeRefPtr<Pattern>());
    frameNode->SetAgentControlMode(AgentControlMode::DENY);
    SimulateTouchExecutor executor(WeakPtr<PipelineContext>(), PointF(0.0f, 0.0f));
    EXPECT_TRUE(executor.HandleSpecialComponentHit(frameNode, 100.0, 200.0));
    EXPECT_EQ(executor.GetFailureState(), NG::ExecutionState::SECURITY_DENIED);
    NG::SpecialComponentHitInfo info;
    NG::BaseExecutor::ConsumeSpecialComponentHit(info);
    EXPECT_FALSE(info.hit);
    NG::UiCommandSecurityGate::ClearContext();
    MockInputManager::controlComponentPermissionGranted = savedGranted;
}

HWTEST_F(PcInputInjectionParserTest, SimulateTouchExecutor_ReportsProducedInput, TestSize.Level1)
{
    auto context = AceType::MakeRefPtr<PcRecoveryContext>();
    SimulateTouchExecutor executor(context, PointF(10.0f, 20.0f), 2);
    EXPECT_FALSE(executor.HasProducedInput());
    EXPECT_EQ(executor.ExecuteStep(), ExecutorResult::PENDING);
    EXPECT_TRUE(executor.HasProducedInput());
}

HWTEST_F(PcInputInjectionParserTest, MouseClickExecutor_KeepsPressedTimeWithinClick, TestSize.Level1)
{
    auto context = AceType::MakeRefPtr<PcRecoveryContext>();
    ClickInput input;
    input.clickCount = 2;
    MouseClickExecutor executor(context, input);
    for (int32_t index = 0; index < 4; ++index) {
        executor.ExecuteStep();
    }
    ASSERT_EQ(context->mice.size(), 4U);
    // PRESS and RELEASE of one click share the press moment; different clicks differ.
    EXPECT_EQ(context->mice[0].pressedTime, context->mice[1].pressedTime);
    EXPECT_EQ(context->mice[2].pressedTime, context->mice[3].pressedTime);
    EXPECT_NE(context->mice[0].pressedTime, context->mice[2].pressedTime);
}

HWTEST_F(PcInputInjectionParserTest, MouseMove_KeepsPressedTimeAcrossSequence, TestSize.Level1)
{
    auto context = AceType::MakeRefPtr<PcRecoveryContext>();
    PressAndMoveExecutor executor(context, RecoveryMoveInput(MoveInputType::MOUSE));
    for (int32_t index = 0; index < 10; ++index) {
        executor.ExecuteStep();
    }
    ASSERT_EQ(context->mice.size(), 10U);
    // The whole press/move/release sequence shares one press moment.
    EXPECT_EQ(context->mice.front().pressedTime, context->mice.back().pressedTime);
    EXPECT_EQ(context->mice[4].pressedTime, context->mice.front().pressedTime);
}

} // namespace OHOS::Ace::NG

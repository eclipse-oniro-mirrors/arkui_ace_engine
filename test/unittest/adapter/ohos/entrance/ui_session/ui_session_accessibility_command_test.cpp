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

#include <limits>
#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "gtest/gtest.h"

#include "adapter/ohos/entrance/ui_session/ui_session_accessibility_command.h"
#include "base/json/json_util.h"

using namespace OHOS::Ace::UiSessionAccessibilityCommand;

namespace OHOS::Ace {

TEST(UiSessionAccessibilityCommandTest, DistinguishesActionEnvelopeAndProviderTarget)
{
    auto native = JsonUtil::ParseJsonString(
        R"({"protocol":"arkui.accessibility","version":1,"action":"ACTION_CLICK","arguments":{}})");
    ASSERT_TRUE(native && native->IsValid());
    Accessibility::ActionType action = Accessibility::ActionType::ACCESSIBILITY_ACTION_INVALID;
    ASSERT_EQ(ParseAccessibilityActionHeader(*native, action), ParseResult::SUCCESS);
    EXPECT_EQ(action, Accessibility::ActionType::ACCESSIBILITY_ACTION_CLICK);
    std::optional<int64_t> elementId;
    EXPECT_TRUE(ParseElementId(*native, elementId));
    EXPECT_FALSE(elementId.has_value());
    std::map<std::string, std::string> arguments;
    EXPECT_TRUE(ParseAccessibilityArguments(*native, action, arguments));
    EXPECT_TRUE(arguments.empty());

    auto third = JsonUtil::ParseJsonString(
        R"({"protocol":"arkui.accessibility","version":1,"elementId":"77",)"
        R"("action":"ACTION_CLICK","arguments":{}})");
    ASSERT_TRUE(third && third->IsValid());
    EXPECT_EQ(ParseAccessibilityActionHeader(*third, action), ParseResult::SUCCESS);
    EXPECT_TRUE(ParseElementId(*third, elementId));
    ASSERT_TRUE(elementId.has_value());
    EXPECT_EQ(*elementId, 77);

    const std::vector<std::string> invalid = {
        R"({"version":1,"action":"ACTION_CLICK","arguments":{}})",
        R"({"protocol":"arkui.accessibility","version":2,"action":"ACTION_CLICK","arguments":{}})",
        R"({"protocol":"arkui.accessibility","version":1,"cmd":"click",)"
        R"("action":"ACTION_CLICK","arguments":{}})",
        R"({"protocol":"arkui.accessibility","version":1,"action":"UNKNOWN","arguments":{}})",
        R"({"protocol":"arkui.accessibility","version":1,"cmd":2,"action":"ACTION_CLICK","arguments":{}})",
    };
    for (const auto& command : invalid) {
        auto parsed = JsonUtil::ParseJsonString(command);
        ASSERT_TRUE(parsed && parsed->IsValid());
        EXPECT_EQ(ParseAccessibilityActionHeader(*parsed, action), ParseResult::INVALID_COMMAND) << command;
    }
}

TEST(UiSessionAccessibilityCommandTest, ValidatesActionArgumentsAndElementId)
{
    auto parseArguments = [](const char* json, Accessibility::ActionType action) {
        auto parsed = JsonUtil::ParseJsonString(json);
        if (!parsed || !parsed->IsValid()) {
            return false;
        }
        std::map<std::string, std::string> result;
        return ParseAccessibilityArguments(*parsed, action, result);
    };
    EXPECT_TRUE(parseArguments(R"({"arguments":{"scrolltype":-1}})",
        Accessibility::ActionType::ACCESSIBILITY_ACTION_SCROLL_FORWARD));
    EXPECT_FALSE(parseArguments(R"({"arguments":{"scrolltype":2}})",
        Accessibility::ActionType::ACCESSIBILITY_ACTION_SCROLL_FORWARD));
    EXPECT_FALSE(parseArguments(R"({"arguments":{"textMoveUnit":3}})",
        Accessibility::ActionType::ACCESSIBILITY_ACTION_NEXT_TEXT));
    EXPECT_FALSE(parseArguments(R"({"arguments":{"offset":2147483648}})",
        Accessibility::ActionType::ACCESSIBILITY_ACTION_SET_CURSOR_POSITION));
    EXPECT_FALSE(parseArguments(R"({"arguments":{"spanId":-1}})",
        Accessibility::ActionType::ACCESSIBILITY_ACTION_SPAN_CLICK));
    EXPECT_FALSE(parseArguments(R"({"arguments":{"spanId":2147483648}})",
        Accessibility::ActionType::ACCESSIBILITY_ACTION_SPAN_CLICK));
    EXPECT_FALSE(parseArguments(R"({"arguments":{"accessibilityFocusScene":1}})",
        Accessibility::ActionType::ACCESSIBILITY_ACTION_ACCESSIBILITY_FOCUS));
    EXPECT_TRUE(parseArguments(R"({"arguments":{"customAction":"refresh"}})",
        Accessibility::ActionType::ACCESSIBILITY_ACTION_CUSTOM));

    const std::vector<std::string> invalidIds = {
        R"({"elementId":77})", R"({"elementId":""})", R"({"elementId":"77x"})",
        R"({"elementId":"9223372036854775808"})",
    };
    for (const auto& command : invalidIds) {
        auto parsed = JsonUtil::ParseJsonString(command);
        ASSERT_TRUE(parsed && parsed->IsValid());
        std::optional<int64_t> elementId;
        EXPECT_FALSE(ParseElementId(*parsed, elementId)) << command;
        EXPECT_FALSE(elementId.has_value());
    }
}

TEST(UiSessionAccessibilityCommandTest, AcceptsOnlyProtocolActionNames)
{
    const std::vector<std::pair<std::string, Accessibility::ActionType>> actions = {
        { "ACTION_CLICK", Accessibility::ActionType::ACCESSIBILITY_ACTION_CLICK },
        { "ACTION_LONG_CLICK", Accessibility::ActionType::ACCESSIBILITY_ACTION_LONG_CLICK },
        { "ACTION_SCROLL_FORWARD", Accessibility::ActionType::ACCESSIBILITY_ACTION_SCROLL_FORWARD },
        { "ACTION_SCROLL_BACKWARD", Accessibility::ActionType::ACCESSIBILITY_ACTION_SCROLL_BACKWARD },
        { "ACTION_FOCUS", Accessibility::ActionType::ACCESSIBILITY_ACTION_FOCUS },
        { "ACTION_CLEAR_FOCUS", Accessibility::ActionType::ACCESSIBILITY_ACTION_CLEAR_FOCUS },
        { "ACTION_ACCESSIBILITY_FOCUS", Accessibility::ActionType::ACCESSIBILITY_ACTION_ACCESSIBILITY_FOCUS },
        { "ACTION_CLEAR_ACCESSIBILITY_FOCUS",
            Accessibility::ActionType::ACCESSIBILITY_ACTION_CLEAR_ACCESSIBILITY_FOCUS },
        { "ACTION_NEXT_AT_MOVEMENT_GRANULARITY", Accessibility::ActionType::ACCESSIBILITY_ACTION_NEXT_TEXT },
        { "ACTION_PREVIOUS_AT_MOVEMENT_GRANULARITY", Accessibility::ActionType::ACCESSIBILITY_ACTION_PREVIOUS_TEXT },
        { "ACTION_SET_TEXT", Accessibility::ActionType::ACCESSIBILITY_ACTION_SET_TEXT },
        { "ACTION_COPY", Accessibility::ActionType::ACCESSIBILITY_ACTION_COPY },
        { "ACTION_PASTE", Accessibility::ActionType::ACCESSIBILITY_ACTION_PASTE },
        { "ACTION_CUT", Accessibility::ActionType::ACCESSIBILITY_ACTION_CUT },
        { "ACTION_SELECT", Accessibility::ActionType::ACCESSIBILITY_ACTION_SELECT },
        { "ACTION_CLEAR_SELECTION", Accessibility::ActionType::ACCESSIBILITY_ACTION_CLEAR_SELECTION },
        { "ACTION_SET_SELECTION", Accessibility::ActionType::ACCESSIBILITY_ACTION_SET_SELECTION },
        { "ACTION_SET_CURSOR_POSITION", Accessibility::ActionType::ACCESSIBILITY_ACTION_SET_CURSOR_POSITION },
        { "ACTION_EXEC_SUB_COMPONENT", Accessibility::ActionType::ACCESSIBILITY_ACTION_SPAN_CLICK },
        { "CUSTOM_ACTION", Accessibility::ActionType::ACCESSIBILITY_ACTION_CUSTOM },
        { "BACK", Accessibility::ActionType::ACCESSIBILITY_ACTION_BACK },
    };
    for (const auto& [name, expected] : actions) {
        auto json = JsonUtil::Create(true);
        json->Put("protocol", "arkui.accessibility");
        json->Put("version", 1);
        json->Put("action", name.c_str());
        Accessibility::ActionType actual = Accessibility::ActionType::ACCESSIBILITY_ACTION_INVALID;
        ASSERT_EQ(ParseAccessibilityActionHeader(*json, actual), ParseResult::SUCCESS) << name;
        EXPECT_EQ(actual, expected) << name;
    }
    for (const auto* unsupported : { "ACTION_NONE", "ACTION_NEXT_HTML_ITEM", "ACTION_PREVIOUS_HTML_ITEM" }) {
        auto json = JsonUtil::Create(true);
        json->Put("protocol", "arkui.accessibility");
        json->Put("version", 1);
        json->Put("action", unsupported);
        Accessibility::ActionType actual = Accessibility::ActionType::ACCESSIBILITY_ACTION_CLICK;
        EXPECT_EQ(ParseAccessibilityActionHeader(*json, actual), ParseResult::UNSUPPORTED_ACTION) << unsupported;
        EXPECT_EQ(actual, Accessibility::ActionType::ACCESSIBILITY_ACTION_INVALID) << unsupported;
    }
    for (const auto& command : {
        R"({"protocol":"arkui.accessibility","version":1,"action":"ACCESSIBILITY_ACTION_CLICK"})",
        R"({"protocol":"arkui.accessibility","version":1,"action":"GLOBAL_ACTION_BACK"})",
        R"({"protocol":"arkui.accessibility","version":"1","action":"ACTION_CLICK"})",
        R"({"protocol":"arkui.accessibility","version":1.5,"action":"ACTION_CLICK"})",
        R"({"protocol":"arkui.accessibility","version":1,"action":"ACTION_CLICK","extra":0})" }) {
        auto json = JsonUtil::ParseJsonString(command);
        Accessibility::ActionType action = Accessibility::ActionType::ACCESSIBILITY_ACTION_CLICK;
        EXPECT_EQ(ParseAccessibilityActionHeader(*json, action), ParseResult::INVALID_COMMAND) << command;
        EXPECT_EQ(action, Accessibility::ActionType::ACCESSIBILITY_ACTION_INVALID);
    }
}

TEST(UiSessionAccessibilityCommandTest, ChecksArgumentTypesAndPreservesValues)
{
    std::map<std::string, std::string> result;
    auto parse = [&result](const char* command, Accessibility::ActionType action) {
        auto json = JsonUtil::ParseJsonString(command);
        return ParseAccessibilityArguments(*json, action, result);
    };
    EXPECT_TRUE(parse(R"({"arguments":{"setText":""}})",
        Accessibility::ActionType::ACCESSIBILITY_ACTION_SET_TEXT));
    EXPECT_EQ(result.at("setText"), "");
    EXPECT_TRUE(parse(R"({"arguments":{"selectTextBegin":0,"selectTextEnd":2147483647,
        "selectTextInForWard":"forward"}})", Accessibility::ActionType::ACCESSIBILITY_ACTION_SET_SELECTION));
    EXPECT_EQ(result.at("selectTextBegin"), "0");
    EXPECT_EQ(result.at("selectTextEnd"), "2147483647");
    EXPECT_EQ(result.at("selectTextInForWard"), "forward");
    EXPECT_TRUE(parse(R"({"arguments":{"spanId":2147483647}})",
        Accessibility::ActionType::ACCESSIBILITY_ACTION_SPAN_CLICK));
    EXPECT_EQ(result.at("spanId"), "2147483647");
    EXPECT_TRUE(parse(R"({"arguments":{}})",
        Accessibility::ActionType::ACCESSIBILITY_ACTION_ACCESSIBILITY_FOCUS));
    EXPECT_TRUE(result.empty());
    EXPECT_TRUE(parse(R"({"arguments":{}})", Accessibility::ActionType::ACCESSIBILITY_ACTION_NEXT_TEXT));
    EXPECT_TRUE(parse(R"({"arguments":{"textMoveUnit":16}})",
        Accessibility::ActionType::ACCESSIBILITY_ACTION_PREVIOUS_TEXT));
    EXPECT_FALSE(parse(R"({"arguments":{"offset":0.5}})",
        Accessibility::ActionType::ACCESSIBILITY_ACTION_SET_CURSOR_POSITION));
    EXPECT_FALSE(parse(R"({"arguments":{"offset":"1"}})",
        Accessibility::ActionType::ACCESSIBILITY_ACTION_SET_CURSOR_POSITION));
    EXPECT_FALSE(parse(R"({"arguments":{"offset":true}})",
        Accessibility::ActionType::ACCESSIBILITY_ACTION_SET_CURSOR_POSITION));
    EXPECT_FALSE(parse(R"({"arguments":{"setText":null}})",
        Accessibility::ActionType::ACCESSIBILITY_ACTION_SET_TEXT));
    EXPECT_FALSE(parse(R"({"arguments":{"customAction":""}})",
        Accessibility::ActionType::ACCESSIBILITY_ACTION_CUSTOM));
    EXPECT_FALSE(parse(R"({"arguments":{"selectTextBegin":0}})",
        Accessibility::ActionType::ACCESSIBILITY_ACTION_SET_SELECTION));
    EXPECT_FALSE(parse(R"({"arguments":{"selectTextBegin":0,"selectTextEnd":1,
        "selectTextInForWard":true}})", Accessibility::ActionType::ACCESSIBILITY_ACTION_SET_SELECTION));
    EXPECT_FALSE(parse(R"({"arguments":{"accessibilityFocusScene":1}})",
        Accessibility::ActionType::ACCESSIBILITY_ACTION_ACCESSIBILITY_FOCUS));
    EXPECT_FALSE(parse(R"({"arguments":{"setText":"value"}})",
        Accessibility::ActionType::ACCESSIBILITY_ACTION_CLICK));
    EXPECT_FALSE(parse(R"({"arguments":[]})", Accessibility::ActionType::ACCESSIBILITY_ACTION_CLICK));
    EXPECT_FALSE(parse(R"({})", Accessibility::ActionType::ACCESSIBILITY_ACTION_CLICK));
}

TEST(UiSessionAccessibilityCommandTest, ProviderIdUsesFullSignedInt64AndResets)
{
    std::optional<int64_t> id;
    for (auto value : { std::numeric_limits<int64_t>::min(), std::numeric_limits<int64_t>::max() }) {
        auto json = JsonUtil::Create(true);
        json->Put("elementId", std::to_string(value).c_str());
        EXPECT_TRUE(ParseElementId(*json, id));
        EXPECT_EQ(id, value);
    }
    auto native = JsonUtil::ParseJsonString("{}");
    EXPECT_TRUE(ParseElementId(*native, id));
    EXPECT_FALSE(id.has_value());
    for (const auto& command : { R"({"elementId":" 77"})", R"({"elementId":"77 "})",
        R"({"elementId":"+77"})", R"({"elementId":"7.7"})", R"({"elementId":"-9223372036854775809"})" }) {
        auto json = JsonUtil::ParseJsonString(command);
        id = 77;
        EXPECT_FALSE(ParseElementId(*json, id));
        EXPECT_FALSE(id.has_value());
    }
}

} // namespace OHOS::Ace

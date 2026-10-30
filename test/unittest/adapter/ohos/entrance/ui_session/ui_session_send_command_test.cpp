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

#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include "gtest/gtest.h"
#include "adapter/ohos/entrance/ui_session/include/ui_session_trace.h"
#include "cJSON.h"
#include "interfaces/inner_api/ui_session/ui_content_proxy.h"
#include "interfaces/inner_api/ui_session/ui_report_proxy.h"
#include "adapter/ohos/entrance/ui_session/ui_session_batch_command.h"
#include "adapter/ohos/entrance/ui_session/ui_session_command_result.h"
#include "core/components_ng/pattern/command/command_validation.h"
#include "core/components_ng/pattern/pattern.h"

namespace OHOS::Ace {
// Host tests have no system trace backend.
UiSessionScopedTrace::UiSessionScopedTrace(const char*, ...) {}
UiSessionScopedTrace::~UiSessionScopedTrace() = default;

namespace {
constexpr int32_t COMMAND_NODE_ID = 42;
constexpr int EXPECTED_REQUEST_SENDS = 2;
constexpr int EXPECTED_FINISH_CALLBACKS = 2;
constexpr int32_t COMMAND_TIMEOUT_MS = 5000;
constexpr int32_t BEFORE_COMMAND_TIMEOUT_MS = COMMAND_TIMEOUT_MS - 1;
constexpr int CONCURRENT_WORKERS = 8;

class CommandRemote : public IRemoteObject {
public:
    std::function<int32_t(uint32_t, MessageParcel&, MessageParcel&, MessageOption&)> send;
    int32_t SendRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option) override
    {
        return send ? send(code, data, reply, option) : 0;
    }
};

class SendCommandTest : public testing::Test {
public:
    void SetUp() override
    {
        MessageParcel::SetDefaultPayloadStorage(true);
        remote = std::make_shared<CommandRemote>();
        proxy = std::make_unique<UIContentServiceProxy>(sptr<IRemoteObject>(remote));
        report = std::make_shared<UiReportStub>();
        handler = std::make_shared<AppExecFwk::EventHandler>();
        report->SetEventHandler(handler);
        proxy->report_ = report;
    }
    void TearDown() override
    {
        proxy.reset();
        report = nullptr;
        MessageParcel::SetDefaultPayloadStorage(false);
    }
    std::shared_ptr<CommandRemote> remote;
    std::unique_ptr<UIContentServiceProxy> proxy;
    sptr<UiReportStub> report;
    std::shared_ptr<AppExecFwk::EventHandler> handler;
};

TEST_F(SendCommandTest, OverloadsSharePendingAndLegacyParcelIsUnchanged)
{
    int sends = 0;
    uint64_t id = 0;
    remote->send = [&sends, &id](uint32_t code, MessageParcel& data, MessageParcel&, MessageOption& option) {
        EXPECT_EQ(data.ReadInterfaceToken(), IUiContentService::GetDescriptor());
        if (sends++ == 0) {
            EXPECT_EQ(option.GetFlags(), MessageOption::TF_SYNC);
            EXPECT_EQ(code, IUiContentService::SENDCOMMAND_EVENT_WITH_FINISH_CALLBACK);
            EXPECT_TRUE(data.ReadUint64(id));
            EXPECT_NE(id, 0u);
        } else {
            EXPECT_EQ(code, IUiContentService::SENDCOMMAND_EVENT);
            EXPECT_EQ(option.GetFlags(), MessageOption::TF_SYNC);
        }
        EXPECT_EQ(data.ReadInt32(), COMMAND_NODE_ID);
        EXPECT_EQ(data.ReadString(), R"({"cmd":"click"})");
        return 0;
    };
    int count = 0;
    EXPECT_EQ(proxy->SendCommand(COMMAND_NODE_ID, R"({"cmd":"click"})", [&count](const std::string&) { ++count; }),
        NO_ERROR);
    EXPECT_EQ(proxy->SendCommand("{}", [](const std::string&) {}), LAST_UNFINISH);
    EXPECT_EQ(proxy->SendCommand(COMMAND_NODE_ID, R"({"cmd":"click"})"), NO_ERROR);
    EXPECT_EQ(sends, EXPECTED_REQUEST_SENDS);
    EXPECT_EQ(count, 0);
    report->ReportCommandResult(id, R"({"result":"SUCCESS"})");
    EXPECT_EQ(count, 1);
    EXPECT_FALSE(report->sendCommandFinishCallback_);
    EXPECT_EQ(report->sendCommandRequestId_, 0u);
    remote->send = {};
    ASSERT_EQ(proxy->SendCommand("{}", [&count](const std::string&) { ++count; }), NO_ERROR);
    report->ReportCommandResult(report->sendCommandRequestId_, R"({"result":"SUCCESS"})");
    EXPECT_EQ(count, EXPECTED_FINISH_CALLBACKS);
    EXPECT_FALSE(report->sendCommandFinishCallback_);
}

TEST_F(SendCommandTest, EmptyCallbackAndSendFailureDoNotRemainPending)
{
    int count = 0;
    auto callback = [&count](const std::string&) { ++count; };
    EXPECT_EQ(proxy->SendCommand(1, "{}", {}), PARAM_INVALID);
    EXPECT_EQ(proxy->SendCommand("{}", {}), PARAM_INVALID);
    uint64_t invalidId = 1;
    EXPECT_FALSE(report->RegisterSendCommandFinishCallback({}, invalidId));
    EXPECT_EQ(invalidId, 0u);
    remote->send = [](uint32_t, MessageParcel&, MessageParcel&, MessageOption&) { return -1; };
    EXPECT_EQ(proxy->SendCommand("{}", callback), REPLY_ERROR);
    EXPECT_EQ(proxy->SendCommand(1, "{}", callback), REPLY_ERROR);
    EXPECT_FALSE(report->sendCommandFinishCallback_);
    handler->SetPostTaskFailure(true);
    EXPECT_EQ(proxy->SendCommand("{}", callback), FAILED);
    EXPECT_FALSE(report->sendCommandFinishCallback_);
    EXPECT_EQ(count, 0);
    report->SetEventHandler(nullptr);
    EXPECT_FALSE(report->PostSendCommandCallbackRemoveTask(1));
}

TEST_F(SendCommandTest, TimeoutOccursAtFiveSecondsAndLateResultsAreIsolated)
{
    int oldCount = 0;
    int newCount = 0;
    ASSERT_EQ(proxy->SendCommand("{}", [&oldCount](const std::string&) { ++oldCount; }), NO_ERROR);
    auto oldId = report->sendCommandRequestId_;
    handler->AdvanceTime(BEFORE_COMMAND_TIMEOUT_MS);
    EXPECT_EQ(proxy->SendCommand("{}", [](const std::string&) {}), LAST_UNFINISH);
    handler->AdvanceTime(1);
    ASSERT_EQ(proxy->SendCommand("{}", [&newCount](const std::string&) { ++newCount; }), NO_ERROR);
    auto newId = report->sendCommandRequestId_;
    EXPECT_NE(oldId, newId);
    report->ReportCommandResult(oldId, "old");
    EXPECT_EQ(oldCount, 0);
    EXPECT_EQ(newCount, 0);
    report->ReportCommandResult(newId, "new");
    report->ReportCommandResult(newId, "duplicate");
    EXPECT_EQ(newCount, 1);
}

TEST_F(SendCommandTest, CallbackCanReenterAndItsNewTimeoutIsPreserved)
{
    int next = FAILED;
    auto callback = [this, &next](const std::string&) {
        next = proxy->SendCommand("{}", [](const std::string&) {});
    };
    ASSERT_EQ(proxy->SendCommand("{}", callback), NO_ERROR);
    auto first = report->sendCommandRequestId_;
    report->ReportCommandResult(first, "done");
    EXPECT_EQ(next, NO_ERROR);
    auto second = report->sendCommandRequestId_;
    report->HandleSendCommandCallbackTimeout(first);
    EXPECT_EQ(report->sendCommandRequestId_, second);
    handler->AdvanceTime(COMMAND_TIMEOUT_MS);
    EXPECT_FALSE(report->sendCommandFinishCallback_);
}

TEST_F(SendCommandTest, DeathNotificationReleasesProxyAndPendingCallback)
{
    int count = 0;
    auto lifetime = std::make_shared<int>(0);
    std::weak_ptr<int> weakLifetime = lifetime;
    ASSERT_EQ(proxy->SendCommand("{}", [lifetime, &count](const std::string&) { ++count; }), NO_ERROR);
    lifetime.reset();
    EXPECT_FALSE(weakLifetime.expired());
    wptr<UiReportStub> weakReport = report;
    report = nullptr;
    UiContentProxyRecipient recipient([this]() { proxy.reset(); });
    recipient.OnRemoteDied(wptr<IRemoteObject>(sptr<IRemoteObject>(remote)));
    EXPECT_TRUE(weakReport.promote() == nullptr);
    EXPECT_TRUE(weakLifetime.expired());
    handler->AdvanceTime(COMMAND_TIMEOUT_MS);
    EXPECT_EQ(count, 0);
}

TEST_F(SendCommandTest, NewConnectionAfterProxyReleaseIgnoresPreviousRequest)
{
    int count = 0;
    ASSERT_EQ(proxy->SendCommand("{}", [&count](const std::string&) { ++count; }), NO_ERROR);
    auto oldId = report->sendCommandRequestId_;
    wptr<UiReportStub> oldReport = report;
    report = nullptr;
    proxy.reset();
    ASSERT_TRUE(oldReport.promote() == nullptr);
    handler->AdvanceTime(1);
    proxy = std::make_unique<UIContentServiceProxy>(sptr<IRemoteObject>(remote));
    ASSERT_EQ(proxy->Connect([](std::string) {}, handler), NO_ERROR);
    report = proxy->report_;
    ASSERT_EQ(proxy->SendCommand("{}", [&count](const std::string&) { ++count; }), NO_ERROR);
    auto newId = report->sendCommandRequestId_;
    EXPECT_NE(oldId, newId);
    handler->AdvanceTime(BEFORE_COMMAND_TIMEOUT_MS);
    report->ReportCommandResult(oldId, "late on new connection");
    EXPECT_EQ(count, 0);
    EXPECT_EQ(report->sendCommandRequestId_, newId);
    report->ReportCommandResult(newId, "new report result");
    EXPECT_EQ(count, 1);
    handler->AdvanceTime(1);
    EXPECT_EQ(count, 1);
}

TEST_F(SendCommandTest, ResultProxyAndStubRoundTripRetainsJsonAndRequestId)
{
    auto reportRemote = std::make_shared<CommandRemote>();
    reportRemote->send = [this](uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option) {
        EXPECT_EQ(code, ReportService::REPORT_COMMAND_RESULT);
        EXPECT_EQ(option.GetFlags(), MessageOption::TF_ASYNC);
        return report->OnRemoteRequest(code, data, reply, option);
    };
    UiReportProxy resultProxy { sptr<IRemoteObject>(reportRemote) };
    const std::string expected = R"({"result":"FAILED", "errorType":"INVALID_PARAM", "errorMessage":"测试"})";
    UiReportProxy { nullptr }.ReportCommandResult(1, expected);
    int count = 0;
    uint64_t requestId = 0;
    ASSERT_TRUE(report->RegisterSendCommandFinishCallback([&expected, &count](const std::string& actual) {
        EXPECT_EQ(actual, expected);
        ++count;
    }, requestId));
    resultProxy.ReportCommandResult(requestId + 1, expected);
    EXPECT_EQ(count, 0);
    resultProxy.ReportCommandResult(requestId, expected);
    resultProxy.ReportCommandResult(requestId, expected);
    EXPECT_EQ(count, 1);
}

TEST_F(SendCommandTest, MalformedResultParcelCannotCompletePendingRequest)
{
    uint64_t requestId = 0;
    int count = 0;
    ASSERT_TRUE(report->RegisterSendCommandFinishCallback([&count](const std::string&) { ++count; }, requestId));
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(ReportService::GetDescriptor());
    data.WriteUint64(requestId);
    EXPECT_EQ(report->OnRemoteRequest(ReportService::REPORT_COMMAND_RESULT, data, reply, option), FAILED);
    EXPECT_EQ(count, 0);
    EXPECT_TRUE(report->sendCommandFinishCallback_);
}

TEST_F(SendCommandTest, ConcurrentRegistrationAndDuplicateResultsAreSingleShot)
{
    std::atomic<int> registered { 0 };
    std::atomic<int> completed { 0 };
    std::atomic<uint64_t> acceptedId { 0 };
    std::vector<std::thread> workers;
    for (int index = 0; index < CONCURRENT_WORKERS; ++index) {
        workers.emplace_back([this, &registered, &completed, &acceptedId]() {
            uint64_t id = 0;
            if (report->RegisterSendCommandFinishCallback([&completed](const std::string&) { ++completed; }, id)) {
                ++registered;
                acceptedId = id;
            }
        });
    }
    for (auto& worker : workers) { worker.join(); }
    EXPECT_EQ(registered, 1);
    workers.clear();
    for (int index = 0; index < CONCURRENT_WORKERS; ++index) {
        workers.emplace_back([this, &acceptedId]() { report->ReportCommandResult(acceptedId, "done"); });
    }
    for (auto& worker : workers) { worker.join(); }
    EXPECT_EQ(completed, 1);
}

TEST(CommandResultTest, TypedParametersCannotSilentlyUseLegacyDefaultValues)
{
    for (const auto& command : {
        R"({"cmd":"setText", "params":{"value":123}})",
        R"({"cmd":"selectText", "selectionStart":"0", "selectionEnd":1})",
        R"({"cmd":"onToggleChange", "params":{"isOn":"true"}})",
        R"({"cmd":"setVideoPlaybackSpeed", "value":999})",
        R"({"cmd":"changeIndex", "params":{"index":"garbage"}})",
        R"({"cmd":"onRatingChange", "params":{"value":"5"}})",
        R"({"cmd":"setDatePickerTime", "params":{"year":2026.5,"month":1,"day":1}})",
        R"({"cmd":"setTimePickerTime", "params":{"hour":"1", "minute":0,"second":0}})",
        R"({"cmd":"alertDialogButtonClick", "params":{"buttonIndex":"0"}})",
        R"({"cmd":"actionSheetClick", "params":{"clickType":"invalid", "index":0}})",
        R"({"cmd":"onSelect", "params":{"index":"0"}})" }) {
        auto json = JsonUtil::ParseJsonString(command);
        EXPECT_FALSE(NG::ValidateCommandParameters(json, json->GetString("cmd")));
    }
    auto json = JsonUtil::ParseJsonString(R"({"cmd":"setText", "params":{"value":""}})");
    EXPECT_TRUE(NG::ValidateCommandParameters(json, "setText"));
    json = JsonUtil::ParseJsonString(R"({"cmd":"setVideoPlaybackSpeed", "value":0.125})");
    EXPECT_TRUE(NG::ValidateCommandParameters(json, "setVideoPlaybackSpeed"));
}

TEST(CommandResultTest, PickerAndOptionalParametersRetainTheirContract)
{
    for (const auto* command : {
        R"({"cmd":"setDatePickerDialogTime", "params":)"
            R"({"year":2026, "month":1, "day":1, "hour":0, "minute":0, "second":0}})",
        R"({"cmd":"actionMenuClick", "params":{}})",
        R"({"cmd":"checked"})",
        R"({"cmd":"checked", "params":null})",
        R"({"cmd":"onSelect", "params":{"value":""}})",
        R"({"cmd":"onSelect", "params":{"index":0}})",
        R"({"cmd":"deleteText", "params":{}})" }) {
        SCOPED_TRACE(command);
        auto json = JsonUtil::ParseJsonString(command);
        EXPECT_TRUE(NG::ValidateCommandParameters(json, json->GetString("cmd")));
    }
    for (const auto* command : {
        R"({"cmd":"setDatePickerDialogTime", "params":{"year":2026, "month":1, "day":1}})",
        R"({"cmd":"setDatePickerDialogTime", "params":{"hour":0, "minute":0, "second":0}})",
        R"({"cmd":"actionMenuClick"})",
        R"({"cmd":"onSelect", "params":{}})",
        R"({"cmd":"onSelect", "params":{"value":false, "index":0}})",
        R"({"cmd":"checked", "params":{"value":0}})" }) {
        SCOPED_TRACE(command);
        auto json = JsonUtil::ParseJsonString(command);
        EXPECT_FALSE(NG::ValidateCommandParameters(json, json->GetString("cmd")));
    }
}

TEST(CommandResultTest, NumericAndIndexBoundariesRetainTheirContract)
{
    for (const auto* command : {
        R"({"cmd":"setVideoPlaybackSpeed", "value":0.125})",
        R"({"cmd":"setVideoPlaybackSpeed", "value":8.0})",
        R"({"cmd":"changeIndex", "params":{"index":"0"}})",
        R"({"cmd":"changeIndex", "params":{"index":"-0"}})",
        R"({"cmd":"changeIndex", "params":{"index":"0002"}})",
        R"({"cmd":"changeIndex", "params":{"index":"2147483647"}})",
        R"({"cmd":"changeIndex", "params":{"index":"0002147483647"}})",
        R"({"cmd":"change", "params":{"type":"forward"}})",
        R"({"cmd":"change", "params":{"type":"index", "index":"-0"}})",
        R"({"cmd":"change", "params":{"type":"index", "index":"2147483647"}})",
        R"({"cmd":"change", "params":{"type":"index", "index":"0"}})" }) {
        SCOPED_TRACE(command);
        auto json = JsonUtil::ParseJsonString(command);
        EXPECT_TRUE(NG::ValidateCommandParameters(json, json->GetString("cmd")));
    }
    for (const auto* command : {
        R"({"cmd":"setVideoPlaybackSpeed", "value":0.124})",
        R"({"cmd":"setVideoPlaybackSpeed", "value":8.001})",
        R"({"cmd":"changeIndex", "params":{"index":"-1"}})",
        R"({"cmd":"changeIndex", "params":{"index":"2147483648"}})",
        R"({"cmd":"changeIndex", "params":{"index":"1x"}})",
        R"({"cmd":"changeIndex", "params":{"index":""}})",
        R"({"cmd":"changeIndex", "params":{"index":"-"}})",
        R"({"cmd":"changeIndex", "params":{"index":" 1"}})",
        R"({"cmd":"changeIndex", "params":{"index":"1 "}})",
        R"({"cmd":"changeIndex", "params":{"index":"\t1"}})",
        R"({"cmd":"changeIndex", "params":{"index":"+1"}})",
        R"({"cmd":"changeIndex", "params":{"index":"+0"}})",
        R"({"cmd":"changeIndex", "params":{"index":"1.0"}})",
        R"({"cmd":"changeIndex", "params":{"index":"1e2"}})",
        R"({"cmd":"changeIndex", "params":{"index":"0x10"}})",
        R"({"cmd":"changeIndex", "params":{"index":"9223372036854775808"}})",
        R"({"cmd":"changeIndex", "params":{"index":"-9223372036854775809"}})",
        R"({"cmd":"changeIndex", "params":{"index":1}})",
        R"({"cmd":"change", "params":{"type":"index"}})",
        R"({"cmd":"change", "params":{"type":"index", "index":" 1"}})",
        R"({"cmd":"change", "params":{"type":"index", "index":"-1"}})",
        R"({"cmd":"change", "params":{"type":"index", "index":"2147483648"}})",
        R"({"cmd":"setCaretPosition", "position":2147483648})",
        R"({"cmd":"setCaretPosition", "position":0.5})" }) {
        SCOPED_TRACE(command);
        auto json = JsonUtil::ParseJsonString(command);
        EXPECT_FALSE(NG::ValidateCommandParameters(json, json->GetString("cmd")));
    }
}

TEST(CommandResultTest, JsonContractAndUtf8Boundaries)
{
    EXPECT_EQ(SerializeCommandResult(NG::CommandResult::Success()), R"({"result":"SUCCESS"})");
    for (auto type : { NG::CommandErrorType::NODE_NOT_FOUND, NG::CommandErrorType::INVALID_COMMAND,
        NG::CommandErrorType::INVALID_PARAM, NG::CommandErrorType::EXECUTE_FAIL }) {
        for (auto length : { 0, 127, 128, 129 }) {
            auto json = SerializeCommandResult(NG::CommandResult::Error(type, std::string(length, 'a')));
            auto root = cJSON_Parse(json.c_str());
            ASSERT_NE(root, nullptr);
            EXPECT_EQ(cJSON_GetArraySize(root), 3);
            EXPECT_STREQ(cJSON_GetObjectItemCaseSensitive(root, "result")->valuestring, "FAILED");
            auto message = cJSON_GetObjectItemCaseSensitive(root, "errorMessage")->valuestring;
            EXPECT_EQ(std::string(message).size(), length > 128 ? 0u : static_cast<size_t>(length));
            cJSON_Delete(root);
        }
    }
    for (const auto& message : { std::string("中文😀"), std::string(122, 'a') + "中文",
        std::string(123, 'a') + "中文" }) {
        auto json = SerializeCommandResult(NG::CommandResult::Error(NG::CommandErrorType::EXECUTE_FAIL, message));
        auto root = cJSON_Parse(json.c_str());
        ASSERT_NE(root, nullptr);
        EXPECT_STREQ(cJSON_GetObjectItemCaseSensitive(root, "errorMessage")->valuestring,
            message.size() > 128 ? "" : message.c_str());
        cJSON_Delete(root);
    }
    auto json = SerializeCommandResult(NG::CommandResult::Error(NG::CommandErrorType::EXECUTE_FAIL,
        std::string("a\0b\n\"\\", 7)));
    EXPECT_NE(json.find("\\u0000"), std::string::npos);
    auto root = cJSON_Parse(json.c_str());
    ASSERT_NE(root, nullptr);
    cJSON_Delete(root);
}

TEST(CommandResultTest, BatchReportsEveryItemInOrderAndContinuesAfterFailure)
{
    const auto parsed = UiSessionBatchCommand::Parse(R"({"cmd":"batch", "commands":{
        "1":{"cmd":"setText", "params":{"value":"a"}},
        "2":{"cmd":"setText", "params":{"value":"b"}},
        "x":{"cmd":"setText", "params":{"value":"c"}},
        "1":{"cmd":"setText", "params":{"value":"duplicate"}},
        "3":{"cmd":"wrong"}, "4":{"cmd":"setText", "params":{}},
        "5":{"cmd":"setText", "params":{"value":"e"}},
        "6":{"cmd":"setText", "params":{"value":"f"}}}})");
    std::vector<int> executed;
    auto json = UiSessionBatchCommand::ExecuteWithResult(parsed, [&executed](int32_t nodeId) -> BatchResolvedTarget {
        if (nodeId == 2) { return { .status = BatchTargetStatus::NOT_FOUND }; }
        if (nodeId == 5) { return { .status = BatchTargetStatus::UNSUPPORTED_TYPE }; }
        BatchResolvedTarget target;
        target.status = BatchTargetStatus::READY;
        target.execute = [&executed, nodeId](const std::string&) {
            executed.push_back(nodeId);
            return nodeId != 6;
        };
        return target;
    });
    EXPECT_EQ(executed, (std::vector<int> { 1, 6 }));
    auto root = cJSON_Parse(json.c_str());
    ASSERT_NE(root, nullptr);
    EXPECT_STREQ(cJSON_GetObjectItemCaseSensitive(root, "result")->valuestring, "FAILED");
    auto results = cJSON_GetObjectItemCaseSensitive(root, "results");
    ASSERT_EQ(cJSON_GetArraySize(results), 8);
    const char* expected[] = { "", "NODE_NOT_FOUND", "INVALID_PARAM", "INVALID_PARAM", "INVALID_COMMAND",
        "INVALID_PARAM", "INVALID_COMMAND", "EXECUTE_FAIL" };
    for (int index = 1; index < static_cast<int>(std::size(expected)); ++index) {
        EXPECT_STREQ(cJSON_GetObjectItemCaseSensitive(cJSON_GetArrayItem(results, index), "errorType")->valuestring,
            expected[index]);
    }
    EXPECT_STREQ(cJSON_GetObjectItemCaseSensitive(cJSON_GetArrayItem(results, 2), "nodeKey")->valuestring, "x");
    cJSON_Delete(root);
}

TEST(CommandResultTest, BatchInvalidEnvelopeExecutesNothingAndAllSuccessIsAggregated)
{
    for (const auto& command : { R"({"cmd":"batch", "commands":{}})", R"({"cmd":"batch", "commands":[]})" }) {
        auto parsed = UiSessionBatchCommand::Parse(command);
        auto json = UiSessionBatchCommand::ExecuteWithResult(parsed, [](int32_t) -> BatchResolvedTarget {
            ADD_FAILURE() << "invalid batch executed";
            return {};
        });
        EXPECT_NE(json.find("INVALID_PARAM"), std::string::npos);
    }
    auto parsed = UiSessionBatchCommand::Parse(R"({"cmd":"batch", "commands":{
        "7":{"cmd":"setText", "params":{"value":"ok"}}}})");
    auto json = UiSessionBatchCommand::ExecuteWithResult(parsed, [](int32_t) -> BatchResolvedTarget {
        BatchResolvedTarget target;
        target.status = BatchTargetStatus::READY;
        target.execute = [](const std::string&) { return true; };
        return target;
    });
    EXPECT_EQ(json, "{\"result\":\"SUCCESS\",\"results\":[{\"nodeId\":7,\"result\":\"SUCCESS\"}]}");
}
} // namespace
} // namespace OHOS::Ace

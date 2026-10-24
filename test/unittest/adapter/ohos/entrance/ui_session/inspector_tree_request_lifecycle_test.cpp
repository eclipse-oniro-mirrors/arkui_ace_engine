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

#include <utility>
#include <vector>

#include "gtest/gtest.h"
#include "interfaces/inner_api/ui_session/ui_content_stub_impl.h"
#include "interfaces/inner_api/ui_session/ui_report_stub.h"
#include "message_parcel.h"

#include "adapter/ohos/entrance/ui_session/include/large_string_ashmem.h"

namespace OHOS::Ace {
TEST(InspectorTreeTask06, LateReportAndCleanupCannotConsumeReplacement)
{
    sptr<UiReportStub> stub = new UiReportStub();
    InspectorTreeRequestToken oldToken;
    InspectorTreeRequestToken newToken;
    int oldCalls = 0;
    int newCalls = 0;
    ASSERT_TRUE(stub->RegisterInspectorTreeRequest([&](std::string, int32_t, bool) { ++oldCalls; }, oldToken));
    ASSERT_FALSE(stub->RegisterInspectorTreeRequest([&](std::string, int32_t, bool) { ++newCalls; }, newToken));
    stub->UnregisterInspectorTreeRequest(oldToken);
    ASSERT_TRUE(stub->RegisterInspectorTreeRequest([&](std::string, int32_t, bool) { ++newCalls; }, newToken));
    EXPECT_NE(oldToken.generation, newToken.generation);
    stub->ReportInspectorTreeForRequest("old", 1, true, oldToken);
    stub->UnregisterInspectorTreeRequest(oldToken);
    EXPECT_EQ(oldCalls, 0);
    EXPECT_EQ(newCalls, 0);
    stub->ReportInspectorTreeForRequest("new", 1, true, newToken);
    EXPECT_EQ(newCalls, 1);
    stub->ReportInspectorTreeForRequest("duplicate", 1, true, newToken);
    EXPECT_EQ(newCalls, 1);
}

TEST(InspectorTreeTask06, ConnectionEpochAndCallbackReentrancy)
{
    sptr<UiReportStub> oldStub = new UiReportStub();
    sptr<UiReportStub> newStub = new UiReportStub();
    InspectorTreeRequestToken oldToken;
    InspectorTreeRequestToken newToken;
    InspectorTreeRequestToken reentrantToken;
    bool registered = false;
    ASSERT_TRUE(oldStub->RegisterInspectorTreeRequest([](std::string, int32_t, bool) {}, oldToken));
    ASSERT_TRUE(newStub->RegisterInspectorTreeRequest(
        [&](std::string, int32_t, bool) {
            registered = newStub->RegisterInspectorTreeRequest([](std::string, int32_t, bool) {}, reentrantToken);
        },
        newToken));
    EXPECT_NE(oldToken.connectionEpoch, newToken.connectionEpoch);
    newStub->ReportInspectorTreeForRequest("old connection", 1, true, oldToken);
    EXPECT_FALSE(registered);
    newStub->ReportInspectorTreeForRequest("new connection", 1, true, newToken);
    EXPECT_TRUE(registered);
    newStub->UnregisterInspectorTreeRequest(newToken);
    InspectorTreeRequestToken overlap;
    EXPECT_FALSE(newStub->RegisterInspectorTreeRequest([](std::string, int32_t, bool) {}, overlap));
    newStub->UnregisterInspectorTreeRequest(reentrantToken);
}
TEST(InspectorTreeTask06, SharedMemoryRejectsOversizeAndOverread)
{
    sptr<LargeStringAshmem> memory = new LargeStringAshmem();
    EXPECT_FALSE(memory->WriteToAshmem("inspector-test", "abc", 4));
    EXPECT_FALSE(memory->WriteToAshmem("inspector-test", "", 100 * 1024 * 1024 + 1));
}
namespace {
class InspectorRemote : public IRemoteObject {
public:
    bool IsProxyObject() const override
    {
        return true;
    }
    bool AddDeathRecipient(const sptr<DeathRecipient>& value) override
    {
        recipient = value;
        return true;
    }
    bool RemoveDeathRecipient(const sptr<DeathRecipient>&) override
    {
        recipient = nullptr;
        return true;
    }
    sptr<DeathRecipient> recipient;
};

class InspectorWindowService : public UIContentServiceStubImpl {
public:
    using UiContentStub::GetInspectorReport;
    using UiContentStub::SaveInspectorReport;
};
} // namespace

TEST(InspectorTreeTask06, SameCallerKeepsSeparateWindowConnections)
{
    InspectorWindowService firstWindow;
    InspectorWindowService secondWindow;
    sptr<IRemoteObject> first = new UiReportStub();
    sptr<IRemoteObject> second = new UiReportStub();
    sptr<IRemoteObject> replacement = new UiReportStub();
    constexpr int32_t caller = 42;
    ASSERT_TRUE(firstWindow.SaveInspectorReport(first, caller));
    ASSERT_TRUE(secondWindow.SaveInspectorReport(second, caller));
    auto pendingFirstRequest = firstWindow.GetInspectorReport(caller);
    EXPECT_EQ(pendingFirstRequest.GetRefPtr(), first.GetRefPtr());
    EXPECT_EQ(secondWindow.GetInspectorReport(caller).GetRefPtr(), second.GetRefPtr());
    ASSERT_TRUE(firstWindow.SaveInspectorReport(replacement, caller));
    EXPECT_EQ(firstWindow.GetInspectorReport(caller).GetRefPtr(), replacement.GetRefPtr());
    EXPECT_EQ(secondWindow.GetInspectorReport(caller).GetRefPtr(), second.GetRefPtr());
    EXPECT_EQ(pendingFirstRequest.GetRefPtr(), first.GetRefPtr());
    EXPECT_EQ(firstWindow.GetInspectorReport(caller + 1), nullptr);
}

TEST(InspectorTreeTask06, WindowCallbacksRouteFullAndVisibleRequestsIndependently)
{
    InspectorWindowService mainWindow;
    InspectorWindowService subWindow;
    std::vector<std::pair<int32_t, bool>> requests;
    auto bindWindow = [&](InspectorWindowService& service, int32_t windowId) {
        service.SetInspectorFunction([&, windowId](bool visible, ParamConfig config) {
            requests.emplace_back(windowId, visible);
            EXPECT_EQ(config.inspectorRoute.token.connectionEpoch, 3u);
            EXPECT_EQ(config.inspectorRoute.token.generation, 7u);
            EXPECT_TRUE(config.withResponseRegion);
            EXPECT_TRUE(config.withCustomContent);
            ASSERT_TRUE(config.inspectorReporter);
            config.inspectorReporter(std::to_string(windowId));
        });
    };
    bindWindow(mainWindow, 101);
    bindWindow(subWindow, 202);
    ParamConfig config;
    config.inspectorRoute.token = { 3, 7 };
    config.withResponseRegion = true;
    config.withCustomContent = true;
    std::vector<std::string> reports;
    config.inspectorReporter = [&](const std::string& value) { reports.push_back(value); };
    EXPECT_EQ(mainWindow.GetInspectorTree(nullptr, config), NO_ERROR);
    EXPECT_EQ(subWindow.GetInspectorTree(nullptr, config), NO_ERROR);
    EXPECT_EQ(subWindow.GetVisibleInspectorTree(nullptr, config), NO_ERROR);
    EXPECT_EQ(mainWindow.GetVisibleInspectorTree(nullptr, config), NO_ERROR);
    EXPECT_EQ(requests,
        (std::vector<std::pair<int32_t, bool>> { { 101, false }, { 202, false }, { 202, true }, { 101, true } }));
    EXPECT_EQ(reports, (std::vector<std::string> { "101", "202", "202", "101" }));
}

TEST(InspectorTreeTask06, RetiredDeathNotificationDoesNotEraseReplacement)
{
    InspectorWindowService window;
    auto* oldObject = new InspectorRemote();
    auto* newObject = new InspectorRemote();
    sptr<IRemoteObject> oldReport = oldObject;
    sptr<IRemoteObject> newReport = newObject;
    ASSERT_TRUE(window.SaveInspectorReport(oldReport, 7));
    auto retiredDeath = oldObject->recipient;
    ASSERT_TRUE(window.SaveInspectorReport(newReport, 7));
    wptr<IRemoteObject> empty(static_cast<IRemoteObject*>(nullptr));
    retiredDeath->OnRemoteDied(empty);
    EXPECT_EQ(window.GetInspectorReport(7).GetRefPtr(), newReport.GetRefPtr());
    auto currentDeath = newObject->recipient;
    currentDeath->OnRemoteDied(empty);
    EXPECT_EQ(window.GetInspectorReport(7), nullptr);
}
} // namespace OHOS::Ace

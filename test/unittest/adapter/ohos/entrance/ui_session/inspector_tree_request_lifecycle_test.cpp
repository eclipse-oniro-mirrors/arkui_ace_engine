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
#define private public
#include "interfaces/inner_api/ui_session/ui_report_stub.h"
#undef private
#include "message_parcel.h"

#include "adapter/ohos/entrance/ui_session/include/large_string_ashmem.h"

namespace OHOS::Ace {
TEST(InspectorTreeTask06, LateReportAndCleanupCannotConsumeReplacement)
{
    sptr<UiReportStub> stub = new UiReportStub();
    uint64_t oldToken = 0;
    uint64_t newToken = 0;
    int oldCalls = 0;
    int newCalls = 0;
    ASSERT_TRUE(stub->RegisterGetInspectorTreeCallback([&](std::string, int32_t, bool) { ++oldCalls; }, &oldToken));
    ASSERT_FALSE(stub->RegisterGetInspectorTreeCallback([&](std::string, int32_t, bool) { ++newCalls; }, &newToken));
    stub->UnregisterGetInspectorTreeCallback(oldToken);
    ASSERT_TRUE(stub->RegisterGetInspectorTreeCallback([&](std::string, int32_t, bool) { ++newCalls; }, &newToken));
    EXPECT_NE(oldToken, newToken);
    stub->ReportInspectorTreeForRequest("old", 1, true, oldToken);
    stub->UnregisterGetInspectorTreeCallback(oldToken);
    EXPECT_EQ(oldCalls, 0);
    EXPECT_EQ(newCalls, 0);
    stub->ReportInspectorTreeForRequest("new", 1, true, newToken);
    EXPECT_EQ(newCalls, 1);
    stub->ReportInspectorTreeForRequest("duplicate", 1, true, newToken);
    EXPECT_EQ(newCalls, 1);
}

TEST(InspectorTreeTask06, ReconnectAndSerialCallbackReentrancy)
{
    sptr<UiReportStub> oldStub = new UiReportStub();
    sptr<UiReportStub> newStub = new UiReportStub();
    uint64_t oldToken = 0;
    uint64_t newToken = 0;
    uint64_t reentrantToken = 0;
    bool registered = false;
    ASSERT_TRUE(oldStub->RegisterGetInspectorTreeCallback([](std::string, int32_t, bool) {}, &oldToken));
    ASSERT_TRUE(newStub->RegisterGetInspectorTreeCallback(
        [&](std::string, int32_t, bool) {
            registered = newStub->RegisterGetInspectorTreeCallback([](std::string, int32_t, bool) {}, &reentrantToken);
        },
        &newToken));
    EXPECT_NE(oldToken, newToken);
    newStub->ReportInspectorTreeForRequest("old connection", 1, true, oldToken);
    EXPECT_FALSE(registered);
    newStub->ReportInspectorTreeForRequest("new connection", 1, true, newToken);
    EXPECT_TRUE(registered);
    newStub->UnregisterGetInspectorTreeCallback(newToken);
    uint64_t overlap = 0;
    EXPECT_FALSE(newStub->RegisterGetInspectorTreeCallback([](std::string, int32_t, bool) {}, &overlap));
    newStub->UnregisterGetInspectorTreeCallback(reentrantToken);
}

TEST(InspectorTreeTask06, SharedMemoryRejectsOversize)
{
    sptr<LargeStringAshmem> memory = new LargeStringAshmem();
    EXPECT_FALSE(memory->WriteToAshmem("inspector-test", "", 100 * 1024 * 1024 + 1));
}

TEST(InspectorTreeTask06, WindowCallbacksHandleSerialFullAndVisibleRequests)
{
    UIContentServiceStubImpl mainWindow;
    UIContentServiceStubImpl subWindow;
    std::vector<std::pair<int32_t, bool>> requests;
    auto bindWindow = [&](UIContentServiceStubImpl& service, int32_t windowId) {
        service.SetInspectorFunction([&, windowId](bool visible, ParamConfig config) {
            requests.emplace_back(windowId, visible);
            EXPECT_EQ(config.inspectorRequestId, requests.size());
            EXPECT_TRUE(config.withResponseRegion);
            EXPECT_TRUE(config.withCustomContent);
        });
    };
    bindWindow(mainWindow, 101);
    bindWindow(subWindow, 202);
    ParamConfig config;
    config.inspectorRequestId = 1;
    config.withResponseRegion = true;
    config.withCustomContent = true;
    EXPECT_EQ(mainWindow.GetInspectorTree(nullptr, config), NO_ERROR);
    ++config.inspectorRequestId;
    EXPECT_EQ(subWindow.GetInspectorTree(nullptr, config), NO_ERROR);
    ++config.inspectorRequestId;
    EXPECT_EQ(subWindow.GetVisibleInspectorTree(nullptr, config), NO_ERROR);
    ++config.inspectorRequestId;
    EXPECT_EQ(mainWindow.GetVisibleInspectorTree(nullptr, config), NO_ERROR);
    EXPECT_EQ(requests,
        (std::vector<std::pair<int32_t, bool>> { { 101, false }, { 202, false }, { 202, true }, { 101, true } }));
}

TEST(InspectorTreeTask06, ExistingTimeoutOnlyClearsItsOwnRequest)
{
    sptr<UiReportStub> stub = new UiReportStub();
    uint64_t first = 0;
    uint64_t second = 0;
    int calls = 0;
    ASSERT_TRUE(stub->RegisterGetInspectorTreeCallback([](std::string, int32_t, bool) {}, &first));
    stub->HandleInspectorTreeCallbackTimeout(first);
    ASSERT_TRUE(stub->RegisterGetInspectorTreeCallback([&](std::string, int32_t, bool) { ++calls; }, &second));
    stub->HandleInspectorTreeCallbackTimeout(first);
    stub->ReportInspectorTreeForRequest("late", 1, true, first);
    EXPECT_EQ(calls, 0);
    stub->ReportInspectorTreeForRequest("current", 1, true, second);
    EXPECT_EQ(calls, 1);
}

TEST(InspectorTreeTask06, ExistingRegistrationAndLegacyReportEntryRemainUsable)
{
    sptr<UiReportStub> stub = new UiReportStub();
    int calls = 0;
    auto callback = [&](std::string, int32_t, bool) { ++calls; };
    ASSERT_TRUE(stub->RegisterGetInspectorTreeCallback(callback));
    ASSERT_FALSE(stub->RegisterGetInspectorTreeCallback(callback));
    stub->ReportInspectorTreeValue("first", 1, true);
    EXPECT_EQ(calls, 1);
    ASSERT_TRUE(stub->RegisterGetInspectorTreeCallback(callback));
    stub->UnregisterGetInspectorTreeCallback();
    stub->ReportInspectorTreeValue("discarded", 1, true);
    EXPECT_EQ(calls, 1);
}
} // namespace OHOS::Ace

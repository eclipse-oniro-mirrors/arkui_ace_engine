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
TEST(InspectorTreeTask06, CallbackRemainsOccupiedUntilLastPart)
{
    sptr<UiReportStub> stub = new UiReportStub();
    int firstCalls = 0;
    int nextCalls = 0;
    auto nextCallback = [&](std::string, int32_t, bool) { ++nextCalls; };
    ASSERT_TRUE(stub->RegisterGetInspectorTreeCallback([&](std::string, int32_t, bool) { ++firstCalls; }));
    ASSERT_FALSE(stub->RegisterGetInspectorTreeCallback(nextCallback));
    stub->ReportInspectorTreeValue("part", 1, false);
    EXPECT_EQ(firstCalls, 1);
    EXPECT_EQ(nextCalls, 0);
    ASSERT_FALSE(stub->RegisterGetInspectorTreeCallback(nextCallback));
    stub->ReportInspectorTreeValue("last", 2, true);
    EXPECT_EQ(firstCalls, 2);
    stub->ReportInspectorTreeValue("duplicate", 2, true);
    EXPECT_EQ(firstCalls, 2);
    ASSERT_TRUE(stub->RegisterGetInspectorTreeCallback(nextCallback));
    stub->ReportInspectorTreeValue("next", 1, true);
    EXPECT_EQ(nextCalls, 1);
}

TEST(InspectorTreeTask06, CompletedCallbackCanRegisterNextSerialRequest)
{
    sptr<UiReportStub> stub = new UiReportStub();
    bool registered = false;
    int nextCalls = 0;
    ASSERT_TRUE(stub->RegisterGetInspectorTreeCallback([&](std::string, int32_t, bool) {
        registered = stub->RegisterGetInspectorTreeCallback([&](std::string, int32_t, bool) { ++nextCalls; });
    }));
    stub->ReportInspectorTreeValue("first", 1, true);
    EXPECT_TRUE(registered);
    EXPECT_FALSE(stub->RegisterGetInspectorTreeCallback([](std::string, int32_t, bool) {}));
    stub->ReportInspectorTreeValue("next", 1, true);
    EXPECT_EQ(nextCalls, 1);
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
            EXPECT_TRUE(config.withResponseRegion);
            EXPECT_TRUE(config.withCustomContent);
        });
    };
    bindWindow(mainWindow, 101);
    bindWindow(subWindow, 202);
    ParamConfig config;
    config.withResponseRegion = true;
    config.withCustomContent = true;
    EXPECT_EQ(mainWindow.GetInspectorTree(nullptr, config), NO_ERROR);
    EXPECT_EQ(subWindow.GetInspectorTree(nullptr, config), NO_ERROR);
    EXPECT_EQ(subWindow.GetVisibleInspectorTree(nullptr, config), NO_ERROR);
    EXPECT_EQ(mainWindow.GetVisibleInspectorTree(nullptr, config), NO_ERROR);
    EXPECT_EQ(requests,
        (std::vector<std::pair<int32_t, bool>> { { 101, false }, { 202, false }, { 202, true }, { 101, true } }));
}

TEST(InspectorTreeTask06, ExistingTimeoutClearsCallbackAndAllowsNextRequest)
{
    sptr<UiReportStub> stub = new UiReportStub();
    int calls = 0;
    auto callback = [&](std::string, int32_t, bool) { ++calls; };
    ASSERT_TRUE(stub->RegisterGetInspectorTreeCallback(callback));
    stub->HandleInspectorTreeCallbackTimeout();
    EXPECT_EQ(calls, 0);
    stub->ReportInspectorTreeValue("discarded", 1, true);
    EXPECT_EQ(calls, 0);
    ASSERT_TRUE(stub->RegisterGetInspectorTreeCallback(callback));
    stub->ReportInspectorTreeValue("current", 1, true);
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

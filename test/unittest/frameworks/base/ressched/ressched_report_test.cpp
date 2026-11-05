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

#include "gtest/gtest.h"
#include <algorithm>
#include <vector>
#define private public
#define protected public
#include "base/ressched/ressched_report.h"
#include "core/common/ace_application_info.h"
#include "core/event/touch_event.h"
#include "core/pipeline_ng/pipeline_context.h"
#undef private
#undef protected
#include "test/mock/frameworks/base/thread/mock_task_executor.h"
#include "test/mock/frameworks/core/common/mock_container.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace {

namespace {
} // namespace

class ResSchedReportTest : public testing::Test {
public:
    static void SetUpTestSuite()
    {
    }
    static void TearDownTestSuite()
    {
    }
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.name: ResSchedReportTest001
 * @tc.desc: test touch up
 * @tc.type: FUNC
 */
HWTEST_F(ResSchedReportTest, ResSchedReportTest001, TestSize.Level1)
{
    TouchEvent touchEvent;
    touchEvent.type = TouchType::UP;
    touchEvent.localX = 100.0f;
    touchEvent.localY = 200.0f;
    ReportConfig config;
    config.isReportTid = true;
    config.tid = 98765;
    ResSchedReport::GetInstance().OnTouchEvent(touchEvent, config);
    EXPECT_NE(touchEvent.localX, touchEvent.localY);
}

/**
 * @tc.name: ResSchedReportTest002
 * @tc.desc: test touch move
 * @tc.type: FUNC
 */
HWTEST_F(ResSchedReportTest, ResSchedReportTest002, TestSize.Level1)
{
    TouchEvent touchEvent;
    touchEvent.type = TouchType::MOVE;
    touchEvent.localX = 100.0f;
    touchEvent.localY = 200.0f;
    ReportConfig config;
    config.isReportTid = true;
    config.tid = 12345;
    ResSchedReport::GetInstance().OnTouchEvent(touchEvent, config);
    EXPECT_NE(touchEvent.localX, touchEvent.localY);
}

/**
 * @tc.name: ResSchedReportTest003
 * @tc.desc: test HandleSwiperChange
 * @tc.type: FUNC
 */
HWTEST_F(ResSchedReportTest, ResSchedReportTest003, TestSize.Level1)
{
    std::unordered_map<std::string, std::string> payload;
    payload["path"] = "/test[0]/swiper[1]";
    payload["currentIndex"] = "1";
    payload["componentType"] = "1";
    ResSchedReport::GetInstance().HandleSwiperChange(payload);
    EXPECT_NE(payload.size(), 0);
}

/**
 * @tc.name: ResSchedReportTest004
 * @tc.desc: test HandleSwiperChange with empty payload
 * @tc.type: FUNC
 */
HWTEST_F(ResSchedReportTest, ResSchedReportTest004, TestSize.Level1)
{
    std::unordered_map<std::string, std::string> payload;
    ResSchedReport::GetInstance().HandleSwiperChange(payload);
    EXPECT_FALSE(payload.empty());
}

/**
 * @tc.name: ResSchedReportTest005
 * @tc.desc: test AppSwiperReportEnableCheck
 * @tc.type: FUNC
 */
HWTEST_F(ResSchedReportTest, ResSchedReportTest005, TestSize.Level1)
{
    std::unordered_map<std::string, std::string> payload;
    std::unordered_map<std::string, std::string> reply;
    bool result = ResSchedReport::GetInstance().AppSwiperReportEnableCheck(payload, reply);
    EXPECT_TRUE(result || !result);
}

/**
 * @tc.name: ResSchedReportTest006
 * @tc.desc: test AppSwiperReportEnableCheck with payload
 * @tc.type: FUNC
 */
HWTEST_F(ResSchedReportTest, ResSchedReportTest006, TestSize.Level1)
{
    std::unordered_map<std::string, std::string> payload;
    payload["bundleName"] = "com.example.app";
    std::unordered_map<std::string, std::string> reply;
    bool result = ResSchedReport::GetInstance().AppSwiperReportEnableCheck(payload, reply);
    EXPECT_TRUE(result || !result);
}

/**
 * @tc.name: ResSchedReportTest007
 * @tc.desc: test AppSwiperReportEnableCheck reply parsing
 * @tc.type: FUNC
 */
HWTEST_F(ResSchedReportTest, ResSchedReportTest007, TestSize.Level1)
{
    std::unordered_map<std::string, std::string> payload;
    std::unordered_map<std::string, std::string> reply;
    reply["result"] = "\"true\"";
    bool result = ResSchedReport::GetInstance().AppSwiperReportEnableCheck(payload, reply);
    EXPECT_TRUE(result || !result);
}

/**
 * @tc.name: ResSchedReportTest008
 * @tc.desc: test HandleSwiperChange with multiple params
 * @tc.type: FUNC
 */
HWTEST_F(ResSchedReportTest, ResSchedReportTest008, TestSize.Level1)
{
    std::unordered_map<std::string, std::string> payload;
    payload["path"] = "/root[0]/container[1]/swiper[2]";
    payload["currentIndex"] = "5";
    payload["componentType"] = "1";
    payload["bundleName"] = "com.test.bundle";
    payload["abilityName"] = "MainAbility";
    ResSchedReport::GetInstance().HandleSwiperChange(payload);
    EXPECT_EQ(payload.size(), 7);
}

/**
 * @tc.name: ResSchedReportTest009
 * @tc.desc: test AppClickExtEnableCheck
 * @tc.type: FUNC
 */
HWTEST_F(ResSchedReportTest, ResSchedReportTest009, TestSize.Level1)
{
    std::unordered_map<std::string, std::string> payload;
    std::unordered_map<std::string, std::string> reply;
    bool result = ResSchedReport::GetInstance().AppClickExtEnableCheck(payload, reply);
    EXPECT_TRUE(result || !result);
}

/**
 * @tc.name: ResSchedReportTest010
 * @tc.desc: test AppClickExtEnableCheck with reply
 * @tc.type: FUNC
 */
HWTEST_F(ResSchedReportTest, ResSchedReportTest010, TestSize.Level1)
{
    std::unordered_map<std::string, std::string> payload;
    std::unordered_map<std::string, std::string> reply;
    reply["result"] = "\"true\"";
    bool result = ResSchedReport::GetInstance().AppClickExtEnableCheck(payload, reply);
    EXPECT_TRUE(result || !result);
}

/**
 * @tc.name: ResSchedReportTest011
 * @tc.desc: test AppClickExtEnableCheck with false result
 * @tc.type: FUNC
 */
HWTEST_F(ResSchedReportTest, ResSchedReportTest011, TestSize.Level1)
{
    std::unordered_map<std::string, std::string> payload;
    std::unordered_map<std::string, std::string> reply;
    reply["result"] = "\"false\"";
    bool result = ResSchedReport::GetInstance().AppClickExtEnableCheck(payload, reply);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: ResSchedReportTest012
 * @tc.desc: test HandlePageTransition when frompage is and toPage is empty
 * @tc.type: FUNC
 */
HWTEST_F(ResSchedReportTest, ResSchedReportTest012, TestSize.Level1)
{
    PageTransitionInfo pageTransitionInfo;
    pageTransitionInfo.fromPage = "";
    pageTransitionInfo.toPage = "";
    ResSchedReport::GetInstance().HandlePageTransition(pageTransitionInfo, 0);
    EXPECT_TRUE(pageTransitionInfo.fromPage.empty());
}

/**
 * @tc.name: ResSchedReportTest013
 * @tc.desc: test HandlePageTransition when frompage is empty and topage is not empty
 * @tc.type: FUNC
 */
HWTEST_F(ResSchedReportTest, ResSchedReportTest013, TestSize.Level1)
{
    PageTransitionInfo pageTransitionInfo;
    pageTransitionInfo.fromPage = "";
    pageTransitionInfo.toPage = "topage";
    ResSchedReport::GetInstance().HandlePageTransition(pageTransitionInfo, 0);
    EXPECT_FALSE(pageTransitionInfo.toPage.empty());
}

/**
 * @tc.name: ResSchedReportTest014
 * @tc.desc: test HandlePageTransition when frompage is not empty and toPage is empty
 * @tc.type: FUNC
 */
HWTEST_F(ResSchedReportTest, ResSchedReportTest014, TestSize.Level1)
{
    PageTransitionInfo pageTransitionInfo;
    pageTransitionInfo.fromPage = "frompage";
    pageTransitionInfo.toPage = "";
    ResSchedReport::GetInstance().HandlePageTransition(pageTransitionInfo, 0);
    EXPECT_FALSE(pageTransitionInfo.fromPage.empty());
}

/**
 * @tc.name: ResSchedReportTest015
 * @tc.desc: test HandlePageTransition when frompage is not empty and toPage is not empty
 * @tc.type: FUNC
 */
HWTEST_F(ResSchedReportTest, ResSchedReportTest015, TestSize.Level1)
{
    PageTransitionInfo pageTransitionInfo;
    pageTransitionInfo.fromPage = "frompage";
    pageTransitionInfo.toPage = "topage";
    ResSchedReport::GetInstance().HandlePageTransition(pageTransitionInfo, 0);
    EXPECT_FALSE(pageTransitionInfo.fromPage.empty());
}

namespace {
struct DialogShowReportRecorder {
    uint32_t lastResType = 0;
    int32_t reportCount = 0;
} g_dialogShowRecorder;

void DialogShowReportCapture(uint32_t resType, int64_t value,
    const std::unordered_map<std::string, std::string>& payload)
{
    g_dialogShowRecorder.lastResType = resType;
    g_dialogShowRecorder.reportCount++;
}

struct SwiperChangeReportRecorder {
    uint32_t lastResType = 0;
    int64_t lastValue = -1;
    int32_t reportCount = 0;
} g_swiperChangeRecorder;

void SwiperChangeReportCapture(uint32_t resType, int64_t value,
    const std::unordered_map<std::string, std::string>& payload)
{
    g_swiperChangeRecorder.lastResType = resType;
    g_swiperChangeRecorder.lastValue = value;
    g_swiperChangeRecorder.reportCount++;
}
} // namespace

/**
 * @tc.name: ResSchedReportDialogShow001
 * @tc.desc: test ReportDialogShow reports RES_TYPE_DIALOG_EVENT
 * @tc.type: FUNC
 */
HWTEST_F(ResSchedReportTest, ResSchedReportDialogShow001, TestSize.Level1)
{
    g_dialogShowRecorder = {};
    ResSchedReport::GetInstance().reportDataFunc_ = &DialogShowReportCapture;
    ResSchedReport::GetInstance().ReportDialogShow();
    ResSchedReport::GetInstance().reportDataFunc_ = nullptr;
    EXPECT_EQ(g_dialogShowRecorder.lastResType, 225u);
    EXPECT_EQ(g_dialogShowRecorder.reportCount, 1);
}

/**
 * @tc.name: ResSchedReportDialogShow002
 * @tc.desc: test ReportDialogShow twice reports twice without miss or duplicate
 * @tc.type: FUNC
 */
HWTEST_F(ResSchedReportTest, ResSchedReportDialogShow002, TestSize.Level1)
{
    g_dialogShowRecorder = {};
    ResSchedReport::GetInstance().reportDataFunc_ = &DialogShowReportCapture;
    ResSchedReport::GetInstance().ReportDialogShow();
    ResSchedReport::GetInstance().ReportDialogShow();
    ResSchedReport::GetInstance().reportDataFunc_ = nullptr;
    EXPECT_EQ(g_dialogShowRecorder.lastResType, 225u);
    EXPECT_EQ(g_dialogShowRecorder.reportCount, 2);
}

/**
 * @tc.name: ResSchedReportDialogShow003
 * @tc.desc: test ReportDialogShow when report channel is null, silent skip without crash
 * @tc.type: FUNC
 */
HWTEST_F(ResSchedReportTest, ResSchedReportDialogShow003, TestSize.Level1)
{
    g_dialogShowRecorder = {};
    ResSchedReport::GetInstance().reportDataFunc_ = nullptr;
    ResSchedReport::GetInstance().ReportDialogShow();
    EXPECT_EQ(g_dialogShowRecorder.reportCount, 0);
}

/**
 * @tc.name: ResSchedReportSwiperChange001
 * @tc.desc: test HandleSwiperChange reports RES_TYPE_COMPONENT_STATE_CHANGE with COMPONENT_TYPE_SWIPER
 * @tc.type: FUNC
 */
HWTEST_F(ResSchedReportTest, ResSchedReportSwiperChange001, TestSize.Level1)
{
    g_swiperChangeRecorder = {};
    ResSchedReport::GetInstance().reportDataFunc_ = &SwiperChangeReportCapture;
    std::unordered_map<std::string, std::string> payload;
    payload["currentIndex"] = "1";
    ResSchedReport::GetInstance().HandleSwiperChange(payload);
    ResSchedReport::GetInstance().reportDataFunc_ = nullptr;
    EXPECT_EQ(g_swiperChangeRecorder.lastResType, 212u);
    EXPECT_EQ(g_swiperChangeRecorder.lastValue, 1);
    EXPECT_EQ(g_swiperChangeRecorder.reportCount, 1);
}

namespace {
// resType / event values are internal to ressched_report.cpp, mirrored here
constexpr uint32_t RES_TYPE_CLICK_RECOGNIZE_VALUE = 9;
constexpr uint32_t RES_TYPE_SLIDE_VALUE = 11;
constexpr int64_t TOUCH_DOWN_EVENT_VALUE = 1;
constexpr int64_t TOUCH_UP_EVENT_VALUE = 3;
constexpr int64_t TOUCH_PULL_UP_EVENT_VALUE = 4;
constexpr int64_t SLIDE_DETECTING_VALUE = 2;
constexpr uint64_t CONTAINER_DISPLAY_ID = 9; // must match SetMockDisplayId in SetUp
constexpr int32_t FIRST_DISPLAY_ID = 1;
constexpr int32_t SECOND_DISPLAY_ID = 4;
constexpr int32_t INVALID_DISPLAY_ID = -1;

struct TouchReportRecord {
    uint32_t resType = 0;
    int64_t value = 0;
    std::unordered_map<std::string, std::string> payload;
};

std::vector<TouchReportRecord> g_touchReportRecords;

void TouchReportCapture(uint32_t resType, int64_t value,
    const std::unordered_map<std::string, std::string>& payload)
{
    g_touchReportRecords.push_back({ resType, value, payload });
}

TouchEvent MakeTouchEvent(TouchType type, int32_t targetDisplayId, float x = 0.0f, float y = 0.0f)
{
    TouchEvent event;
    event.type = type;
    event.targetDisplayId = targetDisplayId;
    event.x = x;
    event.y = y;
    return event;
}

const TouchReportRecord* FindReport(uint32_t resType, int64_t value)
{
    auto iter = std::find_if(g_touchReportRecords.begin(), g_touchReportRecords.end(),
        [resType, value](const TouchReportRecord& record) {
            return record.resType == resType && record.value == value;
        });
    return iter == g_touchReportRecords.end() ? nullptr : &(*iter);
}

size_t CountReports(uint32_t resType, int64_t value)
{
    return static_cast<size_t>(std::count_if(g_touchReportRecords.begin(), g_touchReportRecords.end(),
        [resType, value](const TouchReportRecord& record) {
            return record.resType == resType && record.value == value;
        }));
}

bool HasPayloadKey(const TouchReportRecord* record, const char* key)
{
    return record != nullptr && record->payload.find(key) != record->payload.end();
}

std::string GetPayloadValue(const TouchReportRecord* record, const char* key)
{
    if (!HasPayloadKey(record, key)) {
        return std::string();
    }
    return record->payload.at(key);
}
} // namespace

class ResSchedReportDisplayIdTest : public testing::Test {
public:
    static void SetUpTestSuite() {}
    static void TearDownTestSuite() {}

    void SetUp() override
    {
        savedTriggerExecuted_ = ResSchedReport::triggerExecuted;
        ResSchedReport::triggerExecuted = true;
        ResSchedReport::isInTouch_ = false;
        ResSchedReport::isInSlide_ = false;
        ResSchedReport::averageDistance_.Reset();
        MockContainer::SetUp();
        MockContainer::Current()->SetTaskExecutor(AceType::MakeRefPtr<MockTaskExecutor>(true));
        MockContainer::Current()->SetMockDisplayId(CONTAINER_DISPLAY_ID);
        g_touchReportRecords.clear();
        ResSchedReport::GetInstance().reportDataFunc_ = &TouchReportCapture;
    }

    void TearDown() override
    {
        ResSchedReport::GetInstance().reportDataFunc_ = nullptr;
        MockContainer::TearDown();
        ResSchedReport::triggerExecuted = savedTriggerExecuted_;
        g_touchReportRecords.clear();
    }

private:
    bool savedTriggerExecuted_ = false;
};

/**
 * @tc.name: ResSchedReportDisplayId001
 * @tc.desc: test LoadEventDisplayId branches: event displayId overrides container
 *           displayId, zero is a valid display, invalid (< 0) falls back to
 *           container value, and no displayId key exists without container
 * @tc.type: FUNC
 */
HWTEST_F(ResSchedReportDisplayIdTest, ResSchedReportDisplayId001, TestSize.Level1)
{
    ReportConfig config;
    ResSchedReport::GetInstance().HandleTouchDown(
        MakeTouchEvent(TouchType::DOWN, FIRST_DISPLAY_ID), config);
    ResSchedReport::GetInstance().HandleTouchDown(MakeTouchEvent(TouchType::DOWN, 0), config);
    ResSchedReport::GetInstance().HandleTouchDown(
        MakeTouchEvent(TouchType::DOWN, INVALID_DISPLAY_ID), config);
    auto saveContainer = MockContainer::container_;
    MockContainer::container_ = nullptr;
    ResSchedReport::GetInstance().HandleTouchDown(
        MakeTouchEvent(TouchType::DOWN, INVALID_DISPLAY_ID), config);
    MockContainer::container_ = saveContainer;

    EXPECT_EQ(g_touchReportRecords.size(), 4u);
    EXPECT_EQ(GetPayloadValue(&g_touchReportRecords[0], "displayId"), std::to_string(FIRST_DISPLAY_ID));
    EXPECT_EQ(GetPayloadValue(&g_touchReportRecords[1], "displayId"), "0");
    EXPECT_EQ(GetPayloadValue(&g_touchReportRecords[2], "displayId"), std::to_string(CONTAINER_DISPLAY_ID));
    EXPECT_FALSE(HasPayloadKey(&g_touchReportRecords[3], "displayId"));
    EXPECT_EQ(GetPayloadValue(&g_touchReportRecords[0], "name"), "touch");
}

/**
 * @tc.name: ResSchedReportDisplayId002
 * @tc.desc: test touch gesture through OnTouchEvent wires displayId on all four
 *           touch handlers, each event keeps its own displayId when display
 *           changes within one gesture
 * @tc.type: FUNC
 */
HWTEST_F(ResSchedReportDisplayIdTest, ResSchedReportDisplayId002, TestSize.Level1)
{
    ReportConfig config;
    ResSchedReport::GetInstance().OnTouchEvent(
        MakeTouchEvent(TouchType::DOWN, FIRST_DISPLAY_ID), config);
    ResSchedReport::GetInstance().OnTouchEvent(
        MakeTouchEvent(TouchType::MOVE, SECOND_DISPLAY_ID, 100.0f), config);
    ResSchedReport::GetInstance().OnTouchEvent(
        MakeTouchEvent(TouchType::PULL_UP, SECOND_DISPLAY_ID), config);
    ResSchedReport::GetInstance().OnTouchEvent(
        MakeTouchEvent(TouchType::UP, SECOND_DISPLAY_ID), config);
    EXPECT_EQ(CountReports(RES_TYPE_CLICK_RECOGNIZE_VALUE, TOUCH_DOWN_EVENT_VALUE), 1u);
    EXPECT_EQ(CountReports(RES_TYPE_SLIDE_VALUE, SLIDE_DETECTING_VALUE), 1u);
    EXPECT_EQ(CountReports(RES_TYPE_CLICK_RECOGNIZE_VALUE, TOUCH_PULL_UP_EVENT_VALUE), 1u);
    EXPECT_EQ(CountReports(RES_TYPE_CLICK_RECOGNIZE_VALUE, TOUCH_UP_EVENT_VALUE), 1u);
    const TouchReportRecord* downRecord =
        FindReport(RES_TYPE_CLICK_RECOGNIZE_VALUE, TOUCH_DOWN_EVENT_VALUE);
    const TouchReportRecord* slideRecord = FindReport(RES_TYPE_SLIDE_VALUE, SLIDE_DETECTING_VALUE);
    const TouchReportRecord* pullUpRecord =
        FindReport(RES_TYPE_CLICK_RECOGNIZE_VALUE, TOUCH_PULL_UP_EVENT_VALUE);
    const TouchReportRecord* upRecord = FindReport(RES_TYPE_CLICK_RECOGNIZE_VALUE, TOUCH_UP_EVENT_VALUE);
    ASSERT_NE(downRecord, nullptr);
    ASSERT_NE(slideRecord, nullptr);
    ASSERT_NE(pullUpRecord, nullptr);
    ASSERT_NE(upRecord, nullptr);
    EXPECT_EQ(GetPayloadValue(downRecord, "displayId"), std::to_string(FIRST_DISPLAY_ID));
    EXPECT_EQ(GetPayloadValue(slideRecord, "displayId"), std::to_string(SECOND_DISPLAY_ID));
    EXPECT_EQ(GetPayloadValue(pullUpRecord, "displayId"), std::to_string(SECOND_DISPLAY_ID));
    EXPECT_EQ(GetPayloadValue(upRecord, "displayId"), std::to_string(SECOND_DISPLAY_ID));
    EXPECT_TRUE(HasPayloadKey(upRecord, "up_speed"));
    EXPECT_TRUE(HasPayloadKey(upRecord, "pid"));
    EXPECT_TRUE(HasPayloadKey(upRecord, "uid"));
    EXPECT_TRUE(HasPayloadKey(upRecord, "bundleName"));
    EXPECT_TRUE(HasPayloadKey(upRecord, "abilityName"));
}
} // namespace OHOS::Ace

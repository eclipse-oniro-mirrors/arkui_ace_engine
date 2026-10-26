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

#define protected public
#define private public

#include "test/mock/frameworks/core/common/mock_container.h"
#include "test/mock/frameworks/core/common/mock_theme_default.h"
#include "test/mock/frameworks/core/common/mock_theme_manager.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"

#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/time_picker/timepicker_column_pattern.h"
#include "core/components_ng/pattern/time_picker/timepicker_layout_property.h"
#include "core/components_ng/pattern/time_picker/timepicker_model.h"
#include "core/components_ng/pattern/time_picker/timepicker_model_ng.h"
#include "core/components_ng/pattern/time_picker/timepicker_row_pattern.h"
#include "core/components_ng/pattern/date_picker/picker_theme.h"
#include "core/components_ng/pattern/root/root_pattern.h"

#undef private
#undef protected

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {

RefPtr<Theme> GetTheme(ThemeType type)
{
    if (type == PickerTheme::TypeId()) {
        return MockThemeDefault::GetPickerTheme();
    }
    return nullptr;
}

class TimePickerTestCore : public testing::Test {
public:
    static void SetUpTestCase()
    {
        MockPipelineContext::SetUp();
        MockContainer::SetUp();
    }

    static void TearDownTestCase()
    {
        MockContainer::TearDown();
        MockPipelineContext::TearDown();
    }

    void SetUp() override
    {
        auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
        EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(
            [](ThemeType type) -> RefPtr<Theme> { return GetTheme(type); });
        EXPECT_CALL(*themeManager, GetTheme(_, _)).WillRepeatedly(
            [](ThemeType type, int32_t) -> RefPtr<Theme> { return GetTheme(type); });
        MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
        theme_ = MockPipelineContext::GetCurrent()->GetTheme<PickerTheme>();
        ASSERT_NE(theme_, nullptr);
    }

    void TearDown() override
    {
        ViewStackProcessor::GetInstance()->ClearStack();
    }

protected:
    RefPtr<PickerTheme> theme_;

    RefPtr<FrameNode> CreateTimePickerNode(bool hasSecond = false)
    {
        TimePickerModel::GetInstance()->CreateTimePicker(theme_, hasSecond);
        auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
        if (frameNode) {
            frameNode->MarkModifyDone();
        }
        return AceType::Claim(frameNode);
    }
};

} // namespace

/**
 * @tc.name: TimePickerCore001
 * @tc.desc: Test IsAtStartHour when selected equals start
 * @tc.type: FUNC
 */
HWTEST_F(TimePickerTestCore, TimePickerCore001, TestSize.Level1)
{
    auto frameNode = CreateTimePickerNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TimePickerRowPattern>();
    ASSERT_NE(pattern, nullptr);

    pattern->SetStartTime(PickerTime(8, 0, 0));
    pattern->SetEndTime(PickerTime(18, 0, 0));
    pattern->SetSelectedTime(PickerTime(8, 0, 0));
    frameNode->MarkModifyDone();

    EXPECT_TRUE(pattern->IsAtStartHour());
    EXPECT_FALSE(pattern->IsAtEndHour());
}

/**
 * @tc.name: TimePickerCore002
 * @tc.desc: Test IsAtEndHour when selected equals end
 * @tc.type: FUNC
 */
HWTEST_F(TimePickerTestCore, TimePickerCore002, TestSize.Level1)
{
    auto frameNode = CreateTimePickerNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TimePickerRowPattern>();
    ASSERT_NE(pattern, nullptr);

    pattern->SetStartTime(PickerTime(8, 0, 0));
    pattern->SetEndTime(PickerTime(18, 0, 0));
    pattern->SetSelectedTime(PickerTime(18, 0, 0));
    frameNode->MarkModifyDone();

    EXPECT_FALSE(pattern->IsAtStartHour());
    EXPECT_TRUE(pattern->IsAtEndHour());
}

/**
 * @tc.name: TimePickerCore003
 * @tc.desc: Test IsAtStartHour false when non-boundary
 * @tc.type: FUNC
 */
HWTEST_F(TimePickerTestCore, TimePickerCore003, TestSize.Level1)
{
    auto frameNode = CreateTimePickerNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TimePickerRowPattern>();
    ASSERT_NE(pattern, nullptr);

    pattern->SetStartTime(PickerTime(8, 0, 0));
    pattern->SetEndTime(PickerTime(18, 0, 0));
    pattern->SetSelectedTime(PickerTime(12, 30, 0));
    frameNode->MarkModifyDone();

    EXPECT_FALSE(pattern->IsAtStartHour());
    EXPECT_FALSE(pattern->IsAtEndHour());
}

/**
 * @tc.name: TimePickerCore004
 * @tc.desc: Test IsAtStartMinute when at start hour and minute
 * @tc.type: FUNC
 */
HWTEST_F(TimePickerTestCore, TimePickerCore004, TestSize.Level1)
{
    auto frameNode = CreateTimePickerNode(true);
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TimePickerRowPattern>();
    ASSERT_NE(pattern, nullptr);

    pattern->SetStartTime(PickerTime(8, 30, 0));
    pattern->SetEndTime(PickerTime(18, 30, 0));
    pattern->SetSelectedTime(PickerTime(8, 30, 0));
    frameNode->MarkModifyDone();

    EXPECT_TRUE(pattern->IsAtStartHour());
    EXPECT_TRUE(pattern->IsAtStartMinute());
    EXPECT_FALSE(pattern->IsAtEndMinute());
}

/**
 * @tc.name: TimePickerCore005
 * @tc.desc: Test IsAtEndMinute when at end hour and minute
 * @tc.type: FUNC
 */
HWTEST_F(TimePickerTestCore, TimePickerCore005, TestSize.Level1)
{
    auto frameNode = CreateTimePickerNode(true);
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TimePickerRowPattern>();
    ASSERT_NE(pattern, nullptr);

    pattern->SetStartTime(PickerTime(8, 30, 0));
    pattern->SetEndTime(PickerTime(18, 30, 0));
    pattern->SetSelectedTime(PickerTime(18, 30, 0));
    frameNode->MarkModifyDone();

    EXPECT_TRUE(pattern->IsAtEndHour());
    EXPECT_TRUE(pattern->IsAtEndMinute());
    EXPECT_FALSE(pattern->IsAtStartMinute());
}

/**
 * @tc.name: TimePickerCore006
 * @tc.desc: Test IsAtStartMinute false at start hour but different minute
 * @tc.type: FUNC
 */
HWTEST_F(TimePickerTestCore, TimePickerCore006, TestSize.Level1)
{
    auto frameNode = CreateTimePickerNode(true);
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TimePickerRowPattern>();
    ASSERT_NE(pattern, nullptr);

    pattern->SetStartTime(PickerTime(8, 30, 0));
    pattern->SetEndTime(PickerTime(18, 30, 0));
    pattern->SetSelectedTime(PickerTime(8, 45, 0));
    frameNode->MarkModifyDone();

    EXPECT_TRUE(pattern->IsAtStartHour());
    EXPECT_FALSE(pattern->IsAtStartMinute());
}

/**
 * @tc.name: TimePickerCore007
 * @tc.desc: Test IsStartEndTimeDefined true when start/end set
 * @tc.type: FUNC
 */
HWTEST_F(TimePickerTestCore, TimePickerCore007, TestSize.Level1)
{
    auto frameNode = CreateTimePickerNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TimePickerRowPattern>();
    ASSERT_NE(pattern, nullptr);

    pattern->SetStartTime(PickerTime(8, 0, 0));
    pattern->SetEndTime(PickerTime(18, 0, 0));
    frameNode->MarkModifyDone();

    EXPECT_TRUE(pattern->IsStartEndTimeDefined());
}

/**
 * @tc.name: TimePickerCore008
 * @tc.desc: Test IsStartEndTimeDefined false when default start/end
 * @tc.type: FUNC
 */
HWTEST_F(TimePickerTestCore, TimePickerCore008, TestSize.Level1)
{
    auto frameNode = CreateTimePickerNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TimePickerRowPattern>();
    ASSERT_NE(pattern, nullptr);

    EXPECT_FALSE(pattern->IsStartEndTimeDefined());
}

/**
 * @tc.name: TimePickerCore009
 * @tc.desc: Test GetHourColumnNode returns valid node
 * @tc.type: FUNC
 */
HWTEST_F(TimePickerTestCore, TimePickerCore009, TestSize.Level1)
{
    auto frameNode = CreateTimePickerNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TimePickerRowPattern>();
    ASSERT_NE(pattern, nullptr);

    pattern->UpdateAllChildNode();
    auto hourNode = pattern->GetHourColumnNode();
    EXPECT_NE(hourNode, nullptr);
    auto minuteNode = pattern->GetMinuteColumnNode();
    EXPECT_NE(minuteNode, nullptr);
}

/**
 * @tc.name: TimePickerCore010
 * @tc.desc: Test GetSecondColumnNode returns valid node when hasSecond
 * @tc.type: FUNC
 */
HWTEST_F(TimePickerTestCore, TimePickerCore010, TestSize.Level1)
{
    auto frameNode = CreateTimePickerNode(true);
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TimePickerRowPattern>();
    ASSERT_NE(pattern, nullptr);

    pattern->UpdateAllChildNode();
    auto secondNode = pattern->GetSecondColumnNode();
    EXPECT_NE(secondNode, nullptr);
}

/**
 * @tc.name: TimePickerCore011
 * @tc.desc: Test GetSecondColumnNode returns null when no second
 * @tc.type: FUNC
 */
HWTEST_F(TimePickerTestCore, TimePickerCore011, TestSize.Level1)
{
    auto frameNode = CreateTimePickerNode(false);
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TimePickerRowPattern>();
    ASSERT_NE(pattern, nullptr);

    pattern->UpdateAllChildNode();
    auto secondNode = pattern->GetSecondColumnNode();
    EXPECT_EQ(secondNode, nullptr);
}

/**
 * @tc.name: TimePickerCore012
 * @tc.desc: Test FlushSecondColumnOptions when hasSecond=true does not crash
 * @tc.type: FUNC
 */
HWTEST_F(TimePickerTestCore, TimePickerCore012, TestSize.Level1)
{
    auto frameNode = CreateTimePickerNode(true);
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TimePickerRowPattern>();
    ASSERT_NE(pattern, nullptr);

    pattern->UpdateAllChildNode();
    pattern->FlushSecondColumnOptions();
    auto secondNode = pattern->GetSecondColumnNode();
    ASSERT_NE(secondNode, nullptr);
    auto secondPattern = secondNode->GetPattern<TimePickerColumnPattern>();
    ASSERT_NE(secondPattern, nullptr);
}

/**
 * @tc.name: TimePickerCore013
 * @tc.desc: Test FlushSecondColumnOptions when hasSecond=false is no-op
 * @tc.type: FUNC
 */
HWTEST_F(TimePickerTestCore, TimePickerCore013, TestSize.Level1)
{
    auto frameNode = CreateTimePickerNode(false);
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TimePickerRowPattern>();
    ASSERT_NE(pattern, nullptr);

    pattern->UpdateAllChildNode();
    pattern->FlushSecondColumnOptions();
    EXPECT_FALSE(pattern->GetHasSecond());
}

/**
 * @tc.name: TimePickerCore014
 * @tc.desc: Test GetCanLoopFromLayoutPropertyWithStartEnd for hour column (false when boundary set)
 * @tc.type: FUNC
 */
HWTEST_F(TimePickerTestCore, TimePickerCore014, TestSize.Level1)
{
    auto frameNode = CreateTimePickerNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TimePickerRowPattern>();
    ASSERT_NE(pattern, nullptr);

    pattern->SetStartTime(PickerTime(8, 0, 0));
    pattern->SetEndTime(PickerTime(18, 0, 0));
    pattern->SetSelectedTime(PickerTime(8, 0, 0));
    frameNode->MarkModifyDone();
    pattern->UpdateAllChildNode();

    auto hourNode = pattern->GetHourColumnNode();
    ASSERT_NE(hourNode, nullptr);
    auto hourColumnPattern = hourNode->GetPattern<TimePickerColumnPattern>();
    ASSERT_NE(hourColumnPattern, nullptr);
    EXPECT_FALSE(hourColumnPattern->GetCanLoopFromLayoutPropertyWithStartEnd());
}

/**
 * @tc.name: TimePickerCore015
 * @tc.desc: Test GetCanLoopFromLayoutPropertyWithStartEnd for minute column at boundary (false)
 * @tc.type: FUNC
 */
HWTEST_F(TimePickerTestCore, TimePickerCore015, TestSize.Level1)
{
    auto frameNode = CreateTimePickerNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TimePickerRowPattern>();
    ASSERT_NE(pattern, nullptr);

    pattern->SetStartTime(PickerTime(8, 0, 0));
    pattern->SetEndTime(PickerTime(18, 0, 0));
    pattern->SetSelectedTime(PickerTime(8, 0, 0));
    frameNode->MarkModifyDone();
    pattern->UpdateAllChildNode();

    auto minuteNode = pattern->GetMinuteColumnNode();
    ASSERT_NE(minuteNode, nullptr);
    auto minuteColumnPattern = minuteNode->GetPattern<TimePickerColumnPattern>();
    ASSERT_NE(minuteColumnPattern, nullptr);
    EXPECT_FALSE(minuteColumnPattern->GetCanLoopFromLayoutPropertyWithStartEnd());
}

/**
 * @tc.name: TimePickerCore016
 * @tc.desc: Test GetCanLoopFromLayoutPropertyWithStartEnd for minute column non-boundary (true)
 * @tc.type: FUNC
 */
HWTEST_F(TimePickerTestCore, TimePickerCore016, TestSize.Level1)
{
    auto frameNode = CreateTimePickerNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TimePickerRowPattern>();
    ASSERT_NE(pattern, nullptr);

    pattern->SetStartTime(PickerTime(8, 0, 0));
    pattern->SetEndTime(PickerTime(18, 0, 0));
    pattern->SetSelectedTime(PickerTime(12, 30, 0));
    frameNode->MarkModifyDone();
    pattern->UpdateAllChildNode();

    auto minuteNode = pattern->GetMinuteColumnNode();
    ASSERT_NE(minuteNode, nullptr);
    auto minuteColumnPattern = minuteNode->GetPattern<TimePickerColumnPattern>();
    ASSERT_NE(minuteColumnPattern, nullptr);
    EXPECT_TRUE(minuteColumnPattern->GetCanLoopFromLayoutPropertyWithStartEnd());
}

/**
 * @tc.name: TimePickerCore017
 * @tc.desc: Test GetCanLoopFromLayoutPropertyWithStartEnd for second column at boundary (false)
 * @tc.type: FUNC
 */
HWTEST_F(TimePickerTestCore, TimePickerCore017, TestSize.Level1)
{
    auto frameNode = CreateTimePickerNode(true);
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TimePickerRowPattern>();
    ASSERT_NE(pattern, nullptr);

    pattern->SetStartTime(PickerTime(8, 30, 0));
    pattern->SetEndTime(PickerTime(18, 30, 0));
    pattern->SetSelectedTime(PickerTime(8, 30, 0));
    frameNode->MarkModifyDone();
    pattern->UpdateAllChildNode();

    auto secondNode = pattern->GetSecondColumnNode();
    ASSERT_NE(secondNode, nullptr);
    auto secondColumnPattern = secondNode->GetPattern<TimePickerColumnPattern>();
    ASSERT_NE(secondColumnPattern, nullptr);
    EXPECT_FALSE(secondColumnPattern->GetCanLoopFromLayoutPropertyWithStartEnd());
}

/**
 * @tc.name: TimePickerCore018
 * @tc.desc: Test GetCanLoopFromLayoutPropertyWithStartEnd for second column non-boundary (true)
 * @tc.type: FUNC
 */
HWTEST_F(TimePickerTestCore, TimePickerCore018, TestSize.Level1)
{
    auto frameNode = CreateTimePickerNode(true);
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TimePickerRowPattern>();
    ASSERT_NE(pattern, nullptr);

    pattern->SetStartTime(PickerTime(8, 30, 0));
    pattern->SetEndTime(PickerTime(18, 30, 0));
    pattern->SetSelectedTime(PickerTime(12, 30, 0));
    frameNode->MarkModifyDone();
    pattern->UpdateAllChildNode();

    auto secondNode = pattern->GetSecondColumnNode();
    ASSERT_NE(secondNode, nullptr);
    auto secondColumnPattern = secondNode->GetPattern<TimePickerColumnPattern>();
    ASSERT_NE(secondColumnPattern, nullptr);
    EXPECT_TRUE(secondColumnPattern->GetCanLoopFromLayoutPropertyWithStartEnd());
}

/**
 * @tc.name: TimePickerCore019
 * @tc.desc: Test IsTossNeedToStop returns inverse of GetCanLoopFromLayoutPropertyWithStartEnd
 * @tc.type: FUNC
 */
HWTEST_F(TimePickerTestCore, TimePickerCore019, TestSize.Level1)
{
    auto frameNode = CreateTimePickerNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TimePickerRowPattern>();
    ASSERT_NE(pattern, nullptr);

    pattern->SetStartTime(PickerTime(8, 0, 0));
    pattern->SetEndTime(PickerTime(18, 0, 0));
    pattern->SetSelectedTime(PickerTime(12, 30, 0));
    frameNode->MarkModifyDone();
    pattern->UpdateAllChildNode();

    auto minuteNode = pattern->GetMinuteColumnNode();
    ASSERT_NE(minuteNode, nullptr);
    auto minuteColumnPattern = minuteNode->GetPattern<TimePickerColumnPattern>();
    ASSERT_NE(minuteColumnPattern, nullptr);

    bool canLoop = minuteColumnPattern->GetCanLoopFromLayoutPropertyWithStartEnd();
    EXPECT_EQ(minuteColumnPattern->IsTossNeedToStop(), !canLoop);
}

} // namespace OHOS::Ace::NG

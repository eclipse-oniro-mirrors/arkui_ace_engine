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

#include <set>

#define protected public
#define private public

#include "test/mock/frameworks/core/common/mock_container.h"
#include "test/mock/frameworks/core/common/mock_theme_default.h"
#include "test/mock/frameworks/core/common/mock_theme_manager.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/date_picker/datepicker_column_pattern.h"
#include "core/components_ng/pattern/date_picker/datepicker_model_ng.h"
#include "core/components_ng/pattern/date_picker/datepicker_pattern.h"
#include "core/components_ng/pattern/date_picker/picker_date_f.h"
#include "core/components_ng/pattern/root/root_pattern.h"
#include "base/i18n/date_time_sequence.h"
#include "core/common/ace_application_info.h"

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
    } else {
        return nullptr;
    }
}

class DatePickerTestCore : public testing::Test {
public:
    static void SetUpTestCase()
    {
        MockPipelineContext::SetUp();
        MockContainer::SetUp();
        oldLanguage_ = AceApplicationInfo::GetInstance().GetLanguage();
    }

    static void TearDownTestCase()
    {
        AceApplicationInfo::GetInstance().language_ = oldLanguage_;
        MockContainer::TearDown();
        MockPipelineContext::TearDown();
    }

    void SetUp() override
    {
        auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
        EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(
            [](ThemeType type) -> RefPtr<Theme> { return GetTheme(type); });
        EXPECT_CALL(*themeManager, GetTheme(_, _)).WillRepeatedly(
            [](ThemeType type, int32_t themeScopeId) -> RefPtr<Theme> { return GetTheme(type); });
        MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
        theme_ = MockPipelineContext::GetCurrent()->GetTheme<PickerTheme>();
        ASSERT_NE(theme_, nullptr);
    }

    void TearDown() override
    {
        ViewStackProcessor::GetInstance()->ClearStack();
        AceApplicationInfo::GetInstance().language_ = oldLanguage_;
    }

protected:
    RefPtr<PickerTheme> theme_;
    static std::string oldLanguage_;

    RefPtr<FrameNode> CreateDatePickerNode()
    {
        DatePickerModel::GetInstance()->CreateDatePicker(theme_);
        auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
        if (frameNode) {
            frameNode->MarkModifyDone();
        }
        return AceType::Claim(frameNode);
    }

    static int32_t GetColumnIdFromStack(const RefPtr<UINode>& stackNode)
    {
        auto blendNode = stackNode->GetLastChild();
        if (!blendNode) {
            return -1;
        }
        auto columnNode = blendNode->GetLastChild();
        if (!columnNode) {
            return -1;
        }
        return columnNode->GetId();
    }
};

std::string DatePickerTestCore::oldLanguage_;

} // namespace

/**
 * @tc.name: DatePickerCore001
 * @tc.desc: Test CreateDatePicker column mounting order matches ICU date order for en
 * @tc.type: FUNC
 */
HWTEST_F(DatePickerTestCore, DatePickerCore001, TestSize.Level1)
{
    const std::string language = "en";
    AceApplicationInfo::GetInstance().SetLocale(language, "US", "Latn", "");

    auto frameNode = CreateDatePickerNode();
    ASSERT_NE(frameNode, nullptr);

    auto datePickerPattern = frameNode->GetPattern<DatePickerPattern>();
    ASSERT_NE(datePickerPattern, nullptr);

    auto expectedOrder = DateTimeSequence().GetDateOrder(language).dateOrder;
    EXPECT_EQ(datePickerPattern->dateOrder_, expectedOrder);

    const auto& children = frameNode->GetChildren();
    ASSERT_EQ(children.size(), 3);

    int32_t yearId = datePickerPattern->GetYearId();
    int32_t monthId = datePickerPattern->GetMonthId();
    int32_t dayId = datePickerPattern->GetDayId();

    auto iter = children.begin();
    for (int32_t i = 0; i < 3 && iter != children.end(); ++i, ++iter) {
        int32_t colId = GetColumnIdFromStack(*iter);
        char expectedField = expectedOrder[i * 2];
        if (expectedField == 'y') {
            EXPECT_EQ(colId, yearId);
        } else if (expectedField == 'M') {
            EXPECT_EQ(colId, monthId);
        } else if (expectedField == 'd') {
            EXPECT_EQ(colId, dayId);
        }
    }
}

/**
 * @tc.name: DatePickerCore002
 * @tc.desc: Test CreateDatePicker column mounting order matches ICU date order for zh
 * @tc.type: FUNC
 */
HWTEST_F(DatePickerTestCore, DatePickerCore002, TestSize.Level1)
{
    const std::string language = "zh";
    AceApplicationInfo::GetInstance().SetLocale(language, "CN", "Hans", "");

    auto frameNode = CreateDatePickerNode();
    ASSERT_NE(frameNode, nullptr);

    auto datePickerPattern = frameNode->GetPattern<DatePickerPattern>();
    ASSERT_NE(datePickerPattern, nullptr);

    auto expectedOrder = DateTimeSequence().GetDateOrder(language).dateOrder;
    EXPECT_EQ(datePickerPattern->dateOrder_, expectedOrder);

    const auto& children = frameNode->GetChildren();
    ASSERT_EQ(children.size(), 3);

    int32_t yearId = datePickerPattern->GetYearId();
    int32_t monthId = datePickerPattern->GetMonthId();
    int32_t dayId = datePickerPattern->GetDayId();

    auto iter = children.begin();
    for (int32_t i = 0; i < 3 && iter != children.end(); ++i, ++iter) {
        int32_t colId = GetColumnIdFromStack(*iter);
        char expectedField = expectedOrder[i * 2];
        if (expectedField == 'y') {
            EXPECT_EQ(colId, yearId);
        } else if (expectedField == 'M') {
            EXPECT_EQ(colId, monthId);
        } else if (expectedField == 'd') {
            EXPECT_EQ(colId, dayId);
        }
    }
}

/**
 * @tc.name: DatePickerCore003
 * @tc.desc: Test UpdateDateOrder uses ICU for ug (no longer hardcoded y-d-M)
 * @tc.type: FUNC
 */
HWTEST_F(DatePickerTestCore, DatePickerCore003, TestSize.Level1)
{
    auto frameNode = CreateDatePickerNode();
    ASSERT_NE(frameNode, nullptr);
    auto datePickerPattern = frameNode->GetPattern<DatePickerPattern>();
    ASSERT_NE(datePickerPattern, nullptr);

    const std::string language = "ug";
    AceApplicationInfo::GetInstance().SetLocale(language, "Kashgar", "Uyghur", "");

    datePickerPattern->OnLanguageConfigurationUpdate();

    auto expectedOrder = DateTimeSequence().GetDateOrder(language).dateOrder;
    EXPECT_EQ(datePickerPattern->dateOrder_, expectedOrder);
    EXPECT_NE(datePickerPattern->dateOrder_, "y-d-M");
}

/**
 * @tc.name: DatePickerCore004
 * @tc.desc: Test UpdateDateOrder uses ICU for de
 * @tc.type: FUNC
 */
HWTEST_F(DatePickerTestCore, DatePickerCore004, TestSize.Level1)
{
    auto frameNode = CreateDatePickerNode();
    ASSERT_NE(frameNode, nullptr);
    auto datePickerPattern = frameNode->GetPattern<DatePickerPattern>();
    ASSERT_NE(datePickerPattern, nullptr);

    const std::string language = "de";
    AceApplicationInfo::GetInstance().SetLocale(language, "DE", "Latn", "");

    datePickerPattern->OnLanguageConfigurationUpdate();

    auto expectedOrder = DateTimeSequence().GetDateOrder(language).dateOrder;
    EXPECT_EQ(datePickerPattern->dateOrder_, expectedOrder);
}

/**
 * @tc.name: DatePickerCore005
 * @tc.desc: Test SetOnChange callback resets inited_ on locale change
 * @tc.type: FUNC
 */
HWTEST_F(DatePickerTestCore, DatePickerCore005, TestSize.Level1)
{
    auto frameNode = CreateDatePickerNode();
    ASSERT_NE(frameNode, nullptr);
    auto datePickerPattern = frameNode->GetPattern<DatePickerPattern>();
    ASSERT_NE(datePickerPattern, nullptr);

    auto date = PickerDate(2024, 1, 1);
    datePickerPattern->GetYear(date.GetYear());
    EXPECT_TRUE(DatePickerPattern::inited_);

    Localization::GetInstance()->HandleOnChange();
    EXPECT_FALSE(DatePickerPattern::inited_);
}

/**
 * @tc.name: DatePickerCore006
 * @tc.desc: Test GetFormatString uses ICU date order for month-day
 * @tc.type: FUNC
 */
HWTEST_F(DatePickerTestCore, DatePickerCore006, TestSize.Level1)
{
    auto frameNode = CreateDatePickerNode();
    ASSERT_NE(frameNode, nullptr);
    auto datePickerPattern = frameNode->GetPattern<DatePickerPattern>();
    ASSERT_NE(datePickerPattern, nullptr);

    const std::string language = "en";
    AceApplicationInfo::GetInstance().SetLocale(language, "US", "Latn", "");
    datePickerPattern->OnLanguageConfigurationUpdate();

    auto date = PickerDateF::CreateMonthDay(4, 16, false, false);
    auto result = datePickerPattern->GetFormatString(date);
    EXPECT_FALSE(result.empty());

    auto expectedOrder = DateTimeSequence().GetDateOrder(language).dateOrder;
    auto monthPos = expectedOrder.find('M');
    auto dayPos = expectedOrder.find('d');
    if (monthPos != std::string::npos && dayPos != std::string::npos && dayPos < monthPos) {
        auto dayStr = datePickerPattern->solarDays_[15];
        auto monthStr = datePickerPattern->solarMonths_[3];
        EXPECT_EQ(result, dayStr + " " + monthStr);
    } else {
        auto dayStr = datePickerPattern->solarDays_[15];
        auto monthStr = datePickerPattern->solarMonths_[3];
        EXPECT_EQ(result, monthStr + " " + dayStr);
    }
}

/**
 * @tc.name: DatePickerCore007
 * @tc.desc: Test GetFormatString uses ICU date order for de (day before month)
 * @tc.type: FUNC
 */
HWTEST_F(DatePickerTestCore, DatePickerCore007, TestSize.Level1)
{
    auto frameNode = CreateDatePickerNode();
    ASSERT_NE(frameNode, nullptr);
    auto datePickerPattern = frameNode->GetPattern<DatePickerPattern>();
    ASSERT_NE(datePickerPattern, nullptr);

    const std::string language = "de";
    AceApplicationInfo::GetInstance().SetLocale(language, "DE", "Latn", "");
    datePickerPattern->OnLanguageConfigurationUpdate();

    auto date = PickerDateF::CreateMonthDay(4, 16, false, false);
    auto result = datePickerPattern->GetFormatString(date);
    EXPECT_FALSE(result.empty());

    auto expectedOrder = DateTimeSequence().GetDateOrder(language).dateOrder;
    auto monthPos = expectedOrder.find('M');
    auto dayPos = expectedOrder.find('d');
    auto dayStr = datePickerPattern->solarDays_[15];
    auto monthStr = datePickerPattern->solarMonths_[3];
    if (monthPos != std::string::npos && dayPos != std::string::npos && dayPos < monthPos) {
        EXPECT_EQ(result, dayStr + " " + monthStr);
    } else {
        EXPECT_EQ(result, monthStr + " " + dayStr);
    }
}

/**
 * @tc.name: DatePickerCore008
 * @tc.desc: Test InitColumnsOrder for all 6 date orders
 * @tc.type: FUNC
 */
HWTEST_F(DatePickerTestCore, DatePickerCore008, TestSize.Level1)
{
    auto frameNode = CreateDatePickerNode();
    ASSERT_NE(frameNode, nullptr);
    auto datePickerPattern = frameNode->GetPattern<DatePickerPattern>();
    ASSERT_NE(datePickerPattern, nullptr);

    int32_t yearId = datePickerPattern->GetYearId();
    int32_t monthId = datePickerPattern->GetMonthId();
    int32_t dayId = datePickerPattern->GetDayId();

    auto verifyOrder = [datePickerPattern, yearId, monthId, dayId](const std::string& order) {
        datePickerPattern->SetDateOrder(order);
        RefPtr<FrameNode> columns[COLUMNS_SIZE];
        datePickerPattern->InitColumnsOrder(&columns);
        ASSERT_NE(columns[0], nullptr);
        ASSERT_NE(columns[1], nullptr);
        ASSERT_NE(columns[2], nullptr);

        std::set<int32_t> ids = { columns[0]->GetId(), columns[1]->GetId(), columns[2]->GetId() };
        EXPECT_EQ(ids.size(), 3);
        EXPECT_TRUE(ids.find(yearId) != ids.end());
        EXPECT_TRUE(ids.find(monthId) != ids.end());
        EXPECT_TRUE(ids.find(dayId) != ids.end());
    };

    verifyOrder("y-M-d");
    verifyOrder("M-d-y");
    verifyOrder("d-M-y");
    verifyOrder("y-d-M");
    verifyOrder("d-y-M");
    verifyOrder("M-y-d");
}

/**
 * @tc.name: DatePickerCore009
 * @tc.desc: Test locale switch updates dateOrder_ dynamically
 * @tc.type: FUNC
 */
HWTEST_F(DatePickerTestCore, DatePickerCore009, TestSize.Level1)
{
    const std::string lang1 = "en";
    AceApplicationInfo::GetInstance().SetLocale(lang1, "US", "Latn", "");
    auto frameNode = CreateDatePickerNode();
    ASSERT_NE(frameNode, nullptr);
    auto datePickerPattern = frameNode->GetPattern<DatePickerPattern>();
    ASSERT_NE(datePickerPattern, nullptr);

    EXPECT_EQ(datePickerPattern->dateOrder_, DateTimeSequence().GetDateOrder(lang1).dateOrder);

    const std::string lang2 = "zh";
    AceApplicationInfo::GetInstance().SetLocale(lang2, "CN", "Hans", "");
    datePickerPattern->OnLanguageConfigurationUpdate();

    EXPECT_EQ(datePickerPattern->dateOrder_, DateTimeSequence().GetDateOrder(lang2).dateOrder);
}

/**
 * @tc.name: DatePickerCore010
 * @tc.desc: Test CreateFrameNode creates valid DatePicker with correct date order
 * @tc.type: FUNC
 */
HWTEST_F(DatePickerTestCore, DatePickerCore010, TestSize.Level1)
{
    const std::string language = "en";
    AceApplicationInfo::GetInstance().SetLocale(language, "US", "Latn", "");

    auto nodeId = ElementRegister::GetInstance()->MakeUniqueId();
    auto frameNode = DatePickerModelNG::CreateFrameNode(nodeId);
    ASSERT_NE(frameNode, nullptr);
    frameNode->MarkModifyDone();

    auto datePickerPattern = frameNode->GetPattern<DatePickerPattern>();
    ASSERT_NE(datePickerPattern, nullptr);

    EXPECT_TRUE(datePickerPattern->HasYearNode());
    EXPECT_TRUE(datePickerPattern->HasMonthNode());
    EXPECT_TRUE(datePickerPattern->HasDayNode());
}

/**
 * @tc.name: DatePickerCore011
 * @tc.desc: Test IsAtStartYear when selectedDate equals startDate
 * @tc.type: FUNC
 */
HWTEST_F(DatePickerTestCore, DatePickerCore011, TestSize.Level1)
{
    auto frameNode = CreateDatePickerNode();
    ASSERT_NE(frameNode, nullptr);
    auto datePickerPattern = frameNode->GetPattern<DatePickerPattern>();
    ASSERT_NE(datePickerPattern, nullptr);

    datePickerPattern->SetStartDate(PickerDate(2020, 1, 1));
    datePickerPattern->SetEndDate(PickerDate(2030, 12, 31));
    datePickerPattern->SetSelectDate(PickerDate(2020, 1, 1));
    frameNode->MarkModifyDone();

    EXPECT_TRUE(datePickerPattern->IsAtStartYear());
    EXPECT_FALSE(datePickerPattern->IsAtEndYear());
}

/**
 * @tc.name: DatePickerCore012
 * @tc.desc: Test IsAtEndYear when selectedDate equals endDate
 * @tc.type: FUNC
 */
HWTEST_F(DatePickerTestCore, DatePickerCore012, TestSize.Level1)
{
    auto frameNode = CreateDatePickerNode();
    ASSERT_NE(frameNode, nullptr);
    auto datePickerPattern = frameNode->GetPattern<DatePickerPattern>();
    ASSERT_NE(datePickerPattern, nullptr);

    datePickerPattern->SetStartDate(PickerDate(2020, 1, 1));
    datePickerPattern->SetEndDate(PickerDate(2030, 12, 31));
    datePickerPattern->SetSelectDate(PickerDate(2030, 12, 31));
    frameNode->MarkModifyDone();

    EXPECT_FALSE(datePickerPattern->IsAtStartYear());
    EXPECT_TRUE(datePickerPattern->IsAtEndYear());
}

/**
 * @tc.name: DatePickerCore013
 * @tc.desc: Test IsAtStartMonth when selectedDate equals startDate
 * @tc.type: FUNC
 */
HWTEST_F(DatePickerTestCore, DatePickerCore013, TestSize.Level1)
{
    auto frameNode = CreateDatePickerNode();
    ASSERT_NE(frameNode, nullptr);
    auto datePickerPattern = frameNode->GetPattern<DatePickerPattern>();
    ASSERT_NE(datePickerPattern, nullptr);

    datePickerPattern->SetStartDate(PickerDate(2020, 6, 15));
    datePickerPattern->SetEndDate(PickerDate(2030, 6, 20));
    datePickerPattern->SetSelectDate(PickerDate(2020, 6, 15));
    frameNode->MarkModifyDone();

    EXPECT_TRUE(datePickerPattern->IsAtStartYear());
    EXPECT_TRUE(datePickerPattern->IsAtStartMonth());
    EXPECT_FALSE(datePickerPattern->IsAtEndMonth());
}

/**
 * @tc.name: DatePickerCore014
 * @tc.desc: Test IsAtEndMonth when selectedDate equals endDate
 * @tc.type: FUNC
 */
HWTEST_F(DatePickerTestCore, DatePickerCore014, TestSize.Level1)
{
    auto frameNode = CreateDatePickerNode();
    ASSERT_NE(frameNode, nullptr);
    auto datePickerPattern = frameNode->GetPattern<DatePickerPattern>();
    ASSERT_NE(datePickerPattern, nullptr);

    datePickerPattern->SetStartDate(PickerDate(2020, 6, 15));
    datePickerPattern->SetEndDate(PickerDate(2030, 6, 20));
    datePickerPattern->SetSelectDate(PickerDate(2030, 6, 20));
    frameNode->MarkModifyDone();

    EXPECT_TRUE(datePickerPattern->IsAtEndYear());
    EXPECT_TRUE(datePickerPattern->IsAtEndMonth());
    EXPECT_FALSE(datePickerPattern->IsAtStartMonth());
}

/**
 * @tc.name: DatePickerCore015
 * @tc.desc: Test IsAtStartYear false when selectedDate is non-boundary
 * @tc.type: FUNC
 */
HWTEST_F(DatePickerTestCore, DatePickerCore015, TestSize.Level1)
{
    auto frameNode = CreateDatePickerNode();
    ASSERT_NE(frameNode, nullptr);
    auto datePickerPattern = frameNode->GetPattern<DatePickerPattern>();
    ASSERT_NE(datePickerPattern, nullptr);

    datePickerPattern->SetStartDate(PickerDate(2020, 1, 1));
    datePickerPattern->SetEndDate(PickerDate(2030, 12, 31));
    datePickerPattern->SetSelectDate(PickerDate(2025, 6, 15));
    frameNode->MarkModifyDone();

    EXPECT_FALSE(datePickerPattern->IsAtStartYear());
    EXPECT_FALSE(datePickerPattern->IsAtEndYear());
    EXPECT_FALSE(datePickerPattern->IsAtStartMonth());
    EXPECT_FALSE(datePickerPattern->IsAtEndMonth());
}

/**
 * @tc.name: DatePickerCore016
 * @tc.desc: Test GetCanLoopFromLayoutPropertyWithStartEnd for year column (always false)
 * @tc.type: FUNC
 */
HWTEST_F(DatePickerTestCore, DatePickerCore016, TestSize.Level1)
{
    auto frameNode = CreateDatePickerNode();
    ASSERT_NE(frameNode, nullptr);
    auto datePickerPattern = frameNode->GetPattern<DatePickerPattern>();
    ASSERT_NE(datePickerPattern, nullptr);

    datePickerPattern->SetStartDate(PickerDate(2020, 1, 1));
    datePickerPattern->SetEndDate(PickerDate(2030, 12, 31));
    datePickerPattern->SetSelectDate(PickerDate(2025, 6, 15));
    frameNode->MarkModifyDone();

    auto allChildNode = datePickerPattern->GetAllChildNode();
    auto yearNode = allChildNode["year"];
    ASSERT_NE(yearNode, nullptr);
    auto yearColumnPattern = yearNode->GetPattern<DatePickerColumnPattern>();
    ASSERT_NE(yearColumnPattern, nullptr);
    EXPECT_FALSE(yearColumnPattern->GetCanLoopFromLayoutPropertyWithStartEnd());
}

/**
 * @tc.name: DatePickerCore017
 * @tc.desc: Test GetCanLoopFromLayoutPropertyWithStartEnd for month column at boundary (false)
 * @tc.type: FUNC
 */
HWTEST_F(DatePickerTestCore, DatePickerCore017, TestSize.Level1)
{
    auto frameNode = CreateDatePickerNode();
    ASSERT_NE(frameNode, nullptr);
    auto datePickerPattern = frameNode->GetPattern<DatePickerPattern>();
    ASSERT_NE(datePickerPattern, nullptr);

    datePickerPattern->SetStartDate(PickerDate(2020, 1, 1));
    datePickerPattern->SetEndDate(PickerDate(2030, 12, 31));
    datePickerPattern->SetSelectDate(PickerDate(2020, 6, 15));
    frameNode->MarkModifyDone();

    auto allChildNode = datePickerPattern->GetAllChildNode();
    auto monthNode = allChildNode["month"];
    ASSERT_NE(monthNode, nullptr);
    auto monthColumnPattern = monthNode->GetPattern<DatePickerColumnPattern>();
    ASSERT_NE(monthColumnPattern, nullptr);
    EXPECT_FALSE(monthColumnPattern->GetCanLoopFromLayoutPropertyWithStartEnd());
}

/**
 * @tc.name: DatePickerCore018
 * @tc.desc: Test GetCanLoopFromLayoutPropertyWithStartEnd for month column non-boundary (true)
 * @tc.type: FUNC
 */
HWTEST_F(DatePickerTestCore, DatePickerCore018, TestSize.Level1)
{
    auto frameNode = CreateDatePickerNode();
    ASSERT_NE(frameNode, nullptr);
    auto datePickerPattern = frameNode->GetPattern<DatePickerPattern>();
    ASSERT_NE(datePickerPattern, nullptr);

    datePickerPattern->SetStartDate(PickerDate(2020, 1, 1));
    datePickerPattern->SetEndDate(PickerDate(2030, 12, 31));
    datePickerPattern->SetSelectDate(PickerDate(2025, 6, 15));
    frameNode->MarkModifyDone();

    auto allChildNode = datePickerPattern->GetAllChildNode();
    auto monthNode = allChildNode["month"];
    ASSERT_NE(monthNode, nullptr);
    auto monthColumnPattern = monthNode->GetPattern<DatePickerColumnPattern>();
    ASSERT_NE(monthColumnPattern, nullptr);
    EXPECT_TRUE(monthColumnPattern->GetCanLoopFromLayoutPropertyWithStartEnd());
}

/**
 * @tc.name: DatePickerCore019
 * @tc.desc: Test GetCanLoopFromLayoutPropertyWithStartEnd for day column at boundary (false)
 * @tc.type: FUNC
 */
HWTEST_F(DatePickerTestCore, DatePickerCore019, TestSize.Level1)
{
    auto frameNode = CreateDatePickerNode();
    ASSERT_NE(frameNode, nullptr);
    auto datePickerPattern = frameNode->GetPattern<DatePickerPattern>();
    ASSERT_NE(datePickerPattern, nullptr);

    datePickerPattern->SetStartDate(PickerDate(2020, 6, 15));
    datePickerPattern->SetEndDate(PickerDate(2030, 6, 20));
    datePickerPattern->SetSelectDate(PickerDate(2020, 6, 15));
    frameNode->MarkModifyDone();

    auto allChildNode = datePickerPattern->GetAllChildNode();
    auto dayNode = allChildNode["day"];
    ASSERT_NE(dayNode, nullptr);
    auto dayColumnPattern = dayNode->GetPattern<DatePickerColumnPattern>();
    ASSERT_NE(dayColumnPattern, nullptr);
    EXPECT_FALSE(dayColumnPattern->GetCanLoopFromLayoutPropertyWithStartEnd());
}

/**
 * @tc.name: DatePickerCore020
 * @tc.desc: Test GetCanLoopFromLayoutPropertyWithStartEnd for day column non-boundary (true)
 * @tc.type: FUNC
 */
HWTEST_F(DatePickerTestCore, DatePickerCore020, TestSize.Level1)
{
    auto frameNode = CreateDatePickerNode();
    ASSERT_NE(frameNode, nullptr);
    auto datePickerPattern = frameNode->GetPattern<DatePickerPattern>();
    ASSERT_NE(datePickerPattern, nullptr);

    datePickerPattern->SetStartDate(PickerDate(2020, 6, 15));
    datePickerPattern->SetEndDate(PickerDate(2030, 6, 20));
    datePickerPattern->SetSelectDate(PickerDate(2025, 6, 18));
    frameNode->MarkModifyDone();

    auto allChildNode = datePickerPattern->GetAllChildNode();
    auto dayNode = allChildNode["day"];
    ASSERT_NE(dayNode, nullptr);
    auto dayColumnPattern = dayNode->GetPattern<DatePickerColumnPattern>();
    ASSERT_NE(dayColumnPattern, nullptr);
    EXPECT_TRUE(dayColumnPattern->GetCanLoopFromLayoutPropertyWithStartEnd());
}

/**
 * @tc.name: DatePickerCore021
 * @tc.desc: Test GetCanLoopFromLayoutPropertyWithStartEnd when start=end (false)
 * @tc.type: FUNC
 */
HWTEST_F(DatePickerTestCore, DatePickerCore021, TestSize.Level1)
{
    auto frameNode = CreateDatePickerNode();
    ASSERT_NE(frameNode, nullptr);
    auto datePickerPattern = frameNode->GetPattern<DatePickerPattern>();
    ASSERT_NE(datePickerPattern, nullptr);

    datePickerPattern->SetStartDate(PickerDate(2025, 6, 15));
    datePickerPattern->SetEndDate(PickerDate(2025, 6, 15));
    datePickerPattern->SetSelectDate(PickerDate(2025, 6, 15));
    frameNode->MarkModifyDone();

    auto allChildNode = datePickerPattern->GetAllChildNode();
    auto monthNode = allChildNode["month"];
    ASSERT_NE(monthNode, nullptr);
    auto monthColumnPattern = monthNode->GetPattern<DatePickerColumnPattern>();
    ASSERT_NE(monthColumnPattern, nullptr);
    EXPECT_FALSE(monthColumnPattern->GetCanLoopFromLayoutPropertyWithStartEnd());
}

/**
 * @tc.name: DatePickerCore022
 * @tc.desc: Test GetCanLoopFromLayoutPropertyWithStartEnd when canLoop=false (false)
 * @tc.type: FUNC
 */
HWTEST_F(DatePickerTestCore, DatePickerCore022, TestSize.Level1)
{
    auto frameNode = CreateDatePickerNode();
    ASSERT_NE(frameNode, nullptr);
    auto datePickerPattern = frameNode->GetPattern<DatePickerPattern>();
    ASSERT_NE(datePickerPattern, nullptr);

    auto layoutProperty = frameNode->GetLayoutProperty<DataPickerRowLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    layoutProperty->UpdateCanLoop(false);

    datePickerPattern->SetStartDate(PickerDate(2020, 1, 1));
    datePickerPattern->SetEndDate(PickerDate(2030, 12, 31));
    datePickerPattern->SetSelectDate(PickerDate(2025, 6, 15));
    frameNode->MarkModifyDone();

    auto allChildNode = datePickerPattern->GetAllChildNode();
    auto monthNode = allChildNode["month"];
    ASSERT_NE(monthNode, nullptr);
    auto monthColumnPattern = monthNode->GetPattern<DatePickerColumnPattern>();
    ASSERT_NE(monthColumnPattern, nullptr);
    EXPECT_FALSE(monthColumnPattern->GetCanLoopFromLayoutPropertyWithStartEnd());
}

/**
 * @tc.name: DatePickerCore023
 * @tc.desc: Test IsTossNeedToStop returns inverse of GetCanLoopFromLayoutPropertyWithStartEnd
 * @tc.type: FUNC
 */
HWTEST_F(DatePickerTestCore, DatePickerCore023, TestSize.Level1)
{
    auto frameNode = CreateDatePickerNode();
    ASSERT_NE(frameNode, nullptr);
    auto datePickerPattern = frameNode->GetPattern<DatePickerPattern>();
    ASSERT_NE(datePickerPattern, nullptr);

    datePickerPattern->SetStartDate(PickerDate(2020, 1, 1));
    datePickerPattern->SetEndDate(PickerDate(2030, 12, 31));
    datePickerPattern->SetSelectDate(PickerDate(2025, 6, 15));
    frameNode->MarkModifyDone();

    auto allChildNode = datePickerPattern->GetAllChildNode();
    auto monthNode = allChildNode["month"];
    ASSERT_NE(monthNode, nullptr);
    auto monthColumnPattern = monthNode->GetPattern<DatePickerColumnPattern>();
    ASSERT_NE(monthColumnPattern, nullptr);

    bool canLoop = monthColumnPattern->GetCanLoopFromLayoutPropertyWithStartEnd();
    EXPECT_EQ(monthColumnPattern->IsTossNeedToStop(), !canLoop);
}

/**
 * @tc.name: DatePickerCore024
 * @tc.desc: Test GetAllChildNode returns 3 valid nodes
 * @tc.type: FUNC
 */
HWTEST_F(DatePickerTestCore, DatePickerCore024, TestSize.Level1)
{
    auto frameNode = CreateDatePickerNode();
    ASSERT_NE(frameNode, nullptr);
    auto datePickerPattern = frameNode->GetPattern<DatePickerPattern>();
    ASSERT_NE(datePickerPattern, nullptr);

    auto allChildNode = datePickerPattern->GetAllChildNode();
    EXPECT_NE(allChildNode["year"], nullptr);
    EXPECT_NE(allChildNode["month"], nullptr);
    EXPECT_NE(allChildNode["day"], nullptr);
}

} // namespace OHOS::Ace::NG

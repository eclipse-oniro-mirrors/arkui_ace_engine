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

#include "base/utils/date_util.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace {

class DateUtilGapTest : public testing::Test {
public:
    static void SetUpTestSuite() {}
    static void TearDownTestSuite() {}
};

/**
 * @tc.name: DateUtilGapIsLeapYear001
 * @tc.desc: Test IsLeapYear for all leap rule combinations.
 * @tc.type: FUNC
 */
HWTEST_F(DateUtilGapTest, DateUtilGapIsLeapYear001, TestSize.Level1)
{
    /**
     * @tc.steps: check divisible-by-4, by-100, by-400 and negative/zero years.
     * @tc.expected: (y%4==0 && y%100!=0) || y%400==0 decides leap year.
     */
    EXPECT_TRUE(Date::IsLeapYear(2024));   // %4 == 0, %100 != 0
    EXPECT_TRUE(Date::IsLeapYear(2000));   // %400 == 0
    EXPECT_FALSE(Date::IsLeapYear(2023));  // %4 != 0
    EXPECT_FALSE(Date::IsLeapYear(1900));  // %100 == 0 but %400 != 0
    EXPECT_TRUE(Date::IsLeapYear(0));      // %400 == 0
    EXPECT_TRUE(Date::IsLeapYear(-400));   // negative multiple of 400
    EXPECT_FALSE(Date::IsLeapYear(-100));  // negative %100 == 0
}

/**
 * @tc.name: DateUtilGapDayOfMonth001
 * @tc.desc: Test DayOfMonth across months and leap years.
 * @tc.type: FUNC
 */
HWTEST_F(DateUtilGapTest, DateUtilGapDayOfMonth001, TestSize.Level1)
{
    /**
     * @tc.steps: query month lengths in leap and non-leap years plus bad months.
     * @tc.expected: 31-day months, 30-day months, Feb 29/28, invalid month -> 0.
     */
    EXPECT_EQ(Date::DayOfMonth(2024, 1), 31);
    EXPECT_EQ(Date::DayOfMonth(2024, 3), 31);
    EXPECT_EQ(Date::DayOfMonth(2024, 5), 31);
    EXPECT_EQ(Date::DayOfMonth(2024, 7), 31);
    EXPECT_EQ(Date::DayOfMonth(2024, 8), 31);
    EXPECT_EQ(Date::DayOfMonth(2024, 10), 31);
    EXPECT_EQ(Date::DayOfMonth(2024, 12), 31);
    EXPECT_EQ(Date::DayOfMonth(2024, 4), 30);
    EXPECT_EQ(Date::DayOfMonth(2024, 6), 30);
    EXPECT_EQ(Date::DayOfMonth(2024, 9), 30);
    EXPECT_EQ(Date::DayOfMonth(2024, 11), 30);
    EXPECT_EQ(Date::DayOfMonth(2024, 2), 29); // leap
    EXPECT_EQ(Date::DayOfMonth(2023, 2), 28); // non-leap
    EXPECT_EQ(Date::DayOfMonth(2000, 2), 29); // %400 leap
    EXPECT_EQ(Date::DayOfMonth(1900, 2), 28); // %100 non-leap
    EXPECT_EQ(Date::DayOfMonth(2024, 0), 0);  // invalid month
    EXPECT_EQ(Date::DayOfMonth(2024, 13), 0); // invalid month
}

/**
 * @tc.name: DateUtilGapCalculateWeekDay001
 * @tc.desc: Test CalculateWeekDay against known dates.
 * @tc.type: FUNC
 */
HWTEST_F(DateUtilGapTest, DateUtilGapCalculateWeekDay001, TestSize.Level1)
{
    /**
     * @tc.steps: compute weekday for dates with known weekday, including
     *            January/February (the month-shift branch).
     * @tc.expected: 0 means Monday, 6 means Sunday.
     */
    EXPECT_EQ(Date::CalculateWeekDay(2024, 1, 1), 0);  // Monday
    EXPECT_EQ(Date::CalculateWeekDay(2024, 2, 29), 3); // Thursday
    EXPECT_EQ(Date::CalculateWeekDay(2000, 1, 1), 5);  // Saturday
    EXPECT_EQ(Date::CalculateWeekDay(2000, 3, 1), 2);  // Wednesday
    EXPECT_EQ(Date::CalculateWeekDay(1999, 12, 31), 4);  // Friday
    EXPECT_EQ(Date::CalculateWeekDay(2023, 6, 18), 6); // Sunday
}

/**
 * @tc.name: DateUtilGapCurrent001
 * @tc.desc: Test Date::Current returns sane local date fields.
 * @tc.type: FUNC
 */
HWTEST_F(DateUtilGapTest, DateUtilGapCurrent001, TestSize.Level1)
{
    /**
     * @tc.steps: query current date and validate field ranges.
     * @tc.expected: year >= 2024, month in [1,12], day in [1,31], week in [0,6].
     */
    auto now = Date::Current();
    EXPECT_GE(now.year, 2024u);
    EXPECT_GE(now.month, 1u);
    EXPECT_LE(now.month, 12u);
    EXPECT_GE(now.day, 1u);
    EXPECT_LE(now.day, 31u);
    EXPECT_LE(now.week, 6u);
}
} // namespace OHOS::Ace

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

#include <string>

#include "gtest/gtest.h"

#define protected public
#define private public

#include "test/mock/frameworks/core/common/mock_theme_manager.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"

#include "core/components_ng/pattern/calendar_picker/calendar_picker_model_ng.h"

#undef private
#undef protected

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {

class CalendarPickerModelTestNg : public testing::Test {
public:
    static void SetUpTestCase()
    {
        MockPipelineContext::SetUp();
        auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
        MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    }

    static void TearDownTestCase()
    {
        MockPipelineContext::GetCurrent()->SetThemeManager(nullptr);
        MockPipelineContext::TearDown();
    }
};

HWTEST_F(CalendarPickerModelTestNg, AddLeadingZeroToDate001, TestSize.Level1)
{
    EXPECT_EQ(CalendarPickerModelNG::AddLeadingZeroToDate(5, 2), "05");
}

HWTEST_F(CalendarPickerModelTestNg, AddLeadingZeroToDate002, TestSize.Level1)
{
    EXPECT_EQ(CalendarPickerModelNG::AddLeadingZeroToDate(12, 2), "12");
}

HWTEST_F(CalendarPickerModelTestNg, AddLeadingZeroToDate003, TestSize.Level1)
{
    EXPECT_EQ(CalendarPickerModelNG::AddLeadingZeroToDate(123, 2), "123");
}

HWTEST_F(CalendarPickerModelTestNg, AddLeadingZeroToDate004, TestSize.Level1)
{
    EXPECT_EQ(CalendarPickerModelNG::AddLeadingZeroToDate(5, 0), "5");
}

HWTEST_F(CalendarPickerModelTestNg, AddLeadingZeroToDate005, TestSize.Level1)
{
    EXPECT_EQ(CalendarPickerModelNG::AddLeadingZeroToDate(5, -1), "5");
}

HWTEST_F(CalendarPickerModelTestNg, AddLeadingZeroToDate006, TestSize.Level1)
{
    EXPECT_EQ(CalendarPickerModelNG::AddLeadingZeroToDate(0, 3), "000");
}

HWTEST_F(CalendarPickerModelTestNg, AddLeadingZeroToDate007, TestSize.Level1)
{
    EXPECT_EQ(CalendarPickerModelNG::AddLeadingZeroToDate(7, 4), "0007");
}

HWTEST_F(CalendarPickerModelTestNg, AddLeadingZeroToDate008, TestSize.Level1)
{
    EXPECT_EQ(CalendarPickerModelNG::AddLeadingZeroToDate(2024, 4), "2024");
}

} // namespace
} // namespace OHOS::Ace::NG

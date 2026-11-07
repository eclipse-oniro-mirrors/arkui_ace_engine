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
#include "test/unittest/core/pattern/test_ng.h"
#include "water_flow_item_maps.h"

#include "frameworks/core/components_ng/pattern/waterflow/water_flow_sections.h"

namespace OHOS::Ace::NG {
using namespace testing;
using namespace testing::ext;

class WaterFlowSectionTest : public TestNG {};

/**
 * @tc.name: Update001
 * @tc.desc: Test Update
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowSectionTest, Update001, TestSize.Level1)
{
    WaterFlowSections s;
    int32_t startIdx = -1;
    s.SetOnDataChange([&startIdx](int32_t start) { startIdx = start; });
    s.ChangeData(0, 0, SECTION_5);
    EXPECT_EQ(startIdx, 0);

    // update with the same data, should do nothing
    s.ChangeData(0, 5, SECTION_5);
    EXPECT_EQ(startIdx, 5);

    auto newSection = SECTION_5;
    newSection[3].itemsCount = 100;
    s.ChangeData(0, 5, newSection);
    EXPECT_EQ(startIdx, 4);

    // replace last section. item count update can be skipped
    newSection[4].itemsCount = 10;
    s.ChangeData(4, 1, { newSection[4] });
    EXPECT_EQ(startIdx, 5);

    // updating other properties can't be skipped
    newSection[4].crossCount = 3;
    s.ChangeData(4, 1, { newSection[4] });
    EXPECT_EQ(startIdx, 4);
    EXPECT_EQ(s.sections_.size(), 5);

    // illegal parameters
    s.ChangeData(4, 20, { newSection[4] });
    EXPECT_EQ(startIdx, 5);
    EXPECT_EQ(s.sections_.size(), 5);

    s.ChangeData(5, 20, { newSection[4] });
    EXPECT_EQ(startIdx, 5);
    EXPECT_EQ(s.sections_.size(), 6);
    EXPECT_EQ(s.sections_[5], newSection[4]);
}

/**
 * @tc.name: Update-CAPI001
 * @tc.desc: Test interface for CAPI
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowSectionTest, CAPI001, TestSize.Level1)
{
    WaterFlowSections s;
    int32_t startIdx = -1;
    s.SetOnDataChange([&startIdx](int32_t start) { startIdx = start; });

    s.ReplaceFrom(0, SECTION_7);
    EXPECT_EQ(startIdx, 0);

    s.ReplaceFrom(0, SECTION_7);
    EXPECT_EQ(startIdx, 4);

    auto sections = SECTION_7;
    sections[3].crossCount = 5;
    s.ReplaceFrom(0, sections);
    EXPECT_EQ(startIdx, 3);

    ++sections[2].itemsCount;
    s.ReplaceFrom(0, sections);
    EXPECT_EQ(startIdx, 3);

    --sections[3].itemsCount;
    s.ReplaceFrom(0, sections);
    EXPECT_EQ(startIdx, 4);
    EXPECT_EQ(s.sections_.size(), 4);

    s.ReplaceFrom(3, SECTION_7);
    EXPECT_EQ(startIdx, 3);
    EXPECT_EQ(s.sections_.size(), 7);

    // illegal
    s.ReplaceFrom(20, SECTION_7);
    EXPECT_EQ(startIdx, 7);
    EXPECT_EQ(s.sections_.size(), 11);
}

/**
 * @tc.name: Splice001
 * @tc.desc: Test ChangeData splicing sections in the middle keeps tail sections intact
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowSectionTest, Splice001, TestSize.Level1)
{
    WaterFlowSections s;
    int32_t startIdx = -1;
    int32_t notifyIdx = -1;
    s.SetOnDataChange([&startIdx](int32_t start) { startIdx = start; });
    s.SetNotifySectionChange([&notifyIdx](int32_t start) { notifyIdx = start; });
    s.ChangeData(0, 0, SECTION_5);
    EXPECT_EQ(startIdx, 0);
    // push does not trigger section change notification
    EXPECT_EQ(notifyIdx, -1);

    // splice in the middle: replace sections [1, 3) with 2 new ones, tail must survive
    const std::vector<WaterFlowSections::Section> newSections = {
        WaterFlowSections::Section { .itemsCount = 7, .crossCount = 4 },
        WaterFlowSections::Section { .itemsCount = 8, .crossCount = 6 },
    };
    s.ChangeData(1, 2, newSections);

    EXPECT_EQ(startIdx, 1);
    // itemCount(5) + deletedItemCount(5) + lastReplacedItemCount(0) - 1
    EXPECT_EQ(notifyIdx, 9);
    ASSERT_EQ(s.sections_.size(), 5);
    EXPECT_EQ(s.sections_[0], SECTION_5[0]);
    EXPECT_EQ(s.sections_[1], newSections[0]);
    EXPECT_EQ(s.sections_[2], newSections[1]);
    EXPECT_EQ(s.sections_[3], SECTION_5[3]);
    EXPECT_EQ(s.sections_[4], SECTION_5[4]);
    // prev sections captured before the change
    EXPECT_EQ(s.GetPrevSectionInfo().size(), 5);
    EXPECT_EQ(s.GetPrevSectionInfo()[1], SECTION_5[1]);
}

/**
 * @tc.name: Splice002
 * @tc.desc: Test ChangeData with different delete/insert combinations in the middle
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowSectionTest, Splice002, TestSize.Level1)
{
    WaterFlowSections s;
    int32_t startIdx = -1;
    s.SetOnDataChange([&startIdx](int32_t start) { startIdx = start; });
    s.ChangeData(0, 0, SECTION_5);

    // insert without deletion
    s.ChangeData(2, 0, { SECTION_5[2] });
    EXPECT_EQ(startIdx, 3);
    ASSERT_EQ(s.sections_.size(), 6);
    EXPECT_EQ(s.sections_[0], SECTION_5[0]);
    EXPECT_EQ(s.sections_[1], SECTION_5[1]);
    EXPECT_EQ(s.sections_[2], SECTION_5[2]);
    EXPECT_EQ(s.sections_[3], SECTION_5[2]);
    EXPECT_EQ(s.sections_[4], SECTION_5[3]);
    EXPECT_EQ(s.sections_[5], SECTION_5[4]);

    // grow: replace 1 section with 3
    const std::vector<WaterFlowSections::Section> growSections = {
        WaterFlowSections::Section { .itemsCount = 1, .crossCount = 7 },
        WaterFlowSections::Section { .itemsCount = 2, .crossCount = 8 },
        WaterFlowSections::Section { .itemsCount = 3, .crossCount = 9 },
    };
    s.ChangeData(1, 1, growSections);
    EXPECT_EQ(startIdx, 1);
    ASSERT_EQ(s.sections_.size(), 8);
    EXPECT_EQ(s.sections_[0], SECTION_5[0]);
    EXPECT_EQ(s.sections_[1], growSections[0]);
    EXPECT_EQ(s.sections_[2], growSections[1]);
    EXPECT_EQ(s.sections_[3], growSections[2]);
    EXPECT_EQ(s.sections_[4], SECTION_5[2]);
    EXPECT_EQ(s.sections_[5], SECTION_5[2]);
    EXPECT_EQ(s.sections_[6], SECTION_5[3]);
    EXPECT_EQ(s.sections_[7], SECTION_5[4]);

    // shrink: replace 3 sections with 1
    const WaterFlowSections::Section shrinkSection { .itemsCount = 4, .crossCount = 10 };
    s.ChangeData(2, 3, { shrinkSection });
    EXPECT_EQ(startIdx, 2);
    ASSERT_EQ(s.sections_.size(), 6);
    EXPECT_EQ(s.sections_[0], SECTION_5[0]);
    EXPECT_EQ(s.sections_[1], growSections[0]);
    EXPECT_EQ(s.sections_[2], shrinkSection);
    EXPECT_EQ(s.sections_[3], SECTION_5[2]);
    EXPECT_EQ(s.sections_[4], SECTION_5[3]);
    EXPECT_EQ(s.sections_[5], SECTION_5[4]);

    // delete up to the end
    const WaterFlowSections::Section tailSection { .itemsCount = 6, .crossCount = 11 };
    s.ChangeData(4, 2, { tailSection });
    EXPECT_EQ(startIdx, 4);
    ASSERT_EQ(s.sections_.size(), 5);
    EXPECT_EQ(s.sections_[0], SECTION_5[0]);
    EXPECT_EQ(s.sections_[1], growSections[0]);
    EXPECT_EQ(s.sections_[2], shrinkSection);
    EXPECT_EQ(s.sections_[3], SECTION_5[2]);
    EXPECT_EQ(s.sections_[4], tailSection);
}
} // namespace OHOS::Ace::NG

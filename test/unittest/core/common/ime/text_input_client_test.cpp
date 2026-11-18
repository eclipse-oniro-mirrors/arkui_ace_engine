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
 
#include "core/common/ime/text_input_client.h"
 
using namespace testing;
using namespace testing::ext;
 
namespace OHOS::Ace {
namespace {
// The autoFillKeyboardMap holds every non-UNSPECIFIED AceAutoFillType entry.
// AceAutoFillType has 39 enum values (ACE_UNSPECIFIED .. ACE_LICENSE_CHASSIS_NUMBER);
// ACE_UNSPECIFIED is intentionally excluded from the map, leaving 38 entries.
constexpr size_t EXPECTED_MAP_SIZE = 38;
} // namespace
 
class TextInputClientTest : public testing::Test {
public:
    void SetUp() override {}
    void TearDown() override {}
};
 
/**
 * @tc.name: GetAutoFillKeyboardMap001
 * @tc.desc: Test that GetAutoFillKeyboardMap returns a map containing every
 *           non-UNSPECIFIED AceAutoFillType entry (38 entries in total).
 * @tc.type: FUNC
 */
HWTEST_F(TextInputClientTest, GetAutoFillKeyboardMap001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. Fetch the shared keyboard map via the static accessor.
     */
    const auto& keyBoardMap = TextInputClient::GetAutoFillKeyboardMap();
 
    /**
     * @tc.steps: step2. Verify the map holds all 38 non-UNSPECIFIED entries.
     * @tc.expected: map.size() == 38 and ACE_UNSPECIFIED is not present.
     */
    EXPECT_EQ(keyBoardMap.size(), EXPECTED_MAP_SIZE);
    EXPECT_EQ(keyBoardMap.find(AceAutoFillType::ACE_UNSPECIFIED), keyBoardMap.end());
}
 
/**
 * @tc.name: GetAutoFillKeyboardMap002
 * @tc.desc: Test key AutoFillType -> TextInputType mappings used to drive IME
 *           keyboard selection (secure keyboard for password, email keyboard
 *           for email, phone keyboard for phone numbers, etc.).
 * @tc.type: FUNC
 */
HWTEST_F(TextInputClientTest, GetAutoFillKeyboardMap002, TestSize.Level0)
{
    /**
     * @tc.steps: step1. Fetch the shared keyboard map.
     */
    const auto& keyBoardMap = TextInputClient::GetAutoFillKeyboardMap();
 
    /**
     * @tc.steps: step2. Verify the security-critical and feature-critical mappings.
     * @tc.expected: Each listed AutoFillType maps to the documented TextInputType.
     */
    EXPECT_EQ(keyBoardMap.at(AceAutoFillType::ACE_PASSWORD), TextInputType::VISIBLE_PASSWORD);
    EXPECT_EQ(keyBoardMap.at(AceAutoFillType::ACE_USER_NAME), TextInputType::USER_NAME);
    EXPECT_EQ(keyBoardMap.at(AceAutoFillType::ACE_NEW_PASSWORD), TextInputType::NEW_PASSWORD);
    EXPECT_EQ(keyBoardMap.at(AceAutoFillType::ACE_EMAIL_ADDRESS), TextInputType::EMAIL_ADDRESS);
    EXPECT_EQ(keyBoardMap.at(AceAutoFillType::ACE_PHONE_NUMBER), TextInputType::PHONE);
    EXPECT_EQ(keyBoardMap.at(AceAutoFillType::ACE_PHONE_COUNTRY_CODE), TextInputType::PHONE);
    EXPECT_EQ(keyBoardMap.at(AceAutoFillType::ACE_FULL_PHONE_NUMBER), TextInputType::PHONE);
    EXPECT_EQ(keyBoardMap.at(AceAutoFillType::ACE_BANK_CARD_NUMBER), TextInputType::NUMBER);
    EXPECT_EQ(keyBoardMap.at(AceAutoFillType::ACE_ID_CARD_NUMBER), TextInputType::NUMBER);
    EXPECT_EQ(keyBoardMap.at(AceAutoFillType::ACE_PRECISE_TIME), TextInputType::NUMBER);
    EXPECT_EQ(keyBoardMap.at(AceAutoFillType::ACE_HOUR_AND_MINUTE), TextInputType::NUMBER);
    EXPECT_EQ(keyBoardMap.at(AceAutoFillType::ACE_DATE), TextInputType::NUMBER);
    EXPECT_EQ(keyBoardMap.at(AceAutoFillType::ACE_MONTH), TextInputType::NUMBER);
    EXPECT_EQ(keyBoardMap.at(AceAutoFillType::ACE_YEAR), TextInputType::NUMBER);
    EXPECT_EQ(keyBoardMap.at(AceAutoFillType::ACE_FULL_STREET_ADDRESS), TextInputType::TEXT);
    EXPECT_EQ(keyBoardMap.at(AceAutoFillType::ACE_PERSON_FULL_NAME), TextInputType::TEXT);
    EXPECT_EQ(keyBoardMap.at(AceAutoFillType::ACE_NICKNAME), TextInputType::TEXT);
    EXPECT_EQ(keyBoardMap.at(AceAutoFillType::ACE_PASSPORT_NUMBER), TextInputType::TEXT);
    EXPECT_EQ(keyBoardMap.at(AceAutoFillType::ACE_ORGANIZATION), TextInputType::TEXT);
}
 
/**
 * @tc.name: GetAutoFillKeyboardMap003
 * @tc.desc: Test that repeated calls to GetAutoFillKeyboardMap return references
 *           to the same underlying static map instance (singleton stability).
 * @tc.type: FUNC
 */
HWTEST_F(TextInputClientTest, GetAutoFillKeyboardMap003, TestSize.Level0)
{
    /**
     * @tc.steps: step1. Fetch the map twice via the static accessor.
     */
    const auto& firstRef = TextInputClient::GetAutoFillKeyboardMap();
    const auto& secondRef = TextInputClient::GetAutoFillKeyboardMap();
 
    /**
     * @tc.steps: step2. Verify both references point to the same map instance.
     * @tc.expected: &firstRef == &secondRef.
     */
    EXPECT_EQ(&firstRef, &secondRef);
}
 
/**
 * @tc.name: GetAutoFillKeyboardMap004
 * @tc.desc: Test that ACE_UNSPECIFIED (the default/auto-fill-disabled content
 *           type) is intentionally absent from the keyboard map so callers
 *           fall back to UNSPECIFIED TextInputType.
 * @tc.type: FUNC
 */
HWTEST_F(TextInputClientTest, GetAutoFillKeyboardMap004, TestSize.Level0)
{
    /**
     * @tc.steps: step1. Fetch the shared keyboard map.
     */
    const auto& keyBoardMap = TextInputClient::GetAutoFillKeyboardMap();
 
    /**
     * @tc.steps: step2. Look up ACE_UNSPECIFIED in the map.
     * @tc.expected: find() returns end() (entry does not exist).
     */
    auto it = keyBoardMap.find(AceAutoFillType::ACE_UNSPECIFIED);
    EXPECT_EQ(it, keyBoardMap.end());
}
} // namespace OHOS::Ace
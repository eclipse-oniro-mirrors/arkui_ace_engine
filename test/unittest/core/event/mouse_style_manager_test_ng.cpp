/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "test/unittest/core/event/mouse_style_manager_test_ng.h"
#include <optional>

#include "test/mock/core/pipeline/mock_pipeline_context.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
void MouseStyleManagerTestNG::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "MouseStyleManagerTestNG SetUpTestCase";
}

void MouseStyleManagerTestNG::TearDownTestSuite()
{
    GTEST_LOG_(INFO) << "MouseStyleManagerTestNG TearDownTestCase";
}

void MouseStyleManagerTestNG::SetUp()
{
    MockPipelineContext::SetUp();
}

void MouseStyleManagerTestNG::TearDown()
{
    MockPipelineContext::TearDown();
}

/**
 * @tc.name: MouseStyleManager001
 * @tc.desc: Test case userSetCursor_ is false.
 * @tc.type: FUNC
 */
HWTEST_F(MouseStyleManagerTestNG, MouseStyleManager001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create mouseStyleManager.
     * @tc.expected: mouseStyleManager is not null.
     */
    auto mouseStyleManager = AceType::MakeRefPtr<MouseStyleManager>();
    ASSERT_NE(mouseStyleManager, nullptr);

    /*
     * @tc.steps: step2. Create testCases.
     * @tc.expected: result is correct.
     */
    const std::vector<MockMouseFormatCase> mockMouseFormatCases = {
        {false, std::nullopt, 0, MouseFormat::EAST, false,
            MouseStyleChangeReason::INNER_SET_MOUSESTYLE, false, MouseFormat::DEFAULT},
        {false, std::nullopt, 0, MouseFormat::EAST, false,
            MouseStyleChangeReason::USER_SET_MOUSESTYLE, true, MouseFormat::EAST},
        {false, 0, 0, MouseFormat::EAST, false,
            MouseStyleChangeReason::INNER_SET_MOUSESTYLE, true, MouseFormat::EAST},
        {false, 0, 0, MouseFormat::EAST, false,
            MouseStyleChangeReason::USER_SET_MOUSESTYLE, true, MouseFormat::EAST},
        {false, 0, 1, MouseFormat::EAST, false,
            MouseStyleChangeReason::INNER_SET_MOUSESTYLE, false, MouseFormat::DEFAULT},
        {false, 0, 1, MouseFormat::EAST, false,
            MouseStyleChangeReason::USER_SET_MOUSESTYLE, true, MouseFormat::EAST}
    };

    /*
     * @tc.steps: step3. Run testCases.
     * @tc.expected: result is correct.
     */
    for (auto i = 0; i < mockMouseFormatCases.size(); i++) {
        mouseStyleManager->mouseFormat_ = MouseFormat::DEFAULT;
        mouseStyleManager->userSetCursor_ = mockMouseFormatCases[i].userSetCursor;
        mouseStyleManager->mouseStyleNodeId_ = mockMouseFormatCases[i].mouseStyleNodeId;
        auto result = mouseStyleManager->SetMouseFormat(1, mockMouseFormatCases[i].nodeId,
            mockMouseFormatCases[i].mouseFormat,
            mockMouseFormatCases[i].isByPass,
            mockMouseFormatCases[i].reason);
        EXPECT_EQ(result, mockMouseFormatCases[i].expectSetResult);
        mouseStyleManager->VsyncMouseFormat();
        EXPECT_EQ(mouseStyleManager->mouseFormat_,
            mockMouseFormatCases[i].expectMouseFormat);
    }
}

/**
 * @tc.name: MouseStyleManager002
 * @tc.desc: Test case userSetCursor_ is true.
 * @tc.type: FUNC
 */
HWTEST_F(MouseStyleManagerTestNG, MouseStyleManager002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create mouseStyleManager.
     * @tc.expected: mouseStyleManager is not null.
     */
    auto mouseStyleManager = AceType::MakeRefPtr<MouseStyleManager>();
    ASSERT_NE(mouseStyleManager, nullptr);

    /*
     * @tc.steps: step2. Create testCases.
     * @tc.expected: result is correct.
     */
    const std::vector<MockMouseFormatCase> mockMouseFormatCases = {
        {true, std::nullopt, 0, MouseFormat::EAST, false,
            MouseStyleChangeReason::INNER_SET_MOUSESTYLE, false, MouseFormat::DEFAULT},
        {true, std::nullopt, 0, MouseFormat::EAST, false,
            MouseStyleChangeReason::USER_SET_MOUSESTYLE, true, MouseFormat::EAST},
        {true, 0, 0, MouseFormat::EAST, false,
            MouseStyleChangeReason::INNER_SET_MOUSESTYLE, false, MouseFormat::DEFAULT},
        {true, 0, 0, MouseFormat::EAST, false,
            MouseStyleChangeReason::USER_SET_MOUSESTYLE, true, MouseFormat::EAST},
        {true, 0, 1, MouseFormat::EAST, false,
            MouseStyleChangeReason::INNER_SET_MOUSESTYLE, false, MouseFormat::DEFAULT},
        {true, 0, 1, MouseFormat::EAST, false,
            MouseStyleChangeReason::USER_SET_MOUSESTYLE, true, MouseFormat::EAST}
    };

    /*
     * @tc.steps: step3. Run testCases.
     * @tc.expected: result is correct.
     */
    for (auto i = 0; i < mockMouseFormatCases.size(); i++) {
        mouseStyleManager->mouseFormat_ = MouseFormat::DEFAULT;
        mouseStyleManager->userSetCursor_ = mockMouseFormatCases[i].userSetCursor;
        mouseStyleManager->mouseStyleNodeId_ = mockMouseFormatCases[i].mouseStyleNodeId;
        auto result = mouseStyleManager->SetMouseFormat(1, mockMouseFormatCases[i].nodeId,
            mockMouseFormatCases[i].mouseFormat,
            mockMouseFormatCases[i].isByPass,
            mockMouseFormatCases[i].reason);
        EXPECT_EQ(result, mockMouseFormatCases[i].expectSetResult);
        mouseStyleManager->VsyncMouseFormat();
        EXPECT_EQ(mouseStyleManager->mouseFormat_,
            mockMouseFormatCases[i].expectMouseFormat);
    }
}

/**
 * @tc.name: MouseStyleManager003
 * @tc.desc: Test case isByPass is true.
 * @tc.type: FUNC
 */
HWTEST_F(MouseStyleManagerTestNG, MouseStyleManager003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create mouseStyleManager.
     * @tc.expected: mouseStyleManager is not null.
     */
    auto mouseStyleManager = AceType::MakeRefPtr<MouseStyleManager>();
    ASSERT_NE(mouseStyleManager, nullptr);

    /*
     * @tc.steps: step2. Create testCases.
     * @tc.expected: result is correct.
     */
    const std::vector<MockMouseFormatCase> mockMouseFormatCases = {
        {false, std::nullopt, 0, MouseFormat::EAST, true,
            MouseStyleChangeReason::INNER_SET_MOUSESTYLE, false, MouseFormat::DEFAULT},
        {false, std::nullopt, 0, MouseFormat::EAST, true,
            MouseStyleChangeReason::USER_SET_MOUSESTYLE, false, MouseFormat::DEFAULT}
    };

    /*
     * @tc.steps: step3. Run testCases.
     * @tc.expected: result is correct.
     */
    for (auto i = 0; i < mockMouseFormatCases.size(); i++) {
        mouseStyleManager->mouseFormat_ = MouseFormat::DEFAULT;
        mouseStyleManager->userSetCursor_ = mockMouseFormatCases[i].userSetCursor;
        mouseStyleManager->mouseStyleNodeId_ = mockMouseFormatCases[i].mouseStyleNodeId;
        auto result = mouseStyleManager->SetMouseFormat(1, mockMouseFormatCases[i].nodeId,
            mockMouseFormatCases[i].mouseFormat,
            mockMouseFormatCases[i].isByPass,
            mockMouseFormatCases[i].reason);
        EXPECT_EQ(result, mockMouseFormatCases[i].expectSetResult);
        mouseStyleManager->VsyncMouseFormat();
        EXPECT_EQ(mouseStyleManager->mouseFormat_,
            mockMouseFormatCases[i].expectMouseFormat);
    }
}

/**
 * @tc.name: MouseStyleManager004
 * @tc.desc: Test case one vsync have different input.
 * @tc.type: FUNC
 */
HWTEST_F(MouseStyleManagerTestNG, MouseStyleManager004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create mouseStyleManager.
     * @tc.expected: mouseStyleManager is not null.
     */
    auto mouseStyleManager = AceType::MakeRefPtr<MouseStyleManager>();
    ASSERT_NE(mouseStyleManager, nullptr);
    bool result = false;

    /*
     * @tc.steps: step2. Create testCases.
     * @tc.expected: result is correct.
     */
    const std::vector<MockMouseFormatCase> mockMouseFormatCases = {
        {false, std::nullopt, 0, MouseFormat::EAST, false,
            MouseStyleChangeReason::INNER_SET_MOUSESTYLE, false, MouseFormat::DEFAULT},
        {false, std::nullopt, 0, MouseFormat::EAST, false,
            MouseStyleChangeReason::USER_SET_MOUSESTYLE, true, MouseFormat::EAST},
        {false, 0, 0, MouseFormat::EAST, false,
            MouseStyleChangeReason::INNER_SET_MOUSESTYLE, false, MouseFormat::EAST},
        {false, 0, 0, MouseFormat::EAST, false,
            MouseStyleChangeReason::USER_SET_MOUSESTYLE, true, MouseFormat::EAST},
        {false, 0, 1, MouseFormat::EAST, false,
            MouseStyleChangeReason::INNER_SET_MOUSESTYLE, false, MouseFormat::EAST},
        {false, 0, 1, MouseFormat::EAST, false,
            MouseStyleChangeReason::USER_SET_MOUSESTYLE, true, MouseFormat::EAST}
    };

    /*
     * @tc.steps: step3. Run testCases.
     * @tc.expected: result is correct.
     */
    for (auto i = 0; i < mockMouseFormatCases.size() - 1; i++) {
        mouseStyleManager->mouseFormat_ = MouseFormat::DEFAULT;
        mouseStyleManager->userSetCursor_ = mockMouseFormatCases[i].userSetCursor;
        mouseStyleManager->mouseStyleNodeId_ = mockMouseFormatCases[i].mouseStyleNodeId;
        result = mouseStyleManager->SetMouseFormat(1, mockMouseFormatCases[i].nodeId,
            mockMouseFormatCases[i].mouseFormat,
            mockMouseFormatCases[i].isByPass,
            mockMouseFormatCases[i].reason);
        result = mouseStyleManager->SetMouseFormat(1, mockMouseFormatCases[i + 1].nodeId,
            mockMouseFormatCases[i + 1].mouseFormat,
            mockMouseFormatCases[i + 1].isByPass,
            mockMouseFormatCases[i + 1].reason);
        EXPECT_EQ(result, mockMouseFormatCases[i + 1].expectSetResult);
        mouseStyleManager->VsyncMouseFormat();
        EXPECT_EQ(mouseStyleManager->mouseFormat_,
            mockMouseFormatCases[i + 1].expectMouseFormat);
    }
}

/**
 * @tc.name: MouseStyleManager005
 * @tc.desc: Test case userSetCursor_ is different.
 * @tc.type: FUNC
 */
HWTEST_F(MouseStyleManagerTestNG, MouseStyleManager005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create mouseStyleManager.
     * @tc.expected: mouseStyleManager is not null.
     */
    auto mouseStyleManager = AceType::MakeRefPtr<MouseStyleManager>();
    ASSERT_NE(mouseStyleManager, nullptr);
    bool result = false;

    /*
     * @tc.steps: step2. Create testCases.
     * @tc.expected: result is correct.
     */
    const std::vector<MockMouseFormatCase> mockMouseFormatCases = {
        {false, std::nullopt, 0, MouseFormat::EAST, false,
            MouseStyleChangeReason::INNER_SET_MOUSESTYLE, false, MouseFormat::DEFAULT},
        {true, std::nullopt, 0, MouseFormat::EAST, false,
            MouseStyleChangeReason::USER_SET_MOUSESTYLE, true, MouseFormat::EAST},
        {true, 0, 0, MouseFormat::EAST, false,
            MouseStyleChangeReason::INNER_SET_MOUSESTYLE, false, MouseFormat::DEFAULT},
        {false, 0, 0, MouseFormat::EAST, false,
            MouseStyleChangeReason::USER_SET_MOUSESTYLE, true, MouseFormat::EAST},
        {false, 0, 1, MouseFormat::EAST, false,
            MouseStyleChangeReason::INNER_SET_MOUSESTYLE, false, MouseFormat::DEFAULT},
        {true, 0, 1, MouseFormat::EAST, false,
            MouseStyleChangeReason::USER_SET_MOUSESTYLE, true, MouseFormat::EAST}
    };

    /*
     * @tc.steps: step3. Run testCases.
     * @tc.expected: result is correct.
     */
    for (auto i = 0; i < mockMouseFormatCases.size(); i++) {
        mouseStyleManager->mouseFormat_ = MouseFormat::DEFAULT;
        mouseStyleManager->userSetCursor_ = !mockMouseFormatCases[i].userSetCursor;
        mouseStyleManager->mouseStyleNodeId_ = mockMouseFormatCases[i].mouseStyleNodeId;
        mouseStyleManager->SetUserSetCursor(mockMouseFormatCases[i].userSetCursor);
        result = mouseStyleManager->SetMouseFormat(1, mockMouseFormatCases[i].nodeId,
            mockMouseFormatCases[i].mouseFormat,
            mockMouseFormatCases[i].isByPass,
            mockMouseFormatCases[i].reason);
        EXPECT_EQ(result, mockMouseFormatCases[i].expectSetResult);
        mouseStyleManager->VsyncMouseFormat();
        EXPECT_EQ(mouseStyleManager->mouseFormat_,
            mockMouseFormatCases[i].expectMouseFormat);
    }
}

/**
 * @tc.name: MouseStyleManager006
 * @tc.desc: Test all case when passBy = true.
 * @tc.type: FUNC
 */
HWTEST_F(MouseStyleManagerTestNG, MouseStyleManager006, TestSize.Level1)
{
    auto mouseStyleManager = AceType::MakeRefPtr<MouseStyleManager>();
    ASSERT_NE(mouseStyleManager, nullptr);

    // test all case when passBy = true.
    std::vector<MockMouseFormatCase> mockMouseFormatCases = {};
    std::vector<bool> userSetCursor = { false, true };
    std::vector<MouseStyleChangeReason> reason = { 
        MouseStyleChangeReason::INNER_SET_MOUSESTYLE,
        MouseStyleChangeReason::USER_SET_MOUSESTYLE, MouseStyleChangeReason::CONTAINER_DESTROY_RESET_MOUSESTYLE,
        MouseStyleChangeReason::WINDOW_LOST_FOCUS_RESET_MOUSESTYLE,
        MouseStyleChangeReason::WINDOW_SCENE_LOST_FOCUS_RESET_MOUSESTYLE };
    std::vector<std::optional<int32_t>> mouseStyleNodeId = { std::nullopt, 1 };
    std::vector<int32_t> nodeId = { -1, 1 };

    for (int32_t i = 0; i < userSetCursor.size(); ++i) {
        for (int32_t j = 0; j < reason.size(); ++j) {
            for (int32_t k = 0; k < mouseStyleNodeId.size(); ++k) {
                for (int32_t m = 0; m < nodeId.size(); ++m) {
                    mockMouseFormatCases.push_back({ userSetCursor[i], mouseStyleNodeId[k], nodeId[m],
                        MouseFormat::EAST, true, reason[j], false, MouseFormat::DEFAULT });
                }
            }
        }
    }

    for (int32_t i = 0; i < mockMouseFormatCases.size(); ++i) {
        mouseStyleManager->mouseFormat_ = MouseFormat::DEFAULT;
        mouseStyleManager->userSetCursor_ = mockMouseFormatCases[i].userSetCursor;
        mouseStyleManager->mouseStyleNodeId_ = mockMouseFormatCases[i].mouseStyleNodeId;
        auto result = mouseStyleManager->SetMouseFormat(1, mockMouseFormatCases[i].nodeId,
            mockMouseFormatCases[i].mouseFormat,
            mockMouseFormatCases[i].isByPass,
            mockMouseFormatCases[i].reason);
        EXPECT_EQ(result, mockMouseFormatCases[i].expectSetResult);
        mouseStyleManager->VsyncMouseFormat();
        EXPECT_EQ(mouseStyleManager->mouseFormat_,
            mockMouseFormatCases[i].expectMouseFormat);
    }
}

/**
 * @tc.name: MouseStyleManager007
 * @tc.desc: Test all case when passBy = false.
 * @tc.type: FUNC
 */
HWTEST_F(MouseStyleManagerTestNG, MouseStyleManager007, TestSize.Level1)
{
    auto mouseStyleManager = AceType::MakeRefPtr<MouseStyleManager>();
    ASSERT_NE(mouseStyleManager, nullptr);

    const std::vector<MockMouseFormatCase> mockMouseFormatCases = {
        // INNER_SET_MOUSESTYLE
        { false, std::nullopt, -1, MouseFormat::EAST, false, MouseStyleChangeReason::INNER_SET_MOUSESTYLE, false,
            MouseFormat::DEFAULT },
        { false, std::nullopt, 1, MouseFormat::EAST, false, MouseStyleChangeReason::INNER_SET_MOUSESTYLE, false,
            MouseFormat::DEFAULT },
        { false, 1, -1, MouseFormat::EAST, false, MouseStyleChangeReason::INNER_SET_MOUSESTYLE, false,
            MouseFormat::DEFAULT },
        { false, 1, 1, MouseFormat::EAST, false, MouseStyleChangeReason::INNER_SET_MOUSESTYLE, true,
            MouseFormat::EAST },
        // USER_SET_MOUSESTYLE
        { false, std::nullopt, -1, MouseFormat::EAST, false, MouseStyleChangeReason::USER_SET_MOUSESTYLE, true,
            MouseFormat::EAST },
        { false, std::nullopt, 1, MouseFormat::EAST, false, MouseStyleChangeReason::USER_SET_MOUSESTYLE, true,
            MouseFormat::EAST },
        { false, 1, -1, MouseFormat::EAST, false, MouseStyleChangeReason::USER_SET_MOUSESTYLE, true,
            MouseFormat::EAST },
        { false, 1, 1, MouseFormat::EAST, false, MouseStyleChangeReason::USER_SET_MOUSESTYLE, true, MouseFormat::EAST },
        // CONTAINER_DESTROY_RESET_MOUSESTYLE
        { false, std::nullopt, -1, MouseFormat::EAST, false, MouseStyleChangeReason::CONTAINER_DESTROY_RESET_MOUSESTYLE,
            true, MouseFormat::EAST },
        { false, std::nullopt, 1, MouseFormat::EAST, false, MouseStyleChangeReason::CONTAINER_DESTROY_RESET_MOUSESTYLE,
            true, MouseFormat::EAST },
        { false, 1, -1, MouseFormat::EAST, false, MouseStyleChangeReason::CONTAINER_DESTROY_RESET_MOUSESTYLE, true,
            MouseFormat::EAST },
        { false, 1, 1, MouseFormat::EAST, false, MouseStyleChangeReason::CONTAINER_DESTROY_RESET_MOUSESTYLE, true,
            MouseFormat::EAST },
        // WINDOW_LOST_FOCUS_RESET_MOUSESTYLE
        { false, std::nullopt, -1, MouseFormat::EAST, false, MouseStyleChangeReason::WINDOW_LOST_FOCUS_RESET_MOUSESTYLE,
            true, MouseFormat::EAST },
        { false, std::nullopt, 1, MouseFormat::EAST, false, MouseStyleChangeReason::WINDOW_LOST_FOCUS_RESET_MOUSESTYLE,
            true, MouseFormat::EAST },
        { false, 1, -1, MouseFormat::EAST, false, MouseStyleChangeReason::WINDOW_LOST_FOCUS_RESET_MOUSESTYLE, true,
            MouseFormat::EAST },
        { false, 1, 1, MouseFormat::EAST, false, MouseStyleChangeReason::WINDOW_LOST_FOCUS_RESET_MOUSESTYLE, true,
            MouseFormat::EAST },
        // WINDOW_SCENE_LOST_FOCUS_RESET_MOUSESTYLE
        { false, std::nullopt, -1, MouseFormat::EAST, false,
            MouseStyleChangeReason::WINDOW_SCENE_LOST_FOCUS_RESET_MOUSESTYLE, true, MouseFormat::EAST },
        { false, std::nullopt, 1, MouseFormat::EAST, false,
            MouseStyleChangeReason::WINDOW_SCENE_LOST_FOCUS_RESET_MOUSESTYLE, true, MouseFormat::EAST },
        { false, 1, -1, MouseFormat::EAST, false, MouseStyleChangeReason::WINDOW_SCENE_LOST_FOCUS_RESET_MOUSESTYLE,
            true, MouseFormat::EAST },
        { false, 1, 1, MouseFormat::EAST, false, MouseStyleChangeReason::WINDOW_SCENE_LOST_FOCUS_RESET_MOUSESTYLE, true,
            MouseFormat::EAST },
        // INNER_SET_MOUSESTYLE
        { true, std::nullopt, -1, MouseFormat::EAST, false, MouseStyleChangeReason::INNER_SET_MOUSESTYLE, false,
            MouseFormat::DEFAULT },
        { true, std::nullopt, 1, MouseFormat::EAST, false, MouseStyleChangeReason::INNER_SET_MOUSESTYLE, false,
            MouseFormat::DEFAULT },
        { true, 1, -1, MouseFormat::EAST, false, MouseStyleChangeReason::INNER_SET_MOUSESTYLE, false,
            MouseFormat::DEFAULT },
        { true, 1, 1, MouseFormat::EAST, false, MouseStyleChangeReason::INNER_SET_MOUSESTYLE, false,
            MouseFormat::DEFAULT },
        // USER_SET_MOUSESTYLE
        { true, std::nullopt, -1, MouseFormat::EAST, false, MouseStyleChangeReason::USER_SET_MOUSESTYLE, true,
            MouseFormat::EAST },
        { true, std::nullopt, 1, MouseFormat::EAST, false, MouseStyleChangeReason::USER_SET_MOUSESTYLE, true,
            MouseFormat::EAST },
        { true, 1, -1, MouseFormat::EAST, false, MouseStyleChangeReason::USER_SET_MOUSESTYLE, true, MouseFormat::EAST },
        { true, 1, 1, MouseFormat::EAST, false, MouseStyleChangeReason::USER_SET_MOUSESTYLE, true, MouseFormat::EAST },
        // CONTAINER_DESTROY_RESET_MOUSESTYLE
        { true, std::nullopt, -1, MouseFormat::EAST, false, MouseStyleChangeReason::CONTAINER_DESTROY_RESET_MOUSESTYLE,
            true, MouseFormat::EAST },
        { true, std::nullopt, 1, MouseFormat::EAST, false, MouseStyleChangeReason::CONTAINER_DESTROY_RESET_MOUSESTYLE,
            true, MouseFormat::EAST },
        { true, 1, -1, MouseFormat::EAST, false, MouseStyleChangeReason::CONTAINER_DESTROY_RESET_MOUSESTYLE, true,
            MouseFormat::EAST },
        { true, 1, 1, MouseFormat::EAST, false, MouseStyleChangeReason::CONTAINER_DESTROY_RESET_MOUSESTYLE, true,
            MouseFormat::EAST },
        // WINDOW_LOST_FOCUS_RESET_MOUSESTYLE
        { true, std::nullopt, -1, MouseFormat::EAST, false, MouseStyleChangeReason::WINDOW_LOST_FOCUS_RESET_MOUSESTYLE,
            true, MouseFormat::EAST },
        { true, std::nullopt, 1, MouseFormat::EAST, false, MouseStyleChangeReason::WINDOW_LOST_FOCUS_RESET_MOUSESTYLE,
            true, MouseFormat::EAST },
        { true, 1, -1, MouseFormat::EAST, false, MouseStyleChangeReason::WINDOW_LOST_FOCUS_RESET_MOUSESTYLE, true,
            MouseFormat::EAST },
        { true, 1, 1, MouseFormat::EAST, false, MouseStyleChangeReason::WINDOW_LOST_FOCUS_RESET_MOUSESTYLE, true,
            MouseFormat::EAST },
        // WINDOW_SCENE_LOST_FOCUS_RESET_MOUSESTYLE
        { true, std::nullopt, -1, MouseFormat::EAST, false,
            MouseStyleChangeReason::WINDOW_SCENE_LOST_FOCUS_RESET_MOUSESTYLE, true, MouseFormat::EAST },
        { true, std::nullopt, 1, MouseFormat::EAST, false,
            MouseStyleChangeReason::WINDOW_SCENE_LOST_FOCUS_RESET_MOUSESTYLE, true, MouseFormat::EAST },
        { true, 1, -1, MouseFormat::EAST, false, MouseStyleChangeReason::WINDOW_SCENE_LOST_FOCUS_RESET_MOUSESTYLE, true,
            MouseFormat::EAST },
        { true, 1, 1, MouseFormat::EAST, false, MouseStyleChangeReason::WINDOW_SCENE_LOST_FOCUS_RESET_MOUSESTYLE, true,
            MouseFormat::EAST }
    };

    for (int32_t i = 0; i < mockMouseFormatCases.size(); ++i) {
        mouseStyleManager->mouseFormat_ = MouseFormat::DEFAULT;
        mouseStyleManager->userSetCursor_ = mockMouseFormatCases[i].userSetCursor;
        mouseStyleManager->mouseStyleNodeId_ = mockMouseFormatCases[i].mouseStyleNodeId;
        auto result = mouseStyleManager->SetMouseFormat(1, mockMouseFormatCases[i].nodeId,
            mockMouseFormatCases[i].mouseFormat, mockMouseFormatCases[i].isByPass, mockMouseFormatCases[i].reason);
        EXPECT_EQ(result, mockMouseFormatCases[i].expectSetResult);
        mouseStyleManager->VsyncMouseFormat();
        EXPECT_EQ(mouseStyleManager->mouseFormat_, mockMouseFormatCases[i].expectMouseFormat);
    }
}
} // namespace OHOS::Ace::NG

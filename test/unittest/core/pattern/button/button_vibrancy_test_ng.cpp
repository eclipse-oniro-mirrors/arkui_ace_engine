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

#include <gtest/gtest.h>

#define protected public
#define private public
#include "test/mock/frameworks/core/common/mock_theme_manager.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"
#include "test/mock/adapter/ohos/osal/mock_system_properties.h"

#include "base/memory/ace_type.h"
#include "core/accessibility/accessibility_manager.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/button/button_layout_property.h"
#include "core/components_ng/pattern/button/button_model_ng.h"
#include "core/components_ng/pattern/button/button_pattern.h"
#include "core/components_ng/pattern/text/text_pattern.h"
#include "ui/properties/ui_material_enums.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {
constexpr int32_t NODE_ID = 10;
constexpr int32_t TEXT_NODE_ID = 11;
}

class ButtonVibrancyTestNg : public testing::Test {
public:
    static void SetUpTestSuite()
    {
        MockPipelineContext::SetUp();
    }
    static void TearDownTestSuite()
    {
        MockPipelineContext::TearDown();
    }
};

/**
 * @tc.name: SetVibrancyMode001
 * @tc.desc: Test ButtonModelNG::SetVibrancyMode stores BRIGHTEN into LayoutProperty.
 * @tc.type: FUNC
 */
HWTEST_F(ButtonVibrancyTestNg, SetVibrancyMode001, TestSize.Level1)
{
    auto frameNode = FrameNode::CreateFrameNode(V2::BUTTON_ETS_TAG, NODE_ID, AceType::MakeRefPtr<ButtonPattern>());
    ASSERT_NE(frameNode, nullptr);
    ButtonModelNG::SetVibrancyMode(AccessibilityManager::RawPtr(frameNode), VibrancyMode::BRIGHTEN);
    auto layoutProperty = frameNode->GetLayoutProperty<ButtonLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    EXPECT_TRUE(layoutProperty->HasVibrancyMode());
    EXPECT_EQ(layoutProperty->GetVibrancyModeValue(VibrancyMode::DISABLED), VibrancyMode::BRIGHTEN);
}

/**
 * @tc.name: SetVibrancyMode002
 * @tc.desc: Test ButtonModelNG::SetVibrancyMode stores DARKEN into LayoutProperty.
 * @tc.type: FUNC
 */
HWTEST_F(ButtonVibrancyTestNg, SetVibrancyMode002, TestSize.Level1)
{
    auto frameNode = FrameNode::CreateFrameNode(V2::BUTTON_ETS_TAG, NODE_ID, AceType::MakeRefPtr<ButtonPattern>());
    ASSERT_NE(frameNode, nullptr);
    ButtonModelNG::SetVibrancyMode(AccessibilityManager::RawPtr(frameNode), VibrancyMode::DARKEN);
    auto layoutProperty = frameNode->GetLayoutProperty<ButtonLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    EXPECT_EQ(layoutProperty->GetVibrancyModeValue(VibrancyMode::DISABLED), VibrancyMode::DARKEN);
}

/**
 * @tc.name: SetVibrancyMode003
 * @tc.desc: Test ButtonModelNG::SetVibrancyMode stores AUTO into LayoutProperty.
 * @tc.type: FUNC
 */
HWTEST_F(ButtonVibrancyTestNg, SetVibrancyMode003, TestSize.Level1)
{
    auto frameNode = FrameNode::CreateFrameNode(V2::BUTTON_ETS_TAG, NODE_ID, AceType::MakeRefPtr<ButtonPattern>());
    ASSERT_NE(frameNode, nullptr);
    ButtonModelNG::SetVibrancyMode(AccessibilityManager::RawPtr(frameNode), VibrancyMode::AUTO);
    auto layoutProperty = frameNode->GetLayoutProperty<ButtonLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    EXPECT_EQ(layoutProperty->GetVibrancyModeValue(VibrancyMode::DISABLED), VibrancyMode::AUTO);
}

/**
 * @tc.name: SetVibrancyMode004
 * @tc.desc: Test ButtonModelNG::SetVibrancyMode stores DISABLED into LayoutProperty.
 * @tc.type: FUNC
 */
HWTEST_F(ButtonVibrancyTestNg, SetVibrancyMode004, TestSize.Level1)
{
    auto frameNode = FrameNode::CreateFrameNode(V2::BUTTON_ETS_TAG, NODE_ID, AceType::MakeRefPtr<ButtonPattern>());
    ASSERT_NE(frameNode, nullptr);
    ButtonModelNG::SetVibrancyMode(AccessibilityManager::RawPtr(frameNode), VibrancyMode::DISABLED);
    auto layoutProperty = frameNode->GetLayoutProperty<ButtonLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    EXPECT_EQ(layoutProperty->GetVibrancyModeValue(VibrancyMode::DISABLED), VibrancyMode::DISABLED);
}

/**
 * @tc.name: ResetVibrancyMode001
 * @tc.desc: Test reset vibrancyMode by setting DISABLED reverts to default.
 * @tc.type: FUNC
 */
HWTEST_F(ButtonVibrancyTestNg, ResetVibrancyMode001, TestSize.Level1)
{
    auto frameNode = FrameNode::CreateFrameNode(V2::BUTTON_ETS_TAG, NODE_ID, AceType::MakeRefPtr<ButtonPattern>());
    ASSERT_NE(frameNode, nullptr);
    ButtonModelNG::SetVibrancyMode(AccessibilityManager::RawPtr(frameNode), VibrancyMode::BRIGHTEN);
    ButtonModelNG::SetVibrancyMode(AccessibilityManager::RawPtr(frameNode), VibrancyMode::DISABLED);
    auto layoutProperty = frameNode->GetLayoutProperty<ButtonLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    EXPECT_EQ(layoutProperty->GetVibrancyModeValue(VibrancyMode::DISABLED), VibrancyMode::DISABLED);
}

/**
 * @tc.name: SwitchVibrancyMode001
 * @tc.desc: Test switching BRIGHTEN to DARKEN updates LayoutProperty.
 * @tc.type: FUNC
 */
HWTEST_F(ButtonVibrancyTestNg, SwitchVibrancyMode001, TestSize.Level1)
{
    auto frameNode = FrameNode::CreateFrameNode(V2::BUTTON_ETS_TAG, NODE_ID, AceType::MakeRefPtr<ButtonPattern>());
    ASSERT_NE(frameNode, nullptr);
    ButtonModelNG::SetVibrancyMode(AccessibilityManager::RawPtr(frameNode), VibrancyMode::BRIGHTEN);
    auto layoutProperty = frameNode->GetLayoutProperty<ButtonLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    EXPECT_EQ(layoutProperty->GetVibrancyModeValue(VibrancyMode::DISABLED), VibrancyMode::BRIGHTEN);
    ButtonModelNG::SetVibrancyMode(AccessibilityManager::RawPtr(frameNode), VibrancyMode::DARKEN);
    EXPECT_EQ(layoutProperty->GetVibrancyModeValue(VibrancyMode::DISABLED), VibrancyMode::DARKEN);
}

/**
 * @tc.name: UpdateTextVibrancy001
 * @tc.desc: Test UpdateTextVibrancy applies mode to text child on EXQUISITE device.
 * @tc.type: FUNC
 */
HWTEST_F(ButtonVibrancyTestNg, UpdateTextVibrancy001, TestSize.Level1)
{
    auto frameNode = FrameNode::CreateFrameNode(V2::BUTTON_ETS_TAG, NODE_ID, AceType::MakeRefPtr<ButtonPattern>());
    ASSERT_NE(frameNode, nullptr);
    auto textNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, TEXT_NODE_ID, AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(textNode, nullptr);
    frameNode->AddChild(textNode);
    ButtonModelNG::SetVibrancyMode(AccessibilityManager::RawPtr(frameNode), VibrancyMode::BRIGHTEN);

    auto backupLevel = g_uiMaterialLevel;
    g_uiMaterialLevel = UiMaterialLevel::EXQUISITE;

    auto pattern = frameNode->GetPattern<ButtonPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->UpdateTextVibrancy();

    auto textRenderContext = textNode->GetRenderContext();
    ASSERT_NE(textRenderContext, nullptr);
    EXPECT_TRUE(textRenderContext->HasVibrancyMode());
    EXPECT_EQ(textRenderContext->GetVibrancyModeValue(VibrancyMode::DISABLED), VibrancyMode::BRIGHTEN);

    g_uiMaterialLevel = backupLevel;
}

/**
 * @tc.name: UpdateTextVibrancy002
 * @tc.desc: Test UpdateTextVibrancy skips on non-EXQUISITE device.
 * @tc.type: FUNC
 */
HWTEST_F(ButtonVibrancyTestNg, UpdateTextVibrancy002, TestSize.Level1)
{
    auto frameNode = FrameNode::CreateFrameNode(V2::BUTTON_ETS_TAG, NODE_ID, AceType::MakeRefPtr<ButtonPattern>());
    ASSERT_NE(frameNode, nullptr);
    auto textNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, TEXT_NODE_ID, AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(textNode, nullptr);
    frameNode->AddChild(textNode);
    ButtonModelNG::SetVibrancyMode(AccessibilityManager::RawPtr(frameNode), VibrancyMode::BRIGHTEN);

    auto backupLevel = g_uiMaterialLevel;
    g_uiMaterialLevel = UiMaterialLevel::SMOOTH;

    auto pattern = frameNode->GetPattern<ButtonPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->UpdateTextVibrancy();

    auto textRenderContext = textNode->GetRenderContext();
    ASSERT_NE(textRenderContext, nullptr);
    EXPECT_FALSE(textRenderContext->HasVibrancyMode());

    g_uiMaterialLevel = backupLevel;
}

/**
 * @tc.name: UpdateTextVibrancy003
 * @tc.desc: Test UpdateTextVibrancy applies DARKEN and switch reflects on text node.
 * @tc.type: FUNC
 */
HWTEST_F(ButtonVibrancyTestNg, UpdateTextVibrancy003, TestSize.Level1)
{
    auto frameNode = FrameNode::CreateFrameNode(V2::BUTTON_ETS_TAG, NODE_ID, AceType::MakeRefPtr<ButtonPattern>());
    ASSERT_NE(frameNode, nullptr);
    auto textNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, TEXT_NODE_ID, AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(textNode, nullptr);
    frameNode->AddChild(textNode);

    auto backupLevel = g_uiMaterialLevel;
    g_uiMaterialLevel = UiMaterialLevel::EXQUISITE;

    ButtonModelNG::SetVibrancyMode(AccessibilityManager::RawPtr(frameNode), VibrancyMode::BRIGHTEN);
    auto pattern = frameNode->GetPattern<ButtonPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->UpdateTextVibrancy();
    auto textRenderContext = textNode->GetRenderContext();
    ASSERT_NE(textRenderContext, nullptr);
    EXPECT_EQ(textRenderContext->GetVibrancyModeValue(VibrancyMode::DISABLED), VibrancyMode::BRIGHTEN);

    ButtonModelNG::SetVibrancyMode(AccessibilityManager::RawPtr(frameNode), VibrancyMode::DARKEN);
    pattern->UpdateTextVibrancy();
    EXPECT_EQ(textRenderContext->GetVibrancyModeValue(VibrancyMode::DISABLED), VibrancyMode::DARKEN);

    g_uiMaterialLevel = backupLevel;
}
} // namespace OHOS::Ace::NG

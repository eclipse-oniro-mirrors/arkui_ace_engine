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

#include "divider_base_test_ng.h"

#include "core/components/divider/divider_theme.h"
#include "test/mock/frameworks/core/common/mock_theme_manager.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"

namespace OHOS::Ace::NG {
using namespace testing;
using namespace testing::ext;

class DividerInitialColorTestNG : public DividerBaseTestNG {};

/**
 * @tc.name: InitialColorFromUserSetDividerColor
 * @tc.desc: When DividerColor is explicitly set, the modifier initial color should match the user-set color.
 * @tc.type: FUNC
 */
HWTEST_F(DividerInitialColorTestNG, InitialColorFromUserSetDividerColor, TestSize.Level0)
{
    auto themeManager = AceType::DynamicCast<MockThemeManager>(
        MockPipelineContext::GetCurrent()->GetThemeManager());
    ASSERT_NE(themeManager, nullptr);
    EXPECT_CALL(*themeManager, GetTheme(_, _))
        .WillRepeatedly(Return(AceType::MakeRefPtr<DividerTheme>()));

    Color userColor = Color(0xFF0000);
    auto frameNode = CreateDivider([userColor](DividerModelNG model) {
        model.DividerColor(userColor);
    });
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<DividerPattern>();
    ASSERT_NE(pattern, nullptr);

    pattern->CreateNodePaintMethod();

    auto modifier = pattern->dividerModifier_;
    ASSERT_NE(modifier, nullptr);
    EXPECT_EQ(modifier->color_->Get(), LinearColor(userColor));
}

/**
 * @tc.name: InitialColorFromTheme
 * @tc.desc: When DividerColor is not set, the modifier initial color should fall back to theme color.
 * @tc.type: FUNC
 */
HWTEST_F(DividerInitialColorTestNG, InitialColorFromTheme, TestSize.Level0)
{
    auto themeManager = AceType::DynamicCast<MockThemeManager>(
        MockPipelineContext::GetCurrent()->GetThemeManager());
    ASSERT_NE(themeManager, nullptr);
    EXPECT_CALL(*themeManager, GetTheme(_, _))
        .WillRepeatedly(Return(AceType::MakeRefPtr<DividerTheme>()));

    auto frameNode = CreateDivider([](DividerModelNG model) {});
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<DividerPattern>();
    ASSERT_NE(pattern, nullptr);

    pattern->CreateNodePaintMethod();

    auto modifier = pattern->dividerModifier_;
    ASSERT_NE(modifier, nullptr);

    auto pipeline = frameNode->GetContext();
    ASSERT_NE(pipeline, nullptr);
    auto theme = pipeline->GetTheme<DividerTheme>(frameNode->GetThemeScopeId());
    ASSERT_NE(theme, nullptr);
    EXPECT_EQ(modifier->color_->Get(), LinearColor(theme->GetColor()));
}

/**
 * @tc.name: DividerModifierConstructorWithCustomColor
 * @tc.desc: DividerModifier constructor should accept a custom initial color instead of hardcoding BLUE.
 * @tc.type: FUNC
 */
HWTEST_F(DividerInitialColorTestNG, DividerModifierConstructorWithCustomColor, TestSize.Level0)
{
    LinearColor customColor(Color(0x00FF00));
    auto modifier = AceType::MakeRefPtr<DividerModifier>(customColor);
    ASSERT_NE(modifier, nullptr);
    EXPECT_EQ(modifier->color_->Get(), customColor);
}

/**
 * @tc.name: ModifierCreatedOnce
 * @tc.desc: CreateNodePaintMethod should not recreate the modifier if it already exists.
 * @tc.type: FUNC
 */
HWTEST_F(DividerInitialColorTestNG, ModifierCreatedOnce, TestSize.Level0)
{
    auto frameNode = CreateDivider([](DividerModelNG model) {});
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<DividerPattern>();
    ASSERT_NE(pattern, nullptr);

    pattern->CreateNodePaintMethod();
    auto firstModifier = pattern->dividerModifier_;
    ASSERT_NE(firstModifier, nullptr);

    pattern->CreateNodePaintMethod();
    auto secondModifier = pattern->dividerModifier_;
    ASSERT_NE(secondModifier, nullptr);

    EXPECT_EQ(firstModifier, secondModifier);
}
} // namespace OHOS::Ace::NG

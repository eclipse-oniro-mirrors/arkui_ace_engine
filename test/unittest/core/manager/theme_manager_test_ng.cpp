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

#include <optional>

#include "gtest/gtest.h"
#define private public
#define protected public

#include "test/mock/frameworks/core/common/mock_container.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"

#include "core/components/button/button_theme.h"
#include "core/components/theme/app_theme.h"
#include "core/components/theme/theme_attributes.h"
#include "core/components_ng/pattern/button/button_theme_wrapper.h"
#include "core/components_ng/pattern/counter/counter_theme_wrapper.h"
#include "frameworks/core/components/theme/theme_manager_impl.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS::Ace::NG {

class ThemeManagerTestNg : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
};

void ThemeManagerTestNg::SetUpTestCase()
{
    MockPipelineContext::SetUp();
    MockContainer::SetUp();
}

void ThemeManagerTestNg::TearDownTestCase()
{
    MockPipelineContext::TearDown();
    MockContainer::TearDown();
}

/**
 * @tc.name: ApplyTokenTheme_001
 * @tc.desc: A wrapper returned as Theme applies tokens and retains its component type.
 * @tc.type: FUNC
 */
HWTEST_F(ThemeManagerTestNg, ApplyTokenTheme_001, TestSize.Level1)
{
    RefPtr<Theme> theme = ButtonThemeWrapper::WrapperBuilder().BuildWrapper(nullptr);
    auto buttonTheme = AceType::DynamicCast<ButtonTheme>(theme);
    ASSERT_NE(buttonTheme, nullptr);
    EXPECT_NE(AceType::DynamicCast<ButtonThemeWrapper>(theme), nullptr);

    auto colors = AceType::MakeRefPtr<TokenColors>();
    colors->SetColor(TokenColors::BACKGROUND_EMPHASIZE, Color::RED);
    auto tokenTheme = AceType::MakeRefPtr<TokenTheme>(1);
    tokenTheme->SetColors(colors);
    tokenTheme->SetColorMode(ColorMode::LIGHT);

    theme->ApplyTokenTheme(*tokenTheme);
    EXPECT_EQ(buttonTheme->GetBgColor(), Color::RED);

    colors->SetColor(TokenColors::BACKGROUND_EMPHASIZE, Color::BLUE);
    theme->ApplyTokenTheme(*tokenTheme);
    EXPECT_EQ(buttonTheme->GetBgColor(), Color::BLUE);
}

/**
 * @tc.name: ApplyTokenTheme_002
 * @tc.desc: Counter wrapper has an unambiguous Theme base and shared reference-counted lifetime.
 * @tc.type: FUNC
 */
HWTEST_F(ThemeManagerTestNg, ApplyTokenTheme_002, TestSize.Level1)
{
    auto wrapper = AceType::MakeRefPtr<CounterThemeWrapper>();
    RefPtr<Theme> theme = wrapper;
    auto weak = AceType::WeakClaim(AceType::RawPtr(wrapper));
    wrapper.Reset();
    ASSERT_NE(weak.Upgrade(), nullptr);

    auto colors = AceType::MakeRefPtr<TokenColors>();
    colors->SetColor(TokenColors::FONT_PRIMARY, Color::RED);
    auto tokenTheme = AceType::MakeRefPtr<TokenTheme>(1);
    tokenTheme->SetColors(colors);
    tokenTheme->SetColorMode(ColorMode::LIGHT);
    theme->ApplyTokenTheme(*tokenTheme);

    auto counterTheme = AceType::DynamicCast<CounterTheme>(theme);
    ASSERT_NE(counterTheme, nullptr);
    EXPECT_EQ(counterTheme->GetContentTextStyle().GetTextColor(), Color::RED);
    theme.Reset();
    EXPECT_NE(weak.Upgrade(), nullptr);
    counterTheme.Reset();
    EXPECT_EQ(weak.Upgrade(), nullptr);
}

/**
 * @tc.name: ApplyTokenTheme_003
 * @tc.desc: A regular component theme preserves its values when applying tokens.
 * @tc.type: FUNC
 */
HWTEST_F(ThemeManagerTestNg, ApplyTokenTheme_003, TestSize.Level1)
{
    auto buttonTheme = ButtonTheme::Builder().Build(nullptr);
    ASSERT_NE(buttonTheme, nullptr);
    const auto background = buttonTheme->GetBgColor();
    RefPtr<Theme> theme = buttonTheme;

    auto colors = AceType::MakeRefPtr<TokenColors>();
    const auto tokenBackground = background == Color::RED ? Color::BLUE : Color::RED;
    colors->SetColor(TokenColors::BACKGROUND_EMPHASIZE, tokenBackground);
    auto tokenTheme = AceType::MakeRefPtr<TokenTheme>(1);
    tokenTheme->SetColors(colors);
    tokenTheme->SetColorMode(ColorMode::LIGHT);
    theme->ApplyTokenTheme(*tokenTheme);

    EXPECT_EQ(buttonTheme->GetBgColor(), background);
    EXPECT_EQ(AceType::DynamicCast<ButtonTheme>(theme), buttonTheme);
}

/**
 * @tc.name: ThemeMangerTestNg001
 * @tc.desc: GetThemeOrigin
 * @tc.type: FUNC
 */
HWTEST_F(ThemeManagerTestNg, ThemeManagerTestNg001, TestSize.Level1)
{
    /**
     * @tc.steps: create themeManager
     */
    auto themeManager = AceType::MakeRefPtr<ThemeManagerImpl>();
    auto theme = themeManager->GetThemeOrigin(ButtonTheme::TypeId());
    EXPECT_TRUE(AceType::InstanceOf<ButtonTheme>(theme));
}

/**
 * @tc.name: GetThemesMultiThread001
 * @tc.desc: GetThemesMapKey
 * @tc.type: FUNC
 */
HWTEST_F(ThemeManagerTestNg, GetThemesMultiThread001, TestSize.Level1)
{
    /**
     * @tc.steps: create themeManager
     */
    auto themeManager = AceType::MakeRefPtr<ThemeManagerImpl>();
    auto theme = themeManager->GetThemeMultiThread(ButtonTheme::TypeId());
    EXPECT_TRUE(AceType::InstanceOf<ButtonTheme>(theme));
}

/**
 * @tc.name: GetThemesMultiThread001
 * @tc.desc: GetThemesMapKey
 * @tc.type: FUNC
 */
HWTEST_F(ThemeManagerTestNg, GetThemesMultiThread002, TestSize.Level1)
{
    /**
     * @tc.steps: create themeManager
     */
    auto themeManager = AceType::MakeRefPtr<ThemeManagerImpl>();
    auto theme = themeManager->GetThemeMultiThread(ButtonTheme::TypeId(), 0);
    EXPECT_TRUE(AceType::InstanceOf<ButtonTheme>(theme));
}

/**
 * @tc.name: GetBackgroundColor001
 * @tc.desc: Get app background color from AppTheme, theme style, and fallback value.
 * @tc.type: FUNC
 */
HWTEST_F(ThemeManagerTestNg, GetBackgroundColor001, TestSize.Level1)
{
    auto themeManager = AceType::MakeRefPtr<ThemeManagerImpl>();

    themeManager->themeConstants_->currentThemeStyle_ = nullptr;
    EXPECT_EQ(themeManager->GetBackgroundColor(), Color::WHITE);

    auto themeStyle = AceType::MakeRefPtr<ThemeStyle>();
    themeStyle->SetAttr(THEME_ATTR_BG_COLOR, { .type = ThemeConstantsType::COLOR, .value = Color::RED });
    themeManager->themeConstants_->currentThemeStyle_ = themeStyle;
    EXPECT_EQ(themeManager->GetBackgroundColor(), Color::RED);

    auto appTheme = AceType::MakeRefPtr<AppTheme>();
    appTheme->SetBackgroundColor(Color::BLUE);
    themeManager->themes_[AppTheme::TypeId()] = appTheme;
    EXPECT_EQ(themeManager->GetBackgroundColor(), Color::BLUE);
}
}

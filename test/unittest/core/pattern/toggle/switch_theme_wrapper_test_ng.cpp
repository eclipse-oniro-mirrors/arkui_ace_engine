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

#include "base/memory/ace_type.h"
#include "core/components/theme/theme_constants.h"
#include "core/components_ng/pattern/toggle/switch_theme_wrapper.h"
#include "ui/view/theme/token_colors.h"
#include "ui/view/theme/token_theme.h"

using namespace testing;
using namespace testing::ext;
using OHOS::Ace::ColorMode;
using OHOS::Ace::TokenColors;
using OHOS::Ace::TokenTheme;

namespace OHOS::Ace::NG {

class SwitchThemeWrapperTest : public testing::Test {
public:
    static void SetUpTestSuite() {}
    static void TearDownTestSuite() {}
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @tc.name: SwitchThemeWrapper_ApplyTokenTheme001
 * @tc.desc: ApplyTokenTheme copies brand token colors into point and active
 * fields (focus board/focused bg not overridden).
 * @tc.type: FUNC
 */
HWTEST_F(SwitchThemeWrapperTest, SwitchThemeWrapper_ApplyTokenTheme001, TestSize.Level1)
{
    auto wrapper = AceType::MakeRefPtr<SwitchThemeWrapper>();
    ASSERT_NE(wrapper, nullptr);

    auto tokenColors = AceType::MakeRefPtr<TokenColors>();
    ASSERT_NE(tokenColors, nullptr);
    const Color point = Color::RED;
    const Color active = Color::GREEN;
    tokenColors->SetColor(static_cast<uint32_t>(TokenColors::COMP_BACKGROUND_PRIMARY_CONTRARY), point);
    tokenColors->SetColor(static_cast<uint32_t>(TokenColors::COMP_BACKGROUND_EMPHASIZE), active);

    auto tokenTheme = AceType::MakeRefPtr<TokenTheme>(1);
    ASSERT_NE(tokenTheme, nullptr);
    tokenTheme->SetColors(tokenColors);
    tokenTheme->SetColorMode(ColorMode::LIGHT);

    wrapper->ApplyTokenTheme(*tokenTheme);

    EXPECT_EQ(wrapper->GetPointColor(), point);
    EXPECT_EQ(wrapper->GetActiveColor(), active);
}

/**
 * @tc.name: SwitchThemeWrapper_ApplyTokenTheme002
 * @tc.desc: ApplyTokenTheme does nothing when TokenTheme has no color palette.
 * @tc.type: FUNC
 */
HWTEST_F(SwitchThemeWrapperTest, SwitchThemeWrapper_ApplyTokenTheme002, TestSize.Level1)
{
    auto wrapper = AceType::MakeRefPtr<SwitchThemeWrapper>();
    ASSERT_NE(wrapper, nullptr);

    const Color focusBoardBefore = wrapper->GetFocusBoardColor();
    const Color focusedBgBefore = wrapper->GetFocusedBGColorUnselected();

    auto tokenTheme = AceType::MakeRefPtr<TokenTheme>(2);
    ASSERT_NE(tokenTheme, nullptr);
    tokenTheme->SetColorMode(ColorMode::LIGHT);

    wrapper->ApplyTokenTheme(*tokenTheme);

    EXPECT_EQ(wrapper->GetFocusBoardColor(), focusBoardBefore);
    EXPECT_EQ(wrapper->GetFocusedBGColorUnselected(), focusedBgBefore);
}

/**
 * @tc.name: SwitchThemeWrapper_Registration001
 * @tc.desc: WrapperBuilder builds a SwitchThemeWrapper carrying TokenThemeWrapper interface (INV-3).
 * @tc.type: FUNC
 */
HWTEST_F(SwitchThemeWrapperTest, SwitchThemeWrapper_Registration001, TestSize.Level1)
{
    SwitchThemeWrapper::WrapperBuilder builder;
    auto wrapper = builder.BuildWrapper(nullptr);
    ASSERT_NE(wrapper, nullptr);
    auto switchWrapper = AceType::DynamicCast<SwitchThemeWrapper>(wrapper);
    ASSERT_NE(switchWrapper, nullptr);
    auto switchTheme = AceType::DynamicCast<SwitchTheme>(wrapper);
    ASSERT_NE(switchTheme, nullptr);
}
} // namespace OHOS::Ace::NG

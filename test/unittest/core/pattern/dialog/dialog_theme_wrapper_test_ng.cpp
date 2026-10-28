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
#include "core/components_ng/pattern/dialog/dialog_theme_wrapper.h"
#include "ui/view/theme/token_colors.h"
#include "ui/view/theme/token_theme.h"

using namespace testing;
using namespace testing::ext;
using OHOS::Ace::ColorMode;
using OHOS::Ace::TokenColors;
using OHOS::Ace::TokenTheme;

namespace OHOS::Ace::NG {

class DialogThemeWrapperTest : public testing::Test {
public:
    static void SetUpTestSuite() {}
    static void TearDownTestSuite() {}
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @tc.name: DialogThemeWrapper_ApplyTokenTheme001
 * @tc.desc: ApplyTokenTheme is a no-op for Dialog; highlight/emphasize fields stay unchanged even with a brand palette.
 * @tc.type: FUNC
 */
HWTEST_F(DialogThemeWrapperTest, DialogThemeWrapper_ApplyTokenTheme001, TestSize.Level1)
{
    auto wrapper = AceType::MakeRefPtr<DialogThemeWrapper>();
    ASSERT_NE(wrapper, nullptr);

    const Color highlightBgBefore = wrapper->GetButtonHighlightBgColor();
    const Color highlightFontBefore = wrapper->GetButtonHighlightFontColor();
    const Color defaultFontBefore = wrapper->GetButtonDefaultFontColor();
    const Color emphasizeBgBefore = wrapper->GetEmphasizeButtonBgColor();
    const Color emphasizeTextBefore = wrapper->GetEmphasizeButtonTextColor();

    auto tokenColors = AceType::MakeRefPtr<TokenColors>();
    ASSERT_NE(tokenColors, nullptr);
    tokenColors->SetColor(static_cast<uint32_t>(TokenColors::COMP_BACKGROUND_EMPHASIZE), Color::RED);
    tokenColors->SetColor(static_cast<uint32_t>(TokenColors::BACKGROUND_EMPHASIZE), Color::WHITE);
    tokenColors->SetColor(static_cast<uint32_t>(TokenColors::FONT_ON_PRIMARY), Color::GREEN);
    tokenColors->SetColor(static_cast<uint32_t>(TokenColors::FONT_EMPHASIZE), Color::BLUE);

    auto tokenTheme = AceType::MakeRefPtr<TokenTheme>(1);
    ASSERT_NE(tokenTheme, nullptr);
    tokenTheme->SetColors(tokenColors);
    tokenTheme->SetColorMode(ColorMode::LIGHT);

    wrapper->ApplyTokenTheme(*tokenTheme);

    EXPECT_EQ(wrapper->GetButtonHighlightBgColor(), highlightBgBefore);
    EXPECT_EQ(wrapper->GetButtonHighlightFontColor(), highlightFontBefore);
    EXPECT_EQ(wrapper->GetButtonDefaultFontColor(), defaultFontBefore);
    EXPECT_EQ(wrapper->GetEmphasizeButtonBgColor(), emphasizeBgBefore);
    EXPECT_EQ(wrapper->GetEmphasizeButtonTextColor(), emphasizeTextBefore);
}

/**
 * @tc.name: DialogThemeWrapper_ApplyTokenTheme002
 * @tc.desc: ApplyTokenTheme does nothing when TokenTheme has no color palette.
 * @tc.type: FUNC
 */
HWTEST_F(DialogThemeWrapperTest, DialogThemeWrapper_ApplyTokenTheme002, TestSize.Level1)
{
    auto wrapper = AceType::MakeRefPtr<DialogThemeWrapper>();
    ASSERT_NE(wrapper, nullptr);

    const Color highlightBgBefore = wrapper->GetButtonHighlightBgColor();
    const Color defaultFontBefore = wrapper->GetButtonDefaultFontColor();

    auto tokenTheme = AceType::MakeRefPtr<TokenTheme>(2);
    ASSERT_NE(tokenTheme, nullptr);
    tokenTheme->SetColorMode(ColorMode::LIGHT);

    wrapper->ApplyTokenTheme(*tokenTheme);

    EXPECT_EQ(wrapper->GetButtonHighlightBgColor(), highlightBgBefore);
    EXPECT_EQ(wrapper->GetButtonDefaultFontColor(), defaultFontBefore);
}
} // namespace OHOS::Ace::NG

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
#include "core/components/common/properties/radius.h"
#include "core/components/theme/theme_constants.h"
#include "core/components_ng/pattern/bubble/popup_theme_wrapper.h"
#include "ui/view/theme/token_colors.h"
#include "ui/view/theme/token_theme.h"

using namespace testing;
using namespace testing::ext;
using OHOS::Ace::ColorMode;
using OHOS::Ace::TokenColors;
using OHOS::Ace::TokenTheme;

namespace OHOS::Ace::NG {

class PopupThemeWrapperTest : public testing::Test {
public:
    static void SetUpTestSuite() {}
    static void TearDownTestSuite() {}
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @tc.name: PopupThemeWrapper_ApplyTokenTheme001
 * @tc.desc: ApplyTokenTheme copies brand token colors into button font and primary
 * font fields (focus is not overridden).
 * @tc.type: FUNC
 */
HWTEST_F(PopupThemeWrapperTest, PopupThemeWrapper_ApplyTokenTheme001, TestSize.Level1)
{
    auto wrapper = AceType::MakeRefPtr<PopupThemeWrapper>();
    ASSERT_NE(wrapper, nullptr);

    auto tokenColors = AceType::MakeRefPtr<TokenColors>();
    ASSERT_NE(tokenColors, nullptr);
    const Color buttonFont = Color::RED;
    const Color fontPrimary = Color::GREEN;
    tokenColors->SetColor(static_cast<uint32_t>(TokenColors::FONT_EMPHASIZE), buttonFont);
    tokenColors->SetColor(static_cast<uint32_t>(TokenColors::FONT_PRIMARY), fontPrimary);

    auto tokenTheme = AceType::MakeRefPtr<TokenTheme>(1);
    ASSERT_NE(tokenTheme, nullptr);
    tokenTheme->SetColors(tokenColors);
    tokenTheme->SetColorMode(ColorMode::LIGHT);

    wrapper->ApplyTokenTheme(*tokenTheme);

    EXPECT_EQ(wrapper->GetButtonFontColor(), buttonFont);
    EXPECT_EQ(wrapper->GetFontPrimaryColor(), fontPrimary);
}

/**
 * @tc.name: PopupThemeWrapper_ApplyTokenTheme002
 * @tc.desc: ApplyTokenTheme does nothing when TokenTheme has no color palette.
 * @tc.type: FUNC
 */
HWTEST_F(PopupThemeWrapperTest, PopupThemeWrapper_ApplyTokenTheme002, TestSize.Level1)
{
    auto wrapper = AceType::MakeRefPtr<PopupThemeWrapper>();
    ASSERT_NE(wrapper, nullptr);

    const Color focusBefore = wrapper->GetFocusColor();
    const Color buttonFontBefore = wrapper->GetButtonFontColor();

    auto tokenTheme = AceType::MakeRefPtr<TokenTheme>(2);
    ASSERT_NE(tokenTheme, nullptr);
    tokenTheme->SetColorMode(ColorMode::LIGHT);

    wrapper->ApplyTokenTheme(*tokenTheme);

    EXPECT_EQ(wrapper->GetFocusColor(), focusBefore);
    EXPECT_EQ(wrapper->GetButtonFontColor(), buttonFontBefore);
}

/**
 * @tc.name: PopupThemeWrapper_Registration001
 * @tc.desc: WrapperBuilder builds a PopupThemeWrapper carrying TokenThemeWrapper interface (INV-3).
 * @tc.type: FUNC
 */
HWTEST_F(PopupThemeWrapperTest, PopupThemeWrapper_Registration001, TestSize.Level1)
{
    PopupThemeWrapper::WrapperBuilder builder;
    auto wrapper = builder.BuildWrapper(nullptr);
    ASSERT_NE(wrapper, nullptr);
    auto popupWrapper = AceType::DynamicCast<PopupThemeWrapper>(wrapper);
    ASSERT_NE(popupWrapper, nullptr);
    auto popupTheme = AceType::DynamicCast<PopupTheme>(wrapper);
    ASSERT_NE(popupTheme, nullptr);
}
} // namespace OHOS::Ace::NG

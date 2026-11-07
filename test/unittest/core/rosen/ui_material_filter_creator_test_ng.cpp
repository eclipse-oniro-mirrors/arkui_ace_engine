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

#define private public
#define protected public

#include "core/components_ng/render/ui_material_filter_creator.h"
#include "core/components/common/properties/ui_material.h"

#include "render_service_client/core/ui_effect/property/include/rs_ui_filter_base.h"
#include "render_service_client/core/ui_effect/property/include/rs_ui_shader_base.h"

#undef private
#undef protected

#include "ui/base/utils/utils.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {
const Color COLOR_WITH_ALPHA = Color::FromARGB(128, 255, 0, 0);
const Color COLOR_ZERO_ALPHA = Color::FromARGB(0, 255, 0, 0);
const Color COLOR_FULL_ALPHA = Color::FromARGB(255, 255, 0, 0);
constexpr float DEFAULT_DIP_SCALE = 1.0f;
constexpr float EXPECTED_EMBOSS_RESET_X = 0.0f;
constexpr float EXPECTED_EMBOSS_RESET_Y = 0.0f;

constexpr float TINTED_GLASS_BG_RATES_A_0 = 0.0f;
constexpr float TINTED_GLASS_BG_RATES_A_1 = 0.0f;
constexpr float TINTED_GLASS_BG_KBS_A_0 = 1.0f;
constexpr float TINTED_GLASS_BG_KBS_A_1 = 0.0f;
constexpr float TINTED_GLASS_BG_KBS_A_2 = 1.0f;
constexpr float TINTED_GLASS_BG_POS_A_0 = 1.0f;
constexpr float TINTED_GLASS_BG_POS_A_1 = 1.0f;
constexpr float TINTED_GLASS_BG_POS_A_2 = 1.0f;
constexpr float TINTED_GLASS_BG_NEG_A_0 = 1.0f;
constexpr float TINTED_GLASS_BG_NEG_A_1 = 1.0f;
constexpr float TINTED_GLASS_BG_NEG_A_2 = 1.0f;
constexpr float TINTED_GLASS_LUMA_0 = 0.6628f;
constexpr float TINTED_GLASS_LUMA_1 = 0.3137f;
constexpr float TINTED_GLASS_LUMA_2 = 0.1538f;
constexpr float TINTED_GLASS_MATERIAL_COLOR_FRACTION = 0.0f;
constexpr int TINTED_GLASS_COLOR_BLEND_MODE = 1;
} // namespace

class UiMaterialFilterCreatorTestNg : public testing::Test {
public:
    void SetUp() override;
    void TearDown() override;
};

void UiMaterialFilterCreatorTestNg::SetUp() {}
void UiMaterialFilterCreatorTestNg::TearDown() {}

ImmersiveMaterialConfig CreateGentleMaterialConfig(const Color& materialColor)
{
    ImmersiveMaterialConfig config;
    config.key.level = UiMaterialLevel::GENTLE;
    config.key.style = UiMaterialStyle::REGULAR;
    config.key.transparency = UiMaterialTransparency::GENTLE_THIN;
    config.key.colorMode = ColorMode::LIGHT;
    config.materialColor = materialColor;
    config.dipScale = DEFAULT_DIP_SCALE;
    return config;
}

ImmersiveMaterialConfig CreateECSubMaterialConfig(const Color& materialColor)
{
    ImmersiveMaterialConfig config;
    config.key.level = UiMaterialLevel::GENTLE;
    config.key.style = UiMaterialStyle::REGULAR_EC_SUB;
    config.key.transparency = UiMaterialTransparency::GENTLE_THIN;
    config.key.colorMode = ColorMode::LIGHT;
    config.materialColor = materialColor;
    config.dipScale = DEFAULT_DIP_SCALE;
    return config;
}

ImmersiveMaterialConfig CreateTintedGlassConfig(const Color& materialColor)
{
    auto config = CreateGentleMaterialConfig(materialColor);
    config.colorBlendMode = MaterialColorBlendMode::TINTED_GLASS;
    return config;
}

ImmersiveMaterialConfig CreateTintedGlassECSubConfig(const Color& materialColor)
{
    auto config = CreateECSubMaterialConfig(materialColor);
    config.colorBlendMode = MaterialColorBlendMode::TINTED_GLASS;
    return config;
}

ImmersiveMaterialConfig CreateTintedGlassOverlayConfig(const Color& materialColor)
{
    auto config = CreateECSubMaterialConfig(materialColor);
    config.colorBlendMode = MaterialColorBlendMode::TINTED_GLASS;
    config.needSplitOverlayShader = true;
    return config;
}

/**
 * @tc.name: ConvertToUiMaterialFilter_AlphaGreaterThanZero
 * @tc.desc: When materialColor has alpha > 0, WeightsEmboss and DarkModeWeightsEmboss should be reset to {0, 0}
 * @tc.type: FUNC
 */
HWTEST_F(UiMaterialFilterCreatorTestNg, ConvertToUiMaterialFilter_AlphaGreaterThanZero, TestSize.Level1)
{
    auto config = CreateGentleMaterialConfig(COLOR_WITH_ALPHA);
    auto filter = UiMaterialFilterCreator::ConvertToUiMaterialFilter(config);
    ASSERT_NE(filter, nullptr);
    auto glassFilter = std::static_pointer_cast<Rosen::RSNGFrostedGlassFilter>(filter);
    ASSERT_NE(glassFilter, nullptr);
    auto weightsEmboss = glassFilter->Getter<Rosen::FrostedGlassWeightsEmbossTag>()->Get();
    EXPECT_TRUE(NearEqual(weightsEmboss[0], EXPECTED_EMBOSS_RESET_X));
    EXPECT_TRUE(NearEqual(weightsEmboss[1], EXPECTED_EMBOSS_RESET_Y));
    auto darkModeWeightsEmboss = glassFilter->Getter<Rosen::FrostedGlassDarkModeWeightsEmbossTag>()->Get();
    EXPECT_TRUE(NearEqual(darkModeWeightsEmboss[0], EXPECTED_EMBOSS_RESET_X));
    EXPECT_TRUE(NearEqual(darkModeWeightsEmboss[1], EXPECTED_EMBOSS_RESET_Y));
}

/**
 * @tc.name: ConvertToUiMaterialFilter_AlphaEqualsZero
 * @tc.desc: When materialColor has alpha == 0, WeightsEmboss setter should not be invoked; filter retains original
 *           values
 * @tc.type: FUNC
 */
HWTEST_F(UiMaterialFilterCreatorTestNg, ConvertToUiMaterialFilter_AlphaEqualsZero, TestSize.Level1)
{
    auto config = CreateGentleMaterialConfig(COLOR_ZERO_ALPHA);
    auto filter = UiMaterialFilterCreator::ConvertToUiMaterialFilter(config);
    ASSERT_NE(filter, nullptr);
    auto glassFilter = std::static_pointer_cast<Rosen::RSNGFrostedGlassFilter>(filter);
    ASSERT_NE(glassFilter, nullptr);
    auto weightsEmboss = glassFilter->Getter<Rosen::FrostedGlassWeightsEmbossTag>()->Get();
    auto darkModeWeightsEmboss = glassFilter->Getter<Rosen::FrostedGlassDarkModeWeightsEmbossTag>()->Get();
    EXPECT_TRUE(NearEqual(weightsEmboss[0], EXPECTED_EMBOSS_RESET_X));
    EXPECT_TRUE(NearEqual(weightsEmboss[1], EXPECTED_EMBOSS_RESET_Y));
    EXPECT_TRUE(NearEqual(darkModeWeightsEmboss[0], EXPECTED_EMBOSS_RESET_X));
    EXPECT_TRUE(NearEqual(darkModeWeightsEmboss[1], EXPECTED_EMBOSS_RESET_Y));
    auto materialColorRs = glassFilter->Getter<Rosen::FrostedGlassMaterialColorTag>()->Get();
    EXPECT_TRUE(NearEqual(materialColorRs[3], 0.0f / 255.0f));
}

/**
 * @tc.name: ConvertToUiMaterialFilter_MaterialColorApplied
 * @tc.desc: MaterialColor should be correctly converted and applied to FrostedGlassMaterialColorTag
 * @tc.type: FUNC
 */
HWTEST_F(UiMaterialFilterCreatorTestNg, ConvertToUiMaterialFilter_MaterialColorApplied, TestSize.Level1)
{
    auto config = CreateGentleMaterialConfig(COLOR_WITH_ALPHA);
    auto filter = UiMaterialFilterCreator::ConvertToUiMaterialFilter(config);
    ASSERT_NE(filter, nullptr);
    auto glassFilter = std::static_pointer_cast<Rosen::RSNGFrostedGlassFilter>(filter);
    ASSERT_NE(glassFilter, nullptr);
    auto materialColorRs = glassFilter->Getter<Rosen::FrostedGlassMaterialColorTag>()->Get();
    float expectColorRed = COLOR_WITH_ALPHA.GetRed() / 255.0f;
    float expectColorGreen = COLOR_WITH_ALPHA.GetGreen() / 255.0f;
    float expectColorBlue = COLOR_WITH_ALPHA.GetBlue() / 255.0f;
    float expectColorAlpha = COLOR_WITH_ALPHA.GetAlpha() / 255.0f;
    EXPECT_TRUE(NearEqual(materialColorRs[0], expectColorRed));
    EXPECT_TRUE(NearEqual(materialColorRs[1], expectColorGreen));
    EXPECT_TRUE(NearEqual(materialColorRs[2], expectColorBlue));
    EXPECT_TRUE(NearEqual(materialColorRs[3], expectColorAlpha));
}

/**
 * @tc.name: ConvertToUiMaterialFilter_InvalidKeyReturnsNull
 * @tc.desc: When key is not found in MATERIAL_PARAM_MAP, ConvertToUiMaterialFilter should return nullptr
 * @tc.type: FUNC
 */
HWTEST_F(UiMaterialFilterCreatorTestNg, ConvertToUiMaterialFilter_InvalidKeyReturnsNull, TestSize.Level1)
{
    ImmersiveMaterialConfig config;
    config.key.level = UiMaterialLevel::GENTLE;
    config.key.style = UiMaterialStyle::ULTRA_THICK;
    config.key.transparency = UiMaterialTransparency::NONE;
    config.key.colorMode = ColorMode::LIGHT;
    config.materialColor = COLOR_WITH_ALPHA;
    config.dipScale = DEFAULT_DIP_SCALE;
    auto filter = UiMaterialFilterCreator::ConvertToUiMaterialFilter(config);
    EXPECT_EQ(filter, nullptr);
}

/**
 * @tc.name: ConvertToUiMaterialECSubShader_AlphaGreaterThanZero
 * @tc.desc: When materialColor alpha > 0 in ECSubShader, EffectWeightsEmboss should be reset to {0, 0}
 * @tc.type: FUNC
 */
HWTEST_F(UiMaterialFilterCreatorTestNg, ConvertToUiMaterialECSubShader_AlphaGreaterThanZero, TestSize.Level1)
{
    auto config = CreateECSubMaterialConfig(COLOR_WITH_ALPHA);
    auto shader = UiMaterialFilterCreator::ConvertToUiMaterialECSubShader(config);
    ASSERT_NE(shader, nullptr);
    auto glassEffect = std::static_pointer_cast<Rosen::RSNGFrostedGlassEffect>(shader);
    ASSERT_NE(glassEffect, nullptr);
    auto weightsEmboss = glassEffect->Getter<Rosen::FrostedGlassEffectWeightsEmbossTag>()->Get();
    EXPECT_TRUE(NearEqual(weightsEmboss[0], EXPECTED_EMBOSS_RESET_X));
    EXPECT_TRUE(NearEqual(weightsEmboss[1], EXPECTED_EMBOSS_RESET_Y));
}

/**
 * @tc.name: ConvertToUiMaterialECSubShader_AlphaEqualsZero
 * @tc.desc: When materialColor alpha == 0 in ECSubShader, EffectWeightsEmboss setter should not be invoked
 * @tc.type: FUNC
 */
HWTEST_F(UiMaterialFilterCreatorTestNg, ConvertToUiMaterialECSubShader_AlphaEqualsZero, TestSize.Level1)
{
    auto config = CreateECSubMaterialConfig(COLOR_ZERO_ALPHA);
    auto shader = UiMaterialFilterCreator::ConvertToUiMaterialECSubShader(config);
    ASSERT_NE(shader, nullptr);
    auto glassEffect = std::static_pointer_cast<Rosen::RSNGFrostedGlassEffect>(shader);
    ASSERT_NE(glassEffect, nullptr);
    auto materialColorRs = glassEffect->Getter<Rosen::FrostedGlassEffectMaterialColorTag>()->Get();
    EXPECT_TRUE(NearEqual(materialColorRs[3], 0.0f / 255.0f));
}

/**
 * @tc.name: TintedGlass_AlphaFull_BgEqualsA
 * @tc.desc: TINTED_GLASS with fully opaque color (alpha=1.0), bg params should equal fixed A constants
 * @tc.type: FUNC
 */
HWTEST_F(UiMaterialFilterCreatorTestNg, TintedGlass_AlphaFull_BgEqualsA, TestSize.Level1)
{
    auto config = CreateTintedGlassConfig(COLOR_FULL_ALPHA);
    auto filter = UiMaterialFilterCreator::ConvertToUiMaterialFilter(config);
    ASSERT_NE(filter, nullptr);
    auto glassFilter = std::static_pointer_cast<Rosen::RSNGFrostedGlassFilter>(filter);
    ASSERT_NE(glassFilter, nullptr);

    auto bgRates = glassFilter->Getter<Rosen::FrostedGlassBgRatesTag>()->Get();
    EXPECT_TRUE(NearEqual(bgRates[0], TINTED_GLASS_BG_RATES_A_0));
    EXPECT_TRUE(NearEqual(bgRates[1], TINTED_GLASS_BG_RATES_A_1));
    auto bgKBS = glassFilter->Getter<Rosen::FrostedGlassBgKBSTag>()->Get();
    EXPECT_TRUE(NearEqual(bgKBS[0], TINTED_GLASS_BG_KBS_A_0));
    EXPECT_TRUE(NearEqual(bgKBS[1], TINTED_GLASS_BG_KBS_A_1));
    EXPECT_TRUE(NearEqual(bgKBS[2], TINTED_GLASS_BG_KBS_A_2));
    auto bgPos = glassFilter->Getter<Rosen::FrostedGlassBgPosTag>()->Get();
    EXPECT_TRUE(NearEqual(bgPos[0], TINTED_GLASS_BG_POS_A_0));
    EXPECT_TRUE(NearEqual(bgPos[1], TINTED_GLASS_BG_POS_A_1));
    EXPECT_TRUE(NearEqual(bgPos[2], TINTED_GLASS_BG_POS_A_2));
    auto bgNeg = glassFilter->Getter<Rosen::FrostedGlassBgNegTag>()->Get();
    EXPECT_TRUE(NearEqual(bgNeg[0], TINTED_GLASS_BG_NEG_A_0));
    EXPECT_TRUE(NearEqual(bgNeg[1], TINTED_GLASS_BG_NEG_A_1));
    EXPECT_TRUE(NearEqual(bgNeg[2], TINTED_GLASS_BG_NEG_A_2));

    auto vibrancyStrength = glassFilter->Getter<Rosen::FrostedGlassVibrancyStrengthTag>()->Get();
    EXPECT_TRUE(NearEqual(vibrancyStrength, 0.0f));
    auto colorBlendMode = glassFilter->Getter<Rosen::FrostedGlassColorBlendModeTag>()->Get();
    EXPECT_EQ(colorBlendMode, TINTED_GLASS_COLOR_BLEND_MODE);
    auto lumaParams = glassFilter->Getter<Rosen::FrostedGlassLumaParamsTag>()->Get();
    EXPECT_TRUE(NearEqual(lumaParams[0], TINTED_GLASS_LUMA_0));
    EXPECT_TRUE(NearEqual(lumaParams[1], TINTED_GLASS_LUMA_1));
    EXPECT_TRUE(NearEqual(lumaParams[2], TINTED_GLASS_LUMA_2));
    auto materialColorFraction = glassFilter->Getter<Rosen::FrostedGlassMaterialColorFractionTag>()->Get();
    EXPECT_TRUE(NearEqual(materialColorFraction, TINTED_GLASS_MATERIAL_COLOR_FRACTION));
}

/**
 * @tc.name: TintedGlass_AlphaFull_DarkModeBgEqualsA
 * @tc.desc: TINTED_GLASS with alpha=1.0, darkMode bg params should also equal fixed A constants
 * @tc.type: FUNC
 */
HWTEST_F(UiMaterialFilterCreatorTestNg, TintedGlass_AlphaFull_DarkModeBgEqualsA, TestSize.Level1)
{
    auto config = CreateTintedGlassConfig(COLOR_FULL_ALPHA);
    auto filter = UiMaterialFilterCreator::ConvertToUiMaterialFilter(config);
    ASSERT_NE(filter, nullptr);
    auto glassFilter = std::static_pointer_cast<Rosen::RSNGFrostedGlassFilter>(filter);
    ASSERT_NE(glassFilter, nullptr);

    auto darkBgRates = glassFilter->Getter<Rosen::FrostedGlassDarkModeBgRatesTag>()->Get();
    EXPECT_TRUE(NearEqual(darkBgRates[0], TINTED_GLASS_BG_RATES_A_0));
    EXPECT_TRUE(NearEqual(darkBgRates[1], TINTED_GLASS_BG_RATES_A_1));
    auto darkBgKBS = glassFilter->Getter<Rosen::FrostedGlassDarkModeBgKBSTag>()->Get();
    EXPECT_TRUE(NearEqual(darkBgKBS[0], TINTED_GLASS_BG_KBS_A_0));
    EXPECT_TRUE(NearEqual(darkBgKBS[1], TINTED_GLASS_BG_KBS_A_1));
    EXPECT_TRUE(NearEqual(darkBgKBS[2], TINTED_GLASS_BG_KBS_A_2));
    auto darkBgPos = glassFilter->Getter<Rosen::FrostedGlassDarkModeBgPosTag>()->Get();
    EXPECT_TRUE(NearEqual(darkBgPos[0], TINTED_GLASS_BG_POS_A_0));
    EXPECT_TRUE(NearEqual(darkBgPos[1], TINTED_GLASS_BG_POS_A_1));
    EXPECT_TRUE(NearEqual(darkBgPos[2], TINTED_GLASS_BG_POS_A_2));
    auto darkBgNeg = glassFilter->Getter<Rosen::FrostedGlassDarkModeBgNegTag>()->Get();
    EXPECT_TRUE(NearEqual(darkBgNeg[0], TINTED_GLASS_BG_NEG_A_0));
    EXPECT_TRUE(NearEqual(darkBgNeg[1], TINTED_GLASS_BG_NEG_A_1));
    EXPECT_TRUE(NearEqual(darkBgNeg[2], TINTED_GLASS_BG_NEG_A_2));
}

/**
 * @tc.name: TintedGlass_AlphaZero_ModeStillActive
 * @tc.desc: TINTED_GLASS with alpha=0, bg params equal B (same as LINEAR_MIX), but ColorBlendMode/VibrancyStrength
 * still set
 * @tc.type: FUNC
 */
HWTEST_F(UiMaterialFilterCreatorTestNg, TintedGlass_AlphaZero_ModeStillActive, TestSize.Level1)
{
    auto linearConfig = CreateGentleMaterialConfig(COLOR_ZERO_ALPHA);
    auto linearFilter = UiMaterialFilterCreator::ConvertToUiMaterialFilter(linearConfig);
    ASSERT_NE(linearFilter, nullptr);
    auto linearGlass = std::static_pointer_cast<Rosen::RSNGFrostedGlassFilter>(linearFilter);
    ASSERT_NE(linearGlass, nullptr);
    auto linearBgRates = linearGlass->Getter<Rosen::FrostedGlassBgRatesTag>()->Get();
    auto linearBgKBS = linearGlass->Getter<Rosen::FrostedGlassBgKBSTag>()->Get();

    auto tintedConfig = CreateTintedGlassConfig(COLOR_ZERO_ALPHA);
    auto tintedFilter = UiMaterialFilterCreator::ConvertToUiMaterialFilter(tintedConfig);
    ASSERT_NE(tintedFilter, nullptr);
    auto tintedGlass = std::static_pointer_cast<Rosen::RSNGFrostedGlassFilter>(tintedFilter);
    ASSERT_NE(tintedGlass, nullptr);
    auto tintedBgRates = tintedGlass->Getter<Rosen::FrostedGlassBgRatesTag>()->Get();
    auto tintedBgKBS = tintedGlass->Getter<Rosen::FrostedGlassBgKBSTag>()->Get();

    EXPECT_TRUE(NearEqual(tintedBgRates[0], linearBgRates[0]));
    EXPECT_TRUE(NearEqual(tintedBgRates[1], linearBgRates[1]));
    EXPECT_TRUE(NearEqual(tintedBgKBS[0], linearBgKBS[0]));
    EXPECT_TRUE(NearEqual(tintedBgKBS[1], linearBgKBS[1]));
    EXPECT_TRUE(NearEqual(tintedBgKBS[2], linearBgKBS[2]));

    auto colorBlendMode = tintedGlass->Getter<Rosen::FrostedGlassColorBlendModeTag>()->Get();
    EXPECT_EQ(colorBlendMode, TINTED_GLASS_COLOR_BLEND_MODE);
    auto vibrancyStrength = tintedGlass->Getter<Rosen::FrostedGlassVibrancyStrengthTag>()->Get();
    EXPECT_TRUE(NearEqual(vibrancyStrength, 1.0f));
}

/**
 * @tc.name: TintedGlass_AlphaHalf_BgInterpolated
 * @tc.desc: TINTED_GLASS with alpha=128/255, bg params should be A*alpha + B*(1-alpha)
 * @tc.type: FUNC
 */
HWTEST_F(UiMaterialFilterCreatorTestNg, TintedGlass_AlphaHalf_BgInterpolated, TestSize.Level1)
{
    float alpha = COLOR_WITH_ALPHA.GetAlpha() / 255.0f;

    auto linearConfig = CreateGentleMaterialConfig(COLOR_WITH_ALPHA);
    auto linearFilter = UiMaterialFilterCreator::ConvertToUiMaterialFilter(linearConfig);
    ASSERT_NE(linearFilter, nullptr);
    auto linearGlass = std::static_pointer_cast<Rosen::RSNGFrostedGlassFilter>(linearFilter);
    ASSERT_NE(linearGlass, nullptr);
    auto linearBgRates = linearGlass->Getter<Rosen::FrostedGlassBgRatesTag>()->Get();
    auto linearBgKBS = linearGlass->Getter<Rosen::FrostedGlassBgKBSTag>()->Get();

    auto tintedConfig = CreateTintedGlassConfig(COLOR_WITH_ALPHA);
    auto tintedFilter = UiMaterialFilterCreator::ConvertToUiMaterialFilter(tintedConfig);
    ASSERT_NE(tintedFilter, nullptr);
    auto tintedGlass = std::static_pointer_cast<Rosen::RSNGFrostedGlassFilter>(tintedFilter);
    ASSERT_NE(tintedGlass, nullptr);

    auto tintedBgRates = tintedGlass->Getter<Rosen::FrostedGlassBgRatesTag>()->Get();
    float expectRates0 = TINTED_GLASS_BG_RATES_A_0 * alpha + linearBgRates[0] * (1.0f - alpha);
    float expectRates1 = TINTED_GLASS_BG_RATES_A_1 * alpha + linearBgRates[1] * (1.0f - alpha);
    EXPECT_TRUE(NearEqual(tintedBgRates[0], expectRates0));
    EXPECT_TRUE(NearEqual(tintedBgRates[1], expectRates1));

    auto tintedBgKBS = tintedGlass->Getter<Rosen::FrostedGlassBgKBSTag>()->Get();
    float expectKBS0 = TINTED_GLASS_BG_KBS_A_0 * alpha + linearBgKBS[0] * (1.0f - alpha);
    float expectKBS1 = TINTED_GLASS_BG_KBS_A_1 * alpha + linearBgKBS[1] * (1.0f - alpha);
    float expectKBS2 = TINTED_GLASS_BG_KBS_A_2 * alpha + linearBgKBS[2] * (1.0f - alpha);
    EXPECT_TRUE(NearEqual(tintedBgKBS[0], expectKBS0));
    EXPECT_TRUE(NearEqual(tintedBgKBS[1], expectKBS1));
    EXPECT_TRUE(NearEqual(tintedBgKBS[2], expectKBS2));

    auto vibrancyStrength = tintedGlass->Getter<Rosen::FrostedGlassVibrancyStrengthTag>()->Get();
    EXPECT_TRUE(NearEqual(vibrancyStrength, 1.0f - alpha));
}

/**
 * @tc.name: ECSub_TintedGlass_AlphaFull_BgEqualsA
 * @tc.desc: ECSub TINTED_GLASS with alpha=1.0, Effect bg params should equal fixed A constants
 * @tc.type: FUNC
 */
HWTEST_F(UiMaterialFilterCreatorTestNg, ECSub_TintedGlass_AlphaFull_BgEqualsA, TestSize.Level1)
{
    auto config = CreateTintedGlassECSubConfig(COLOR_FULL_ALPHA);
    auto shader = UiMaterialFilterCreator::ConvertToUiMaterialECSubShader(config);
    ASSERT_NE(shader, nullptr);
    auto glassEffect = std::static_pointer_cast<Rosen::RSNGFrostedGlassEffect>(shader);
    ASSERT_NE(glassEffect, nullptr);

    auto bgRates = glassEffect->Getter<Rosen::FrostedGlassEffectBgRatesTag>()->Get();
    EXPECT_TRUE(NearEqual(bgRates[0], TINTED_GLASS_BG_RATES_A_0));
    EXPECT_TRUE(NearEqual(bgRates[1], TINTED_GLASS_BG_RATES_A_1));
    auto bgKBS = glassEffect->Getter<Rosen::FrostedGlassEffectBgKBSTag>()->Get();
    EXPECT_TRUE(NearEqual(bgKBS[0], TINTED_GLASS_BG_KBS_A_0));
    EXPECT_TRUE(NearEqual(bgKBS[1], TINTED_GLASS_BG_KBS_A_1));
    EXPECT_TRUE(NearEqual(bgKBS[2], TINTED_GLASS_BG_KBS_A_2));

    auto vibrancyStrength = glassEffect->Getter<Rosen::FrostedGlassEffectVibrancyStrengthTag>()->Get();
    EXPECT_TRUE(NearEqual(vibrancyStrength, 0.0f));
    auto colorBlendMode = glassEffect->Getter<Rosen::FrostedGlassEffectColorBlendModeTag>()->Get();
    EXPECT_EQ(colorBlendMode, TINTED_GLASS_COLOR_BLEND_MODE);
}

/**
 * @tc.name: ECSub_TintedGlass_AlphaHalf_BgInterpolated
 * @tc.desc: ECSub TINTED_GLASS with alpha=128/255, Effect bg params should be interpolated
 * @tc.type: FUNC
 */
HWTEST_F(UiMaterialFilterCreatorTestNg, ECSub_TintedGlass_AlphaHalf_BgInterpolated, TestSize.Level1)
{
    float alpha = COLOR_WITH_ALPHA.GetAlpha() / 255.0f;

    auto linearConfig = CreateECSubMaterialConfig(COLOR_WITH_ALPHA);
    auto linearShader = UiMaterialFilterCreator::ConvertToUiMaterialECSubShader(linearConfig);
    ASSERT_NE(linearShader, nullptr);
    auto linearEffect = std::static_pointer_cast<Rosen::RSNGFrostedGlassEffect>(linearShader);
    ASSERT_NE(linearEffect, nullptr);
    auto linearBgRates = linearEffect->Getter<Rosen::FrostedGlassEffectBgRatesTag>()->Get();

    auto tintedConfig = CreateTintedGlassECSubConfig(COLOR_WITH_ALPHA);
    auto tintedShader = UiMaterialFilterCreator::ConvertToUiMaterialECSubShader(tintedConfig);
    ASSERT_NE(tintedShader, nullptr);
    auto tintedEffect = std::static_pointer_cast<Rosen::RSNGFrostedGlassEffect>(tintedShader);
    ASSERT_NE(tintedEffect, nullptr);

    auto tintedBgRates = tintedEffect->Getter<Rosen::FrostedGlassEffectBgRatesTag>()->Get();
    float expectRates0 = TINTED_GLASS_BG_RATES_A_0 * alpha + linearBgRates[0] * (1.0f - alpha);
    float expectRates1 = TINTED_GLASS_BG_RATES_A_1 * alpha + linearBgRates[1] * (1.0f - alpha);
    EXPECT_TRUE(NearEqual(tintedBgRates[0], expectRates0));
    EXPECT_TRUE(NearEqual(tintedBgRates[1], expectRates1));

    auto vibrancyStrength = tintedEffect->Getter<Rosen::FrostedGlassEffectVibrancyStrengthTag>()->Get();
    EXPECT_TRUE(NearEqual(vibrancyStrength, 1.0f - alpha));
}

/**
 * @tc.name: Overlay_TintedGlass_AlphaFull_BgEqualsA
 * @tc.desc: Overlay TINTED_GLASS with alpha=1.0, Effect bg params should equal fixed A constants
 * @tc.type: FUNC
 */
HWTEST_F(UiMaterialFilterCreatorTestNg, Overlay_TintedGlass_AlphaFull_BgEqualsA, TestSize.Level1)
{
    auto config = CreateTintedGlassOverlayConfig(COLOR_FULL_ALPHA);
    auto overlay = UiMaterialFilterCreator::ConvertToUiMaterialECSubShaderOverlay(config);
    ASSERT_NE(overlay, nullptr);
    auto glassEffect = std::static_pointer_cast<Rosen::RSNGFrostedGlassEffect>(overlay);
    ASSERT_NE(glassEffect, nullptr);

    auto bgRates = glassEffect->Getter<Rosen::FrostedGlassEffectBgRatesTag>()->Get();
    EXPECT_TRUE(NearEqual(bgRates[0], TINTED_GLASS_BG_RATES_A_0));
    EXPECT_TRUE(NearEqual(bgRates[1], TINTED_GLASS_BG_RATES_A_1));
    auto bgKBS = glassEffect->Getter<Rosen::FrostedGlassEffectBgKBSTag>()->Get();
    EXPECT_TRUE(NearEqual(bgKBS[0], TINTED_GLASS_BG_KBS_A_0));
    EXPECT_TRUE(NearEqual(bgKBS[1], TINTED_GLASS_BG_KBS_A_1));
    EXPECT_TRUE(NearEqual(bgKBS[2], TINTED_GLASS_BG_KBS_A_2));

    auto vibrancyStrength = glassEffect->Getter<Rosen::FrostedGlassEffectVibrancyStrengthTag>()->Get();
    EXPECT_TRUE(NearEqual(vibrancyStrength, 0.0f));
    auto colorBlendMode = glassEffect->Getter<Rosen::FrostedGlassEffectColorBlendModeTag>()->Get();
    EXPECT_EQ(colorBlendMode, TINTED_GLASS_COLOR_BLEND_MODE);
}
} // namespace OHOS::Ace::NG

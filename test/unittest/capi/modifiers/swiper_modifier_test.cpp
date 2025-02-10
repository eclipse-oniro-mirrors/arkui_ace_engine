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

#include "swiper_modifier_test.h"

#include "modifier_test_base.h"
#include "modifiers_test_utils.h"

#include "core/interfaces/native/utility/converter.h"
#include "core/interfaces/native/utility/reverse_converter.h"

#include "core/components/common/layout/constants.h"
#include "core/components/declaration/swiper/swiper_declaration.h"
#include "core/components/swiper/swiper_indicator_theme.h"
#include "core/components_ng/pattern/swiper/swiper_pattern.h"

namespace OHOS::Ace::NG {

using namespace testing;
using namespace testing::ext;
using namespace Converter;

namespace {
static const Ark_Boolean ABOOL_TRUE(true);
static const Ark_Boolean ABOOL_FALSE(false);
static const Ark_Int32 AINT32_POS(1234);
static const Ark_Int32 AINT32_NEG(INT_MIN);
static const Ark_Float32 AFLT32_POS(1.234f);
static const Ark_Float32 AFLT32_NEG(-5.6789f);

static const auto ANUM_INT_POS = ArkValue<Ark_Number>(AINT32_POS);
static const auto ANUM_INT_NEG = ArkValue<Ark_Number>(AINT32_NEG);
static const auto ASTR_INVALID = ArkValue<Ark_String>("(*%^&#@");

static const auto ALEN_VP_POS = ArkValue<Ark_Length>(AFLT32_POS);
static const auto ALEN_VP_NEG = ArkValue<Ark_Length>(AFLT32_NEG);

static const auto OPT_LEN_VP_POS = ArkValue<Opt_Length>(AFLT32_POS);
static const auto OPT_LEN_VP_NEG = ArkValue<Opt_Length>(AFLT32_NEG);
static const auto OPT_LEN_UNDEF = ArkValue<Opt_Length>();

static const auto OPT_BOOL_UNDEF = ArkValue<Opt_Boolean>(Ark_Empty());
static const auto OPT_BOOL_TRUE = ArkValue<Opt_Boolean>(true);
static const auto OPT_BOOL_FALSE = ArkValue<Opt_Boolean>(false);

static const auto OPT_COLOR_UNDEF = ArkValue<Opt_ResourceColor>();

static const std::string EXPECTED_TRUE("true");
static const std::string EXPECTED_FALSE("false");
static const std::string EXPECTED_VP_ZERO(Dimension(0, DimensionUnit::VP).ToString());
static const std::string EXPECTED_PX_POS(Dimension(AINT32_POS, DimensionUnit::PX).ToString());
static const std::string EXPECTED_VP_POS(Dimension(AFLT32_POS, DimensionUnit::VP).ToString());

static const Dimension THEME_SWIPER_INDICATOR_SIZE(9876, DimensionUnit::VP);
static const Dimension THEME_SWIPER_FONT_SIZE(321, DimensionUnit::PX);
static const Color THEME_SWIPER_INDICATOR_COLOR(Color::BLUE);
static const Color THEME_SWIPER_ARROW_COLOR(Color::GREEN);

const auto RES_NAME = NamedResourceId("aa.bb.cc", Converter::ResourceType::COLOR);
const auto RES_ID = IntResourceId(1234, Converter::ResourceType::COLOR);
const auto CONTEXT_ID = 123;
} // namespace

class SwiperModifierTest : public ModifierTestBase<GENERATED_ArkUISwiperModifier,
    &GENERATED_ArkUINodeModifiers::getSwiperModifier, GENERATED_ARKUI_SWIPER> {
public:
    static void SetUpTestCase()
    {
        ModifierTestBase::SetUpTestCase();

        // set test values to Theme Pattern as data for the Theme building
        auto themeStyle = SetupThemeStyle("swiper_pattern");
        themeStyle->SetAttr("swiper_indicator_size", { .value = THEME_SWIPER_INDICATOR_SIZE });
        themeStyle->SetAttr("indicator_color", { .value = THEME_SWIPER_INDICATOR_COLOR });
        themeStyle->SetAttr("indicator_color_selected", { .value = THEME_SWIPER_INDICATOR_COLOR });
        themeStyle->SetAttr("indicator_text_font_size", { .value = THEME_SWIPER_FONT_SIZE });
        themeStyle->SetAttr(ARROW_COLOR_PRIMARY, { .value = THEME_SWIPER_ARROW_COLOR });
        themeStyle->SetAttr(ARROW_COLOR_COMPONENT_NORMAL, { .value = THEME_SWIPER_ARROW_COLOR });

        SetupTheme<SwiperIndicatorTheme>();
    }
};

/**
 * @tc.name: setIndexTest
 * @tc.desc: Check the functionality of SwiperModifier.IndexImpl
 * @tc.type: FUNC
 */
HWTEST_F(SwiperModifierTest, setIndexTest, TestSize.Level1)
{
    static const std::string PROP_NAME("index");
    static const std::string DEFAULT_VALUE("0");
    ASSERT_NE(modifier_->setIndex, nullptr);

    auto numberInt = ArkValue<Ark_Number>(123456);
    modifier_->setIndex(node_, &numberInt);
    auto checkVal2 = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkVal2, "123456");

    auto numberFlt = ArkValue<Ark_Number>(1.23456f);
    modifier_->setIndex(node_, &numberFlt);
    auto checkVal3 = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkVal3, "1");

    auto numberIntNeg = ArkValue<Ark_Number>(-1);
    modifier_->setIndex(node_, &numberIntNeg);
    auto checkVal4 = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkVal4, DEFAULT_VALUE);

    auto numberFltNeg = ArkValue<Ark_Number>(-1.111f);
    modifier_->setIndex(node_, &numberFltNeg);
    auto checkVal5 = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkVal5, DEFAULT_VALUE);
}
/**
 * @tc.name: setAutoPlayTest
 * @tc.desc: Check the functionality of SwiperModifier.AutoPlayImpl
 * @tc.type: FUNC
 */
HWTEST_F(SwiperModifierTest, setAutoPlayTest, TestSize.Level1)
{
    static const std::string PROP_NAME("autoPlay");
    static const std::string &DEFAULT_VALUE(EXPECTED_FALSE);
    ASSERT_NE(modifier_->setAutoPlay0, nullptr);

    auto checkInitial = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkInitial, DEFAULT_VALUE);

    modifier_->setAutoPlay0(node_, ABOOL_TRUE);
    auto checkVal2 = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkVal2, EXPECTED_TRUE);

    modifier_->setAutoPlay0(node_, ABOOL_FALSE);
    auto checkVal3 = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkVal3, EXPECTED_FALSE);
}
/**
 * @tc.name: setIntervalTest
 * @tc.desc: Check the functionality of SwiperModifier.IntervalImpl
 * @tc.type: FUNC
 */
HWTEST_F(SwiperModifierTest, setIntervalTest, TestSize.Level1)
{
    static const std::string PROP_NAME("interval");
    static const std::string DEFAULT_VALUE(std::to_string(static_cast<int>(DEFAULT_SWIPER_AUTOPLAY_INTERVAL)));
    ASSERT_NE(modifier_->setInterval, nullptr);

    auto checkInitial = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkInitial, DEFAULT_VALUE);

    auto numberInt = ArkValue<Ark_Number>(123456);
    modifier_->setInterval(node_, &numberInt);
    auto checkVal2 = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkVal2, "123456");

    auto numberFlt = ArkValue<Ark_Number>(1.23456f);
    modifier_->setInterval(node_, &numberFlt);
    auto checkVal3 = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkVal3, "1");

    auto numberIntNeg = ArkValue<Ark_Number>(-1);
    modifier_->setInterval(node_, &numberIntNeg);
    auto checkVal4 = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkVal4, DEFAULT_VALUE);

    auto numberFltNeg = ArkValue<Ark_Number>(-1.111f);
    modifier_->setInterval(node_, &numberFltNeg);
    auto checkVal5 = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkVal5, DEFAULT_VALUE);
}

/**
 * @tc.name: setIndicatorTestDotPadding
 * @tc.desc: Check the functionality of SwiperModifier.IndicatorImpl with Dot type, the padding's subattr
 * @tc.type: FUNC
 */
HWTEST_F(SwiperModifierTest, setIndicatorTestDotPadding, TestSize.Level1)
{
    typedef std::tuple<Ark_DotIndicator, std::string> OneTestStep;
    static const std::string PROP_NAME("indicator");
    static const std::string &DEFAULT_VALUE(EXPECTED_VP_ZERO);
    static const std::vector<OneTestStep> testPlan = {
    { { ._left = OPT_LEN_VP_POS, ._top = OPT_LEN_VP_POS, ._right = OPT_LEN_VP_POS, ._bottom = OPT_LEN_VP_POS,
        ._start = ArkValue<Opt_LengthMetrics>(), ._end = ArkValue<Opt_LengthMetrics>(),
        }, EXPECTED_VP_POS },
    { { ._left = OPT_LEN_VP_NEG, ._top = OPT_LEN_VP_NEG, ._right = OPT_LEN_VP_NEG, ._bottom = OPT_LEN_VP_NEG,
        ._start = ArkValue<Opt_LengthMetrics>(), ._end = ArkValue<Opt_LengthMetrics>(),
        }, DEFAULT_VALUE },
    };

    ASSERT_NE(modifier_->setIndicator, nullptr);

    auto checkInitial = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkInitial, EXPECTED_TRUE);

    static const std::vector<std::string> keys = { "left", "right", "top", "bottom",
        // "start", "end" - these fields are not supported in SwiperPattern::GetDotIndicatorStyle()
    };
    for (const auto &[indicator, expect]: testPlan) {
        auto arkParam = ArkUnion<Ark_Union_DotIndicator_DigitIndicator_Boolean, Ark_DotIndicator>(indicator);
        modifier_->setIndicator(node_, &arkParam);
        auto strWithObj = GetAttrValue<std::string>(node_, PROP_NAME);
        for (const auto &nameKey: keys) {
            auto checkPadding = GetAttrValue<std::string>(strWithObj, nameKey);
            EXPECT_EQ(checkPadding, expect);
        }
    }
}

/**
 * @tc.name: setIndicatorTestDotSize
 * @tc.desc: Check the functionality of SwiperModifier.IndicatorImpl with Dot type, the size's subattr
 * @tc.type: FUNC
 */
HWTEST_F(SwiperModifierTest, setIndicatorTestDotSize, TestSize.Level1)
{
    typedef std::tuple<Ark_DotIndicator, std::string> OneTestStep;
    static const std::string PROP_NAME("indicator");
    static const std::string DEFAULT_SIZE = THEME_SWIPER_INDICATOR_SIZE.ToString();
    static const std::vector<OneTestStep> testPlan = {
    { { ._itemWidth = OPT_LEN_VP_POS, ._itemHeight = OPT_LEN_VP_POS,
        ._selectedItemWidth = OPT_LEN_VP_POS, ._selectedItemHeight = OPT_LEN_VP_POS,
        }, EXPECTED_VP_POS },
    { { ._itemWidth = OPT_LEN_VP_NEG, ._itemHeight = OPT_LEN_VP_NEG,
        ._selectedItemWidth = OPT_LEN_VP_NEG, ._selectedItemHeight = OPT_LEN_VP_NEG,
        }, DEFAULT_SIZE },
    };

    ASSERT_NE(modifier_->setIndicator, nullptr);

    auto checkInitial = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkInitial, EXPECTED_TRUE);

    static const std::vector<std::string> keys = {
        "itemWidth", "itemHeight", "selectedItemWidth", "selectedItemHeight"
    };
    for (const auto &[indicator, expect]: testPlan) {
        auto arkParam = ArkUnion<Ark_Union_DotIndicator_DigitIndicator_Boolean, Ark_DotIndicator>(indicator);
        modifier_->setIndicator(node_, &arkParam);
        auto strWithObj = GetAttrValue<std::string>(node_, PROP_NAME);
        for (const auto &nameKey: keys) {
            auto checkVal = GetAttrValue<std::string>(strWithObj, nameKey);
            EXPECT_EQ(checkVal, expect);
        }
    }
}

/**
 * @tc.name: setIndicatorTestDotColor
 * @tc.desc: Check the functionality of SwiperModifier.IndicatorImpl with Dot type, the Color type subattr
 * @tc.type: FUNC
 */
HWTEST_F(SwiperModifierTest, setIndicatorTestDotColor, TestSize.Level1)
{
    typedef std::pair<Ark_ResourceColor, std::string> OneTestStep;
    static const std::string PROP_NAME("indicator");
    static const std::string DEFAULT_VALUE(THEME_SWIPER_INDICATOR_COLOR.ToString());
    static const std::string EXPECTED_RESOURCE_COLOR =
        Color::RED.ToString(); // Color::RED is result of stubs for ThemeConstants::GetColorByName
    static const std::vector<OneTestStep> testPlan = {
        { ArkUnion<Ark_ResourceColor, Ark_Color>(ARK_COLOR_WHITE), "#FFFFFFFF" },
        { ArkUnion<Ark_ResourceColor, Ark_Number>(0x123456), "#FF123456" },
        { ArkUnion<Ark_ResourceColor, Ark_Number>(0.5f), "#00000000" },
        { ArkUnion<Ark_ResourceColor, Ark_String>("#11223344"), "#11223344" },
        { ArkUnion<Ark_ResourceColor, Ark_String>("65535"), "#FF00FFFF" },
        { CreateResourceUnion<Ark_ResourceColor>(RES_NAME), EXPECTED_RESOURCE_COLOR },
        { CreateResourceUnion<Ark_ResourceColor>(RES_ID), EXPECTED_RESOURCE_COLOR },
    };

    ASSERT_NE(modifier_->setIndicator, nullptr);

    auto checkInitial = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkInitial, EXPECTED_TRUE);

    Ark_DotIndicator indicator = {
        ._color = ArkValue<Opt_ResourceColor>(Ark_Empty()),
        ._selectedColor = ArkValue<Opt_ResourceColor>(Ark_Empty())
    };
    auto arkParam = ArkUnion<Ark_Union_DotIndicator_DigitIndicator_Boolean, Ark_DotIndicator>(indicator);
    modifier_->setIndicator(node_, &arkParam);
    auto strWithObj = GetAttrValue<std::string>(node_, PROP_NAME);
    auto checkColorDflt = GetAttrValue<std::string>(strWithObj, "color");
    EXPECT_EQ(checkColorDflt, DEFAULT_VALUE);
    auto checkSelColorDflt = GetAttrValue<std::string>(strWithObj, "selectedColor");
    EXPECT_EQ(checkSelColorDflt, DEFAULT_VALUE);

    for (const auto &[arkResColor, expected]: testPlan) {
        Ark_DotIndicator indicator = {
            ._color = ArkValue<Opt_ResourceColor>(arkResColor),
            ._selectedColor = ArkValue<Opt_ResourceColor>(arkResColor)
        };
        auto arkParam = ArkUnion<Ark_Union_DotIndicator_DigitIndicator_Boolean, Ark_DotIndicator>(indicator);
        modifier_->setIndicator(node_, &arkParam);
        auto strWithObj = GetAttrValue<std::string>(node_, PROP_NAME);
        auto checkColor = GetAttrValue<std::string>(strWithObj, "color");
        EXPECT_EQ(checkColor, expected);
        auto checkSelColor = GetAttrValue<std::string>(strWithObj, "selectedColor");
        EXPECT_EQ(checkSelColor, expected);
    }
}

/**
 * @tc.name: setIndicatorTestDotOther
 * @tc.desc: Check the functionality of SwiperModifier.IndicatorImpl with Dot type, the other subattr
 * @tc.type: FUNC
 */
HWTEST_F(SwiperModifierTest, setIndicatorTestDotOther, TestSize.Level1)
{
    typedef std::tuple<Ark_DotIndicator, std::string, int> OneTestStep;
    static const std::string PROP_NAME("indicator");
    static const std::vector<OneTestStep> testPlan = {
        { { ._mask = OPT_BOOL_UNDEF, ._maxDisplayCount = ArkValue<Opt_Number>(Ark_Empty()) },
            EXPECTED_FALSE, 0},
        { { ._mask = OPT_BOOL_TRUE, ._maxDisplayCount = ArkValue<Opt_Number>(5325) },
            EXPECTED_TRUE, 5325 },
        { { ._mask = OPT_BOOL_FALSE, ._maxDisplayCount = ArkValue<Opt_Number>(INT_MIN) },
            EXPECTED_FALSE, 0 },
    };

    ASSERT_NE(modifier_->setIndicator, nullptr);

    auto checkInitial = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkInitial, EXPECTED_TRUE);

    for (const auto &[indicator, expectMask, expectCount]: testPlan) {
        Ark_Union_DotIndicator_DigitIndicator_Boolean arkParam = { .selector = 0, .value0 = indicator };
        modifier_->setIndicator(node_, &arkParam);
        auto strWithObj = GetAttrValue<std::string>(node_, PROP_NAME);
        auto checkMask = GetAttrValue<std::string>(strWithObj, "mask");
        EXPECT_EQ(checkMask, expectMask);
        auto checkMaxDispCnt = GetAttrValue<int>(strWithObj, "maxDisplayCount");
        EXPECT_EQ(checkMaxDispCnt, expectCount);
    }
}
/**
 * @tc.name: setIndicatorTestDigitPadding
 * @tc.desc: Check the functionality of SwiperModifier.IndicatorImpl with Digit indicator, padding's attributes
 * @tc.type: FUNC
 */
HWTEST_F(SwiperModifierTest, setIndicatorTestDigitPadding, TestSize.Level1)
{
    typedef std::tuple<Ark_DigitIndicator, std::string> OneTestStep;
    static const std::string PROP_NAME("indicator");
    static const std::string DEFAULT_VALUE("0.00vp");
    static const std::vector<OneTestStep> testPlan = {
    { { ._left = OPT_LEN_VP_POS, ._top = OPT_LEN_VP_POS, ._right = OPT_LEN_VP_POS, ._bottom = OPT_LEN_VP_POS,
        ._start = ArkValue<Opt_LengthMetrics>(), ._end = ArkValue<Opt_LengthMetrics>(),
        }, EXPECTED_VP_POS },
    { { ._left = OPT_LEN_VP_NEG, ._top = OPT_LEN_VP_NEG, ._right = OPT_LEN_VP_NEG, ._bottom = OPT_LEN_VP_NEG,
        ._start = ArkValue<Opt_LengthMetrics>(), ._end = ArkValue<Opt_LengthMetrics>(),
        }, DEFAULT_VALUE },
    };

    ASSERT_NE(modifier_->setIndicator, nullptr);

    auto checkInitial = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkInitial, EXPECTED_TRUE);

    static const std::vector<std::string> keys = { "left", "right", "top", "bottom",
        // "start", "end" - these fields are not supported in SwiperPattern::GetDigitIndicatorStyle()
    };
    for (const auto &[indicator, expect]: testPlan) {
        Ark_Union_DotIndicator_DigitIndicator_Boolean arkParam = { .selector = 1, .value1 = indicator };
        modifier_->setIndicator(node_, &arkParam);
        auto strWithObj = GetAttrValue<std::string>(node_, PROP_NAME);
        for (const auto &nameKey: keys) {
            auto checkVal = GetAttrValue<std::string>(strWithObj, nameKey);
            EXPECT_EQ(checkVal, expect);
        }
    }
}

/**
 * @tc.name: setIndicatorTestDigitFontSize
 * @tc.desc: Check the functionality of SwiperModifier.IndicatorImpl with Digit indicator, font size attributes
 * @tc.type: FUNC
 */
HWTEST_F(SwiperModifierTest, setIndicatorTestDigitFontSize, TestSize.Level1)
{
    typedef std::tuple<Ark_Font, std::string> OneTestStep;
    static const std::string PROP_NAME("indicator");
    static const std::string DEFAULT_VALUE(THEME_SWIPER_FONT_SIZE.ToString());
    static const std::vector<OneTestStep> testPlan = {
        {{ .size = OPT_LEN_VP_POS }, EXPECTED_VP_POS},
        {{ .size = OPT_LEN_VP_NEG }, DEFAULT_VALUE},
        {{ .size = ArkValue<Opt_Dimension>(Ark_Empty()) }, DEFAULT_VALUE},
    };

    ASSERT_NE(modifier_->setIndicator, nullptr);

    auto checkInitial = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkInitial, EXPECTED_TRUE);

    for (const auto &[font, expect]: testPlan) {
        Ark_DigitIndicator indicator = {
            ._digitFont = ArkValue<Opt_Font>(font),
            ._selectedDigitFont = ArkValue<Opt_Font>(font)
        };
        auto arkParam = ArkUnion<Ark_Union_DotIndicator_DigitIndicator_Boolean, Ark_DigitIndicator>(indicator);
        modifier_->setIndicator(node_, &arkParam);
        auto strWithObj = GetAttrValue<std::string>(node_, PROP_NAME);
        auto checkFontSz = GetAttrValue<std::string>(strWithObj, "fontSize");
        EXPECT_EQ(checkFontSz, expect);
        auto checkSelFontSz = GetAttrValue<std::string>(strWithObj, "selectedFontSize");
        EXPECT_EQ(checkSelFontSz, expect);
    }
}

/**
 * @tc.name: setIndicatorTestDigitFontWeight
 * @tc.desc: Check the functionality of SwiperModifier.IndicatorImpl with Digit indicator, font weight attributes
 * @tc.type: FUNC
 */
HWTEST_F(SwiperModifierTest, setIndicatorTestDigitFontWeight, TestSize.Level1)
{
    typedef Opt_Union_FontWeight_Number_String FontWeightT;
    typedef std::tuple<Ark_Font, std::string> OneTestStep;
    static const std::string PROP_NAME("indicator");
    static const std::string DEFAULT_VALUE("FontWeight.Normal");
    static const std::vector<OneTestStep> testPlan = {
        { {.weight = ArkUnion<FontWeightT, Ark_FontWeight>(ARK_FONT_WEIGHT_BOLD)}, "FontWeight.Bold" },
        { {.weight = ArkUnion<FontWeightT, Ark_FontWeight>(ARK_FONT_WEIGHT_REGULAR)}, "FontWeight.Regular" },
        { {.weight = ArkUnion<FontWeightT, Ark_Number>(100)}, "100" },
        { {.weight = ArkUnion<FontWeightT, Ark_Number>(-111)}, DEFAULT_VALUE },
        { {.weight = ArkUnion<FontWeightT, Ark_Number>(300.00f)}, "300" },
        { {.weight = ArkUnion<FontWeightT, Ark_Number>(-123.456f)}, DEFAULT_VALUE },
        { {.weight = ArkUnion<FontWeightT, Ark_String>("700")}, "700" },
        { {.weight = ArkUnion<FontWeightT, Ark_String>("bold")}, "FontWeight.Bold" },
        { {.weight = ArkUnion<FontWeightT, Ark_String>("InvalidData!")}, DEFAULT_VALUE },
        { {.weight = ArkValue<FontWeightT>(Ark_Empty())}, DEFAULT_VALUE },
    };

    ASSERT_NE(modifier_->setIndicator, nullptr);

    auto checkInitial = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkInitial, EXPECTED_TRUE);

    {
        Ark_DigitIndicator indicator = {
            ._digitFont = ArkValue<Opt_Font>(Ark_Empty()),
            ._selectedDigitFont = ArkValue<Opt_Font>(Ark_Empty()),
        };
        auto arkParam = ArkUnion<Ark_Union_DotIndicator_DigitIndicator_Boolean, Ark_DigitIndicator>(indicator);
        modifier_->setIndicator(node_, &arkParam);
        auto strWithObj = GetAttrValue<std::string>(node_, PROP_NAME);
        auto checkFontWeightDftl = GetAttrValue<std::string>(strWithObj, "fontWeight");
        EXPECT_EQ(checkFontWeightDftl, DEFAULT_VALUE);
        auto checkSelFontWeightDftl = GetAttrValue<std::string>(strWithObj, "selectedFontWeight");
        EXPECT_EQ(checkSelFontWeightDftl, DEFAULT_VALUE);
    }
    for (const auto &[font, expect]: testPlan) {
        Ark_DigitIndicator indicator = {
            ._digitFont = ArkValue<Opt_Font>(font),
            ._selectedDigitFont = ArkValue<Opt_Font>(font)
        };
        auto arkParam = ArkUnion<Ark_Union_DotIndicator_DigitIndicator_Boolean, Ark_DigitIndicator>(indicator);
        modifier_->setIndicator(node_, &arkParam);
        auto strWithObj = GetAttrValue<std::string>(node_, PROP_NAME);
        auto checkFontWeight = GetAttrValue<std::string>(strWithObj, "fontWeight");
        EXPECT_EQ(checkFontWeight, expect);
        auto checkSelFontWeight = GetAttrValue<std::string>(strWithObj, "selectedFontWeight");
        EXPECT_EQ(checkSelFontWeight, expect);
    }
}

/**
 * @tc.name: setIndicatorTestDigitFontColor
 * @tc.desc: Check the functionality of SwiperModifier.IndicatorImpl with Digit Indicator, the Color type subattributes
 * @tc.type: FUNC
 */
HWTEST_F(SwiperModifierTest, setIndicatorTestDigitFontColor, TestSize.Level1)
{
    typedef std::pair<Ark_ResourceColor, std::string> OneTestStep;
    static const std::string PROP_NAME("indicator");
    static const std::string DEFAULT_VALUE(Color::TRANSPARENT.ToString());
    static const std::string EXPECTED_RESOURCE_COLOR =
        Color::RED.ToString(); // Color::RED is result of stubs for ThemeConstants::GetColorByName
    static const std::vector<OneTestStep> testPlan = {
        { ArkUnion<Ark_ResourceColor, Ark_Color>(ARK_COLOR_WHITE), "#FFFFFFFF" },
        { ArkUnion<Ark_ResourceColor, Ark_Number>(0x123456), "#FF123456" },
        { ArkUnion<Ark_ResourceColor, Ark_Number>(0.5f), "#00000000" },
        { ArkUnion<Ark_ResourceColor, Ark_String>("#11223344"), "#11223344" },
        { ArkUnion<Ark_ResourceColor, Ark_String>("65535"), "#FF00FFFF" },
        { CreateResourceUnion<Ark_ResourceColor>(RES_NAME), EXPECTED_RESOURCE_COLOR },
        { CreateResourceUnion<Ark_ResourceColor>(RES_ID), EXPECTED_RESOURCE_COLOR },
    };

    ASSERT_NE(modifier_->setIndicator, nullptr);

    auto checkInitial = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkInitial, EXPECTED_TRUE);

    Ark_DigitIndicator indicator = {
        ._fontColor = ArkValue<Opt_ResourceColor>(Ark_Empty()),
        ._selectedFontColor = ArkValue<Opt_ResourceColor>(Ark_Empty())
    };
    auto arkParam = ArkUnion<Ark_Union_DotIndicator_DigitIndicator_Boolean, Ark_DigitIndicator>(indicator);
    modifier_->setIndicator(node_, &arkParam);
    auto strWithObj = GetAttrValue<std::string>(node_, PROP_NAME);
    auto checkColorDflt = GetAttrValue<std::string>(strWithObj, "fontColor");
    EXPECT_EQ(checkColorDflt, DEFAULT_VALUE);
    auto checkSelColorDflt = GetAttrValue<std::string>(strWithObj, "selectedFontColor");
    EXPECT_EQ(checkSelColorDflt, DEFAULT_VALUE);

    for (const auto &[arkResColor, expected]: testPlan) {
        Ark_DigitIndicator indicator = {
            ._fontColor = ArkValue<Opt_ResourceColor>(arkResColor),
            ._selectedFontColor = ArkValue<Opt_ResourceColor>(arkResColor)
        };
        auto arkParam = ArkUnion<Ark_Union_DotIndicator_DigitIndicator_Boolean, Ark_DigitIndicator>(indicator);
        modifier_->setIndicator(node_, &arkParam);
        auto strWithObj = GetAttrValue<std::string>(node_, PROP_NAME);
        auto checkColor = GetAttrValue<std::string>(strWithObj, "fontColor");
        EXPECT_EQ(checkColor, expected);
        auto checkSelColor = GetAttrValue<std::string>(strWithObj, "selectedFontColor");
        EXPECT_EQ(checkSelColor, expected);
    }
}

/**
 * @tc.name: setIndicatorTestBoolean
 * @tc.desc: Check the functionality of SwiperModifier.IndicatorImpl with Boolean type
 * @tc.type: FUNC
 */
HWTEST_F(SwiperModifierTest, setIndicatorTestBoolean, TestSize.Level1)
{
    static const std::string PROP_NAME("indicator");
    ASSERT_NE(modifier_->setIndicator, nullptr);

    auto checkInitial = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkInitial, EXPECTED_TRUE);

    auto boolIndFalse = ArkUnion<Ark_Union_DotIndicator_DigitIndicator_Boolean, Ark_Boolean>(ABOOL_FALSE);
    modifier_->setIndicator(node_, &boolIndFalse);
    auto checkFalse = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkFalse, EXPECTED_FALSE);

    auto boolIndTrue = ArkUnion<Ark_Union_DotIndicator_DigitIndicator_Boolean, Ark_Boolean>(ABOOL_TRUE);
    modifier_->setIndicator(node_, &boolIndTrue);
    auto checkTrue = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkTrue, EXPECTED_TRUE);
}

/**
 * @tc.name: setDisplayArrowTestStyleShowBg
 * @tc.desc: Check the functionality of SwiperModifier.setDisplayArrow with the Style size subattruibutes
 * @tc.type: FUNC
 */
HWTEST_F(SwiperModifierTest, setDisplayArrowTestStyleShowBg, TestSize.Level1)
{
    static const std::string PROP_NAME("showBackground");
    static const std::string DEFAULT_VALUE = EXPECTED_FALSE;

    ASSERT_NE(modifier_->setDisplayArrow, nullptr);

    auto checkValInitial = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkValInitial, DEFAULT_VALUE);

    Ark_Union_ArrowStyle_Boolean arkParam1 = {.selector=0, .value0={.showBackground = OPT_BOOL_TRUE}};
    modifier_->setDisplayArrow(node_, &arkParam1, nullptr);
    auto checkInitial = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkInitial, EXPECTED_TRUE);

    Ark_Union_ArrowStyle_Boolean arkParam2 = {.selector=0, .value0={.showBackground = OPT_BOOL_FALSE}};
    modifier_->setDisplayArrow(node_, &arkParam2, nullptr);
    auto checkVal2 = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkVal2, EXPECTED_FALSE);

    Ark_Union_ArrowStyle_Boolean arkParam3 = {.selector=0, .value0={.showBackground = OPT_BOOL_UNDEF}};
    modifier_->setDisplayArrow(node_, &arkParam3, nullptr);
    auto checkVal3 = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkVal3, DEFAULT_VALUE);
}

/**
 * @tc.name: setDisplayArrowTestStyleSize
 * @tc.desc: Check the functionality of SwiperModifier.setDisplayArrow with the Style size subattruibutes
 * @tc.type: FUNC
 */
HWTEST_F(SwiperModifierTest, setDisplayArrowTestStyleSize, TestSize.Level1)
{
    typedef std::tuple<Ark_ArrowStyle, std::string> OneTestStep;
    static const std::string PROP_NAME("arrowSize");
    static const std::string &DEFAULT_VALUE_S(EXPECTED_VP_ZERO);
    static const std::string &DEFAULT_VALUE_B(EXPECTED_VP_ZERO);
    static const std::vector<OneTestStep> testPlan = {
        { {.isSidebarMiddle = OPT_BOOL_FALSE, .arrowSize = OPT_LEN_VP_POS}, "1.23vp" },
        { {.isSidebarMiddle = OPT_BOOL_FALSE, .arrowSize = OPT_LEN_VP_NEG}, DEFAULT_VALUE_S },
        { {.isSidebarMiddle = OPT_BOOL_FALSE, .arrowSize = OPT_LEN_UNDEF}, DEFAULT_VALUE_S },
        { {.isSidebarMiddle = OPT_BOOL_TRUE, .arrowSize = OPT_LEN_VP_POS}, "1.23vp" },
        { {.isSidebarMiddle = OPT_BOOL_TRUE, .arrowSize = OPT_LEN_VP_NEG}, DEFAULT_VALUE_B },
        { {.isSidebarMiddle = OPT_BOOL_TRUE, .arrowSize = OPT_LEN_UNDEF}, DEFAULT_VALUE_B },
        { {.isSidebarMiddle = OPT_BOOL_UNDEF, .arrowSize = OPT_LEN_VP_POS}, "1.23vp" },
        { {.isSidebarMiddle = OPT_BOOL_UNDEF, .arrowSize = OPT_LEN_VP_NEG}, DEFAULT_VALUE_S },
        { {.isSidebarMiddle = OPT_BOOL_UNDEF, .arrowSize = OPT_LEN_UNDEF}, DEFAULT_VALUE_S },
    };

    ASSERT_NE(modifier_->setDisplayArrow, nullptr);

    auto checkInitial = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkInitial, "0.00vp");

    for (const auto &[style, expect]: testPlan) {
        Ark_Union_ArrowStyle_Boolean arkParam = {
            .selector = 0, .value0 = style
        };
        modifier_->setDisplayArrow(node_, &arkParam, nullptr);
        auto checkVal2 = GetAttrValue<std::string>(node_, PROP_NAME);
        EXPECT_EQ(checkVal2, expect);
        auto checkVal3 = GetAttrValue<std::string>(node_, "isSidebarMiddle");
        auto optExp3 = Converter::OptConvert<bool>(style.isSidebarMiddle);
        bool expect3 = optExp3 && *optExp3;
        EXPECT_EQ(checkVal3, expect3 ? EXPECTED_TRUE : EXPECTED_FALSE);
    }
}

/**
 * @tc.name: setDisplayArrowTestStyleBgSize
 * @tc.desc: Check the functionality of SwiperModifier.setDisplayArrow with the Style, bg size subattruibutes
 * @tc.type: FUNC
 */
HWTEST_F(SwiperModifierTest, setDisplayArrowTestStyleBgSize, TestSize.Level1)
{
    typedef std::tuple<Ark_ArrowStyle, std::string> OneTestStep;
    static const std::string PROP_NAME("arrowBackgroundSize");
    static const std::string &DEFAULT_VALUE_S(EXPECTED_VP_ZERO);
    static const std::string &DEFAULT_VALUE_B(EXPECTED_VP_ZERO);
    static const std::vector<OneTestStep> testPlan = {
        { {.isSidebarMiddle = OPT_BOOL_FALSE, .backgroundSize = OPT_LEN_VP_POS}, "1.23vp" },
        { {.isSidebarMiddle = OPT_BOOL_FALSE, .backgroundSize = OPT_LEN_VP_NEG}, DEFAULT_VALUE_S },
        { {.isSidebarMiddle = OPT_BOOL_FALSE, .backgroundSize = OPT_LEN_UNDEF}, DEFAULT_VALUE_S },
        { {.isSidebarMiddle = OPT_BOOL_TRUE, .backgroundSize = OPT_LEN_VP_POS}, "1.23vp" },
        { {.isSidebarMiddle = OPT_BOOL_TRUE, .backgroundSize = OPT_LEN_VP_NEG}, DEFAULT_VALUE_B },
        { {.isSidebarMiddle = OPT_BOOL_TRUE, .backgroundSize = OPT_LEN_UNDEF}, DEFAULT_VALUE_B },
        { {.isSidebarMiddle = OPT_BOOL_UNDEF, .backgroundSize = OPT_LEN_VP_POS}, "1.23vp" },
        { {.isSidebarMiddle = OPT_BOOL_UNDEF, .backgroundSize = OPT_LEN_VP_NEG}, DEFAULT_VALUE_S },
        { {.isSidebarMiddle = OPT_BOOL_UNDEF, .backgroundSize = OPT_LEN_UNDEF}, DEFAULT_VALUE_S },
    };

    ASSERT_NE(modifier_->setDisplayArrow, nullptr);

    auto checkInitial = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkInitial, "0.00vp");

    for (const auto &[style, expect]: testPlan) {
        Ark_Union_ArrowStyle_Boolean arkParam = {
            .selector = 0, .value0 = style
        };
        modifier_->setDisplayArrow(node_, &arkParam, nullptr);
        auto checkVal2 = GetAttrValue<std::string>(node_, PROP_NAME);
        EXPECT_EQ(checkVal2, expect);
    }
}

/**
 * @tc.name: setDisplayArrowTestStyleColorDefault
 * @tc.desc: Check the functionality of SwiperModifier.DisplayArrowImpl with Style, the Color's default and init
 * @tc.type: FUNC
 */
HWTEST_F(SwiperModifierTest, setDisplayArrowTestStyleColorDefault, TestSize.Level1)
{
    static const std::string DEFAULT_VALUE = Color::TRANSPARENT.ToString();
    {
        auto initArrowColor = GetAttrValue<std::string>(node_, "arrowColor");
        EXPECT_EQ(initArrowColor, Color::TRANSPARENT.ToString());
        auto initArrowBgColor = GetAttrValue<std::string>(node_, "arrowBackgroundColor");
        EXPECT_EQ(initArrowBgColor, Color::TRANSPARENT.ToString());
    }
    {
        Ark_Union_ArrowStyle_Boolean arkParam = { .selector = 0, .value0 = {
            .isSidebarMiddle = OPT_BOOL_FALSE,
            .arrowColor = OPT_COLOR_UNDEF,
            .backgroundColor = OPT_COLOR_UNDEF,
        } };
        modifier_->setDisplayArrow(node_, &arkParam, nullptr);
        auto defaultSmallArrowColor = GetAttrValue<std::string>(node_, "arrowColor");
        EXPECT_EQ(defaultSmallArrowColor, DEFAULT_VALUE);
        auto defaultSmallArrowBgColor = GetAttrValue<std::string>(node_, "arrowBackgroundColor");
        EXPECT_EQ(defaultSmallArrowBgColor, Color::TRANSPARENT.ToString());
    }
    {
        Ark_Union_ArrowStyle_Boolean arkParam = { .selector = 0, .value0 = {
            .isSidebarMiddle = OPT_BOOL_TRUE,
            .arrowColor = OPT_COLOR_UNDEF,
            .backgroundColor = OPT_COLOR_UNDEF,
        } };
        modifier_->setDisplayArrow(node_, &arkParam, nullptr);
        auto defaultBigArrowColor = GetAttrValue<std::string>(node_, "arrowColor");
        EXPECT_EQ(defaultBigArrowColor, DEFAULT_VALUE);
        auto defaultBigArrowBgColor = GetAttrValue<std::string>(node_, "arrowBackgroundColor");
        EXPECT_EQ(defaultBigArrowBgColor, DEFAULT_VALUE);
    }
}

/**
 * @tc.name: setDisplayArrowTestStyleColor
 * @tc.desc: Check the functionality of SwiperModifier.DisplayArrowImpl with Style, the Color's subattributes
 * @tc.type: FUNC
 */
HWTEST_F(SwiperModifierTest, setDisplayArrowTestStyleColor, TestSize.Level1)
{
    typedef std::pair<Ark_ResourceColor, std::string> OneTestStep;
    static const std::string EXPECTED_RESOURCE_COLOR =
        Color::RED.ToString(); // Color::RED is result of stubs for ThemeConstants::GetColorByName
    static const std::vector<OneTestStep> testPlan = {
        { ArkUnion<Ark_ResourceColor, Ark_Color>(ARK_COLOR_WHITE), "#FFFFFFFF" },
        { ArkUnion<Ark_ResourceColor, Ark_Number>(0x123456), "#FF123456" },
        { ArkUnion<Ark_ResourceColor, Ark_Number>(0.5f), "#00000000" },
        { ArkUnion<Ark_ResourceColor, Ark_String>("#11223344"), "#11223344" },
        { ArkUnion<Ark_ResourceColor, Ark_String>("65535"), "#FF00FFFF" },
        { CreateResourceUnion<Ark_ResourceColor>(RES_NAME), EXPECTED_RESOURCE_COLOR },
        { CreateResourceUnion<Ark_ResourceColor>(RES_ID), EXPECTED_RESOURCE_COLOR },
    };

    ASSERT_NE(modifier_->setDisplayArrow, nullptr);

    for (const auto &[arkResColor, expected]: testPlan) {
        Ark_ArrowStyle style = {
             .arrowColor = ArkValue<Opt_ResourceColor>(arkResColor),
             .backgroundColor = ArkValue<Opt_ResourceColor>(arkResColor)
        };
        auto arkParam = ArkUnion<Ark_Union_ArrowStyle_Boolean, Ark_ArrowStyle>(style);
        modifier_->setDisplayArrow(node_, &arkParam, nullptr);
        auto checkColor = GetAttrValue<std::string>(node_, "arrowColor");
        EXPECT_EQ(checkColor, expected);
        auto checkBgColor = GetAttrValue<std::string>(node_, "arrowBackgroundColor");
        EXPECT_EQ(checkBgColor, expected);
    }
}

/**
 * @tc.name: setDisplayArrowTestBoolean
 * @tc.desc: Check the functionality of SwiperModifier.setDisplayArrow with Boolean type
 * @tc.type: FUNC
 */
HWTEST_F(SwiperModifierTest, setDisplayArrowTestBoolean, TestSize.Level1)
{
    static const std::string PROP_NAME("displayArrow");
    static const std::string PROP_NAME_EXT("hoverShow");
    ASSERT_NE(modifier_->setDisplayArrow, nullptr);

    auto isDisplayArrowDefault = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(isDisplayArrowDefault, EXPECTED_FALSE);
    auto isHoverShowDefault = GetAttrValue<std::string>(node_, PROP_NAME_EXT);
    EXPECT_EQ(isHoverShowDefault, EXPECTED_FALSE);

    auto boolFalse = ArkUnion<Ark_Union_ArrowStyle_Boolean, Ark_Boolean>(ABOOL_FALSE);
    modifier_->setDisplayArrow(node_, &boolFalse, nullptr);
    auto checkVal2 = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkVal2, EXPECTED_FALSE);
    auto checkVal2x = GetAttrValue<std::string>(node_, PROP_NAME_EXT);
    EXPECT_EQ(checkVal2x, EXPECTED_FALSE);

    auto boolIndTrue = ArkUnion<Ark_Union_ArrowStyle_Boolean, Ark_Boolean>(ABOOL_TRUE);
    modifier_->setDisplayArrow(node_, &boolIndTrue, nullptr);
    auto checkVal3 = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkVal3, EXPECTED_TRUE);
    auto checkVal3x = GetAttrValue<std::string>(node_, PROP_NAME_EXT);
    EXPECT_EQ(checkVal3x, EXPECTED_FALSE);

    modifier_->setDisplayArrow(node_, &boolIndTrue, &OPT_BOOL_TRUE);
    auto checkVal4x = GetAttrValue<std::string>(node_, PROP_NAME_EXT);
    EXPECT_EQ(checkVal4x, EXPECTED_TRUE);

    modifier_->setDisplayArrow(node_, &boolIndTrue, &OPT_BOOL_UNDEF);
    auto checkVal5x = GetAttrValue<std::string>(node_, PROP_NAME_EXT);
    EXPECT_EQ(checkVal5x, EXPECTED_TRUE); // nothing change

    modifier_->setDisplayArrow(node_, &boolIndTrue, &OPT_BOOL_FALSE);
    auto checkVal6x = GetAttrValue<std::string>(node_, PROP_NAME_EXT);
    EXPECT_EQ(checkVal6x, EXPECTED_FALSE);

    modifier_->setDisplayArrow(node_, &boolIndTrue, &OPT_BOOL_UNDEF);
    auto checkVal7x = GetAttrValue<std::string>(node_, PROP_NAME_EXT);
    EXPECT_EQ(checkVal7x, EXPECTED_FALSE); // nothing change
}
/**
 * @tc.name: setLoopTest
 * @tc.desc: Check the functionality of SwiperModifier.LoopImpl
 * @tc.type: FUNC
 */
HWTEST_F(SwiperModifierTest, setLoopTest, TestSize.Level1)
{
    static const std::string PROP_NAME("loop");
    static const std::string &DEFAULT_VALUE(EXPECTED_TRUE);
    ASSERT_NE(modifier_->setLoop, nullptr);

    auto checkInitial = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkInitial, DEFAULT_VALUE);

    modifier_->setLoop(node_, ABOOL_TRUE);
    auto checkTrue = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkTrue, EXPECTED_TRUE);

    modifier_->setLoop(node_, ABOOL_FALSE);
    auto checkFalse = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkFalse, EXPECTED_FALSE);
}
/**
 * @tc.name: setDurationTest
 * @tc.desc: Check the functionality of SwiperModifier.DurationImpl
 * @tc.type: FUNC
 */
HWTEST_F(SwiperModifierTest, setDurationTest, TestSize.Level1)
{
    static const std::string PROP_NAME("duration");
    static const std::string DEFAULT_VALUE(std::to_string(static_cast<int32_t>(DEFAULT_SWIPER_ANIMATION_DURATION)));
    ASSERT_NE(modifier_->setDuration, nullptr);

    auto checkInitial = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkInitial, DEFAULT_VALUE);

    auto numberInt = ArkValue<Ark_Number>(123456);
    modifier_->setDuration(node_, &numberInt);
    auto checkVal2 = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkVal2, "123456");

    auto numberFlt = ArkValue<Ark_Number>(1.23456f);
    modifier_->setDuration(node_, &numberFlt);
    auto checkVal3 = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkVal3, "1");

    auto numberIntNeg = ArkValue<Ark_Number>(-1);
    modifier_->setDuration(node_, &numberIntNeg);
    auto checkVal4 = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkVal4, DEFAULT_VALUE);

    auto numberFltNeg = ArkValue<Ark_Number>(-1.111f);
    modifier_->setDuration(node_, &numberFltNeg);
    auto checkVal5 = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkVal5, DEFAULT_VALUE);
}
/**
 * @tc.name: setVerticalTest
 * @tc.desc: Check the functionality of SwiperModifier.VerticalImpl
 * @tc.type: FUNC
 */
HWTEST_F(SwiperModifierTest, setVerticalTest, TestSize.Level1)
{
    static const std::string PROP_NAME("vertical");
    static const std::string DEFAULT_VALUE(EXPECTED_FALSE);
    ASSERT_NE(modifier_->setVertical, nullptr);

    auto checkInitial = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkInitial, DEFAULT_VALUE);

    modifier_->setVertical(node_, ABOOL_TRUE);
    auto checkVal2 = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkVal2, EXPECTED_TRUE);

    modifier_->setVertical(node_, ABOOL_FALSE);
    auto checkVal3 = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkVal3, EXPECTED_FALSE);
}
/**
 * @tc.name: setItemSpaceTest
 * @tc.desc: Check the functionality of SwiperModifier.ItemSpaceImpl
 * @tc.type: FUNC
 */
HWTEST_F(SwiperModifierTest, setItemSpaceTest, TestSize.Level1)
{
    using namespace Converter;
    using TypeArg0 = Ark_Union_Number_String;
    using OneTestStep = std::pair<TypeArg0, std::string>;
    static const std::string PROP_NAME("itemSpace");
    static const std::string &DEFAULT_VALUE(EXPECTED_VP_ZERO);
    static const std::vector<OneTestStep> testPlan = {
        { ArkUnion<TypeArg0, Ark_Number>(AINT32_POS), Dimension(AINT32_POS, DimensionUnit::VP).ToString()},
        { ArkUnion<TypeArg0, Ark_Number>(AFLT32_POS), Dimension(AFLT32_POS, DimensionUnit::VP).ToString()},
        { ArkUnion<TypeArg0, Ark_String>("7.89px"), "7.89px"},
        { ArkUnion<TypeArg0, Ark_String>("1.2345vp"), "1.23vp"},
        { ArkUnion<TypeArg0, Ark_Number>(AINT32_NEG), DEFAULT_VALUE},
        { ArkUnion<TypeArg0, Ark_Number>(AFLT32_NEG), DEFAULT_VALUE},
        { ArkUnion<TypeArg0, Ark_String>("-4.5px"), DEFAULT_VALUE},
        { ArkUnion<TypeArg0, Ark_String>("-56vp"), DEFAULT_VALUE},
    };

    ASSERT_NE(modifier_->setItemSpace, nullptr);

    auto checkInitial = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkInitial, DEFAULT_VALUE);

    for (const auto &[itemSpace, expected]: testPlan) {
        modifier_->setItemSpace(node_, &itemSpace);
        auto checkVal = GetAttrValue<std::string>(node_, PROP_NAME);
        EXPECT_EQ(checkVal, expected);
    }
}
/**
 * @tc.name: setDisplayModeTest
 * @tc.desc: Check the functionality of SwiperModifier.DisplayModeImpl
 * @tc.type: FUNC
 */
HWTEST_F(SwiperModifierTest, setDisplayModeTest, TestSize.Level1)
{
    static const std::string PROP_NAME("displayMode");
    static const std::string DEFAULT_VALUE("SwiperDisplayMode.Stretch"); // corrrsponds to
    // SwiperDisplayMode::STRETCH in frameworks/core/components/declaration/swiper/swiper_declaration.h
    ASSERT_NE(modifier_->setDisplayMode, nullptr);

    auto checkInitial = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkInitial, DEFAULT_VALUE);

    auto arkValue0 = ARK_SWIPER_DISPLAY_MODE_STRETCH;
    modifier_->setDisplayMode(node_, arkValue0);
    auto checkVal2 = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkVal2, "SwiperDisplayMode.Stretch");

    auto arkValue1 = ARK_SWIPER_DISPLAY_MODE_AUTO_LINEAR;
    modifier_->setDisplayMode(node_, arkValue1);
    auto checkVal3 = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkVal3, "SwiperDisplayMode.AutoLinear");

    auto arkValue2 = static_cast<Ark_SwiperDisplayMode>(INT_MAX);
    modifier_->setDisplayMode(node_, arkValue2);
    auto checkVal4 = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkVal4, "SwiperDisplayMode.AutoLinear"); // nothing changes

    auto arkValue3 = static_cast<Ark_SwiperDisplayMode>(INT_MIN);
    modifier_->setDisplayMode(node_, arkValue3);
    auto checkVal5 = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkVal5, "SwiperDisplayMode.AutoLinear"); // nothing changes
}
/**
 * @tc.name: setCachedCountTest
 * @tc.desc: Check the functionality of SwiperModifier.CachedCountImpl
 * @tc.type: FUNC
 */
HWTEST_F(SwiperModifierTest, setCachedCountTest, TestSize.Level1)
{
    static const std::string PROP_NAME("cachedCount");
    static const int DEFAULT_VALUE(DEFAULT_SWIPER_CACHED_SIZE);
    ASSERT_NE(modifier_->setCachedCount0, nullptr);

    auto checkInitial = GetAttrValue<int>(node_, PROP_NAME);
    EXPECT_EQ(checkInitial, DEFAULT_VALUE);

    Ark_Number numberInt = ArkValue<Ark_Number>(123456);
    modifier_->setCachedCount0(node_, &numberInt);
    auto checkVal2 = GetAttrValue<int>(node_, PROP_NAME);
    EXPECT_EQ(checkVal2, 123456);

    Ark_Number numberFlt = ArkValue<Ark_Number>(1.23456f);
    modifier_->setCachedCount0(node_, &numberFlt);
    auto checkVal3 = GetAttrValue<int>(node_, PROP_NAME);
    EXPECT_EQ(checkVal3, 1);

    Ark_Number numberIntNeg = ArkValue<Ark_Number>(-111);
    modifier_->setCachedCount0(node_, &numberIntNeg);
    auto checkVal4 = GetAttrValue<int>(node_, PROP_NAME);
    EXPECT_EQ(checkVal4, DEFAULT_VALUE);

    Ark_Number numberFltNeg = ArkValue<Ark_Number>(-1.111f);
    modifier_->setCachedCount0(node_, &numberFltNeg);
    auto checkVal5 = GetAttrValue<int>(node_, PROP_NAME);
    EXPECT_EQ(checkVal5, DEFAULT_VALUE);
}
/**
 * @tc.name: setDisplayCountTestNumber
 * @tc.desc: Check the functionality of SwiperModifier.DisplayCountImpl with Ark_Number arg
 * @tc.type: FUNC
 */
HWTEST_F(SwiperModifierTest, setDisplayCountTestNumber, TestSize.Level1)
{
    static const std::string PROP_NAME("displayCount");
    static const int DEFAULT_VALUE(DEFAULT_SWIPER_DISPLAY_COUNT);
    ASSERT_NE(modifier_->setDisplayCount, nullptr);

    auto checkInitial = GetAttrValue<int>(node_, PROP_NAME);
    EXPECT_EQ(checkInitial, DEFAULT_VALUE);

    auto numberInt = ArkUnion<Ark_Union_Number_String_SwiperAutoFill, Ark_Number>(123456);
    modifier_->setDisplayCount(node_, &numberInt, nullptr);
    auto checkVal2 = GetAttrValue<int>(node_, PROP_NAME);
    EXPECT_EQ(checkVal2, 123456);

    auto numberFlt = ArkUnion<Ark_Union_Number_String_SwiperAutoFill, Ark_Number>(1.23456f);
    modifier_->setDisplayCount(node_, &numberFlt, nullptr);
    auto checkVal3 = GetAttrValue<int>(node_, PROP_NAME);
    EXPECT_EQ(checkVal3, 1);

    auto numberIntNeg = ArkUnion<Ark_Union_Number_String_SwiperAutoFill, Ark_Number>(-111);
    modifier_->setDisplayCount(node_, &numberIntNeg, nullptr);
    auto checkVal4 = GetAttrValue<int>(node_, PROP_NAME);
    EXPECT_EQ(checkVal4, DEFAULT_VALUE);

    auto numberFltNeg = ArkUnion<Ark_Union_Number_String_SwiperAutoFill, Ark_Number>(-1.111f);
    modifier_->setDisplayCount(node_, &numberFltNeg, nullptr);

    auto checkVal5 = GetAttrValue<int>(node_, PROP_NAME);
    EXPECT_EQ(checkVal5, DEFAULT_VALUE);
}
/**
 * @tc.name: setDisplayCountTestString
 * @tc.desc: Check the functionality of SwiperModifier.DisplayCountImpl with Ark_String arg
 * @tc.type: FUNC
 */
HWTEST_F(SwiperModifierTest, setDisplayCountTestString, TestSize.Level1)
{
    static const std::string PROP_NAME("displayCount");
    static const int DEFAULT_VALUE(DEFAULT_SWIPER_DISPLAY_COUNT);
    ASSERT_NE(modifier_->setDisplayCount, nullptr);

    auto checkInitial = GetAttrValue<int>(node_, PROP_NAME);
    EXPECT_EQ(checkInitial, DEFAULT_VALUE);

    auto regularVal = ArkUnion<Ark_Union_Number_String_SwiperAutoFill, Ark_String>("1234");
    modifier_->setDisplayCount(node_, &regularVal, nullptr);
    auto checkVal2 = GetAttrValue<int>(node_, PROP_NAME);
    EXPECT_EQ(checkVal2, 1234);

    auto arkValue0 = ArkValue<Ark_SwiperDisplayMode>(ARK_SWIPER_DISPLAY_MODE_STRETCH);
    modifier_->setDisplayMode(node_, arkValue0);
    auto autoVal = ArkUnion<Ark_Union_Number_String_SwiperAutoFill, Ark_String>("auto");
    modifier_->setDisplayCount(node_, &autoVal, nullptr);
    auto checkVal3 = GetAttrValue<int>(node_, PROP_NAME);
    EXPECT_EQ(checkVal3, DEFAULT_VALUE);
    auto checkValDispMode= GetAttrValue<std::string>(node_, "displayMode");
    EXPECT_EQ(checkValDispMode, "SwiperDisplayMode.AutoLinear");

    auto negVal = ArkUnion<Ark_Union_Number_String_SwiperAutoFill, Ark_String>("-1234");
    modifier_->setDisplayCount(node_, &negVal, nullptr);
    auto checkVal4 = GetAttrValue<int>(node_, PROP_NAME);
    EXPECT_EQ(checkVal4, DEFAULT_VALUE);
}

/**
 * @tc.name: setDisplayCountTestObj
 * @tc.desc: Check the functionality of SwiperModifier.DisplayCountImpl with Ark_VP object argument
 * @tc.type: FUNC
 */
HWTEST_F(SwiperModifierTest, setDisplayCountTestObj, TestSize.Level1)
{
    typedef Ark_Union_Number_String_SwiperAutoFill displayCountArgT;
    typedef Ark_SwiperAutoFill checkT;
    static const std::string PROP_NAME("minSize");
    static const std::string &DEFAULT_VALUE(EXPECTED_VP_ZERO);
    ASSERT_NE(modifier_->setDisplayCount, nullptr);

    auto checkInitial = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkInitial, DEFAULT_VALUE);

    auto intVal = ArkUnion<displayCountArgT, checkT>(ArkValue<checkT>(ArkUnion<Ark_VP, Ark_Number>(123)));
    modifier_->setDisplayCount(node_, &intVal, nullptr);
    auto checkInt = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkInt, "123.00vp");

    auto floatVal = ArkUnion<displayCountArgT, checkT>(ArkValue<checkT>(ArkUnion<Ark_VP, Ark_Number>(1.23f)));
    modifier_->setDisplayCount(node_, &floatVal, nullptr);
    auto checkFloat = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkFloat, "1.23vp");

    auto strPx = ArkUnion<displayCountArgT, checkT>(ArkValue<checkT>(ArkUnion<Ark_VP, Ark_String>("45px")));
    modifier_->setDisplayCount(node_, &strPx, nullptr);
    auto checkStrPx = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkStrPx, "45.00px");

    auto strVp = ArkUnion<displayCountArgT, checkT>(ArkValue<checkT>(ArkUnion<Ark_VP, Ark_String>("5.60vp")));
    modifier_->setDisplayCount(node_, &strVp, nullptr);
    auto checkStrVp = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkStrVp, "5.60vp");

    auto intNegVal = ArkUnion<displayCountArgT, checkT>(ArkValue<checkT>(ArkUnion<Ark_VP, Ark_Number>(-123)));
    modifier_->setDisplayCount(node_, &intNegVal, nullptr);
    auto checkIntNeg = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkIntNeg, DEFAULT_VALUE);

    auto floatNegVal = ArkUnion<displayCountArgT, checkT>(ArkValue<checkT>(ArkUnion<Ark_VP, Ark_Number>(-1.23f)));
    modifier_->setDisplayCount(node_, &floatNegVal, nullptr);
    auto checkFloatNeg = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkFloatNeg, DEFAULT_VALUE);

    auto strPxNeg = ArkUnion<displayCountArgT, checkT>(ArkValue<checkT>(ArkUnion<Ark_VP, Ark_String>("-4.5px")));
    modifier_->setDisplayCount(node_, &strPxNeg, nullptr);
    auto checkStrPxNeg = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkStrPxNeg, DEFAULT_VALUE);

    auto strVpNeg = ArkUnion<displayCountArgT, checkT>(ArkValue<checkT>(ArkUnion<Ark_VP, Ark_String>("-56vp")));
    modifier_->setDisplayCount(node_, &strVpNeg, nullptr);
    auto checkStrVpNeg = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkStrVpNeg, DEFAULT_VALUE);
}

/**
 * @tc.name: setDisplayCountTestByGroup
 * @tc.desc: Check the functionality of SwiperModifier.DisplayCountImpl with ByGroup optional
 * @tc.type: FUNC
 */
HWTEST_F(SwiperModifierTest, setDisplayCountTestByGroup, TestSize.Level1)
{
    static const std::string PROP_NAME("swipeByGroup");
    static const std::string DEFAULT_VALUE(EXPECTED_FALSE);
    ASSERT_NE(modifier_->setDisplayCount, nullptr);

    Ark_Union_Number_String_SwiperAutoFill aceFakeArg0;

    auto checkInitial = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkInitial, DEFAULT_VALUE);

    modifier_->setDisplayCount(node_, &aceFakeArg0, &OPT_BOOL_TRUE);
    auto checkVal2 = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkVal2, EXPECTED_TRUE);

    modifier_->setDisplayCount(node_, &aceFakeArg0, nullptr);
    auto checkVal2opt = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkVal2opt, EXPECTED_TRUE); // nothing change if no optional arg

    modifier_->setDisplayCount(node_, &aceFakeArg0, &OPT_BOOL_FALSE);
    auto checkVal3 = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkVal3, EXPECTED_FALSE);

    modifier_->setDisplayCount(node_, &aceFakeArg0, nullptr);
    auto checkVal3opt = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkVal3opt, EXPECTED_FALSE); // nothing change if no optional arg
}

/**
 * @tc.name: setEffectModeTest
 * @tc.desc: Check the functionality of SwiperModifier.EffectModeImpl
 * @tc.type: FUNC
 */
HWTEST_F(SwiperModifierTest, setEffectModeTest, TestSize.Level1)
{
    static const std::string PROP_NAME("effectMode");
    static const std::string DEFAULT_VALUE("EdgeEffect.Spring");
    auto checkInitial = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkInitial, DEFAULT_VALUE);

    auto arkValue0 = ARK_EDGE_EFFECT_SPRING;
    modifier_->setEffectMode(node_, arkValue0);
    auto checkVal2 = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkVal2, "EdgeEffect.Spring");

    auto arkValue1 = ARK_EDGE_EFFECT_FADE;
    modifier_->setEffectMode(node_, arkValue1);
    auto checkVal3 = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkVal3, "EdgeEffect.Fade");

    auto arkValue2 = static_cast<Ark_EdgeEffect>(INT_MAX);
    modifier_->setEffectMode(node_, arkValue2);
    auto checkVal4 = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkVal4, "EdgeEffect.Fade"); // nothing changes

    auto arkValue3 = static_cast<Ark_EdgeEffect>(INT_MIN);
    modifier_->setEffectMode(node_, arkValue3);
    auto checkVal5 = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkVal5, "EdgeEffect.Fade"); // nothing changes
}
/**
 * @tc.name: setDisableSwipeTest
 * @tc.desc: Check the functionality of SwiperModifier.DisableSwipeImpl
 * @tc.type: FUNC
 */
HWTEST_F(SwiperModifierTest, setDisableSwipeTest, TestSize.Level1)
{
    static const std::string PROP_NAME("disableSwipe");
    static const std::string DEFAULT_VALUE(EXPECTED_FALSE);
    ASSERT_NE(modifier_->setDisableSwipe, nullptr);

    auto checkInitial = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkInitial, DEFAULT_VALUE);

    modifier_->setDisableSwipe(node_, ABOOL_TRUE);
    auto checkVal2 = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkVal2, EXPECTED_TRUE);

    modifier_->setDisableSwipe(node_, ABOOL_FALSE);
    auto checkVal3 = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkVal3, EXPECTED_FALSE);
}
/**
 * @tc.name: setCurveTestBuiltIn
 * @tc.desc: Check the functionality of SwiperModifier.CurveImpl for built-in curves
 * @tc.type: FUNC
 */
HWTEST_F(SwiperModifierTest, setCurveTestBuiltIn, TestSize.Level1)
{
    static const std::string PROP_NAME("curve");
    static const std::string DEFAULT_VALUE(Curves::ToString(Curves::EASE_IN_OUT));
    ASSERT_NE(modifier_->setCurve, nullptr);

    auto checkInitial = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkInitial, Curves::DEFAULT_CURVE_NAME);

    auto arkCurveEasyIn =
        ArkUnion<Ark_Union_Curve_String_ICurve, Ark_Curve>(ARK_CURVE_EASE_IN);
    modifier_->setCurve(node_, &arkCurveEasyIn);
    auto checkEasyIO = GetAttrValue<std::string>(node_, PROP_NAME);
    auto expectedCurveEasyIn =
        Framework::CreateCurve(Framework::CurveIntToString(ArkUI_AnimationCurve::ARKUI_CURVE_EASE_IN));
    EXPECT_EQ(checkEasyIO, Curves::ToString(expectedCurveEasyIn));

    modifier_->setCurve(node_, nullptr);
    auto checkNull = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkNull, Curves::ToString(expectedCurveEasyIn));

    auto arkCurveLinear =
        ArkUnion<Ark_Union_Curve_String_ICurve, Ark_Curve>(ARK_CURVE_LINEAR);
    modifier_->setCurve(node_, &arkCurveLinear);
    auto checkLinear = GetAttrValue<std::string>(node_, PROP_NAME);
    auto expectedCurveLinear =
        Framework::CreateCurve(Framework::CurveIntToString(ArkUI_AnimationCurve::ARKUI_CURVE_LINEAR));
    EXPECT_EQ(checkLinear, Curves::ToString(expectedCurveLinear));

    auto arkCurveInv = ArkUnion<Ark_Union_Curve_String_ICurve, Ark_Curve>(static_cast<Ark_Curve>(INT_MIN));
    modifier_->setCurve(node_, &arkCurveInv);
    auto checkInv = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkInv, DEFAULT_VALUE);
}
/**
 * @tc.name: setCurveTestCustom
 * @tc.desc: Check the functionality of SwiperModifier.CurveImpl for Custom curves
 * @tc.type: FUNC
 */
HWTEST_F(SwiperModifierTest, setCurveTestCustom, TestSize.Level1)
{
    static const std::string PROP_NAME("curve");
    static const std::string DEFAULT_VALUE(Curves::ToString(Curves::EASE_IN_OUT));
    ASSERT_NE(modifier_->setCurve, nullptr);

    auto checkInitial = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkInitial, Curves::DEFAULT_CURVE_NAME);

    auto arkCurveCustom = ArkUnion<Ark_Union_Curve_String_ICurve, Ark_String>("interpolating-spring(1, 1, 28, 34)");
    modifier_->setCurve(node_, &arkCurveCustom);
    auto checkCustStr = GetAttrValue<std::string>(node_, PROP_NAME);
    // this can't be exactly check due to SwiperPaintProperty::ToJsonValue supports the built-in Curves only
    EXPECT_NE(checkCustStr, DEFAULT_VALUE);

    auto arkCurveInv = ArkUnion<Ark_Union_Curve_String_ICurve, Ark_String>("invalidCurveDefinition");
    modifier_->setCurve(node_, &arkCurveInv);
    auto checkInv = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkInv, DEFAULT_VALUE);
}

/**
 * @tc.name: setOnChangeTest
 * @tc.desc: Check the functionality of SwiperModifier.OnChangeImpl
 * @tc.type: FUNC
 */
HWTEST_F(SwiperModifierTest, setOnChangeTest, TestSize.Level1)
{
    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    ASSERT_NE(frameNode, nullptr);
    auto eventHub = frameNode->GetEventHub<NG::SwiperEventHub>();
    ASSERT_NE(eventHub, nullptr);

    struct CheckEvent {
        int32_t nodeId;
        int32_t index;
    };
    static std::optional<CheckEvent> checkEvent = std::nullopt;
    void (*checkCallback)(const Ark_Int32, const Ark_Number) =
        [](const Ark_Int32 resourceId, const Ark_Number index) {
            checkEvent = {
                .nodeId = resourceId,
                .index = Converter::Convert<Ark_Int32>(index)
            };
        };
    auto func = Converter::ArkValue<Callback_Number_Void>(checkCallback, CONTEXT_ID);

    ASSERT_NE(modifier_->setOnChange, nullptr);

    modifier_->setOnChange(node_, &func);

    EXPECT_EQ(checkEvent.has_value(), false);
    eventHub->FireChangeEvent(0, 321, false);
    ASSERT_TRUE(checkEvent.has_value());
    EXPECT_EQ(checkEvent->nodeId, CONTEXT_ID);
    EXPECT_EQ(checkEvent->index, 321);
}
/**
 * @tc.name: setIndicatorStyleTest
 * @tc.desc: Check the functionality of SwiperModifier.IndicatorStyleImpl
 * @tc.type: FUNC
 */
HWTEST_F(SwiperModifierTest, setIndicatorStyleTest, TestSize.Level1)
{
    static const std::string PROP_NAME("indicator");
    ASSERT_NE(modifier_->setIndicatorStyle, nullptr);
    ASSERT_NE(modifier_->setIndicator, nullptr);

    auto checkValInitial = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkValInitial, EXPECTED_TRUE);

    Opt_IndicatorStyle style = Converter::ArkValue<Opt_IndicatorStyle>(Ark_IndicatorStyle{
        .left = OPT_LEN_VP_NEG,
        .top = OPT_LEN_VP_NEG,
        .right = OPT_LEN_VP_POS,
        .bottom = OPT_LEN_VP_POS,
        .size = OPT_LEN_VP_POS,
        .mask = OPT_BOOL_TRUE,
        .color = ArkUnion<Opt_ResourceColor, Ark_Number>(0x12345678),
        .selectedColor = ArkUnion<Opt_ResourceColor, Ark_String>("65535"),
    });
    modifier_->setIndicatorStyle(node_, &style);
    auto strWithObj = GetAttrValue<std::string>(node_, PROP_NAME);
    ASSERT_NE(strWithObj, EXPECTED_TRUE);
    ASSERT_NE(strWithObj, EXPECTED_FALSE);

    auto checkL = GetAttrValue<std::string>(strWithObj, "left");
    EXPECT_EQ(checkL, "0.00vp");
    auto checkT = GetAttrValue<std::string>(strWithObj, "top");
    EXPECT_EQ(checkT, "0.00vp");
    auto checkR = GetAttrValue<std::string>(strWithObj, "right");
    EXPECT_EQ(checkR, "1.23vp");
    auto checkB = GetAttrValue<std::string>(strWithObj, "bottom");
    EXPECT_EQ(checkB, "1.23vp");

    auto checkW = GetAttrValue<std::string>(strWithObj, "itemWidth");
    EXPECT_EQ(checkW, "1.23vp");
    auto checkH = GetAttrValue<std::string>(strWithObj, "itemHeight");
    EXPECT_EQ(checkH, "1.23vp");
    auto checkSelW = GetAttrValue<std::string>(strWithObj, "selectedItemWidth");
    EXPECT_EQ(checkSelW, "1.23vp");
    auto checkSelH = GetAttrValue<std::string>(strWithObj, "selectedItemHeight");
    EXPECT_EQ(checkSelH, "1.23vp");

    auto checkMask = GetAttrValue<std::string>(strWithObj, "mask");
    EXPECT_EQ(checkMask, EXPECTED_TRUE);

    auto checkColor = GetAttrValue<std::string>(strWithObj, "color");
    EXPECT_EQ(checkColor, "#12345678");
    auto checkSelColor = GetAttrValue<std::string>(strWithObj, "selectedColor");
    EXPECT_EQ(checkSelColor, "#FF00FFFF");
}

/**
 * @tc.name: setIndicatorStyleTestInvalid
 * @tc.desc: Check the functionality of SwiperModifier.IndicatorStyleImpl with undefined param
 * @tc.type: FUNC
 */
HWTEST_F(SwiperModifierTest, setIndicatorStyleTestInvalid, TestSize.Level1)
{
    static const std::string PROP_NAME("indicator");
    static const std::string DEFAULT_PADDING("0.00vp");
    static const std::string DEFAULT_SIZE(THEME_SWIPER_INDICATOR_SIZE.ToString());
    ASSERT_NE(modifier_->setIndicatorStyle, nullptr);
    ASSERT_NE(modifier_->setIndicator, nullptr);

    auto checkInitial = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkInitial, EXPECTED_TRUE);

    Opt_IndicatorStyle styleInvalid = Converter::ArkValue<Opt_IndicatorStyle>();
    modifier_->setIndicatorStyle(node_, &styleInvalid);
    auto strWithObj = GetAttrValue<std::string>(node_, PROP_NAME);
    ASSERT_NE(strWithObj, EXPECTED_TRUE);
    ASSERT_NE(strWithObj, EXPECTED_FALSE);

    auto checkL = GetAttrValue<std::string>(strWithObj, "left");
    EXPECT_EQ(checkL, DEFAULT_PADDING);
    auto checkT = GetAttrValue<std::string>(strWithObj, "top");
    EXPECT_EQ(checkT, DEFAULT_PADDING);
    auto checkR = GetAttrValue<std::string>(strWithObj, "right");
    EXPECT_EQ(checkR, DEFAULT_PADDING);
    auto checkB = GetAttrValue<std::string>(strWithObj, "bottom");
    EXPECT_EQ(checkB, DEFAULT_PADDING);

    auto checkW = GetAttrValue<std::string>(strWithObj, "itemWidth");
    EXPECT_EQ(checkW, DEFAULT_SIZE);
    auto checkH = GetAttrValue<std::string>(strWithObj, "itemHeight");
    EXPECT_EQ(checkH, DEFAULT_SIZE);
    auto checkSelW = GetAttrValue<std::string>(strWithObj, "selectedItemWidth");
    EXPECT_EQ(checkSelW, DEFAULT_SIZE);
    auto checkSelH = GetAttrValue<std::string>(strWithObj, "selectedItemHeight");
    EXPECT_EQ(checkSelH, DEFAULT_SIZE);

    auto checkMask = GetAttrValue<std::string>(strWithObj, "mask");
    EXPECT_EQ(checkMask, EXPECTED_FALSE);

    auto checkColor = GetAttrValue<std::string>(strWithObj, "color");
    EXPECT_EQ(checkColor, THEME_SWIPER_INDICATOR_COLOR.ToString());
    auto checkSelColor = GetAttrValue<std::string>(strWithObj, "selectedColor");
    EXPECT_EQ(checkSelColor, THEME_SWIPER_INDICATOR_COLOR.ToString());
}

/**
 * @tc.name: setPrevMarginTest
 * @tc.desc: Check the functionality of SwiperModifier.PrevMarginImpl
 * @tc.type: FUNC
 */
HWTEST_F(SwiperModifierTest, setPrevMarginTest, TestSize.Level1)
{
    static const std::string PROP_NAME("prevMargin");
    static const std::string PROP_NAME_OPT("prevMarginIgnoreBlank");
    static const std::string &DEFAULT_VALUE(EXPECTED_VP_ZERO);
    ASSERT_NE(modifier_->setPrevMargin, nullptr);

    // test init state
    auto checkInit = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkInit, DEFAULT_VALUE);
    auto checkInitOpt = GetAttrValue<std::string>(node_, PROP_NAME_OPT);
    EXPECT_EQ(checkInitOpt, EXPECTED_FALSE);

    // test the main arg
    modifier_->setPrevMargin(node_, &ALEN_VP_POS, nullptr);
    auto checkPxPos = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkPxPos, EXPECTED_VP_POS);
    modifier_->setPrevMargin(node_, &ALEN_VP_NEG, nullptr);
    auto checkVpNeg = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkVpNeg, DEFAULT_VALUE);

    // test the optional arg
    modifier_->setPrevMargin(node_, &ALEN_VP_POS, &OPT_BOOL_TRUE);
    auto checkOptTrue = GetAttrValue<std::string>(node_, PROP_NAME_OPT);
    EXPECT_EQ(checkOptTrue, EXPECTED_TRUE);
    modifier_->setPrevMargin(node_, &ALEN_VP_POS, &OPT_BOOL_UNDEF);
    auto checkOptUndef = GetAttrValue<std::string>(node_, PROP_NAME_OPT);
    EXPECT_EQ(checkOptUndef, EXPECTED_TRUE); // nothing change with the undefined optional param
    modifier_->setPrevMargin(node_, &ALEN_VP_POS, nullptr);
    auto checkOptNull = GetAttrValue<std::string>(node_, PROP_NAME_OPT);
    EXPECT_EQ(checkOptNull, EXPECTED_TRUE); // nothing change with no optional param
    modifier_->setPrevMargin(node_, &ALEN_VP_POS, &OPT_BOOL_FALSE);
    auto checkOptFalse = GetAttrValue<std::string>(node_, PROP_NAME_OPT);
    EXPECT_EQ(checkOptFalse, EXPECTED_FALSE);
}
/**
 * @tc.name: setNextMarginTest
 * @tc.desc: Check the functionality of SwiperModifier.NextMarginImpl
 * @tc.type: FUNC
 */
HWTEST_F(SwiperModifierTest, setNextMarginTest, TestSize.Level1)
{
    static const std::string PROP_NAME("nextMargin");
    static const std::string PROP_NAME_OPT("nextMarginIgnoreBlank");
    static const std::string &DEFAULT_VALUE(EXPECTED_VP_ZERO);
    ASSERT_NE(modifier_->setNextMargin, nullptr);

    // test init state
    auto checkInit = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkInit, DEFAULT_VALUE);
    auto checkInitOpt = GetAttrValue<std::string>(node_, PROP_NAME_OPT);
    EXPECT_EQ(checkInitOpt, EXPECTED_FALSE);

    // test the main arg
    modifier_->setNextMargin(node_, &ALEN_VP_POS, nullptr);
    auto checkPxPos = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkPxPos, EXPECTED_VP_POS);
    modifier_->setNextMargin(node_, &ALEN_VP_NEG, nullptr);
    auto checkVpNeg = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkVpNeg, DEFAULT_VALUE);

    // test the optional arg
    modifier_->setNextMargin(node_, &ALEN_VP_POS, &OPT_BOOL_TRUE);
    auto checkOptTrue = GetAttrValue<std::string>(node_, PROP_NAME_OPT);
    EXPECT_EQ(checkOptTrue, EXPECTED_TRUE);
    modifier_->setNextMargin(node_, &ALEN_VP_POS, &OPT_BOOL_UNDEF);
    auto checkOptUndef = GetAttrValue<std::string>(node_, PROP_NAME_OPT);
    EXPECT_EQ(checkOptUndef, EXPECTED_TRUE); // nothing change with the undefined optional param
    modifier_->setNextMargin(node_, &ALEN_VP_POS, nullptr);
    auto checkOptNull = GetAttrValue<std::string>(node_, PROP_NAME_OPT);
    EXPECT_EQ(checkOptNull, EXPECTED_TRUE); // nothing change with no optional param
    modifier_->setNextMargin(node_, &ALEN_VP_POS, &OPT_BOOL_FALSE);
    auto checkOptFalse = GetAttrValue<std::string>(node_, PROP_NAME_OPT);
    EXPECT_EQ(checkOptFalse, EXPECTED_FALSE);
}
/**
 * @tc.name: setOnAnimationStartTest
 * @tc.desc: Check the functionality of SwiperModifier.OnAnimationStartImpl
 * @tc.type: FUNC
 */
HWTEST_F(SwiperModifierTest, setOnAnimationStartTest, TestSize.Level1)
{
    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    ASSERT_NE(frameNode, nullptr);
    auto eventHub = frameNode->GetEventHub<NG::SwiperEventHub>();
    ASSERT_NE(eventHub, nullptr);

    struct CheckEvent {
        int32_t nodeId;
        int32_t index;
        int32_t targetIndex;
        AnimationCallbackInfo info;
    };
    static std::optional<CheckEvent> checkEvent = std::nullopt;
    void (*checkCallback)(const Ark_Int32, const Ark_Number, const Ark_Number, const Ark_SwiperAnimationEvent) =
        [](const Ark_Int32 resourceId, const Ark_Number index, const Ark_Number targetIndex,
        const Ark_SwiperAnimationEvent extraInfo) {
            checkEvent = {
                .nodeId = resourceId,
                .index = Converter::Convert<Ark_Int32>(index),
                .targetIndex = Converter::Convert<Ark_Int32>(targetIndex),
                .info = {
                    .currentOffset = Converter::Convert<Ark_Float32>(extraInfo.currentOffset),
                    .targetOffset = Converter::Convert<Ark_Float32>(extraInfo.targetOffset),
                    .velocity = Converter::Convert<Ark_Float32>(extraInfo.velocity),
                }
            };
        };
    auto func = Converter::ArkValue<OnSwiperAnimationStartCallback>(checkCallback, CONTEXT_ID);

    ASSERT_NE(modifier_->setOnAnimationStart, nullptr);

    modifier_->setOnAnimationStart(node_, &func);

    EXPECT_EQ(checkEvent.has_value(), false);
    eventHub->FireAnimationStartEvent(123, 456, {
        .currentOffset = 1.23f,
        .targetOffset = -4.56f,
        .velocity = 78.9f,
    });
    ASSERT_EQ(checkEvent.has_value(), true);
    EXPECT_EQ(checkEvent->nodeId, CONTEXT_ID);
    EXPECT_EQ(checkEvent->index, 123);
    EXPECT_EQ(checkEvent->targetIndex, 456);
    EXPECT_EQ(checkEvent->info.currentOffset, 1.23f);
    EXPECT_EQ(checkEvent->info.targetOffset, -4.56f);
    EXPECT_EQ(checkEvent->info.velocity, 78.9f);
}
/**
 * @tc.name: setOnAnimationEndTest
 * @tc.desc: Check the functionality of SwiperModifier.OnAnimationEndImpl
 * @tc.type: FUNC
 */
HWTEST_F(SwiperModifierTest, setOnAnimationEndTest, TestSize.Level1)
{
    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    ASSERT_NE(frameNode, nullptr);
    auto eventHub = frameNode->GetEventHub<NG::SwiperEventHub>();
    ASSERT_NE(eventHub, nullptr);

    struct CheckEvent {
        int32_t nodeId;
        int32_t index;
        AnimationCallbackInfo info;
    };
    static std::optional<CheckEvent> checkEvent = std::nullopt;
    void (*checkCallback)(const Ark_Int32, const Ark_Number, const Ark_SwiperAnimationEvent) =
        [](const Ark_Int32 resourceId, const Ark_Number index, const Ark_SwiperAnimationEvent extraInfo) {
            checkEvent = {
            .nodeId = resourceId,
            .index = Converter::Convert<Ark_Int32>(index),
            .info = {
                .currentOffset = Converter::Convert<Ark_Float32>(extraInfo.currentOffset),
                .targetOffset = Converter::Convert<Ark_Float32>(extraInfo.targetOffset),
                .velocity = Converter::Convert<Ark_Float32>(extraInfo.velocity),
            }
        };
        };
    auto func = Converter::ArkValue<OnSwiperAnimationEndCallback>(checkCallback, CONTEXT_ID);

    // the start animation before is required for the end of animation testing
    eventHub->FireAnimationStartEvent(0, 0, {});

    ASSERT_NE(modifier_->setOnAnimationEnd, nullptr);

    modifier_->setOnAnimationEnd(node_, &func);

    EXPECT_EQ(checkEvent.has_value(), false);
    eventHub->FireAnimationEndEvent(321, {
        .currentOffset = 1.23f,
        .targetOffset = -4.56f,
        .velocity = 78.9f,
    });
    ASSERT_EQ(checkEvent.has_value(), true);
    EXPECT_EQ(checkEvent->nodeId, CONTEXT_ID);
    EXPECT_EQ(checkEvent->index, 321);
    EXPECT_EQ(checkEvent->info.currentOffset, 1.23f);
    EXPECT_EQ(checkEvent->info.targetOffset, -4.56f);
    EXPECT_EQ(checkEvent->info.velocity, 78.9f);
}
/**
 * @tc.name: setOnGestureSwipeTest
 * @tc.desc: Check the functionality of SwiperModifier.OnGestureSwipeImpl
 * @tc.type: FUNC
 */
HWTEST_F(SwiperModifierTest, setOnGestureSwipeTest, TestSize.Level1)
{
    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    ASSERT_NE(frameNode, nullptr);
    auto eventHub = frameNode->GetEventHub<NG::SwiperEventHub>();
    ASSERT_NE(eventHub, nullptr);

    struct CheckEvent {
        int32_t nodeId;
        int32_t index;
        AnimationCallbackInfo info;
    };
    static std::optional<CheckEvent> checkEvent = std::nullopt;
    void (*checkCallback)(const Ark_Int32, const Ark_Number, const Ark_SwiperAnimationEvent) =
        [](const Ark_Int32 resourceId, const Ark_Number index, const Ark_SwiperAnimationEvent extraInfo) {
            checkEvent = {
                .nodeId = resourceId,
                .index = Converter::Convert<Ark_Int32>(index),
                .info = {
                    .currentOffset = Converter::Convert<Ark_Float32>(extraInfo.currentOffset),
                    .targetOffset = Converter::Convert<Ark_Float32>(extraInfo.targetOffset),
                    .velocity = Converter::Convert<Ark_Float32>(extraInfo.velocity),
                }
            };
        };
    auto func = Converter::ArkValue<OnSwiperGestureSwipeCallback>(checkCallback, CONTEXT_ID);

    ASSERT_NE(modifier_->setOnGestureSwipe, nullptr);

    modifier_->setOnGestureSwipe(node_, &func);

    EXPECT_EQ(checkEvent.has_value(), false);
    eventHub->FireGestureSwipeEvent(321, {
        .currentOffset = 1.23f,
        .targetOffset = -4.56f,
        .velocity = 78.9f,
    });
    ASSERT_EQ(checkEvent.has_value(), true);
    EXPECT_EQ(checkEvent->nodeId, CONTEXT_ID);
    EXPECT_EQ(checkEvent->index, 321);
    EXPECT_EQ(checkEvent->info.currentOffset, 1.23f);
    EXPECT_EQ(checkEvent->info.targetOffset, -4.56f);
    EXPECT_EQ(checkEvent->info.velocity, 78.9f);
}
/**
 * @tc.name: setNestedScrollTest
 * @tc.desc: Check the functionality of SwiperModifier.NestedScrollImpl
 * @tc.type: FUNC
 */
HWTEST_F(SwiperModifierTest, DISABLED_setNestedScrollTest, TestSize.Level1)
{
    ASSERT_NE(modifier_->setNestedScroll, nullptr);
    // the result can't be checked due to nothing corresponded attributes in JSON object,
    // that is provided by the SwiperPattern::ToJsonValue and SwiperLayoutProperty::ToJsonValue
}
/**
 * @tc.name: setCustomContentTransition
 * @tc.desc: Check the functionality of SwiperModifier.CustomContentTransitionImpl
 * @tc.type: FUNC
 */
HWTEST_F(SwiperModifierTest, setCustomContentTransition, TestSize.Level1)
{
    ASSERT_NE(modifier_->setCustomContentTransition, nullptr);

    const int32_t TIMEOUT = 1000;
    const int32_t CONTEXT_ID = 123;
    const int32_t EXPECTED_INDEX_VALUE = 2342;

    ASSERT_NE(fullAPI_->getAccessors(), nullptr);
    static const auto *accessor = fullAPI_->getAccessors()->getSwiperContentTransitionProxyAccessor();
    ASSERT_NE(accessor, nullptr);
    ASSERT_NE(accessor->getFinalizer, nullptr);
    ASSERT_NE(accessor->getIndex, nullptr);

    static std::optional<std::pair<int32_t, int32_t>> checkInvoke;
    void (*fakeDeveloperCallbackFunc)(const Ark_Int32 resourceId, const Ark_Materialized parameter) =
        [](const Ark_Int32 resourceId, const Ark_Materialized parameter) {
            auto peer = reinterpret_cast<SwiperContentTransitionProxyPeer*>(parameter.ptr);
            // get to further test: incoming resource, data from incoming peer via accessor
            checkInvoke = { resourceId, accessor->getIndex(peer) };

            auto destroy = reinterpret_cast<void (*)(SwiperContentTransitionProxyPeer *)>(accessor->getFinalizer());
            if (destroy) {
                (*destroy)(peer);
            }
        };
    ASSERT_FALSE(checkInvoke.has_value());

    // setup the callback object via C-API
    Ark_SwiperContentAnimatedTransition transition {
        .timeout = ArkValue<Opt_Number>(TIMEOUT),
        .transition = ArkValue<Callback_SwiperContentTransitionProxy_Void>(fakeDeveloperCallbackFunc, CONTEXT_ID)
    };
    modifier_->setCustomContentTransition(node_, &transition);

    // check the callback object that was setup
    auto frameNode = reinterpret_cast<FrameNode *>(node_);
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<SwiperPattern>();
    ASSERT_NE(pattern, nullptr);
    auto swiperContentAnimatedTransition = pattern->GetSwiperCustomContentTransition();
    ASSERT_NE(swiperContentAnimatedTransition, nullptr);
    EXPECT_EQ(swiperContentAnimatedTransition->timeout, TIMEOUT);
    ASSERT_NE(swiperContentAnimatedTransition->transition, nullptr);

    // simulate of the callback function invoking from ace_engine part
    auto swiperContentTransitionProxy = AceType::MakeRefPtr<SwiperContentTransitionProxy>();
    swiperContentTransitionProxy->SetIndex(EXPECTED_INDEX_VALUE);
    swiperContentAnimatedTransition->transition(swiperContentTransitionProxy);

    // check the invoking result
    ASSERT_TRUE(checkInvoke.has_value());
    EXPECT_EQ(checkInvoke.value().first, CONTEXT_ID);
    EXPECT_EQ(checkInvoke.value().second, EXPECTED_INDEX_VALUE);
}
/**
 * @tc.name: setOnContentDidScrollTest
 * @tc.desc: Check the functionality of SwiperModifier.OnContentDidScrollImpl
 * @tc.type: FUNC
 */
HWTEST_F(SwiperModifierTest, setOnContentDidScrollTest, TestSize.Level1)
{
    struct OnDidScrollParams {
        int32_t nodeId;
        int32_t selectedIndex;
        int32_t index;
        float position;
        float mainAxisLength;
    };

    struct OnDidScrollParams expected {
        reinterpret_cast<UINode *>(node_)->GetId(),
        1122,
        3344,
        55.66f,
        77.88f,
    };

    static std::optional<OnDidScrollParams> checkEvent = std::nullopt;
    void (*checkCallback)(const Ark_Int32, Ark_Number, Ark_Number, Ark_Number, Ark_Number) =
        [](const Ark_Int32 resourceId, Ark_Number selectedIndex,
            Ark_Number index, Ark_Number position, Ark_Number mainAxisLength) {
            checkEvent = {
                .nodeId = resourceId,
                .selectedIndex = Converter::Convert<int32_t>(selectedIndex),
                .index = Converter::Convert<int32_t>(index),
                .position = Converter::Convert<float>(position),
                .mainAxisLength = Converter::Convert<float>(mainAxisLength),
            };
        };
    auto func = Converter::ArkValue<ContentDidScrollCallback>(checkCallback, CONTEXT_ID);

    EXPECT_NE(modifier_->setOnContentDidScroll, nullptr);
    EXPECT_FALSE(checkEvent);

    modifier_->setOnContentDidScroll(node_, &func);

    // check the callback func that was setup
    auto frameNode = reinterpret_cast<FrameNode *>(node_);
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<SwiperPattern>();
    ASSERT_NE(pattern, nullptr);
    auto onContentDidScroll = pattern->GetOnContentDidScroll();
    ASSERT_NE(onContentDidScroll, nullptr);

    // simulate the callback invoking from ace_engine
    (*onContentDidScroll.get())(expected.selectedIndex, expected.index, expected.position, expected.mainAxisLength);
    ASSERT_TRUE(checkEvent);
    EXPECT_EQ(checkEvent->nodeId, CONTEXT_ID);
    EXPECT_EQ(checkEvent->selectedIndex, expected.selectedIndex);
    EXPECT_EQ(checkEvent->index, expected.index);
    EXPECT_EQ(checkEvent->position, expected.position);
    EXPECT_EQ(checkEvent->mainAxisLength, expected.mainAxisLength);
}
/**
 * @tc.name: setIndicatorInteractiveTest
 * @tc.desc: Check the functionality of SwiperModifier.IndicatorInteractiveImpl
 * @tc.type: FUNC
 */
HWTEST_F(SwiperModifierTest, setIndicatorInteractiveTest, TestSize.Level1)
{
    static const std::string PROP_NAME("indicatorInteractive");
    static const std::string &DEFAULT_VALUE(EXPECTED_TRUE);
    ASSERT_NE(modifier_->setIndicatorInteractive, nullptr);

    auto checkInitial = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkInitial, DEFAULT_VALUE);

    modifier_->setIndicatorInteractive(node_, ABOOL_FALSE);
    auto checkFalse = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkFalse, EXPECTED_FALSE);

    modifier_->setIndicatorInteractive(node_, ABOOL_TRUE);
    auto checkTrue = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkTrue, EXPECTED_TRUE);
}

/*
 * @tc.name: setOnChangeEventIndexImpl
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(SwiperModifierTest, setOnChangeEventIndexImpl, TestSize.Level1)
{
    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    auto eventHub = frameNode->GetEventHub<SwiperEventHub>();
    ASSERT_NE(eventHub, nullptr);

    struct CheckEvent {
        int32_t nodeId;
        int32_t value;
    };
    static std::optional<CheckEvent> checkEvent = std::nullopt;
    static constexpr int32_t contextId = 123;

    auto checkCallback = [](const Ark_Int32 resourceId, const Ark_Number parameter) {
        checkEvent = {
            .nodeId = resourceId,
            .value = Converter::Convert<int32_t>(parameter)
        };
    };

    Callback_Number_Void arkCallback = Converter::ArkValue<Callback_Number_Void>(checkCallback, contextId);

    modifier_->set__onChangeEvent_index(node_, &arkCallback);

    ASSERT_EQ(checkEvent.has_value(), false);
    eventHub->FireChangeEvent(0, 1, false);
    ASSERT_EQ(checkEvent.has_value(), true);
    EXPECT_EQ(checkEvent->nodeId, contextId);
    EXPECT_EQ(checkEvent->value, 1);
    eventHub->FireChangeEvent(1, 2, false);
    ASSERT_EQ(checkEvent.has_value(), true);
    EXPECT_EQ(checkEvent->nodeId, contextId);
    EXPECT_EQ(checkEvent->value, 2);
}
} // namespace OHOS::Ace::NG

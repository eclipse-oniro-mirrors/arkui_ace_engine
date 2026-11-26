/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include <memory>

#include "gtest/gtest.h"
#include <cmath>
#include <limits>
#include <optional>

#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"
#include "core/components/common/properties/text_style.h"
#include "base/geometry/dimension.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace {
namespace {
const double DEFAULT_DOUBLE = 1.0;
const double ZERO_DOUBLE = 0.0;

const Dimension DIMENSION_PX(DEFAULT_DOUBLE, DimensionUnit::PX);
const Dimension DIMENSION_VP(DEFAULT_DOUBLE, DimensionUnit::VP);
const Dimension DIMENSION_FP(DEFAULT_DOUBLE, DimensionUnit::FP);
const Dimension DIMENSION_LPX(DEFAULT_DOUBLE, DimensionUnit::LPX);
const Dimension DIMENSION_PCT(DEFAULT_DOUBLE, DimensionUnit::PERCENT);
const Dimension DIMENSION_AUTO(DEFAULT_DOUBLE, DimensionUnit::AUTO);
const Dimension DIMENSION_CALC(DEFAULT_DOUBLE, DimensionUnit::CALC);
const Dimension DIMENSION_NONE(DEFAULT_DOUBLE, DimensionUnit::NONE);

const std::string DIMENSION_PX_STR = StringUtils::DoubleToString(DEFAULT_DOUBLE).append("px");
const std::string DIMENSION_VP_STR = StringUtils::DoubleToString(DEFAULT_DOUBLE).append("vp");
const std::string DIMENSION_FP_STR = StringUtils::DoubleToString(DEFAULT_DOUBLE).append("fp");
const std::string DIMENSION_LPX_STR = StringUtils::DoubleToString(DEFAULT_DOUBLE).append("lpx");
const std::string DIMENSION_PCT_STR = StringUtils::DoubleToString(DEFAULT_DOUBLE * 100).append("%");
const std::string DIMENSION_AUTO_STR = StringUtils::DoubleToString(DEFAULT_DOUBLE).append("auto");
const std::string DIMENSION_NONE_STR = StringUtils::DoubleToString(DEFAULT_DOUBLE).append("none");
} // namespace

class DimensionTest : public testing::Test {
public:
    static void SetUpTestSuite()
    {
        NG::MockPipelineContext::SetUp();
    }
    static void TearDownTestSuite()
    {
        NG::MockPipelineContext::TearDown();
    }
};

/**
 * @tc.name: DimensionTest001
 * @tc.desc: Test the function ConvertToVp of the class Dimension.
 * @tc.type: FUNC
 */
HWTEST_F(DimensionTest, DimensionTest001, TestSize.Level1)
{
    /**
     * @tc.steps1: Test the function ConvertToVp of the class Dimension.
     * @tc.expected: The return values are equal to DEFAULT_DOUBLE or ZERO_DOUBLE
     */
    EXPECT_DOUBLE_EQ(DIMENSION_PX.ConvertToVp(), DEFAULT_DOUBLE);
    EXPECT_DOUBLE_EQ(DIMENSION_VP.ConvertToVp(), DEFAULT_DOUBLE);
    EXPECT_DOUBLE_EQ(DIMENSION_FP.ConvertToVp(), DEFAULT_DOUBLE);
    EXPECT_DOUBLE_EQ(DIMENSION_LPX.ConvertToVp(), DEFAULT_DOUBLE);
    EXPECT_DOUBLE_EQ(DIMENSION_PCT.ConvertToVp(), ZERO_DOUBLE);
    EXPECT_DOUBLE_EQ(DIMENSION_AUTO.ConvertToVp(), ZERO_DOUBLE);
    EXPECT_DOUBLE_EQ(DIMENSION_CALC.ConvertToVp(), ZERO_DOUBLE);
    EXPECT_DOUBLE_EQ(DIMENSION_NONE.ConvertToVp(), DEFAULT_DOUBLE);
}

/**
 * @tc.name: DimensionTest002
 * @tc.desc: Test the function ConvertToPx of the class Dimension.
 * @tc.type: FUNC
 */
HWTEST_F(DimensionTest, DimensionTest002, TestSize.Level1)
{
    /**
     * @tc.steps1: Test the function ConvertToPx of the class Dimension.
     * @tc.expected: The return values are equal to DEFAULT_DOUBLE or ZERO_DOUBLE.
     */
    EXPECT_DOUBLE_EQ(DIMENSION_PX.ConvertToPx(), DEFAULT_DOUBLE);
    EXPECT_DOUBLE_EQ(DIMENSION_VP.ConvertToPx(), DEFAULT_DOUBLE);
    EXPECT_DOUBLE_EQ(DIMENSION_FP.ConvertToPx(), DEFAULT_DOUBLE);
    EXPECT_DOUBLE_EQ(DIMENSION_LPX.ConvertToPx(), DEFAULT_DOUBLE);
    EXPECT_DOUBLE_EQ(DIMENSION_PCT.ConvertToPx(), ZERO_DOUBLE);
    EXPECT_DOUBLE_EQ(DIMENSION_AUTO.ConvertToPx(), ZERO_DOUBLE);
    EXPECT_DOUBLE_EQ(DIMENSION_CALC.ConvertToPx(), ZERO_DOUBLE);
    EXPECT_DOUBLE_EQ(DIMENSION_NONE.ConvertToPx(), DEFAULT_DOUBLE);
}

/**
 * @tc.name: DimensionTest003
 * @tc.desc: Test the function ToString of the class Dimension.
 * @tc.type: FUNC
 */
HWTEST_F(DimensionTest, DimensionTest003, TestSize.Level1)
{
    /**
     * @tc.steps1: Test the function ToString of the class Dimension.
     * @tc.expected: The return values are equal to DIMENSION_STR of PX, VP, FP, LPX, PCT and AUTO.
     */
    EXPECT_EQ(DIMENSION_PX.ToString(), DIMENSION_PX_STR);
    EXPECT_EQ(DIMENSION_VP.ToString(), DIMENSION_VP_STR);
    EXPECT_EQ(DIMENSION_FP.ToString(), DIMENSION_FP_STR);
    EXPECT_EQ(DIMENSION_LPX.ToString(), DIMENSION_LPX_STR);
    EXPECT_EQ(DIMENSION_PCT.ToString(), DIMENSION_PCT_STR);
    EXPECT_EQ(DIMENSION_AUTO.ToString(), DIMENSION_AUTO_STR);
    Dimension dimension(DEFAULT_DOUBLE);
    dimension.SetUnit(static_cast<DimensionUnit>(7));
    EXPECT_EQ(dimension.ToString(), DIMENSION_PX_STR);
    dimension.SetUnit(DimensionUnit::NONE);
    EXPECT_EQ(dimension.ToString(), DIMENSION_NONE_STR);
}

/**
 * @tc.name: DimensionTest004
 * @tc.desc: Test the function NormalizeToPx of the class Dimension with -DEFAULT_DOUBLE.
 * @tc.type: FUNC
 */
HWTEST_F(DimensionTest, DimensionTest004, TestSize.Level1)
{
    /**
     * @tc.steps1: Test the function NormalizeToPx of the class Dimension with DIMENSION_PX.
     * @tc.expected: The return values are equal to DEFAULT_DOUBLE.
     */
    double result = 0;
    EXPECT_TRUE(DIMENSION_PX.NormalizeToPx(-DEFAULT_DOUBLE, -DEFAULT_DOUBLE, -DEFAULT_DOUBLE, -DEFAULT_DOUBLE, result));
    EXPECT_DOUBLE_EQ(result, DEFAULT_DOUBLE);

    /**
     * @tc.steps2: Test the function NormalizeToPx of the class Dimension with DIMENSION_VP.
     * @tc.expected: The return values are equal to ZERO_DOUBLE.
     */
    result = 0;
    EXPECT_FALSE(
        DIMENSION_VP.NormalizeToPx(-DEFAULT_DOUBLE, -DEFAULT_DOUBLE, -DEFAULT_DOUBLE, -DEFAULT_DOUBLE, result));
    EXPECT_DOUBLE_EQ(result, ZERO_DOUBLE);

    /**
     * @tc.steps3: Test the function NormalizeToPx of the class Dimension with DIMENSION_FP.
     * @tc.expected: The return values are equal to ZERO_DOUBLE.
     */
    result = 0;
    EXPECT_FALSE(
        DIMENSION_FP.NormalizeToPx(-DEFAULT_DOUBLE, -DEFAULT_DOUBLE, -DEFAULT_DOUBLE, -DEFAULT_DOUBLE, result));
    EXPECT_DOUBLE_EQ(result, ZERO_DOUBLE);

    /**
     * @tc.steps4: Test the function NormalizeToPx of the class Dimension with DIMENSION_LPX.
     * @tc.expected: The return values are equal to ZERO_DOUBLE.
     */
    result = 0;
    EXPECT_FALSE(
        DIMENSION_LPX.NormalizeToPx(-DEFAULT_DOUBLE, -DEFAULT_DOUBLE, -DEFAULT_DOUBLE, -DEFAULT_DOUBLE, result));
    EXPECT_DOUBLE_EQ(result, ZERO_DOUBLE);

    /**
     * @tc.steps5: Test the function NormalizeToPx of the class Dimension with DIMENSION_PCT.
     * @tc.expected: The return values are equal to ZERO_DOUBLE.
     */
    result = 0;
    EXPECT_FALSE(
        DIMENSION_PCT.NormalizeToPx(-DEFAULT_DOUBLE, -DEFAULT_DOUBLE, -DEFAULT_DOUBLE, -DEFAULT_DOUBLE, result));
    EXPECT_DOUBLE_EQ(result, ZERO_DOUBLE);

    /**
     * @tc.steps6: Test the function NormalizeToPx of the class Dimension with DIMENSION_AUTO.
     * @tc.expected: The return values are equal to ZERO_DOUBLE.
     */
    result = 0;
    EXPECT_FALSE(
        DIMENSION_AUTO.NormalizeToPx(-DEFAULT_DOUBLE, -DEFAULT_DOUBLE, -DEFAULT_DOUBLE, -DEFAULT_DOUBLE, result));
    EXPECT_DOUBLE_EQ(result, ZERO_DOUBLE);

    /**
     * @tc.steps7: Test the function NormalizeToPx of the class Dimension with DIMENSION_CALC.
     * @tc.expected: The return values are equal to ZERO_DOUBLE.
     */
    result = 0;
    EXPECT_FALSE(
        DIMENSION_CALC.NormalizeToPx(-DEFAULT_DOUBLE, -DEFAULT_DOUBLE, -DEFAULT_DOUBLE, -DEFAULT_DOUBLE, result));
    EXPECT_DOUBLE_EQ(result, ZERO_DOUBLE);
}

/**
 * @tc.name: DimensionTest005
 * @tc.desc: Test the function NormalizeToPx of the class Dimension with DEFAULT_DOUBLE.
 * @tc.type: FUNC
 */
HWTEST_F(DimensionTest, DimensionTest005, TestSize.Level1)
{
    /**
     * @tc.steps1: Test the function NormalizeToPx of the class Dimension with DIMENSION_PX.
     * @tc.expected: The return values are equal to DEFAULT_DOUBLE.
     */
    double result = 0;
    EXPECT_TRUE(DIMENSION_PX.NormalizeToPx(DEFAULT_DOUBLE, DEFAULT_DOUBLE, DEFAULT_DOUBLE, DEFAULT_DOUBLE, result));
    EXPECT_DOUBLE_EQ(result, DEFAULT_DOUBLE);

    /**
     * @tc.steps2: Test the function NormalizeToPx of the class Dimension with DIMENSION_VP.
     * @tc.expected: The return values are equal to DEFAULT_DOUBLE.
     */
    result = 0;
    EXPECT_TRUE(DIMENSION_VP.NormalizeToPx(DEFAULT_DOUBLE, DEFAULT_DOUBLE, DEFAULT_DOUBLE, DEFAULT_DOUBLE, result));
    EXPECT_DOUBLE_EQ(result, DEFAULT_DOUBLE);

    /**
     * @tc.steps3: Test the function NormalizeToPx of the class Dimension with DIMENSION_FP.
     * @tc.expected: The return values are equal to DEFAULT_DOUBLE.
     */
    result = 0;
    EXPECT_TRUE(DIMENSION_FP.NormalizeToPx(DEFAULT_DOUBLE, DEFAULT_DOUBLE, DEFAULT_DOUBLE, DEFAULT_DOUBLE, result));
    EXPECT_DOUBLE_EQ(result, DEFAULT_DOUBLE);

    /**
     * @tc.steps4: Test the function NormalizeToPx of the class Dimension with DIMENSION_LPX.
     * @tc.expected: The return values are equal to DEFAULT_DOUBLE.
     */
    result = 0;
    EXPECT_TRUE(DIMENSION_LPX.NormalizeToPx(DEFAULT_DOUBLE, DEFAULT_DOUBLE, DEFAULT_DOUBLE, DEFAULT_DOUBLE, result));
    EXPECT_DOUBLE_EQ(result, DEFAULT_DOUBLE);

    /**
     * @tc.steps5: Test the function NormalizeToPx of the class Dimension with DIMENSION_PCT.
     * @tc.expected: The return values are equal to DEFAULT_DOUBLE.
     */
    result = 0;
    EXPECT_TRUE(DIMENSION_PCT.NormalizeToPx(DEFAULT_DOUBLE, DEFAULT_DOUBLE, DEFAULT_DOUBLE, DEFAULT_DOUBLE, result));
    EXPECT_DOUBLE_EQ(result, DEFAULT_DOUBLE);

    /**
     * @tc.steps6: Test the function NormalizeToPx of the class Dimension with DIMENSION_AUTO.
     * @tc.expected: The return values are equal to ZERO_DOUBLE.
     */
    result = 0;
    EXPECT_FALSE(DIMENSION_AUTO.NormalizeToPx(DEFAULT_DOUBLE, DEFAULT_DOUBLE, DEFAULT_DOUBLE, DEFAULT_DOUBLE, result));
    EXPECT_DOUBLE_EQ(result, ZERO_DOUBLE);

    /**
     * @tc.steps7: Test the function NormalizeToPx of the class Dimension with DIMENSION_CALC.
     * @tc.expected: The return values are equal to ZERO_DOUBLE.
     */
    result = 0;
    EXPECT_FALSE(DIMENSION_CALC.NormalizeToPx(DEFAULT_DOUBLE, DEFAULT_DOUBLE, DEFAULT_DOUBLE, DEFAULT_DOUBLE, result));
    EXPECT_DOUBLE_EQ(result, ZERO_DOUBLE);
}

/**
 * @tc.name: DimensionTest006
 * @tc.desc: ConvertToFp().
 * @tc.type: FUNC
 */
HWTEST_F(DimensionTest, DimensionTest006, TestSize.Level1)
{
    /**
     * @tc.steps1: Test the function ConvertToFp of the class Dimension.
     * @tc.expected: The return values are equal to DEFAULT_DOUBLE or ZERO_DOUBLE
     */
    EXPECT_DOUBLE_EQ(DIMENSION_PX.ConvertToFp(), DEFAULT_DOUBLE);
    EXPECT_DOUBLE_EQ(DIMENSION_VP.ConvertToFp(), DEFAULT_DOUBLE);
    EXPECT_DOUBLE_EQ(DIMENSION_FP.ConvertToFp(), DEFAULT_DOUBLE);
    EXPECT_DOUBLE_EQ(DIMENSION_LPX.ConvertToFp(), DEFAULT_DOUBLE);
    EXPECT_DOUBLE_EQ(DIMENSION_PCT.ConvertToFp(), ZERO_DOUBLE);
    EXPECT_DOUBLE_EQ(DIMENSION_AUTO.ConvertToFp(), ZERO_DOUBLE);
    EXPECT_DOUBLE_EQ(DIMENSION_CALC.ConvertToFp(), ZERO_DOUBLE);
    EXPECT_DOUBLE_EQ(DIMENSION_NONE.ConvertToFp(), DEFAULT_DOUBLE);
}

/**
 * @tc.name: DimensionTest007
 * @tc.desc: FromString().
 * @tc.type: FUNC
 */
HWTEST_F(DimensionTest, DimensionTest007, TestSize.Level1)
{
    /**
     * @tc.steps1: Test the function FromString of the class Dimension.
     * @tc.expected: The return values are equal to DEFAULT_DOUBLE or ZERO_DOUBLE
     */
    EXPECT_DOUBLE_EQ(Dimension::FromString(DIMENSION_PX_STR).Value(), DEFAULT_DOUBLE);
    EXPECT_DOUBLE_EQ(Dimension::FromString(DIMENSION_VP_STR).Value(), DEFAULT_DOUBLE);
    EXPECT_DOUBLE_EQ(Dimension::FromString(DIMENSION_FP_STR).Value(), DEFAULT_DOUBLE);
    EXPECT_DOUBLE_EQ(Dimension::FromString(DIMENSION_LPX_STR).Value(), DEFAULT_DOUBLE);
    EXPECT_DOUBLE_EQ(Dimension::FromString(DIMENSION_PCT_STR).Value(), DEFAULT_DOUBLE);
    EXPECT_DOUBLE_EQ(Dimension::FromString(DIMENSION_AUTO_STR).Value(), DEFAULT_DOUBLE);
    EXPECT_DOUBLE_EQ(Dimension::FromString("").Value(), ZERO_DOUBLE);
}

/**
 * @tc.name: DimensionTest008
 * @tc.desc: ConvertToPxWithSize().
 * @tc.type: FUNC
 */
HWTEST_F(DimensionTest, DimensionTest008, TestSize.Level1)
{
    /**
     * @tc.steps1: Test the function ConvertToPxWithSize of the class Dimension.
     * @tc.expected: The return values are equal to DEFAULT_DOUBLE * size.
     */
    Dimension dimension(DEFAULT_DOUBLE, DimensionUnit::PERCENT);
    EXPECT_DOUBLE_EQ(dimension.ConvertToPxWithSize(5.0), DEFAULT_DOUBLE * 5.0);
    dimension.SetUnit(DimensionUnit::PX);
    EXPECT_DOUBLE_EQ(dimension.ConvertToPxWithSize(5.0), DEFAULT_DOUBLE);
}

/**
 * @tc.name: DimensionTest009
 * @tc.desc: ConvertToPxDistribute().
 * @tc.type: FUNC
 */
HWTEST_F(DimensionTest, DimensionTest009, TestSize.Level1)
{
    /**
     * @tc.steps1: Test the function ConvertToPxDistribute of the class Dimension.
     * @tc.expected: The return values are equal to DEFAULT_DOUBLE * size.
     */
    TextStyle textStyle;
    auto defaultFp = DIMENSION_FP.ConvertToPxDistribute(textStyle.GetMaxFontScale(), textStyle.GetMinFontScale());
    EXPECT_DOUBLE_EQ(defaultFp, DEFAULT_DOUBLE);
}

class DimensionGapTest : public testing::Test {
public:
    static void SetUpTestSuite()
    {
        NG::MockPipelineContext::SetUp();
    }
    static void TearDownTestSuite()
    {
        NG::MockPipelineContext::TearDown();
    }

    void SetUp() override
    {
        auto pipeline = NG::MockPipelineContext::GetCurrentContext();
        ASSERT_TRUE(pipeline);
        pipeline->SetFontScale(1.0f);
        pipeline->SetMaxAppFontScale(static_cast<float>(INT32_MAX));
        pipeline->SetFollowSystem(false);
    }
};

/**
 * @tc.name: DimensionGapResetInvalidValue001
 * @tc.desc: Test ResetInvalidValue with NaN and normal values.
 * @tc.type: FUNC
 */
HWTEST_F(DimensionGapTest, DimensionGapResetInvalidValue001, TestSize.Level1)
{
    /**
     * @tc.steps: construct NaN dimension and normal dimension, call ResetInvalidValue.
     * @tc.expected: NaN resets to 0 px, normal value keeps unchanged.
     */
    Dimension nanDim(std::nan(""), DimensionUnit::VP);
    nanDim.ResetInvalidValue();
    EXPECT_DOUBLE_EQ(nanDim.Value(), 0.0);
    EXPECT_TRUE(nanDim.Unit() == DimensionUnit::PX);

    Dimension normalDim(5.0, DimensionUnit::VP);
    normalDim.ResetInvalidValue();
    EXPECT_DOUBLE_EQ(normalDim.Value(), 5.0);
    EXPECT_TRUE(normalDim.Unit() == DimensionUnit::VP);
}

/**
 * @tc.name: DimensionGapGetAdaptDimensionUnit001
 * @tc.desc: Test GetAdaptDimensionUnit picks the smaller unit.
 * @tc.type: FUNC
 */
HWTEST_F(DimensionGapTest, DimensionGapGetAdaptDimensionUnit001, TestSize.Level1)
{
    /**
     * @tc.steps: compare units of different enum values in both directions.
     * @tc.expected: always returns the unit with smaller enum value.
     */
    Dimension first(1.0, DimensionUnit::VP);
    Dimension second(1.0, DimensionUnit::PX);
    EXPECT_TRUE(first.GetAdaptDimensionUnit(second) == DimensionUnit::PX);
    EXPECT_TRUE(second.GetAdaptDimensionUnit(first) == DimensionUnit::PX);

    Dimension sameA(1.0, DimensionUnit::FP);
    Dimension sameB(2.0, DimensionUnit::FP);
    EXPECT_TRUE(sameA.GetAdaptDimensionUnit(sameB) == DimensionUnit::FP);
}

/**
 * @tc.name: DimensionGapToString001
 * @tc.desc: Test ToString branches for all units including invalid enums.
 * @tc.type: FUNC
 */
HWTEST_F(DimensionGapTest, DimensionGapToString001, TestSize.Level1)
{
    /**
     * @tc.steps: stringify dimensions of every unit plus out-of-range enums.
     * @tc.expected: CALC and out-of-range fall to px; NONE/INVALID have own
     *               suffixes; PERCENT multiplies by 100.
     */
    EXPECT_EQ(Dimension(1.0, DimensionUnit::VP).ToString(), "1.00vp");
    EXPECT_EQ(Dimension(1.0, DimensionUnit::FP).ToString(), "1.00fp");
    EXPECT_EQ(Dimension(1.0, DimensionUnit::LPX).ToString(), "1.00lpx");
    EXPECT_EQ(Dimension(1.0, DimensionUnit::AUTO).ToString(), "1.00auto");
    EXPECT_EQ(Dimension(1.0, DimensionUnit::NONE).ToString(), "1.00none");
    EXPECT_EQ(Dimension(1.0, DimensionUnit::INVALID).ToString(), "1.00invalid");
    EXPECT_EQ(Dimension(1.0, DimensionUnit::CALC).ToString(), "1.00px");
    EXPECT_EQ(Dimension(1.0, DimensionUnit::PERCENT).ToString(), "100.00%");
    EXPECT_EQ(Dimension(1.0, static_cast<DimensionUnit>(-3)).ToString(), "1.00px");
}

/**
 * @tc.name: DimensionGapFromString001
 * @tc.desc: Test FromString suffix parsing branches.
 * @tc.type: FUNC
 */
HWTEST_F(DimensionGapTest, DimensionGapFromString001, TestSize.Level1)
{
    /**
     * @tc.steps: parse strings with known/unknown suffixes and no digits.
     * @tc.expected: known suffix maps to its unit; unknown suffix keeps default
     *               FP unit; percent divides by 100; empty/no-digit returns 0.
     */
    EXPECT_TRUE(Dimension::FromString("10px").Unit() == DimensionUnit::PX);
    EXPECT_TRUE(Dimension::FromString("10vp").Unit() == DimensionUnit::VP);
    EXPECT_TRUE(Dimension::FromString("10fp").Unit() == DimensionUnit::FP);
    EXPECT_TRUE(Dimension::FromString("10lpx").Unit() == DimensionUnit::LPX);
    EXPECT_TRUE(Dimension::FromString("10auto").Unit() == DimensionUnit::AUTO);
    EXPECT_TRUE(Dimension::FromString("50%").Unit() == DimensionUnit::PERCENT);
    EXPECT_DOUBLE_EQ(Dimension::FromString("50%").Value(), 0.5);
    // unknown suffix keeps default FP unit
    EXPECT_TRUE(Dimension::FromString("10none").Unit() == DimensionUnit::FP);
    // no digit found: value 0 with default FP unit
    auto noDigit = Dimension::FromString("abc");
    EXPECT_DOUBLE_EQ(noDigit.Value(), 0.0);
    EXPECT_TRUE(noDigit.Unit() == DimensionUnit::FP);
    auto empty = Dimension::FromString("");
    EXPECT_DOUBLE_EQ(empty.Value(), 0.0);
}

/**
 * @tc.name: DimensionGapConvertToPxWithScale001
 * @tc.desc: Test ConvertToPx(dipScale) and ConvertToPxWithSize.
 * @tc.type: FUNC
 */
HWTEST_F(DimensionGapTest, DimensionGapConvertToPxWithScale001, TestSize.Level1)
{
    /**
     * @tc.steps: convert vp/fp with explicit dip scale, percent with parent size.
     * @tc.expected: vp/fp multiply by dip scale; others passthrough; percent
     *               multiplies by parent size.
     */
    EXPECT_DOUBLE_EQ(Dimension(2.0, DimensionUnit::VP).ConvertToPx(3.0), 6.0);
    EXPECT_DOUBLE_EQ(Dimension(2.0, DimensionUnit::FP).ConvertToPx(3.0), 6.0);
    EXPECT_DOUBLE_EQ(Dimension(2.0, DimensionUnit::PX).ConvertToPx(3.0), 2.0);
    EXPECT_DOUBLE_EQ(Dimension(2.0, DimensionUnit::PERCENT).ConvertToPx(3.0), 2.0);

    EXPECT_DOUBLE_EQ(Dimension(0.5, DimensionUnit::PERCENT).ConvertToPxWithSize(200.0), 100.0);
    EXPECT_DOUBLE_EQ(Dimension(2.0, DimensionUnit::PX).ConvertToPxWithSize(200.0), 2.0);
}

/**
 * @tc.name: DimensionGapGetNativeValue001
 * @tc.desc: Test GetNativeValue unit dispatch.
 * @tc.type: FUNC
 */
HWTEST_F(DimensionGapTest, DimensionGapGetNativeValue001, TestSize.Level1)
{
    /**
     * @tc.steps: query native value for same unit / percent / px / fp / vp targets.
     * @tc.expected: same-unit and percent return raw value; others convert.
     */
    EXPECT_DOUBLE_EQ(Dimension(2.0, DimensionUnit::VP).GetNativeValue(DimensionUnit::VP), 2.0);
    EXPECT_DOUBLE_EQ(Dimension(2.0, DimensionUnit::VP).GetNativeValue(DimensionUnit::PERCENT), 2.0);
    EXPECT_DOUBLE_EQ(Dimension(2.0, DimensionUnit::PX).GetNativeValue(DimensionUnit::PX), 2.0);
    EXPECT_DOUBLE_EQ(Dimension(2.0, DimensionUnit::VP).GetNativeValue(DimensionUnit::PX), 2.0);
}

/**
 * @tc.name: DimensionGapConvertToPxByCustomFontScale001
 * @tc.desc: Test ConvertToPxByCustomFontScale clamping both bounds.
 * @tc.type: FUNC
 */
HWTEST_F(DimensionGapTest, DimensionGapConvertToPxByCustomFontScale001, TestSize.Level1)
{
    /**
     * @tc.steps: set font scale below min, inside range and above max.
     * @tc.expected: font scale is clamped into [min, max] before multiplying.
     */
    auto pipeline = NG::MockPipelineContext::GetCurrentContext();
    ASSERT_TRUE(pipeline);
    Dimension dim(2.0, DimensionUnit::FP);

    pipeline->SetFontScale(1.0f);
    EXPECT_DOUBLE_EQ(dim.ConvertToPxByCustomFontScale(0.5f, 3.2f), 2.0);

    pipeline->SetFontScale(0.1f); // below min -> clamp to 0.5
    EXPECT_DOUBLE_EQ(dim.ConvertToPxByCustomFontScale(0.5f, 3.2f), 1.0);

    pipeline->SetFontScale(5.0f); // above max -> clamp to 3.2
    EXPECT_NEAR(dim.ConvertToPxByCustomFontScale(0.5f, 3.2f), 6.4, 1e-5);

    // null pipeline overload returns raw value
    RefPtr<PipelineBase> nullPipeline;
    EXPECT_DOUBLE_EQ(dim.ConvertToPxByCustomFontScale(0.5f, 3.2f, nullPipeline), 2.0);
}

/**
 * @tc.name: DimensionGapConvertToPxByAppFontScale001
 * @tc.desc: Test ConvertToPxByAppFontScale with max app font scale limit.
 * @tc.type: FUNC
 */
HWTEST_F(DimensionGapTest, DimensionGapConvertToPxByAppFontScale001, TestSize.Level1)
{
    /**
     * @tc.steps: clamp font scale between min and pipeline max app font scale.
     * @tc.expected: min bound and max app bound are both enforced.
     */
    auto pipeline = NG::MockPipelineContext::GetCurrentContext();
    ASSERT_TRUE(pipeline);
    Dimension dim(2.0, DimensionUnit::FP);

    pipeline->SetMaxAppFontScale(3.2f);
    pipeline->SetFontScale(1.0f);
    EXPECT_DOUBLE_EQ(dim.ConvertToPxByAppFontScale(0.5f), 2.0);

    pipeline->SetFontScale(0.1f); // clamped to min 0.5
    EXPECT_DOUBLE_EQ(dim.ConvertToPxByAppFontScale(0.5f), 1.0);

    pipeline->SetFontScale(5.0f); // clamped to max 3.2
    EXPECT_NEAR(dim.ConvertToPxByAppFontScale(0.5f), 6.4, 1e-5);

    pipeline->SetMaxAppFontScale(2.0f);
    pipeline->SetFontScale(4.0f); // clamped to new max 2.0
    EXPECT_DOUBLE_EQ(dim.ConvertToPxByAppFontScale(0.5f), 4.0);

    RefPtr<PipelineBase> nullPipeline;
    EXPECT_DOUBLE_EQ(dim.ConvertToPxByAppFontScale(0.5f, nullPipeline), 2.0);
}

/**
 * @tc.name: DimensionGapConvertToVpByAppFontScale001
 * @tc.desc: Test ConvertToVpByAppFontScale follow-system guard.
 * @tc.type: FUNC
 */
HWTEST_F(DimensionGapTest, DimensionGapConvertToVpByAppFontScale001, TestSize.Level1)
{
    /**
     * @tc.steps: toggle follow system flag and font scale.
     * @tc.expected: not-follow-system returns raw value; follow-system scales value.
     */
    auto pipeline = NG::MockPipelineContext::GetCurrentContext();
    ASSERT_TRUE(pipeline);
    Dimension dim(2.0, DimensionUnit::FP);

    pipeline->SetFollowSystem(false);
    pipeline->SetFontScale(3.0f);
    EXPECT_DOUBLE_EQ(dim.ConvertToVpByAppFontScale(), 2.0);

    pipeline->SetFollowSystem(true);
    pipeline->SetFontScale(3.0f);
    EXPECT_DOUBLE_EQ(dim.ConvertToVpByAppFontScale(), 6.0);

    // WithEnv variant: env font scale wins when provided
    pipeline->SetFollowSystem(true);
    EXPECT_DOUBLE_EQ(dim.ConvertToVpByAppFontScaleWithEnv(std::nullopt), 6.0);
    EXPECT_DOUBLE_EQ(dim.ConvertToVpByAppFontScaleWithEnv(0.5f), 1.0);
}

/**
 * @tc.name: DimensionGapConvertToFpWithEnv001
 * @tc.desc: Test ConvertToFpWithEnv unit dispatch and zero-scale guard.
 * @tc.type: FUNC
 */
HWTEST_F(DimensionGapTest, DimensionGapConvertToFpWithEnv001, TestSize.Level1)
{
    /**
     * @tc.steps: convert fp/vp/px/none/lpx units with env font scale.
     * @tc.expected: fp passthrough; others divide by scale; zero scale returns 0.
     */
    auto pipeline = NG::MockPipelineContext::GetCurrentContext();
    ASSERT_TRUE(pipeline);

    EXPECT_DOUBLE_EQ(Dimension(2.0, DimensionUnit::FP).ConvertToFpWithEnv(3.0f), 2.0);
    EXPECT_DOUBLE_EQ(Dimension(2.0, DimensionUnit::VP).ConvertToFpWithEnv(2.0f), 1.0);
    EXPECT_DOUBLE_EQ(Dimension(2.0, DimensionUnit::PX).ConvertToFpWithEnv(2.0f), 1.0);
    EXPECT_DOUBLE_EQ(Dimension(2.0, DimensionUnit::NONE).ConvertToFpWithEnv(2.0f), 1.0);
    EXPECT_DOUBLE_EQ(Dimension(2.0, DimensionUnit::LPX).ConvertToFpWithEnv(2.0f), 1.0);
    // zero font scale guard
    EXPECT_DOUBLE_EQ(Dimension(2.0, DimensionUnit::VP).ConvertToFpWithEnv(0.0f), 0.0);
    // percent falls to the final 0.0
    EXPECT_DOUBLE_EQ(Dimension(2.0, DimensionUnit::PERCENT).ConvertToFpWithEnv(2.0f), 0.0);
    // nullopt env scale reads pipeline font scale
    pipeline->SetFontScale(4.0f);
    EXPECT_DOUBLE_EQ(Dimension(2.0, DimensionUnit::VP).ConvertToFpWithEnv(std::nullopt), 0.5);
}

/**
 * @tc.name: DimensionGapConvertToPxDistribute001
 * @tc.desc: Test ConvertToPxDistribute allowScale and optional bounds paths.
 * @tc.type: FUNC
 */
HWTEST_F(DimensionGapTest, DimensionGapConvertToPxDistribute001, TestSize.Level1)
{
    /**
     * @tc.steps: distribute non-fp unit, fp without scaling and fp with bounds.
     * @tc.expected: non-fp converts directly; !allowScale uses raw dip multiply;
     *               bounds select app/custom font scale paths.
     */
    auto pipeline = NG::MockPipelineContext::GetCurrentContext();
    ASSERT_TRUE(pipeline);
    pipeline->SetFontScale(2.0f);
    pipeline->SetMaxAppFontScale(static_cast<float>(INT32_MAX));

    // non-fp unit: ConvertToPx passthrough for px
    EXPECT_DOUBLE_EQ(Dimension(2.0, DimensionUnit::PX).ConvertToPxDistribute(0.0f, 3.0f, true), 2.0);
    // fp + !allowScale: value * dipScale
    EXPECT_DOUBLE_EQ(Dimension(2.0, DimensionUnit::FP).ConvertToPxDistribute(0.0f, 3.0f, false), 2.0);
    // fp + allowScale + no max bound: app font scale path (clamp(2.0, 0, INT32_MAX) = 2.0)
    EXPECT_DOUBLE_EQ(
        Dimension(2.0, DimensionUnit::FP).ConvertToPxDistribute(0.0f, std::nullopt, true), 4.0);
    // fp + allowScale + custom bounds: clamp(2.0, 0.5, 3.2) = 2.0
    EXPECT_DOUBLE_EQ(Dimension(2.0, DimensionUnit::FP).ConvertToPxDistribute(0.5f, 3.2f, true), 4.0);
}

/**
 * @tc.name: DimensionGapConvertToPxDistributeWithEnv001
 * @tc.desc: Test ConvertToPxDistributeWithEnv env scale paths.
 * @tc.type: FUNC
 */
HWTEST_F(DimensionGapTest, DimensionGapConvertToPxDistributeWithEnv001, TestSize.Level1)
{
    /**
     * @tc.steps: distribute fp with env font scale and optional bounds.
     * @tc.expected: env scale overrides pipeline font scale; !allowScale and
     *               non-fp branches behave like ConvertToPxDistribute.
     */
    auto pipeline = NG::MockPipelineContext::GetCurrentContext();
    ASSERT_TRUE(pipeline);

    // non-fp unit goes ConvertToPx
    EXPECT_DOUBLE_EQ(
        Dimension(2.0, DimensionUnit::PX).ConvertToPxDistributeWithEnv(0.0f, 3.0f, true, 2.0f), 2.0);
    // fp + !allowScale
    EXPECT_DOUBLE_EQ(
        Dimension(2.0, DimensionUnit::FP).ConvertToPxDistributeWithEnv(0.0f, 3.0f, false, 2.0f), 2.0);
    // fp + allowScale + no max -> app font scale with env: clamp(3.0, 0, INT32_MAX)=3.0
    EXPECT_DOUBLE_EQ(
        Dimension(2.0, DimensionUnit::FP).ConvertToPxDistributeWithEnv(0.0f, std::nullopt, true, 3.0f),
        6.0);
    // fp + allowScale + custom bounds: clamp(3.0, 0.5, 2.0)=2.0
    EXPECT_DOUBLE_EQ(
        Dimension(2.0, DimensionUnit::FP).ConvertToPxDistributeWithEnv(0.5f, 2.0f, true, 3.0f), 4.0);
}

/**
 * @tc.name: DimensionGapConvertToPxByFontScaleWithEnv001
 * @tc.desc: Test WithEnv variants of font scale conversions.
 * @tc.type: FUNC
 */
HWTEST_F(DimensionGapTest, DimensionGapConvertToPxByFontScaleWithEnv001, TestSize.Level1)
{
    /**
     * @tc.steps: call all WithEnv overloads with null/explicit pipeline and env scale.
     * @tc.expected: null pipeline returns raw value; env scale clamped by bounds.
     */
    auto pipeline = NG::MockPipelineContext::GetCurrentContext();
    ASSERT_TRUE(pipeline);
    Dimension dim(2.0, DimensionUnit::FP);
    RefPtr<PipelineBase> nullPipeline;

    // overloads without an explicit pipeline use the current pipeline and apply env scale
    EXPECT_DOUBLE_EQ(dim.ConvertToPxByCustomFontScaleWithEnv(0.5f, 3.2f, 2.0f), 4.0);
    EXPECT_DOUBLE_EQ(dim.ConvertToPxByCustomFontScaleWithEnv(0.5f, 3.2f, nullPipeline, 2.0f), 2.0);
    EXPECT_DOUBLE_EQ(dim.ConvertToPxByAppFontScaleWithEnv(0.5f, 2.0f), 4.0);
    EXPECT_DOUBLE_EQ(dim.ConvertToPxByAppFontScaleWithEnv(0.5f, nullPipeline, 2.0f), 2.0);

    // explicit pipeline with env scale clamped by custom bounds
    EXPECT_DOUBLE_EQ(
        dim.ConvertToPxByCustomFontScaleWithEnv(0.5f, 2.0f, pipeline, 5.0f), 4.0);
    // app font scale path: clamp(5.0, 0.5, INT32_MAX) = 5.0
    EXPECT_DOUBLE_EQ(dim.ConvertToPxByAppFontScaleWithEnv(0.5f, 5.0f), 10.0);
}

/**
 * @tc.name: DimensionGapConvertToUnitWithPipeline001
 * @tc.desc: Test ConvertToPx/ConvertToVp/ConvertToFp unit dispatch under pipeline.
 * @tc.type: FUNC
 */
HWTEST_F(DimensionGapTest, DimensionGapConvertToUnitWithPipeline001, TestSize.Level1)
{
    /**
     * @tc.steps: convert every unit via pipeline aware conversions.
     * @tc.expected: none/px passthrough; vp/fp scaled by font settings; lpx scaled
     *               by logic scale; percent/auto/calc return 0.
     */
    auto pipeline = NG::MockPipelineContext::GetCurrentContext();
    ASSERT_TRUE(pipeline);
    pipeline->SetFontScale(1.0f);
    pipeline->SetFollowSystem(true);

    // ConvertToPx: NONE/PX passthrough, VP * dipScale(1.0), LPX * logicScale(1.0)
    EXPECT_DOUBLE_EQ(Dimension(2.0, DimensionUnit::NONE).ConvertToPx(), 2.0);
    EXPECT_DOUBLE_EQ(Dimension(2.0, DimensionUnit::PX).ConvertToPx(), 2.0);
    EXPECT_DOUBLE_EQ(Dimension(2.0, DimensionUnit::VP).ConvertToPx(), 2.0);
    EXPECT_DOUBLE_EQ(Dimension(2.0, DimensionUnit::LPX).ConvertToPx(), 2.0);
    EXPECT_DOUBLE_EQ(Dimension(2.0, DimensionUnit::FP).ConvertToPx(), 2.0);
    EXPECT_DOUBLE_EQ(Dimension(2.0, DimensionUnit::PERCENT).ConvertToPx(), 0.0);

    // ConvertToVp: NONE/PX divided by dipScale(1.0); FP via app font scale
    EXPECT_DOUBLE_EQ(Dimension(2.0, DimensionUnit::NONE).ConvertToVp(), 2.0);
    EXPECT_DOUBLE_EQ(Dimension(2.0, DimensionUnit::PX).ConvertToVp(), 2.0);
    EXPECT_DOUBLE_EQ(Dimension(2.0, DimensionUnit::LPX).ConvertToVp(), 2.0);
    EXPECT_DOUBLE_EQ(Dimension(2.0, DimensionUnit::FP).ConvertToVp(), 2.0);
    EXPECT_DOUBLE_EQ(Dimension(2.0, DimensionUnit::PERCENT).ConvertToVp(), 0.0);

    // ConvertToFp with fontScale 2.0: VP -> 1.0
    pipeline->SetFontScale(2.0f);
    EXPECT_DOUBLE_EQ(Dimension(2.0, DimensionUnit::VP).ConvertToFp(), 1.0);
    // fontScale 0 guard -> 0.0
    pipeline->SetFontScale(0.0f);
    EXPECT_DOUBLE_EQ(Dimension(2.0, DimensionUnit::VP).ConvertToFp(), 0.0);
}
} // namespace OHOS::Ace

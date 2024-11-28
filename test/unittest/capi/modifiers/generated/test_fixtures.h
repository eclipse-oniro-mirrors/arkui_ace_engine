/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef GENERATED_FOUNDATION_ACE_CAPI_TEST_FIXTURES_H
#define GENERATED_FOUNDATION_ACE_CAPI_TEST_FIXTURES_H

#include <string>
#include <tuple>
#include <vector>

#include "arkoala_api.h"
#include "test_fixtures_enums.h"

#include "core/interfaces/native/utility/reverse_converter.h"

namespace OHOS::Ace::NG::Fixtures {
enum ResID {
    STRING_RES_0_ID,
    STRING_RES_1_ID,
    STRING_RES_2_ID,
    STRING_RES_NO_EMPTY_0_ID,
    STRING_RES_NO_EMPTY_1_ID,
    STRING_EMPTY_RES_UNDEFINED_0_ID,
    FONT_FAMILY_RESOURCE_0_ID,
    FONT_FAMILY_RESOURCE_1_ID,
    FONT_FAMILY_RESOURCE_2_ID,
    COLORS_RES_0_ID,
    RES_DATA_FOR_LENGTH_0_ID,
    RES_DATA_FOR_LENGTH_1_ID,
    RES_DATA_FOR_LENGTH_2_ID,
    RES_DATA_FOR_LENGTH_3_ID,
    RES_DATA_FOR_LENGTH_4_ID,
    RES_DATA_FOR_LENGTH_5_ID,
    RES_DATA_FOR_LENGTH_6_ID,
    RES_DATA_FOR_LENGTH_7_ID,
    RES_DATA_FOR_LENGTH_8_ID,
    RES_DATA_FOR_LENGTH_9_ID,
    RES_DATA_FOR_LENGTH_10_ID,
    RES_DATA_FOR_LENGTH_11_ID,
    DIMENSIONS_RES_NON_NEG_0_ID,
    DIMENSIONS_RES_NON_NEG_1_ID,
    DIMENSIONS_RES_NON_NEG_2_ID,
    DIMENSIONS_RES_NON_NEG_3_ID,
    DIMENSIONS_RES_NON_NEG_4_ID,
    DIMENSIONS_RES_NON_NEG_5_ID,
    DIMENSIONS_RES_NON_NEG_6_ID,
    DIMENSIONS_RES_NON_NEG_7_ID,
    DIMENSIONS_RES_NON_NEG_8_ID,
    DIMENSIONS_RES_NON_NEG_9_ID,
    DIMENSIONS_RES_NON_NEG_10_ID,
    DIMENSIONS_RES_NON_NEG_11_ID,
    DIMENSIONS_RES_NON_NEG_12_ID,
    DIMENSIONS_RES_NON_NEG_13_ID,
    DIMENSIONS_RES_NON_NEG_NON_PCT_0_ID,
    DIMENSIONS_RES_NON_NEG_NON_PCT_1_ID,
    DIMENSIONS_RES_NON_NEG_NON_PCT_2_ID,
    DIMENSIONS_RES_NON_NEG_NON_PCT_3_ID,
    DIMENSIONS_RES_NON_NEG_NON_PCT_4_ID,
    DIMENSIONS_RES_NON_NEG_NON_PCT_5_ID,
    DIMENSIONS_RES_NON_NEG_NON_PCT_6_ID,
    DIMENSIONS_RES_NON_NEG_NON_PCT_7_ID,
    DIMENSIONS_RES_NON_NEG_NON_PCT_8_ID,
    DIMENSIONS_RES_NON_NEG_NON_PCT_9_ID,
    DIMENSIONS_RES_NON_NEG_NON_PCT_10_ID,
    DIMENSIONS_RES_NON_NEG_NON_PCT_11_ID,
    DIMENSIONS_RES_NON_NEG_NON_PCT_12_ID,
    DIMENSIONS_RES_NON_NEG_NON_PCT_13_ID,
    MAX_FONT_SCALE_RES_0_ID,
    MAX_FONT_SCALE_RES_1_ID,
    MAX_FONT_SCALE_RES_2_ID,
    MIN_FONT_SCALE_RES_0_ID,
    MIN_FONT_SCALE_RES_1_ID,
    MIN_FONT_SCALE_RES_2_ID,
    SCROLLABLE_FRICTION_RES_0_ID,
    SCROLLABLE_FRICTION_RES_1_ID,
    SCROLLABLE_FRICTION_RES_2_ID,
    SCROLLABLE_FRICTION_RES_3_ID,
    SCROLLABLE_FRICTION_RES_4_ID,
    OPACITY_RESOURCE_0_ID,
    OPACITY_RESOURCE_1_ID,
    OPACITY_RESOURCE_2_ID,
    OPACITY_RESOURCE_3_ID,
    OPACITY_RESOURCE_4_ID,
    OPACITY_RESOURCE_5_ID,
    OPACITY_RESOURCE_6_ID,
    OPACITY_RESOURCE_7_ID,
    FRICTION_RESOURCE_0_ID,
    FRICTION_RESOURCE_1_ID,
    FRICTION_RESOURCE_2_ID,
    FRICTION_RESOURCE_3_ID,
    FRICTION_RESOURCE_4_ID,
    FRICTION_RESOURCE_5_ID,
    FRICTION_RESOURCE_6_ID,
    FRICTION_RESOURCE_7_ID,
    SHADOW_RADIUS_RES_0_ID,
    SHADOW_RADIUS_RES_1_ID,
    SHADOW_RADIUS_RES_2_ID,
    SHADOW_RADIUS_RES_3_ID,
    SHADOW_RADIUS_RES_4_ID,
    SHADOW_RADIUS_RES_5_ID,
    SHADOW_OFFSET_RES_0_ID,
    SHADOW_OFFSET_RES_1_ID,
    SHADOW_OFFSET_RES_2_ID,
    SHADOW_OFFSET_RES_3_ID,
    SHADOW_OFFSET_RES_4_ID,
    SHADOW_OFFSET_RES_5_ID,
    SHADOW_OFFSET_RES_6_ID,
};

inline constexpr auto STRING_RES_0_STR = "STRING_RES_0_STR";
inline constexpr auto STRING_RES_1_STR = "STRING_RES_1_STR";
inline constexpr auto STRING_RES_2_STR = "STRING_RES_2_STR";
inline constexpr auto STRING_RES_NO_EMPTY_0_STR = "STRING_RES_NO_EMPTY_0_STR";
inline constexpr auto STRING_RES_NO_EMPTY_1_STR = "STRING_RES_NO_EMPTY_1_STR";
inline constexpr auto STRING_EMPTY_RES_UNDEFINED_0_STR = "STRING_EMPTY_RES_UNDEFINED_0_STR";
inline constexpr auto FONT_FAMILY_RESOURCE_0_STR = "FONT_FAMILY_RESOURCE_0_STR";
inline constexpr auto FONT_FAMILY_RESOURCE_1_STR = "FONT_FAMILY_RESOURCE_1_STR";
inline constexpr auto FONT_FAMILY_RESOURCE_2_STR = "FONT_FAMILY_RESOURCE_2_STR";
inline constexpr auto COLORS_RES_0_STR = "COLORS_RES_0_STR";
inline constexpr auto RES_DATA_FOR_LENGTH_0_STR = "RES_DATA_FOR_LENGTH_0_STR";
inline constexpr auto RES_DATA_FOR_LENGTH_1_STR = "RES_DATA_FOR_LENGTH_1_STR";
inline constexpr auto RES_DATA_FOR_LENGTH_2_STR = "RES_DATA_FOR_LENGTH_2_STR";
inline constexpr auto RES_DATA_FOR_LENGTH_3_STR = "RES_DATA_FOR_LENGTH_3_STR";
inline constexpr auto RES_DATA_FOR_LENGTH_4_STR = "RES_DATA_FOR_LENGTH_4_STR";
inline constexpr auto RES_DATA_FOR_LENGTH_5_STR = "RES_DATA_FOR_LENGTH_5_STR";
inline constexpr auto RES_DATA_FOR_LENGTH_6_STR = "RES_DATA_FOR_LENGTH_6_STR";
inline constexpr auto RES_DATA_FOR_LENGTH_7_STR = "RES_DATA_FOR_LENGTH_7_STR";
inline constexpr auto RES_DATA_FOR_LENGTH_8_STR = "RES_DATA_FOR_LENGTH_8_STR";
inline constexpr auto RES_DATA_FOR_LENGTH_9_STR = "RES_DATA_FOR_LENGTH_9_STR";
inline constexpr auto RES_DATA_FOR_LENGTH_10_STR = "RES_DATA_FOR_LENGTH_10_STR";
inline constexpr auto RES_DATA_FOR_LENGTH_11_STR = "RES_DATA_FOR_LENGTH_11_STR";
inline constexpr auto DIMENSIONS_RES_NON_NEG_0_STR = "DIMENSIONS_RES_NON_NEG_0_STR";
inline constexpr auto DIMENSIONS_RES_NON_NEG_1_STR = "DIMENSIONS_RES_NON_NEG_1_STR";
inline constexpr auto DIMENSIONS_RES_NON_NEG_2_STR = "DIMENSIONS_RES_NON_NEG_2_STR";
inline constexpr auto DIMENSIONS_RES_NON_NEG_3_STR = "DIMENSIONS_RES_NON_NEG_3_STR";
inline constexpr auto DIMENSIONS_RES_NON_NEG_4_STR = "DIMENSIONS_RES_NON_NEG_4_STR";
inline constexpr auto DIMENSIONS_RES_NON_NEG_5_STR = "DIMENSIONS_RES_NON_NEG_5_STR";
inline constexpr auto DIMENSIONS_RES_NON_NEG_6_STR = "DIMENSIONS_RES_NON_NEG_6_STR";
inline constexpr auto DIMENSIONS_RES_NON_NEG_7_STR = "DIMENSIONS_RES_NON_NEG_7_STR";
inline constexpr auto DIMENSIONS_RES_NON_NEG_8_STR = "DIMENSIONS_RES_NON_NEG_8_STR";
inline constexpr auto DIMENSIONS_RES_NON_NEG_9_STR = "DIMENSIONS_RES_NON_NEG_9_STR";
inline constexpr auto DIMENSIONS_RES_NON_NEG_10_STR = "DIMENSIONS_RES_NON_NEG_10_STR";
inline constexpr auto DIMENSIONS_RES_NON_NEG_11_STR = "DIMENSIONS_RES_NON_NEG_11_STR";
inline constexpr auto DIMENSIONS_RES_NON_NEG_12_STR = "DIMENSIONS_RES_NON_NEG_12_STR";
inline constexpr auto DIMENSIONS_RES_NON_NEG_13_STR = "DIMENSIONS_RES_NON_NEG_13_STR";
inline constexpr auto DIMENSIONS_RES_NON_NEG_NON_PCT_0_STR = "DIMENSIONS_RES_NON_NEG_NON_PCT_0_STR";
inline constexpr auto DIMENSIONS_RES_NON_NEG_NON_PCT_1_STR = "DIMENSIONS_RES_NON_NEG_NON_PCT_1_STR";
inline constexpr auto DIMENSIONS_RES_NON_NEG_NON_PCT_2_STR = "DIMENSIONS_RES_NON_NEG_NON_PCT_2_STR";
inline constexpr auto DIMENSIONS_RES_NON_NEG_NON_PCT_3_STR = "DIMENSIONS_RES_NON_NEG_NON_PCT_3_STR";
inline constexpr auto DIMENSIONS_RES_NON_NEG_NON_PCT_4_STR = "DIMENSIONS_RES_NON_NEG_NON_PCT_4_STR";
inline constexpr auto DIMENSIONS_RES_NON_NEG_NON_PCT_5_STR = "DIMENSIONS_RES_NON_NEG_NON_PCT_5_STR";
inline constexpr auto DIMENSIONS_RES_NON_NEG_NON_PCT_6_STR = "DIMENSIONS_RES_NON_NEG_NON_PCT_6_STR";
inline constexpr auto DIMENSIONS_RES_NON_NEG_NON_PCT_7_STR = "DIMENSIONS_RES_NON_NEG_NON_PCT_7_STR";
inline constexpr auto DIMENSIONS_RES_NON_NEG_NON_PCT_8_STR = "DIMENSIONS_RES_NON_NEG_NON_PCT_8_STR";
inline constexpr auto DIMENSIONS_RES_NON_NEG_NON_PCT_9_STR = "DIMENSIONS_RES_NON_NEG_NON_PCT_9_STR";
inline constexpr auto DIMENSIONS_RES_NON_NEG_NON_PCT_10_STR = "DIMENSIONS_RES_NON_NEG_NON_PCT_10_STR";
inline constexpr auto DIMENSIONS_RES_NON_NEG_NON_PCT_11_STR = "DIMENSIONS_RES_NON_NEG_NON_PCT_11_STR";
inline constexpr auto DIMENSIONS_RES_NON_NEG_NON_PCT_12_STR = "DIMENSIONS_RES_NON_NEG_NON_PCT_12_STR";
inline constexpr auto DIMENSIONS_RES_NON_NEG_NON_PCT_13_STR = "DIMENSIONS_RES_NON_NEG_NON_PCT_13_STR";
inline constexpr auto MAX_FONT_SCALE_RES_0_STR = "MAX_FONT_SCALE_RES_0_STR";
inline constexpr auto MAX_FONT_SCALE_RES_1_STR = "MAX_FONT_SCALE_RES_1_STR";
inline constexpr auto MAX_FONT_SCALE_RES_2_STR = "MAX_FONT_SCALE_RES_2_STR";
inline constexpr auto MIN_FONT_SCALE_RES_0_STR = "MIN_FONT_SCALE_RES_0_STR";
inline constexpr auto MIN_FONT_SCALE_RES_1_STR = "MIN_FONT_SCALE_RES_1_STR";
inline constexpr auto MIN_FONT_SCALE_RES_2_STR = "MIN_FONT_SCALE_RES_2_STR";
inline constexpr auto SCROLLABLE_FRICTION_RES_0_STR = "SCROLLABLE_FRICTION_RES_0_STR";
inline constexpr auto SCROLLABLE_FRICTION_RES_1_STR = "SCROLLABLE_FRICTION_RES_1_STR";
inline constexpr auto SCROLLABLE_FRICTION_RES_2_STR = "SCROLLABLE_FRICTION_RES_2_STR";
inline constexpr auto SCROLLABLE_FRICTION_RES_3_STR = "SCROLLABLE_FRICTION_RES_3_STR";
inline constexpr auto SCROLLABLE_FRICTION_RES_4_STR = "SCROLLABLE_FRICTION_RES_4_STR";
inline constexpr auto OPACITY_RESOURCE_0_STR = "OPACITY_RESOURCE_0_STR";
inline constexpr auto OPACITY_RESOURCE_1_STR = "OPACITY_RESOURCE_1_STR";
inline constexpr auto OPACITY_RESOURCE_2_STR = "OPACITY_RESOURCE_2_STR";
inline constexpr auto OPACITY_RESOURCE_3_STR = "OPACITY_RESOURCE_3_STR";
inline constexpr auto OPACITY_RESOURCE_4_STR = "OPACITY_RESOURCE_4_STR";
inline constexpr auto OPACITY_RESOURCE_5_STR = "OPACITY_RESOURCE_5_STR";
inline constexpr auto OPACITY_RESOURCE_6_STR = "OPACITY_RESOURCE_6_STR";
inline constexpr auto OPACITY_RESOURCE_7_STR = "OPACITY_RESOURCE_7_STR";
inline constexpr auto FRICTION_RESOURCE_0_STR = "FRICTION_RESOURCE_0_STR";
inline constexpr auto FRICTION_RESOURCE_1_STR = "FRICTION_RESOURCE_1_STR";
inline constexpr auto FRICTION_RESOURCE_2_STR = "FRICTION_RESOURCE_2_STR";
inline constexpr auto FRICTION_RESOURCE_3_STR = "FRICTION_RESOURCE_3_STR";
inline constexpr auto FRICTION_RESOURCE_4_STR = "FRICTION_RESOURCE_4_STR";
inline constexpr auto FRICTION_RESOURCE_5_STR = "FRICTION_RESOURCE_5_STR";
inline constexpr auto FRICTION_RESOURCE_6_STR = "FRICTION_RESOURCE_6_STR";
inline constexpr auto FRICTION_RESOURCE_7_STR = "FRICTION_RESOURCE_7_STR";
inline constexpr auto SHADOW_RADIUS_RES_0_STR = "SHADOW_RADIUS_RES_0_STR";
inline constexpr auto SHADOW_RADIUS_RES_1_STR = "SHADOW_RADIUS_RES_1_STR";
inline constexpr auto SHADOW_RADIUS_RES_2_STR = "SHADOW_RADIUS_RES_2_STR";
inline constexpr auto SHADOW_RADIUS_RES_3_STR = "SHADOW_RADIUS_RES_3_STR";
inline constexpr auto SHADOW_RADIUS_RES_4_STR = "SHADOW_RADIUS_RES_4_STR";
inline constexpr auto SHADOW_RADIUS_RES_5_STR = "SHADOW_RADIUS_RES_5_STR";
inline constexpr auto SHADOW_OFFSET_RES_0_STR = "SHADOW_OFFSET_RES_0_STR";
inline constexpr auto SHADOW_OFFSET_RES_1_STR = "SHADOW_OFFSET_RES_1_STR";
inline constexpr auto SHADOW_OFFSET_RES_2_STR = "SHADOW_OFFSET_RES_2_STR";
inline constexpr auto SHADOW_OFFSET_RES_3_STR = "SHADOW_OFFSET_RES_3_STR";
inline constexpr auto SHADOW_OFFSET_RES_4_STR = "SHADOW_OFFSET_RES_4_STR";
inline constexpr auto SHADOW_OFFSET_RES_5_STR = "SHADOW_OFFSET_RES_5_STR";
inline constexpr auto SHADOW_OFFSET_RES_6_STR = "SHADOW_OFFSET_RES_6_STR";

extern std::vector<std::tuple<ResID, std::string, ResRawValue>> resourceInitTable;

// Fixture 'Boolean' for type 'Ark_Boolean'
extern std::vector<std::tuple<std::string, Ark_Boolean, std::string>> testFixtureBooleanValidValues;

// Fixture 'String' for type 'Ark_String'
extern std::vector<std::tuple<std::string, Ark_String, std::string>> testFixtureStringValidValues;

// Fixture 'StringNoEmpty' for type 'Ark_String'
extern std::vector<std::tuple<std::string, Ark_String, std::string>> testFixtureStringNoEmptyValidValues;

// Fixture 'StringRes' for type 'Ark_Resource'
extern std::vector<std::tuple<std::string, Ark_Resource, std::string>> testFixtureStringResValidValues;

// Fixture 'StringResNoEmpty' for type 'Ark_Resource'
extern std::vector<std::tuple<std::string, Ark_Resource, std::string>> testFixtureStringResNoEmptyValidValues;

// Fixture 'StringEmptyUndefined' for type 'Ark_String'
extern std::vector<std::tuple<std::string, Ark_String, std::string>> testFixtureStringEmptyUndefinedValidValues;

// Fixture 'StringEmptyResUndefined' for type 'Ark_Resource'
extern std::vector<std::tuple<std::string, Ark_Resource, std::string>> testFixtureStringEmptyResUndefinedValidValues;

// Fixture 'FontFamilyResource' for type 'Ark_Resource'
extern std::vector<std::tuple<std::string, Ark_Resource, std::string>> testFixtureFontFamilyResourceValidValues;
extern std::vector<std::tuple<std::string, Ark_Resource>> testFixtureFontFamilyResourceInvalidValues;

// Fixture 'FontFamilyString' for type 'Ark_String'
extern std::vector<std::tuple<std::string, Ark_String, std::string>> testFixtureFontFamilyStringValidValues;
extern std::vector<std::tuple<std::string, Ark_String>> testFixtureFontFamilyStringInvalidValues;

// Fixture 'ColorsStr' for type 'Ark_String'
extern std::vector<std::tuple<std::string, Ark_String, std::string>> testFixtureColorsStrValidValues;
extern std::vector<std::tuple<std::string, Ark_String>> testFixtureColorsStrInvalidValues;

// Fixture 'ColorsNum' for type 'Ark_Number'
extern std::vector<std::tuple<std::string, Ark_Number, std::string>> testFixtureColorsNumValidValues;

// Fixture 'TextMaxLength' for type 'Ark_Number'
extern std::vector<std::tuple<std::string, Ark_Number, std::string>> testFixtureTextMaxLengthValidValues;

// Fixture 'ColorsRes' for type 'Ark_Resource'
extern std::vector<std::tuple<std::string, Ark_Resource, std::string>> testFixtureColorsResValidValues;

// Fixture 'TextAlignType' for type 'Ark_TextAlign'
extern std::vector<std::tuple<std::string, Ark_TextAlign, std::string>> testFixtureTextAlignTypeValidValues;
extern std::vector<std::tuple<std::string, Ark_TextAlign>> testFixtureTextAlignTypeInvalidValues;

// Fixture 'TextInputContentType' for type 'Ark_ContentType'
extern std::vector<std::tuple<std::string, Ark_ContentType, std::string>> testFixtureTextInputContentTypeValidValues;
extern std::vector<std::tuple<std::string, Ark_ContentType>> testFixtureTextInputContentTypeInvalidValues;

// Fixture 'TextInputCancelButtonStyle' for type 'Ark_CancelButtonStyle'
extern std::vector<std::tuple<std::string, Ark_CancelButtonStyle, std::string>>
    testFixtureTextInputCancelButtonStyleValidValues;
extern std::vector<std::tuple<std::string, Ark_CancelButtonStyle>> testFixtureTextInputCancelButtonStyleInvalidValues;

// Fixture 'TextInputLineBreakStrategy' for type 'Ark_LineBreakStrategy'
extern std::vector<std::tuple<std::string, Ark_LineBreakStrategy, std::string>>
    testFixtureTextInputLineBreakStrategyValidValues;
extern std::vector<std::tuple<std::string, Ark_LineBreakStrategy>> testFixtureTextInputLineBreakStrategyInvalidValues;

// Fixture 'TextInputTextContentStyle' for type 'Ark_TextContentStyle'
extern std::vector<std::tuple<std::string, Ark_TextContentStyle, std::string>>
    testFixtureTextInputTextContentStyleValidValues;
extern std::vector<std::tuple<std::string, Ark_TextContentStyle>> testFixtureTextInputTextContentStyleInvalidValues;

// Fixture 'ImageSpanVerticalAlign' for type 'Ark_ImageSpanAlignment'
extern std::vector<std::tuple<std::string, Ark_ImageSpanAlignment, std::string>>
    testFixtureImageSpanVerticalAlignValidValues;
extern std::vector<std::tuple<std::string, Ark_ImageSpanAlignment>> testFixtureImageSpanVerticalAlignInvalidValues;

// Fixture 'ProgressType' for type 'Ark_ProgressType'
extern std::vector<std::tuple<std::string, Ark_ProgressType, std::string>> testFixtureProgressTypeValidValues;
extern std::vector<std::tuple<std::string, Ark_ProgressType>> testFixtureProgressTypeInvalidValues;

// Fixture 'ColorsEnum' for type 'Ark_Color'
extern std::vector<std::tuple<std::string, Ark_Color, std::string>> testFixtureColorsEnumValidValues;
extern std::vector<std::tuple<std::string, Ark_Color>> testFixtureColorsEnumInvalidValues;

// Fixture 'VariableFontWeightEnum' for type 'Ark_FontWeight'
extern std::vector<std::tuple<std::string, Ark_FontWeight, std::string>> testFixtureVariableFontWeightEnumValidValues;
extern std::vector<std::tuple<std::string, Ark_FontWeight>> testFixtureVariableFontWeightEnumInvalidValues;

// Fixture 'LengthNonNegNonPct' for type 'Ark_Length'
extern std::vector<std::tuple<std::string, Ark_Length, std::string>> testFixtureLengthNonNegNonPctValidValues;
extern std::vector<std::tuple<std::string, Ark_Length>> testFixtureLengthNonNegNonPctInvalidValues;

// Fixture 'LengthNonPct' for type 'Ark_Length'
extern std::vector<std::tuple<std::string, Ark_Length, std::string>> testFixtureLengthNonPctValidValues;
extern std::vector<std::tuple<std::string, Ark_Length>> testFixtureLengthNonPctInvalidValues;

// Fixture 'LengthNonNeg' for type 'Ark_Length'
extern std::vector<std::tuple<std::string, Ark_Length, std::string>> testFixtureLengthNonNegValidValues;
extern std::vector<std::tuple<std::string, Ark_Length>> testFixtureLengthNonNegInvalidValues;

// Fixture 'LengthPositive' for type 'Ark_Length'
extern std::vector<std::tuple<std::string, Ark_Length, std::string>> testFixtureLengthPositiveValidValues;
extern std::vector<std::tuple<std::string, Ark_Length>> testFixtureLengthPositiveInvalidValues;

// Fixture 'ResDataForLength' for type 'Ark_Resource'
extern std::vector<std::tuple<std::string, Ark_Resource, std::string>> testFixtureResDataForLengthValidValues;

// Fixture 'LengthResPositive' for type 'Ark_Length'
extern std::vector<std::tuple<std::string, Ark_Length, std::string>> testFixtureLengthResPositiveValidValues;
extern std::vector<std::tuple<std::string, Ark_Length>> testFixtureLengthResPositiveInvalidValues;

// Fixture 'LengthAny' for type 'Ark_Length'
extern std::vector<std::tuple<std::string, Ark_Length, std::string>> testFixtureLengthAnyValidValues;

// Fixture 'LengthMetricsNonNeg' for type 'Ark_LengthMetrics'
extern std::vector<std::tuple<std::string, Ark_LengthMetrics, std::string>> testFixtureLengthMetricsNonNegValidValues;
extern std::vector<std::tuple<std::string, Ark_LengthMetrics>> testFixtureLengthMetricsNonNegInvalidValues;

// Fixture 'BaselineOffsetNum' for type 'Ark_Number'
extern std::vector<std::tuple<std::string, Ark_Number, std::string>> testFixtureBaselineOffsetNumValidValues;

// Fixture 'BaselineOffsetStr' for type 'Ark_String'
extern std::vector<std::tuple<std::string, Ark_String, std::string>> testFixtureBaselineOffsetStrValidValues;

// Fixture 'DimensionsNumNonNeg' for type 'Ark_Number'
extern std::vector<std::tuple<std::string, Ark_Number, std::string>> testFixtureDimensionsNumNonNegValidValues;
extern std::vector<std::tuple<std::string, Ark_Number>> testFixtureDimensionsNumNonNegInvalidValues;

// Fixture 'DimensionsStrNonPerc' for type 'Ark_String'
extern std::vector<std::tuple<std::string, Ark_String, std::string>> testFixtureDimensionsStrNonPercValidValues;
extern std::vector<std::tuple<std::string, Ark_String>> testFixtureDimensionsStrNonPercInvalidValues;

// Fixture 'DimensionsNumAny' for type 'Ark_Number'
extern std::vector<std::tuple<std::string, Ark_Number, std::string>> testFixtureDimensionsNumAnyValidValues;

// Fixture 'DimensionsStrNonNeg' for type 'Ark_String'
extern std::vector<std::tuple<std::string, Ark_String, std::string>> testFixtureDimensionsStrNonNegValidValues;
extern std::vector<std::tuple<std::string, Ark_String>> testFixtureDimensionsStrNonNegInvalidValues;

// Fixture 'DimensionsStrNonNegNonPct' for type 'Ark_String'
extern std::vector<std::tuple<std::string, Ark_String, std::string>> testFixtureDimensionsStrNonNegNonPctValidValues;
extern std::vector<std::tuple<std::string, Ark_String>> testFixtureDimensionsStrNonNegNonPctInvalidValues;

// Fixture 'DimensionsResNonNeg' for type 'Ark_Resource'
extern std::vector<std::tuple<std::string, Ark_Resource, std::string>> testFixtureDimensionsResNonNegValidValues;
extern std::vector<std::tuple<std::string, Ark_Resource>> testFixtureDimensionsResNonNegInvalidValues;

// Fixture 'DimensionsResNonNegNonPct' for type 'Ark_Resource'
extern std::vector<std::tuple<std::string, Ark_Resource, std::string>> testFixtureDimensionsResNonNegNonPctValidValues;
extern std::vector<std::tuple<std::string, Ark_Resource>> testFixtureDimensionsResNonNegNonPctInvalidValues;

// Fixture 'MaxFontScaleNum' for type 'Ark_Number'
extern std::vector<std::tuple<std::string, Ark_Number, std::string>> testFixtureMaxFontScaleNumValidValues;

// Fixture 'MaxFontScaleRes' for type 'Ark_Resource'
extern std::vector<std::tuple<std::string, Ark_Resource, std::string>> testFixtureMaxFontScaleResValidValues;

// Fixture 'MinFontScaleNum' for type 'Ark_Number'
extern std::vector<std::tuple<std::string, Ark_Number, std::string>> testFixtureMinFontScaleNumValidValues;

// Fixture 'MinFontScaleRes' for type 'Ark_Resource'
extern std::vector<std::tuple<std::string, Ark_Resource, std::string>> testFixtureMinFontScaleResValidValues;

// Fixture 'NumberAnything' for type 'Ark_Number'
extern std::vector<std::tuple<std::string, Ark_Number, std::string>> testFixtureNumberAnythingValidValues;

// Fixture 'NumberFloatAnything' for type 'Ark_Number'
extern std::vector<std::tuple<std::string, Ark_Number, std::string>> testFixtureNumberFloatAnythingValidValues;

// Fixture 'NumberFloatPositive' for type 'Ark_Number'
extern std::vector<std::tuple<std::string, Ark_Number, std::string>> testFixtureNumberFloatPositiveValidValues;

// Fixture 'NumberIntFloor' for type 'Ark_Number'
extern std::vector<std::tuple<std::string, Ark_Number, std::string>> testFixtureNumberIntFloorValidValues;

// Fixture 'NumberNonNeg' for type 'Ark_Number'
extern std::vector<std::tuple<std::string, Ark_Number, std::string>> testFixtureNumberNonNegValidValues;
extern std::vector<std::tuple<std::string, Ark_Number>> testFixtureNumberNonNegInvalidValues;

// Fixture 'NumberPosIntFloor' for type 'Ark_Number'
extern std::vector<std::tuple<std::string, Ark_Number, std::string>> testFixtureNumberPosIntFloorValidValues;
extern std::vector<std::tuple<std::string, Ark_Number>> testFixtureNumberPosIntFloorInvalidValues;

// Fixture 'NumberNonNegIntFloor' for type 'Ark_Number'
extern std::vector<std::tuple<std::string, Ark_Number, std::string>> testFixtureNumberNonNegIntFloorValidValues;
extern std::vector<std::tuple<std::string, Ark_Number>> testFixtureNumberNonNegIntFloorInvalidValues;

// Fixture 'NumberNonNegIntCeil' for type 'Ark_Number'
extern std::vector<std::tuple<std::string, Ark_Number, std::string>> testFixtureNumberNonNegIntCeilValidValues;
extern std::vector<std::tuple<std::string, Ark_Number>> testFixtureNumberNonNegIntCeilInvalidValues;

// Fixture 'FlingSpeedLimit' for type 'Ark_Number'
extern std::vector<std::tuple<std::string, Ark_Number, std::string>> testFixtureFlingSpeedLimitValidValues;
extern std::vector<std::tuple<std::string, Ark_Number>> testFixtureFlingSpeedLimitInvalidValues;

// Fixture 'NumberPercentageThresholdFloor' for type 'Ark_Number'
extern std::vector<std::tuple<std::string, Ark_Number, std::string>>
    testFixtureNumberPercentageThresholdFloorValidValues;
extern std::vector<std::tuple<std::string, Ark_Number>> testFixtureNumberPercentageThresholdFloorInvalidValues;

// Fixture 'FontWeightNumbers' for type 'Ark_Number'
extern std::vector<std::tuple<std::string, Ark_Number, std::string>> testFixtureFontWeightNumbersValidValues;
extern std::vector<std::tuple<std::string, Ark_Number>> testFixtureFontWeightNumbersInvalidValues;

// Fixture 'FontWeightStrings' for type 'Ark_String'
extern std::vector<std::tuple<std::string, Ark_String, std::string>> testFixtureFontWeightStringsValidValues;
extern std::vector<std::tuple<std::string, Ark_String>> testFixtureFontWeightStringsInvalidValues;

// Fixture 'ScrollableFrictionNum' for type 'Ark_Number'
extern std::vector<std::tuple<std::string, Ark_Number, std::string>> testFixtureScrollableFrictionNumValidValues;
extern std::vector<std::tuple<std::string, Ark_Number>> testFixtureScrollableFrictionNumInvalidValues;

// Fixture 'ScrollableFrictionRes' for type 'Ark_Resource'
extern std::vector<std::tuple<std::string, Ark_Resource, std::string>> testFixtureScrollableFrictionResValidValues;
extern std::vector<std::tuple<std::string, Ark_Resource>> testFixtureScrollableFrictionResInvalidValues;

// Fixture 'Opacity' for type 'Ark_Number'
extern std::vector<std::tuple<std::string, Ark_Number, std::string>> testFixtureOpacityValidValues;

// Fixture 'OpacityResource' for type 'Ark_Resource'
extern std::vector<std::tuple<std::string, Ark_Resource, std::string>> testFixtureOpacityResourceValidValues;

// Fixture 'FixtureNavBarWidthRange' for type 'Ark_Tuple_Dimension_Dimension'
extern std::vector<std::tuple<std::string, Ark_Tuple_Dimension_Dimension, std::string>>
    testFixtureFixtureNavBarWidthRangeValidValues;
extern std::vector<std::tuple<std::string, Ark_Tuple_Dimension_Dimension>>
    testFixtureFixtureNavBarWidthRangeInvalidValues;

// Fixture 'TextInputBarState' for type 'Ark_BarState'
extern std::vector<std::tuple<std::string, Ark_BarState, std::string>> testFixtureTextInputBarStateValidValues;
extern std::vector<std::tuple<std::string, Ark_BarState>> testFixtureTextInputBarStateInvalidValues;

// Fixture 'TextInputBreakWord' for type 'Ark_WordBreak'
extern std::vector<std::tuple<std::string, Ark_WordBreak, std::string>> testFixtureTextInputBreakWordValidValues;
extern std::vector<std::tuple<std::string, Ark_WordBreak>> testFixtureTextInputBreakWordInvalidValues;

// Fixture 'TextSelectableMode' for type 'Ark_TextSelectableMode'
extern std::vector<std::tuple<std::string, Ark_TextSelectableMode, std::string>>
    testFixtureTextSelectableModeValidValues;
extern std::vector<std::tuple<std::string, Ark_TextSelectableMode>> testFixtureTextSelectableModeInvalidValues;

// Fixture 'FrictionResource' for type 'Ark_Resource'
extern std::vector<std::tuple<std::string, Ark_Resource, std::string>> testFixtureFrictionResourceValidValues;

// Fixture 'TimeFormat' for type 'Ark_String'
extern std::vector<std::tuple<std::string, Ark_String, std::string>> testFixtureTimeFormatValidValues;
extern std::vector<std::tuple<std::string, Ark_String>> testFixtureTimeFormatInvalidValues;

// Fixture 'FontFeature' for type 'Ark_String'
extern std::vector<std::tuple<std::string, Ark_String, std::string>> testFixtureFontFeatureValidValues;
extern std::vector<std::tuple<std::string, Ark_String>> testFixtureFontFeatureInvalidValues;

// Fixture 'TextInputCaretPosition' for type 'Ark_String'
extern std::vector<std::tuple<std::string, Ark_String, std::string>> testFixtureTextInputCaretPositionValidValues;

// Fixture 'ShadowRadiusNumber' for type 'Ark_Number'
extern std::vector<std::tuple<std::string, Ark_Number, std::string>> testFixtureShadowRadiusNumberValidValues;
extern std::vector<std::tuple<std::string, Ark_Number>> testFixtureShadowRadiusNumberInvalidValues;

// Fixture 'ShadowRadiusRes' for type 'Ark_Resource'
extern std::vector<std::tuple<std::string, Ark_Resource, std::string>> testFixtureShadowRadiusResValidValues;
extern std::vector<std::tuple<std::string, Ark_Resource>> testFixtureShadowRadiusResInvalidValues;

// Fixture 'ShadowOffsetNumber' for type 'Ark_Number'
extern std::vector<std::tuple<std::string, Ark_Number, std::string>> testFixtureShadowOffsetNumberValidValues;

// Fixture 'ShadowOffsetRes' for type 'Ark_Resource'
extern std::vector<std::tuple<std::string, Ark_Resource, std::string>> testFixtureShadowOffsetResValidValues;

// Fixture 'TimerInputCount' for type 'Ark_Number'
extern std::vector<std::tuple<std::string, Ark_Number, std::string>> testFixtureTimerInputCountValidValues;
extern std::vector<std::tuple<std::string, Ark_Number>> testFixtureTimerInputCountInvalidValues;

// Fixture 'TextTimerDateFormat' for type 'Ark_String'
extern std::vector<std::tuple<std::string, Ark_String, std::string>> testFixtureTextTimerDateFormatValidValues;
extern std::vector<std::tuple<std::string, Ark_String>> testFixtureTextTimerDateFormatInvalidValues;
} // namespace OHOS::Ace::NG::Fixtures
#endif // GENERATED_FOUNDATION_ACE_CAPI_TEST_FIXTURES_H

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

#include "test_fixtures.h"

#include "modifiers_test_utils.h"

#include "core/components/theme/theme_style.h"

namespace OHOS::Ace::NG::Fixtures {
std::vector<std::tuple<ResID, std::string, ResRawValue>> resourceInitTable = {
    { STRING_RES_0_ID, STRING_RES_0_STR, "abc" },
    { STRING_RES_1_ID, STRING_RES_1_STR, "" },
    { STRING_RES_2_ID, STRING_RES_2_STR, "xyz" },
    { FONT_FAMILY_RESOURCE_0_ID, FONT_FAMILY_RESOURCE_0_STR, "Arial" },
    { FONT_FAMILY_RESOURCE_1_ID, FONT_FAMILY_RESOURCE_1_STR, "Arial, Sans Serif" },
    { FONT_FAMILY_RESOURCE_2_ID, FONT_FAMILY_RESOURCE_2_STR, "" },
    { COLORS_RES_0_ID, COLORS_RES_0_STR, Color(0xA1FAC0DE) },
    { DIMENSIONS_RES_NON_NEG_NON_PCT_0_ID, DIMENSIONS_RES_NON_NEG_NON_PCT_0_STR, 123._vp },
    { DIMENSIONS_RES_NON_NEG_NON_PCT_1_ID, DIMENSIONS_RES_NON_NEG_NON_PCT_1_STR, 0._vp },
    { DIMENSIONS_RES_NON_NEG_NON_PCT_2_ID, DIMENSIONS_RES_NON_NEG_NON_PCT_2_STR, 1.23_vp },
    { DIMENSIONS_RES_NON_NEG_NON_PCT_3_ID, DIMENSIONS_RES_NON_NEG_NON_PCT_3_STR, 123._fp },
    { DIMENSIONS_RES_NON_NEG_NON_PCT_4_ID, DIMENSIONS_RES_NON_NEG_NON_PCT_4_STR, 0._fp },
    { DIMENSIONS_RES_NON_NEG_NON_PCT_5_ID, DIMENSIONS_RES_NON_NEG_NON_PCT_5_STR, 1.23_fp },
    { DIMENSIONS_RES_NON_NEG_NON_PCT_6_ID, DIMENSIONS_RES_NON_NEG_NON_PCT_6_STR, 123._px },
    { DIMENSIONS_RES_NON_NEG_NON_PCT_7_ID, DIMENSIONS_RES_NON_NEG_NON_PCT_7_STR, 0._px },
    { DIMENSIONS_RES_NON_NEG_NON_PCT_8_ID, DIMENSIONS_RES_NON_NEG_NON_PCT_8_STR, 1.23_px },
    { DIMENSIONS_RES_NON_NEG_NON_PCT_9_ID, DIMENSIONS_RES_NON_NEG_NON_PCT_9_STR, -123._vp },
    { DIMENSIONS_RES_NON_NEG_NON_PCT_10_ID, DIMENSIONS_RES_NON_NEG_NON_PCT_10_STR, -123._vp },
    { DIMENSIONS_RES_NON_NEG_NON_PCT_11_ID, DIMENSIONS_RES_NON_NEG_NON_PCT_11_STR, -123._px },
    { DIMENSIONS_RES_NON_NEG_NON_PCT_12_ID, DIMENSIONS_RES_NON_NEG_NON_PCT_12_STR, 0.2_pct },
    { DIMENSIONS_RES_NON_NEG_NON_PCT_13_ID, DIMENSIONS_RES_NON_NEG_NON_PCT_13_STR, -0.2_pct },
    { OPACITY_RESOURCE_0_ID, OPACITY_RESOURCE_0_STR, 0.1f },
    { OPACITY_RESOURCE_1_ID, OPACITY_RESOURCE_1_STR, 0.0f },
    { OPACITY_RESOURCE_2_ID, OPACITY_RESOURCE_2_STR, 0.9f },
    { OPACITY_RESOURCE_3_ID, OPACITY_RESOURCE_3_STR, 1.0f },
    { OPACITY_RESOURCE_4_ID, OPACITY_RESOURCE_4_STR, -0.1f },
    { OPACITY_RESOURCE_5_ID, OPACITY_RESOURCE_5_STR, 100.0f },
    { OPACITY_RESOURCE_6_ID, OPACITY_RESOURCE_6_STR, -100.0f },
    { OPACITY_RESOURCE_7_ID, OPACITY_RESOURCE_7_STR, 1.1f },
};

// Fixture 'Boolean' for type 'Ark_Boolean'
std::vector<std::tuple<std::string, Ark_Boolean, std::string>> testFixtureBooleanValidValues = {
    { "true", Converter::ArkValue<Ark_Boolean>(true), "true" },
    { "false", Converter::ArkValue<Ark_Boolean>(false), "false" },
};

// Fixture 'String' for type 'Ark_String'
std::vector<std::tuple<std::string, Ark_String, std::string>> testFixtureStringValidValues = {
    { "\"abc\"", Converter::ArkValue<Ark_String>("abc"), "abc" },
    { "\"\"", Converter::ArkValue<Ark_String>(""), "" },
    { "\"xyz\"", Converter::ArkValue<Ark_String>("xyz"), "xyz" },
};

// Fixture 'StringRes' for type 'Ark_Resource'
std::vector<std::tuple<std::string, Ark_Resource, std::string>> testFixtureStringResValidValues = {
    { "ResId:STRING_RES_0_ID", CreateResource(STRING_RES_0_ID, NodeModifier::ResourceType::STRING), "abc" },
    { "ResName:STRING_RES_0_STR", CreateResource(STRING_RES_0_STR, NodeModifier::ResourceType::STRING), "abc" },
    { "ResId:STRING_RES_1_ID", CreateResource(STRING_RES_1_ID, NodeModifier::ResourceType::STRING), "" },
    { "ResName:STRING_RES_1_STR", CreateResource(STRING_RES_1_STR, NodeModifier::ResourceType::STRING), "" },
    { "ResId:STRING_RES_2_ID", CreateResource(STRING_RES_2_ID, NodeModifier::ResourceType::STRING), "xyz" },
    { "ResName:STRING_RES_2_STR", CreateResource(STRING_RES_2_STR, NodeModifier::ResourceType::STRING), "xyz" },
};

// Fixture 'FontFamilyResource' for type 'Ark_Resource'
std::vector<std::tuple<std::string, Ark_Resource, std::string>> testFixtureFontFamilyResourceValidValues = {
    { "ResId:FONT_FAMILY_RESOURCE_0_ID", CreateResource(FONT_FAMILY_RESOURCE_0_ID, NodeModifier::ResourceType::STRING),
        "Arial" },
    { "ResName:FONT_FAMILY_RESOURCE_0_STR",
        CreateResource(FONT_FAMILY_RESOURCE_0_STR, NodeModifier::ResourceType::STRING), "Arial" },
    { "ResId:FONT_FAMILY_RESOURCE_1_ID", CreateResource(FONT_FAMILY_RESOURCE_1_ID, NodeModifier::ResourceType::STRING),
        "Arial, Sans Serif" },
    { "ResName:FONT_FAMILY_RESOURCE_1_STR",
        CreateResource(FONT_FAMILY_RESOURCE_1_STR, NodeModifier::ResourceType::STRING), "Arial, Sans Serif" },
};

std::vector<std::tuple<std::string, Ark_Resource>> testFixtureFontFamilyResourceInvalidValues = {
    { "ResId:FONT_FAMILY_RESOURCE_2_ID",
        CreateResource(FONT_FAMILY_RESOURCE_2_ID, NodeModifier::ResourceType::STRING) },
    { "ResName:FONT_FAMILY_RESOURCE_2_STR",
        CreateResource(FONT_FAMILY_RESOURCE_2_STR, NodeModifier::ResourceType::STRING) },
};

// Fixture 'FontFamilyString' for type 'Ark_String'
std::vector<std::tuple<std::string, Ark_String, std::string>> testFixtureFontFamilyStringValidValues = {
    { "\"Arial\"", Converter::ArkValue<Ark_String>("Arial"), "Arial" },
    { "\"Arial, Sans Serif\"", Converter::ArkValue<Ark_String>("Arial, Sans Serif"), "Arial, Sans Serif" },
};

std::vector<std::tuple<std::string, Ark_String>> testFixtureFontFamilyStringInvalidValues = {
    { "\"\"", Converter::ArkValue<Ark_String>("") },
};

// Fixture 'ColorsStr' for type 'Ark_String'
std::vector<std::tuple<std::string, Ark_String, std::string>> testFixtureColorsStrValidValues = {
    { "\"#123\"", Converter::ArkValue<Ark_String>("#123"), "#FF112233" },
    { "\"#11223344\"", Converter::ArkValue<Ark_String>("#11223344"), "#11223344" },
    { "\"#123456\"", Converter::ArkValue<Ark_String>("#123456"), "#FF123456" },
    { "\"65535\"", Converter::ArkValue<Ark_String>("65535"), "#FF00FFFF" },
};

std::vector<std::tuple<std::string, Ark_String>> testFixtureColorsStrInvalidValues = {
    { "\"invalid\"", Converter::ArkValue<Ark_String>("invalid") },
    { "\"\"", Converter::ArkValue<Ark_String>("") },
};

// Fixture 'ColorsNum' for type 'Ark_Number'
std::vector<std::tuple<std::string, Ark_Number, std::string>> testFixtureColorsNumValidValues = {
    { "0", Converter::ArkValue<Ark_Number>(0), "#00000000" },
    { "1", Converter::ArkValue<Ark_Number>(1), "#FF000001" },
    { "65535", Converter::ArkValue<Ark_Number>(65535), "#FF00FFFF" },
    { "-1", Converter::ArkValue<Ark_Number>(-1), "#FFFFFFFF" },
    { "0xBE7AC0DE", Converter::ArkValue<Ark_Number>(0xBE7AC0DE), "#BE7AC0DE" },
};

// Fixture 'ColorsRes' for type 'Ark_Resource'
std::vector<std::tuple<std::string, Ark_Resource, std::string>> testFixtureColorsResValidValues = {
    { "ResId:COLORS_RES_0_ID", CreateResource(COLORS_RES_0_ID, NodeModifier::ResourceType::COLOR), "#A1FAC0DE" },
    { "ResName:COLORS_RES_0_STR", CreateResource(COLORS_RES_0_STR, NodeModifier::ResourceType::COLOR), "#A1FAC0DE" },
};

// Fixture 'ColorsEnum' for type 'Ark_Color'
std::vector<std::tuple<std::string, Ark_Color, std::string>> testFixtureColorsEnumValidValues = {
    { "ARK_COLOR_WHITE", Converter::ArkValue<Ark_Color>(ARK_COLOR_WHITE), "#FFFFFFFF" },
    { "ARK_COLOR_BLACK", Converter::ArkValue<Ark_Color>(ARK_COLOR_BLACK), "#FF000000" },
    { "ARK_COLOR_BLUE", Converter::ArkValue<Ark_Color>(ARK_COLOR_BLUE), "#FF0000FF" },
    { "ARK_COLOR_BROWN", Converter::ArkValue<Ark_Color>(ARK_COLOR_BROWN), "#FFA52A2A" },
    { "ARK_COLOR_GRAY", Converter::ArkValue<Ark_Color>(ARK_COLOR_GRAY), "#FF808080" },
    { "ARK_COLOR_GREEN", Converter::ArkValue<Ark_Color>(ARK_COLOR_GREEN), "#FF008000" },
    { "ARK_COLOR_GREY", Converter::ArkValue<Ark_Color>(ARK_COLOR_GREY), "#FF808080" },
    { "ARK_COLOR_ORANGE", Converter::ArkValue<Ark_Color>(ARK_COLOR_ORANGE), "#FFFFA500" },
    { "ARK_COLOR_PINK", Converter::ArkValue<Ark_Color>(ARK_COLOR_PINK), "#FFFFC0CB" },
    { "ARK_COLOR_RED", Converter::ArkValue<Ark_Color>(ARK_COLOR_RED), "#FFFF0000" },
    { "ARK_COLOR_YELLOW", Converter::ArkValue<Ark_Color>(ARK_COLOR_YELLOW), "#FFFFFF00" },
    { "ARK_COLOR_TRANSPARENT", Converter::ArkValue<Ark_Color>(ARK_COLOR_TRANSPARENT), "#00000000" },
};

std::vector<std::tuple<std::string, Ark_Color>> testFixtureColorsEnumInvalidValues = {
    { "static_cast<Ark_Color>(-1)", Converter::ArkValue<Ark_Color>(static_cast<Ark_Color>(-1)) },
    { "static_cast<Ark_Color>(INT_MAX)", Converter::ArkValue<Ark_Color>(static_cast<Ark_Color>(INT_MAX)) },
};

// Fixture 'LengthNonNegNonPct' for type 'Ark_Length'
std::vector<std::tuple<std::string, Ark_Length, std::string>> testFixtureLengthNonNegNonPctValidValues = {
    { "123.0_vp", Converter::ArkValue<Ark_Length>(123.0_vp), "123.00vp" },
    { "0.0_vp", Converter::ArkValue<Ark_Length>(0.0_vp), "0.00vp" },
    { "1.23_vp", Converter::ArkValue<Ark_Length>(1.23_vp), "1.23vp" },
    { "123.0_fp", Converter::ArkValue<Ark_Length>(123.0_fp), "123.00fp" },
    { "0.0_fp", Converter::ArkValue<Ark_Length>(0.0_fp), "0.00fp" },
    { "1.23_fp", Converter::ArkValue<Ark_Length>(1.23_fp), "1.23fp" },
    { "123.0_px", Converter::ArkValue<Ark_Length>(123.0_px), "123.00px" },
    { "0.0_px", Converter::ArkValue<Ark_Length>(0.0_px), "0.00px" },
    { "1.23_px", Converter::ArkValue<Ark_Length>(1.23_px), "1.23px" },
};

std::vector<std::tuple<std::string, Ark_Length>> testFixtureLengthNonNegNonPctInvalidValues = {
    { "-1", Converter::ArkValue<Ark_Length>(-1) },
    { "-2.f", Converter::ArkValue<Ark_Length>(-2.f) },
    { "-2.3_vp", Converter::ArkValue<Ark_Length>(-2.3_vp) },
    { "-4.5_fp", Converter::ArkValue<Ark_Length>(-4.5_fp) },
    { "-5.6_px", Converter::ArkValue<Ark_Length>(-5.6_px) },
    { "0.5_pct", Converter::ArkValue<Ark_Length>(0.5_pct) },
    { "0.0_pct", Converter::ArkValue<Ark_Length>(0.0_pct) },
    { "-0.8_pct", Converter::ArkValue<Ark_Length>(-0.8_pct) },
};

// Fixture 'LengthNonPct' for type 'Ark_Length'
std::vector<std::tuple<std::string, Ark_Length, std::string>> testFixtureLengthNonPctValidValues = {
    { "123.0_vp", Converter::ArkValue<Ark_Length>(123.0_vp), "123.00vp" },
    { "0.0_vp", Converter::ArkValue<Ark_Length>(0.0_vp), "0.00vp" },
    { "1.23_vp", Converter::ArkValue<Ark_Length>(1.23_vp), "1.23vp" },
    { "123.0_fp", Converter::ArkValue<Ark_Length>(123.0_fp), "123.00fp" },
    { "0.0_fp", Converter::ArkValue<Ark_Length>(0.0_fp), "0.00fp" },
    { "1.23_fp", Converter::ArkValue<Ark_Length>(1.23_fp), "1.23fp" },
    { "123.0_px", Converter::ArkValue<Ark_Length>(123.0_px), "123.00px" },
    { "0.0_px", Converter::ArkValue<Ark_Length>(0.0_px), "0.00px" },
    { "1.23_px", Converter::ArkValue<Ark_Length>(1.23_px), "1.23px" },
    { "-2.3_vp", Converter::ArkValue<Ark_Length>(-2.3_vp), "-2.30vp" },
    { "-4.5_fp", Converter::ArkValue<Ark_Length>(-4.5_fp), "-4.50fp" },
    { "-5.6_px", Converter::ArkValue<Ark_Length>(-5.6_px), "-5.60px" },
};

std::vector<std::tuple<std::string, Ark_Length>> testFixtureLengthNonPctInvalidValues = {
    { "0.5_pct", Converter::ArkValue<Ark_Length>(0.5_pct) },
    { "0.0_pct", Converter::ArkValue<Ark_Length>(0.0_pct) },
    { "-0.8_pct", Converter::ArkValue<Ark_Length>(-0.8_pct) },
};

// Fixture 'LengthNonNeg' for type 'Ark_Length'
std::vector<std::tuple<std::string, Ark_Length, std::string>> testFixtureLengthNonNegValidValues = {
    { "123.0_vp", Converter::ArkValue<Ark_Length>(123.0_vp), "123.00vp" },
    { "0.0_vp", Converter::ArkValue<Ark_Length>(0.0_vp), "0.00vp" },
    { "1.23_vp", Converter::ArkValue<Ark_Length>(1.23_vp), "1.23vp" },
    { "123.0_fp", Converter::ArkValue<Ark_Length>(123.0_fp), "123.00fp" },
    { "0.0_fp", Converter::ArkValue<Ark_Length>(0.0_fp), "0.00fp" },
    { "1.23_fp", Converter::ArkValue<Ark_Length>(1.23_fp), "1.23fp" },
    { "123.0_px", Converter::ArkValue<Ark_Length>(123.0_px), "123.00px" },
    { "0.0_px", Converter::ArkValue<Ark_Length>(0.0_px), "0.00px" },
    { "1.23_px", Converter::ArkValue<Ark_Length>(1.23_px), "1.23px" },
    { "0.5_pct", Converter::ArkValue<Ark_Length>(0.5_pct), "50.00%" },
    { "0.0_pct", Converter::ArkValue<Ark_Length>(0.0_pct), "0.00%" },
};

std::vector<std::tuple<std::string, Ark_Length>> testFixtureLengthNonNegInvalidValues = {
    { "-1", Converter::ArkValue<Ark_Length>(-1) },
    { "-2.f", Converter::ArkValue<Ark_Length>(-2.f) },
    { "-2.3_vp", Converter::ArkValue<Ark_Length>(-2.3_vp) },
    { "-4.5_fp", Converter::ArkValue<Ark_Length>(-4.5_fp) },
    { "-5.6_px", Converter::ArkValue<Ark_Length>(-5.6_px) },
    { "-0.8_pct", Converter::ArkValue<Ark_Length>(-0.8_pct) },
};

// Fixture 'LengthAny' for type 'Ark_Length'
std::vector<std::tuple<std::string, Ark_Length, std::string>> testFixtureLengthAnyValidValues = {
    { "123.0_vp", Converter::ArkValue<Ark_Length>(123.0_vp), "123.00vp" },
    { "0.0_vp", Converter::ArkValue<Ark_Length>(0.0_vp), "0.00vp" },
    { "1.23_vp", Converter::ArkValue<Ark_Length>(1.23_vp), "1.23vp" },
    { "123.0_fp", Converter::ArkValue<Ark_Length>(123.0_fp), "123.00fp" },
    { "0.0_fp", Converter::ArkValue<Ark_Length>(0.0_fp), "0.00fp" },
    { "1.23_fp", Converter::ArkValue<Ark_Length>(1.23_fp), "1.23fp" },
    { "123.0_px", Converter::ArkValue<Ark_Length>(123.0_px), "123.00px" },
    { "0.0_px", Converter::ArkValue<Ark_Length>(0.0_px), "0.00px" },
    { "1.23_px", Converter::ArkValue<Ark_Length>(1.23_px), "1.23px" },
    { "-2.3_vp", Converter::ArkValue<Ark_Length>(-2.3_vp), "-2.30vp" },
    { "-4.5_fp", Converter::ArkValue<Ark_Length>(-4.5_fp), "-4.50fp" },
    { "-5.6_px", Converter::ArkValue<Ark_Length>(-5.6_px), "-5.60px" },
    { "0.5_pct", Converter::ArkValue<Ark_Length>(0.5_pct), "50.00%" },
    { "0.0_pct", Converter::ArkValue<Ark_Length>(0.0_pct), "0.00%" },
    { "-0.8_pct", Converter::ArkValue<Ark_Length>(-0.8_pct), "-80.00%" },
};

// Fixture 'DimensionsNumNonNeg' for type 'Ark_Number'
std::vector<std::tuple<std::string, Ark_Number, std::string>> testFixtureDimensionsNumNonNegValidValues = {
    { "123", Converter::ArkValue<Ark_Number>(123), "123.00vp" },
    { "0", Converter::ArkValue<Ark_Number>(0), "0.00vp" },
    { "1.23", Converter::ArkValue<Ark_Number>(1.23), "1.23vp" },
};

std::vector<std::tuple<std::string, Ark_Number>> testFixtureDimensionsNumNonNegInvalidValues = {
    { "-1", Converter::ArkValue<Ark_Number>(-1) },
};

// Fixture 'DimensionsNumAny' for type 'Ark_Number'
std::vector<std::tuple<std::string, Ark_Number, std::string>> testFixtureDimensionsNumAnyValidValues = {
    { "123", Converter::ArkValue<Ark_Number>(123), "123.00vp" },
    { "0", Converter::ArkValue<Ark_Number>(0), "0.00vp" },
    { "1.23", Converter::ArkValue<Ark_Number>(1.23), "1.23vp" },
    { "-2", Converter::ArkValue<Ark_Number>(-2), "-2.00vp" },
    { "-3.45", Converter::ArkValue<Ark_Number>(-3.45), "-3.45vp" },
};

// Fixture 'DimensionsStrNonNegNonPct' for type 'Ark_String'
std::vector<std::tuple<std::string, Ark_String, std::string>> testFixtureDimensionsStrNonNegNonPctValidValues = {
    { "\"123vp\"", Converter::ArkValue<Ark_String>("123vp"), "123.00vp" },
    { "\"0vp\"", Converter::ArkValue<Ark_String>("0vp"), "0.00vp" },
    { "\"1.23vp\"", Converter::ArkValue<Ark_String>("1.23vp"), "1.23vp" },
    { "\"123fp\"", Converter::ArkValue<Ark_String>("123fp"), "123.00fp" },
    { "\"0fp\"", Converter::ArkValue<Ark_String>("0fp"), "0.00fp" },
    { "\"1.23fp\"", Converter::ArkValue<Ark_String>("1.23fp"), "1.23fp" },
    { "\"123px\"", Converter::ArkValue<Ark_String>("123px"), "123.00px" },
    { "\"0px\"", Converter::ArkValue<Ark_String>("0px"), "0.00px" },
    { "\"1.23px\"", Converter::ArkValue<Ark_String>("1.23px"), "1.23px" },
    { "\"123lpx\"", Converter::ArkValue<Ark_String>("123lpx"), "123.00lpx" },
    { "\"0lpx\"", Converter::ArkValue<Ark_String>("0lpx"), "0.00lpx" },
    { "\"1.23lpx\"", Converter::ArkValue<Ark_String>("1.23lpx"), "1.23lpx" },
};

std::vector<std::tuple<std::string, Ark_String>> testFixtureDimensionsStrNonNegNonPctInvalidValues = {
    { "\"-123vp\"", Converter::ArkValue<Ark_String>("-123vp") },
    { "\"-123vp\"", Converter::ArkValue<Ark_String>("-123vp") },
    { "\"-123px\"", Converter::ArkValue<Ark_String>("-123px") },
    { "\"-123lpx\"", Converter::ArkValue<Ark_String>("-123lpx") },
    { "\"20%\"", Converter::ArkValue<Ark_String>("20%") },
    { "\"-20%\"", Converter::ArkValue<Ark_String>("-20%") },
};

// Fixture 'DimensionsResNonNegNonPct' for type 'Ark_Resource'
std::vector<std::tuple<std::string, Ark_Resource, std::string>> testFixtureDimensionsResNonNegNonPctValidValues = {
    { "ResId:DIMENSIONS_RES_NON_NEG_NON_PCT_0_ID",
        CreateResource(DIMENSIONS_RES_NON_NEG_NON_PCT_0_ID, NodeModifier::ResourceType::FLOAT), "123.00vp" },
    { "ResName:DIMENSIONS_RES_NON_NEG_NON_PCT_0_STR",
        CreateResource(DIMENSIONS_RES_NON_NEG_NON_PCT_0_STR, NodeModifier::ResourceType::FLOAT), "123.00vp" },
    { "ResId:DIMENSIONS_RES_NON_NEG_NON_PCT_1_ID",
        CreateResource(DIMENSIONS_RES_NON_NEG_NON_PCT_1_ID, NodeModifier::ResourceType::FLOAT), "0.00vp" },
    { "ResName:DIMENSIONS_RES_NON_NEG_NON_PCT_1_STR",
        CreateResource(DIMENSIONS_RES_NON_NEG_NON_PCT_1_STR, NodeModifier::ResourceType::FLOAT), "0.00vp" },
    { "ResId:DIMENSIONS_RES_NON_NEG_NON_PCT_2_ID",
        CreateResource(DIMENSIONS_RES_NON_NEG_NON_PCT_2_ID, NodeModifier::ResourceType::FLOAT), "1.23vp" },
    { "ResName:DIMENSIONS_RES_NON_NEG_NON_PCT_2_STR",
        CreateResource(DIMENSIONS_RES_NON_NEG_NON_PCT_2_STR, NodeModifier::ResourceType::FLOAT), "1.23vp" },
    { "ResId:DIMENSIONS_RES_NON_NEG_NON_PCT_3_ID",
        CreateResource(DIMENSIONS_RES_NON_NEG_NON_PCT_3_ID, NodeModifier::ResourceType::FLOAT), "123.00fp" },
    { "ResName:DIMENSIONS_RES_NON_NEG_NON_PCT_3_STR",
        CreateResource(DIMENSIONS_RES_NON_NEG_NON_PCT_3_STR, NodeModifier::ResourceType::FLOAT), "123.00fp" },
    { "ResId:DIMENSIONS_RES_NON_NEG_NON_PCT_4_ID",
        CreateResource(DIMENSIONS_RES_NON_NEG_NON_PCT_4_ID, NodeModifier::ResourceType::FLOAT), "0.00fp" },
    { "ResName:DIMENSIONS_RES_NON_NEG_NON_PCT_4_STR",
        CreateResource(DIMENSIONS_RES_NON_NEG_NON_PCT_4_STR, NodeModifier::ResourceType::FLOAT), "0.00fp" },
    { "ResId:DIMENSIONS_RES_NON_NEG_NON_PCT_5_ID",
        CreateResource(DIMENSIONS_RES_NON_NEG_NON_PCT_5_ID, NodeModifier::ResourceType::FLOAT), "1.23fp" },
    { "ResName:DIMENSIONS_RES_NON_NEG_NON_PCT_5_STR",
        CreateResource(DIMENSIONS_RES_NON_NEG_NON_PCT_5_STR, NodeModifier::ResourceType::FLOAT), "1.23fp" },
    { "ResId:DIMENSIONS_RES_NON_NEG_NON_PCT_6_ID",
        CreateResource(DIMENSIONS_RES_NON_NEG_NON_PCT_6_ID, NodeModifier::ResourceType::FLOAT), "123.00px" },
    { "ResName:DIMENSIONS_RES_NON_NEG_NON_PCT_6_STR",
        CreateResource(DIMENSIONS_RES_NON_NEG_NON_PCT_6_STR, NodeModifier::ResourceType::FLOAT), "123.00px" },
    { "ResId:DIMENSIONS_RES_NON_NEG_NON_PCT_7_ID",
        CreateResource(DIMENSIONS_RES_NON_NEG_NON_PCT_7_ID, NodeModifier::ResourceType::FLOAT), "0.00px" },
    { "ResName:DIMENSIONS_RES_NON_NEG_NON_PCT_7_STR",
        CreateResource(DIMENSIONS_RES_NON_NEG_NON_PCT_7_STR, NodeModifier::ResourceType::FLOAT), "0.00px" },
    { "ResId:DIMENSIONS_RES_NON_NEG_NON_PCT_8_ID",
        CreateResource(DIMENSIONS_RES_NON_NEG_NON_PCT_8_ID, NodeModifier::ResourceType::FLOAT), "1.23px" },
    { "ResName:DIMENSIONS_RES_NON_NEG_NON_PCT_8_STR",
        CreateResource(DIMENSIONS_RES_NON_NEG_NON_PCT_8_STR, NodeModifier::ResourceType::FLOAT), "1.23px" },
};

std::vector<std::tuple<std::string, Ark_Resource>> testFixtureDimensionsResNonNegNonPctInvalidValues = {
    { "ResId:DIMENSIONS_RES_NON_NEG_NON_PCT_9_ID",
        CreateResource(DIMENSIONS_RES_NON_NEG_NON_PCT_9_ID, NodeModifier::ResourceType::FLOAT) },
    { "ResName:DIMENSIONS_RES_NON_NEG_NON_PCT_9_STR",
        CreateResource(DIMENSIONS_RES_NON_NEG_NON_PCT_9_STR, NodeModifier::ResourceType::FLOAT) },
    { "ResId:DIMENSIONS_RES_NON_NEG_NON_PCT_10_ID",
        CreateResource(DIMENSIONS_RES_NON_NEG_NON_PCT_10_ID, NodeModifier::ResourceType::FLOAT) },
    { "ResName:DIMENSIONS_RES_NON_NEG_NON_PCT_10_STR",
        CreateResource(DIMENSIONS_RES_NON_NEG_NON_PCT_10_STR, NodeModifier::ResourceType::FLOAT) },
    { "ResId:DIMENSIONS_RES_NON_NEG_NON_PCT_11_ID",
        CreateResource(DIMENSIONS_RES_NON_NEG_NON_PCT_11_ID, NodeModifier::ResourceType::FLOAT) },
    { "ResName:DIMENSIONS_RES_NON_NEG_NON_PCT_11_STR",
        CreateResource(DIMENSIONS_RES_NON_NEG_NON_PCT_11_STR, NodeModifier::ResourceType::FLOAT) },
    { "ResId:DIMENSIONS_RES_NON_NEG_NON_PCT_12_ID",
        CreateResource(DIMENSIONS_RES_NON_NEG_NON_PCT_12_ID, NodeModifier::ResourceType::FLOAT) },
    { "ResName:DIMENSIONS_RES_NON_NEG_NON_PCT_12_STR",
        CreateResource(DIMENSIONS_RES_NON_NEG_NON_PCT_12_STR, NodeModifier::ResourceType::FLOAT) },
    { "ResId:DIMENSIONS_RES_NON_NEG_NON_PCT_13_ID",
        CreateResource(DIMENSIONS_RES_NON_NEG_NON_PCT_13_ID, NodeModifier::ResourceType::FLOAT) },
    { "ResName:DIMENSIONS_RES_NON_NEG_NON_PCT_13_STR",
        CreateResource(DIMENSIONS_RES_NON_NEG_NON_PCT_13_STR, NodeModifier::ResourceType::FLOAT) },
};

// Fixture 'NumberAnything' for type 'Ark_Number'
std::vector<std::tuple<std::string, Ark_Number, std::string>> testFixtureNumberAnythingValidValues = {
    { "100", Converter::ArkValue<Ark_Number>(100), "100" },
    { "0", Converter::ArkValue<Ark_Number>(0), "0" },
    { "-100", Converter::ArkValue<Ark_Number>(-100), "-100" },
    { "12.34", Converter::ArkValue<Ark_Number>(12.34), "12.34" },
    { "-56.78", Converter::ArkValue<Ark_Number>(-56.78), "-56.78" },
};

// Fixture 'NumberNonNeg' for type 'Ark_Number'
std::vector<std::tuple<std::string, Ark_Number, std::string>> testFixtureNumberNonNegValidValues = {
    { "100", Converter::ArkValue<Ark_Number>(100), "100" },
    { "12.34", Converter::ArkValue<Ark_Number>(12.34), "12.34" },
    { "0", Converter::ArkValue<Ark_Number>(0), "0" },
};

std::vector<std::tuple<std::string, Ark_Number>> testFixtureNumberNonNegInvalidValues = {
    { "-100", Converter::ArkValue<Ark_Number>(-100) },
    { "-56.78", Converter::ArkValue<Ark_Number>(-56.78) },
};

// Fixture 'NumberPosIntFloor' for type 'Ark_Number'
std::vector<std::tuple<std::string, Ark_Number, std::string>> testFixtureNumberPosIntFloorValidValues = {
    { "100", Converter::ArkValue<Ark_Number>(100), "100" },
    { "12.34", Converter::ArkValue<Ark_Number>(12.34), "12" },
    { "1", Converter::ArkValue<Ark_Number>(1), "1" },
};

std::vector<std::tuple<std::string, Ark_Number>> testFixtureNumberPosIntFloorInvalidValues = {
    { "0", Converter::ArkValue<Ark_Number>(0) },
    { "-1", Converter::ArkValue<Ark_Number>(-1) },
    { "-100", Converter::ArkValue<Ark_Number>(-100) },
    { "-23.45", Converter::ArkValue<Ark_Number>(-23.45) },
};

// Fixture 'NumberNonNegIntFloor' for type 'Ark_Number'
std::vector<std::tuple<std::string, Ark_Number, std::string>> testFixtureNumberNonNegIntFloorValidValues = {
    { "100", Converter::ArkValue<Ark_Number>(100), "100" },
    { "12.34", Converter::ArkValue<Ark_Number>(12.34), "12" },
    { "0", Converter::ArkValue<Ark_Number>(0), "0" },
    { "1", Converter::ArkValue<Ark_Number>(1), "1" },
};

std::vector<std::tuple<std::string, Ark_Number>> testFixtureNumberNonNegIntFloorInvalidValues = {
    { "-1", Converter::ArkValue<Ark_Number>(-1) },
    { "-100", Converter::ArkValue<Ark_Number>(-100) },
    { "-23.45", Converter::ArkValue<Ark_Number>(-23.45) },
};

// Fixture 'NumberNonNegIntCeil' for type 'Ark_Number'
std::vector<std::tuple<std::string, Ark_Number, std::string>> testFixtureNumberNonNegIntCeilValidValues = {
    { "100", Converter::ArkValue<Ark_Number>(100), "100" },
    { "12.34", Converter::ArkValue<Ark_Number>(12.34), "13" },
    { "0", Converter::ArkValue<Ark_Number>(0), "0" },
    { "1", Converter::ArkValue<Ark_Number>(1), "1" },
};

std::vector<std::tuple<std::string, Ark_Number>> testFixtureNumberNonNegIntCeilInvalidValues = {
    { "-1", Converter::ArkValue<Ark_Number>(-1) },
    { "-100", Converter::ArkValue<Ark_Number>(-100) },
    { "-23.45", Converter::ArkValue<Ark_Number>(-23.45) },
};

// Fixture 'FontWeightNumbers' for type 'Ark_Number'
std::vector<std::tuple<std::string, Ark_Number, std::string>> testFixtureFontWeightNumbersValidValues = {
    { "100", Converter::ArkValue<Ark_Number>(100), "100" },
    { "200", Converter::ArkValue<Ark_Number>(200), "200" },
    { "300", Converter::ArkValue<Ark_Number>(300), "300" },
    { "400", Converter::ArkValue<Ark_Number>(400), "400" },
    { "500", Converter::ArkValue<Ark_Number>(500), "500" },
    { "600", Converter::ArkValue<Ark_Number>(600), "600" },
    { "700", Converter::ArkValue<Ark_Number>(700), "700" },
    { "800", Converter::ArkValue<Ark_Number>(800), "800" },
    { "900", Converter::ArkValue<Ark_Number>(900), "900" },
};

std::vector<std::tuple<std::string, Ark_Number>> testFixtureFontWeightNumbersInvalidValues = {
    { "0", Converter::ArkValue<Ark_Number>(0) },
    { "1000", Converter::ArkValue<Ark_Number>(1000) },
    { "-100", Converter::ArkValue<Ark_Number>(-100) },
};

// Fixture 'FontWeightStrings' for type 'Ark_String'
std::vector<std::tuple<std::string, Ark_String, std::string>> testFixtureFontWeightStringsValidValues = {
    { "\"100\"", Converter::ArkValue<Ark_String>("100"), "100" },
    { "\"200\"", Converter::ArkValue<Ark_String>("200"), "200" },
    { "\"300\"", Converter::ArkValue<Ark_String>("300"), "300" },
    { "\"400\"", Converter::ArkValue<Ark_String>("400"), "400" },
    { "\"500\"", Converter::ArkValue<Ark_String>("500"), "500" },
    { "\"600\"", Converter::ArkValue<Ark_String>("600"), "600" },
    { "\"700\"", Converter::ArkValue<Ark_String>("700"), "700" },
    { "\"800\"", Converter::ArkValue<Ark_String>("800"), "800" },
    { "\"900\"", Converter::ArkValue<Ark_String>("900"), "900" },
};

std::vector<std::tuple<std::string, Ark_String>> testFixtureFontWeightStringsInvalidValues = {
    { "\"0\"", Converter::ArkValue<Ark_String>("0") },
    { "\"1000\"", Converter::ArkValue<Ark_String>("1000") },
    { "\"-100\"", Converter::ArkValue<Ark_String>("-100") },
};

// Fixture 'Opacity' for type 'Ark_Number'
std::vector<std::tuple<std::string, Ark_Number, std::string>> testFixtureOpacityValidValues = {
    { "0.1f", Converter::ArkValue<Ark_Number>(0.1f), "0.100000" },
    { "0", Converter::ArkValue<Ark_Number>(0), "0.000000" },
    { "0.9f", Converter::ArkValue<Ark_Number>(0.9f), "0.900000" },
    { "1", Converter::ArkValue<Ark_Number>(1), "1.000000" },
    { "-0.1f", Converter::ArkValue<Ark_Number>(-0.1f), "0.000000" },
    { "100", Converter::ArkValue<Ark_Number>(100), "1.000000" },
    { "-100", Converter::ArkValue<Ark_Number>(-100), "0.000000" },
    { "1.1f", Converter::ArkValue<Ark_Number>(1.1f), "1.000000" },
};

// Fixture 'OpacityResource' for type 'Ark_Resource'
std::vector<std::tuple<std::string, Ark_Resource, std::string>> testFixtureOpacityResourceValidValues = {
    { "ResId:OPACITY_RESOURCE_0_ID", CreateResource(OPACITY_RESOURCE_0_ID, NodeModifier::ResourceType::FLOAT),
        "0.100000" },
    { "ResName:OPACITY_RESOURCE_0_STR", CreateResource(OPACITY_RESOURCE_0_STR, NodeModifier::ResourceType::FLOAT),
        "0.100000" },
    { "ResId:OPACITY_RESOURCE_1_ID", CreateResource(OPACITY_RESOURCE_1_ID, NodeModifier::ResourceType::FLOAT),
        "0.000000" },
    { "ResName:OPACITY_RESOURCE_1_STR", CreateResource(OPACITY_RESOURCE_1_STR, NodeModifier::ResourceType::FLOAT),
        "0.000000" },
    { "ResId:OPACITY_RESOURCE_2_ID", CreateResource(OPACITY_RESOURCE_2_ID, NodeModifier::ResourceType::FLOAT),
        "0.900000" },
    { "ResName:OPACITY_RESOURCE_2_STR", CreateResource(OPACITY_RESOURCE_2_STR, NodeModifier::ResourceType::FLOAT),
        "0.900000" },
    { "ResId:OPACITY_RESOURCE_3_ID", CreateResource(OPACITY_RESOURCE_3_ID, NodeModifier::ResourceType::FLOAT),
        "1.000000" },
    { "ResName:OPACITY_RESOURCE_3_STR", CreateResource(OPACITY_RESOURCE_3_STR, NodeModifier::ResourceType::FLOAT),
        "1.000000" },
    { "ResId:OPACITY_RESOURCE_4_ID", CreateResource(OPACITY_RESOURCE_4_ID, NodeModifier::ResourceType::FLOAT),
        "0.000000" },
    { "ResName:OPACITY_RESOURCE_4_STR", CreateResource(OPACITY_RESOURCE_4_STR, NodeModifier::ResourceType::FLOAT),
        "0.000000" },
    { "ResId:OPACITY_RESOURCE_5_ID", CreateResource(OPACITY_RESOURCE_5_ID, NodeModifier::ResourceType::FLOAT),
        "1.000000" },
    { "ResName:OPACITY_RESOURCE_5_STR", CreateResource(OPACITY_RESOURCE_5_STR, NodeModifier::ResourceType::FLOAT),
        "1.000000" },
    { "ResId:OPACITY_RESOURCE_6_ID", CreateResource(OPACITY_RESOURCE_6_ID, NodeModifier::ResourceType::FLOAT),
        "0.000000" },
    { "ResName:OPACITY_RESOURCE_6_STR", CreateResource(OPACITY_RESOURCE_6_STR, NodeModifier::ResourceType::FLOAT),
        "0.000000" },
    { "ResId:OPACITY_RESOURCE_7_ID", CreateResource(OPACITY_RESOURCE_7_ID, NodeModifier::ResourceType::FLOAT),
        "1.000000" },
    { "ResName:OPACITY_RESOURCE_7_STR", CreateResource(OPACITY_RESOURCE_7_STR, NodeModifier::ResourceType::FLOAT),
        "1.000000" },
};

} // namespace OHOS::Ace::NG::Fixtures

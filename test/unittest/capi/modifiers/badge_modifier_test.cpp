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

#include "badge_modifier_test.h"

#include <gtest/gtest.h>

#include "arkoala_api_generated.h"
#include "modifier_test_base.h"
#include "modifiers_test_utils.h"

#include "core/components/badge/badge_theme.h"
#include "core/interfaces/native/utility/converter.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {

const auto ATTRIBUTE_POSITION_NAME = "position";
const auto ATTRIBUTE_POSITION_DEFAULT_VALUE = "BadgePosition.RightTop";
const auto ATTRIBUTE_SET_STYLE_NAME = "style";
const auto ATTRIBUTE_SET_STYLE_X_NAME = "x";
const auto ATTRIBUTE_SET_STYLE_X_DEFAULT_VALUE = "0.00vp";
const auto ATTRIBUTE_SET_STYLE_Y_NAME = "y";
const auto ATTRIBUTE_SET_STYLE_Y_DEFAULT_VALUE = "0.00vp";
const auto ATTRIBUTE_SET_STYLE_COLOR_NAME = "color";
const auto ATTRIBUTE_SET_STYLE_COLOR_DEFAULT_VALUE = "#FFFFFFFF";
const auto ATTRIBUTE_SET_STYLE_FONT_SIZE_NAME = "fontSize";
const auto ATTRIBUTE_SET_STYLE_FONT_SIZE_DEFAULT_VALUE = "10.00vp";
const auto ATTRIBUTE_SET_STYLE_BADGE_COLOR_NAME = "badgeColor";
const auto ATTRIBUTE_SET_STYLE_BADGE_COLOR_DEFAULT_VALUE = "#FFFF0000";
const auto ATTRIBUTE_SET_STYLE_BADGE_SIZE_NAME = "badgeSize";
const auto ATTRIBUTE_SET_STYLE_BADGE_SIZE_DEFAULT_VALUE = "16.00vp";
const auto ATTRIBUTE_SET_STYLE_BORDER_COLOR_NAME = "borderColor";
const auto ATTRIBUTE_SET_STYLE_BORDER_COLOR_DEFAULT_VALUE = "#FF000000";
const auto ATTRIBUTE_SET_STYLE_BORDER_WIDTH_NAME = "borderWidth";
const auto ATTRIBUTE_SET_STYLE_BORDER_WIDTH_DEFAULT_VALUE = "0.00px";
const auto ATTRIBUTE_SET_STYLE_FONT_WEIGHT_NAME = "fontWeight";
const auto ATTRIBUTE_SET_STYLE_FONT_WEIGHT_DEFAULT_VALUE = "FontWeight.Normal";
const auto ATTRIBUTE_COUNT_NAME = "count";
const auto ATTRIBUTE_COUNT_DEFAULT_VALUE = "0";
const auto ATTRIBUTE_MAX_COUNT_NAME = "maxCount";
const auto ATTRIBUTE_MAX_COUNT_DEFAULT_VALUE = "99";
const auto ATTRIBUTE_VALUE_NAME = "value";
const auto ATTRIBUTE_VALUE_DEFAULT_VALUE = "";

// resource names and id
const auto RES_COLOR_NAME = NamedResourceId{"color_name", Converter::ResourceType::COLOR};
const auto COLOR_BY_STRING = Color(0xFF123456);

template<typename T>
void FillEmptyOptions(T& options)
{
    options.position = Converter::ArkValue<Opt_Union_BadgePosition_Position>(Ark_Empty());
    options.style = {
        .color = Converter::ArkValue<Opt_ResourceColor>(Ark_Empty()),
        .fontSize = Converter::ArkValue<Opt_Union_Number_String>(Ark_Empty()),
        .badgeSize = Converter::ArkValue<Opt_Union_Number_String>(Ark_Empty()),
        .badgeColor = Converter::ArkValue<Opt_ResourceColor>(Ark_Empty()),
        .borderColor = Converter::ArkValue<Opt_ResourceColor>(Ark_Empty()),
        .borderWidth = Converter::ArkValue<Opt_Length>(Ark_Empty()),
        .fontWeight = Converter::ArkValue<Opt_Union_Number_FontWeight_String>(Ark_Empty()),
    };
}

void InitStringOptions(Ark_BadgeParamWithString& options)
{
    FillEmptyOptions(options);
    options.value = Converter::ArkValue<Ark_String>("");
}

void InitNumberOptions(Ark_BadgeParamWithNumber& options)
{
    FillEmptyOptions(options);
    options.maxCount = Converter::ArkValue<Opt_Number>(Ark_Empty());
    options.count = Converter::ArkValue<Ark_Number>(0);
}

} // namespace

namespace Converter {

void AssignArkValue(Ark_FontWeight& dst, const FontWeight& src)
{
    switch (src) {
        case FontWeight::W100: dst = Ark_FontWeight::ARK_FONT_WEIGHT_LIGHTER; break;
        case FontWeight::W400: dst = Ark_FontWeight::ARK_FONT_WEIGHT_NORMAL; break;
        case FontWeight::W700: dst = Ark_FontWeight::ARK_FONT_WEIGHT_BOLD; break;
        case FontWeight::W900: dst = Ark_FontWeight::ARK_FONT_WEIGHT_BOLDER; break;
        case FontWeight::LIGHTER: dst = Ark_FontWeight::ARK_FONT_WEIGHT_LIGHTER; break;
        case FontWeight::NORMAL: dst = Ark_FontWeight::ARK_FONT_WEIGHT_NORMAL; break;
        case FontWeight::REGULAR: dst = Ark_FontWeight::ARK_FONT_WEIGHT_REGULAR; break;
        case FontWeight::MEDIUM: dst = Ark_FontWeight::ARK_FONT_WEIGHT_MEDIUM; break;
        case FontWeight::BOLD: dst = Ark_FontWeight::ARK_FONT_WEIGHT_BOLD; break;
        case FontWeight::BOLDER: dst = Ark_FontWeight::ARK_FONT_WEIGHT_BOLDER; break;
        default:
            dst = static_cast<Ark_FontWeight>(-1);
            LOGE("Unexpected enum value in Ark_FontWeight: %{public}d", src);
    }
}

} // namespace Converter

class BadgeModifierTest : public ModifierTestBase<GENERATED_ArkUIBadgeModifier,
                              &GENERATED_ArkUINodeModifiers::getBadgeModifier, GENERATED_ARKUI_BADGE> {
public:
    static void SetUpTestCase()
    {
        ModifierTestBase::SetUpTestCase();

        SetupTheme<BadgeTheme>();

        AddResource(RES_COLOR_NAME, COLOR_BY_STRING);
    }
};

typedef std::pair<std::string, std::string> TestVector;

static const std::vector<TestVector> DEFAULT_0_TEST1_PLAN = {
    { ATTRIBUTE_POSITION_NAME, ATTRIBUTE_POSITION_DEFAULT_VALUE },
    { ATTRIBUTE_COUNT_NAME, ATTRIBUTE_COUNT_DEFAULT_VALUE },
    { ATTRIBUTE_MAX_COUNT_NAME, ATTRIBUTE_MAX_COUNT_DEFAULT_VALUE },
    { ATTRIBUTE_VALUE_NAME, ATTRIBUTE_VALUE_DEFAULT_VALUE },
};

static const std::vector<TestVector> DEFAULT_0_TEST2_PLAN = {
    { ATTRIBUTE_SET_STYLE_X_NAME, ATTRIBUTE_SET_STYLE_X_DEFAULT_VALUE },
    { ATTRIBUTE_SET_STYLE_Y_NAME, ATTRIBUTE_SET_STYLE_Y_DEFAULT_VALUE },
    { ATTRIBUTE_SET_STYLE_COLOR_NAME, ATTRIBUTE_SET_STYLE_COLOR_DEFAULT_VALUE },
    { ATTRIBUTE_SET_STYLE_FONT_SIZE_NAME, ATTRIBUTE_SET_STYLE_FONT_SIZE_DEFAULT_VALUE },
    { ATTRIBUTE_SET_STYLE_BADGE_COLOR_NAME, ATTRIBUTE_SET_STYLE_BADGE_COLOR_DEFAULT_VALUE },
    { ATTRIBUTE_SET_STYLE_BADGE_SIZE_NAME, ATTRIBUTE_SET_STYLE_BADGE_SIZE_DEFAULT_VALUE },
    { ATTRIBUTE_SET_STYLE_BORDER_COLOR_NAME, ATTRIBUTE_SET_STYLE_BORDER_COLOR_DEFAULT_VALUE },
    { ATTRIBUTE_SET_STYLE_BORDER_WIDTH_NAME, ATTRIBUTE_SET_STYLE_BORDER_WIDTH_DEFAULT_VALUE },
    { ATTRIBUTE_SET_STYLE_FONT_WEIGHT_NAME, ATTRIBUTE_SET_STYLE_FONT_WEIGHT_DEFAULT_VALUE },
};

/*
 * @tc.name: setBadgeOptionsTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(BadgeModifierTest, setBadgeOptionsTestDefaultValues, TestSize.Level1)
{
    auto jsonValue = GetJsonValue(node_);
    jsonValue->Delete("position");

    auto badgeStyleAttrs = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_SET_STYLE_NAME);

    std::string strResult;
    for (const auto& [key, expected] : DEFAULT_0_TEST1_PLAN) {
        strResult = jsonValue->GetString(key);
        EXPECT_EQ(strResult, expected);
    }

    for (const auto& [key, expected] : DEFAULT_0_TEST2_PLAN) {
        strResult = badgeStyleAttrs->GetString(key);
        EXPECT_EQ(strResult, expected);
    }
}

static const std::vector<TestVector> EMPTY_0_TEST1_PLAN = {
    { ATTRIBUTE_POSITION_NAME, "BadgePosition.RightTop" },
    { ATTRIBUTE_COUNT_NAME, "0" },
    { ATTRIBUTE_MAX_COUNT_NAME, "99" },
};

static const std::vector<TestVector> EMPTY_0_TEST2_PLAN = {
    { ATTRIBUTE_SET_STYLE_X_NAME, "0.00vp" },
    { ATTRIBUTE_SET_STYLE_Y_NAME, "0.00vp" },
    { ATTRIBUTE_SET_STYLE_COLOR_NAME, "#FF000000" },
    { ATTRIBUTE_SET_STYLE_FONT_SIZE_NAME, "10.00vp" },
    { ATTRIBUTE_SET_STYLE_BADGE_COLOR_NAME, "#FF000000" },
    { ATTRIBUTE_SET_STYLE_BADGE_SIZE_NAME, "16.00vp" },
    { ATTRIBUTE_SET_STYLE_BORDER_COLOR_NAME, "#FF000000" },
    { ATTRIBUTE_SET_STYLE_BORDER_WIDTH_NAME, "0.00px" },
    { ATTRIBUTE_SET_STYLE_FONT_WEIGHT_NAME, "FontWeight.Normal" },
};

/*
 * @tc.name: setBadgeOptions0TestEmptyValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(BadgeModifierTest, setBadgeOptions0TestEmptyValues, TestSize.Level1)
{
    Ark_BadgeParamWithNumber inputValueOptions;
    InitNumberOptions(inputValueOptions);

    modifier_->setBadgeOptions0(node_, &inputValueOptions);

    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    jsonValue->Delete("position");
    auto badgeStyleAttrs = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_SET_STYLE_NAME);

    std::string strResult;
    for (const auto& [key, expected] : EMPTY_0_TEST1_PLAN) {
        strResult = jsonValue->GetString(key);
        EXPECT_EQ(strResult, expected);
    }

    for (const auto& [key, expected] : EMPTY_0_TEST2_PLAN) {
        strResult = badgeStyleAttrs->GetString(key);
        EXPECT_EQ(strResult, expected);
    }
}

static const std::vector<TestVector> VALID_0_TEST1_PLAN = {
    { ATTRIBUTE_POSITION_NAME, "BadgePosition.RightTop" },
    { ATTRIBUTE_COUNT_NAME, "4" },
    { ATTRIBUTE_MAX_COUNT_NAME, "20" },
};

static const std::vector<TestVector> VALID_0_TEST2_PLAN = {
    { ATTRIBUTE_SET_STYLE_X_NAME, "12.00px" },
    { ATTRIBUTE_SET_STYLE_Y_NAME, "14.00px" },
    { ATTRIBUTE_SET_STYLE_COLOR_NAME, "#FF808080" },
    { ATTRIBUTE_SET_STYLE_FONT_SIZE_NAME, "8.00vp" },
    { ATTRIBUTE_SET_STYLE_BADGE_COLOR_NAME, "#FF00FF00" },
    { ATTRIBUTE_SET_STYLE_BADGE_SIZE_NAME, "32.00vp" },
    { ATTRIBUTE_SET_STYLE_BORDER_COLOR_NAME, "#FF0000FF" },
    { ATTRIBUTE_SET_STYLE_BORDER_WIDTH_NAME, "2.45vp" },
    { ATTRIBUTE_SET_STYLE_FONT_WEIGHT_NAME, "FontWeight.Medium" },
};

/*
 * @tc.name: setBadgeOptions0TestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(BadgeModifierTest, setBadgeOptions0TestValidValues, TestSize.Level1)
{
    Ark_BadgeParamWithNumber inputValueOptions;
    InitNumberOptions(inputValueOptions);
    Ark_Position position;
    position.x = Converter::ArkValue<Opt_Length>("12.0px");
    position.y = Converter::ArkValue<Opt_Length>("14.0px");

    inputValueOptions.position = Converter::ArkUnion<Opt_Union_BadgePosition_Position, Ark_Position>(position);
    inputValueOptions.style = {
        .color =
            Converter::ArkValue<Opt_ResourceColor>(Converter::ArkUnion<Ark_ResourceColor, Ark_Color>(ARK_COLOR_GRAY)),
        .fontSize = Converter::ArkUnion<Opt_Union_Number_String, Ark_String>("8.00vp"),
        .badgeSize = Converter::ArkUnion<Opt_Union_Number_String, Ark_Number>(32.00f),
        .badgeColor =
            Converter::ArkValue<Opt_ResourceColor>(Converter::ArkUnion<Ark_ResourceColor, Ark_String>("#FF00FF00")),
        .borderColor =
            Converter::ArkValue<Opt_ResourceColor>(Converter::ArkUnion<Ark_ResourceColor, Ark_Number>(0xFF0000FF)),
        .borderWidth = Converter::ArkValue<Opt_Length>(Dimension(2.45f, DimensionUnit::VP)),
        .fontWeight = Converter::ArkUnion<Opt_Union_Number_FontWeight_String, Ark_FontWeight>(
            Converter::ArkValue<Ark_FontWeight>(FontWeight::MEDIUM)),
    };
    inputValueOptions.count = Converter::ArkValue<Ark_Number>(4);
    inputValueOptions.maxCount = Converter::ArkValue<Opt_Number>(20);

    modifier_->setBadgeOptions0(node_, &inputValueOptions);

    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    jsonValue->Delete("position");
    auto badgeStyleAttrs = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_SET_STYLE_NAME);

    std::string strResult;
    for (const auto& [key, expected] : VALID_0_TEST1_PLAN) {
        strResult = jsonValue->GetString(key);
        EXPECT_EQ(strResult, expected);
    }

    for (const auto& [key, expected] : VALID_0_TEST2_PLAN) {
        strResult = badgeStyleAttrs->GetString(key);
        EXPECT_EQ(strResult, expected);
    }
}

static const std::vector<TestVector> INVALID_0_TEST1_PLAN = {
    { ATTRIBUTE_POSITION_NAME, "BadgePosition.RightTop" },
    { ATTRIBUTE_COUNT_NAME, "-1" },
    { ATTRIBUTE_MAX_COUNT_NAME, "-100" },
};

static const std::vector<TestVector> INVALID_0_TEST2_PLAN = {
    { ATTRIBUTE_SET_STYLE_X_NAME, "0.00vp" },
    { ATTRIBUTE_SET_STYLE_Y_NAME, "0.00vp" },
    { ATTRIBUTE_SET_STYLE_COLOR_NAME, "#FF000000" },
    { ATTRIBUTE_SET_STYLE_FONT_SIZE_NAME, "10.00vp" },
    { ATTRIBUTE_SET_STYLE_BADGE_COLOR_NAME, "#FF000000" },
    { ATTRIBUTE_SET_STYLE_BADGE_SIZE_NAME, "16.00vp" },
    { ATTRIBUTE_SET_STYLE_BORDER_COLOR_NAME, "#FF000000" },
    { ATTRIBUTE_SET_STYLE_BORDER_WIDTH_NAME, "0.00px" },
    { ATTRIBUTE_SET_STYLE_FONT_WEIGHT_NAME, "FontWeight.Normal" },
};

/*
 * @tc.name: setBadgeOptions0TestInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(BadgeModifierTest, setBadgeOptions0TestInvalidValues, TestSize.Level1)
{
    Ark_BadgeParamWithNumber inputValueOptions;
    InitNumberOptions(inputValueOptions);

    Ark_Position position;
    position.x = Converter::ArkValue<Opt_Length>(Dimension(-12.00));
    position.y = Converter::ArkValue<Opt_Length>(Dimension(-14.00));

    inputValueOptions.position = Converter::ArkUnion<Opt_Union_BadgePosition_Position, Ark_Position>(position);
    inputValueOptions.style = {
        .color =
            Converter::ArkValue<Opt_ResourceColor>(Converter::ArkUnion<Ark_ResourceColor, Ark_String>("invalid color")),
        .fontSize = Converter::ArkUnion<Opt_Union_Number_String, Ark_String>("-8.00vp"),
        .badgeSize = Converter::ArkUnion<Opt_Union_Number_String, Ark_String>("-32.00vp"),
        .badgeColor =
            Converter::ArkValue<Opt_ResourceColor>(Converter::ArkUnion<Ark_ResourceColor, Ark_String>("-100 color")),
        .borderColor =
            Converter::ArkValue<Opt_ResourceColor>(Converter::ArkUnion<Ark_ResourceColor, Ark_String>("no color")),
        .borderWidth = Converter::ArkValue<Opt_Length>(Dimension(-2.45f, DimensionUnit::VP)),
        .fontWeight = Converter::ArkUnion<Opt_Union_Number_FontWeight_String, Ark_FontWeight>(
            Converter::ArkValue<Ark_FontWeight>(static_cast<FontWeight>(-100))),
    };
    inputValueOptions.count = Converter::ArkValue<Ark_Number>(-1);
    inputValueOptions.maxCount = Converter::ArkValue<Opt_Number>(-100);

    modifier_->setBadgeOptions0(node_, &inputValueOptions);

    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    jsonValue->Delete("position");
    auto badgeStyleAttrs = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_SET_STYLE_NAME);

    std::string strResult;
    for (const auto& [key, expected] : INVALID_0_TEST1_PLAN) {
        strResult = jsonValue->GetString(key);
        EXPECT_EQ(strResult, expected);
    }

    for (const auto& [key, expected] : INVALID_0_TEST2_PLAN) {
        strResult = badgeStyleAttrs->GetString(key);
        EXPECT_EQ(strResult, expected);
    }
}

static const std::vector<TestVector> EMPTY_1_TEST1_PLAN = {
    { ATTRIBUTE_POSITION_NAME, "BadgePosition.RightTop" },
    { ATTRIBUTE_VALUE_NAME, "" },
};

static const std::vector<TestVector> EMPTY_1_TEST2_PLAN = {
    { ATTRIBUTE_SET_STYLE_X_NAME, "0.00vp" },
    { ATTRIBUTE_SET_STYLE_Y_NAME, "0.00vp" },
    { ATTRIBUTE_SET_STYLE_COLOR_NAME, "#FF000000" },
    { ATTRIBUTE_SET_STYLE_FONT_SIZE_NAME, "10.00vp" },
    { ATTRIBUTE_SET_STYLE_BADGE_COLOR_NAME, "#FF000000" },
    { ATTRIBUTE_SET_STYLE_BADGE_SIZE_NAME, "16.00vp" },
    { ATTRIBUTE_SET_STYLE_BORDER_COLOR_NAME, "#FF000000" },
    { ATTRIBUTE_SET_STYLE_BORDER_WIDTH_NAME, "0.00px" },
    { ATTRIBUTE_SET_STYLE_FONT_WEIGHT_NAME, "FontWeight.Normal" },
};

/*
 * @tc.name: setBadgeOptions1TestEmptyValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(BadgeModifierTest, setBadgeOptions1TestEmptyValues, TestSize.Level1)
{
    Ark_BadgeParamWithString inputValueOptions;
    InitStringOptions(inputValueOptions);

    modifier_->setBadgeOptions1(node_, &inputValueOptions);

    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    jsonValue->Delete("position");
    auto badgeStyleAttrs = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_SET_STYLE_NAME);

    std::string strResult;
    for (const auto& [key, expected] : EMPTY_1_TEST1_PLAN) {
        strResult = jsonValue->GetString(key);
        EXPECT_EQ(strResult, expected);
    }

    for (const auto& [key, expected] : EMPTY_1_TEST2_PLAN) {
        strResult = badgeStyleAttrs->GetString(key);
        EXPECT_EQ(strResult, expected);
    }
}

static const std::vector<TestVector> VALID_1_TEST1_PLAN = {
    { ATTRIBUTE_POSITION_NAME, "BadgePosition.Left" },
    { ATTRIBUTE_VALUE_NAME, "badge_value" },
};

static const std::vector<TestVector> VALID_1_TEST2_PLAN = {
    { ATTRIBUTE_SET_STYLE_X_NAME, "0.00vp" },
    { ATTRIBUTE_SET_STYLE_Y_NAME, "0.00vp" },
    { ATTRIBUTE_SET_STYLE_COLOR_NAME, "#FF00FFFF" },
    { ATTRIBUTE_SET_STYLE_FONT_SIZE_NAME, "28.00vp" },
    { ATTRIBUTE_SET_STYLE_BADGE_COLOR_NAME, "#FF0000FF" },
    { ATTRIBUTE_SET_STYLE_BADGE_SIZE_NAME, "32.00px" },
    { ATTRIBUTE_SET_STYLE_BORDER_COLOR_NAME, "#FF123456" },
    { ATTRIBUTE_SET_STYLE_BORDER_WIDTH_NAME, "10.00px" },
    { ATTRIBUTE_SET_STYLE_FONT_WEIGHT_NAME, "100" },
};

/*
 * @tc.name: setBadgeOptions1TestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(BadgeModifierTest, setBadgeOptions1TestValidValues, TestSize.Level1)
{
    Ark_BadgeParamWithString inputValueOptions;
    InitStringOptions(inputValueOptions);

    Ark_Position position;
    position.x = Converter::ArkValue<Opt_Length>(16.00f);
    position.y = Converter::ArkValue<Opt_Length>(24.00f);

    inputValueOptions.position = Converter::ArkUnion<Opt_Union_BadgePosition_Position, Ark_BadgePosition>(
        Converter::ArkValue<Ark_BadgePosition>(BadgePosition::LEFT)
        );
    inputValueOptions.style = {
        .color = Converter::ArkValue<Opt_ResourceColor>(Converter::ArkUnion<Ark_ResourceColor, Ark_Number>(0xFF00FFFF)),
        .fontSize = Converter::ArkUnion<Opt_Union_Number_String, Ark_Number>(28.00f),
        .badgeSize = Converter::ArkUnion<Opt_Union_Number_String, Ark_String>("32.00px"),
        .badgeColor =
            Converter::ArkValue<Opt_ResourceColor>(Converter::ArkUnion<Ark_ResourceColor, Ark_Color>(ARK_COLOR_BLUE)),
        .borderColor =Converter::ArkValue<Opt_ResourceColor>(
            Converter::ArkUnion<Ark_ResourceColor, Ark_Resource>(CreateResource(RES_COLOR_NAME))),
        .borderWidth = Converter::ArkValue<Opt_Length>(10),
        .fontWeight = Converter::ArkUnion<Opt_Union_Number_FontWeight_String, Ark_String>("100"),
    };
    inputValueOptions.value = Converter::ArkValue<Ark_String>("badge_value");

    modifier_->setBadgeOptions1(node_, &inputValueOptions);

    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    jsonValue->Delete("position");
    auto badgeStyleAttrs = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_SET_STYLE_NAME);

    std::string strResult;
    for (const auto& [key, expected] : VALID_1_TEST1_PLAN) {
        strResult = jsonValue->GetString(key);
        EXPECT_EQ(strResult, expected);
    }

    for (const auto& [key, expected] : VALID_1_TEST2_PLAN) {
        strResult = badgeStyleAttrs->GetString(key);
        EXPECT_EQ(strResult, expected);
    }
}

static const std::vector<TestVector> INVALID_1_TEST1_PLAN = {
    { ATTRIBUTE_POSITION_NAME, "BadgePosition.RightTop" },
    { ATTRIBUTE_VALUE_NAME, "" },
};

static const std::vector<TestVector> INVALID_1_TEST2_PLAN = {
    { ATTRIBUTE_SET_STYLE_X_NAME, "0.00vp" },
    { ATTRIBUTE_SET_STYLE_Y_NAME, "0.00vp" },
    { ATTRIBUTE_SET_STYLE_COLOR_NAME, "#FF000000" },
    { ATTRIBUTE_SET_STYLE_FONT_SIZE_NAME, "10.00vp" },
    { ATTRIBUTE_SET_STYLE_BADGE_COLOR_NAME, "#FF000000" },
    { ATTRIBUTE_SET_STYLE_BADGE_SIZE_NAME, "16.00vp" },
    { ATTRIBUTE_SET_STYLE_BORDER_COLOR_NAME, "#FF000000" },
    { ATTRIBUTE_SET_STYLE_BORDER_WIDTH_NAME, "0.00px" },
    { ATTRIBUTE_SET_STYLE_FONT_WEIGHT_NAME, "FontWeight.Normal" },
};

/*
 * @tc.name: setBadgeOptions1TestInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(BadgeModifierTest, setBadgeOptions1TestInvalidValues, TestSize.Level1)
{
    Ark_BadgeParamWithString inputValueOptions;
    InitStringOptions(inputValueOptions);

    Ark_Position position;
    position.x = Converter::ArkValue<Opt_Length>(Dimension(-12.00, DimensionUnit::PX));
    position.y = Converter::ArkValue<Opt_Length>(Dimension(-10.00, DimensionUnit::PX));

    inputValueOptions.position = Converter::ArkUnion<Opt_Union_BadgePosition_Position, Ark_Position>(position);
    inputValueOptions.style = {
        .color =
            Converter::ArkValue<Opt_ResourceColor>(Converter::ArkUnion<Ark_ResourceColor, Ark_String>("invalid color")),
        .fontSize = Converter::ArkUnion<Opt_Union_Number_String, Ark_String>("50%"),
        .badgeSize = Converter::ArkUnion<Opt_Union_Number_String, Ark_String>("10%"),
        .badgeColor =
            Converter::ArkValue<Opt_ResourceColor>(Converter::ArkUnion<Ark_ResourceColor, Ark_String>("no color")),
        .borderColor =
            Converter::ArkValue<Opt_ResourceColor>(Converter::ArkUnion<Ark_ResourceColor, Ark_String>("blue color")),
        .borderWidth = Converter::ArkValue<Opt_Length>(Dimension(0.55f, DimensionUnit::PERCENT)),
        .fontWeight = Converter::ArkUnion<Opt_Union_Number_FontWeight_String, Ark_Number>(-100),
    };
    inputValueOptions.value = Converter::ArkValue<Ark_String>("");

    modifier_->setBadgeOptions1(node_, &inputValueOptions);

    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    jsonValue->Delete("position");
    auto badgeStyleAttrs = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_SET_STYLE_NAME);

    std::string strResult;
    for (const auto& [key, expected] : INVALID_1_TEST1_PLAN) {
        strResult = jsonValue->GetString(key);
        EXPECT_EQ(strResult, expected);
    }

    for (const auto& [key, expected] : INVALID_1_TEST2_PLAN) {
        strResult = badgeStyleAttrs->GetString(key);
        EXPECT_EQ(strResult, expected);
    }
}

} // namespace OHOS::Ace::NG

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

#include <gtest/gtest.h>

#include "modifier_test_base.h"
#include "modifiers_test_utils.h"

#include "core/interfaces/arkoala/utility/converter.h"
#include "core/interfaces/arkoala/utility/reverse_converter.h"
#include "core/components_ng/pattern/calendar_picker/calendar_picker_event_hub.h"
#include "core/components/picker/picker_theme.h"
#include "core/components/calendar/calendar_theme.h"
#include "arkoala_api_generated.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
using namespace Converter;
namespace  {
const auto ATTRIBUTE_TEXT_STYLE_NAME = "textStyle";
const auto ATTRIBUTE_EDGE_ALIGN_TYPE_NAME = "edgeAlign";
const auto ATTRIBUTE_EDGE_ALIGN_ALIGN_TYPE_NAME = "alignType";
const auto ATTRIBUTE_EDGE_ALIGN_OFFSET_NAME = "offset";
const auto ATTRIBUTE_TEXT_STYLE_COLOR_NAME = "color";
const auto ATTRIBUTE_FONT_SIZE_DEFAULT_VALUE = "0.00px";
const auto ATTRIBUTE_TEXT_STYLE_FONT_NAME = "font";
const auto ATTRIBUTE_FONT_SIZE_NAME = "size";
const auto ATTRIBUTE_FONT_WEIGHT_NAME = "weight";

const auto RES_COLOR_ID = IntResourceId{102001, NodeModifier::ResourceType::COLOR};
const auto RES_COLOR_ID_VALUE = Color(0xFF654321);
const auto RES_COLOR_NAME = NamedResourceId{"color_name", NodeModifier::ResourceType::COLOR};
const auto RES_COLOR_NAME_VALUE = Color(0xFF123456);

const auto RES_FONT_FAMILY_NAME = NamedResourceId{"res_font_family_id", NodeModifier::ResourceType::STRARRAY};
const auto RES_FONT_FAMILY_NAME_VALUE = "res_font_family_string_id";
const auto RES_FONT_FAMILY_ID = IntResourceId{102002, NodeModifier::ResourceType::STRARRAY};
const auto RES_FONT_FAMILY_ID_VALUE = "res_font_family_number_id";

struct CalendarAlignTest {
    Ark_CalendarAlign calendarAlignType;
    Ark_Offset offset;
    std::string expectedCalendarAlignType;
    std::string expectedDx;
    std::string expectedDy;
};

inline Ark_Resource ArkRes(Ark_String* name, int id = -1,
    NodeModifier::ResourceType type = NodeModifier::ResourceType::COLOR, const char* module = "",
    const char* bundle = "")
{
    return { .id = { .tag = ARK_TAG_INT32, .i32 = static_cast<Ark_Int32>(id) },
        .type = { .tag = ARK_TAG_INT32, .i32 = static_cast<Ark_Int32>(type) },
        .moduleName = { .chars = module },
        .bundleName = { .chars = bundle },
        .params = { .tag = ARK_TAG_OBJECT, .value = { .array = name, .length = name ? 1 : 0 } }
    };
}

const Ark_Int32 AINT32_POS(70);
const Ark_Int32 AINT32_NEG(INT_MIN);
const Ark_Float32 AFLT32_POS(1.234f);
const Ark_Float32 AFLT32_NEG(-5.6789f);
const auto CHECK_AINT32_POS = "70.00px";
const auto CHECK_AFLT32_POS = "1.23vp";

const auto RES_CONTENT_STR = "aa.bb.cc";
const auto RES_NAME_STR = "res_name";
const auto RES_CONTENT = Converter::ArkValue<Ark_String>(RES_CONTENT_STR);
const auto RES_NAME = Converter::ArkValue<Ark_String>(RES_NAME_STR);
const Opt_Union_String_Resource OPT_UNION_RESOURCE_RESOURCE =
    Converter::ArkUnion<Opt_Union_String_Resource, Ark_Resource>(
        ArkRes(const_cast<Ark_String*>(&RES_NAME), 1234, NodeModifier::ResourceType::STRING));
const std::string CHECK_RESOURCE_STR(RES_CONTENT_STR);

typedef std::pair<Opt_Union_String_Resource, std::string> UnionStringResourceTestStep;
const std::vector<UnionStringResourceTestStep> UNION_RESOURCE_STRING_PLAN = {
    { Converter::ArkUnion<Opt_Union_String_Resource, Ark_String>(RES_CONTENT), CHECK_RESOURCE_STR },
    { OPT_UNION_RESOURCE_RESOURCE, CHECK_RESOURCE_STR }
};

const float TEST_OFFSET_X1 = 11.f;
const float TEST_OFFSET_Y1 = 35.f;
const Ark_Offset offset1 = { .dx = ArkValue<Ark_Length>(TEST_OFFSET_X1), .dy = ArkValue<Ark_Length>(TEST_OFFSET_Y1) };

const float TEST_OFFSET_X2 = 25.f;
const float TEST_OFFSET_Y2 = 40.f;
const Ark_Offset offset2 = { .dx = ArkValue<Ark_Length>(TEST_OFFSET_X2), .dy = ArkValue<Ark_Length>(TEST_OFFSET_Y2) };

const std::vector<CalendarAlignTest> CALENDAR_ALIGN_TEST_PLAN = {
    {
        .calendarAlignType = ARK_CALENDAR_ALIGN_START, .expectedCalendarAlignType = "CalendarAlign.START",
        .offset = offset1, .expectedDx = "11.00vp", .expectedDy = "35.00vp"
    },
    {
        .calendarAlignType = ARK_CALENDAR_ALIGN_CENTER, .expectedCalendarAlignType = "CalendarAlign.CENTER",
        .offset = offset1, .expectedDx = "11.00vp", .expectedDy = "35.00vp"
    },
    {
        .calendarAlignType = ARK_CALENDAR_ALIGN_END, .expectedCalendarAlignType = "CalendarAlign.END",
        .offset = offset1, .expectedDx = "11.00vp", .expectedDy = "35.00vp"
    },
    {
        .calendarAlignType = ARK_CALENDAR_ALIGN_END, .expectedCalendarAlignType = "CalendarAlign.END",
        .offset = offset2, .expectedDx = "25.00vp", .expectedDy = "40.00vp"
    }
};
} // namespace

class CalendarPickerModifierTest : public ModifierTestBase<
    GENERATED_ArkUICalendarPickerModifier,
    &GENERATED_ArkUINodeModifiers::getCalendarPickerModifier,
    GENERATED_ARKUI_CALENDAR_PICKER
> {
public:
    static void SetUpTestCase()
    {
        ModifierTestBase::SetUpTestCase();

        SetupTheme<PickerTheme>();
        SetupTheme<CalendarTheme>();
        SetupTheme<IconTheme>();

        AddResource(RES_FONT_FAMILY_NAME, RES_FONT_FAMILY_NAME_VALUE);
        AddResource(RES_FONT_FAMILY_ID, RES_FONT_FAMILY_ID_VALUE);
        AddResource(RES_COLOR_ID, RES_COLOR_ID_VALUE);
        AddResource(RES_COLOR_NAME, RES_COLOR_NAME_VALUE);
    }
};

/*
 * @tc.name: setEdgeAlignTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CalendarPickerModifierTest, setEdgeAlignTest, TestSize.Level1)
{
    ASSERT_NE(modifier_->setEdgeAlign, nullptr);

    for (const auto& data: CALENDAR_ALIGN_TEST_PLAN) {
        auto optOffset = ArkValue<Opt_Offset>(data.offset);
        modifier_->setEdgeAlign(node_, data.calendarAlignType, &optOffset);

        auto fullJson = GetJsonValue(node_);
        
        auto edgeAlignJson = GetAttrValue<std::unique_ptr<JsonValue>>(fullJson, ATTRIBUTE_EDGE_ALIGN_TYPE_NAME);
        ASSERT_NE(edgeAlignJson, nullptr);

        auto alignTypeStr = GetAttrValue<std::string>(edgeAlignJson, ATTRIBUTE_EDGE_ALIGN_ALIGN_TYPE_NAME);
        EXPECT_EQ(alignTypeStr, data.expectedCalendarAlignType);

        auto offsetJson = GetAttrValue<std::unique_ptr<JsonValue>>(edgeAlignJson, ATTRIBUTE_EDGE_ALIGN_OFFSET_NAME);
        ASSERT_NE(offsetJson, nullptr);
        auto actualDx = GetAttrValue<std::string>(offsetJson, "dX");
        auto actualDy = GetAttrValue<std::string>(offsetJson, "dY");
        EXPECT_EQ(actualDx, data.expectedDx);
        EXPECT_EQ(actualDy, data.expectedDy);
    };
}

typedef std::pair<Opt_ResourceColor, std::string> ArkColorTest;
const std::vector<ArkColorTest> COLOR_TEST_PLAN = {
    { Converter::ArkUnion<Opt_ResourceColor, Ark_Color>(ARK_COLOR_WHITE), "#FFFFFFFF" },
    { Converter::ArkUnion<Opt_ResourceColor, Ark_Color>(ARK_COLOR_BLACK), "#FF000000" },
    { Converter::ArkUnion<Opt_ResourceColor, Ark_Color>(ARK_COLOR_BLUE), "#FF0000FF" },
    { Converter::ArkUnion<Opt_ResourceColor, Ark_Color>(ARK_COLOR_BROWN), "#FFA52A2A" },
    { Converter::ArkUnion<Opt_ResourceColor, Ark_Color>(ARK_COLOR_GRAY), "#FF808080" },
    { Converter::ArkUnion<Opt_ResourceColor, Ark_Color>(ARK_COLOR_GREEN), "#FF008000" },
    { Converter::ArkUnion<Opt_ResourceColor, Ark_Color>(ARK_COLOR_GREY), "#FF808080" },
    { Converter::ArkUnion<Opt_ResourceColor, Ark_Color>(ARK_COLOR_ORANGE), "#FFFFA500" },
    { Converter::ArkUnion<Opt_ResourceColor, Ark_Color>(ARK_COLOR_PINK), "#FFFFC0CB" },
    { Converter::ArkUnion<Opt_ResourceColor, Ark_Color>(ARK_COLOR_RED), "#FFFF0000" },
    { Converter::ArkUnion<Opt_ResourceColor, Ark_Color>(ARK_COLOR_YELLOW), "#FFFFFF00" },
    { Converter::ArkUnion<Opt_ResourceColor, Ark_Color>(ARK_COLOR_TRANSPARENT), "#00000000" },
};

typedef std::pair<Opt_Union_FontWeight_Number_String, std::string> ArkFontWeightTest;
const std::vector<ArkFontWeightTest> FONT_WEIGHT_TEST_PLAN = {
    { Converter::ArkUnion<Opt_Union_FontWeight_Number_String, Ark_FontWeight>(ARK_FONT_WEIGHT_LIGHTER),
        "FontWeight.Lighter" },
    { Converter::ArkUnion<Opt_Union_FontWeight_Number_String, Ark_FontWeight>(ARK_FONT_WEIGHT_NORMAL),
        "FontWeight.Normal" },
    { Converter::ArkUnion<Opt_Union_FontWeight_Number_String, Ark_FontWeight>(ARK_FONT_WEIGHT_REGULAR),
        "FontWeight.Regular" }
};

typedef std::pair<Opt_Length, std::string> OptLengthTestStep;
const std::vector<OptLengthTestStep> FONT_SIZE_TEST_PLAN = {
    { Converter::ArkValue<Opt_Length>(AINT32_POS), CHECK_AINT32_POS },
    { Converter::ArkValue<Opt_Length>(AINT32_NEG), ATTRIBUTE_FONT_SIZE_DEFAULT_VALUE },
    { Converter::ArkValue<Opt_Length>(AFLT32_NEG), ATTRIBUTE_FONT_SIZE_DEFAULT_VALUE },
    { Converter::ArkValue<Opt_Length>(AFLT32_POS), CHECK_AFLT32_POS }
};

/*
 * @tc.name: setTextStyleColorTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CalendarPickerModifierTest, setTextStyleColorTest, TestSize.Level1)
{
    ASSERT_NE(modifier_->setTextStyle, nullptr);
    Ark_Font font = {
        .size = FONT_SIZE_TEST_PLAN[0].first,
        .weight = FONT_WEIGHT_TEST_PLAN[0].first
    };
    Ark_PickerTextStyle pickerStyle;
    pickerStyle.font.value = font;

    auto frameNode = reinterpret_cast<FrameNode *>(node_);
    ASSERT_NE(frameNode, nullptr);

    for (auto color : COLOR_TEST_PLAN) {
        pickerStyle.color = color.first;
        modifier_->setTextStyle(node_, &pickerStyle);
        auto fullJson = GetJsonValue(node_);
        auto styleObject = GetAttrValue<std::unique_ptr<JsonValue>>(fullJson, ATTRIBUTE_TEXT_STYLE_NAME);
        auto checkColor = GetAttrValue<std::string>(styleObject, ATTRIBUTE_TEXT_STYLE_COLOR_NAME);
        EXPECT_EQ(checkColor, color.second);
    }
}

/*
 * @tc.name: setTextStyleFontWeightTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CalendarPickerModifierTest, setTextStyleFontWeightTest, TestSize.Level1)
{
    ASSERT_NE(modifier_->setTextStyle, nullptr);
    Ark_Font font = {
        .size = FONT_SIZE_TEST_PLAN[0].first,
        .weight = FONT_WEIGHT_TEST_PLAN[0].first
    };
    Ark_PickerTextStyle pickerStyle;
    auto sizeStr = FONT_SIZE_TEST_PLAN[0].second;

    auto frameNode = reinterpret_cast<FrameNode *>(node_);
    ASSERT_NE(frameNode, nullptr);

    for (auto weight : FONT_WEIGHT_TEST_PLAN) {
        font.weight = weight.first;
        pickerStyle.font.value = font;
        modifier_->setTextStyle(node_, &pickerStyle);
        auto fullJson = GetJsonValue(node_);
        auto styleObject = GetAttrValue<std::unique_ptr<JsonValue>>(fullJson, ATTRIBUTE_TEXT_STYLE_NAME);
        auto fontObject = GetAttrValue<std::string>(styleObject, ATTRIBUTE_TEXT_STYLE_FONT_NAME);
        auto checkSize = GetAttrValue<std::string>(fontObject, ATTRIBUTE_FONT_SIZE_NAME);
        auto checkWeight = GetAttrValue<std::string>(fontObject, ATTRIBUTE_FONT_WEIGHT_NAME);
        EXPECT_EQ(checkSize, sizeStr);
        EXPECT_EQ(checkWeight, weight.second);
    }
}

/*
 * @tc.name: setTextStyleFontSizeTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CalendarPickerModifierTest, setTextStyleFontSizeTest, TestSize.Level1)
{
    ASSERT_NE(modifier_->setTextStyle, nullptr);
    Ark_Font font = {
        .size = FONT_SIZE_TEST_PLAN[0].first,
        .weight = FONT_WEIGHT_TEST_PLAN[0].first
    };
    Ark_PickerTextStyle pickerStyle;
    auto weightStr = FONT_WEIGHT_TEST_PLAN[0].second;

    auto frameNode = reinterpret_cast<FrameNode *>(node_);
    ASSERT_NE(frameNode, nullptr);

    for (auto size : FONT_SIZE_TEST_PLAN) {
        font.size = size.first;
        pickerStyle.font.value = font;
        modifier_->setTextStyle(node_, &pickerStyle);
        auto fullJson = GetJsonValue(node_);
        auto styleObject = GetAttrValue<std::unique_ptr<JsonValue>>(fullJson, ATTRIBUTE_TEXT_STYLE_NAME);
        auto fontObject = GetAttrValue<std::string>(styleObject, ATTRIBUTE_TEXT_STYLE_FONT_NAME);
        auto checkSize = GetAttrValue<std::string>(fontObject, ATTRIBUTE_FONT_SIZE_NAME);
        auto checkWeight = GetAttrValue<std::string>(fontObject, ATTRIBUTE_FONT_WEIGHT_NAME);
        EXPECT_EQ(checkSize, size.second);
        EXPECT_EQ(checkWeight, weightStr);
    }
}

} // namespace OHOS::Ace::NG

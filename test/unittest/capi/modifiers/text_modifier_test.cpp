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
#include "generated/test_fixtures.h"
#include "generated/type_helpers.h"

#include "core/interfaces/native/utility/reverse_converter.h"

#include "core/components/text/text_theme.h"
#include "core/components_ng/pattern/text/text_event_hub.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {
const std::string FONT_COLOR_ATTR = "fontColor";
const std::string FONT_SIZE_ATTR = "fontSize";
const std::string FONT_STYLE_ATTR = "fontStyle";
const std::string FONT_FAMILY_ATTR = "fontFamily";
const std::string FONT_WEIGHT_ATTR = "fontWeight";

const std::string TEXT_ALIGN_ATTR = "textAlign";
const std::string TEXT_OVERFLOW_ATTR = "textOverflow";
const std::string TEXT_CASE_ATTR = "textCase";

const std::string LINE_HEIGHT_ATTR = "lineHeight";
const std::string MIN_FONT_SIZE_ATTR = "minFontSize";
const std::string MAX_FONT_SIZE_ATTR = "maxFontSize";
const std::string MIN_FONT_SCALE_ATTR = "minFontScale";
const std::string MAX_FONT_SCALE_ATTR = "maxFontScale";
const std::string MAX_LINES_ATTR = "maxLines";

const std::string DECORATION_ATTR = "decoration";
const std::string LETTER_SPACING_ATTR = "letterSpacing";
const std::string BASELINE_OFFSET_ATTR = "baselineOffset";
const std::string COPY_OPTIONS_ATTR = "copyOption";
const std::string DRAGGABLE_ATTR = "draggable";
const std::string TEXT_SHADOW_ATTR = "textShadow";
const std::string HEIGHT_ADAPTIVE_POLICY_ATTR = "heightAdaptivePolicy";
const std::string TEXT_INDENT_ATTR = "textIndent";
const std::string WORD_BREAK_ATTR = "wordBreak";
const std::string LINE_BREAK_STRATEGY_ATTR = "lineBreakStrategy";
const std::string ELLIPSIS_MODE_ATTR = "ellipsisMode";
const std::string TEXT_SELECTABLE_ATTR = "textSelectable";
const auto RES_NAME = NamedResourceId("aa.bb.cc", NodeModifier::ResourceType::COLOR);
const auto RES_NAME1 = NamedResourceId("aa.bb.cc", NodeModifier::ResourceType::FLOAT);

const uint32_t FLOAT_RES_0_ID = 100;
const double FLOAT_RES_0_VALUE = 0.705f;
const Ark_Resource FLOAT_RES_0 = CreateResource(FLOAT_RES_0_ID, NodeModifier::ResourceType::FLOAT);

const uint32_t FLOAT_RES_1_ID = 101;
const double FLOAT_RES_1_VALUE = 5.2f;
const Ark_Resource FLOAT_RES_1 = CreateResource(FLOAT_RES_1_ID, NodeModifier::ResourceType::FLOAT);

const uint32_t FLOAT_RES_2_ID = 102;
const float FLOAT_RES_2_VALUE = 10.f;
const auto FLOAT_RES_2_STORED_VALUE = Dimension(FLOAT_RES_2_VALUE, DimensionUnit::PX);
const Ark_Resource FLOAT_RES_2 = CreateResource(FLOAT_RES_2_ID, NodeModifier::ResourceType::FLOAT);

const auto FONT_SIZE_ATTR_DEFAULT_VALUE = "16.00fp";
const auto TEXT_OVERFLOW_ATTR_DEFAULT_VALUE = "TextOverflow.Clip";

const auto ATTRIBUTE_FONT_NAME = "font";
const auto ATTRIBUTE_FONT_WEIGHT_NAME = "fontWeight";
const auto ATTRIBUTE_VARIABLE_FONT_WEIGHT_NAME = "variableFontWeight";
const auto ATTRIBUTE_VARIABLE_FONT_WEIGHT_DEFAULT_VALUE = "400";
const auto ATTRIBUTE_FONT_WEIGHT_DEFAULT_VALUE = "FontWeight.Normal";
const auto ATTRIBUTE_ENABLE_VARIABLE_FONT_WEIGHT_NAME = "enableVariableFontWeight";
const auto ATTRIBUTE_ENABLE_VARIABLE_FONT_WEIGHT_DEFAULT_VALUE = "false";
const auto ATTRIBUTE_TEXT_SHADOW_NAME = "textShadow";
const auto ATTRIBUTE_TEXT_SHADOW_I_RADIUS_NAME = "radius";
const auto ATTRIBUTE_TEXT_SHADOW_I_TYPE_NAME = "type";
const auto ATTRIBUTE_TEXT_SHADOW_I_COLOR_NAME = "color";
const auto ATTRIBUTE_TEXT_SHADOW_I_OFFSET_X_NAME = "offsetX";
const auto ATTRIBUTE_TEXT_SHADOW_I_OFFSET_Y_NAME = "offsetY";
const auto ATTRIBUTE_CONTENT_NAME = "content";
const auto ATTRIBUTE_CONTENT_DEFAULT_VALUE = "";

    struct EventsTracker {
        static inline GENERATED_ArkUITextEventsReceiver textEventReceiver {};

        static inline const GENERATED_ArkUIEventsAPI eventsApiImpl = {
            .getTextEventsReceiver = [] () -> const GENERATED_ArkUITextEventsReceiver* {
                return &textEventReceiver;
            }
        };
    };
} // namespace

class TextModifierTest : public ModifierTestBase<GENERATED_ArkUITextModifier,
    &GENERATED_ArkUINodeModifiers::getTextModifier, GENERATED_ARKUI_TEXT> {
public:
    static void SetUpTestCase()
    {
        ModifierTestBase::SetUpTestCase();

        SetupTheme<TextTheme>();

        for (auto& [id, strid, res] : Fixtures::resourceInitTable) {
            AddResource(id, res);
            AddResource(strid, res);
        }

        AddResource(FLOAT_RES_0_ID, FLOAT_RES_0_VALUE);
        AddResource(FLOAT_RES_1_ID, FLOAT_RES_1_VALUE);
        AddResource(FLOAT_RES_2_ID, FLOAT_RES_2_STORED_VALUE);

        // setup the test event handler
        fullAPI_->setArkUIEventsAPI(&EventsTracker::eventsApiImpl);
    }
};

HWTEST_F(TextModifierTest, setFontColor, TestSize.Level1)
{
    const Ark_ResourceColor color1 = { .selector = 0, .value0 = ARK_COLOR_GREEN };
    modifier_->setFontColor(node_, &color1);
    auto checkVal1 = GetStringAttribute(node_, FONT_COLOR_ATTR);
    EXPECT_EQ(checkVal1, "#FF008000");

    auto resNameColor = CreateResourceUnion<Ark_ResourceColor>(RES_NAME);
    modifier_->setFontColor(node_, &resNameColor);
    auto checkVal7 = GetStringAttribute(node_, FONT_COLOR_ATTR);
    EXPECT_EQ(checkVal7, "#FFFF0000"); // Color::RED is result of mocked ThemeConstants::GetColorByName
}

HWTEST_F(TextModifierTest, setFontSize, TestSize.Level1)
{
    const Ark_Union_Number_String_Resource size1 = {
        .selector = 0,
        .value0 = Converter::ArkValue<Ark_Number>(10)
    };
    modifier_->setFontSize(node_, &size1);
    auto checkVal1 = GetStringAttribute(node_, FONT_SIZE_ATTR);
    EXPECT_EQ(checkVal1, "10.00vp");

    const Ark_Union_Number_String_Resource size2 = {
        .selector = 1,
        .value1 = Converter::ArkValue<Ark_String>("10.00vp")
    };

    modifier_->setFontSize(node_, &size2);
    auto checkVal2 = GetStringAttribute(node_, FONT_SIZE_ATTR);
    EXPECT_EQ(checkVal2, "10.00vp");

    auto size3 = CreateResourceUnion<Ark_Union_Number_String_Resource>(RES_NAME);

    modifier_->setFontSize(node_, &size3);
    auto checkVal3 = GetStringAttribute(node_, FONT_SIZE_ATTR);
    EXPECT_EQ(checkVal3, FONT_SIZE_ATTR_DEFAULT_VALUE);
}

HWTEST_F(TextModifierTest, setMinFontSize, TestSize.Level1)
{
    const Ark_Union_Number_String_Resource size1 = {
        .selector = 0,
        .value0 = Converter::ArkValue<Ark_Number>(11.25f)
    };
    modifier_->setMinFontSize(node_, &size1);
    auto checkVal1 = GetStringAttribute(node_, MIN_FONT_SIZE_ATTR);
    EXPECT_EQ(checkVal1, "11.25vp");

    const Ark_Union_Number_String_Resource size2 = {
        .selector = 1,
        .value1 = Converter::ArkValue<Ark_String>("11.25px")
    };

    modifier_->setMinFontSize(node_, &size2);
    auto checkVal2 = GetStringAttribute(node_, MIN_FONT_SIZE_ATTR);
    EXPECT_EQ(checkVal2, "11.25px");

    auto size3 = CreateResourceUnion<Ark_Union_Number_String_Resource>(RES_NAME1);

    modifier_->setMinFontSize(node_, &size3);
    auto checkVal3 = GetStringAttribute(node_, MIN_FONT_SIZE_ATTR);
    EXPECT_EQ(checkVal3, "10.00px");
}

HWTEST_F(TextModifierTest, setMaxFontSize, TestSize.Level1)
{
    const Ark_Union_Number_String_Resource size1 = {
        .selector = 0,
        .value0 = Converter::ArkValue<Ark_Number>(11.25f)
    };
    modifier_->setMaxFontSize(node_, &size1);
    auto checkVal1 = GetStringAttribute(node_, MAX_FONT_SIZE_ATTR);
    EXPECT_EQ(checkVal1, "11.25vp");

    const Ark_Union_Number_String_Resource size2 = {
        .selector = 1,
        .value1 = Converter::ArkValue<Ark_String>("11.25px")
    };

    modifier_->setMaxFontSize(node_, &size2);
    auto checkVal2 = GetStringAttribute(node_, MAX_FONT_SIZE_ATTR);
    EXPECT_EQ(checkVal2, "11.25px");

    auto size3 = CreateResourceUnion<Ark_Union_Number_String_Resource>(RES_NAME1);

    modifier_->setMaxFontSize(node_, &size3);
    auto checkVal3 = GetStringAttribute(node_, MAX_FONT_SIZE_ATTR);
    EXPECT_EQ(checkVal3, "10.00px");
}

HWTEST_F(TextModifierTest, setMinFontScale, TestSize.Level1)
{
    const Ark_Union_Number_Resource scale1 = {
        .selector = 0,
        .value0 = Converter::ArkValue<Ark_Number>(0.825f)
    };

    modifier_->setMinFontScale(node_, &scale1);
    auto checkVal1 = GetStringAttribute(node_, MIN_FONT_SCALE_ATTR);
    EXPECT_EQ(checkVal1, "0.825000");

    const auto scale2 = Converter::ArkUnion<Ark_Union_Number_Resource, Ark_Resource>(FLOAT_RES_0);

    modifier_->setMinFontScale(node_, &scale2);
    auto checkVal2 = GetStringAttribute(node_, MIN_FONT_SCALE_ATTR);
    EXPECT_EQ(checkVal2, "0.705000");
}

HWTEST_F(TextModifierTest, setMaxFontScale, TestSize.Level1)
{
    const Ark_Union_Number_Resource scale1 = {
        .selector = 0,
        .value0 = Converter::ArkValue<Ark_Number>(1.25f)
    };

    modifier_->setMaxFontScale(node_, &scale1);
    auto checkVal1 = GetStringAttribute(node_, MAX_FONT_SCALE_ATTR);
    EXPECT_EQ(checkVal1, "1.250000");

    const auto scale2 = Converter::ArkUnion<Ark_Union_Number_Resource, Ark_Resource>(FLOAT_RES_1);

    modifier_->setMaxFontScale(node_, &scale2);
    auto checkVal2 = GetStringAttribute(node_, MAX_FONT_SCALE_ATTR);
    EXPECT_EQ(checkVal2, "5.200000");
}

HWTEST_F(TextModifierTest, setFontStyle, TestSize.Level1)
{
    modifier_->setFontStyle(node_, ARK_FONT_STYLE_NORMAL);
    auto checkVal1 = GetStringAttribute(node_, FONT_STYLE_ATTR);
    EXPECT_EQ(checkVal1, "FontStyle.Normal");

    modifier_->setFontStyle(node_, ARK_FONT_STYLE_ITALIC);
    auto checkVal2 = GetStringAttribute(node_, FONT_STYLE_ATTR);
    EXPECT_EQ(checkVal2, "FontStyle.Italic");
}

HWTEST_F(TextModifierTest, setFontWeight, TestSize.Level1)
{
    const Ark_Union_Number_FontWeight_String w1 = {
        .selector = 0,
        .value0 = Converter::ArkValue<Ark_Number>(100)
    };
    modifier_->setFontWeight0(node_, &w1);
    auto checkVal1 = GetStringAttribute(node_, FONT_WEIGHT_ATTR);
    EXPECT_EQ(checkVal1, "100");

    const Ark_Union_Number_FontWeight_String w2 = {
        .selector = 1,
        .value1 = ARK_FONT_WEIGHT_MEDIUM
    };
    modifier_->setFontWeight0(node_, &w2);
    auto checkVal2 = GetStringAttribute(node_, FONT_WEIGHT_ATTR);
    EXPECT_EQ(checkVal2, "FontWeight.Medium");

    const Ark_Union_Number_FontWeight_String w3 = {
        .selector = 2,
        .value2 = Converter::ArkValue<Ark_String>("normal")
    };
    modifier_->setFontWeight0(node_, &w3);
    auto checkVal3 = GetStringAttribute(node_, FONT_WEIGHT_ATTR);
    EXPECT_EQ(checkVal3, "FontWeight.Normal");
}

HWTEST_F(TextModifierTest, setTextAlign, TestSize.Level1)
{
    modifier_->setTextAlign(node_, ARK_TEXT_ALIGN_START);
    auto checkVal1 = GetStringAttribute(node_, TEXT_ALIGN_ATTR);
    EXPECT_EQ(checkVal1, "TextAlign.Start");

    modifier_->setTextAlign(node_, ARK_TEXT_ALIGN_CENTER);
    auto checkVal2 = GetStringAttribute(node_, TEXT_ALIGN_ATTR);
    EXPECT_EQ(checkVal2, "TextAlign.Center");

    modifier_->setTextAlign(node_, ARK_TEXT_ALIGN_END);
    auto checkVal3 = GetStringAttribute(node_, TEXT_ALIGN_ATTR);
    EXPECT_EQ(checkVal3, "TextAlign.End");

    modifier_->setTextAlign(node_, ARK_TEXT_ALIGN_JUSTIFY);
    auto checkVal4 = GetStringAttribute(node_, TEXT_ALIGN_ATTR);
    EXPECT_EQ(checkVal4, "TextAlign.Justify");
}

HWTEST_F(TextModifierTest, setLineHeight, TestSize.Level1)
{
    const Ark_Union_Number_String_Resource size1 = {
        .selector = 0,
        .value0 = Converter::ArkValue<Ark_Number>(11)
    };
    modifier_->setLineHeight(node_, &size1);
    auto checkVal1 = GetStringAttribute(node_, LINE_HEIGHT_ATTR);
    EXPECT_EQ(checkVal1, "11.00vp");

    const Ark_Union_Number_String_Resource size2 = {
        .selector = 1,
        .value1 = Converter::ArkValue<Ark_String>("12.00vp")
    };

    modifier_->setLineHeight(node_, &size2);
    auto checkVal2 = GetStringAttribute(node_, LINE_HEIGHT_ATTR);
    EXPECT_EQ(checkVal2, "12.00vp");

    const auto size3 = Converter::ArkUnion<Ark_Union_Number_String_Resource, Ark_Resource>(FLOAT_RES_2);

    modifier_->setLineHeight(node_, &size3);
    auto checkVal3 = GetStringAttribute(node_, LINE_HEIGHT_ATTR);
    EXPECT_EQ(checkVal3, "10.00px");
}

HWTEST_F(TextModifierTest, setTextOverflow, TestSize.Level1)
{
    Ark_TextOverflowOptions v1 = { .overflow = ARK_TEXT_OVERFLOW_NONE };
    modifier_->setTextOverflow(node_, &v1);
    auto checkVal1 = GetStringAttribute(node_, TEXT_OVERFLOW_ATTR);
    EXPECT_EQ(checkVal1, "TextOverflow.None");

    Ark_TextOverflowOptions v2 = { .overflow = ARK_TEXT_OVERFLOW_CLIP };
    modifier_->setTextOverflow(node_, &v2);
    auto checkVal2 = GetStringAttribute(node_, TEXT_OVERFLOW_ATTR);
    EXPECT_EQ(checkVal2, "TextOverflow.Clip");

    Ark_TextOverflowOptions v3 = { .overflow = ARK_TEXT_OVERFLOW_ELLIPSIS };
    modifier_->setTextOverflow(node_, &v3);
    auto checkVal3 = GetStringAttribute(node_, TEXT_OVERFLOW_ATTR);
    EXPECT_EQ(checkVal3, "TextOverflow.Ellipsis");

    Ark_TextOverflowOptions v4 = { .overflow = ARK_TEXT_OVERFLOW_MARQUEE };
    modifier_->setTextOverflow(node_, &v4);
    auto checkVal4 = GetStringAttribute(node_, TEXT_OVERFLOW_ATTR);
    EXPECT_EQ(checkVal4, "TextOverflow.Marquee");
}

HWTEST_F(TextModifierTest, setTextOverflowTestDefaultValue, TestSize.Level1)
{
    auto checkVal1 = GetStringAttribute(node_, TEXT_OVERFLOW_ATTR);
    EXPECT_EQ(checkVal1, TEXT_OVERFLOW_ATTR_DEFAULT_VALUE);
}

HWTEST_F(TextModifierTest, setTextOverflowTestInvalidValue, TestSize.Level1)
{
    Ark_TextOverflowOptions v1 = { .overflow = ARK_TEXT_OVERFLOW_NONE };
    modifier_->setTextOverflow(node_, &v1);
    Ark_TextOverflowOptions invalidValue = { .overflow = static_cast<Ark_TextOverflow>(-1) };
    modifier_->setTextOverflow(node_, &invalidValue);
    auto checkVal1 = GetStringAttribute(node_, TEXT_OVERFLOW_ATTR);
    EXPECT_EQ(checkVal1, TEXT_OVERFLOW_ATTR_DEFAULT_VALUE);
}

HWTEST_F(TextModifierTest, setFontFamily, TestSize.Level1)
{
    Ark_Union_String_Resource v1 = {
        .selector = 0,
        .value0 = Converter::ArkValue<Ark_String>("testFontFamily")
    };
    modifier_->setFontFamily(node_, &v1);
    auto checkVal1 = GetStringAttribute(node_, FONT_FAMILY_ATTR);
    EXPECT_EQ(checkVal1, "testFontFamily");
}

HWTEST_F(TextModifierTest, setMaxLines, TestSize.Level1)
{
    auto v1 = Converter::ArkValue<Ark_Number>(10);
    modifier_->setMaxLines(node_, &v1);
    auto checkVal1 = GetStringAttribute(node_, MAX_LINES_ATTR);
    EXPECT_EQ(checkVal1, "10");
}

HWTEST_F(TextModifierTest, setDecoration, TestSize.Level1)
{
    Ark_ResourceColor color1 = { .selector = 0, .value0 = ARK_COLOR_GREEN };
    Ark_DecorationStyleInterface v1 = {
        .type = ARK_TEXT_DECORATION_TYPE_UNDERLINE,
        .style = { .tag = ARK_TAG_OBJECT, .value = ARK_TEXT_DECORATION_STYLE_DASHED },
        .color = { .tag = ARK_TAG_OBJECT, .value = color1 }
    };

    modifier_->setDecoration(node_, &v1);
    auto checkVal1 = GetStringAttribute(node_, DECORATION_ATTR);
    EXPECT_EQ(checkVal1,
        "{\"type\":\"TextDecorationType.Underline\",\"color\":\"#FF008000\",\"style\":\"TextDecorationStyle.DASHED\"}");
}

HWTEST_F(TextModifierTest, setLetterSpacing, TestSize.Level1)
{
    Ark_Union_Number_String v1 = {
        .selector = 0,
        .value0 = Converter::ArkValue<Ark_Number>(11.25f)
    };

    modifier_->setLetterSpacing(node_, &v1);
    auto checkVal1 = GetStringAttribute(node_, LETTER_SPACING_ATTR);
    EXPECT_EQ(checkVal1, "11.25vp");

    Ark_Union_Number_String v2 = {
        .selector = 1,
        .value1 = Converter::ArkValue<Ark_String>("12.25vp")
    };

    modifier_->setLetterSpacing(node_, &v2);
    auto checkVal2 = GetStringAttribute(node_, LETTER_SPACING_ATTR);
    EXPECT_EQ(checkVal2, "12.25vp");
}

HWTEST_F(TextModifierTest, setTextCase, TestSize.Level1)
{
    modifier_->setTextCase(node_, ARK_TEXT_CASE_NORMAL);
    auto checkVal1 = GetStringAttribute(node_, TEXT_CASE_ATTR);
    EXPECT_EQ(checkVal1, "TextCase.Normal");

    modifier_->setTextCase(node_, ARK_TEXT_CASE_LOWER_CASE);
    auto checkVal2 = GetStringAttribute(node_, TEXT_CASE_ATTR);
    EXPECT_EQ(checkVal2, "TextCase.LowerCase");

    modifier_->setTextCase(node_, ARK_TEXT_CASE_UPPER_CASE);
    auto checkVal3 = GetStringAttribute(node_, TEXT_CASE_ATTR);
    EXPECT_EQ(checkVal3, "TextCase.UpperCase");
}

HWTEST_F(TextModifierTest, setBaselineOffset, TestSize.Level1)
{
    Ark_Union_Number_String v1 = {
        .selector = 0,
        .value0 = Converter::ArkValue<Ark_Number>(11)
    };

    modifier_->setBaselineOffset(node_, &v1);
    auto checkVal1 = GetStringAttribute(node_, BASELINE_OFFSET_ATTR);
    EXPECT_EQ(checkVal1, "11");

    Ark_Union_Number_String v2 = {
        .selector = 1,
        .value1 = Converter::ArkValue<Ark_String>("12")
    };

    modifier_->setBaselineOffset(node_, &v2);
    auto checkVal2 = GetStringAttribute(node_, BASELINE_OFFSET_ATTR);
    EXPECT_EQ(checkVal2, "12");
}

HWTEST_F(TextModifierTest, setCopyOption, TestSize.Level1)
{
    modifier_->setCopyOption(node_, ARK_COPY_OPTIONS_NONE);
    auto checkVal1 = GetStringAttribute(node_, COPY_OPTIONS_ATTR);
    EXPECT_EQ(checkVal1, "CopyOptions.None");

    modifier_->setCopyOption(node_, ARK_COPY_OPTIONS_IN_APP);
    auto checkVal2 = GetStringAttribute(node_, COPY_OPTIONS_ATTR);
    EXPECT_EQ(checkVal2, "CopyOptions.InApp");

    modifier_->setCopyOption(node_, ARK_COPY_OPTIONS_LOCAL_DEVICE);
    auto checkVal3 = GetStringAttribute(node_, COPY_OPTIONS_ATTR);
    EXPECT_EQ(checkVal3, "CopyOptions.Local");

    modifier_->setCopyOption(node_, ARK_COPY_OPTIONS_CROSS_DEVICE);
    auto checkVal4 = GetStringAttribute(node_, COPY_OPTIONS_ATTR);
    EXPECT_EQ(checkVal4, "CopyOptions.Distributed");
}

HWTEST_F(TextModifierTest, setTextShadow, TestSize.Level1)
{
    Ark_ShadowOptions shadow = {
        .radius = { .selector = 0, .value0 = Converter::ArkValue<Ark_Number>(1.5f) },
        .type = { .tag = ARK_TAG_OBJECT, .value = ARK_SHADOW_TYPE_COLOR },
        .color = { .tag = ARK_TAG_OBJECT, .value = { .selector = 0, .value0 = ARK_COLOR_GREEN }},
        .offsetX = { .tag = ARK_TAG_OBJECT,
                     .value = { .selector = 0, .value0 = Converter::ArkValue<Ark_Number>(2.5f) }},
        .offsetY = { .tag = ARK_TAG_OBJECT,
                     .value = { .selector = 0, .value0 = Converter::ArkValue<Ark_Number>(3.5f) }}
    };

    Ark_Union_ShadowOptions_Array_ShadowOptions v1 = {
        .selector = 0,
        .value0 = shadow
    };

    modifier_->setTextShadow(node_, &v1);
    auto checkVal1 = GetStringAttribute(node_, TEXT_SHADOW_ATTR);
    EXPECT_EQ(checkVal1,
        "{\"radius\":\"1.500000\",\"color\":\"#FF008000\","
        "\"offsetX\":\"2.500000\",\"offsetY\":\"3.500000\",\"type\":\"0\"}");
}

HWTEST_F(TextModifierTest, setHeightAdaptivePolicy, TestSize.Level1)
{
    modifier_->setHeightAdaptivePolicy(node_, ARK_TEXT_HEIGHT_ADAPTIVE_POLICY_MAX_LINES_FIRST);
    auto checkVal3 = GetStringAttribute(node_, HEIGHT_ADAPTIVE_POLICY_ATTR);
    EXPECT_EQ(checkVal3, "TextHeightAdaptivePolicy.MAX_LINES_FIRST");

    modifier_->setHeightAdaptivePolicy(node_, ARK_TEXT_HEIGHT_ADAPTIVE_POLICY_MIN_FONT_SIZE_FIRST);
    auto checkVal4 = GetStringAttribute(node_, HEIGHT_ADAPTIVE_POLICY_ATTR);
    EXPECT_EQ(checkVal4, "TextHeightAdaptivePolicy.MIN_FONT_SIZE_FIRST");

    modifier_->setHeightAdaptivePolicy(node_, ARK_TEXT_HEIGHT_ADAPTIVE_POLICY_LAYOUT_CONSTRAINT_FIRST);
    auto checkVal5 = GetStringAttribute(node_, HEIGHT_ADAPTIVE_POLICY_ATTR);
    EXPECT_EQ(checkVal5, "TextHeightAdaptivePolicy.LAYOUT_CONSTRAINT_FIRST");
}

HWTEST_F(TextModifierTest, setTextIndent, TestSize.Level1)
{
    Ark_Length v1 = {
        .type = ARK_TAG_LENGTH,
        .unit = static_cast<int>(DimensionUnit::PX),
        .value = 11.25f
    };

    modifier_->setTextIndent(node_, &v1);
    auto checkVal1 = GetStringAttribute(node_, TEXT_INDENT_ATTR);
    EXPECT_EQ(checkVal1, "11.25px");
}

HWTEST_F(TextModifierTest, setWordBreak, TestSize.Level1)
{
    modifier_->setWordBreak(node_, ARK_WORD_BREAK_NORMAL);
    auto checkVal1 = GetStringAttribute(node_, WORD_BREAK_ATTR);
    EXPECT_EQ(checkVal1, "normal");

    modifier_->setWordBreak(node_, ARK_WORD_BREAK_BREAK_ALL);
    auto checkVal2 = GetStringAttribute(node_, WORD_BREAK_ATTR);
    EXPECT_EQ(checkVal2, "break-all");

    modifier_->setWordBreak(node_, ARK_WORD_BREAK_BREAK_WORD);
    auto checkVal3 = GetStringAttribute(node_, WORD_BREAK_ATTR);
    EXPECT_EQ(checkVal3, "break-word");
}
HWTEST_F(TextModifierTest, setLineBreakStrategy, TestSize.Level1)
{
    modifier_->setLineBreakStrategy(node_, ARK_LINE_BREAK_STRATEGY_GREEDY);
    auto checkVal3 = GetStringAttribute(node_, LINE_BREAK_STRATEGY_ATTR);
    EXPECT_EQ(checkVal3, "greedy");

    modifier_->setLineBreakStrategy(node_, ARK_LINE_BREAK_STRATEGY_HIGH_QUALITY);
    auto checkVal4 = GetStringAttribute(node_, LINE_BREAK_STRATEGY_ATTR);
    EXPECT_EQ(checkVal4, "high-quality");

    modifier_->setLineBreakStrategy(node_, ARK_LINE_BREAK_STRATEGY_BALANCED);
    auto checkVal5 = GetStringAttribute(node_, LINE_BREAK_STRATEGY_ATTR);
    EXPECT_EQ(checkVal5, "balanced");
}
HWTEST_F(TextModifierTest, setEllipsisMode, TestSize.Level1)
{
    modifier_->setEllipsisMode(node_, ARK_ELLIPSIS_MODE_START);
    auto checkVal1 = GetStringAttribute(node_, ELLIPSIS_MODE_ATTR);
    EXPECT_EQ(checkVal1, "EllipsisMode.START");

    modifier_->setEllipsisMode(node_, ARK_ELLIPSIS_MODE_CENTER);
    auto checkVal2 = GetStringAttribute(node_, ELLIPSIS_MODE_ATTR);
    EXPECT_EQ(checkVal2, "EllipsisMode.CENTER");

    modifier_->setEllipsisMode(node_, ARK_ELLIPSIS_MODE_END);
    auto checkVal3 = GetStringAttribute(node_, ELLIPSIS_MODE_ATTR);
    EXPECT_EQ(checkVal3, "EllipsisMode.END");
}
HWTEST_F(TextModifierTest, setSelectableMode, TestSize.Level1)
{
    modifier_->setTextSelectable(node_, ARK_TEXT_SELECTABLE_MODE_SELECTABLE_UNFOCUSABLE);
    auto checkVal1 = GetStringAttribute(node_, TEXT_SELECTABLE_ATTR);
    EXPECT_EQ(checkVal1, "selectable-unfocusable");

    modifier_->setTextSelectable(node_, ARK_TEXT_SELECTABLE_MODE_SELECTABLE_FOCUSABLE);
    auto checkVal2 = GetStringAttribute(node_, TEXT_SELECTABLE_ATTR);
    EXPECT_EQ(checkVal2, "selectable-focusable");

    modifier_->setTextSelectable(node_, ARK_TEXT_SELECTABLE_MODE_UNSELECTABLE);
    auto checkVal3 = GetStringAttribute(node_, TEXT_SELECTABLE_ATTR);
    EXPECT_EQ(checkVal3, "unselectable");
}

HWTEST_F(TextModifierTest, setOnCopyTest, TestSize.Level1)
{
    Callback_String_Void func{};
    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    auto eventHub = frameNode->GetEventHub<TextEventHub>();

    struct CopyEvent {
        int32_t nodeId;
        std::string value;
    };

    static std::optional<CopyEvent> checkEvent = std::nullopt;
    EventsTracker::textEventReceiver.onCopy = [](Ark_Int32 nodeId, const Ark_String value)
    {
        checkEvent = {
            .nodeId = nodeId,
            .value = value.chars
        };
    };

    modifier_->setOnCopy(node_, &func);

    // check true value
    EXPECT_EQ(checkEvent.has_value(), false);
    eventHub->FireOnCopy("test_string");
    EXPECT_EQ(checkEvent.has_value(), true);
    EXPECT_EQ(checkEvent->nodeId, frameNode->GetId());
    EXPECT_EQ(checkEvent->value, "test_string");
}

HWTEST_F(TextModifierTest, setOnSelectionChange, TestSize.Level1)
{
    Callback_Number_Number_Void func{};
    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    auto eventHub = frameNode->GetEventHub<TextEventHub>();

    struct SelectionChangeEvent {
        int32_t nodeId;
        int32_t start;
        int32_t end;
    };

    static std::optional<SelectionChangeEvent> selectionEvent = std::nullopt;
    EventsTracker::textEventReceiver.onTextSelectionChange = [](Ark_Int32 nodeId,
        const Ark_Number selectionStart, const Ark_Number selectionEnd)
    {
        selectionEvent = {
            .nodeId = nodeId,
            .start = selectionStart.i32,
            .end = selectionEnd.i32
        };
    };

    modifier_->setOnTextSelectionChange(node_, &func);

    // check true value
    EXPECT_EQ(selectionEvent.has_value(), false);
    eventHub->FireOnSelectionChange(1, 10);
    EXPECT_EQ(selectionEvent.has_value(), true);
    EXPECT_EQ(selectionEvent->nodeId, frameNode->GetId());
    EXPECT_EQ(selectionEvent->start, 1);
    EXPECT_EQ(selectionEvent->end, 10);
}

using namespace Converter;

/*
 * @tc.name: setFontWeight0TestVariableFontWeightValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, setFontWeight0TestVariableFontWeightValidValues, TestSize.Level1)
{
    Ark_Union_Number_FontWeight_String initValueFontWeight;

    // Initial setup
    initValueFontWeight = ArkUnion<Ark_Union_Number_FontWeight_String, Ark_FontWeight>(
        std::get<1>(Fixtures::testFixtureEnumFontWeightValidValues[0]));

    auto checkValue = [this, &initValueFontWeight](const std::string& input,
                          const Ark_Union_Number_FontWeight_String& value, const std::string& expectedStr) {
        Ark_Union_Number_FontWeight_String inputValueFontWeight = initValueFontWeight;

        inputValueFontWeight = value;
        modifier_->setFontWeight0(node_, &inputValueFontWeight);
        auto jsonValue = GetJsonValue(node_);
        auto font = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_FONT_NAME);
        auto resultStr = GetAttrValue<std::string>(font, ATTRIBUTE_VARIABLE_FONT_WEIGHT_NAME);
        EXPECT_EQ(resultStr, expectedStr)
            << "Input value is: " << input << ", method: setFontWeight0, attribute: fontWeight";
    };

    for (auto& [input, value, expected] : Fixtures::testFixtureVariableFontWeightEnumValidValues) {
        checkValue(input, ArkUnion<Ark_Union_Number_FontWeight_String, Ark_FontWeight>(value), expected);
    }
    for (auto& [input, value, expected] : Fixtures::testFixtureFontWeightNumbersValidValues) {
        checkValue(input, ArkUnion<Ark_Union_Number_FontWeight_String, Ark_Number>(value), expected);
    }
    for (auto& [input, value, expected] : Fixtures::testFixtureFontWeightStringsValidValues) {
        checkValue(input, ArkUnion<Ark_Union_Number_FontWeight_String, Ark_String>(value), expected);
    }
}

/*
 * @tc.name: setFontWeight0TestVariableFontWeightInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, setFontWeight0TestVariableFontWeightInvalidValues, TestSize.Level1)
{
    Ark_Union_Number_FontWeight_String initValueFontWeight;

    // Initial setup
    initValueFontWeight = ArkUnion<Ark_Union_Number_FontWeight_String, Ark_FontWeight>(
        std::get<1>(Fixtures::testFixtureEnumFontWeightValidValues[0]));

    auto checkValue = [this, &initValueFontWeight](
                          const std::string& input, const Ark_Union_Number_FontWeight_String& value) {
        Ark_Union_Number_FontWeight_String inputValueFontWeight = initValueFontWeight;

        modifier_->setFontWeight0(node_, &inputValueFontWeight);
        inputValueFontWeight = value;
        modifier_->setFontWeight0(node_, &inputValueFontWeight);
        auto jsonValue = GetJsonValue(node_);
        auto font = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_FONT_NAME);
        auto resultStr = GetAttrValue<std::string>(font, ATTRIBUTE_VARIABLE_FONT_WEIGHT_NAME);
        EXPECT_EQ(resultStr, ATTRIBUTE_VARIABLE_FONT_WEIGHT_DEFAULT_VALUE)
            << "Input value is: " << input << ", method: setFontWeight0, attribute: fontWeight";
    };

    for (auto& [input, value] : Fixtures::testFixtureVariableFontWeightEnumInvalidValues) {
        checkValue(input, ArkUnion<Ark_Union_Number_FontWeight_String, Ark_FontWeight>(value));
    }
    for (auto& [input, value] : Fixtures::testFixtureFontWeightNumbersInvalidValues) {
        checkValue(input, ArkUnion<Ark_Union_Number_FontWeight_String, Ark_Number>(value));
    }
    for (auto& [input, value] : Fixtures::testFixtureFontWeightStringsInvalidValues) {
        checkValue(input, ArkUnion<Ark_Union_Number_FontWeight_String, Ark_String>(value));
    }
    // Check invalid union
    checkValue("invalid union", ArkUnion<Ark_Union_Number_FontWeight_String, Ark_Empty>(nullptr));
}

/*
 * @tc.name: setFontWeight1TestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, setFontWeight1TestDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    auto font = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_FONT_NAME);
    std::string resultStr;

    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_FONT_WEIGHT_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_FONT_WEIGHT_DEFAULT_VALUE) <<
        "Default value for attribute 'fontWeight.weight'";

    resultStr =
        GetAttrValue<std::string>(font, ATTRIBUTE_ENABLE_VARIABLE_FONT_WEIGHT_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_ENABLE_VARIABLE_FONT_WEIGHT_DEFAULT_VALUE) <<
        "Default value for attribute 'fontWeight.options.enableVariableFontWeight'";
}

/*
 * @tc.name: setFontWeight1TestFontWeightValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, setFontWeight1TestFontWeightValidValues, TestSize.Level1)
{
    Ark_Union_Number_FontWeight_String initValueWeight;
    Opt_FontSettingOptions initValueOptions;

    // Initial setup
    initValueWeight = ArkUnion<Ark_Union_Number_FontWeight_String, Ark_FontWeight>(
        std::get<1>(Fixtures::testFixtureEnumFontWeightValidValues[0]));
    Ark_FontSettingOptions options = {
        .enableVariableFontWeight =
            ArkValue<Opt_Boolean>(std::get<1>(Fixtures::testFixtureBooleanValidValues[0]))
    };
    initValueOptions = ArkValue<Opt_FontSettingOptions>(options);

    auto checkValue = [this, &initValueWeight, &initValueOptions](const std::string& input,
                          const Ark_Union_Number_FontWeight_String& value, const std::string& expectedStr) {
        Ark_Union_Number_FontWeight_String inputValueWeight = initValueWeight;
        Opt_FontSettingOptions inputValueOptions = initValueOptions;

        inputValueWeight = value;
        modifier_->setFontWeight1(node_, &inputValueWeight, &inputValueOptions);
        auto jsonValue = GetJsonValue(node_);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_FONT_WEIGHT_NAME);
        EXPECT_EQ(resultStr, expectedStr)
            << "Input value is: " << input << ", method: setFontWeight1, attribute: fontWeight";
    };

    for (auto& [input, value, expected] : Fixtures::testFixtureEnumFontWeightValidValues) {
        checkValue(input, ArkUnion<Ark_Union_Number_FontWeight_String, Ark_FontWeight>(value), expected);
    }
    for (auto& [input, value, expected] : Fixtures::testFixtureFontWeightNumbersValidValues) {
        checkValue(input, ArkUnion<Ark_Union_Number_FontWeight_String, Ark_Number>(value), expected);
    }
    for (auto& [input, value, expected] : Fixtures::testFixtureFontWeightStringsValidValues) {
        checkValue(input, ArkUnion<Ark_Union_Number_FontWeight_String, Ark_String>(value), expected);
    }
}

/*
 * @tc.name: setFontWeight1TestFontWeightInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, setFontWeight1TestFontWeightInvalidValues, TestSize.Level1)
{
    Ark_Union_Number_FontWeight_String initValueWeight;
    Opt_FontSettingOptions initValueOptions;

    // Initial setup
    initValueWeight = ArkUnion<Ark_Union_Number_FontWeight_String, Ark_FontWeight>(
        std::get<1>(Fixtures::testFixtureEnumFontWeightValidValues[0]));
    Ark_FontSettingOptions options = {
        .enableVariableFontWeight =
            ArkValue<Opt_Boolean>(std::get<1>(Fixtures::testFixtureBooleanValidValues[0]))
    };
    initValueOptions = ArkValue<Opt_FontSettingOptions>(options);

    auto checkValue = [this, &initValueWeight, &initValueOptions](
                          const std::string& input, const Ark_Union_Number_FontWeight_String& value) {
        Ark_Union_Number_FontWeight_String inputValueWeight = initValueWeight;
        Opt_FontSettingOptions inputValueOptions = initValueOptions;

        modifier_->setFontWeight1(node_, &inputValueWeight, &inputValueOptions);
        inputValueWeight = value;
        modifier_->setFontWeight1(node_, &inputValueWeight, &inputValueOptions);
        auto jsonValue = GetJsonValue(node_);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_FONT_WEIGHT_NAME);
        EXPECT_EQ(resultStr, ATTRIBUTE_FONT_WEIGHT_DEFAULT_VALUE)
            << "Input value is: " << input << ", method: setFontWeight1, attribute: fontWeight";
    };

    for (auto& [input, value] : Fixtures::testFixtureEnumFontWeightInvalidValues) {
        checkValue(input, ArkUnion<Ark_Union_Number_FontWeight_String, Ark_FontWeight>(value));
    }
    // Check invalid union
    checkValue("invalid union", ArkUnion<Ark_Union_Number_FontWeight_String, Ark_Empty>(nullptr));
}

/*
 * @tc.name: setFontWeight1TestOptionsEnableVariableFontWeightValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, setFontWeight1TestOptionsEnableVariableFontWeightValidValues, TestSize.Level1)
{
    Ark_Union_Number_FontWeight_String initValueWeight;
    Opt_FontSettingOptions initValueOptions;

    // Initial setup
    initValueWeight = ArkUnion<Ark_Union_Number_FontWeight_String, Ark_FontWeight>(
        std::get<1>(Fixtures::testFixtureEnumFontWeightValidValues[0]));
    Ark_FontSettingOptions options = {
        .enableVariableFontWeight =
            ArkValue<Opt_Boolean>(std::get<1>(Fixtures::testFixtureBooleanValidValues[0]))
    };
    initValueOptions = ArkValue<Opt_FontSettingOptions>(options);

    auto checkValue = [this, &initValueWeight, &initValueOptions](
                          const std::string& input, const Opt_Boolean& value, const std::string& expectedStr) {
        Ark_Union_Number_FontWeight_String inputValueWeight = initValueWeight;
        Opt_FontSettingOptions inputValueOptions = initValueOptions;

        inputValueOptions.value.enableVariableFontWeight = value;
        modifier_->setFontWeight1(node_, &inputValueWeight, &inputValueOptions);
        auto jsonValue = GetJsonValue(node_);
        auto font = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_FONT_NAME);
        auto resultStr = GetAttrValue<std::string>(font, ATTRIBUTE_ENABLE_VARIABLE_FONT_WEIGHT_NAME);
        EXPECT_EQ(resultStr, expectedStr)
            << "Input value is: " << input
            << ", method: setFontWeight1, attribute: font.enableVariableFontWeight";
    };

    for (auto& [input, value, expected] : Fixtures::testFixtureBooleanValidValues) {
        checkValue(input, ArkValue<Opt_Boolean>(value), expected);
    }
}

/*
 * @tc.name: setFontWeight1TestVariableFontWeightValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, setFontWeight1TestVariableFontWeightValidValues, TestSize.Level1)
{
    Ark_Union_Number_FontWeight_String initValueWeight;
    Opt_FontSettingOptions initValueOptions;

    // Initial setup
    initValueWeight = ArkUnion<Ark_Union_Number_FontWeight_String, Ark_FontWeight>(
        std::get<1>(Fixtures::testFixtureEnumFontWeightValidValues[0]));
    Ark_FontSettingOptions options = {
        .enableVariableFontWeight =
            ArkValue<Opt_Boolean>(std::get<1>(Fixtures::testFixtureBooleanValidValues[0]))
    };
    initValueOptions = ArkValue<Opt_FontSettingOptions>(options);

    auto checkValue = [this, &initValueWeight, &initValueOptions](const std::string& input,
                          const Ark_Union_Number_FontWeight_String& value, const std::string& expectedStr) {
        Ark_Union_Number_FontWeight_String inputValueWeight = initValueWeight;
        Opt_FontSettingOptions inputValueOptions = initValueOptions;

        inputValueWeight = value;
        modifier_->setFontWeight1(node_, &inputValueWeight, &inputValueOptions);
        auto jsonValue = GetJsonValue(node_);
        auto font = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_FONT_NAME);
        auto resultStr = GetAttrValue<std::string>(font, ATTRIBUTE_VARIABLE_FONT_WEIGHT_NAME);
        EXPECT_EQ(resultStr, expectedStr)
            << "Input value is: " << input << ", method: setFontWeight1, attribute: fontWeight";
    };

    for (auto& [input, value, expected] : Fixtures::testFixtureVariableFontWeightEnumValidValues) {
        checkValue(input, ArkUnion<Ark_Union_Number_FontWeight_String, Ark_FontWeight>(value), expected);
    }
    for (auto& [input, value, expected] : Fixtures::testFixtureFontWeightNumbersValidValues) {
        checkValue(input, ArkUnion<Ark_Union_Number_FontWeight_String, Ark_Number>(value), expected);
    }
    for (auto& [input, value, expected] : Fixtures::testFixtureFontWeightStringsValidValues) {
        checkValue(input, ArkUnion<Ark_Union_Number_FontWeight_String, Ark_String>(value), expected);
    }
}

/*
 * @tc.name: setFontWeight1TestVariableFontWeightInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, setFontWeight1TestVariableFontWeightInvalidValues, TestSize.Level1)
{
    Ark_Union_Number_FontWeight_String initValueWeight;
    Opt_FontSettingOptions initValueOptions;

    // Initial setup
    initValueWeight = ArkUnion<Ark_Union_Number_FontWeight_String, Ark_FontWeight>(
        std::get<1>(Fixtures::testFixtureEnumFontWeightValidValues[0]));
    Ark_FontSettingOptions options = {
        .enableVariableFontWeight =
            ArkValue<Opt_Boolean>(std::get<1>(Fixtures::testFixtureBooleanValidValues[0]))
    };
    initValueOptions = ArkValue<Opt_FontSettingOptions>(options);

    auto checkValue = [this, &initValueWeight, &initValueOptions](
                          const std::string& input, const Ark_Union_Number_FontWeight_String& value) {
        Ark_Union_Number_FontWeight_String inputValueWeight = initValueWeight;
        Opt_FontSettingOptions inputValueOptions = initValueOptions;

        modifier_->setFontWeight1(node_, &inputValueWeight, &inputValueOptions);
        inputValueWeight = value;
        modifier_->setFontWeight1(node_, &inputValueWeight, &inputValueOptions);
        auto jsonValue = GetJsonValue(node_);
        auto font = GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_FONT_NAME);
        auto resultStr = GetAttrValue<std::string>(font, ATTRIBUTE_VARIABLE_FONT_WEIGHT_NAME);
        EXPECT_EQ(resultStr, ATTRIBUTE_VARIABLE_FONT_WEIGHT_DEFAULT_VALUE)
            << "Input value is: " << input << ", method: setFontWeight1, attribute: fontWeight";
    };

    for (auto& [input, value] : Fixtures::testFixtureVariableFontWeightEnumInvalidValues) {
        checkValue(input, ArkUnion<Ark_Union_Number_FontWeight_String, Ark_FontWeight>(value));
    }
    for (auto& [input, value] : Fixtures::testFixtureFontWeightNumbersInvalidValues) {
        checkValue(input, ArkUnion<Ark_Union_Number_FontWeight_String, Ark_Number>(value));
    }
    for (auto& [input, value] : Fixtures::testFixtureFontWeightStringsInvalidValues) {
        checkValue(input, ArkUnion<Ark_Union_Number_FontWeight_String, Ark_String>(value));
    }
    // Check invalid union
    checkValue("invalid union", ArkUnion<Ark_Union_Number_FontWeight_String, Ark_Empty>(nullptr));
}

/*
 * @tc.name: setTextShadowTestArrayValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, setTextShadowTestArrayValues, TestSize.Level1)
{
    std::string resultStr;
    Ark_ShadowOptions shadow1 = {
        .color = ArkUnion<Opt_Union_Color_String_Resource_ColoringStrategy, Ark_String>("#FFAABB01"),
        .offsetX = { .tag = ARK_TAG_OBJECT, .value = ArkUnion<Ark_Union_Number_Resource, Ark_Number>(50.f) },
        .offsetY = { .tag = ARK_TAG_OBJECT, .value = ArkUnion<Ark_Union_Number_Resource, Ark_Number>(55.f) },
        .radius = ArkUnion<Ark_Union_Number_Resource, Ark_Number>(12),
        .type = { .tag = ARK_TAG_OBJECT, .value = ARK_SHADOW_TYPE_COLOR }
    };
    Ark_ShadowOptions shadow2 = {
        .color = ArkUnion<Opt_Union_Color_String_Resource_ColoringStrategy, Ark_String>("#FFAABB22"),
        .offsetX = { .tag = ARK_TAG_OBJECT, .value = ArkUnion<Ark_Union_Number_Resource, Ark_Number>(70.f) },
        .offsetY = { .tag = ARK_TAG_OBJECT, .value = ArkUnion<Ark_Union_Number_Resource, Ark_Number>(75.f) },
        .radius = ArkUnion<Ark_Union_Number_Resource, Ark_Number>(20),
        .type = { .tag = ARK_TAG_OBJECT, .value = ARK_SHADOW_TYPE_BLUR }
    };
    std::vector<Ark_ShadowOptions> vec = { shadow1, shadow2 };
    Array_ShadowOptions array = { .array = vec.data(), .length = vec.size() };
    auto inputValueTextShadow = ArkUnion<Ark_Union_ShadowOptions_Array_ShadowOptions, Array_ShadowOptions>(array);
    modifier_->setTextShadow(node_, &inputValueTextShadow);

    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::unique_ptr<JsonValue> resultTextShadow =
        GetAttrValue<std::unique_ptr<JsonValue>>(jsonValue, ATTRIBUTE_TEXT_SHADOW_NAME);
    ASSERT_NE(resultTextShadow, nullptr);
    ASSERT_EQ(resultTextShadow->GetArraySize(), vec.size());

    std::unique_ptr<JsonValue> item;
    auto get = [&item](const std::string& attrName) {
        return GetAttrValue<std::string>(item, attrName);
    };

    item = resultTextShadow->GetArrayItem(0);
    ASSERT_NE(item, nullptr);
    EXPECT_EQ(get(ATTRIBUTE_TEXT_SHADOW_I_COLOR_NAME), "#FFAABB01");
    EXPECT_EQ(get(ATTRIBUTE_TEXT_SHADOW_I_OFFSET_X_NAME), "50.000000");
    EXPECT_EQ(get(ATTRIBUTE_TEXT_SHADOW_I_OFFSET_Y_NAME), "55.000000");
    EXPECT_EQ(get(ATTRIBUTE_TEXT_SHADOW_I_RADIUS_NAME), "12.000000");
    EXPECT_EQ(get(ATTRIBUTE_TEXT_SHADOW_I_TYPE_NAME), "0");

    item = resultTextShadow->GetArrayItem(1);
    ASSERT_NE(item, nullptr);
    EXPECT_EQ(get(ATTRIBUTE_TEXT_SHADOW_I_COLOR_NAME), "#FFAABB22");
    EXPECT_EQ(get(ATTRIBUTE_TEXT_SHADOW_I_OFFSET_X_NAME), "70.000000");
    EXPECT_EQ(get(ATTRIBUTE_TEXT_SHADOW_I_OFFSET_Y_NAME), "75.000000");
    EXPECT_EQ(get(ATTRIBUTE_TEXT_SHADOW_I_RADIUS_NAME), "20.000000");
    EXPECT_EQ(get(ATTRIBUTE_TEXT_SHADOW_I_TYPE_NAME), "1");
}

/*
 * @tc.name: setTextOptionsTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, setTextOptionsTestDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::string resultStr;

    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_CONTENT_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_CONTENT_DEFAULT_VALUE) << "Default value for attribute 'content'";
}

/*
 * @tc.name: setTextOptionsTestContentValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest, setTextOptionsTestContentValidValues, TestSize.Level1)
{
    auto checkValue = [this](const std::string& input, const std::string& expectedStr,
                          const Opt_Union_String_Resource& value) {
        auto textOptions = Converter::ArkValue<Opt_TextOptions>(Ark_Empty());

        modifier_->setTextOptions(node_, &value, &textOptions);
        auto jsonValue = GetJsonValue(node_);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_CONTENT_NAME);
        EXPECT_EQ(resultStr, expectedStr) <<
            "Input value is: " << input << ", method: setTextOptions, attribute: content";
    };

    for (auto& [input, value, expected] : Fixtures::testFixtureStringResValidValues) {
        checkValue(input, expected, ArkUnion<Opt_Union_String_Resource, Ark_Resource>(value));
    }
    for (auto& [input, value, expected] : Fixtures::testFixtureStringValidValues) {
        checkValue(input, expected, ArkUnion<Opt_Union_String_Resource, Ark_String>(value));
    }
}
}
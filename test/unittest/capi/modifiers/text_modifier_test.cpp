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

#include "core/interfaces/arkoala/utility/reverse_converter.h"

#include "core/components/text/text_theme.h"
#include "core/components_ng/pattern/text/text_event_hub.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
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

inline Ark_Resource ArkResource(Ark_String* name, int id = -1,
    NodeModifier::ResourceType type = NodeModifier::ResourceType::COLOR)
{
    return {
        .id = Converter::ArkValue<Ark_Number>(id),
        .type = Converter::ArkValue<Ark_Number>(static_cast<int>(type)),
        .moduleName = Converter::ArkValue<Ark_String>(""),
        .bundleName = Converter::ArkValue<Ark_String>(""),
        .params = {
            .tag = ARK_TAG_OBJECT,
            .value = { .array = name, .length = name ? 1 : 0 }
        }
    };
}

namespace {
    struct EventsTracker {
        static inline GENERATED_ArkUITextEventsReceiver textEventReceiver {};

        static inline const GENERATED_ArkUIEventsAPI eventsApiImpl = {
            .getTextEventsReceiver = [] () -> const GENERATED_ArkUITextEventsReceiver* {
                return &textEventReceiver;
            }
        };
    };
}

class TextModifierTest : public ModifierTestBase<GENERATED_ArkUITextModifier,
    &GENERATED_ArkUINodeModifiers::getTextModifier, GENERATED_ARKUI_TEXT> {
public:
    static void SetUpTestCase()
    {
        ModifierTestBase::SetUpTestCase();

        SetupTheme<TextTheme>();

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

    auto resName = Converter::ArkValue<Ark_String>("aa.bb.cc");
    Ark_ResourceColor resNameColor = { .selector = 3, .value3 = ArkResource(&resName) };
    modifier_->setFontColor(node_, &resNameColor);
    auto checkVal7 = GetStringAttribute(node_, FONT_COLOR_ATTR);
    EXPECT_EQ(checkVal7, "#FFFF0000"); // Color::RED is result of mocked ThemeConstants::GetColorByName
}

HWTEST_F(TextModifierTest, setFontSize, TestSize.Level1)
{
    const Union_Number_String_Resource size1 = {
        .selector = 0,
        .value0 = Converter::ArkValue<Ark_Number>(10)
    };
    modifier_->setFontSize(node_, &size1);
    auto checkVal1 = GetStringAttribute(node_, FONT_SIZE_ATTR);
    EXPECT_EQ(checkVal1, "10.00vp");

    const Union_Number_String_Resource size2 = {
        .selector = 1,
        .value1 = Converter::ArkValue<Ark_String>("10.00vp")
    };

    modifier_->setFontSize(node_, &size2);
    auto checkVal2 = GetStringAttribute(node_, FONT_SIZE_ATTR);
    EXPECT_EQ(checkVal2, "10.00vp");

    auto resName = Converter::ArkValue<Ark_String>("aa.bb.cc");
    const Union_Number_String_Resource size3 = {
        .selector = 2,
        .value2 = ArkResource(&resName)
    };

    modifier_->setFontSize(node_, &size3);
    auto checkVal3 = GetStringAttribute(node_, FONT_SIZE_ATTR);
    EXPECT_EQ(checkVal3, "14.00px");
}

HWTEST_F(TextModifierTest, setMinFontSize, TestSize.Level1)
{
    const Union_Number_String_Resource size1 = {
        .selector = 0,
        .value0 = Converter::ArkValue<Ark_Number>(11.25f)
    };
    modifier_->setMinFontSize(node_, &size1);
    auto checkVal1 = GetStringAttribute(node_, MIN_FONT_SIZE_ATTR);
    EXPECT_EQ(checkVal1, "11.25vp");

    const Union_Number_String_Resource size2 = {
        .selector = 1,
        .value1 = Converter::ArkValue<Ark_String>("11.25px")
    };

    modifier_->setMinFontSize(node_, &size2);
    auto checkVal2 = GetStringAttribute(node_, MIN_FONT_SIZE_ATTR);
    EXPECT_EQ(checkVal2, "11.25px");

    auto resName = Converter::ArkValue<Ark_String>("aa.bb.cc");
    const Union_Number_String_Resource size3 = {
        .selector = 2,
        .value2 = ArkResource(&resName, -1, NodeModifier::ResourceType::FLOAT)
    };

    modifier_->setMinFontSize(node_, &size3);
    auto checkVal3 = GetStringAttribute(node_, MIN_FONT_SIZE_ATTR);
    EXPECT_EQ(checkVal3, "10.00px");
}

HWTEST_F(TextModifierTest, setMaxFontSize, TestSize.Level1)
{
    const Union_Number_String_Resource size1 = {
        .selector = 0,
        .value0 = Converter::ArkValue<Ark_Number>(11.25f)
    };
    modifier_->setMaxFontSize(node_, &size1);
    auto checkVal1 = GetStringAttribute(node_, MAX_FONT_SIZE_ATTR);
    EXPECT_EQ(checkVal1, "11.25vp");

    const Union_Number_String_Resource size2 = {
        .selector = 1,
        .value1 = Converter::ArkValue<Ark_String>("11.25px")
    };

    modifier_->setMaxFontSize(node_, &size2);
    auto checkVal2 = GetStringAttribute(node_, MAX_FONT_SIZE_ATTR);
    EXPECT_EQ(checkVal2, "11.25px");

    auto resName = Converter::ArkValue<Ark_String>("aa.bb.cc");
    const Union_Number_String_Resource size3 = {
        .selector = 2,
        .value2 = ArkResource(&resName, -1, NodeModifier::ResourceType::FLOAT)
    };

    modifier_->setMaxFontSize(node_, &size3);
    auto checkVal3 = GetStringAttribute(node_, MAX_FONT_SIZE_ATTR);
    EXPECT_EQ(checkVal3, "10.00px");
}

HWTEST_F(TextModifierTest, setMinFontScale, TestSize.Level1)
{
    const Union_Number_Resource scale1 = {
        .selector = 0,
        .value0 = Converter::ArkValue<Ark_Number>(1.25f)
    };

    modifier_->setMinFontScale(node_, &scale1);
    auto checkVal1 = GetStringAttribute(node_, MIN_FONT_SCALE_ATTR);
    EXPECT_EQ(checkVal1, "1.250000");

    auto resName = Converter::ArkValue<Ark_String>("aa.bb.cc");
    const Union_Number_Resource scale2 = {
        .selector = 1,
        .value1 = ArkResource(&resName, -1, NodeModifier::ResourceType::FLOAT)
    };

    modifier_->setMinFontScale(node_, &scale2);
    auto checkVal2 = GetStringAttribute(node_, MIN_FONT_SCALE_ATTR);
    EXPECT_EQ(checkVal2, "0.000000");
}

HWTEST_F(TextModifierTest, setMaxFontScale, TestSize.Level1)
{
    const Union_Number_Resource scale1 = {
        .selector = 0,
        .value0 = Converter::ArkValue<Ark_Number>(1.25f)
    };

    modifier_->setMaxFontScale(node_, &scale1);
    auto checkVal1 = GetStringAttribute(node_, MAX_FONT_SCALE_ATTR);
    EXPECT_EQ(checkVal1, "1.250000");

    auto resName = Converter::ArkValue<Ark_String>("aa.bb.cc");
    const Union_Number_Resource scale2 = {
        .selector = 1,
        .value1 = ArkResource(&resName, -1, NodeModifier::ResourceType::FLOAT)
    };

    modifier_->setMaxFontScale(node_, &scale2);
    auto checkVal2 = GetStringAttribute(node_, MAX_FONT_SCALE_ATTR);
    EXPECT_EQ(checkVal2, "0.000000");
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
    const Union_Number_FontWeight_String w1 = {
        .selector = 0,
        .value0 = Converter::ArkValue<Ark_Number>(100)
    };
    modifier_->setFontWeight0(node_, &w1);
    auto checkVal1 = GetStringAttribute(node_, FONT_WEIGHT_ATTR);
    EXPECT_EQ(checkVal1, "100");

    const Union_Number_FontWeight_String w2 = {
        .selector = 1,
        .value1 = ARK_FONT_WEIGHT_MEDIUM
    };
    modifier_->setFontWeight0(node_, &w2);
    auto checkVal2 = GetStringAttribute(node_, FONT_WEIGHT_ATTR);
    EXPECT_EQ(checkVal2, "FontWeight.Medium");

    const Union_Number_FontWeight_String w3 = {
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
    const Union_Number_String_Resource size1 = {
        .selector = 0,
        .value0 = Converter::ArkValue<Ark_Number>(11)
    };
    modifier_->setLineHeight(node_, &size1);
    auto checkVal1 = GetStringAttribute(node_, LINE_HEIGHT_ATTR);
    EXPECT_EQ(checkVal1, "11.00vp");

    const Union_Number_String_Resource size2 = {
        .selector = 1,
        .value1 = Converter::ArkValue<Ark_String>("12.00vp")
    };

    modifier_->setLineHeight(node_, &size2);
    auto checkVal2 = GetStringAttribute(node_, LINE_HEIGHT_ATTR);
    EXPECT_EQ(checkVal2, "12.00vp");

    auto resName = Converter::ArkValue<Ark_String>("aa.bb.cc");
    const Union_Number_String_Resource size3 = {
        .selector = 2,
        .value2 = ArkResource(&resName, -1, NodeModifier::ResourceType::FLOAT)
    };

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

HWTEST_F(TextModifierTest, setFontFamily, TestSize.Level1)
{
    Union_String_Resource v1 = {
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
    Union_Number_String v1 = {
        .selector = 0,
        .value0 = Converter::ArkValue<Ark_Number>(11.25f)
    };

    modifier_->setLetterSpacing(node_, &v1);
    auto checkVal1 = GetStringAttribute(node_, LETTER_SPACING_ATTR);
    EXPECT_EQ(checkVal1, "11.25vp");

    Union_Number_String v2 = {
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
    Union_Number_String v1 = {
        .selector = 0,
        .value0 = Converter::ArkValue<Ark_Number>(11)
    };

    modifier_->setBaselineOffset(node_, &v1);
    auto checkVal1 = GetStringAttribute(node_, BASELINE_OFFSET_ATTR);
    EXPECT_EQ(checkVal1, "11");

    Union_Number_String v2 = {
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

    Union_ShadowOptions_Array_ShadowOptions v1 = {
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
    Ark_Function func = {};
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

    modifier_->setOnCopy(node_, func);

    // check true value
    EXPECT_EQ(checkEvent.has_value(), false);
    eventHub->FireOnCopy("test_string");
    EXPECT_EQ(checkEvent.has_value(), true);
    EXPECT_EQ(checkEvent->nodeId, frameNode->GetId());
    EXPECT_EQ(checkEvent->value, "test_string");
}

HWTEST_F(TextModifierTest, setOnSelectionChange, TestSize.Level1)
{
    Ark_Function func = {};
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

    modifier_->setOnTextSelectionChange(node_, func);

    // check true value
    EXPECT_EQ(selectionEvent.has_value(), false);
    eventHub->FireOnSelectionChange(1, 10);
    EXPECT_EQ(selectionEvent.has_value(), true);
    EXPECT_EQ(selectionEvent->nodeId, frameNode->GetId());
    EXPECT_EQ(selectionEvent->start, 1);
    EXPECT_EQ(selectionEvent->end, 10);
}
}
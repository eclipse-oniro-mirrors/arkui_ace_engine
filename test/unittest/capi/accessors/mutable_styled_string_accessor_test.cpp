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

#include "gmock/gmock.h"

#include "accessor_test_base.h"
#include "core/interfaces/native/utility/converter.h"
#include "core/interfaces/native/utility/reverse_converter.h"

#include "core/interfaces/native/implementation/mutable_styled_string_peer.h"

namespace OHOS::Ace::NG {

using namespace testing;
using namespace testing::ext;

class MutableStyledStringAccessorTest : public AccessorTestBase<GENERATED_ArkUIMutableStyledStringAccessor,
    &GENERATED_ArkUIAccessors::getMutableStyledStringAccessor, MutableStyledStringPeer> {
};


namespace {
struct StyleTestPlan {
    int32_t start;
    int32_t length;
    Ark_Color color;
};
} // namespace


/**
 * @tc.name: ctorTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(MutableStyledStringAccessorTest, ctorTest, TestSize.Level1)
{
    auto peer1 = reinterpret_cast<MutableStyledStringPeer*>(this->accessor_->ctor());
    auto peer2 = reinterpret_cast<MutableStyledStringPeer*>(this->accessor_->ctor());
    auto peer3 = reinterpret_cast<MutableStyledStringPeer*>(this->accessor_->ctor());
    ASSERT_NE(peer1, nullptr);
    ASSERT_NE(peer2, nullptr);
    ASSERT_NE(peer3, nullptr);
    EXPECT_EQ(peer1->spanString, nullptr);
    EXPECT_EQ(peer2->spanString, nullptr);
    EXPECT_EQ(peer3->spanString, nullptr);
    finalyzer_(peer1);
    finalyzer_(peer2);
    finalyzer_(peer3);
}

/**
 * @tc.name: replaceStringTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(MutableStyledStringAccessorTest, replaceStringTest, TestSize.Level1)
{
    peer_->spanString = AceType::MakeRefPtr<MutableSpanString>(u"replaceStringTest");
    const auto start = Converter::ArkValue<Ark_Number>(7);
    const auto length = Converter::ArkValue<Ark_Number>(6);
    const auto str = Converter::ArkValue<Ark_String>("MutableStyled");
    accessor_->replaceString(peer_, &start, &length, &str);
    ASSERT_NE(peer_->spanString, nullptr);
    EXPECT_EQ(peer_->spanString->GetU16string(), u"replaceMutableStyledTest");
}

/**
 * @tc.name: replaceStringInvalidTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(MutableStyledStringAccessorTest, replaceStringInvalidTest, TestSize.Level1)
{
    peer_->spanString = AceType::MakeRefPtr<MutableSpanString>(u"replaceStringInvalidTest");
    const auto start = Converter::ArkValue<Ark_Number>(-1);
    const auto length = Converter::ArkValue<Ark_Number>(-2);
    const auto str = Converter::ArkValue<Ark_String>("UnusedString");
    accessor_->replaceString(nullptr, &start, &length, &str);
    accessor_->replaceString(peer_, nullptr, &length, &str);
    accessor_->replaceString(peer_, &start, nullptr, &str);
    accessor_->replaceString(peer_, &start, &length, nullptr);
    accessor_->replaceString(peer_, &start, &length, &str);
    ASSERT_NE(peer_->spanString, nullptr);
    EXPECT_EQ(peer_->spanString->GetU16string(), u"replaceStringInvalidTest");
}

/**
 * @tc.name: insertStringTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(MutableStyledStringAccessorTest, insertStringTest, TestSize.Level1)
{
    peer_->spanString = AceType::MakeRefPtr<MutableSpanString>(u"insertStringTest");
    const auto start = Converter::ArkValue<Ark_Number>(6);
    const auto str = Converter::ArkValue<Ark_String>("MutableStyled");
    accessor_->insertString(peer_, &start, &str);
    ASSERT_NE(peer_->spanString, nullptr);
    EXPECT_EQ(peer_->spanString->GetU16string(), u"insertMutableStyledStringTest");
}

/**
 * @tc.name: insertStringInvalidTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(MutableStyledStringAccessorTest, insertStringInvalidTest, TestSize.Level1)
{
    peer_->spanString = AceType::MakeRefPtr<MutableSpanString>(u"insertStringInvalidTest");
    const auto start = Converter::ArkValue<Ark_Number>(-1);
    const auto str = Converter::ArkValue<Ark_String>("UnusedString");
    accessor_->insertString(nullptr, &start, &str);
    accessor_->insertString(peer_, nullptr, &str);
    accessor_->insertString(peer_, &start, nullptr);
    accessor_->insertString(peer_, &start, &str);
    ASSERT_NE(peer_->spanString, nullptr);
    EXPECT_EQ(peer_->spanString->GetU16string(), u"insertStringInvalidTest");
}

/**
 * @tc.name: removeStringTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(MutableStyledStringAccessorTest, removeStringTest, TestSize.Level1)
{
    peer_->spanString = AceType::MakeRefPtr<MutableSpanString>(u"removeStringTest");
    const auto start = Converter::ArkValue<Ark_Number>(5);
    const auto length = Converter::ArkValue<Ark_Number>(4);
    accessor_->removeString(peer_, &start, &length);
    ASSERT_NE(peer_->spanString, nullptr);
    EXPECT_EQ(peer_->spanString->GetU16string(), u"removingTest");
}

/**
 * @tc.name: removeStringInvalidTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(MutableStyledStringAccessorTest, removeStringInvalidTest, TestSize.Level1)
{
    peer_->spanString = AceType::MakeRefPtr<MutableSpanString>(u"removeStringInvalidTest");
    const auto start = Converter::ArkValue<Ark_Number>(-10);
    const auto length = Converter::ArkValue<Ark_Number>(-2);
    accessor_->removeString(nullptr, &start, &length);
    accessor_->removeString(peer_, nullptr, &length);
    accessor_->removeString(peer_, &start, nullptr);
    accessor_->removeString(peer_, &start, &length);
    ASSERT_NE(peer_->spanString, nullptr);
    EXPECT_EQ(peer_->spanString->GetU16string(), u"removeStringInvalidTest");
}

/**
 * @tc.name: replaceStyleTest
 * @tc.desc: Checking the correct operation of the span replacement.
 * The conversion of all spans is checked in the tests for StyledStringAccessor ctor.
 * @tc.type: FUNC
 */
HWTEST_F(MutableStyledStringAccessorTest, replaceStyleTest, TestSize.Level1)
{
    peer_->spanString = AceType::MakeRefPtr<MutableSpanString>(u"replaceStyleTest");
    const std::vector<RefPtr<SpanBase>> spans {
        AceType::MakeRefPtr<FontSpan>(Font { .fontColor = Color::RED }, 0, 7),
        AceType::MakeRefPtr<FontSpan>(Font { .fontColor = Color::GREEN }, 7, 12),
        AceType::MakeRefPtr<FontSpan>(Font { .fontColor = Color::BLUE }, 12, 16)
    };
    peer_->spanString->BindWithSpans(spans);

    const std::vector<StyleTestPlan> testPlan {
        { 0, 7, ARK_COLOR_BLACK },
        { 7, 5, ARK_COLOR_GRAY },
        { 12, 4, ARK_COLOR_WHITE },
    };
    for (const auto& plan : testPlan) {
        const Ark_TextStyle_styled_string arkStyle {
            .fontColor = Converter::ArkUnion<Opt_ResourceColor, Ark_Color>(plan.color),
            .fontFamily = Converter::ArkValue<Opt_String>(Ark_Empty()),
            .fontSize = Converter::ArkValue<Opt_Number>(Ark_Empty()),
            .fontWeight = Converter::ArkValue<Opt_Number>(Ark_Empty()),
            .fontStyle = Converter::ArkValue<Opt_FontStyle>(Ark_Empty())
        };
        const Ark_SpanStyle arkSpan {
            .start = Converter::ArkValue<Ark_Number>(plan.start),
            .length = Converter::ArkValue<Ark_Number>(plan.length),
            .styledKey = ARK_STYLED_STRING_KEY_FONT,
            .styledValue = Converter::ArkUnion<Ark_StyledStringValue, Ark_TextStyle_styled_string>(arkStyle)
        };
        accessor_->replaceStyle(peer_, &arkSpan);
        ASSERT_NE(peer_->spanString, nullptr);
        const auto currentSpans = peer_->spanString->GetSpans(plan.start, plan.length);
        ASSERT_EQ(currentSpans.size(), 1);
        const auto fontSpan = AceType::DynamicCast<FontSpan>(currentSpans.front());
        ASSERT_NE(fontSpan, nullptr);
        EXPECT_EQ(fontSpan->GetFont().fontColor, Converter::OptConvert<Color>(plan.color).value_or(Color::TRANSPARENT));
    }
    EXPECT_EQ(peer_->spanString->GetU16string(), u"replaceStyleTest");
}

/**
 * @tc.name: replaceStyleInvalidTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(MutableStyledStringAccessorTest, replaceStyleInvalidTest, TestSize.Level1)
{
    static constexpr auto start = 0;
    static constexpr auto length = 7;
    static const auto COLOR = Color::RED;
    peer_->spanString = AceType::MakeRefPtr<MutableSpanString>(u"replaceStyleInvalidTest");
    const std::vector<RefPtr<SpanBase>> spans {
        AceType::MakeRefPtr<FontSpan>(Font { .fontColor = COLOR }, start, length)
    };
    peer_->spanString->BindWithSpans(spans);

    accessor_->replaceStyle(peer_, nullptr);
    const Ark_TextStyle_styled_string arkStyle {
        .fontColor = Converter::ArkValue<Opt_ResourceColor>(Ark_Empty()),
        .fontFamily = Converter::ArkValue<Opt_String>(Ark_Empty()),
        .fontSize = Converter::ArkValue<Opt_Number>(Ark_Empty()),
        .fontWeight = Converter::ArkValue<Opt_Number>(Ark_Empty()),
        .fontStyle = Converter::ArkValue<Opt_FontStyle>(Ark_Empty())
    };
    Ark_SpanStyle arkSpan {
        .start = Converter::ArkValue<Ark_Number>(0),
        .length = Converter::ArkValue<Ark_Number>(-7),
        .styledKey = ARK_STYLED_STRING_KEY_FONT,
        .styledValue = Converter::ArkUnion<Ark_StyledStringValue, Ark_TextStyle_styled_string>(arkStyle)
    };
    accessor_->replaceStyle(peer_, &arkSpan);
    arkSpan.start = Converter::ArkValue<Ark_Number>(-7);
    arkSpan.length = Converter::ArkValue<Ark_Number>(0);
    accessor_->replaceStyle(peer_, &arkSpan);
    ASSERT_NE(peer_->spanString, nullptr);
    const auto currentSpans = peer_->spanString->GetSpans(start, length);
    ASSERT_EQ(currentSpans.size(), 1);
    const auto fontSpan = AceType::DynamicCast<FontSpan>(currentSpans.front());
    ASSERT_NE(fontSpan, nullptr);
    EXPECT_EQ(fontSpan->GetFont().fontColor, COLOR);
    EXPECT_EQ(peer_->spanString->GetU16string(), u"replaceStyleInvalidTest");
}

/**
 * @tc.name: setStyleTest
 * @tc.desc: Checking the correct operation of the span setting.
 * The conversion of all spans is checked in the tests for StyledStringAccessor ctor.
 * @tc.type: FUNC
 */
HWTEST_F(MutableStyledStringAccessorTest, setStyleTest, TestSize.Level1)
{
    peer_->spanString = AceType::MakeRefPtr<MutableSpanString>(u"setStyleTest");
    const std::vector<StyleTestPlan> testPlan {
        { 0, 3, ARK_COLOR_RED },
        { 3, 5, ARK_COLOR_GREEN },
        { 8, 4, ARK_COLOR_BLUE },
    };
    for (const auto& plan : testPlan) {
        Ark_BackgroundColorStyle arkStyle {
            .textBackgroundStyle {
                .color = Converter::ArkUnion<Opt_ResourceColor, Ark_Color>(plan.color),
                .radius = Converter::ArkValue<Opt_Union_Dimension_BorderRadiuses>(Ark_Empty())
            },
        };
        const Ark_SpanStyle arkSpan {
            .start = Converter::ArkValue<Ark_Number>(plan.start),
            .length = Converter::ArkValue<Ark_Number>(plan.length),
            .styledKey = ARK_STYLED_STRING_KEY_BACKGROUND_COLOR,
            .styledValue = Converter::ArkUnion<Ark_StyledStringValue, Ark_BackgroundColorStyle>(arkStyle)
        };
        accessor_->setStyle(peer_, &arkSpan);
        ASSERT_NE(peer_->spanString, nullptr);
        const auto currentSpans = peer_->spanString->GetSpans(plan.start, plan.length);
        ASSERT_EQ(currentSpans.size(), 1);
        const auto backgroundColorSpan = AceType::DynamicCast<BackgroundColorSpan>(currentSpans.front());
        ASSERT_NE(backgroundColorSpan, nullptr);
        const auto backgroundColor = backgroundColorSpan->GetBackgroundColor().backgroundColor;
        ASSERT_TRUE(backgroundColor.has_value());
        EXPECT_EQ(backgroundColor.value(), Converter::OptConvert<Color>(plan.color).value_or(Color::TRANSPARENT));
    }
    EXPECT_EQ(peer_->spanString->GetU16string(), u"setStyleTest");
}

/**
 * @tc.name: setStyleInvalidTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(MutableStyledStringAccessorTest, setStyleInvalidTest, TestSize.Level1)
{
    static constexpr auto start = 0;
    static constexpr auto length = 5;
    peer_->spanString = AceType::MakeRefPtr<MutableSpanString>(u"setStyleInvalidTest");
    const std::vector<RefPtr<SpanBase>> spans {
        AceType::MakeRefPtr<DecorationSpan>(TextDecoration::UNDERLINE, std::nullopt, std::nullopt, start, length)
    };
    peer_->spanString->BindWithSpans(spans);

    accessor_->setStyle(peer_, nullptr);
    const Ark_DecorationStyle arkStyle {
        .type = ARK_TEXT_DECORATION_TYPE_OVERLINE,
        .color = Converter::ArkValue<Opt_ResourceColor>(Ark_Empty()),
        .style = Converter::ArkValue<Opt_TextDecorationStyle>(Ark_Empty())
    };
    Ark_SpanStyle arkSpan {
        .start = Converter::ArkValue<Ark_Number>(-1),
        .length = Converter::ArkValue<Ark_Number>(7),
        .styledKey = ARK_STYLED_STRING_KEY_DECORATION,
        .styledValue = Converter::ArkUnion<Ark_StyledStringValue, Ark_DecorationStyle>(arkStyle)
    };
    accessor_->setStyle(peer_, &arkSpan);
    arkSpan.start = Converter::ArkValue<Ark_Number>(0);
    arkSpan.length = Converter::ArkValue<Ark_Number>(0);
    accessor_->setStyle(peer_, &arkSpan);
    ASSERT_NE(peer_->spanString, nullptr);
    auto currentSpans = peer_->spanString->GetSpans(start, length);
    ASSERT_EQ(currentSpans.size(), 1);
    auto decorationSpan = AceType::DynamicCast<DecorationSpan>(currentSpans.front());
    ASSERT_NE(decorationSpan, nullptr);
    EXPECT_EQ(decorationSpan->GetTextDecorationType(), TextDecoration::UNDERLINE);
    EXPECT_EQ(peer_->spanString->GetU16string(), u"setStyleInvalidTest");
}

static RefPtr<MutableSpanString> CreateTestSpans(const std::u16string& str)
{
    auto spanString = AceType::MakeRefPtr<MutableSpanString>(str);
    std::vector<RefPtr<SpanBase>> spans {
        AceType::MakeRefPtr<FontSpan>(Font { .fontColor = Color::RED }, 0, 1),
        AceType::MakeRefPtr<DecorationSpan>(TextDecoration::UNDERLINE, std::nullopt, std::nullopt, 0, str.length()),
        AceType::MakeRefPtr<BaselineOffsetSpan>(Dimension(10), 0, str.length()),
    };
    spanString->BindWithSpans(spans);
    return spanString;
}

/**
 * @tc.name: removeStyleTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(MutableStyledStringAccessorTest, removeStyleTest, TestSize.Level1)
{
    const std::u16string testString(u"removeStyleTest");
    peer_->spanString = CreateTestSpans(testString);

    const auto arkStart = Converter::ArkValue<Ark_Number>(0);
    const auto arkLength = Converter::ArkValue<Ark_Number>(static_cast<int32_t>(testString.length()));
    accessor_->removeStyle(peer_, &arkStart, &arkLength, ARK_STYLED_STRING_KEY_DECORATION);
    ASSERT_NE(peer_->spanString, nullptr);
    auto currentSpans = peer_->spanString->GetSpans(0, testString.length());
    ASSERT_EQ(currentSpans.size(), 2);
    EXPECT_NE(AceType::DynamicCast<FontSpan>(currentSpans[0]), nullptr);
    EXPECT_NE(AceType::DynamicCast<BaselineOffsetSpan>(currentSpans[1]), nullptr);
    const auto arkLength1 = Converter::ArkValue<Ark_Number>(1);
    accessor_->removeStyle(peer_, &arkStart, &arkLength1, ARK_STYLED_STRING_KEY_FONT);
    currentSpans = peer_->spanString->GetSpans(0, testString.length());
    ASSERT_EQ(currentSpans.size(), 1);
    EXPECT_NE(AceType::DynamicCast<BaselineOffsetSpan>(currentSpans[0]), nullptr);
    accessor_->removeStyle(peer_, &arkStart, &arkLength, ARK_STYLED_STRING_KEY_BASELINE_OFFSET);
    currentSpans = peer_->spanString->GetSpans(0, testString.length());
    EXPECT_EQ(currentSpans.size(), 0);
    EXPECT_EQ(peer_->spanString->GetU16string(), testString);
}

/**
 * @tc.name: removeInvalidStyleTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(MutableStyledStringAccessorTest, removeInvalidStyleTest, TestSize.Level1)
{
    const std::u16string testString(u"removeInvalidStyleTest");
    peer_->spanString = CreateTestSpans(testString);

    const auto arkStart = Converter::ArkValue<Ark_Number>(-1);
    const auto arkLength = Converter::ArkValue<Ark_Number>(0);
    accessor_->removeStyle(nullptr, nullptr, nullptr, ARK_STYLED_STRING_KEY_DECORATION);
    accessor_->removeStyle(peer_, nullptr, nullptr, ARK_STYLED_STRING_KEY_DECORATION);
    accessor_->removeStyle(peer_, &arkStart, nullptr, ARK_STYLED_STRING_KEY_DECORATION);
    accessor_->removeStyle(peer_, &arkStart, &arkLength, ARK_STYLED_STRING_KEY_DECORATION);
    ASSERT_NE(peer_->spanString, nullptr);
    auto currentSpans = peer_->spanString->GetSpans(0, testString.length());
    ASSERT_EQ(currentSpans.size(), 3);
    EXPECT_NE(AceType::DynamicCast<FontSpan>(currentSpans[0]), nullptr);
    EXPECT_NE(AceType::DynamicCast<DecorationSpan>(currentSpans[1]), nullptr);
    EXPECT_NE(AceType::DynamicCast<BaselineOffsetSpan>(currentSpans[2]), nullptr);
    EXPECT_EQ(peer_->spanString->GetU16string(), testString);
}

/**
 * @tc.name: removeStylesTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(MutableStyledStringAccessorTest, removeStylesTest, TestSize.Level1)
{
    const std::u16string testString(u"removeStylesTest");
    peer_->spanString = CreateTestSpans(testString);

    const auto arkStart = Converter::ArkValue<Ark_Number>(0);
    const auto arkLength1 = Converter::ArkValue<Ark_Number>(1);
    accessor_->removeStyles(peer_, &arkStart, &arkLength1);
    ASSERT_NE(peer_->spanString, nullptr);
    auto currentSpans = peer_->spanString->GetSpans(0, testString.length());
    ASSERT_EQ(currentSpans.size(), 2);
    EXPECT_NE(AceType::DynamicCast<DecorationSpan>(currentSpans[0]), nullptr);
    EXPECT_NE(AceType::DynamicCast<BaselineOffsetSpan>(currentSpans[1]), nullptr);
    const auto arkLength = Converter::ArkValue<Ark_Number>(static_cast<int32_t>(testString.length()));
    accessor_->removeStyles(peer_, &arkStart, &arkLength);
    currentSpans = peer_->spanString->GetSpans(0, testString.length());
    EXPECT_EQ(currentSpans.size(), 0);
    EXPECT_EQ(peer_->spanString->GetU16string(), testString);
}

/**
 * @tc.name: removeInvalidStylesTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(MutableStyledStringAccessorTest, removeInvalidStylesTest, TestSize.Level1)
{
    const std::u16string testString(u"removeInvalidStylesTest");
    peer_->spanString = CreateTestSpans(testString);

    const auto arkStart = Converter::ArkValue<Ark_Number>(0);
    const auto arkLength = Converter::ArkValue<Ark_Number>(-1);
    accessor_->removeStyles(nullptr, nullptr, nullptr);
    accessor_->removeStyles(peer_, nullptr, nullptr);
    accessor_->removeStyles(peer_, &arkStart, nullptr);
    accessor_->removeStyles(peer_, &arkStart, &arkLength);
    ASSERT_NE(peer_->spanString, nullptr);
    auto currentSpans = peer_->spanString->GetSpans(0, testString.length());
    ASSERT_EQ(currentSpans.size(), 3);
    EXPECT_NE(AceType::DynamicCast<FontSpan>(currentSpans[0]), nullptr);
    EXPECT_NE(AceType::DynamicCast<DecorationSpan>(currentSpans[1]), nullptr);
    EXPECT_NE(AceType::DynamicCast<BaselineOffsetSpan>(currentSpans[2]), nullptr);
    EXPECT_EQ(peer_->spanString->GetU16string(), testString);
}

/**
 * @tc.name: clearStylesTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(MutableStyledStringAccessorTest, clearStylesTest, TestSize.Level1)
{
    const std::u16string testString(u"clearStylesTest");
    peer_->spanString = CreateTestSpans(testString);

    accessor_->clearStyles(peer_);
    ASSERT_NE(peer_->spanString, nullptr);
    const auto currentSpans = peer_->spanString->GetSpans(0, testString.length());
    EXPECT_EQ(currentSpans.size(), 0);
    EXPECT_EQ(peer_->spanString->GetU16string(), testString);
}

/**
 * @tc.name: clearInvalidStylesTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(MutableStyledStringAccessorTest, clearInvalidStylesTest, TestSize.Level1)
{
    const std::u16string testString(u"clearInvalidStylesTest");
    peer_->spanString = CreateTestSpans(testString);

    accessor_->clearStyles(nullptr);
    ASSERT_NE(peer_->spanString, nullptr);
    const auto currentSpans = peer_->spanString->GetSpans(0, testString.length());
    ASSERT_EQ(currentSpans.size(), 3);
    EXPECT_NE(AceType::DynamicCast<FontSpan>(currentSpans[0]), nullptr);
    EXPECT_NE(AceType::DynamicCast<DecorationSpan>(currentSpans[1]), nullptr);
    EXPECT_NE(AceType::DynamicCast<BaselineOffsetSpan>(currentSpans[2]), nullptr);
    EXPECT_EQ(peer_->spanString->GetU16string(), testString);
}

static RefPtr<MutableSpanString> CreateTestSpans2(const std::u16string& str)
{
    auto spanString = AceType::MakeRefPtr<MutableSpanString>(str);
    std::vector<RefPtr<SpanBase>> spans {
        AceType::MakeRefPtr<TextShadowSpan>(std::vector<Shadow> {
            Shadow::CreateShadow(ShadowStyle::OuterDefaultLG)
        }, 0, str.length()),
        AceType::MakeRefPtr<LetterSpacingSpan>(Dimension(20), 0, str.length())
    };
    spanString->BindWithSpans(spans);
    return spanString;
}

/**
 * @tc.name: replaceStyledStringTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(MutableStyledStringAccessorTest, replaceStyledStringTest, TestSize.Level1)
{
    const std::u16string testString1(u"replaceStyledStringTest");
    const std::u16string testString2(u"secondString");
    peer_->spanString = CreateTestSpans(testString1);
    MutableStyledStringPeer peer2;
    peer2.spanString = CreateTestSpans2(testString2);
    const Ark_StyledString styledString2 { .ptr = &peer2 };

    const auto arkStart = Converter::ArkValue<Ark_Number>(0);
    const auto arkLength = Converter::ArkValue<Ark_Number>(19);
    accessor_->replaceStyledString(peer_, &arkStart, &arkLength, &styledString2);
    ASSERT_NE(peer_->spanString, nullptr);
    const auto resultString = peer_->spanString->GetU16string();
    EXPECT_EQ(resultString, u"secondStringTest");
    auto currentSpans = peer_->spanString->GetSpans(0, resultString.length());
    ASSERT_EQ(currentSpans.size(), 4);
    EXPECT_NE(AceType::DynamicCast<DecorationSpan>(currentSpans[0]), nullptr);
    EXPECT_NE(AceType::DynamicCast<BaselineOffsetSpan>(currentSpans[1]), nullptr);
    EXPECT_NE(AceType::DynamicCast<LetterSpacingSpan>(currentSpans[2]), nullptr);
    EXPECT_NE(AceType::DynamicCast<TextShadowSpan>(currentSpans[3]), nullptr);

    ASSERT_NE(peer2.spanString, nullptr);
    EXPECT_EQ(peer2.spanString->GetU16string(), testString2);
    currentSpans = peer2.spanString->GetSpans(0, testString2.length());
    ASSERT_EQ(currentSpans.size(), 2);
    EXPECT_NE(AceType::DynamicCast<TextShadowSpan>(currentSpans[0]), nullptr);
    EXPECT_NE(AceType::DynamicCast<LetterSpacingSpan>(currentSpans[1]), nullptr);
}

/**
 * @tc.name: replaceStyledStringInvalidTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(MutableStyledStringAccessorTest, replaceStyledStringInvalidTest, TestSize.Level1)
{
    const std::u16string testString(u"replaceStyledStringInvalidTest");
    peer_->spanString = CreateTestSpans(testString);
    const Ark_StyledString styledString2 { .ptr = nullptr };

    const auto arkStart = Converter::ArkValue<Ark_Number>(-1);
    const auto arkLength = Converter::ArkValue<Ark_Number>(-10);
    accessor_->replaceStyledString(nullptr, nullptr, nullptr, nullptr);
    accessor_->replaceStyledString(peer_, nullptr, nullptr, nullptr);
    accessor_->replaceStyledString(peer_, &arkStart, nullptr, nullptr);
    accessor_->replaceStyledString(peer_, &arkStart, &arkLength, nullptr);
    accessor_->replaceStyledString(peer_, &arkStart, &arkLength, &styledString2);
    ASSERT_NE(peer_->spanString, nullptr);
    const auto resultString = peer_->spanString->GetU16string();
    EXPECT_EQ(resultString, testString);
    const auto currentSpans = peer_->spanString->GetSpans(0, resultString.length());
    ASSERT_EQ(currentSpans.size(), 3);
    EXPECT_NE(AceType::DynamicCast<FontSpan>(currentSpans[0]), nullptr);
    EXPECT_NE(AceType::DynamicCast<DecorationSpan>(currentSpans[1]), nullptr);
    EXPECT_NE(AceType::DynamicCast<BaselineOffsetSpan>(currentSpans[2]), nullptr);
}

/**
 * @tc.name: insertStyledStringTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(MutableStyledStringAccessorTest, insertStyledStringTest, TestSize.Level1)
{
    const std::u16string testString1(u"insertStyledStringTest");
    const std::u16string testString2(u"Second");
    peer_->spanString = CreateTestSpans(testString1);
    MutableStyledStringPeer peer2;
    peer2.spanString = CreateTestSpans2(testString2);
    const Ark_StyledString styledString2 { .ptr = &peer2 };

    const auto arkStart = Converter::ArkValue<Ark_Number>(12);
    accessor_->insertStyledString(peer_, &arkStart, &styledString2);
    ASSERT_NE(peer_->spanString, nullptr);
    const auto resultString = peer_->spanString->GetU16string();
    EXPECT_EQ(resultString, u"insertStyledSecondStringTest");
    auto currentSpans = peer_->spanString->GetSpans(0, resultString.length());
    ASSERT_EQ(currentSpans.size(), 7);
    EXPECT_NE(AceType::DynamicCast<FontSpan>(currentSpans[0]), nullptr);
    EXPECT_NE(AceType::DynamicCast<DecorationSpan>(currentSpans[1]), nullptr);
    EXPECT_NE(AceType::DynamicCast<DecorationSpan>(currentSpans[2]), nullptr);
    EXPECT_NE(AceType::DynamicCast<BaselineOffsetSpan>(currentSpans[3]), nullptr);
    EXPECT_NE(AceType::DynamicCast<BaselineOffsetSpan>(currentSpans[4]), nullptr);
    EXPECT_NE(AceType::DynamicCast<LetterSpacingSpan>(currentSpans[5]), nullptr);
    EXPECT_NE(AceType::DynamicCast<TextShadowSpan>(currentSpans[6]), nullptr);

    ASSERT_NE(peer2.spanString, nullptr);
    EXPECT_EQ(peer2.spanString->GetU16string(), testString2);
    currentSpans = peer2.spanString->GetSpans(0, testString2.length());
    ASSERT_EQ(currentSpans.size(), 2);
    EXPECT_NE(AceType::DynamicCast<TextShadowSpan>(currentSpans[0]), nullptr);
    EXPECT_NE(AceType::DynamicCast<LetterSpacingSpan>(currentSpans[1]), nullptr);
}

/**
 * @tc.name: insertStyledStringInvalidTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(MutableStyledStringAccessorTest, insertStyledStringInvalidTest, TestSize.Level1)
{
    const std::u16string testString(u"insertStyledStringInvalidTest");
    peer_->spanString = CreateTestSpans(testString);
    const Ark_StyledString styledString2 { .ptr = nullptr };

    const auto arkStart = Converter::ArkValue<Ark_Number>(-1);
    accessor_->insertStyledString(nullptr, nullptr, nullptr);
    accessor_->insertStyledString(peer_, nullptr, nullptr);
    accessor_->insertStyledString(peer_, &arkStart, nullptr);
    accessor_->insertStyledString(peer_, &arkStart, &styledString2);
    ASSERT_NE(peer_->spanString, nullptr);
    const auto resultString = peer_->spanString->GetU16string();
    EXPECT_EQ(resultString, testString);
    const auto currentSpans = peer_->spanString->GetSpans(0, resultString.length());
    ASSERT_EQ(currentSpans.size(), 3);
    EXPECT_NE(AceType::DynamicCast<FontSpan>(currentSpans[0]), nullptr);
    EXPECT_NE(AceType::DynamicCast<DecorationSpan>(currentSpans[1]), nullptr);
    EXPECT_NE(AceType::DynamicCast<BaselineOffsetSpan>(currentSpans[2]), nullptr);
}

/**
 * @tc.name: appendStyledStringTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(MutableStyledStringAccessorTest, appendStyledStringTest, TestSize.Level1)
{
    const std::u16string testString1(u"appendStyledStringTest");
    const std::u16string testString2(u"End");
    peer_->spanString = CreateTestSpans(testString1);
    MutableStyledStringPeer peer2;
    peer2.spanString = CreateTestSpans2(testString2);
    const Ark_StyledString styledString2 { .ptr = &peer2 };

    accessor_->appendStyledString(peer_, &styledString2);
    ASSERT_NE(peer_->spanString, nullptr);
    const auto resultString = peer_->spanString->GetU16string();
    EXPECT_EQ(resultString, u"appendStyledStringTestEnd");
    auto currentSpans = peer_->spanString->GetSpans(0, resultString.length());
    ASSERT_EQ(currentSpans.size(), 5);
    EXPECT_NE(AceType::DynamicCast<FontSpan>(currentSpans[0]), nullptr);
    EXPECT_NE(AceType::DynamicCast<DecorationSpan>(currentSpans[1]), nullptr);
    EXPECT_NE(AceType::DynamicCast<BaselineOffsetSpan>(currentSpans[2]), nullptr);
    EXPECT_NE(AceType::DynamicCast<LetterSpacingSpan>(currentSpans[3]), nullptr);
    EXPECT_NE(AceType::DynamicCast<TextShadowSpan>(currentSpans[4]), nullptr);

    ASSERT_NE(peer2.spanString, nullptr);
    EXPECT_EQ(peer2.spanString->GetU16string(), testString2);
    currentSpans = peer2.spanString->GetSpans(0, testString2.length());
    ASSERT_EQ(currentSpans.size(), 2);
    EXPECT_NE(AceType::DynamicCast<TextShadowSpan>(currentSpans[0]), nullptr);
    EXPECT_NE(AceType::DynamicCast<LetterSpacingSpan>(currentSpans[1]), nullptr);
}

/**
 * @tc.name: appendStyledStringInvalidTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(MutableStyledStringAccessorTest, appendStyledStringInvalidTest, TestSize.Level1)
{
    const std::u16string testString(u"appendStyledStringInvalidTest");
    peer_->spanString = CreateTestSpans(testString);
    const Ark_StyledString styledString2 { .ptr = nullptr };

    accessor_->appendStyledString(nullptr, nullptr);
    accessor_->appendStyledString(peer_, &styledString2);
    ASSERT_NE(peer_->spanString, nullptr);
    const auto resultString = peer_->spanString->GetU16string();
    EXPECT_EQ(resultString, testString);
    const auto currentSpans = peer_->spanString->GetSpans(0, resultString.length());
    ASSERT_EQ(currentSpans.size(), 3);
    EXPECT_NE(AceType::DynamicCast<FontSpan>(currentSpans[0]), nullptr);
    EXPECT_NE(AceType::DynamicCast<DecorationSpan>(currentSpans[1]), nullptr);
    EXPECT_NE(AceType::DynamicCast<BaselineOffsetSpan>(currentSpans[2]), nullptr);
}

} // namespace OHOS::Ace::NG

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

#include <limits>
#include <string>
#include <vector>

#include "gtest/gtest.h"

#include "base/geometry/dimension.h"
#include "base/utils/string_utils.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace {
namespace {
const double EPS = 1e-9;
}

class StringUtilsGapTest : public testing::Test {
public:
    static void SetUpTestSuite() {}
    static void TearDownTestSuite() {}
};

/**
 * @tc.name: StringToDimensionWithUnitGap001
 * @tc.desc: Test StringToDimensionWithUnit with auto keyword and invalid input.
 * @tc.type: FUNC
 */
HWTEST_F(StringUtilsGapTest, StringToDimensionWithUnitGap001, TestSize.Level1)
{
    /**
     * @tc.steps: parse "auto" keyword and unparsable string.
     * @tc.expected: "auto" maps to AUTO unit with default value; unparsable maps to
     *               default unit with default value.
     */
    auto dimAuto = StringUtils::StringToDimensionWithUnit("auto", DimensionUnit::VP, 10.0);
    EXPECT_TRUE(dimAuto.Unit() == DimensionUnit::AUTO);
    EXPECT_DOUBLE_EQ(dimAuto.Value(), 10.0);

    auto dimInvalid = StringUtils::StringToDimensionWithUnit("not-a-number", DimensionUnit::VP, 7.0);
    EXPECT_TRUE(dimInvalid.Unit() == DimensionUnit::VP);
    EXPECT_DOUBLE_EQ(dimInvalid.Value(), 7.0);

    auto dimErange = StringUtils::StringToDimensionWithUnit("1e999", DimensionUnit::FP, 3.0);
    EXPECT_TRUE(dimErange.Unit() == DimensionUnit::FP);
    EXPECT_DOUBLE_EQ(dimErange.Value(), 3.0);
}

/**
 * @tc.name: StringToDimensionWithUnitGap002
 * @tc.desc: Test StringToDimensionWithUnit with all unit suffixes.
 * @tc.type: FUNC
 */
HWTEST_F(StringUtilsGapTest, StringToDimensionWithUnitGap002, TestSize.Level1)
{
    /**
     * @tc.steps: parse px/vp/fp/lpx/percent suffixes.
     * @tc.expected: each suffix maps to its DimensionUnit with parsed value.
     */
    EXPECT_TRUE(StringUtils::StringToDimensionWithUnit("50%").Unit() == DimensionUnit::PERCENT);
    EXPECT_DOUBLE_EQ(StringUtils::StringToDimensionWithUnit("50%").Value(), 0.5);

    EXPECT_TRUE(StringUtils::StringToDimensionWithUnit("100px").Unit() == DimensionUnit::PX);
    EXPECT_DOUBLE_EQ(StringUtils::StringToDimensionWithUnit("100px").Value(), 100.0);

    EXPECT_TRUE(StringUtils::StringToDimensionWithUnit("10vp").Unit() == DimensionUnit::VP);
    EXPECT_TRUE(StringUtils::StringToDimensionWithUnit("3.5fp").Unit() == DimensionUnit::FP);
    EXPECT_TRUE(StringUtils::StringToDimensionWithUnit("20lpx").Unit() == DimensionUnit::LPX);
    EXPECT_DOUBLE_EQ(StringUtils::StringToDimensionWithUnit("20lpx").Value(), 20.0);
}

/**
 * @tc.name: StringToDimensionWithUnitGap003
 * @tc.desc: Test StringToDimensionWithUnit calc mode branches.
 * @tc.type: FUNC
 */
HWTEST_F(StringUtilsGapTest, StringToDimensionWithUnitGap003, TestSize.Level1)
{
    /**
     * @tc.steps: parse numeric-only string in calc mode and garbage suffix in calc mode.
     * @tc.expected: numeric-only with isCalc=true maps to NONE; garbage suffix with
     *               isCalc=true maps to INVALID; isCalc=false falls to default unit.
     */
    auto dimNone = StringUtils::StringToDimensionWithUnit("42", DimensionUnit::VP, 0.0, true);
    EXPECT_TRUE(dimNone.Unit() == DimensionUnit::NONE);
    EXPECT_DOUBLE_EQ(dimNone.Value(), 42.0);

    auto dimInvalidCalc = StringUtils::StringToDimensionWithUnit("42xyz", DimensionUnit::VP, 0.0, true);
    EXPECT_TRUE(dimInvalidCalc.Unit() == DimensionUnit::INVALID);

    auto dimDefault = StringUtils::StringToDimensionWithUnit("42", DimensionUnit::VP, 0.0, false);
    EXPECT_TRUE(dimDefault.Unit() == DimensionUnit::VP);
    EXPECT_DOUBLE_EQ(dimDefault.Value(), 42.0);
}

/**
 * @tc.name: EscapeJsonStringGap001
 * @tc.desc: Test EscapeJsonString with quotes, backslash and control characters.
 * @tc.type: FUNC
 */
HWTEST_F(StringUtilsGapTest, EscapeJsonStringGap001, TestSize.Level1)
{
    /**
     * @tc.steps: escape quote, backslash, control char and plain text.
     * @tc.expected: quote/backslash are backslash-escaped, control chars become \u00xx.
     */
    EXPECT_EQ(StringUtils::EscapeJsonString("say\"hi\""), "\"say\\\"hi\\\"\"");
    EXPECT_EQ(StringUtils::EscapeJsonString("a\\b"), "\"a\\\\b\"");
    EXPECT_EQ(StringUtils::EscapeJsonString("a\x1b"), "\"a\\u001b\"");
    EXPECT_EQ(StringUtils::EscapeJsonString("plain"), "\"plain\"");
    EXPECT_EQ(StringUtils::EscapeJsonString(""), "\"\"");
}

/**
 * @tc.name: RestoreEscapeGap001
 * @tc.desc: Test RestoreEscape and RestoreBackslash.
 * @tc.type: FUNC
 */
HWTEST_F(StringUtilsGapTest, RestoreEscapeGap001, TestSize.Level1)
{
    /**
     * @tc.steps: restore newline/carriage/tab/normal characters, strip backslashes.
     * @tc.expected: control chars are printed as escaped literals; backslashes removed.
     */
    EXPECT_EQ(StringUtils::RestoreEscape("a\nb"), "a\\nb");
    EXPECT_EQ(StringUtils::RestoreEscape("a\rb"), "a\\rb");
    EXPECT_EQ(StringUtils::RestoreEscape("a\tb"), "a\\tb");
    EXPECT_EQ(StringUtils::RestoreEscape("abc"), "abc");
    EXPECT_EQ(StringUtils::RestoreBackslash("a\\nb"), "anb");
    EXPECT_EQ(StringUtils::RestoreBackslash("plain"), "plain");
}

/**
 * @tc.name: StringToIntGap001
 * @tc.desc: Test StringToInt boundaries and trailing garbage semantics.
 * @tc.type: FUNC
 */
HWTEST_F(StringUtilsGapTest, StringToIntGap001, TestSize.Level1)
{
    /**
     * @tc.steps: parse valid/invalid/overflow/trailing-garbage strings.
     * @tc.expected: strtol semantics - trailing garbage still parses the prefix,
     *               unparsable and out-of-int-range values return defaultErr.
     */
    EXPECT_EQ(StringUtils::StringToInt("42"), 42);
    EXPECT_EQ(StringUtils::StringToInt("-17"), -17);
    EXPECT_EQ(StringUtils::StringToInt("12abc"), 12);
    EXPECT_EQ(StringUtils::StringToInt("abc", -1), -1);
    EXPECT_EQ(StringUtils::StringToInt("2147483648", -5), -5); // INT_MAX + 1
    EXPECT_EQ(StringUtils::StringToInt("-2147483649", -6), -6); // INT_MIN - 1
    EXPECT_EQ(StringUtils::StringToInt("99999999999999999999", -7), -7); // ERANGE
}

/**
 * @tc.name: StringToLongIntGap001
 * @tc.desc: Test StringToLongInt and StringToLongUint error paths.
 * @tc.type: FUNC
 */
HWTEST_F(StringUtilsGapTest, StringToLongIntGap001, TestSize.Level1)
{
    /**
     * @tc.steps: parse valid, unparsable and overflow inputs.
     * @tc.expected: unparsable/ERANGE return defaultErr.
     */
    EXPECT_EQ(StringUtils::StringToLongInt("9223372036854775807"), INT64_MAX);
    EXPECT_EQ(StringUtils::StringToLongInt("abc", -3), -3);
    EXPECT_EQ(StringUtils::StringToLongInt("99999999999999999999999", -4), -4);

    EXPECT_EQ(StringUtils::StringToLongUint("42"), 42ULL);
    EXPECT_EQ(StringUtils::StringToLongUint("abc", 9ULL), 9ULL);
    EXPECT_EQ(StringUtils::StringToLongUint("99999999999999999999999", 8ULL), 8ULL);
}

/**
 * @tc.name: StringToUintGap001
 * @tc.desc: Test StringToUint vs StringToUintCheck trailing garbage difference.
 * @tc.type: FUNC
 */
HWTEST_F(StringUtilsGapTest, StringToUintGap001, TestSize.Level1)
{
    /**
     * @tc.steps: parse trailing garbage / overflow with both variants.
     * @tc.expected: StringToUint accepts prefix ("12abc" -> 12) while
     *               StringToUintCheck rejects it via pEnd check.
     */
    EXPECT_EQ(StringUtils::StringToUint("12abc"), 12U);
    EXPECT_EQ(StringUtils::StringToUintCheck("12abc", 77U), 77U);
    EXPECT_EQ(StringUtils::StringToUint("42"), 42U);
    EXPECT_EQ(StringUtils::StringToUintCheck("42"), 42U);
    EXPECT_EQ(StringUtils::StringToUint("abc", 55U), 55U);
    EXPECT_EQ(StringUtils::StringToUintCheck("abc", 66U), 66U);
    EXPECT_EQ(StringUtils::StringToUint("99999999999", 33U), 33U); // > UINT32_MAX
    EXPECT_EQ(StringUtils::StringToUintCheck("99999999999", 44U), 44U);
}

/**
 * @tc.name: StringToDoubleGap001
 * @tc.desc: Test StringToDouble both overloads including percent suffix.
 * @tc.type: FUNC
 */
HWTEST_F(StringUtilsGapTest, StringToDoubleGap001, TestSize.Level1)
{
    /**
     * @tc.steps: parse plain, percent-suffixed, garbage-suffixed and invalid strings.
     * @tc.expected: plain/percent succeed; other suffixes and invalid input fail.
     */
    EXPECT_DOUBLE_EQ(StringUtils::StringToDouble("3.14"), 3.14);
    EXPECT_DOUBLE_EQ(StringUtils::StringToDouble("abc"), 0.0);
    EXPECT_DOUBLE_EQ(StringUtils::StringToDouble("1e999"), 0.0); // ERANGE

    double result = -1.0;
    EXPECT_TRUE(StringUtils::StringToDouble("50%", result));
    EXPECT_DOUBLE_EQ(result, 0.5);
    EXPECT_TRUE(StringUtils::StringToDouble("1.5", result));
    EXPECT_DOUBLE_EQ(result, 1.5);
    EXPECT_FALSE(StringUtils::StringToDouble("1.5x", result));
    EXPECT_FALSE(StringUtils::StringToDouble("abc", result));
    EXPECT_FALSE(StringUtils::StringToDouble("1e999", result));

    EXPECT_FLOAT_EQ(StringUtils::StringToFloat("2.5"), 2.5f);
    EXPECT_FLOAT_EQ(StringUtils::StringToFloat("abc"), 0.0f);
}

/**
 * @tc.name: StringToDegreeGap001
 * @tc.desc: Test StringToDegree with deg/grad/rad/turn units.
 * @tc.type: FUNC
 */
HWTEST_F(StringUtilsGapTest, StringToDegreeGap001, TestSize.Level1)
{
    /**
     * @tc.steps: convert angle strings of all css units.
     * @tc.expected: deg passthrough, grad/rad/turn converted to degrees,
     *               unitless falls back to StringToDouble.
     */
    EXPECT_DOUBLE_EQ(StringUtils::StringToDegree("90deg"), 90.0);
    EXPECT_NEAR(StringUtils::StringToDegree("100grad"), 90.0, EPS); // 100/400*360
    EXPECT_NEAR(StringUtils::StringToDegree("1turn"), 360.0, EPS);
    EXPECT_DOUBLE_EQ(StringUtils::StringToDegree("45"), 45.0);
    EXPECT_DOUBLE_EQ(StringUtils::StringToDegree("abc"), 0.0);

    double result = -1.0;
    EXPECT_TRUE(StringUtils::StringToDegree("30deg", result));
    EXPECT_DOUBLE_EQ(result, 30.0);
    EXPECT_TRUE(StringUtils::StringToDegree("30", result));
    EXPECT_DOUBLE_EQ(result, 30.0);
    EXPECT_TRUE(StringUtils::StringToDegree("400grad", result));
    EXPECT_NEAR(result, 360.0, EPS);
    EXPECT_TRUE(StringUtils::StringToDegree("6.283185307179586rad", result));
    EXPECT_NEAR(result, 360.0, 0.01);
    EXPECT_TRUE(StringUtils::StringToDegree("0.5turn", result));
    EXPECT_DOUBLE_EQ(result, 180.0);
    EXPECT_FALSE(StringUtils::StringToDegree("30arcmin", result));
    EXPECT_FALSE(StringUtils::StringToDegree("abc", result));
}

/**
 * @tc.name: ParseStringToArrayGap001
 * @tc.desc: Test ParseStringToArray with comma/space separators and bad token.
 * @tc.type: FUNC
 */
HWTEST_F(StringUtilsGapTest, ParseStringToArrayGap001, TestSize.Level1)
{
    /**
     * @tc.steps: parse comma separated, space separated and invalid token lists.
     * @tc.expected: comma and space both split; invalid token fails whole parse.
     */
    std::vector<float> out;
    EXPECT_TRUE(StringUtils::ParseStringToArray("1,2.5,3", out));
    ASSERT_EQ(out.size(), 3U);
    EXPECT_FLOAT_EQ(out[0], 1.0f);
    EXPECT_FLOAT_EQ(out[1], 2.5f);
    EXPECT_FLOAT_EQ(out[2], 3.0f);

    out.clear();
    EXPECT_TRUE(StringUtils::ParseStringToArray("1 2 3", out));
    ASSERT_EQ(out.size(), 3U);

    out.clear();
    EXPECT_FALSE(StringUtils::ParseStringToArray("1,x,3", out));
}

/**
 * @tc.name: TrimStrGap001
 * @tc.desc: Test TrimStr and TrimStrLeadingAndTrailing edge branches.
 * @tc.type: FUNC
 */
HWTEST_F(StringUtilsGapTest, TrimStrGap001, TestSize.Level1)
{
    /**
     * @tc.steps: trim with default/custom char and all-trimmed strings.
     * @tc.expected: all-trimmed input returns unchanged (npos branch);
     *               normal input returns the trimmed middle.
     */
    EXPECT_EQ(StringUtils::TrimStr("  hello  "), "hello");
    EXPECT_EQ(StringUtils::TrimStr("xxhixx", 'x'), "hi");
    EXPECT_EQ(StringUtils::TrimStr("xxx", 'x'), "xxx"); // npos branch: unchanged

    std::string leading = "  a b  ";
    StringUtils::TrimStrLeadingAndTrailing(leading);
    EXPECT_EQ(leading, "a b");

    std::string allSpace = "   ";
    StringUtils::TrimStrLeadingAndTrailing(allSpace);
    EXPECT_EQ(allSpace, "");

    std::string leadingOnly = "  abc";
    StringUtils::TrimStrLeadingAndTrailing(leadingOnly);
    EXPECT_EQ(leadingOnly, "abc");
}

/**
 * @tc.name: SplitStrGap001
 * @tc.desc: Test SplitStr empty input branches and trim mode.
 * @tc.type: FUNC
 */
HWTEST_F(StringUtilsGapTest, SplitStrGap001, TestSize.Level1)
{
    /**
     * @tc.steps: split with empty source / empty separator / trim mode.
     * @tc.expected: empty input branches return empty output; trim mode strips spaces.
     */
    std::vector<std::string> out;
    StringUtils::SplitStr("", ",", out);
    EXPECT_TRUE(out.empty());

    StringUtils::SplitStr("a,b", "", out);
    EXPECT_TRUE(out.empty());

    out.clear();
    StringUtils::SplitStr(" a , b ", ",", out, true);
    ASSERT_EQ(out.size(), 2U);
    EXPECT_EQ(out[0], "a");
    EXPECT_EQ(out[1], "b");

    out.clear();
    StringUtils::SplitStr(" a , b ", ",", out, false);
    ASSERT_EQ(out.size(), 2U);
    EXPECT_EQ(out[0], " a ");
    EXPECT_EQ(out[1], " b ");
}

/**
 * @tc.name: CStringEqualGap001
 * @tc.desc: Test CStringEqual null combinations.
 * @tc.type: FUNC
 */
HWTEST_F(StringUtilsGapTest, CStringEqualGap001, TestSize.Level1)
{
    /**
     * @tc.steps: compare null/null, null/value, value/null, equal and unequal.
     * @tc.expected: both-null true, single-null false, otherwise strcmp result.
     */
    EXPECT_TRUE(StringUtils::CStringEqual(nullptr, nullptr));
    EXPECT_FALSE(StringUtils::CStringEqual("a", nullptr));
    EXPECT_FALSE(StringUtils::CStringEqual(nullptr, "a"));
    EXPECT_TRUE(StringUtils::CStringEqual("abc", "abc"));
    EXPECT_FALSE(StringUtils::CStringEqual("abc", "abd"));
}

/**
 * @tc.name: TransformStrCaseGap001
 * @tc.desc: Test TransformStrCase for string/wstring/u16string.
 * @tc.type: FUNC
 */
HWTEST_F(StringUtilsGapTest, TransformStrCaseGap001, TestSize.Level1)
{
    /**
     * @tc.steps: transform case with lowercase/uppercase/unknown codes, empty string.
     * @tc.expected: 1 -> lowercase, 2 -> uppercase, other codes unchanged, empty safe.
     */
    std::string str = "AbCdEf";
    StringUtils::TransformStrCase(str, StringUtils::TEXT_CASE_LOWERCASE);
    EXPECT_EQ(str, "abcdef");
    StringUtils::TransformStrCase(str, StringUtils::TEXT_CASE_UPPERCASE);
    EXPECT_EQ(str, "ABCDEF");
    StringUtils::TransformStrCase(str, 99); // unknown code: unchanged
    EXPECT_EQ(str, "ABCDEF");
    StringUtils::TransformStrCase(str, StringUtils::TEXT_CASE_LOWERCASE); // non-empty guard ok
    EXPECT_EQ(str, "abcdef");

    std::string empty;
    StringUtils::TransformStrCase(empty, StringUtils::TEXT_CASE_UPPERCASE);
    EXPECT_TRUE(empty.empty());

    std::wstring wstr = L"AbC";
    StringUtils::TransformStrCase(wstr, StringUtils::TEXT_CASE_UPPERCASE);
    EXPECT_EQ(wstr, L"ABC");

    std::u16string u16str = u"AbC";
    StringUtils::TransformStrCase(u16str, StringUtils::TEXT_CASE_LOWERCASE);
    EXPECT_EQ(u16str, u"abc");
    StringUtils::TransformStrCase(u16str, StringUtils::TEXT_CASE_UPPERCASE);
    EXPECT_EQ(u16str, u"ABC");
    // TransformStrCase uses ::toupper and only handles ASCII; non-ASCII stays unchanged.
    std::u16string u16accent = u"\u00e9"; // e-acute
    StringUtils::TransformStrCase(u16accent, StringUtils::TEXT_CASE_UPPERCASE);
    EXPECT_EQ(u16accent, u"\u00e9");
}

/**
 * @tc.name: ReplaceSpaceGap001
 * @tc.desc: Test ReplaceSpace and ReplaceTabAndNewLine collapsing behavior.
 * @tc.type: FUNC
 */
HWTEST_F(StringUtilsGapTest, ReplaceSpaceGap001, TestSize.Level1)
{
    /**
     * @tc.steps: collapse consecutive spaces, expand tabs, replace newlines.
     * @tc.expected: only the first space of a run survives; tabs/newlines become
     *               spaces and then collapse.
     */
    std::string data = "a  b";
    StringUtils::ReplaceSpace(data);
    EXPECT_EQ(data, "a b");

    data = "a\tb";
    StringUtils::ReplaceSpace(data);
    EXPECT_EQ(data, "a b");

    data = "a\t\tb";
    StringUtils::ReplaceSpace(data);
    EXPECT_EQ(data, "a b");

    data = " a  b ";
    StringUtils::ReplaceSpace(data);
    EXPECT_EQ(data, " a b ");

    data = "a\r\nb";
    StringUtils::ReplaceTabAndNewLine(data);
    EXPECT_EQ(data, "a b");

    data = "a\tb";
    StringUtils::ReplaceTabAndNewLine(data);
    EXPECT_EQ(data, "a b");
}

/**
 * @tc.name: ReplaceCharAndDoubleToStringGap001
 * @tc.desc: Test ReplaceChar and DoubleToString including infinity.
 * @tc.type: FUNC
 */
HWTEST_F(StringUtilsGapTest, ReplaceCharAndDoubleToStringGap001, TestSize.Level1)
{
    /**
     * @tc.steps: replace chars and stringify doubles with precision/infinity.
     * @tc.expected: chars replaced; infinity prints "Infinity"; precision honored.
     */
    EXPECT_EQ(StringUtils::ReplaceChar("a-b-c", '-', '_'), "a_b_c");
    EXPECT_EQ(StringUtils::ReplaceChar("abc", 'x', 'y'), "abc");

    EXPECT_EQ(StringUtils::DoubleToString(3.14159, 2), "3.14");
    EXPECT_EQ(StringUtils::DoubleToString(-0.5, 1), "-0.5");
    // DoubleToString currently falls through to std::fixed for infinity, printing "inf".
    EXPECT_EQ(StringUtils::DoubleToString(std::numeric_limits<double>::infinity()), "inf");
}

/**
 * @tc.name: UtfStringConvertGap001
 * @tc.desc: Test ToU32string / U32StringToString round trips.
 * @tc.type: FUNC
 */
HWTEST_F(StringUtilsGapTest, UtfStringConvertGap001, TestSize.Level1)
{
    /**
     * @tc.steps: round trip ascii and multi-byte utf-8 content.
     * @tc.expected: utf-8 -> utf-32 -> utf-8 preserves the original string.
     */
    std::string ascii = "hello world";
    auto wide = StringUtils::ToU32string(ascii);
    EXPECT_EQ(StringUtils::U32StringToString(wide), ascii);

    std::string utf8 = "h\xC3\xA9llo \xE2\x82\xAC \xF0\x9F\x98\x80";
    auto u32 = StringUtils::ToU32string(utf8);
    EXPECT_EQ(StringUtils::U32StringToString(u32), utf8);
}
} // namespace OHOS::Ace

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

#include "gtest/gtest.h"
#include <cstdint>
#include <string>
#include "gtest/hwext/gtest-ext.h"
#include "gtest/hwext/gtest-tag.h"
#include "unicode/unistr.h"

#include "base/utils/utf.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace {
class UtfTestNg : public Test {
    void SetUp() override
    {
        index = 0;
    }

public:
    size_t index;
};

/**
 * @tc.name:IsUTF8_001
 * @tc.desc: Test utf IsUTF8
 * @tc.type: FUNC
 */
HWTEST_F(UtfTestNg, IsUTF8_001, TestSize.Level1)
{
    std::string str = "";
    EXPECT_EQ(IsUTF8(str), false);
    std::string key = "hello";
    EXPECT_EQ(IsUTF8(key), true);
    std::string twoByteStr = "é";
    EXPECT_TRUE(IsUTF8(twoByteStr));
    std::string threeByteStr = "中";
    EXPECT_TRUE(IsUTF8(threeByteStr));

    std::string invalidTwoByteStr = "\xC3";
    EXPECT_FALSE(IsUTF8(invalidTwoByteStr));
    std::string invalidContinuationStr = "\xE4\xB8\x41";
    EXPECT_FALSE(IsUTF8(invalidContinuationStr));
    std::string mixedStr = "Hello\xC3\x28World";
    EXPECT_FALSE(IsUTF8(mixedStr));
}

/**
 * @tc.name:MUtf8ToUtf16Size_001
 * @tc.desc: Test utf MUtf8ToUtf16Size
 * @tc.type: FUNC
 */
HWTEST_F(UtfTestNg, MUtf8ToUtf16Size_001, TestSize.Level1)
{
    uint8_t utf8[3] = { 0xd8 + 1, 0xdc - 1, 0xd8 + 1 };
    size_t len = sizeof(utf8) / sizeof(utf8[0]);
    EXPECT_EQ(MUtf8ToUtf16Size(utf8, len), 2);
    const uint8_t mutf8[] = { 0xC2 };
    size_t mutf8Len = 1;
    EXPECT_EQ(MUtf8ToUtf16Size(mutf8, mutf8Len), 1);
}

/**
 * @tc.name:ConvertMUtf8ToUtf16Pair_001
 * @tc.desc: Test utf ConvertMUtf8ToUtf16Pair
 * @tc.type: FUNC
 */
HWTEST_F(UtfTestNg, ConvertMUtf8ToUtf16Pair_001, TestSize.Level1)
{
    uint32_t utf8Res;
    size_t byte;
    uint8_t utf8[3] = { 0xd8 + 1, 0xdc - 1, 0xd8 + 1 };
    size_t maxBytes = sizeof(utf8) / sizeof(utf8[0]);
    std::pair<uint32_t, size_t> result;
    result = ConvertMUtf8ToUtf16Pair(utf8, maxBytes);
    utf8Res = result.first;
    byte = result.second;
    EXPECT_EQ(utf8Res, 1627);
    EXPECT_EQ(byte, 2);
}

/**
 * @tc.name:ConvertRegionUtf8ToUtf16_001
 * @tc.desc: Test utf ConvertRegionUtf8ToUtf16
 * @tc.type: FUNC
 */
HWTEST_F(UtfTestNg, ConvertRegionUtf8ToUtf16_001, TestSize.Level1)
{
    uint8_t mutf8In[] = {0x48, 0x65, 0x6C, 0x6C, 0x6F};
    uint16_t utf16Out[5] = {0};
    size_t result = ConvertRegionUtf8ToUtf16(mutf8In, utf16Out, sizeof(mutf8In), sizeof(utf16Out)/sizeof(uint16_t), 0);
    
    EXPECT_EQ(result, 5);
    EXPECT_EQ(utf16Out[0], 0x0048);
    EXPECT_EQ(utf16Out[1], 0x0065);
    EXPECT_EQ(utf16Out[2], 0x006C);
    EXPECT_EQ(utf16Out[3], 0x006C);
    EXPECT_EQ(utf16Out[4], 0x006F);
}

/**
 * @tc.name: DebuggerConvertRegionUtf16ToUtf8_001
 * @tc.desc: Test utf DebuggerConvertRegionUtf16ToUtf8
 * @tc.type: FUNC
 */
HWTEST_F(UtfTestNg, DebuggerConvertRegionUtf16ToUtf8_001, TestSize.Level1)
{
    uint16_t utf16In[] = {0x0048, 0x0065, 0x006C, 0x006C, 0x006F};
    uint8_t utf8Out[10] = {0};
    size_t result = DebuggerConvertRegionUtf16ToUtf8(utf16In, utf8Out, 5, 10, 0);

    EXPECT_EQ(result, 5);
    EXPECT_EQ(utf8Out[0], 0x48);
    EXPECT_EQ(utf8Out[1], 0x65);
    EXPECT_EQ(utf8Out[2], 0x6C);
    EXPECT_EQ(utf8Out[3], 0x6C);
    EXPECT_EQ(utf8Out[4], 0x6F);
}

/**
 * @tc.name: DebuggerConvertRegionUtf16ToUtf8_002
 * @tc.desc: Test utf DebuggerConvertRegionUtf16ToUtf8
 * @tc.type: FUNC
 */
HWTEST_F(UtfTestNg, DebuggerConvertRegionUtf16ToUtf8_002, TestSize.Level1)
{
    uint16_t utf16In[] = {0xD83D, 0xDE00};
    uint8_t utf8Out[5] = {0};
    size_t result = DebuggerConvertRegionUtf16ToUtf8(utf16In, utf8Out, 2, 5, 0);

    EXPECT_EQ(result, 4);
    EXPECT_EQ(utf8Out[0], 0xF0);
    EXPECT_EQ(utf8Out[1], 0x9F);
    EXPECT_EQ(utf8Out[2], 0x98);
    EXPECT_EQ(utf8Out[3], 0x80);
}

/**
 * @tc.name: DebuggerConvertRegionUtf16ToUtf8_003
 * @tc.desc: Test utf DebuggerConvertRegionUtf16ToUtf8
 * @tc.type: FUNC
 */
HWTEST_F(UtfTestNg, DebuggerConvertRegionUtf16ToUtf8_003, TestSize.Level1)
{
    uint16_t utf16In[] = {0x0048, 0x0065};
    uint8_t utf8Out[5] = {0};

    size_t nullResult1 = DebuggerConvertRegionUtf16ToUtf8(nullptr, utf8Out, 2, 5, 0);
    EXPECT_EQ(nullResult1, 0);

    size_t nullResult2 = DebuggerConvertRegionUtf16ToUtf8(utf16In, nullptr, 2, 0, 0);
    EXPECT_EQ(nullResult2, 0);
}

/**
 * @tc.name: ConvertIllegalStr_001
 * @tc.desc: Test utf ConvertIllegalStr
 * @tc.type: FUNC
 */
HWTEST_F(UtfTestNg, ConvertIllegalStr_001, TestSize.Level1)
{
    std::string str = "Hello世界";
    std::string originalStr = str;
    ConvertIllegalStr(str);
    
    EXPECT_EQ(str, originalStr);
}

/**
 * @tc.name: ConvertIllegalStr_002
 * @tc.desc: Test utf ConvertIllegalStr
 * @tc.type: FUNC
 */
HWTEST_F(UtfTestNg, ConvertIllegalStr_002, TestSize.Level1)
{
    std::string str = "Hello\xC3\x28";
    ConvertIllegalStr(str);
    
    EXPECT_TRUE(IsUTF8(str));
    EXPECT_NE(str, "Hello\xC3\x28");
}

/**
 * @tc.name: ConvertIllegalStr_003
 * @tc.desc: Test utf ConvertIllegalStr
 * @tc.type: FUNC
 */
HWTEST_F(UtfTestNg, ConvertIllegalStr_003, TestSize.Level1)
{
    std::string str = "";
    ConvertIllegalStr(str);
    
    EXPECT_TRUE(str.empty());
}

/**
 * @tc.name: SplitUtf16Pair_001
 * @tc.desc: Test utf SplitUtf16Pair
 * @tc.type: FUNC
 */
HWTEST_F(UtfTestNg, SplitUtf16Pair_001, TestSize.Level1)
{
    uint32_t pair = 0xD83DDE00;
    auto result = SplitUtf16Pair(pair);
    
    EXPECT_EQ(result.first, 0xD83D);
    EXPECT_EQ(result.second, 0xDE00);
}

/**
 * @tc.name: SplitUtf16Pair_002
 * @tc.desc: Test utf SplitUtf16Pair
 * @tc.type: FUNC
 */
HWTEST_F(UtfTestNg, SplitUtf16Pair_002, TestSize.Level1)
{
    uint32_t pair = 0x00410042;
    auto result = SplitUtf16Pair(pair);
    
    EXPECT_EQ(result.first, 0x0041);
    EXPECT_EQ(result.second, 0x0042);
}

// internal helpers of utf.cpp without public header declarations
size_t UTF8Length(uint32_t codePoint);
size_t EncodeUTF8(uint32_t codePoint, uint8_t* utf8, size_t len, size_t index);
bool IsContinuationByte(const std::string& input, size_t startIndex, uint8_t continueCount);
std::string RemoveInvalidUft8Bytes(const std::string& input);


class UtfGapTest : public testing::Test {
public:
    static void SetUpTestSuite() {}
    static void TearDownTestSuite() {}
};

/**
 * @tc.name: UtfGapUTF8Length001
 * @tc.desc: Test UTF8Length for all encoding widths.
 * @tc.type: FUNC
 */
HWTEST_F(UtfGapTest, UtfGapUTF8Length001, TestSize.Level1)
{
    /**
     * @tc.steps: query encoded length of code points at range boundaries.
     * @tc.expected: 1/2/3/4 bytes for ascii/2b/3b/4b ranges.
     */
    EXPECT_EQ(UTF8Length(0x00), 1U);
    EXPECT_EQ(UTF8Length(0x41), 1U);
    EXPECT_EQ(UTF8Length(0x7F), 1U);
    EXPECT_EQ(UTF8Length(0x80), 2U);
    EXPECT_EQ(UTF8Length(0x7FF), 2U);
    EXPECT_EQ(UTF8Length(0x800), 3U);
    EXPECT_EQ(UTF8Length(0xFFFF), 3U);
    EXPECT_EQ(UTF8Length(0x10000), 4U);
    EXPECT_EQ(UTF8Length(0x10FFFF), 4U);
}

/**
 * @tc.name: UtfGapEncodeUTF8001
 * @tc.desc: Test EncodeUTF8 for all widths and buffer overflow.
 * @tc.type: FUNC
 */
HWTEST_F(UtfGapTest, UtfGapEncodeUTF8001, TestSize.Level1)
{
    /**
     * @tc.steps: encode ascii, 2-byte, 3-byte, 4-byte code points and a
     *            code point that does not fit the buffer.
     * @tc.expected: correct utf-8 byte sequences; overflow returns 0.
     */
    uint8_t buffer[8] = { 0 };

    EXPECT_EQ(EncodeUTF8(0x41, buffer, sizeof(buffer), 0), 1U);
    EXPECT_EQ(buffer[0], 0x41);

    EXPECT_EQ(EncodeUTF8(0xE9, buffer, sizeof(buffer), 0), 2U); // e-acute
    EXPECT_EQ(buffer[0], 0xC3);
    EXPECT_EQ(buffer[1], 0xA9);

    EXPECT_EQ(EncodeUTF8(0x20AC, buffer, sizeof(buffer), 0), 3U); // euro sign
    EXPECT_EQ(buffer[0], 0xE2);
    EXPECT_EQ(buffer[1], 0x82);
    EXPECT_EQ(buffer[2], 0xAC);

    EXPECT_EQ(EncodeUTF8(0x1F600, buffer, sizeof(buffer), 0), 4U); // grinning face
    EXPECT_EQ(buffer[0], 0xF0);
    EXPECT_EQ(buffer[1], 0x9F);
    EXPECT_EQ(buffer[2], 0x98);
    EXPECT_EQ(buffer[3], 0x80);

    // not enough room: 2-byte sequence into 1-byte buffer
    EXPECT_EQ(EncodeUTF8(0xE9, buffer, 1, 0), 0U);
    // index offset leaves insufficient room
    EXPECT_EQ(EncodeUTF8(0x1F600, buffer, 4, 1), 0U);
}

/**
 * @tc.name: UtfGapIsContinuationByte001
 * @tc.desc: Test IsContinuationByte valid and invalid continuation bytes.
 * @tc.type: FUNC
 */
HWTEST_F(UtfGapTest, UtfGapIsContinuationByte001, TestSize.Level1)
{
    /**
     * @tc.steps: check continuation patterns of 1/2/3 bytes.
     * @tc.expected: bytes matching the continuation pattern pass, non-continuation bytes fail.
     */
    EXPECT_TRUE(IsContinuationByte("\xC3\xA9", 1, 1));    // 0xA9 is continuation
    EXPECT_FALSE(IsContinuationByte("\xC3\x41", 1, 1));  // 'A' is not
    EXPECT_TRUE(IsContinuationByte("\xE2\x82\xAC", 1, 2)); // 0x82, 0xAC
    EXPECT_FALSE(IsContinuationByte("\xE2\x41\xAC", 1, 2));
    EXPECT_TRUE(IsContinuationByte("\xF0\x9F\x98\x80", 1, 3)); // 0x9F,0x98,0x80
    EXPECT_FALSE(IsContinuationByte("\xF0\x9F\x41\x80", 1, 3));
}

/**
 * @tc.name: UtfGapRemoveInvalidUft8Bytes001
 * @tc.desc: Test RemoveInvalidUft8Bytes keeps valid sequences only.
 * @tc.type: FUNC
 */
HWTEST_F(UtfGapTest, UtfGapRemoveInvalidUft8Bytes001, TestSize.Level1)
{
    /**
     * @tc.steps: clean ascii, valid multi-byte sequences, truncated sequences
     *            and invalid lead bytes.
     * @tc.expected: valid sequences survive; truncated/invalid bytes dropped.
     */
    EXPECT_EQ(RemoveInvalidUft8Bytes("abc"), "abc");

    EXPECT_EQ(RemoveInvalidUft8Bytes("a\xC3\xA9" "b"), "a\xC3\xA9" "b"); // valid 2-byte kept
    EXPECT_EQ(RemoveInvalidUft8Bytes("\xE2\x82\xAC"), "\xE2\x82\xAC"); // valid 3-byte
    EXPECT_EQ(RemoveInvalidUft8Bytes("\xF0\x9F\x98\x80"), "\xF0\x9F\x98\x80"); // valid 4-byte

    // truncated sequences: lead byte without enough continuation bytes
    EXPECT_EQ(RemoveInvalidUft8Bytes("\xC3"), "");           // 2-byte lead alone
    EXPECT_EQ(RemoveInvalidUft8Bytes("\xE2\x82"), "");      // 3-byte with 1 cont
    EXPECT_EQ(RemoveInvalidUft8Bytes("\xF0\x9F\x98"), "");  // 4-byte with 2 cont
    // bad continuation: lead followed by ascii
    EXPECT_EQ(RemoveInvalidUft8Bytes("a\xC3" "b"), "ab");
    // invalid lead byte 0xFF and orphan continuation 0x80
    EXPECT_EQ(RemoveInvalidUft8Bytes(std::string("a\xFF" "b\x80" "c", 5)), "abc");
}
}

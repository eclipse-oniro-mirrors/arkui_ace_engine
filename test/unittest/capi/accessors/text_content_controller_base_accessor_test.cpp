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
#include "core/interfaces/native/implementation/text_content_controller_base_peer.h"
#include "core/interfaces/native/utility/converter.h"
#include "core/interfaces/native/utility/reverse_converter.h"

namespace OHOS::Ace::NG {

using namespace testing;
using namespace testing::ext;
using namespace Converter;

namespace {
constexpr int32_t LINES_NUM = 1;
class MockTextContentControllerBase : public TextFieldControllerBase {
public:
    MockTextContentControllerBase() = default;
    ~MockTextContentControllerBase() override = default;
    MOCK_METHOD(int32_t, GetTextContentLinesNum, (), (override));
    MOCK_METHOD(Rect, GetTextContentRect, (), (override));
    MOCK_METHOD(NG::OffsetF, GetCaretPosition, (), (override));
    MOCK_METHOD(void, DeleteText, (int32_t, int32_t), (override));
    MOCK_METHOD(SelectionInfo, GetSelection, (), (override));
    MOCK_METHOD(int32_t, AddText, (std::u16string, int32_t), (override));
    MOCK_METHOD(std::u16string, GetText, (), (override));
    MOCK_METHOD(void, ClearPreviewText, (), (override));
};
} // namespace

class TextContentControllerBaseAccessorTest :
    public AccessorTestBase<GENERATED_ArkUITextContentControllerBaseAccessor,
    &GENERATED_ArkUIAccessors::getTextContentControllerBaseAccessor, TextContentControllerBasePeer> {
public:
    void SetUp(void) override
    {
        AccessorTestBase::SetUp();
        mockTextContentControllerBase_ = new MockTextContentControllerBase();
        mockTextContentControllerBaseKeeper_ = AceType::Claim(mockTextContentControllerBase_);
        ASSERT_NE(mockTextContentControllerBaseKeeper_, nullptr);
        ASSERT_NE(peer_, nullptr);
        peer_->controller_ = mockTextContentControllerBaseKeeper_;
        ASSERT_NE(mockTextContentControllerBase_, nullptr);
    }

    void TearDown() override
    {
        AccessorTestBase::TearDown();
        mockTextContentControllerBaseKeeper_ = nullptr;
        mockTextContentControllerBase_ = nullptr;
    }

    MockTextContentControllerBase *mockTextContentControllerBase_ = nullptr;
    RefPtr<MockTextContentControllerBase> mockTextContentControllerBaseKeeper_ = nullptr;
};

/**
 * @tc.name: GetTextContentLinesNumTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextContentControllerBaseAccessorTest, GetTextContentLinesNumTest, TestSize.Level1)
{
    ASSERT_NE(accessor_->getTextContentLineCount, nullptr);
    EXPECT_CALL(*mockTextContentControllerBase_, GetTextContentLinesNum()).Times(1).WillOnce(Return(LINES_NUM));
    auto checkValue = Converter::Convert<int32_t>(accessor_->getTextContentLineCount(peer_));
    EXPECT_EQ(checkValue, LINES_NUM);
}

/**
 * @tc.name: GetCaretOffsetTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextContentControllerBaseAccessorTest, GetCaretOffsetTest, TestSize.Level1)
{
    ASSERT_NE(accessor_->getCaretOffset, nullptr);
    EXPECT_CALL(*mockTextContentControllerBase_, GetCaretPosition()).Times(1).WillOnce(Return(NG::OffsetF(0, 0)));
    Ark_CaretOffset offset = accessor_->getCaretOffset(peer_);
    auto x = Converter::Convert<int32_t>(offset.x);
    auto y = Converter::Convert<int32_t>(offset.y);
    EXPECT_EQ(x, 0);
    EXPECT_EQ(y, 0);
}

/**
 * @tc.name: GetTextContentRectTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextContentControllerBaseAccessorTest, DISABLED_GetTextContentRectTest, TestSize.Level1)
{
    ASSERT_NE(accessor_->getTextContentRect, nullptr);
    EXPECT_CALL(*mockTextContentControllerBase_, GetTextContentRect()).Times(1).WillOnce(Return(Rect(0, 0, 1, 1)));
    auto checkValue = accessor_->getTextContentRect(peer_);
    EXPECT_EQ(&checkValue, nullptr); // fix after updating return value
}

/**
 * @tc.name: DeleteTextTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextContentControllerBaseAccessorTest, DeleteTextTest, TestSize.Level1)
{
    ASSERT_NE(accessor_->getSelection, nullptr);
    const auto defaultStart = -1;
    const auto defaultEnd = -1;
    const auto start = 1;
    const auto end = 2;

    EXPECT_CALL(*mockTextContentControllerBase_, DeleteText(defaultStart, defaultEnd)).Times(0);
    accessor_->deleteText(peer_, nullptr);

    Ark_TextRange range = {
        .start = Converter::ArkValue<Opt_Number>(Ark_Empty()),
        .end = Converter::ArkValue<Opt_Number>(Ark_Empty())
    };
    auto rangeOpt = Converter::ArkValue<Opt_TextRange>(range);
    EXPECT_CALL(*mockTextContentControllerBase_, DeleteText(defaultStart, defaultEnd)).Times(1);
    accessor_->deleteText(peer_, &rangeOpt);

    range = {
        .start = Converter::ArkValue<Opt_Number>(start),
        .end = Converter::ArkValue<Opt_Number>(end)
    };
    rangeOpt = Converter::ArkValue<Opt_TextRange>(range);
    EXPECT_CALL(*mockTextContentControllerBase_, DeleteText(start, end)).Times(1);
    accessor_->deleteText(peer_, &rangeOpt);
}

/**
 * @tc.name: GetSelectionTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextContentControllerBaseAccessorTest, GetSelectionTest, TestSize.Level1)
{
    ASSERT_NE(accessor_->getSelection, nullptr);
    const auto start = 1;
    const auto end = 2;
    SelectionInfo info;
    info.SetSelectionStart(start);
    info.SetSelectionEnd(end);
    EXPECT_CALL(*mockTextContentControllerBase_, GetSelection()).Times(1).WillOnce(Return(info));
    auto checkValue = Converter::Convert<TextRange>(accessor_->getSelection(peer_));
    EXPECT_EQ(checkValue.start, start);
    EXPECT_EQ(checkValue.end, end);
}

/**
 * @tc.name: AddTextTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextContentControllerBaseAccessorTest, AddTextTest, TestSize.Level1)
{
    ASSERT_NE(accessor_->addText, nullptr);
    const auto errorValue = 0;
    const std::string text = "Hello";
    const auto offset = 1;
    const auto defaultOffset = -1;
    auto arkOptions = Converter::ArkValue<Opt_TextContentControllerOptions>(Ark_Empty());

    EXPECT_CALL(*mockTextContentControllerBase_, AddText(UtfUtils::Str8ToStr16(text), offset)).Times(0);
    auto checkValue = Converter::Convert<int32_t>(accessor_->addText(peer_, nullptr, &arkOptions));
    EXPECT_EQ(checkValue, errorValue);

    auto arkText = Converter::ArkValue<Ark_String>(text);
    EXPECT_CALL(*mockTextContentControllerBase_,
        AddText(UtfUtils::Str8ToStr16(text), defaultOffset)).Times(1).WillOnce(Return(offset));
    checkValue = Converter::Convert<int32_t>(accessor_->addText(peer_, &arkText, nullptr));
    EXPECT_EQ(checkValue, offset);

    Ark_TextContentControllerOptions options {
        .offset = Converter::ArkValue<Opt_Number>(offset)
    };
    arkOptions = Converter::ArkValue<Opt_TextContentControllerOptions>(options);
    EXPECT_CALL(*mockTextContentControllerBase_,
        AddText(UtfUtils::Str8ToStr16(text), offset)).Times(1).WillOnce(Return(offset));
    checkValue = Converter::Convert<int32_t>(accessor_->addText(peer_, &arkText, &arkOptions));
    EXPECT_EQ(checkValue, offset);
}

/**
 * @tc.name: GetTextTestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextContentControllerBaseAccessorTest, GetTextTestValidValues, TestSize.Level1)
{
    TextRange range = {
        .start = 2,
        .end = 5
    };
    auto arkRange = Converter::ArkValue<Opt_TextRange>(range);
    EXPECT_CALL(*mockTextContentControllerBase_, GetText()).WillOnce(Return(u"Qwerty"));
    auto checkValue = Converter::Convert<std::u16string>(accessor_->getText(peer_, &arkRange));
    EXPECT_EQ(checkValue, u"ert");
}

/**
 * @tc.name: GetTextTestInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextContentControllerBaseAccessorTest, GetTextTestInvalidValues, TestSize.Level1)
{
    auto arkRange = Converter::ArkValue<Opt_TextRange>(Ark_Empty());
    EXPECT_CALL(*mockTextContentControllerBase_, GetText()).WillOnce(Return(u"Qwerty"));
    auto checkValue = Converter::Convert<std::u16string>(accessor_->getText(peer_, &arkRange));
    EXPECT_EQ(checkValue, u"Qwerty");
}

/**
 * @tc.name: ClearPreviewTextTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextContentControllerBaseAccessorTest, ClearPreviewTextTest, TestSize.Level1)
{
    EXPECT_CALL(*mockTextContentControllerBase_, ClearPreviewText()).Times(1);
    accessor_->clearPreviewText(peer_);
}

} // namespace OHOS::Ace::NG

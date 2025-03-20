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

#include <gmock/gmock.h>

#include "richeditor_accessor_test.h"

#include "core/components_ng/pattern/text/span/span_string.h"
#include "core/interfaces/native/implementation/rich_editor_controller_peer_impl.h"
#include "core/interfaces/native/implementation/styled_string_peer.h"
#include "core/interfaces/native/utility/converter.h"
#include "core/interfaces/native/utility/reverse_converter.h"

#include "accessor_test_base.h"
#include "accessor_test_utils.h"
#include "test/unittest/capi/utils/custom_node_builder_test_helper.h"

namespace OHOS::Ace {
bool operator==(const TextSpanOptions& lhs, const TextSpanOptions& rhs)
{
    return lhs.offset.value() == rhs.offset.value();
}
bool operator==(const ImageSpanOptions& lhs, const ImageSpanOptions& rhs)
{
    return lhs.offset.value() == rhs.offset.value();
}
bool operator==(const SymbolSpanOptions& lhs, const SymbolSpanOptions& rhs)
{
    return lhs.offset.value() == rhs.offset.value();
}
bool operator==(const SpanOptionBase& lhs, const SpanOptionBase& rhs)
{
    return lhs.offset.value() == rhs.offset.value();
}

bool operator==(const UpdateParagraphStyle& lhs, const UpdateParagraphStyle& rhs)
{
    return lhs.textAlign.value() == rhs.textAlign.value() &&
        lhs.wordBreak.value() == rhs.wordBreak.value() &&
        lhs.lineBreakStrategy.value() == rhs.lineBreakStrategy.value();
}

bool operator==(const RangeOptions& lhs, const RangeOptions& rhs)
{
    return lhs.start.value() == rhs.start.value() && lhs.end.value() == rhs.end.value();
}
} // OHOS::Ace

static constexpr int TEST_OFFSET = 5;
static constexpr int TEST_START = 1;
static constexpr int TEST_END = 3;
static const auto TEST_VALUE = "test value";
static const std::vector<std::string> FONT_FAMILIES = { "Arial" };
static constexpr double FONT_SIZE = 12.0;
static constexpr OHOS::Ace::FontWeight FONT_WEIGHT = OHOS::Ace::FontWeight::BOLD;
static constexpr OHOS::Ace::FontStyle FONT_STYLE = OHOS::Ace::FontStyle::ITALIC;
static const OHOS::Ace::Color FONT_COLOUR = OHOS::Ace::Color::RED;

namespace OHOS::Ace::NG {

using namespace testing;
using namespace testing::ext;

namespace Converter {
void AssignArkValue(Ark_RichEditorTextSpanOptions& dst, const OHOS::Ace::TextSpanOptions& src)
{
    dst.offset = Converter::ArkValue<Opt_Number>(src.offset);
}
void AssignArkValue(Ark_RichEditorImageSpanOptions& dst, const OHOS::Ace::ImageSpanOptions& src)
{
    dst.offset = Converter::ArkValue<Opt_Number>(src.offset);
}
void AssignArkValue(Ark_RichEditorSymbolSpanOptions& dst, const OHOS::Ace::SymbolSpanOptions& src)
{
    dst.offset = Converter::ArkValue<Opt_Number>(src.offset);
}
void AssignArkValue(Ark_RichEditorBuilderSpanOptions& dst, const OHOS::Ace::SpanOptionBase& src)
{
    dst.offset = Converter::ArkValue<Opt_Number>(src.offset);
}

void AssignArkValue(Ark_RichEditorUpdateTextSpanStyleOptions& dst, const TextSpanOptionsForUpdate& src,
    Converter::ConvContext *ctx)
{
    dst.start = Converter::ArkValue<Opt_Number>(src.start);
    dst.end = Converter::ArkValue<Opt_Number>(src.end);
    if (src.textStyle) {
        dst.textStyle = Converter::ArkValue<Ark_RichEditorTextStyle>(src.textStyle.value(), ctx);
    }
}
void AssignArkValue(Ark_Type_RichEditorController_updateSpanStyle_value& dst,
                    const TextSpanOptionsForUpdate& src, Converter::ConvContext *ctx)
{
    dst = Converter::ArkUnion<Ark_Type_RichEditorController_updateSpanStyle_value,
        Ark_RichEditorUpdateTextSpanStyleOptions>(src, ctx);
}

void AssignArkValue(Ark_DecorationStyleInterface& dst, const Converter::TextDecorationStruct& src,
                    Converter::ConvContext *ctx)
{
    dst.color = Converter::ArkUnion<Opt_ResourceColor, Ark_String>(src.color, ctx);
    dst.style = Converter::ArkValue<Opt_TextDecorationStyle>(src.style);
    dst.type = Converter::ArkValue<Ark_TextDecorationType>(src.textDecoration);
}

void AssignArkValue(Ark_RichEditorTextStyle& dst, const OHOS::Ace::TextStyle& style,
    Converter::ConvContext *ctx)
{
    dst.fontColor = Converter::ArkUnion<Opt_ResourceColor, Ark_String>(style.GetTextColor(), ctx);
    dst.fontSize = Converter::ArkUnion<Opt_Union_Length_Number, Ark_Length>(style.GetFontSize());
    dst.fontStyle = Converter::ArkValue<Opt_FontStyle>(style.GetFontStyle());
    dst.fontWeight = Converter::ArkUnion<
        Opt_Union_Number_FontWeight_String, Ark_FontWeight>(style.GetFontWeight());

    auto families = style.GetFontFamilies();
    if (!families.empty()) {
        dst.fontFamily = Converter::ArkUnion<Opt_ResourceStr, Ark_String>(families[0], ctx);
    }

    Converter::TextDecorationStruct decoration;
    decoration.color = style.GetTextDecorationColor();
    decoration.style = style.GetTextDecorationStyle();
    decoration.textDecoration = style.GetTextDecoration();
    dst.decoration = Converter::ArkValue<Opt_DecorationStyleInterface>(decoration, ctx);
}

void AssignArkValue(Ark_LineBreakStrategy& dst, const OHOS::Ace::LineBreakStrategy& src)
{
    switch (src) {
        case LineBreakStrategy::GREEDY: dst = ARK_LINE_BREAK_STRATEGY_GREEDY; break;
        case LineBreakStrategy::HIGH_QUALITY: dst = ARK_LINE_BREAK_STRATEGY_HIGH_QUALITY; break;
        case LineBreakStrategy::BALANCED: dst = ARK_LINE_BREAK_STRATEGY_BALANCED; break;
        default: LOGE("Unexpected enum value in LineBreakStrategy: %{public}d", src);
    }
}

void AssignArkValue(Ark_RichEditorParagraphStyle& dst, const UpdateParagraphStyle& style)
{
    dst.textAlign = Converter::ArkValue<Opt_TextAlign>(style.textAlign);
    dst.wordBreak = Converter::ArkValue<Opt_WordBreak>(style.wordBreak);
    dst.lineBreakStrategy = Converter::ArkValue<Opt_LineBreakStrategy>(style.lineBreakStrategy);
}

void AssignArkValue(Ark_RichEditorParagraphStyleOptions& dst, const Converter::UpdateParagraphStyleForUpdate& style)
{
    dst.start = Converter::ArkValue<Opt_Number>(style.start);
    dst.end = Converter::ArkValue<Opt_Number>(style.end);
    dst.style = Converter::ArkValue<Ark_RichEditorParagraphStyle>(style.style.value());
}

void AssignArkValue(Ark_RichEditorRange& dst, const RangeOptions& src)
{
    dst.start = Converter::ArkValue<Opt_Number>(src.start);
    dst.end = Converter::ArkValue<Opt_Number>(src.end);
}
} // Converter

namespace {
class MockRichEditorController : public RichEditorController {
public:
    MockRichEditorController() = default;
    ~MockRichEditorController() override = default;
    MOCK_METHOD(int32_t, AddImageSpan, (const ImageSpanOptions&));
    MOCK_METHOD(int32_t, AddTextSpan, (const TextSpanOptions&));
    MOCK_METHOD(int32_t, AddSymbolSpan, (const SymbolSpanOptions&));
    MOCK_METHOD(int32_t, AddPlaceholderSpan, (const RefPtr<UINode>&, const SpanOptionBase&));
    MOCK_METHOD(void, UpdateSpanStyle, (int32_t, int32_t, TextStyle, ImageSpanAttribute));
    MOCK_METHOD(void, UpdateParagraphStyle, (int32_t, int32_t, const struct UpdateParagraphStyle&));
    MOCK_METHOD(void, DeleteSpans, (const RangeOptions&));
    MOCK_METHOD(SelectionInfo, GetSpansInfo, (int32_t start, int32_t end));
    MOCK_METHOD(std::vector<ParagraphInfo>, GetParagraphsInfo, (int32_t start, int32_t end));
    MOCK_METHOD(SelectionInfo, GetSelectionSpansInfo, ());
    MOCK_METHOD(RefPtr<SpanStringBase>, ToStyledString, (int32_t, int32_t));
    MOCK_METHOD(SelectionInfo, FromStyledString, (RefPtr<SpanStringBase>));
};
} // namespace

class RichEditorControllerAccessorTest : public AccessorTestBase<GENERATED_ArkUIRichEditorControllerAccessor,
    &GENERATED_ArkUIAccessors::getRichEditorControllerAccessor, RichEditorControllerPeer> {
public:
    void SetUp(void) override
    {
        AccessorTestBase::SetUp();

        mockRichEditorController_ = new MockRichEditorController();
        mockRichEditorControllerKeeper_ = AceType::Claim(mockRichEditorController_);
        ASSERT_NE(mockRichEditorControllerKeeper_, nullptr);

        auto peerImpl = reinterpret_cast<GeneratedModifier::RichEditorControllerPeerImpl *>(peer_);
        ASSERT_NE(peerImpl, nullptr);
        peerImpl->AddTargetController(mockRichEditorControllerKeeper_);

        ASSERT_NE(mockRichEditorController_, nullptr);
    }

    void TearDown() override
    {
        mockRichEditorControllerKeeper_ = nullptr;
        mockRichEditorController_ = nullptr;
    }

    MockRichEditorController *mockRichEditorController_ = nullptr;
    RefPtr<MockRichEditorController> mockRichEditorControllerKeeper_ = nullptr;
    Ark_VMContext vmContext_ = nullptr;
};

/**
 * @tc.name: addImageSpanTest
 * @tc.desc: Check the functionality of addImageSpan
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorControllerAccessorTest, addImageSpanTest, TestSize.Level1)
{
    ASSERT_NE(accessor_->addImageSpan, nullptr);

    std::string urlString = "https://www.example.com/xxx.png";
    std::string resName = "app.string.image_url";
    AddResource(resName, urlString);
    int resID = 2345;
    const auto RES_ID = IntResourceId{resID, Converter::ResourceType::STRING};
    Ark_ResourceStr arkResourceStr = CreateResourceUnion<Ark_ResourceStr>(RES_ID);

    ImageSpanOptions imageSpanOptions;
    imageSpanOptions.offset = TEST_OFFSET;
    auto value = Converter::ArkUnion<Ark_Union_PixelMap_ResourceStr, Ark_ResourceStr>(arkResourceStr);
    auto options = Converter::ArkValue<Opt_RichEditorImageSpanOptions>(imageSpanOptions);

    EXPECT_CALL(*mockRichEditorController_, AddImageSpan(imageSpanOptions)).Times(1);
    accessor_->addImageSpan(peer_, &value, &options);
}

/**
 * @tc.name: addTextSpanTest
 * @tc.desc: Check the functionality of addTextSpan
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorControllerAccessorTest, addTextSpanTest, TestSize.Level1)
{
    ASSERT_NE(accessor_->addTextSpan, nullptr);

    TextSpanOptions textSpanOptions;
    textSpanOptions.offset = TEST_OFFSET;
    textSpanOptions.value = TEST_VALUE;
    Ark_String value = Converter::ArkValue<Ark_String>(TEST_VALUE);
    auto options = Converter::ArkValue<Opt_RichEditorTextSpanOptions>(textSpanOptions);

    EXPECT_CALL(*mockRichEditorController_, AddTextSpan(textSpanOptions)).Times(1);
    accessor_->addTextSpan(peer_, &value, &options);
}

/**
 * @tc.name: addSymbolSpanTest
 * @tc.desc: Check the functionality of addSymbolSpan
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorControllerAccessorTest, addSymbolSpanTest, TestSize.Level1)
{
    ASSERT_NE(accessor_->addSymbolSpan, nullptr);

    SymbolSpanOptions symbolSpanOptions;
    symbolSpanOptions.offset = TEST_OFFSET;

    std::string text = "S";
    std::string resName = "app.string.symbol";
    Ark_Resource value = CreateResource(resName.c_str(), Converter::ResourceType::INTEGER);
    AddResource(resName, text);

    auto options = Converter::ArkValue<Opt_RichEditorSymbolSpanOptions>(symbolSpanOptions);

    EXPECT_CALL(*mockRichEditorController_, AddSymbolSpan(symbolSpanOptions)).Times(1);
    accessor_->addSymbolSpan(peer_, &value, &options);
}

/**
 * @tc.name: addBuilderSpanTest
 * @tc.desc: Check the functionality of addBuilderSpan
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorControllerAccessorTest, addBuilderSpanTest, TestSize.Level1)
{
    ASSERT_NE(accessor_->addBuilderSpan, nullptr);

    SpanOptionBase spanOptions = {.offset = TEST_OFFSET};
    auto options = Converter::ArkValue<Opt_RichEditorBuilderSpanOptions>(spanOptions);
    auto nodeId = 123;
    // Creating custom node
    auto customSpanNode = SpanNode::GetOrCreateSpanNode(nodeId);
    auto customUI_Node = static_cast<RefPtr<UINode>>(customSpanNode);
    auto customNodeHandle = reinterpret_cast<Ark_NodeHandle>(Referenced::RawPtr(customSpanNode));
    // Creating custom node builder with helper and customNodeHandle
    int callsCount = 0;
    CustomNodeBuilderTestHelper<RichEditorControllerAccessorTest> builderHelper(this, nullptr, customNodeHandle);
    const auto builder = builderHelper.GetBuilder();
    // Testing custom node builder
    EXPECT_CALL(*mockRichEditorController_, AddPlaceholderSpan(customUI_Node, spanOptions)).Times(1);
    accessor_->addBuilderSpan(peer_, &builder, &options);
    EXPECT_EQ(builderHelper.GetCallsCount(), ++callsCount);
}

/**
 * @tc.name: updateSpanStyleTest
 * @tc.desc: Check the functionality of updateSpanStyle
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorControllerAccessorTest, updateSpanStyleTest, TestSize.Level1)
{
    ASSERT_NE(accessor_->updateSpanStyle, nullptr);
    Converter::TextSpanOptionsForUpdate updateOptions;
    updateOptions.start = TEST_START;
    updateOptions.end = TEST_END;
    updateOptions.textStyle = TextStyle(FONT_FAMILIES, FONT_SIZE, FONT_WEIGHT, FONT_STYLE, FONT_COLOUR);
    Converter::ConvContext ctx;
    auto value = Converter::ArkValue<Ark_Type_RichEditorController_updateSpanStyle_value>(updateOptions, &ctx);

    EXPECT_CALL(*mockRichEditorController_, UpdateSpanStyle(
        updateOptions.start, updateOptions.end, updateOptions.textStyle.value(), updateOptions.imageSpanAttribute
    )).Times(1);
    accessor_->updateSpanStyle(peer_, &value);
}

/**
 * @tc.name: updateParagraphStyleTest
 * @tc.desc: Check the functionality of updateParagraphStyle
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorControllerAccessorTest, updateParagraphStyleTest, TestSize.Level1)
{
    ASSERT_NE(accessor_->updateParagraphStyle, nullptr);
    Converter::UpdateParagraphStyleForUpdate updateOptions;
    updateOptions.start = TEST_START;
    updateOptions.end = TEST_END;
    updateOptions.style = UpdateParagraphStyle();
    updateOptions.style->textAlign = TextAlign::END;
    updateOptions.style->wordBreak = WordBreak::BREAK_ALL;
    updateOptions.style->lineBreakStrategy = LineBreakStrategy::HIGH_QUALITY;
    auto value = Converter::ArkValue<Ark_RichEditorParagraphStyleOptions>(updateOptions);

    EXPECT_CALL(*mockRichEditorController_, UpdateParagraphStyle(
        updateOptions.start, updateOptions.end, updateOptions.style.value()
    )).Times(1);
    accessor_->updateParagraphStyle(peer_, &value);
}

/**
 * @tc.name: deleteSpansTest
 * @tc.desc: Check the functionality of deleteSpans
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorControllerAccessorTest, deleteSpansTest, TestSize.Level1)
{
    ASSERT_NE(accessor_->deleteSpans, nullptr);
    RangeOptions options;
    options.start = TEST_START;
    options.end = TEST_END;

    auto value = Converter::ArkValue<Opt_RichEditorRange>(options);
    EXPECT_CALL(*mockRichEditorController_, DeleteSpans(options)).Times(1);
    accessor_->deleteSpans(peer_, &value);
}

/**
 * @tc.name: getSpansTest
 * @tc.desc: Check the functionality of getSpans
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorControllerAccessorTest, getSpansTest, TestSize.Level1)
{
    ASSERT_NE(accessor_->getSpans, nullptr);
    RangeOptions options;
    options.start = TEST_START;
    options.end = TEST_END;

    auto value = Converter::ArkValue<Opt_RichEditorRange>(options);
    EXPECT_CALL(*mockRichEditorController_, GetSpansInfo(TEST_START, TEST_END)).Times(1);
    accessor_->getSpans(peer_, &value);
}

/**
 * @tc.name: getParagraphsTest
 * @tc.desc: Check the functionality of getParagraphs
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorControllerAccessorTest, getParagraphsTest, TestSize.Level1)
{
    ASSERT_NE(accessor_->getParagraphs, nullptr);
    RangeOptions options;
    options.start = TEST_START;
    options.end = TEST_END;

    auto value = Converter::ArkValue<Opt_RichEditorRange>(options);
    EXPECT_CALL(*mockRichEditorController_, GetParagraphsInfo(TEST_START, TEST_END)).Times(1);
    accessor_->getParagraphs(peer_, &value);
}

/**
 * @tc.name: getSelectionTest
 * @tc.desc: Check the functionality of getSelection
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorControllerAccessorTest, getSelectionTest, TestSize.Level1)
{
    ASSERT_NE(accessor_->getSelection, nullptr);
    EXPECT_CALL(*mockRichEditorController_, GetSelectionSpansInfo()).Times(1);
    accessor_->getSelection(peer_);
}

/**
 * @tc.name: fromStyledStringTest
 * @tc.desc: Check the functionality of fromStyledString
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorControllerAccessorTest, fromStyledStringTest, TestSize.Level1)
{
    ASSERT_NE(accessor_->fromStyledString, nullptr);
    RefPtr<SpanStringBase> updateSpanStyle = Referenced::MakeRefPtr<SpanString>(TEST_VALUE);
    Ark_StyledString inValue = StyledStringPeer::Create(updateSpanStyle);
    EXPECT_CALL(*mockRichEditorController_, FromStyledString(updateSpanStyle)).Times(1);
    accessor_->fromStyledString(vmContext_, peer_, inValue);
    StyledStringPeer::Destroy(inValue);
}

/**
 * @tc.name: toStyledStringTest
 * @tc.desc: Check the functionality of toStyledString
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorControllerAccessorTest, toStyledStringTest, TestSize.Level1)
{
    ASSERT_NE(accessor_->toStyledString, nullptr);
    RangeOptions options;
    options.start = TEST_START;
    options.end = TEST_END;
    auto value = Converter::ArkValue<Ark_RichEditorRange>(options);
    EXPECT_CALL(*mockRichEditorController_, ToStyledString(TEST_START, TEST_END)).Times(1);
    accessor_->toStyledString(vmContext_, peer_, &value);
}
} // namespace OHOS::Ace::NG

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
#include "accessor_test_base.h"
#include "core/interfaces/native/implementation/styled_string_peer.h"
#include "core/interfaces/native/implementation/mutable_styled_string_peer.h"
#include "core/components_ng/pattern/text/span/span_string.h"
#include "core/interfaces/native/utility/converter.h"
#include "core/interfaces/native/utility/reverse_converter.h"
#include "core/interfaces/native/implementation/background_color_style_peer.h"
#include "core/interfaces/native/implementation/baseline_offset_style_peer.h"
#include "core/interfaces/native/implementation/decoration_style_peer.h"
#include "core/interfaces/native/implementation/gesture_style_peer.h"
#include "core/interfaces/native/implementation/length_metrics_peer.h"
#include "core/interfaces/native/implementation/letter_spacing_style_peer.h"
#include "core/interfaces/native/implementation/line_height_style_peer.h"
#include "core/interfaces/native/implementation/paragraph_style_peer.h"
#include "core/interfaces/native/implementation/pixel_map_peer.h"
#include "core/interfaces/native/implementation/text_shadow_style_peer.h"
#include "core/interfaces/native/implementation/text_style_styled_string_peer.h"
#include "core/interfaces/native/implementation/url_style_peer.h"
#include "adapter/ohos/capability/html/span_to_html.h"
#include "adapter/ohos/capability/html/html_to_span.h"
#include "test/mock/base/mock_task_executor.h"
#include "test/mock/core/pipeline/mock_pipeline_context.h"
#include "core/common/ace_engine.h"

#include "gmock/gmock.h"

namespace OHOS::Ace::NG {

constexpr int TEST_LENGTH = 2;
constexpr int TEST_START_STR = 1;
constexpr int TEST_START_DCRN = TEST_START_STR + TEST_LENGTH + 1;
constexpr int TEST_START_BSL = TEST_START_DCRN + TEST_LENGTH + 1;
constexpr int TEST_START_LSP = TEST_START_BSL + TEST_LENGTH + 1;
constexpr int TEST_START_TSH = TEST_START_LSP + TEST_LENGTH + 1;
constexpr int TEST_START_LNHT = TEST_START_TSH + TEST_LENGTH + 1;
constexpr int TEST_START_BGCL = TEST_START_LNHT + TEST_LENGTH + 1;
constexpr int TEST_START_URL = TEST_START_BGCL + TEST_LENGTH + 1;
constexpr int TEST_START_PSST = TEST_START_URL + TEST_LENGTH + 1;
constexpr int TEST_START_PSPM = TEST_START_PSST + TEST_LENGTH + 1;
constexpr auto STRING_TEST_VALUE = "This is a test string for styled text, and more text to test it out.\n";


PixelMapPeer* CreatePixelMap()
{
    static PixelMapPeer pixelMapPeer;
    static std::string src = "test";
    auto voidChar = src.data();
    void* voidPtr = static_cast<void*>(voidChar);
    pixelMapPeer.pixelMap = PixelMap::CreatePixelMap(voidPtr);
    return &pixelMapPeer;
}
const Ark_PixelMap TEST_PIXELMAP = CreatePixelMap();
const std::string TEST_SIZEOPTIONS = "1.00px";
const Ark_SizeOptions TEST_ARK_SIZEOPTIONS {
    .width = Converter::ArkValue<Opt_Length>(TEST_SIZEOPTIONS),
    .height = Converter::ArkValue<Opt_Length>(TEST_SIZEOPTIONS)
};
const std::tuple<VerticalAlign, Ark_ImageSpanAlignment> TEST_VERTICALALIGN = {
    VerticalAlign::CENTER, ARK_IMAGE_SPAN_ALIGNMENT_CENTER};
const std::tuple<ImageFit, Ark_ImageFit> TEST_IMAGEFIT = { ImageFit::FILL, ARK_IMAGE_FIT_FILL };
const Ark_LengthMetrics TEST_LENGTHMETRICS = Converter::ArkValue<Ark_LengthMetrics>(123.0_vp);
const std::string TEST_LENGTHMETRICS_STR = "[123.00vp,123.00vp,123.00vp,123.00vp]";
const std::string TEST_LENGTHMETRICS_BR_STR =
    "radiusTopLeft: [123.00vp]radiusTopRight: [123.00vp]radiusBottomLeft: [123.00vp]radiusBottomRight: [123.00vp]";

const Ark_ImageAttachmentLayoutStyle TEST_IMAGELAYOUTSTYLE {
    .margin = Converter::ArkUnion<Opt_Union_LengthMetrics_Margin, Ark_LengthMetrics>(TEST_LENGTHMETRICS),
    .padding = Converter::ArkUnion<Opt_Union_LengthMetrics_Padding, Ark_LengthMetrics>(TEST_LENGTHMETRICS),
    .borderRadius = Converter::ArkUnion<Opt_Union_LengthMetrics_BorderRadiuses, Ark_LengthMetrics>(TEST_LENGTHMETRICS)
};

const Ark_ImageAttachmentInterface IMAGEATTACHMENT_TEST_VALUE {
    .value = TEST_PIXELMAP,
    .size = Converter::ArkValue<Opt_SizeOptions>(TEST_ARK_SIZEOPTIONS),
    .verticalAlign = Converter::ArkValue<Opt_ImageSpanAlignment>(std::get<1>(TEST_VERTICALALIGN)),
    .objectFit = Converter::ArkValue<Opt_ImageFit>(std::get<1>(TEST_IMAGEFIT)),
    .layoutStyle = Converter::ArkValue<Opt_ImageAttachmentLayoutStyle>(TEST_IMAGELAYOUTSTYLE)
};

const std::tuple<std::string, Dimension> TEST_FONT_SIZE = { "16.00vp", 16.0_vp };
const std::tuple<Ace::FontWeight, int32_t> TEST_FONT_WEIGHT = { FontWeight::W900, 900 };
const std::vector<std::string> TEST_FONT_FAMILIES = { "Arial" };
const std::tuple<std::string, Ark_Color> TEST_FONT_COLOR = {
    "#FFFF0000", Converter::ArkValue<Ark_Color>(ARK_COLOR_RED) };
const Ace::FontStyle TEST_FONT_STYLE = Ace::FontStyle::ITALIC;
const std::tuple<std::string, Ark_Color> TEST_DCRN_COLOR = {
    "#FFFFFF00", Converter::ArkValue<Ark_Color>(ARK_COLOR_YELLOW) };
const Ace::TextDecoration TEST_DCRN_TYPE = Ace::TextDecoration::UNDERLINE;
const Ace::TextDecorationStyle TEST_DCRN_STYLE = TextDecorationStyle::SOLID;
const std::tuple<std::string, Dimension> TEST_BASELINE_OFFSET = { "10.00vp", 10.0_vp };
const std::tuple<std::string, Dimension> TEST_LETTER_SPACING = { "12.00vp", 12.0_vp };
const std::tuple<std::string, Dimension> TEST_LINE_HEIGHT = { "14.00vp", 14.0_vp };
const int TEST_TEXT_SHADOW_RADIUS = 5;
const int TEST_TEXT_SHADOW_OFFSET = 3;
const std::tuple<ShadowType, Ark_ShadowType> TEST_TEXT_SHADOW_TYPE = {
    Ace::ShadowType::COLOR, ARK_SHADOW_TYPE_COLOR };
const std::tuple<std::string, Ark_Color> TEST_TEXT_SHADOW_COLOR = {
    "#FFFF0000", Converter::ArkValue<Ark_Color>(ARK_COLOR_RED) };
const std::tuple<std::string, Ark_Color> TEST_BGCL_COLOR = {
    "#FFFFFF00", Converter::ArkValue<Ark_Color>(ARK_COLOR_YELLOW) };
const std::tuple<std::string, float> TEST_BGCL_RADIUS = { "10.00vp", 10.f };
const std::string TEST_URL = "https://www.test.test";
const std::tuple<Ace::TextAlign, Ark_TextAlign> TEST_PSST_TEXT_ALIGN = {
    Ace::TextAlign::CENTER, ARK_TEXT_ALIGN_CENTER};
const std::tuple<std::string, Dimension> TEST_PSST_TEXT_INDEN = { "5.00vp", 5.0_vp };
const int TEST_PSST_MAX_LINES = 8;
const std::tuple<Ace::TextOverflow, Ark_TextOverflow> TEST_PSST_OVERFLOW = {
    Ace::TextOverflow::ELLIPSIS, ARK_TEXT_OVERFLOW_ELLIPSIS };
const std::tuple<Ace::WordBreak, Ark_WordBreak> TEST_PSST_WORD_BREAK = {
    Ace::WordBreak::BREAK_ALL, ARK_WORD_BREAK_BREAK_ALL };
const std::tuple<std::string, Dimension> TEST_PSST_LEADING_MARGIN = { "width: 2.00vp height: 2.00vp", 2.0_vp };
const std::tuple<std::string, Ark_Tuple_Dimension_Dimension> TEST_TUPLE_DIMENSION_DIMENSION = {
    "width: 0.10fp height: 10.00vp",
    Converter::ArkValue<Ark_Tuple_Dimension_Dimension>(std::pair<const Dimension, Dimension> {
        0.1_fp, 10.0_vp
    })};
const Ark_LeadingMarginPlaceholder TEST_PSPM_LEADING_MARGIN {
    .pixelMap = TEST_PIXELMAP,
    .size = std::get<1>(TEST_TUPLE_DIMENSION_DIMENSION)
};
const int EXPECTED_NODE_ID = 777;
const int TEST_CONTAINER_ID = 888;

const std::vector<std::pair<int, Ace::SpanType>> SPAN_TYPE_TEST_VALUES = {
    { TEST_START_STR, Ace::SpanType::Font },
    { TEST_START_DCRN, Ace::SpanType::Decoration },
    { TEST_START_BSL, Ace::SpanType::BaselineOffset },
    { TEST_START_LSP, Ace::SpanType::LetterSpacing },
    { TEST_START_TSH, Ace::SpanType::TextShadow },
    { TEST_START_LNHT, Ace::SpanType::LineHeight },
    { TEST_START_BGCL, Ace::SpanType::BackgroundColor },
    { TEST_START_URL, Ace::SpanType::Url },
    { TEST_START_PSST, Ace::SpanType::ParagraphStyle },
    { TEST_START_PSPM, Ace::SpanType::ParagraphStyle },
    };

namespace GeneratedModifier {
    const GENERATED_ArkUIImageAttachmentAccessor* GetImageAttachmentAccessor();
    const GENERATED_ArkUIStyledStringAccessor* GetStyledStringAccessor();
    const GENERATED_ArkUITextStyle_styled_stringAccessor* GetTextStyle_styled_stringAccessor();
    const GENERATED_ArkUIDecorationStyleAccessor* GetDecorationStyleAccessor();
    const GENERATED_ArkUIBaselineOffsetStyleAccessor* GetBaselineOffsetStyleAccessor();
    const GENERATED_ArkUILetterSpacingStyleAccessor* GetLetterSpacingStyleAccessor();
    const GENERATED_ArkUITextShadowStyleAccessor* GetTextShadowStyleAccessor();
    const GENERATED_ArkUILineHeightStyleAccessor* GetLineHeightStyleAccessor();
    const GENERATED_ArkUIBackgroundColorStyleAccessor* GetBackgroundColorStyleAccessor();
    const GENERATED_ArkUIUrlStyleAccessor* GetUrlStyleAccessor();
    const GENERATED_ArkUIParagraphStyleAccessor* GetParagraphStyleAccessor();
}
struct StyleOptionsKeeper {
    std::vector<Ark_StyleOptions> testArrayStyles = {};

    const std::vector<Ark_StyleOptions> GetOptions()
    {
        testArrayStyles.clear();
        for (const auto& span : SPAN_TYPE_TEST_VALUES) {
            auto& item = testArrayStyles.emplace_back(
                Ark_StyleOptions {
                    .start = Converter::ArkValue<Opt_Number>(span.first),
                    .length = Converter::ArkValue<Opt_Number>(TEST_LENGTH),
                    .styledKey = Converter::ArkValue<Ark_StyledStringKey>(span.second)
            });

            if (span.second == Ace::SpanType::Font) {
                FillTextStyle(item.styledValue);
            } else if (span.second == Ace::SpanType::Decoration) {
                FillDecorationStyle(item.styledValue);
            } else if (span.second == Ace::SpanType::BaselineOffset) {
                FillBaselineOffsetStyle(item.styledValue);
            } else if (span.second == Ace::SpanType::LetterSpacing) {
                FillLetterSpacing(item.styledValue);
            } else if (span.second == Ace::SpanType::TextShadow) {
                FillTextShadow(item.styledValue);
            } else if (span.second == Ace::SpanType::LineHeight) {
                FillLineHeight(item.styledValue);
            } else if (span.second == Ace::SpanType::BackgroundColor) {
                FillBackgroundColor(item.styledValue);
            } else if (span.second == Ace::SpanType::Url) {
                FillUrl(item.styledValue);
            } else if (span.second == Ace::SpanType::ParagraphStyle && span.first == TEST_START_PSST) {
                FillParagraphStyle(item.styledValue);
            } else if (span.second == Ace::SpanType::ParagraphStyle && span.first == TEST_START_PSPM) {
                FillParagraphStylePM(item.styledValue);
            }
        }

        return testArrayStyles;
    }

    void TearDown()
    {
        GeneratedModifier::GetTextStyle_styled_stringAccessor()->destroyPeer(peerTextStyle);
        GeneratedModifier::GetDecorationStyleAccessor()->destroyPeer(decorationStylePeer);
        GeneratedModifier::GetBaselineOffsetStyleAccessor()->destroyPeer(peerBaselineOffsetStyle);
        GeneratedModifier::GetLetterSpacingStyleAccessor()->destroyPeer(peerLetterSpacingStyle);
        GeneratedModifier::GetTextShadowStyleAccessor()->destroyPeer(peerTextShadowStyle);
        GeneratedModifier::GetLineHeightStyleAccessor()->destroyPeer(peerLineHeightStyle);
        GeneratedModifier::GetBackgroundColorStyleAccessor()->destroyPeer(peerBackgroundColorStyle);
        GeneratedModifier::GetUrlStyleAccessor()->destroyPeer(peerUrlStyle);
        GeneratedModifier::GetParagraphStyleAccessor()->destroyPeer(peerParagraphStyle);
        GeneratedModifier::GetParagraphStyleAccessor()->destroyPeer(peerParagraphStylePM);
        while (!lengthMetrics.empty()) {
            auto item = lengthMetrics.back();
            lengthMetrics.pop_back();
            LengthMetricsPeer::Destroy(item);
        }
    }

private:
    std::vector<LengthMetricsPeer*> lengthMetrics = {};
    LengthMetricsPeer* CreateLengthMetricsPeer(const Dimension& dimension)
    {
        auto lengthMetricsPeer = LengthMetricsPeer::Create(dimension);
        lengthMetrics.push_back(lengthMetricsPeer);
        return lengthMetricsPeer;
    }

    // TextStyle_styled_string
    Ark_TextStyle_styled_string peerTextStyle = {};
    void FillTextStyle(Ark_StyledStringValue& styledValue)
    {
        auto fontSizePeer = CreateLengthMetricsPeer(std::get<1>(TEST_FONT_SIZE));
        const Ark_TextStyleInterface textStyle = {
            .fontColor = Converter::ArkUnion<Opt_ResourceColor, Ark_Color>(std::get<1>(TEST_FONT_COLOR)),
            .fontFamily =  Converter::ArkUnion<Opt_ResourceStr, Ark_String>(TEST_FONT_FAMILIES[0]),
            .fontSize = Converter::ArkValue<Opt_LengthMetrics>(fontSizePeer),
            .fontWeight = Converter::ArkUnion<Opt_Union_Number_FontWeight_String, Ark_Number>(
                std::get<1>(TEST_FONT_WEIGHT)),
            .fontStyle = Converter::ArkValue<Opt_FontStyle>(TEST_FONT_STYLE),
        };
        auto optTextStyle = Converter::ArkValue<Opt_TextStyleInterface>(textStyle);
        peerTextStyle = GeneratedModifier::GetTextStyle_styled_stringAccessor()->ctor(&optTextStyle);
        styledValue = Converter::ArkUnion<Ark_StyledStringValue, Ark_TextStyle_styled_string>(peerTextStyle);
    };

    // DecorationStyle
    Ark_DecorationStyle decorationStylePeer = {};
    void FillDecorationStyle(Ark_StyledStringValue& styledValue)
    {
        const Ark_DecorationStyleInterface decorationStyle = {
            .type = Converter::ArkValue<Ark_TextDecorationType>(TEST_DCRN_TYPE),
            .color = Converter::ArkUnion<Opt_ResourceColor, Ark_Color>(std::get<1>(TEST_DCRN_COLOR)),
            .style = Converter::ArkValue<Opt_TextDecorationStyle>(TEST_DCRN_STYLE),
        };
        decorationStylePeer = GeneratedModifier::GetDecorationStyleAccessor()->ctor(&decorationStyle);
        styledValue = Converter::ArkUnion<Ark_StyledStringValue, Ark_DecorationStyle>(decorationStylePeer);
    }

    // BaselineOffsetStyle
    Ark_BaselineOffsetStyle peerBaselineOffsetStyle = {};
    void FillBaselineOffsetStyle(Ark_StyledStringValue& styledValue)
    {
        auto baselineOffsetOffset = CreateLengthMetricsPeer(std::get<1>(TEST_BASELINE_OFFSET));
        peerBaselineOffsetStyle = GeneratedModifier::GetBaselineOffsetStyleAccessor()->ctor(baselineOffsetOffset);
        styledValue = Converter::ArkUnion<Ark_StyledStringValue, Ark_BaselineOffsetStyle>(peerBaselineOffsetStyle);
    }

    // LetterSpacing
    Ark_LetterSpacingStyle peerLetterSpacingStyle = {};
    void FillLetterSpacing(Ark_StyledStringValue& styledValue)
    {
        auto letterSpacingPeer = CreateLengthMetricsPeer(std::get<1>(TEST_LETTER_SPACING));
        peerLetterSpacingStyle = GeneratedModifier::GetLetterSpacingStyleAccessor()->ctor(letterSpacingPeer);
        styledValue = Converter::ArkUnion<Ark_StyledStringValue, Ark_LetterSpacingStyle>(peerLetterSpacingStyle);
    }

    // TextShadow
    Ark_TextShadowStyle peerTextShadowStyle = {};
    void FillTextShadow(Ark_StyledStringValue& styledValue)
    {
        const Ark_ShadowOptions shadowOptions = {
            .radius = Converter::ArkUnion<Ark_Union_Number_Resource, Ark_Number>(TEST_TEXT_SHADOW_RADIUS),
            .type = Converter::ArkValue<Opt_ShadowType>(std::get<1>(TEST_TEXT_SHADOW_TYPE)),
            .color= Converter::ArkUnion<Opt_Union_Color_String_Resource_ColoringStrategy, Ark_Color>(
                std::get<1>(TEST_TEXT_SHADOW_COLOR)),
            .offsetX = Converter::ArkUnion<Opt_Union_Number_Resource, Ark_Number>(TEST_TEXT_SHADOW_OFFSET),
            .offsetY = Converter::ArkUnion<Opt_Union_Number_Resource, Ark_Number>(TEST_TEXT_SHADOW_OFFSET),
            .fill = Converter::ArkValue<Opt_Boolean>(true)
        };
        auto textShadow = Converter::ArkUnion<Ark_Union_ShadowOptions_Array_ShadowOptions,
            Ark_ShadowOptions>(shadowOptions);
        peerTextShadowStyle = GeneratedModifier::GetTextShadowStyleAccessor()->ctor(&textShadow);
        styledValue = Converter::ArkUnion<Ark_StyledStringValue, Ark_TextShadowStyle>(peerTextShadowStyle);
    }

    // LineHeightStyle
    Ark_LineHeightStyle peerLineHeightStyle = {};
    void FillLineHeight(Ark_StyledStringValue& styledValue)
    {
        auto lineHeightPeer = CreateLengthMetricsPeer(std::get<1>(TEST_LINE_HEIGHT));
        peerLineHeightStyle = GeneratedModifier::GetLineHeightStyleAccessor()->ctor(lineHeightPeer);
        styledValue = Converter::ArkUnion<Ark_StyledStringValue, Ark_LineHeightStyle>(peerLineHeightStyle);
    }

    // BackgroundColor
    Ark_BackgroundColorStyle peerBackgroundColorStyle = {};
    void FillBackgroundColor(Ark_StyledStringValue& styledValue)
    {
        Ark_TextBackgroundStyle textBackgroundStyle = {
            .color = Converter::ArkUnion<Opt_ResourceColor, Ark_Color>(std::get<1>(TEST_BGCL_COLOR)),
            .radius = Converter::ArkUnion<
                Opt_Union_Dimension_BorderRadiuses, Ark_Length>(std::get<1>(TEST_BGCL_RADIUS))
        };
        peerBackgroundColorStyle = GeneratedModifier::GetBackgroundColorStyleAccessor()->ctor(&textBackgroundStyle);
        styledValue = Converter::ArkUnion<Ark_StyledStringValue, Ark_BackgroundColorStyle>(peerBackgroundColorStyle);
    }

    // Url
    Ark_UrlStyle peerUrlStyle = {};
    void FillUrl(Ark_StyledStringValue& styledValue)
    {
        auto url = Converter::ArkValue<Ark_String>(TEST_URL);
        peerUrlStyle = GeneratedModifier::GetUrlStyleAccessor()->ctor(&url);
        styledValue = Converter::ArkUnion<Ark_StyledStringValue, Ark_UrlStyle>(peerUrlStyle);
    }

    // ParagraphStyle
    Ark_ParagraphStyle peerParagraphStyle = {};
    void FillParagraphStyle(Ark_StyledStringValue& styledValue)
    {
        auto textIndentPeer = CreateLengthMetricsPeer(std::get<1>(TEST_PSST_TEXT_INDEN));
        auto leadingMarginPeer = CreateLengthMetricsPeer(std::get<1>(TEST_PSST_LEADING_MARGIN));
        Ark_ParagraphStyleInterface paragraphStyle = {
            .textAlign = Converter::ArkValue<Opt_TextAlign>(std::get<1>(TEST_PSST_TEXT_ALIGN)),
            .textIndent = Converter::ArkValue<Opt_LengthMetrics>(textIndentPeer),
            .maxLines = Converter::ArkValue<Opt_Number>(TEST_PSST_MAX_LINES),
            .overflow = Converter::ArkValue<Opt_TextOverflow>(std::get<1>(TEST_PSST_OVERFLOW)),
            .wordBreak = Converter::ArkValue<Opt_WordBreak>(std::get<1>(TEST_PSST_WORD_BREAK)),
            .leadingMargin = Converter::ArkUnion<
                Opt_Union_LengthMetrics_LeadingMarginPlaceholder, Ark_LengthMetrics>(leadingMarginPeer),
        };
        auto optParagraphStyle = Converter::ArkValue<Opt_ParagraphStyleInterface>(paragraphStyle);
        peerParagraphStyle = GeneratedModifier::GetParagraphStyleAccessor()->ctor(&optParagraphStyle);
        styledValue = Converter::ArkUnion<Ark_StyledStringValue, Ark_ParagraphStyle>(peerParagraphStyle);
    }

    // ParagraphStylePM
    Ark_ParagraphStyle peerParagraphStylePM = {};
    void FillParagraphStylePM(Ark_StyledStringValue& styledValue)
    {
        auto textIndentPMPeer = CreateLengthMetricsPeer(std::get<1>(TEST_PSST_TEXT_INDEN));
        auto leadingMargin = Converter::ArkUnion<
            Opt_Union_LengthMetrics_LeadingMarginPlaceholder, Ark_LeadingMarginPlaceholder>(TEST_PSPM_LEADING_MARGIN);
        Ark_ParagraphStyleInterface paragraphStyle = {
            .textAlign = Converter::ArkValue<Opt_TextAlign>(std::get<1>(TEST_PSST_TEXT_ALIGN)),
            .textIndent = Converter::ArkValue<Opt_LengthMetrics>(textIndentPMPeer),
            .maxLines = Converter::ArkValue<Opt_Number>(TEST_PSST_MAX_LINES),
            .overflow = Converter::ArkValue<Opt_TextOverflow>(std::get<1>(TEST_PSST_OVERFLOW)),
            .wordBreak = Converter::ArkValue<Opt_WordBreak>(std::get<1>(TEST_PSST_WORD_BREAK)),
            .leadingMargin = leadingMargin,
        };
        auto optParagraphStyle = Converter::ArkValue<Opt_ParagraphStyleInterface>(paragraphStyle);
        peerParagraphStylePM = GeneratedModifier::GetParagraphStyleAccessor()->ctor(&optParagraphStyle);
        styledValue = Converter::ArkUnion<Ark_StyledStringValue, Ark_ParagraphStyle>(peerParagraphStylePM);
    }
};
namespace {
    static StyleOptionsKeeper g_styleOptions;
}

using namespace testing;
using namespace testing::ext;

struct StyledStringUnionNull {
    Ark_Union_String_ImageAttachment_CustomSpan* Union() { return nullptr; }
    Opt_Array_StyleOptions* Styles() { return nullptr; }
    void TearDown() {}
};
struct StyledStringUnionString {
    Ark_Union_String_ImageAttachment_CustomSpan value = Converter::ArkUnion<
            Ark_Union_String_ImageAttachment_CustomSpan, Ark_String>(STRING_TEST_VALUE);
    Opt_Array_StyleOptions stylesOpt;
    std::unique_ptr<Converter::ArkArrayHolder<Array_StyleOptions>> holderStyles;

    Ark_Union_String_ImageAttachment_CustomSpan* Union()
    {
        return &value;
    }
    Opt_Array_StyleOptions* Styles()
    {
        holderStyles = std::make_unique<Converter::ArkArrayHolder<Array_StyleOptions>>(g_styleOptions.GetOptions());
        stylesOpt = holderStyles->OptValue<Opt_Array_StyleOptions>();
        return &stylesOpt;
    }
    void TearDown()
    {
        g_styleOptions.TearDown();
    }
};

struct StyledStringUnionImageAttachment {
    Ark_Union_String_ImageAttachment_CustomSpan* Union()
    {
        peer = GeneratedModifier::GetImageAttachmentAccessor()->ctor(&IMAGEATTACHMENT_TEST_VALUE);
        static Ark_Union_String_ImageAttachment_CustomSpan value = Converter::ArkUnion<
            Ark_Union_String_ImageAttachment_CustomSpan, Ark_ImageAttachment>(peer);
        return &value;
    }
    Opt_Array_StyleOptions* Styles() { return nullptr; }
    void TearDown()
    {
        GeneratedModifier::GetImageAttachmentAccessor()->destroyPeer(peer);
    }
private:
    ImageAttachmentPeer* peer = nullptr;
};

template <typename V1>
class StyledStringAccessorTest : public AccessorTestCtorBase<GENERATED_ArkUIStyledStringAccessor,
    &GENERATED_ArkUIAccessors::getStyledStringAccessor, StyledStringPeer> {
public:
    static void SetUpTestCase()
    {
        AccessorTestCtorBase::SetUpTestCase();
        SetupTheme<TextTheme>();

        auto container = Container::CurrentSafely();
        ASSERT_NE(container, nullptr);
        auto mockContainer = AceType::DynamicCast<MockContainer>(container);
        ASSERT_NE(mockContainer, nullptr);
        auto taskExecutor = AceType::MakeRefPtr<MockTaskExecutor>(false);
        mockContainer->SetTaskExecutor(taskExecutor);
        mockContainer->UpdateCurrent(TEST_CONTAINER_ID);
        AceEngine& aceEngine = AceEngine::Get();
        aceEngine.AddContainer(TEST_CONTAINER_ID, container);
    }

    void SetUp(void) override
    {
        AccessorTestCtorBase::SetUp();
    }

    static void TearDownTestCase()
    {
        AccessorTestCtorBase::TearDownTestCase();

        AceEngine& aceEngine = AceEngine::Get();
        aceEngine.RemoveContainer(TEST_CONTAINER_ID);
    }

    void* CreatePeerInstance() override
    {
        auto value = settings.Union();
        auto styles = settings.Styles();
        return accessor_->ctor(value, styles);
    }

    void TearDown() override
    {
        AccessorTestCtorBase::TearDown();
        settings.TearDown();
    }

    bool IsSpanItemSame(const std::list<RefPtr<NG::SpanItem>>& src, const std::list<RefPtr<NG::SpanItem>>& other)
    {
        return std::equal(src.begin(), src.end(), other.begin(), other.end(),
            [](const RefPtr<NG::SpanItem>& lhs, const RefPtr<NG::SpanItem>& rhs) -> bool {
                return lhs->interval.first == rhs->interval.first && lhs->interval.second == rhs->interval.second &&
                    lhs->content == rhs->content;
            });
    }

    Ark_VMContext vmContext_ = nullptr;
    Ark_AsyncWorkerPtr asyncWorker_ = nullptr;
private:
    V1 settings;
};

using StyledStringAccessorUnionNullTest = StyledStringAccessorTest<StyledStringUnionNull>;
using StyledStringAccessorUnionStringTest = StyledStringAccessorTest<StyledStringUnionString>;
using StyledStringAccessorUnionImageAttachmentTest = StyledStringAccessorTest<StyledStringUnionImageAttachment>;

/**
 * @tc.name: peerSucceeded
 * @tc.desc: peer succeeded
 * @tc.type: FUNC
 */
HWTEST_F(StyledStringAccessorUnionNullTest, peerSucceeded, TestSize.Level1)
{
    ASSERT_NE(peer_, nullptr);
}

/**
 * @tc.name:styledStringCtorText
 * @tc.desc: Union string is casted
 * @tc.type: FUNC
 */
HWTEST_F(StyledStringAccessorUnionStringTest, styledStringCtorText, TestSize.Level1)
{
    ASSERT_NE(peer_->spanString, nullptr);
    EXPECT_EQ(peer_->spanString->GetString(), STRING_TEST_VALUE);
}

/**
 * @tc.name:styledStringCtorSpansOn
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(StyledStringAccessorUnionStringTest, styledStringCtorSpansOn, TestSize.Level1)
{
    ASSERT_NE(peer_->spanString, nullptr);
    auto spansStr = peer_->spanString->GetSpans(TEST_START_STR, TEST_LENGTH);
    EXPECT_EQ(spansStr.size(), 1);
    auto spansDcrn = peer_->spanString->GetSpans(TEST_START_DCRN, TEST_LENGTH);
    EXPECT_EQ(spansDcrn.size(), 1);
    auto spansBsl = peer_->spanString->GetSpans(TEST_START_BSL, TEST_LENGTH);
    EXPECT_EQ(spansBsl.size(), 1);
    auto spansLsp = peer_->spanString->GetSpans(TEST_START_LSP, TEST_LENGTH);
    EXPECT_EQ(spansLsp.size(), 1);
    auto spansShadow = peer_->spanString->GetSpans(TEST_START_TSH, TEST_LENGTH);
    EXPECT_EQ(spansShadow.size(), 1);
    auto spansLineHeight = peer_->spanString->GetSpans(TEST_START_LNHT, TEST_LENGTH);
    EXPECT_EQ(spansLineHeight.size(), 1);
    auto spansBackgroundColor = peer_->spanString->GetSpans(TEST_START_BGCL, TEST_LENGTH);
    EXPECT_EQ(spansBackgroundColor.size(), 1);
    auto spansUrl = peer_->spanString->GetSpans(TEST_START_URL, TEST_LENGTH);
    EXPECT_EQ(spansUrl.size(), 1);
    auto paragraphSpan = peer_->spanString->GetSpans(TEST_START_PSST, TEST_LENGTH);
    EXPECT_EQ(paragraphSpan.size(), 1);
    auto paragraphSpanPM = peer_->spanString->GetSpans(TEST_START_PSPM, TEST_LENGTH);
    EXPECT_EQ(paragraphSpanPM.size(), 1);
}

/**
 * @tc.name:styledStringCtorSpansFont
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(StyledStringAccessorUnionStringTest, styledStringCtorSpansFont, TestSize.Level1)
{
    ASSERT_NE(peer_->spanString, nullptr);

    auto spans = peer_->spanString->GetSpans(TEST_START_STR, TEST_LENGTH);
    ASSERT_EQ(spans.size(), 1);
    ASSERT_NE(spans[0], nullptr);
    auto spanFont = AceType::DynamicCast<FontSpan>(spans[0]);
    EXPECT_NE(spanFont, nullptr);
    Font font = spanFont->GetFont();
    EXPECT_EQ(font.GetFontColor(), std::get<0>(TEST_FONT_COLOR));
    EXPECT_EQ(font.GetFontFamily(), TEST_FONT_FAMILIES[0]);
    EXPECT_TRUE(font.GetFontSize().has_value());
    if (font.GetFontSize().has_value()) {
        EXPECT_EQ(font.GetFontSize()->ToString(), std::get<0>(TEST_FONT_SIZE));
    }
    EXPECT_TRUE(font.GetFontStyle().has_value());
    if (font.GetFontStyle().has_value()) {
        EXPECT_EQ(font.GetFontStyle().value(), TEST_FONT_STYLE);
    }
    EXPECT_TRUE(font.GetFontWeight().has_value());
    if (font.GetFontWeight().has_value()) {
        EXPECT_EQ(font.GetFontWeight().value(), std::get<0>(TEST_FONT_WEIGHT));
    }
}

/**
 * @tc.name:styledStringCtorDecorationSpan
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(StyledStringAccessorUnionStringTest, styledStringCtorDecorationSpan, TestSize.Level1)
{
    ASSERT_NE(peer_->spanString, nullptr);

    auto spans = peer_->spanString->GetSpans(TEST_START_DCRN, TEST_LENGTH);
    ASSERT_EQ(spans.size(), 1);
    auto decorationSpan = AceType::DynamicCast<DecorationSpan>(spans[0]);
    EXPECT_NE(decorationSpan, nullptr);
    EXPECT_EQ(decorationSpan->GetTextDecorationType(), TEST_DCRN_TYPE);
    EXPECT_TRUE(decorationSpan->GetColor().has_value());
    if (decorationSpan->GetColor()) {
        EXPECT_EQ(decorationSpan->GetColor()->ToString(), std::get<0>(TEST_DCRN_COLOR));
    }
    EXPECT_TRUE(decorationSpan->GetTextDecorationStyle().has_value());
    if (decorationSpan->GetTextDecorationStyle()) {
        EXPECT_EQ(decorationSpan->GetTextDecorationStyle().value(), TEST_DCRN_STYLE);
    }
}

/**
 * @tc.name:styledStringCtorBaselineOffsetSpan
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(StyledStringAccessorUnionStringTest, styledStringCtorBaselineOffsetSpan, TestSize.Level1)
{
    ASSERT_NE(peer_->spanString, nullptr);

    auto spans = peer_->spanString->GetSpans(TEST_START_BSL, TEST_LENGTH);
    ASSERT_EQ(spans.size(), 1);
    auto baselineSpan = AceType::DynamicCast<BaselineOffsetSpan>(spans[0]);
    EXPECT_NE(baselineSpan, nullptr);
    EXPECT_EQ(baselineSpan->GetBaselineOffset().ToString(), std::get<0>(TEST_BASELINE_OFFSET));
}

/**
 * @tc.name:styledStringCtorLetterSpacing
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(StyledStringAccessorUnionStringTest, styledStringCtorLetterSpacing, TestSize.Level1)
{
    ASSERT_NE(peer_->spanString, nullptr);

    auto spans = peer_->spanString->GetSpans(TEST_START_LSP, TEST_LENGTH);
    ASSERT_EQ(spans.size(), 1);
    auto letterSpacingSpan = AceType::DynamicCast<LetterSpacingSpan>(spans[0]);
    EXPECT_NE(letterSpacingSpan, nullptr);
    EXPECT_EQ(letterSpacingSpan->GetLetterSpacing().ToString(), std::get<0>(TEST_LETTER_SPACING));
}

/**
 * @tc.name:styledStringCtorTextShadow
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(StyledStringAccessorUnionStringTest, styledStringCtorTextShadow, TestSize.Level1)
{
    ASSERT_NE(peer_->spanString, nullptr);

    auto spans = peer_->spanString->GetSpans(TEST_START_TSH, TEST_LENGTH);
    ASSERT_EQ(spans.size(), 1);
    auto spansShadow = AceType::DynamicCast<TextShadowSpan>(spans[0]);
    EXPECT_NE(spansShadow, nullptr);
    auto shadows = spansShadow->GetTextShadow();
    EXPECT_EQ(shadows.size(), 1);
    Ace::Shadow shadow = shadows[0];
    EXPECT_EQ(shadow.GetShadowType(), std::get<0>(TEST_TEXT_SHADOW_TYPE));
    EXPECT_EQ(shadow.GetColor().ToString(), std::get<0>(TEST_TEXT_SHADOW_COLOR));
    EXPECT_EQ(shadow.GetBlurRadius(), TEST_TEXT_SHADOW_RADIUS);
    EXPECT_EQ(shadow.GetOffset().GetX(), TEST_TEXT_SHADOW_OFFSET);
    EXPECT_EQ(shadow.GetOffset().GetY(), TEST_TEXT_SHADOW_OFFSET);
}

/**
 * @tc.name:styledStringCtorLineHeight
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(StyledStringAccessorUnionStringTest, styledStringCtorLineHeight, TestSize.Level1)
{
    ASSERT_NE(peer_->spanString, nullptr);

    auto spans = peer_->spanString->GetSpans(TEST_START_LNHT, TEST_LENGTH);
    ASSERT_EQ(spans.size(), 1);
    auto lineHeight = AceType::DynamicCast<LineHeightSpan>(spans[0]);
    EXPECT_NE(lineHeight, nullptr);
    EXPECT_EQ(lineHeight->GetLineHeight().ToString(), std::get<0>(TEST_LINE_HEIGHT));
}

/**
 * @tc.name:styledStringCtorBackgroundColor
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(StyledStringAccessorUnionStringTest, styledStringCtorBackgroundColor, TestSize.Level1)
{
    ASSERT_NE(peer_->spanString, nullptr);

    auto spans = peer_->spanString->GetSpans(TEST_START_BGCL, TEST_LENGTH);
    ASSERT_EQ(spans.size(), 1);
    auto backgroundColor = AceType::DynamicCast<BackgroundColorSpan>(spans[0]);
    EXPECT_NE(backgroundColor, nullptr);
    TextBackgroundStyle style = backgroundColor->GetBackgroundColor();
    EXPECT_TRUE(style.backgroundColor.has_value());
    if (style.backgroundColor.has_value()) {
        EXPECT_EQ(style.backgroundColor.value().ToString(), std::get<0>(TEST_BGCL_COLOR));
    }
    EXPECT_TRUE(style.backgroundRadius.has_value());
    if (style.backgroundRadius.has_value()) {
        EXPECT_TRUE(style.backgroundRadius->radiusTopLeft.has_value());
        if (auto radius = style.backgroundRadius->radiusTopLeft; radius) {
            EXPECT_EQ(radius->ToString(), std::get<0>(TEST_BGCL_RADIUS));
        }
        EXPECT_TRUE(style.backgroundRadius->radiusTopRight.has_value());
        if (auto radius = style.backgroundRadius->radiusTopRight; radius) {
            EXPECT_EQ(radius->ToString(), std::get<0>(TEST_BGCL_RADIUS));
        }
        EXPECT_TRUE(style.backgroundRadius->radiusBottomLeft.has_value());
        if (auto radius = style.backgroundRadius->radiusBottomLeft; radius) {
            EXPECT_EQ(radius->ToString(), std::get<0>(TEST_BGCL_RADIUS));
        }
        EXPECT_TRUE(style.backgroundRadius->radiusBottomRight.has_value());
        if (auto radius = style.backgroundRadius->radiusBottomRight; radius) {
            EXPECT_EQ(radius->ToString(), std::get<0>(TEST_BGCL_RADIUS));
        }
    }
}

/**
 * @tc.name:styledStringCtorUrl
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(StyledStringAccessorUnionStringTest, styledStringCtorUrl, TestSize.Level1)
{
    ASSERT_NE(peer_->spanString, nullptr);

    auto spans = peer_->spanString->GetSpans(TEST_START_URL, TEST_LENGTH);
    ASSERT_EQ(spans.size(), 1);
    auto urlSpan = AceType::DynamicCast<UrlSpan>(spans[0]);
    EXPECT_NE(urlSpan, nullptr);
    EXPECT_EQ(urlSpan->GetUrlSpanAddress(), TEST_URL);
}

/**
 * @tc.name:DISABLED_styledStringCtorGesture
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(StyledStringAccessorUnionStringTest, DISABLED_styledStringCtorGesture, TestSize.Level1)
{
    // not implement
}

/**
 * @tc.name:styledStringCtorParagraphStyle
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(StyledStringAccessorUnionStringTest, styledStringCtorParagraphStyle, TestSize.Level1)
{
    ASSERT_NE(peer_->spanString, nullptr);

    auto spans = peer_->spanString->GetSpans(TEST_START_PSST, TEST_LENGTH);
    ASSERT_EQ(spans.size(), 1);
    auto paragraphSpan = AceType::DynamicCast<ParagraphStyleSpan>(spans[0]);
    EXPECT_NE(paragraphSpan, nullptr);
    SpanParagraphStyle style = paragraphSpan->GetParagraphStyle();
    EXPECT_TRUE(style.align.has_value());
    if (style.align.has_value()) {
        EXPECT_EQ(style.align.value(), std::get<0>(TEST_PSST_TEXT_ALIGN));
    }
    EXPECT_TRUE(style.maxLines.has_value());
    if (style.maxLines.has_value()) {
        EXPECT_EQ(style.maxLines.value(), TEST_PSST_MAX_LINES);
    }
    EXPECT_TRUE(style.wordBreak.has_value());
    if (style.wordBreak.has_value()) {
        EXPECT_EQ(style.wordBreak.value(), std::get<0>(TEST_PSST_WORD_BREAK));
    }
    EXPECT_TRUE(style.textOverflow.has_value());
    if (style.textOverflow.has_value()) {
        EXPECT_EQ(style.textOverflow.value(), std::get<0>(TEST_PSST_OVERFLOW));
    }
    EXPECT_TRUE(style.leadingMargin.has_value());
    if (style.leadingMargin.has_value()) {
        auto size = style.leadingMargin.value().size;
        EXPECT_EQ(size.ToString(), std::get<0>(TEST_PSST_LEADING_MARGIN));
    }
}

/**
 * @tc.name: styledStringCtorParagraphStylePixelMap
 * @tc.desc: PixelMap check in the ParagraphStyle
 * @tc.type: FUNC
 */
HWTEST_F(StyledStringAccessorUnionStringTest, styledStringCtorParagraphStylePixelMap, TestSize.Level1)
{
    ASSERT_NE(peer_->spanString, nullptr);
    auto spans = peer_->spanString->GetSpans(TEST_START_PSPM, TEST_LENGTH);
    ASSERT_EQ(spans.size(), 1);
    auto paragraphSpan = AceType::DynamicCast<ParagraphStyleSpan>(spans[0]);
    ASSERT_NE(paragraphSpan, nullptr);
    SpanParagraphStyle style = paragraphSpan->GetParagraphStyle();
    ASSERT_TRUE(style.leadingMargin.has_value());
    auto size = style.leadingMargin.value().size;
    EXPECT_EQ(size.ToString(), std::get<0>(TEST_TUPLE_DIMENSION_DIMENSION));
    auto pixMap = style.leadingMargin.value().pixmap;
    EXPECT_EQ(pixMap, TEST_PIXELMAP->pixelMap);
}

/**
 * @tc.name:DISABLED_styledStringCtorExtSpan
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(StyledStringAccessorUnionStringTest, DISABLED_styledStringCtorExtSpan, TestSize.Level1)
{
    // not implement
}

/**
 * @tc.name:styledStringEquals
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(StyledStringAccessorUnionStringTest, styledStringEquals, TestSize.Level1)
{
    ASSERT_NE(peer_->spanString, nullptr);
    EXPECT_TRUE(accessor_->equals(peer_, peer_));
}

/**
 * @tc.name:styledStringLength
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(StyledStringAccessorUnionStringTest, styledStringLength, TestSize.Level1)
{
    ASSERT_NE(peer_->spanString, nullptr);

    auto length = StringUtils::ToWstring(STRING_TEST_VALUE).length();
    EXPECT_EQ(peer_->spanString->GetLength(), length);
    EXPECT_EQ(Converter::Convert<int32_t>(accessor_->getLength(peer_)), length);
}

/**
 * @tc.name:DISABLED_styledStringGetString
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(StyledStringAccessorUnionStringTest, DISABLED_styledStringGetString, TestSize.Level1)
{
    // not implement
}

/**
 * @tc.name:DISABLED_styledStringStyles
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(StyledStringAccessorUnionStringTest, DISABLED_styledStringStyles, TestSize.Level1)
{
    // not implement
}

/**
 * @tc.name:DISABLED_styledStringSubStyledString
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(StyledStringAccessorUnionStringTest, DISABLED_styledStringSubStyledString, TestSize.Level1)
{
}

/**
 * @tc.name: styledStringFromHtml
 * @tc.desc: converting html to StyledString
 * @tc.type: FUNC
 */
HWTEST_F(StyledStringAccessorUnionStringTest, styledStringFromHtml, TestSize.Level1)
{
    ASSERT_NE(accessor_->fromHtml, nullptr);
    SpanToHtml toHtml;
    auto htmlFromSpan = toHtml.ToHtml(*peer_->spanString);

    struct CheckEvent {
        int32_t nodeId;
        std::optional<StyledStringPeer*> value;
        std::optional<StringArray> errors;
    };
    static std::optional<CheckEvent> checkEvent = std::nullopt;
    auto onFromHtmlFunc = [](const Ark_Int32 resourceId, const Opt_StyledString value, const Opt_Array_String error) {
        checkEvent = {
            .nodeId = resourceId,
            .value = Converter::OptConvert<Ark_StyledString>(value),
            .errors = Converter::OptConvert<StringArray>(error)
        };
    };

    auto arkCallback = Converter::ArkValue<Callback_Opt_StyledString_Opt_Array_String_Void>(
        onFromHtmlFunc, EXPECTED_NODE_ID);

    auto arkStr = Converter::ArkValue<Ark_String>(htmlFromSpan);

    EXPECT_FALSE(checkEvent.has_value());
    accessor_->fromHtml(vmContext_, asyncWorker_, &arkStr, &arkCallback);
    ASSERT_TRUE(checkEvent.has_value());

    EXPECT_EQ(checkEvent->nodeId, EXPECTED_NODE_ID);
    ASSERT_TRUE(checkEvent->errors.has_value());
    StringArray errors = checkEvent->errors.value();
    if (errors.size() > 0) {
        for (const auto& error : errors) { EXPECT_EQ(error, ""); }
    } else {
        EXPECT_EQ(checkEvent->errors.value().size(), 0);
        ASSERT_TRUE(checkEvent->value.has_value());
        StyledStringPeer* mss = checkEvent->value.value();
        ASSERT_NE(mss, nullptr);
        auto mSpanString = mss->GetMutableString();
        ASSERT_NE(mSpanString, nullptr);

        HtmlToSpan hts;
        auto styledString = hts.ToSpanString(htmlFromSpan);
        ASSERT_NE(styledString, nullptr);
        auto baseString = styledString->GetString();
        auto outString = mSpanString->GetString();
        EXPECT_EQ(outString, baseString);

        EXPECT_TRUE(IsSpanItemSame(mSpanString->GetSpanItems(), peer_->spanString->GetSpanItems()));
    }
}

/**
 * @tc.name:DISABLED_styledStringToHtml
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(StyledStringAccessorUnionStringTest, DISABLED_styledStringToHtml, TestSize.Level1)
{
    // not implement
}

/**
 * @tc.name:DISABLED_styledStringMarshalling
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(StyledStringAccessorUnionStringTest, DISABLED_styledStringMarshalling, TestSize.Level1)
{
    // not implement
}

/**
 * @tc.name: styledStringUnmarshallingTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(StyledStringAccessorUnionStringTest, styledStringUnmarshalling1Test, TestSize.Level1)
{
    ASSERT_NE(accessor_->unmarshalling1, nullptr);

    std::vector<uint8_t> tlvData;
    auto encodeResult = peer_->spanString->EncodeTlv(tlvData);
    ASSERT_TRUE(encodeResult);
    auto data = tlvData.data();
    size_t bufferSize = tlvData.size();
    std::string testBuffer(data, data + bufferSize);
    ASSERT_FALSE(testBuffer.empty());

    struct CheckEvent {
        int32_t nodeId;
        std::string value;
    };
    static std::optional<CheckEvent> checkEvent = std::nullopt;
    auto onUnmarshalling = [](const Ark_Int32 resourceId,
        const Opt_StyledString value, const Opt_Array_String error) {
        auto arkPeer = Converter::OptConvert<Ark_StyledString>(value);
        ASSERT_TRUE(arkPeer.has_value());
        auto peer = arkPeer.value();
        ASSERT_NE(peer, nullptr);
        auto accessor = GeneratedModifier::GetStyledStringAccessor();
        ASSERT_NE(accessor, nullptr);
        checkEvent = {
            .nodeId = resourceId,
            .value = peer->spanString->GetString()
        };
        accessor->destroyPeer(peer);
    };

    auto arkCallback = Converter::ArkValue<
        Callback_Opt_StyledString_Opt_Array_String_Void>(onUnmarshalling, EXPECTED_NODE_ID);
    auto akrBuffer = Converter::ArkValue<Ark_Buffer>(testBuffer, nullptr);

    EXPECT_FALSE(checkEvent.has_value());
    accessor_->unmarshalling1(vmContext_, asyncWorker_, &akrBuffer, &arkCallback);
    ASSERT_TRUE(checkEvent.has_value());
    EXPECT_EQ(checkEvent->nodeId, EXPECTED_NODE_ID);
    EXPECT_EQ(peer_->spanString->GetString(), checkEvent->value);
}

/**
 * @tc.name: ctorImageAttachmentTest
 * @tc.desc: ImageAttachment check
 * @tc.type: FUNC
 */
HWTEST_F(StyledStringAccessorUnionImageAttachmentTest, ctorImageAttachmentTest, TestSize.Level1)
{
    ASSERT_NE(peer_->spanString, nullptr);
    auto spans = peer_->spanString->GetSpans(0, 1);
    ASSERT_EQ(spans.size(), 1);
    auto imageSpan = AceType::DynamicCast<ImageSpan>(spans[0]);
    ASSERT_NE(imageSpan, nullptr);
    const ImageSpanOptions& options = imageSpan->GetImageSpanOptions();
    ASSERT_TRUE(options.imagePixelMap.has_value());
    EXPECT_EQ(options.imagePixelMap.value(), TEST_PIXELMAP->pixelMap);
    const std::optional<ImageSpanAttribute>& imageAttribute = imageSpan->GetImageAttribute();
    ASSERT_TRUE(imageAttribute.has_value());
    ASSERT_TRUE(imageAttribute.value().size.has_value());
    ASSERT_TRUE(imageAttribute->size->width.has_value());
    ASSERT_TRUE(imageAttribute->size->height.has_value());
    EXPECT_EQ(imageAttribute->size->width->ToString(), TEST_SIZEOPTIONS);
    EXPECT_EQ(imageAttribute->size->height->ToString(), TEST_SIZEOPTIONS);
    ASSERT_TRUE(imageAttribute.value().verticalAlign.has_value());
    EXPECT_EQ(imageAttribute.value().verticalAlign.value(), std::get<0>(TEST_VERTICALALIGN));
    ASSERT_TRUE(imageAttribute.value().objectFit.has_value());
    EXPECT_EQ(imageAttribute.value().objectFit.value(), std::get<0>(TEST_IMAGEFIT));
    ASSERT_TRUE(imageAttribute.value().marginProp.has_value());
    auto marginStr = imageAttribute.value().marginProp.value().ToString();
    EXPECT_EQ(marginStr, TEST_LENGTHMETRICS_STR);
    ASSERT_TRUE(imageAttribute.value().borderRadius.has_value());
    auto borderRadiusStr = imageAttribute.value().borderRadius.value().ToString();
    EXPECT_EQ(borderRadiusStr, TEST_LENGTHMETRICS_BR_STR);
    ASSERT_TRUE(imageAttribute.value().paddingProp.has_value());
    auto paddingStr = imageAttribute.value().paddingProp.value().ToString();
    EXPECT_EQ(paddingStr, TEST_LENGTHMETRICS_STR);
}
} // namespace OHOS::Ace::NG

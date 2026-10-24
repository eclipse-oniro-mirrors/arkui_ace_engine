/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include <utility>
#include "test/unittest/core/pattern/rich_editor/rich_editor_common_test_ng.h"
#include "test/mock/frameworks/core/components_ng/render/mock_paragraph.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"
#include "test/mock/frameworks/core/common/mock_container.h"
#include "test/mock/frameworks/core/components_ng/render/mock_canvas_image.h"
#include "test/mock/frameworks/base/thread/mock_task_executor.h"
#include "core/components_ng/layout/layout_wrapper_node.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"
#include "core/components_ng/pattern/page_translate/page_translate_node.h"
#include "core/components_ng/pattern/text/span/span_string.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
enum class SpanType {
    TEXT,
    SYMBOL,
    IMAGE,
    BUILDER
};
namespace {
const std::vector<std::u16string> INSERT_VALUE_LIST = {
    u"哈哈哈haha123",
    u"\n哈哈哈haha123",
    u"哈哈哈\nhaha123",
    u"哈哈哈haha123\n",
    u"\n哈哈哈\nhaha123",
    u"\n哈哈哈haha123\n",
    u"哈哈哈\nhaha123\n",
    u"\n哈哈哈\nhaha123\n",
};
const auto BUILDER_NODE_1 = FrameNode::GetOrCreateFrameNode(V2::ROW_ETS_TAG,
    ElementRegister::GetInstance()->MakeUniqueId(),
    []() { return AceType::MakeRefPtr<LinearLayoutPattern>(false); });
const std::unordered_map<SpanType, std::function<void(RefPtr<RichEditorPattern>)>> SPAN_CONSTRUCTOR_MAP = {
    { SpanType::TEXT, [](RefPtr<RichEditorPattern> pattern) { pattern->AddTextSpan(TEXT_SPAN_OPTIONS_1);} },
    { SpanType::IMAGE, [](RefPtr<RichEditorPattern> pattern) { pattern->AddImageSpan(IMAGE_SPAN_OPTIONS_1);} },
    { SpanType::SYMBOL, [](RefPtr<RichEditorPattern> pattern) { pattern->AddSymbolSpan(SYMBOL_SPAN_OPTIONS_1);} },
    { SpanType::BUILDER, [](RefPtr<RichEditorPattern> pattern) { pattern->AddPlaceholderSpan(BUILDER_NODE_1, {});} }
};
} // namespace

class RichEditorSpanAmendTestNg : public RichEditorCommonTestNg {
public:
    void SetUp() override;
    void TearDown() override;
    template<typename... Args>
    void InitSpans(Args... args);
    template<typename T, typename... Args>
    void ConstructSpans(RefPtr<RichEditorPattern> pattern, T value, Args... args);
    void ConstructSpans(RefPtr<RichEditorPattern> pattern) {}
    static void TearDownTestSuite();
};

void RichEditorSpanAmendTestNg::SetUp()
{
    MockPipelineContext::SetUp();
    MockContainer::SetUp();
    MockContainer::Current()->taskExecutor_ = AceType::MakeRefPtr<MockTaskExecutor>();
    auto* stack = ViewStackProcessor::GetInstance();
    auto nodeId = stack->ClaimNodeId();
    richEditorNode_ = FrameNode::GetOrCreateFrameNode(
        V2::RICH_EDITOR_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<RichEditorPattern>(); });
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    richEditorPattern->InitScrollablePattern();
    richEditorPattern->SetRichEditorController(AceType::MakeRefPtr<RichEditorController>());
    richEditorPattern->GetRichEditorController()->SetPattern(AceType::WeakClaim(AceType::RawPtr(richEditorPattern)));
    richEditorPattern->CreateNodePaintMethod();
    richEditorNode_->GetGeometryNode()->SetContentSize({});
}

void RichEditorSpanAmendTestNg::TearDown()
{
    richEditorNode_ = nullptr;
    MockParagraph::TearDown();
}

void RichEditorSpanAmendTestNg::TearDownTestSuite()
{
    TestNG::TearDownTestSuite();
}

template<typename... Args>
void RichEditorSpanAmendTestNg::InitSpans(Args... args)
{
    RichEditorCommonTestNg::ClearSpan();
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ConstructSpans(pattern, args...);
}

template<typename T, typename... Args>
void RichEditorSpanAmendTestNg::ConstructSpans(RefPtr<RichEditorPattern> pattern, T value, Args... args)
{
    if constexpr (std::is_same_v<T, std::string>) {
        pattern->AddTextSpan({ .value = value });
    } else if constexpr (std::is_same_v<T, SpanType>) {
        auto it = SPAN_CONSTRUCTOR_MAP.find(value);
        (it->second)(pattern);
    }
    ConstructSpans(pattern, args...);
}

/**
 * @tc.name: AddTextSpan001
 * @tc.desc: test AddTextSpan
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSpanAmendTestNg, AddTextSpan001, TestSize.Level0)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    TextSpanOptions options;
    richEditorPattern->previewTextRecord_.previewContent = u"123";
    richEditorPattern->previewTextRecord_.previewTextHasStarted = true;
    richEditorPattern->previewTextRecord_.startOffset = 0;
    richEditorPattern->previewTextRecord_.endOffset = 0;
    EXPECT_EQ(richEditorPattern->previewTextRecord_.IsValid(), true);
    int32_t res = richEditorPattern->AddTextSpan(options, TextChangeReason::UNKNOWN, true, 0);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: AddTextSpan001
 * @tc.desc: test AddTextSpan
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSpanAmendTestNg, PlaceholderSpanNodeTest001, TestSize.Level0)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);

    auto placeholderSpanNode = PlaceholderSpanNode::GetOrCreateSpanNode(V2::PLACEHOLDER_SPAN_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<PlaceholderSpanPattern>(); });
    ASSERT_NE(placeholderSpanNode, nullptr);
    auto placeholderSpanItem = placeholderSpanNode->GetSpanItem();
    ASSERT_NE(placeholderSpanItem, nullptr);
    auto placeholderSpanPattern = placeholderSpanNode->GetPattern<PlaceholderSpanPattern>();
    ASSERT_NE(placeholderSpanPattern, nullptr);

    auto layoutWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(
        richEditorNode_, AceType::MakeRefPtr<GeometryNode>(), richEditorNode_->GetLayoutProperty());
    DirtySwapConfig config = { .frameSizeChange = true };

    placeholderSpanItem->needReLayout = false;
    placeholderSpanPattern->OnDirtyLayoutWrapperSwap(layoutWrapper, config);
    EXPECT_EQ(placeholderSpanItem->needReLayout, true);
}

/**
 * @tc.name: SpanAddAndClear001
 * @tc.desc: test delete span
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSpanAmendTestNg, SpanAddAndClear001, TestSize.Level0)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto richEditorController = richEditorPattern->GetRichEditorController();
    ASSERT_NE(richEditorController, nullptr);
    auto contentNode = richEditorNode_->GetChildAtIndex(0);
    ASSERT_NE(contentNode, nullptr);
    AddSpan(INIT_VALUE_1);
    AddImageSpan();
    AddSpan(INIT_VALUE_2);
    RangeOptions option1;
    option1.start = 5;
    option1.end = 10;
    richEditorController->DeleteSpans(option1);
    EXPECT_EQ(contentNode->GetChildren().size(), 2);
    ClearSpan();
    AddSpan(INIT_VALUE_1);
    AddImageSpan();
    AddSpan(INIT_VALUE_2);
    RangeOptions option2;
    option2.start = 10;
    option2.end = 5;
    richEditorController->DeleteSpans(option2);
    EXPECT_EQ(contentNode->GetChildren().size(), 2);
    ClearSpan();
    AddSpan(INIT_VALUE_1);
    AddImageSpan();
    AddSpan(INIT_VALUE_2);
    RangeOptions option3;
    option3.start = -5;
    option3.end = 10;
    richEditorController->DeleteSpans(option3);
    EXPECT_EQ(contentNode->GetChildren().size(), 1);
}

/**
 * @tc.name: SpanAddAndClear002
 * @tc.desc: test delete span
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSpanAmendTestNg, SpanAddAndClear002, TestSize.Level0)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto contentNode = richEditorNode_->GetChildAtIndex(0);
    ASSERT_NE(contentNode, nullptr);
    auto richEditorController = richEditorPattern->GetRichEditorController();
    ASSERT_NE(richEditorController, nullptr);
    AddSpan(INIT_VALUE_1);
    AddImageSpan();
    AddSpan(INIT_VALUE_2);
    RangeOptions option4;
    option4.start = 5;
    option4.end = -10;
    richEditorController->DeleteSpans(option4);
    EXPECT_EQ(contentNode->GetChildren().size(), 3);
    ClearSpan();
    AddSpan(INIT_VALUE_1);
    AddImageSpan();
    AddSpan(INIT_VALUE_2);
    RangeOptions option5;
    richEditorController->DeleteSpans(option5);
    EXPECT_TRUE(contentNode->GetChildren().empty());
    ClearSpan();
    AddSpan(INIT_VALUE_1);
    AddImageSpan();
    AddSpan(INIT_VALUE_2);
    option5.start = 100;
    option5.end = 10;
    richEditorController->DeleteSpans(option5);
    EXPECT_EQ(contentNode->GetChildren().size(), 3);
    ClearSpan();
    AddSpan(INIT_VALUE_1);
    AddImageSpan();
    AddSpan(INIT_VALUE_2);
    option5.start = 3;
    option5.end = 3;
    richEditorController->DeleteSpans(option5);
    EXPECT_EQ(contentNode->GetChildren().size(), 3);
    ClearSpan();
    richEditorController->DeleteSpans(option5);
}

/**
 * @tc.name: SymbolTest001
 * @tc.desc: test update symbol span
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSpanAmendTestNg, SymbolTest001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. get richEditor controller
     */
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto richEditorController = richEditorPattern->GetRichEditorController();
    ASSERT_NE(richEditorController, nullptr);

    /**
     * @tc.steps: step2. initalize symbol span properties
     */
    TextStyle style;
    style.SetFontSize(FONT_SIZE_VALUE);
    style.SetFontWeight(FONT_WEIGHT_VALUE);
    style.SetSymbolColorList(SYMBOL_COLOR_LIST_1);
    style.SetRenderStrategy(RENDER_STRATEGY_SINGLE);
    style.SetEffectStrategy(EFFECT_STRATEGY_NONE);
    SymbolSpanOptions options;
    options.symbolId = SYMBOL_ID;
    options.style = style;

    /**
     * @tc.steps: step3. add symbol span
     */
    auto index1 = richEditorController->AddSymbolSpan(options);
    EXPECT_EQ(index1, 0);
    auto index2 = richEditorController->AddSymbolSpan(options);
    EXPECT_EQ(index2, 1);

    /**
     * @tc.steps: step4. update symbol span style
     */
    struct UpdateSpanStyle updateSpanStyle;
    updateSpanStyle.updateSymbolFontSize = FONT_SIZE_VALUE_2;
    updateSpanStyle.updateSymbolFontWeight = FONT_WEIGHT_BOLD;
    updateSpanStyle.updateSymbolColor = SYMBOL_COLOR_LIST_2;
    updateSpanStyle.updateSymbolRenderingStrategy = RENDER_STRATEGY_MULTI_COLOR;
    updateSpanStyle.updateSymbolEffectStrategy = EFFECT_STRATEGY_SCALE;
    richEditorController->SetUpdateSpanStyle(updateSpanStyle);

    ImageSpanAttribute imageStyle;
    style.SetFontSize(FONT_SIZE_VALUE_2);
    style.SetFontWeight(FONT_WEIGHT_BOLD);
    style.SetSymbolColorList(SYMBOL_COLOR_LIST_2);
    style.SetRenderStrategy(RENDER_STRATEGY_MULTI_COLOR);
    style.SetEffectStrategy(EFFECT_STRATEGY_SCALE);

    // update the first symbol span
    richEditorController->UpdateSpanStyle(0, 2, style, imageStyle);

    /**
     * @tc.steps: step5. test symbol span style
     */
    auto contentNode = richEditorNode_->GetChildAtIndex(0);
    auto newSpan1 = AceType::DynamicCast<SpanNode>(contentNode->GetChildAtIndex(0));
    ASSERT_NE(newSpan1, nullptr);
    EXPECT_EQ(newSpan1->GetFontSize(), FONT_SIZE_VALUE_2);
    EXPECT_EQ(newSpan1->GetFontWeight(), FONT_WEIGHT_BOLD);
    EXPECT_EQ(newSpan1->GetSymbolColorList(), SYMBOL_COLOR_LIST_2);
    EXPECT_EQ(newSpan1->GetSymbolRenderingStrategy(), RENDER_STRATEGY_MULTI_COLOR);
    EXPECT_EQ(newSpan1->GetSymbolEffectStrategy(), EFFECT_STRATEGY_SCALE);

    auto newSpan2 = AceType::DynamicCast<SpanNode>(contentNode->GetChildAtIndex(1));
    ASSERT_NE(newSpan2, nullptr);
    EXPECT_EQ(newSpan2->GetFontSize(), FONT_SIZE_VALUE);
    EXPECT_EQ(newSpan2->GetFontWeight(), FONT_WEIGHT_VALUE);
    EXPECT_EQ(newSpan2->GetSymbolColorList(), SYMBOL_COLOR_LIST_1);
    EXPECT_EQ(newSpan2->GetSymbolRenderingStrategy(), RENDER_STRATEGY_SINGLE);
    EXPECT_EQ(newSpan2->GetSymbolEffectStrategy(), EFFECT_STRATEGY_NONE);

    ClearSpan();
}

/**
 * @tc.name: SetPlaceHolderStyledString
 * @tc.desc: test SetPlaceHolderStyledString
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSpanAmendTestNg, SetPlaceHolderStyledString, TestSize.Level1)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto richEditorController = richEditorPattern->GetRichEditorController();
    ASSERT_NE(richEditorController, nullptr);
    auto spanString = AceType::MakeRefPtr<SpanString>(INIT_VALUE_1);
    ASSERT_NE(spanString, nullptr);
    // Setup content host so MarkDirtyNode can be triggered
    richEditorPattern->OnAttachToFrameNode();
    auto host = richEditorPattern->GetContentHost();
    ASSERT_NE(host, nullptr);
    auto hostLayoutProperty = host->GetLayoutProperty();
    ASSERT_NE(hostLayoutProperty, nullptr);
    hostLayoutProperty->CleanDirty();
    richEditorController->SetPlaceholderStyledString(spanString);
    EXPECT_NE(richEditorPattern->styledPlaceholder_, nullptr);
    // Branch: host != nullptr, verify content host marked dirty with PROPERTY_UPDATE_MEASURE
    EXPECT_EQ(hostLayoutProperty->GetPropertyChangeFlag() & PROPERTY_UPDATE_MEASURE,
        PROPERTY_UPDATE_MEASURE);
    richEditorPattern->isShowPlaceholder_ = true;
    richEditorPattern->richTextRect_ = RectF(50, 50, 100, 140);
    richEditorPattern->contentRect_ = RectF(0, 0, 100, 100);
    std::vector<std::list<RefPtr<SpanItem>>> spans;
    richEditorPattern->SetPlaceholder(spans);
    EXPECT_EQ(spans.size(), 1);
    EXPECT_EQ(richEditorPattern->richTextRect_.GetOffset(), OffsetF(50.0f, 50.0f));
    // Branch: contentRect offset == (0,0) -> UpdateRichTextRectOffsetWithPadding
    richEditorPattern->isShowPlaceholder_ = false;
    PaddingProperty padding;
    padding.left = CalcLength(Dimension(20.0f, DimensionUnit::PX));
    padding.top = CalcLength(Dimension(30.0f, DimensionUnit::PX));
    richEditorNode_->GetLayoutProperty<RichEditorLayoutProperty>()->UpdatePadding(padding);
    richEditorPattern->SetPlaceholder(spans);
    EXPECT_EQ(richEditorPattern->richTextRect_.GetOffset(), OffsetF(20.0f, 30.0f));
    // Branch: contentRect offset != (0,0) -> richTextRect_.SetOffset(contentRect_.GetOffset())
    richEditorPattern->isShowPlaceholder_ = false;
    richEditorPattern->contentRect_ = RectF(10.0f, 20.0f, 100, 100);
    richEditorPattern->SetPlaceholder(spans);
    EXPECT_EQ(richEditorPattern->richTextRect_.GetOffset(), OffsetF(10.0f, 20.0f));
}

/**
 * @tc.name: PlaceholderImageNode
 * @tc.desc: test PlaceholderImageNode
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSpanAmendTestNg, PlaceholderImageNode, TestSize.Level1)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    ImageSpanOptions options;
    void* voidPtr = static_cast<void*>(new char[0]);
    RefPtr<PixelMap> pixelMap = PixelMap::CreatePixelMap(voidPtr);
    ASSERT_NE(pixelMap, nullptr);
    options.imagePixelMap = pixelMap;
    auto spanString = AceType::MakeRefPtr<SpanString>(options);
    ASSERT_NE(spanString, nullptr);

    // Set placeholder with image.
    richEditorPattern->SetPlaceholderStyledString(spanString);
    auto& placeholderSpanNodes = richEditorPattern->placeholderImageNodes_;
    std::vector<std::list<RefPtr<SpanItem>>> spans;
    richEditorPattern->SetPlaceholder(spans);
    EXPECT_EQ(spans.size(), 1);
    EXPECT_EQ(placeholderSpanNodes.size(), 1);

    // Remove placeholder image nodes when a text span is added.
    richEditorPattern->AddTextSpan(TEXT_SPAN_OPTIONS_1);
    EXPECT_TRUE(placeholderSpanNodes.empty());
    richEditorPattern->spans_.clear();
    richEditorPattern->SetPlaceholder(spans);
    EXPECT_EQ(spans.size(), 1);
    EXPECT_EQ(placeholderSpanNodes.size(), 1);

    // Remove placeholder image nodes since real content exists.
    richEditorPattern->BeforeCreateLayoutWrapper();
    EXPECT_EQ(placeholderSpanNodes.size(), 1);
    richEditorPattern->AddTextSpan(TEXT_SPAN_OPTIONS_1);
    ASSERT_FALSE(richEditorPattern->spans_.empty());
    richEditorPattern->BeforeCreateLayoutWrapper();
    EXPECT_TRUE(placeholderSpanNodes.empty());
}

/**
 * @tc.name: PageTranslate_ApplyAndReset
 * @tc.desc: Test RichEditorPattern ApplyPageTranslateResult and ResetPageTranslate
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSpanAmendTestNg, PageTranslate_ApplyAndReset, TestSize.Level1)
{
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);

    // ApplyPageTranslateResult with negative version returns true but does not set translated placeholder
    EXPECT_TRUE(richEditorPattern->ApplyPageTranslateResult("result", -1));
    EXPECT_FALSE(richEditorPattern->pageTranslatedContent_.has_value());

    // ApplyPageTranslateResult with valid result sets translated placeholder and version
    EXPECT_TRUE(richEditorPattern->ApplyPageTranslateResult("translated", 1));
    ASSERT_TRUE(richEditorPattern->pageTranslatedContent_.has_value());
    EXPECT_EQ(richEditorPattern->pageTranslatedContent_.value(), u"translated");

    // ResetPageTranslate clears translated placeholder and version
    richEditorPattern->ResetPageTranslate();
    EXPECT_FALSE(richEditorPattern->pageTranslatedContent_.has_value());

    // ResetPageTranslate again when already clean is a no-op
    richEditorPattern->ResetPageTranslate();
    EXPECT_FALSE(richEditorPattern->pageTranslatedContent_.has_value());
}

/**
 * @tc.name: PageTranslate_GetCurrentPlaceholderText
 * @tc.desc: Test RichEditorPattern GetCurrentPlaceholderText for plain and styled placeholder
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSpanAmendTestNg, PageTranslate_GetCurrentPlaceholderText, TestSize.Level1)
{
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);

    // plain placeholder: returns layoutProperty placeholder value
    auto layoutProperty = richEditorNode_->GetLayoutProperty<RichEditorLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    layoutProperty->UpdatePlaceholder(u"plain placeholder");
    EXPECT_EQ(richEditorPattern->GetCurrentPlaceholderText(), u"plain placeholder");

    // styled placeholder: returns styledPlaceholder_->GetU16string()
    RefPtr<SpanString> spanString = AceType::MakeRefPtr<SpanString>(u"Styled Placeholder");
    richEditorPattern->styledPlaceholder_ = spanString;
    EXPECT_EQ(richEditorPattern->GetCurrentPlaceholderText(), u"Styled Placeholder");
    richEditorPattern->styledPlaceholder_ = nullptr;
}

/**
 * @tc.name: PageTranslate_GetPageTranslateTextForReport
 * @tc.desc: Test RichEditorPattern GetPageTranslateTextForReport for plain and styled placeholder
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSpanAmendTestNg, PageTranslate_GetPageTranslateTextForReport, TestSize.Level1)
{
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto layoutProperty = richEditorNode_->GetLayoutProperty<RichEditorLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    // isShowPlaceholder_ is false returns empty string
    richEditorPattern->isShowPlaceholder_ = false;
    EXPECT_TRUE(richEditorPattern->GetPageTranslateTextForReport().empty());

    // isShowPlaceholder_ is true, lastDrawn mismatch returns empty string (plain placeholder)
    richEditorPattern->isShowPlaceholder_ = true;
    layoutProperty->UpdatePlaceholder(u"plain placeholder");
    richEditorPattern->lastDrawnPageTranslateContent_ = u"mismatch";
    EXPECT_TRUE(richEditorPattern->GetPageTranslateTextForReport().empty());

    // isShowPlaceholder_ is true, lastDrawn match returns the text (plain placeholder)
    richEditorPattern->lastDrawnPageTranslateContent_ = u"plain placeholder";
    EXPECT_EQ(richEditorPattern->GetPageTranslateTextForReport(), "plain placeholder");

    // styled placeholder: lastDrawn match returns the styled text
    RefPtr<SpanString> spanString = AceType::MakeRefPtr<SpanString>(u"Styled Placeholder");
    richEditorPattern->styledPlaceholder_ = spanString;
    richEditorPattern->lastDrawnPageTranslateContent_ = u"Styled Placeholder";
    EXPECT_EQ(richEditorPattern->GetPageTranslateTextForReport(), "Styled Placeholder");
    richEditorPattern->styledPlaceholder_ = nullptr;
}

/**
 * @tc.name: PageTranslate_ReportAndOnSourceChanged
 * @tc.desc: Test RichEditorPattern ReportPageTranslatePlaceholderDrawn and OnPlaceholderSourceTextChanged
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSpanAmendTestNg, PageTranslate_ReportAndOnSourceChanged, TestSize.Level1)
{
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto layoutProperty = richEditorNode_->GetLayoutProperty<RichEditorLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    // ReportPageTranslatePlaceholderDrawn sets lastDrawn to current placeholder (plain)
    richEditorPattern->isShowPlaceholder_ = true;
    layoutProperty->UpdatePlaceholder(u"plain placeholder");
    richEditorPattern->ReportPageTranslatePlaceholderDrawn();
    EXPECT_EQ(richEditorPattern->lastDrawnPageTranslateContent_, u"plain placeholder");

    // ReportPageTranslatePlaceholderDrawn with styled placeholder
    RefPtr<SpanString> spanString = AceType::MakeRefPtr<SpanString>(u"Styled Placeholder");
    richEditorPattern->styledPlaceholder_ = spanString;
    richEditorPattern->lastDrawnPageTranslateContent_.clear();
    richEditorPattern->ReportPageTranslatePlaceholderDrawn();
    EXPECT_EQ(richEditorPattern->lastDrawnPageTranslateContent_, u"Styled Placeholder");
}

/**
 * @tc.name: PageTranslate_OnSourceChangedGate
 * @tc.desc: Test OnPlaceholderSourceTextChanged gate uses isShowPlaceholder_ not lastDrawn
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSpanAmendTestNg, PageTranslate_OnSourceChangedGate, TestSize.Level1)
{
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    // setup: ContentChangeManager to verify ResetTranslateTextNode via translateTextVersions_
    auto host = richEditorPattern->GetHost();
    auto mgr = host->GetContext()->GetContentChangeManager();
    ASSERT_NE(mgr, nullptr);
    mgr->StartTextTranslateReport();
    auto nodeId = host->GetId();

    // branch: pageTranslatedContent_ has value -> gate passes, ResetTranslateTextNode clears entry
    richEditorPattern->ApplyPageTranslateResult("translated", 1);
    ASSERT_TRUE(richEditorPattern->pageTranslatedContent_.has_value());
    EXPECT_EQ(mgr->UpdateTranslateVersionIfNeeded(nodeId, "placeholder"), 1);
    EXPECT_EQ(mgr->translateTextVersions_.count(nodeId), 1);
    EXPECT_EQ(mgr->translateTextVersions_[nodeId].second, 1);
    EXPECT_EQ(mgr->UpdateTranslateVersionIfNeeded(nodeId, "placeholder"), -1);
    EXPECT_EQ(mgr->translateTextVersions_[nodeId].second, 1);
    richEditorPattern->OnPlaceholderSourceTextChanged();
    EXPECT_FALSE(richEditorPattern->pageTranslatedContent_.has_value());
    EXPECT_TRUE(richEditorPattern->lastDrawnPageTranslateContent_.empty());
    EXPECT_EQ(mgr->translateTextVersions_.count(nodeId), 0);
    EXPECT_EQ(mgr->UpdateTranslateVersionIfNeeded(nodeId, "placeholder"), 1);
    EXPECT_EQ(mgr->translateTextVersions_.count(nodeId), 1);
    EXPECT_EQ(mgr->translateTextVersions_[nodeId].second, 1);

    // branch: isShowPlaceholder_ true -> gate passes, ResetTranslateTextNode clears entry
    richEditorPattern->isShowPlaceholder_ = true;
    richEditorPattern->OnPlaceholderSourceTextChanged();
    EXPECT_EQ(mgr->translateTextVersions_.count(nodeId), 0);
    EXPECT_EQ(mgr->UpdateTranslateVersionIfNeeded(nodeId, "placeholder"), 1);
    EXPECT_EQ(mgr->translateTextVersions_.count(nodeId), 1);
    EXPECT_EQ(mgr->translateTextVersions_[nodeId].second, 1);

    // branch: isShowPlaceholder_ false -> gate blocks, entry retained (dedup)
    richEditorPattern->isShowPlaceholder_ = false;
    richEditorPattern->OnPlaceholderSourceTextChanged();
    EXPECT_EQ(mgr->translateTextVersions_.count(nodeId), 1);
    EXPECT_EQ(mgr->UpdateTranslateVersionIfNeeded(nodeId, "placeholder"), -1);
    EXPECT_EQ(mgr->translateTextVersions_.count(nodeId), 1);
    EXPECT_EQ(mgr->translateTextVersions_[nodeId].second, 1);
}

/**
 * @tc.name: PageTranslate_GetPageTranslateNodeId
 * @tc.desc: Test RichEditorPattern GetPageTranslateNodeId returns host id
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSpanAmendTestNg, PageTranslate_GetPageTranslateNodeId, TestSize.Level1)
{
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);

    EXPECT_EQ(richEditorPattern->GetPageTranslateNodeId(), richEditorNode_->GetId());
}

/**
 * @tc.name: PageTranslate_ClearPlaceholderImageNode
 * @tc.desc: Test ClearPlaceholderImageNode and SetTranslatedStyledPlaceholder clears image nodes
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSpanAmendTestNg, PageTranslate_ClearPlaceholderImageNode, TestSize.Level1)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    richEditorPattern->OnAttachToFrameNode();

    // Branch: no image nodes → no-op
    EXPECT_TRUE(richEditorPattern->placeholderImageNodes_.empty());
    richEditorPattern->ClearPlaceholderImageNode();
    EXPECT_TRUE(richEditorPattern->placeholderImageNodes_.empty());

    // Branch: image nodes present → cleared by ClearPlaceholderImageNode
    ImageSpanOptions options;
    void* voidPtr = static_cast<void*>(new char[0]);
    RefPtr<PixelMap> pixelMap = PixelMap::CreatePixelMap(voidPtr);
    ASSERT_NE(pixelMap, nullptr);
    options.imagePixelMap = pixelMap;
    auto spanString = AceType::MakeRefPtr<SpanString>(options);
    richEditorPattern->SetPlaceholderStyledString(spanString);
    std::vector<std::list<RefPtr<SpanItem>>> spans;
    richEditorPattern->SetPlaceholder(spans);
    EXPECT_EQ(richEditorPattern->placeholderImageNodes_.size(), 1);
    richEditorPattern->ClearPlaceholderImageNode();
    EXPECT_TRUE(richEditorPattern->placeholderImageNodes_.empty());

    // Branch: SetTranslatedStyledPlaceholder clears image nodes via SetPlaceholder
    richEditorPattern->SetPlaceholder(spans);
    EXPECT_EQ(richEditorPattern->placeholderImageNodes_.size(), 1);
    richEditorPattern->ApplyPageTranslateResult("translated", 1);
    ASSERT_TRUE(richEditorPattern->pageTranslatedContent_.has_value());
    spans.clear();
    richEditorPattern->SetPlaceholder(spans);
    EXPECT_TRUE(richEditorPattern->placeholderImageNodes_.empty());
    EXPECT_EQ(spans.size(), 1);
}

namespace {
// Build a SpanItem with given type/content/position; spanRange = [position-len, position).
RefPtr<SpanItem> MakeSpan(SpanItemType type, std::u16string content, int32_t position)
{
    auto span = AceType::MakeRefPtr<SpanItem>();
    span->spanItemType = type;
    span->content = content;
    span->position = position;
    span->rangeStart = position - static_cast<int32_t>(content.length());
    return span;
}

RefPtr<ImageSpanItem> MakeImageSpan(std::optional<std::u16string> imageTag, int32_t position)
{
    auto span = AceType::MakeRefPtr<ImageSpanItem>();
    span->content = u" ";
    span->position = position;
    span->rangeStart = position - static_cast<int32_t>(span->content.length());
    if (imageTag.has_value()) {
        span->options.imageTag = imageTag.value();
    }
    return span;
}

void ClearAndLoadSpans(RefPtr<RichEditorPattern> pattern, std::initializer_list<RefPtr<SpanItem>> spans)
{
    pattern->spans_.clear();
    for (const auto& span : spans) {
        pattern->spans_.emplace_back(span);
    }
}
} // namespace

/**
 * @tc.name: BuildRichText001
 * @tc.desc: Invalid range guards, span filtering (position==-1 / no-overlap / empty spans_).
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSpanAmendTestNg, BuildRichText001, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    auto s0 = MakeSpan(SpanItemType::NORMAL, u"hello", 5);   // [0,5)
    auto s1 = MakeImageSpan(u"[img]", 6);                    // [5,6)
    auto s2 = MakeSpan(SpanItemType::NORMAL, u"world", 11);  // [6,11)
    auto sInvalid = MakeSpan(SpanItemType::NORMAL, u"x", -1); // position==-1 skipped
    ClearAndLoadSpans(pattern, { s0, s1, s2, sInvalid });

    // Branch: start<0 -> return empty
    auto result = pattern->BuildRichText(-1, 5);
    EXPECT_TRUE(result.IsEmpty());

    // Branch: end<0 -> return empty
    result = pattern->BuildRichText(0, -1);
    EXPECT_TRUE(result.IsEmpty());

    // Branch: start>end -> return empty
    result = pattern->BuildRichText(5, 2);
    EXPECT_TRUE(result.IsEmpty());

    // Branch: no-overlap filtering — request [5,11) skips s0 (spanEnd 5 not > start 5)
    result = pattern->BuildRichText(5, 11);
    EXPECT_EQ(result.spans.size(), 2u);
    EXPECT_EQ(result.spans[0].range.start, 0); // relative range starts at 0
    EXPECT_EQ(result.spans[0].range.end, 1);

    // Branch: empty spans_ returns empty
    ClearAndLoadSpans(pattern, {});
    result = pattern->BuildRichText(0, 10);
    EXPECT_TRUE(result.IsEmpty());
}

/**
 * @tc.name: BuildRichText002
 * @tc.desc: Type mapping + content extraction per SpanItemType, SYMBOL partial overlap, unknown type skip.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSpanAmendTestNg, BuildRichText002, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);

    // NORMAL -> TEXT, content truncated on overlap
    ClearAndLoadSpans(pattern, { MakeSpan(SpanItemType::NORMAL, u"ab", 2) });
    auto result = pattern->BuildRichText(0, 2);
    ASSERT_EQ(result.spans.size(), 1u);
    EXPECT_EQ(result.spans[0].type, RichSpanType::TEXT);
    EXPECT_EQ(result.spans[0].content, "ab");

    // IMAGE -> IMAGE, nullopt imageTag fallback to single space
    ClearAndLoadSpans(pattern, { MakeImageSpan(std::nullopt, 1) });
    result = pattern->BuildRichText(0, 1);
    ASSERT_EQ(result.spans.size(), 1u);
    EXPECT_EQ(result.spans[0].type, RichSpanType::IMAGE);
    EXPECT_EQ(result.spans[0].content, " ");

    // SYMBOL -> SYMBOL; full overlap keeps 2-space content
    ClearAndLoadSpans(pattern, { MakeSpan(SpanItemType::SYMBOL, u"  ", 2) });
    result = pattern->BuildRichText(0, 2);
    ASSERT_EQ(result.spans.size(), 1u);
    EXPECT_EQ(result.spans[0].type, RichSpanType::SYMBOL);
    EXPECT_EQ(result.spans[0].content, "  ");
    EXPECT_EQ(result.spans[0].range.start, 0);
    EXPECT_EQ(result.spans[0].range.end, 2);

    // SYMBOL partial overlap (1 of 2 chars): content stays "  " (non-truncating)
    result = pattern->BuildRichText(1, 3);
    EXPECT_EQ(result.spans[0].content, "  ");
    EXPECT_EQ(result.spans[0].range.end, 1);

    // CustomSpan -> CUSTOM
    auto custom = AceType::MakeRefPtr<CustomSpanItem>();
    custom->content = u" ";
    custom->position = 1;
    custom->rangeStart = 0;
    ClearAndLoadSpans(pattern, { custom });
    result = pattern->BuildRichText(0, 1);
    ASSERT_EQ(result.spans.size(), 1u);
    EXPECT_EQ(result.spans[0].type, RichSpanType::CUSTOM);
    EXPECT_EQ(result.spans[0].content, " ");

    // PLACEHOLDER -> CUSTOM (non-truncating)
    auto placeholder = AceType::MakeRefPtr<PlaceholderSpanItem>();
    placeholder->content = u" ";
    placeholder->position = 1;
    placeholder->rangeStart = 0;
    ClearAndLoadSpans(pattern, { placeholder });
    result = pattern->BuildRichText(0, 1);
    ASSERT_EQ(result.spans.size(), 1u);
    EXPECT_EQ(result.spans[0].type, RichSpanType::CUSTOM);
    EXPECT_EQ(result.spans[0].content, " ");

    // Unknown type -> skipped (nullopt)
    auto unknown = MakeSpan(static_cast<SpanItemType>(99), u"?", 1);
    ClearAndLoadSpans(pattern, { unknown });
    result = pattern->BuildRichText(0, 1);
    EXPECT_TRUE(result.IsEmpty());
}

/**
 * @tc.name: BuildRichText003
 * @tc.desc: TEXT content truncation: full/left/right/both/single-char/non-ASCII overlap.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSpanAmendTestNg, BuildRichText003, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    // TEXT "hello world" [0,11)
    ClearAndLoadSpans(pattern, { MakeSpan(SpanItemType::NORMAL, u"hello world", 11) });

    // Full overlap
    auto result = pattern->BuildRichText(0, 11);
    ASSERT_EQ(result.spans.size(), 1u);
    EXPECT_EQ(result.spans[0].content, "hello world");
    EXPECT_EQ(result.spans[0].range.start, 0);
    EXPECT_EQ(result.spans[0].range.end, 11);

    // Left partial (span starts before range)
    result = pattern->BuildRichText(3, 11);
    EXPECT_EQ(result.spans[0].content, "lo world");
    EXPECT_EQ(result.spans[0].range.start, 0);
    EXPECT_EQ(result.spans[0].range.end, 8);

    // Right partial (span ends after range)
    result = pattern->BuildRichText(0, 7);
    EXPECT_EQ(result.spans[0].content, "hello w");
    EXPECT_EQ(result.spans[0].range.end, 7);

    // Both sides partial
    result = pattern->BuildRichText(2, 8);
    EXPECT_EQ(result.spans[0].content, "llo wo");
    EXPECT_EQ(result.spans[0].range.start, 0);
    EXPECT_EQ(result.spans[0].range.end, 6);

    // Single char overlap
    result = pattern->BuildRichText(5, 6);
    EXPECT_EQ(result.spans[0].content, " ");
    EXPECT_EQ(result.spans[0].range.end, 1);

    // Non-ASCII: u"你好世界" (4 u16 chars) [0,4), request [1,3)
    ClearAndLoadSpans(pattern, { MakeSpan(SpanItemType::NORMAL, u"你好世界", 4) });
    result = pattern->BuildRichText(1, 3);
    EXPECT_EQ(result.spans[0].content, "好世");
    EXPECT_EQ(result.spans[0].range.start, 0);
    EXPECT_EQ(result.spans[0].range.end, 2);
}

/**
 * @tc.name: BuildRichText004
 * @tc.desc: IMAGE content extraction: non-empty tag / empty string / nullopt fallback.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSpanAmendTestNg, BuildRichText004, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);

    // has_value=true, non-empty tag; content length (3) != range span (1) for IMAGE
    ClearAndLoadSpans(pattern, { MakeImageSpan(u"[图片]", 1) });
    auto result = pattern->BuildRichText(0, 1);
    ASSERT_EQ(result.spans.size(), 1u);
    EXPECT_EQ(result.spans[0].content, "[图片]");
    EXPECT_EQ(result.spans[0].range.start, 0);
    EXPECT_EQ(result.spans[0].range.end, 1);

    // has_value=true, empty string preserved (no fallback)
    ClearAndLoadSpans(pattern, { MakeImageSpan(u"", 1) });
    result = pattern->BuildRichText(0, 1);
    ASSERT_EQ(result.spans.size(), 1u);
    EXPECT_EQ(result.spans[0].content, "");

    // has_value=false, fallback to single space
    ClearAndLoadSpans(pattern, { MakeImageSpan(std::nullopt, 1) });
    result = pattern->BuildRichText(0, 1);
    ASSERT_EQ(result.spans.size(), 1u);
    EXPECT_EQ(result.spans[0].content, " ");
}

/**
 * @tc.name: BuildRichText006
 * @tc.desc: Multi-span relative range continuity with mixed TEXT truncation and IMAGE non-truncation.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSpanAmendTestNg, BuildRichText006, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    ClearAndLoadSpans(pattern, {
        MakeSpan(SpanItemType::NORMAL, u"hello", 5),    // [0,5)
        MakeImageSpan(u"[图片]", 6),                    // [5,6)
        MakeSpan(SpanItemType::NORMAL, u"world", 11),   // [6,11)
    });

    auto result = pattern->BuildRichText(3, 8);
    ASSERT_EQ(result.spans.size(), 3u);
    EXPECT_EQ(result.spans[0].content, "lo");
    EXPECT_EQ(result.spans[0].range.start, 0);
    EXPECT_EQ(result.spans[0].range.end, 2);
    EXPECT_EQ(result.spans[1].content, "[图片]");
    EXPECT_EQ(result.spans[1].range.start, 2);
    EXPECT_EQ(result.spans[1].range.end, 3);
    EXPECT_EQ(result.spans[2].content, "wo");
    EXPECT_EQ(result.spans[2].range.start, 3);
    EXPECT_EQ(result.spans[2].range.end, 5);
}

/**
 * @tc.name: BuildRichText007
 * @tc.desc: Boundary: zero-length range skips all spans; range beyond text end clamps overlapEnd.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSpanAmendTestNg, BuildRichText007, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    ClearAndLoadSpans(pattern, {
        MakeSpan(SpanItemType::NORMAL, u"hello", 5),    // [0,5)
        MakeImageSpan(u"[图片]", 6),                    // [5,6)
        MakeSpan(SpanItemType::NORMAL, u"world", 11),   // [6,11)
    });

    // Zero-length range [5,5): all spans skipped
    auto result = pattern->BuildRichText(5, 5);
    EXPECT_TRUE(result.IsEmpty());

    // Range beyond text end: overlapEnd clamped by min
    result = pattern->BuildRichText(9, 20);
    ASSERT_EQ(result.spans.size(), 1u);
    EXPECT_EQ(result.spans[0].content, "ld");
    EXPECT_EQ(result.spans[0].range.start, 0);
    EXPECT_EQ(result.spans[0].range.end, 2);
}

/**
 * @tc.name: BuildRichText008
 * @tc.desc: Zero-length span (empty content) and zero-length overlap (start==end crossing span).
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSpanAmendTestNg, BuildRichText008, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);

    // Zero-length span: content=u"", position=5 -> spanRange [5,5)
    ClearAndLoadSpans(pattern, { MakeSpan(SpanItemType::NORMAL, u"", 5) });
    auto result = pattern->BuildRichText(0, 10);
    ASSERT_EQ(result.spans.size(), 1u);
    EXPECT_EQ(result.spans[0].content, "");
    EXPECT_EQ(result.spans[0].range.start, 5);
    EXPECT_EQ(result.spans[0].range.end, 5);

    // Zero-length overlap: TEXT "hello" [2,7), request [5,5) -> overlap [5,5)
    ClearAndLoadSpans(pattern, { MakeSpan(SpanItemType::NORMAL, u"hello", 7) });
    result = pattern->BuildRichText(5, 5);
    ASSERT_EQ(result.spans.size(), 1u);
    EXPECT_EQ(result.spans[0].content, "");
    EXPECT_EQ(result.spans[0].range.start, 0);
    EXPECT_EQ(result.spans[0].range.end, 0);
}

/**
 * @tc.name: BuildRichText009
 * @tc.desc: SYMBOL span-aligned overlap: range span equals content length, ranges stay contiguous.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSpanAmendTestNg, BuildRichText009, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    // TEXT[0,5) + SYMBOL[5,7) + TEXT[7,12)
    ClearAndLoadSpans(pattern, {
        MakeSpan(SpanItemType::NORMAL, u"hello", 5),
        MakeSpan(SpanItemType::SYMBOL, u"  ", 7),
        MakeSpan(SpanItemType::NORMAL, u"world", 12),
    });

    auto result = pattern->BuildRichText(3, 9);
    ASSERT_EQ(result.spans.size(), 3u);
    EXPECT_EQ(result.spans[0].content, "lo");
    EXPECT_EQ(result.spans[0].range.start, 0);
    EXPECT_EQ(result.spans[0].range.end, 2);
    EXPECT_EQ(result.spans[1].content, "  ");
    EXPECT_EQ(result.spans[1].range.start, 2);
    EXPECT_EQ(result.spans[1].range.end, 4);
    EXPECT_EQ(result.spans[2].content, "wo");
    EXPECT_EQ(result.spans[2].range.start, 4);
    EXPECT_EQ(result.spans[2].range.end, 6);
}

/**
 * @tc.name: GetLeftRichTextOfCursor001
 * @tc.desc: Cursor at start/middle/end, selection, number=0, clamp-to-0, negative number entry guard.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSpanAmendTestNg, GetLeftRichTextOfCursor001, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    ClearAndLoadSpans(pattern, {
        MakeSpan(SpanItemType::NORMAL, u"hello", 5),
        MakeImageSpan(u"[img]", 6),
        MakeSpan(SpanItemType::NORMAL, u"world", 11),
    });

    // Cursor at end, number=6 -> BuildRichText(5,11)
    pattern->caretPosition_ = 11;
    pattern->textSelector_.Update(-1, -1);
    auto result = pattern->GetLeftRichTextOfCursor(6);
    ASSERT_EQ(result.spans.size(), 2u);
    EXPECT_EQ(result.spans[0].content, "[img]");
    EXPECT_EQ(result.spans[1].content, "world");

    // Cursor at middle, number=3 -> BuildRichText(2,5)
    pattern->caretPosition_ = 5;
    result = pattern->GetLeftRichTextOfCursor(3);
    ASSERT_EQ(result.spans.size(), 1u);
    EXPECT_EQ(result.spans[0].content, "llo");

    // Cursor at start, number=5 -> BuildRichText(0,0) empty
    pattern->caretPosition_ = 0;
    result = pattern->GetLeftRichTextOfCursor(5);
    EXPECT_TRUE(result.IsEmpty());

    // Selection [2,5): end=min(2,5)=2, number=3 -> start=max(0,-1)=0 -> BuildRichText(0,2)
    pattern->caretPosition_ = 5;
    pattern->textSelector_.Update(2, 5);
    result = pattern->GetLeftRichTextOfCursor(3);
    ASSERT_EQ(result.spans.size(), 1u);
    EXPECT_EQ(result.spans[0].content, "he");

    // number exceeds end -> start clamped to 0, no fallback to full text
    result = pattern->GetLeftRichTextOfCursor(10);
    ASSERT_EQ(result.spans.size(), 1u);
    EXPECT_EQ(result.spans[0].content, "he");

    // number=0 -> BuildRichText(end,end) empty
    pattern->caretPosition_ = 5;
    pattern->textSelector_.Update(-1, -1);
    result = pattern->GetLeftRichTextOfCursor(0);
    EXPECT_TRUE(result.IsEmpty());

    // negative number -> entry guard returns empty
    result = pattern->GetLeftRichTextOfCursor(-3);
    EXPECT_TRUE(result.IsEmpty());
}

/**
 * @tc.name: GetRightRichTextOfCursor001
 * @tc.desc: Cursor at start/middle/end and selection; entry guards symmetric to GetLeft.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSpanAmendTestNg, GetRightRichTextOfCursor001, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    ClearAndLoadSpans(pattern, {
        MakeSpan(SpanItemType::NORMAL, u"hello", 5),
        MakeImageSpan(u"[img]", 6),
        MakeSpan(SpanItemType::NORMAL, u"world", 11),
    });

    // Cursor at start, number=5 -> BuildRichText(0,5)
    pattern->caretPosition_ = 0;
    pattern->textSelector_.Update(-1, -1);
    auto result = pattern->GetRightRichTextOfCursor(5);
    ASSERT_EQ(result.spans.size(), 1u);
    EXPECT_EQ(result.spans[0].content, "hello");

    // Cursor at middle, number=3 -> BuildRichText(5,8)
    pattern->caretPosition_ = 5;
    result = pattern->GetRightRichTextOfCursor(3);
    ASSERT_EQ(result.spans.size(), 2u);
    EXPECT_EQ(result.spans[0].content, "[img]");
    EXPECT_EQ(result.spans[1].content, "wo");

    // Cursor at end, number=5 -> end clamped to textLen(11) -> BuildRichText(11,11) returns empty
    pattern->caretPosition_ = 11;
    result = pattern->GetRightRichTextOfCursor(5);
    EXPECT_TRUE(result.IsEmpty());

    // Selection [2,5): start=max(2,5)=5 -> BuildRichText(5,8)
    pattern->caretPosition_ = 2;
    pattern->textSelector_.Update(2, 5);
    result = pattern->GetRightRichTextOfCursor(3);
    ASSERT_EQ(result.spans.size(), 2u);
    EXPECT_EQ(result.spans[0].content, "[img]");
}
} // namespace OHOS::Ace::NG

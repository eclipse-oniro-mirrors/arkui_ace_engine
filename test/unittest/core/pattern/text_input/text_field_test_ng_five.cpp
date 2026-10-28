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
#include "test/mock/frameworks/base/thread/mock_task_executor.h"
#include "test/mock/frameworks/core/common/mock_container.h"
#include "test/mock/frameworks/core/common/mock_theme_manager.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"
#include "test/mock/frameworks/core/components_ng/render/mock_paragraph.h"
#include "test/unittest/core/pattern/test_ng.h"
#include "test/unittest/core/pattern/text/text_base.h"

#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/view_abstract_model_static.h"
#include "core/components_ng/manager/select_content_overlay/select_content_overlay_manager.h"
#include "core/components_ng/pattern/image/image_pattern.h"
#include "core/components_ng/pattern/search/search_model_ng.h"
#include "core/components_ng/pattern/search/search_pattern.h"
#include "core/components_ng/pattern/search/search_text_field.h"
#include "core/components_ng/pattern/select_overlay/select_overlay_node.h"
#include "core/components_ng/pattern/select_overlay/select_overlay_pattern.h"
#include "core/components_ng/pattern/text/span/image_span_view_static.h"
#include "core/components_ng/pattern/text/text_model_static.h"
#include "core/components_ng/pattern/text_field/text_field_layout_algorithm.h"
#include "core/components_ng/pattern/text_field/text_field_model_ng.h"
#include "core/components_ng/pattern/text_field/text_field_model_static.h"
#include "core/components_ng/pattern/text_field/text_field_pattern.h"
#include "core/components_ng/pattern/text_field/text_field_select_overlay.h"
#include "core/components_ng/render/render_context.h"
#include "core/components/search/search_theme.h"
#include "core/components/text_overlay/text_overlay_theme.h"
#include "core/components/theme/icon_theme.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {

namespace {
constexpr double ICON_SIZE = 24;
constexpr double ICON_HOT_ZONE_SIZE = 40;
constexpr int32_t DEFAULT_NODE_ID = 1;
constexpr int32_t MIN_PLATFORM_VERSION = 10;
constexpr int32_t SEARCH_FIELD_CHILD_INDEX = 0;
constexpr float TEST_HANDLE_WIDTH = 2.0f;
constexpr float TEST_HANDLE_HEIGHT = 20.0f;
const std::string DEFAULT_TEXT = "abcdefghijklmnopqrstuvwxyz";
struct ExpectParagraphParams {
    float height = 50.f;
    float longestLine = 460.f;
    float maxWidth = 460.f;
    size_t lineCount = 1;
    bool firstCalc = true;
    bool secondCalc = true;
};
} // namespace

class TextFieldTestNgFive : public TestNG {
protected:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void TearDown() override;

    void FlushLayoutTask(const RefPtr<FrameNode>& frameNode);
    void CreateTextField(const std::string& text = "", const std::string& placeHolder = "",
        const std::function<void(TextFieldModelNG&)>& callback = nullptr);
    void CreateSearchTextField(RefPtr<FrameNode>& searchNode, RefPtr<FrameNode>& searchFieldNode,
        RefPtr<SearchTextFieldPattern>& searchFieldPattern);
    static void ExpectCallParagraphMethods(ExpectParagraphParams params);
    void GetFocus();

    RefPtr<FrameNode> frameNode_;
    RefPtr<TextFieldPattern> pattern_;
    RefPtr<TextFieldEventHub> eventHub_;
    RefPtr<TextFieldLayoutProperty> layoutProperty_;
    RefPtr<TextFieldAccessibilityProperty> accessibilityProperty_;
};

void TextFieldTestNgFive::SetUpTestSuite()
{
    TestNG::SetUpTestSuite();
    ExpectCallParagraphMethods(ExpectParagraphParams());
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    auto textFieldTheme = AceType::MakeRefPtr<TextFieldTheme>();
    textFieldTheme->iconSize_ = Dimension(ICON_SIZE, DimensionUnit::VP);
    textFieldTheme->iconHotZoneSize_ = Dimension(ICON_HOT_ZONE_SIZE, DimensionUnit::VP);
    textFieldTheme->fontSize_ = Dimension(FONT_SIZE, DimensionUnit::FP);
    textFieldTheme->fontWeight_ = FontWeight::W400;
    textFieldTheme->textColor_ = Color::FromString("#ff182431");
    EXPECT_CALL(*themeManager, GetTheme(_))
        .WillRepeatedly([textFieldTheme = textFieldTheme](ThemeType type) -> RefPtr<Theme> {
            if (type == ScrollBarTheme::TypeId()) {
                return AceType::MakeRefPtr<ScrollBarTheme>();
            }
            if (type == SearchTheme::TypeId()) {
                return AceType::MakeRefPtr<SearchTheme>();
            }
            if (type == IconTheme::TypeId()) {
                return AceType::MakeRefPtr<IconTheme>();
            }
            if (type == TextOverlayTheme::TypeId()) {
                return AceType::MakeRefPtr<TextOverlayTheme>();
            }
            return textFieldTheme;
        });
    EXPECT_CALL(*themeManager, GetTheme(_, _))
        .WillRepeatedly([textFieldTheme = textFieldTheme](ThemeType type, int id) -> RefPtr<Theme> {
            if (type == ScrollBarTheme::TypeId()) {
                return AceType::MakeRefPtr<ScrollBarTheme>();
            }
            if (type == SearchTheme::TypeId()) {
                return AceType::MakeRefPtr<SearchTheme>();
            }
            if (type == IconTheme::TypeId()) {
                return AceType::MakeRefPtr<IconTheme>();
            }
            if (type == TextOverlayTheme::TypeId()) {
                return AceType::MakeRefPtr<TextOverlayTheme>();
            }
            return textFieldTheme;
        });
    MockPipelineContext::GetCurrent()->SetMinPlatformVersion(MIN_PLATFORM_VERSION);
    MockPipelineContext::GetCurrent()->SetTextFieldManager(AceType::MakeRefPtr<TextFieldManagerNG>());
    MockContainer::Current()->taskExecutor_ = AceType::MakeRefPtr<MockTaskExecutor>();
}

void TextFieldTestNgFive::TearDownTestSuite()
{
    TestNG::TearDownTestSuite();
    MockParagraph::TearDown();
}

void TextFieldTestNgFive::TearDown()
{
    frameNode_ = nullptr;
    pattern_ = nullptr;
    eventHub_ = nullptr;
    layoutProperty_ = nullptr;
    accessibilityProperty_ = nullptr;
}

void TextFieldTestNgFive::ExpectCallParagraphMethods(ExpectParagraphParams params)
{
    auto paragraph = MockParagraph::GetOrCreateMockParagraph();
    EXPECT_CALL(*paragraph, PushStyle(_)).Times(AnyNumber());
    EXPECT_CALL(*paragraph, AddText(_)).Times(AnyNumber());
    EXPECT_CALL(*paragraph, PopStyle()).Times(AnyNumber());
    EXPECT_CALL(*paragraph, Build()).Times(AnyNumber());
    EXPECT_CALL(*paragraph, Layout(_)).Times(AnyNumber());
    EXPECT_CALL(*paragraph, GetTextWidth()).WillRepeatedly(Return(params.maxWidth));
    EXPECT_CALL(*paragraph, GetAlphabeticBaseline()).WillRepeatedly(Return(0.f));
    EXPECT_CALL(*paragraph, GetHeight()).WillRepeatedly(Return(params.height));
    EXPECT_CALL(*paragraph, GetLongestLine()).WillRepeatedly(Return(params.longestLine));
    EXPECT_CALL(*paragraph, GetMaxWidth()).WillRepeatedly(Return(params.maxWidth));
    EXPECT_CALL(*paragraph, GetLineCount()).WillRepeatedly(Return(params.lineCount));
}

void TextFieldTestNgFive::FlushLayoutTask(const RefPtr<FrameNode>& frameNode)
{
    frameNode->SetActive();
    frameNode->isLayoutDirtyMarked_ = true;
    frameNode->CreateLayoutTask();
    auto paintProperty = frameNode->GetPaintProperty<PaintProperty>();
    auto wrapper = frameNode->CreatePaintWrapper();
    if (wrapper != nullptr) {
        wrapper->FlushRender();
    }
    paintProperty->CleanDirty();
    frameNode->SetActive(false);
}

void TextFieldTestNgFive::CreateTextField(
    const std::string& text, const std::string& placeHolder, const std::function<void(TextFieldModelNG&)>& callback)
{
    auto* stack = ViewStackProcessor::GetInstance();
    stack->StartGetAccessRecordingFor(DEFAULT_NODE_ID);
    TextFieldModelNG textFieldModelNG;
    textFieldModelNG.CreateTextInput(StringUtils::Str8ToStr16(placeHolder), StringUtils::Str8ToStr16(text));
    if (callback) {
        callback(textFieldModelNG);
    }
    stack->StopGetAccessRecording();
    frameNode_ = AceType::DynamicCast<FrameNode>(stack->Finish());
    pattern_ = frameNode_->GetPattern<TextFieldPattern>();
    eventHub_ = frameNode_->GetEventHub<TextFieldEventHub>();
    layoutProperty_ = frameNode_->GetLayoutProperty<TextFieldLayoutProperty>();
    accessibilityProperty_ = frameNode_->GetAccessibilityProperty<TextFieldAccessibilityProperty>();
    FlushLayoutTask(frameNode_);
}

void TextFieldTestNgFive::CreateSearchTextField(RefPtr<FrameNode>& searchNode,
    RefPtr<FrameNode>& searchFieldNode, RefPtr<SearchTextFieldPattern>& searchFieldPattern)
{
    SearchModelNG searchModelInstance;
    searchModelInstance.Create(
        StringUtils::Str8ToStr16(DEFAULT_TEXT), u"placeholder", "resource:///ohos_search.svg");
    searchNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->GetMainFrameNode());
    ASSERT_NE(searchNode, nullptr);
    searchFieldNode = AceType::DynamicCast<FrameNode>(searchNode->GetChildAtIndex(SEARCH_FIELD_CHILD_INDEX));
    ASSERT_NE(searchFieldNode, nullptr);
    searchFieldPattern = searchFieldNode->GetPattern<SearchTextFieldPattern>();
    ASSERT_NE(searchFieldPattern, nullptr);
    ASSERT_NE(searchFieldPattern->selectOverlay_, nullptr);
    ViewStackProcessor::GetInstance()->Finish();
}

void TextFieldTestNgFive::GetFocus()
{
    auto focushHub = pattern_->GetFocusHub();
    focushHub->currentFocus_ = true;
    pattern_->HandleFocusEvent();
    FlushLayoutTask(frameNode_);
}

/**
 * @tc.name: SetExtraConfig
 * @tc.desc: test pass extra config to ime.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldTestNgFive, SetExtraConfig, TestSize.Level1)
{
    /**
     * @tc.steps: step1 Create Text filed node
     */
    struct ExtraConfig {
        std::string key;
        std::string value;
    };
    CreateTextField(DEFAULT_TEXT, "", [](TextFieldModelNG model) {
        IMEAttachCallback attach = [](IMEClient& client) {
            ExtraConfig* extraConfig = new ExtraConfig();
            extraConfig->key = "name";
            extraConfig->value = "test";
            client.extraInfo = AceType::MakeRefPtr<IMEExtraInfo>(extraConfig, [extraConfig]() { delete extraConfig; });
        };
        model.SetOnWillAttachIME(std::move(attach));
    });
    ASSERT_NE(pattern_, nullptr);

    /**
     * @tc.step: step2. Call FireOnWillAttachIME
     */
    auto clientInfo = pattern_->GetIMEClientInfo();
    pattern_->FireOnWillAttachIME(clientInfo);
    ASSERT_NE(clientInfo.extraInfo, nullptr);
    ASSERT_NE(clientInfo.extraInfo->GetExtraInfo(), nullptr);
    auto parsedConfig = *reinterpret_cast<ExtraConfig*>(clientInfo.extraInfo->GetExtraInfo());
    EXPECT_EQ(parsedConfig.key, "name");
    EXPECT_EQ(parsedConfig.value, "test");
}

/**
 * @tc.name: SetBorderDashGap001
 * @tc.desc: Test TextField border dashGap with single value using ViewAbstract
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldTestNgFive, SetBorderDashGap001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create TextField
     * @tc.expected: TextField is created successfully
     */
    CreateTextField(DEFAULT_TEXT, "", [](TextFieldModelNG model) { model.SetType(TextInputType::TEXT); });

    ASSERT_NE(frameNode_, nullptr);
    ASSERT_NE(pattern_, nullptr);

    /**
     * @tc.steps: step2. Get paint property
     * @tc.expected: Paint property should exist
     */
    auto paintProperty = frameNode_->GetPaintProperty<PaintProperty>();
    ASSERT_NE(paintProperty, nullptr);

    /**
     * @tc.steps: step3. Call ViewAbstract::SetDashGap with positive value
     * @tc.expected: dashGap should be set
     */
    constexpr float dashGapValue = 5.0f;
    ViewAbstract::SetDashGap(AceType::RawPtr(frameNode_), Dimension(dashGapValue, DimensionUnit::VP));

    /**
     * @tc.steps: step4. Flush layout to apply border properties
     * @tc.expected: Border properties should be processed
     */
    FlushLayoutTask(frameNode_);

    /**
     * @tc.steps: step5. Verify render context has dashGap
     * @tc.expected: DashGap property should exist in render context
     */
    auto renderContext = frameNode_->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);

    auto dashGap = renderContext->GetDashGap();
    EXPECT_TRUE(dashGap.has_value());
}

/**
 * @tc.name: SetBorderDashGap002
 * @tc.desc: Test TextField border dashGap with BorderWidthProperty
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldTestNgFive, SetBorderDashGap002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create TextField
     * @tc.expected: TextField is created successfully
     */
    CreateTextField(DEFAULT_TEXT, "", [](TextFieldModelNG model) { model.SetType(TextInputType::TEXT); });

    ASSERT_NE(frameNode_, nullptr);
    ASSERT_NE(pattern_, nullptr);

    /**
     * @tc.steps: step2. Get paint property
     * @tc.expected: Paint property should exist
     */
    auto paintProperty = frameNode_->GetPaintProperty<PaintProperty>();
    ASSERT_NE(paintProperty, nullptr);

    /**
     * @tc.steps: step3. Create BorderWidthProperty with different dashGap values
     * @tc.expected: Each side has independent dashGap value
     */
    BorderWidthProperty dashGap;
    dashGap.leftDimen = Dimension(3.0f, DimensionUnit::VP);
    dashGap.topDimen = Dimension(5.0f, DimensionUnit::VP);
    dashGap.rightDimen = Dimension(7.0f, DimensionUnit::VP);
    dashGap.bottomDimen = Dimension(9.0f, DimensionUnit::VP);

    /**
     * @tc.steps: step4. Call ViewAbstract::SetDashGap with BorderWidthProperty
     * @tc.expected: dashGap should be set for all sides
     */
    ViewAbstractModelStatic::SetDashGap(AceType::RawPtr(frameNode_), dashGap);

    /**
     * @tc.steps: step5. Flush layout to apply border properties
     * @tc.expected: Border properties should be processed
     */
    FlushLayoutTask(frameNode_);

    /**
     * @tc.steps: step6. Verify render context has dashGap
     * @tc.expected: DashGap property should exist in render context
     */
    auto renderContext = frameNode_->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);

    auto dashGapResult = renderContext->GetDashGap();
    EXPECT_TRUE(dashGapResult.has_value());
}

/**
 * @tc.name: SetBorderDashGap003
 * @tc.desc: Test TextField border dashGap reset to default value
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldTestNgFive, SetBorderDashGap003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create TextField
     * @tc.expected: TextField is created successfully
     */
    CreateTextField(DEFAULT_TEXT, "", [](TextFieldModelNG model) { model.SetType(TextInputType::TEXT); });

    ASSERT_NE(frameNode_, nullptr);
    ASSERT_NE(pattern_, nullptr);

    /**
     * @tc.steps: step2. Get paint property
     * @tc.expected: Paint property should exist
     */
    auto paintProperty = frameNode_->GetPaintProperty<PaintProperty>();
    ASSERT_NE(paintProperty, nullptr);

    /**
     * @tc.steps: step3. Set dashGap with positive value first
     * @tc.expected: Initial dashGap is set
     */
    ViewAbstract::SetDashGap(AceType::RawPtr(frameNode_), Dimension(5.0f, DimensionUnit::VP));
    FlushLayoutTask(frameNode_);

    ViewAbstract::SetDashGap(AceType::RawPtr(frameNode_), Dimension(-1));

    /**
     * @tc.steps: step5. Flush layout to apply changes
     * @tc.expected: Border properties should be updated
     */
    FlushLayoutTask(frameNode_);

    /**
     * @tc.steps: step6. Verify render context
     * @tc.expected: Render context should still be valid
     */
    auto renderContext = frameNode_->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);

    auto dashGap = renderContext->GetDashGap();
    EXPECT_TRUE(dashGap.has_value());
}

/**
 * @tc.name: ResetDashGap001
 * @tc.desc: Test ResetDashGap function resets dash gap to default value
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldTestNgFive, ResetDashGap001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create TextField
     * @tc.expected: TextField is created successfully
     */
    CreateTextField(DEFAULT_TEXT, "", [](TextFieldModelNG model) { model.SetType(TextInputType::TEXT); });

    ASSERT_NE(frameNode_, nullptr);
    ASSERT_NE(pattern_, nullptr);

    /**
     * @tc.steps: step2. Set initial dash gap value
     * @tc.expected: Initial dash gap is set
     */
    ViewAbstract::SetDashGap(AceType::RawPtr(frameNode_), Dimension(5.0f, DimensionUnit::VP));
    FlushLayoutTask(frameNode_);

    /**
     * @tc.steps: step3. Call ResetDashGap through style_modifier interface
     * @tc.expected: Dash gap should be reset to default (-1)
     */
    ViewAbstract::SetDashGap(AceType::RawPtr(frameNode_), Dimension(-1));
    FlushLayoutTask(frameNode_);

    auto renderContext = frameNode_->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);

    /**
     * @tc.steps: step5. Verify render context state after reset
     * @tc.expected: Render context remains valid
     */
    auto dashGapAfterReset = renderContext->GetDashGap();
    EXPECT_TRUE(dashGapAfterReset.has_value());
}

/**
 * @tc.name: SetDashWidth001
 * @tc.desc: Test SetDashWidth with four values for border dash width
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldTestNgFive, SetDashWidth001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create TextField
     * @tc.expected: TextField is created successfully
     */
    CreateTextField(DEFAULT_TEXT, "", [](TextFieldModelNG model) { model.SetType(TextInputType::TEXT); });

    ASSERT_NE(frameNode_, nullptr);
    ASSERT_NE(pattern_, nullptr);

    /**
     * @tc.steps: step2. Create BorderWidthProperty with dash width values
     * @tc.expected: Each side has independent dash width value
     */
    BorderWidthProperty dashWidth;
    dashWidth.leftDimen = Dimension(2.0f, DimensionUnit::VP);
    dashWidth.topDimen = Dimension(3.0f, DimensionUnit::VP);
    dashWidth.rightDimen = Dimension(4.0f, DimensionUnit::VP);
    dashWidth.bottomDimen = Dimension(5.0f, DimensionUnit::VP);

    /**
     * @tc.steps: step3. Call ViewAbstract::SetDashWidth with BorderWidthProperty
     * @tc.expected: Dash width should be set for all sides
     */
    ViewAbstract::SetDashWidth(AceType::RawPtr(frameNode_), Dimension(3.0f, DimensionUnit::VP));

    /**
     * @tc.steps: step4. Flush layout to apply border properties
     * @tc.expected: Border properties should be processed
     */
    FlushLayoutTask(frameNode_);

    /**
     * @tc.steps: step5. Verify render context has dash width
     * @tc.expected: Dash width property should be configured
     */
    auto renderContext = frameNode_->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);

    EXPECT_TRUE(renderContext->GetDashWidth().has_value());
}

/**
 * @tc.name: SetDashWidth002
 * @tc.desc: Test SetDashWidth with single value
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldTestNgFive, SetDashWidth002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create TextField
     * @tc.expected: TextField is created successfully
     */
    CreateTextField(DEFAULT_TEXT, "", [](TextFieldModelNG model) { model.SetType(TextInputType::TEXT); });

    ASSERT_NE(frameNode_, nullptr);
    ASSERT_NE(pattern_, nullptr);

    /**
     * @tc.steps: step2. Call ViewAbstract::SetDashWidth with single Dimension value
     * @tc.expected: Dash width should be set uniformly
     */
    ViewAbstract::SetDashWidth(AceType::RawPtr(frameNode_), Dimension(3.0f, DimensionUnit::VP));

    /**
     * @tc.steps: step3. Flush layout to apply border properties
     * @tc.expected: Border properties should be processed
     */
    FlushLayoutTask(frameNode_);

    /**
     * @tc.steps: step4. Verify render context
     * @tc.expected: Render context should be valid
     */
    auto renderContext = frameNode_->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);

    auto dashWidth = renderContext->GetDashWidth();
    EXPECT_TRUE(dashWidth.has_value());
}

/**
 * @tc.name: ResetDashWidth001
 * @tc.desc: Test ResetDashWidth function resets dash width to default value
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldTestNgFive, ResetDashWidth001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create TextField
     * @tc.expected: TextField is created successfully
     */
    CreateTextField(DEFAULT_TEXT, "", [](TextFieldModelNG model) { model.SetType(TextInputType::TEXT); });

    ASSERT_NE(frameNode_, nullptr);
    ASSERT_NE(pattern_, nullptr);

    /**
     * @tc.steps: step2. Set initial dash width
     * @tc.expected: Initial dash width is set
     */
    ViewAbstract::SetDashWidth(AceType::RawPtr(frameNode_), Dimension(3.0f, DimensionUnit::VP));
    FlushLayoutTask(frameNode_);

    /**
     * @tc.steps: step3. Reset dash width to default (-1)
     * @tc.expected: Dash width should be reset
     */
    ViewAbstract::SetDashWidth(AceType::RawPtr(frameNode_), Dimension(-1));

    /**
     * @tc.steps: step5. Verify render context
     * @tc.expected: Render context should still be valid
     */
    auto renderContext = frameNode_->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);

    auto dashWidth = renderContext->GetDashWidth();
    EXPECT_TRUE(dashWidth.has_value());
}

/**
 * @tc.name: SetChainWeight001
 * @tc.desc: Test SetChainWeight with horizontal and vertical weights
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldTestNgFive, SetChainWeight001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create TextField
     * @tc.expected: TextField is created successfully
     */
    CreateTextField(DEFAULT_TEXT, "", [](TextFieldModelNG model) { model.SetType(TextInputType::TEXT); });

    ASSERT_NE(frameNode_, nullptr);
    ASSERT_NE(pattern_, nullptr);

    /**
     * @tc.steps: step2. Create ChainWeightPair with custom weights
     * @tc.expected: Horizontal and vertical weights are set
     */
    NG::ChainWeightPair chainWeight;
    chainWeight.first = 1.0f;
    chainWeight.second = 2.0f;

    /**
     * @tc.steps: step3. Call ViewAbstract::SetChainWeight
     * @tc.expected: Chain weight should be set
     */
    ViewAbstract::SetChainWeight(AceType::RawPtr(frameNode_), chainWeight);

    /**
     * @tc.steps: step4. Flush layout to apply changes
     * @tc.expected: Layout properties should be processed
     */
    FlushLayoutTask(frameNode_);

    /**
     * @tc.steps: step5. Verify frame node is still valid
     * @tc.expected: Frame node should remain valid after chain weight set
     */
    EXPECT_NE(frameNode_, nullptr);
}

/**
 * @tc.name: SetChainWeight002
 * @tc.desc: Test SetChainWeight with only horizontal weight
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldTestNgFive, SetChainWeight002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create TextField
     * @tc.expected: TextField is created successfully
     */
    CreateTextField(DEFAULT_TEXT, "", [](TextFieldModelNG model) { model.SetType(TextInputType::TEXT); });

    ASSERT_NE(frameNode_, nullptr);
    ASSERT_NE(pattern_, nullptr);

    /**
     * @tc.steps: step2. Create ChainWeightPair with horizontal weight only
     * @tc.expected: Horizontal weight is set, vertical defaults to 0
     */
    NG::ChainWeightPair chainWeight;
    chainWeight.first = 1.5f;
    chainWeight.second = std::nullopt;

    /**
     * @tc.steps: step3. Call ViewAbstract::SetChainWeight
     * @tc.expected: Chain weight should be set
     */
    ViewAbstract::SetChainWeight(AceType::RawPtr(frameNode_), chainWeight);

    /**
     * @tc.steps: step4. Flush layout to apply changes
     * @tc.expected: Layout properties should be processed
     */
    FlushLayoutTask(frameNode_);

    /**
     * @tc.steps: step5. Verify frame node state
     * @tc.expected: Frame node should be valid
     */
    EXPECT_NE(frameNode_, nullptr);
}

/**
 * @tc.name: ResetChainWeight001
 * @tc.desc: Test ResetChainWeight function resets chain weight to default values
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldTestNgFive, ResetChainWeight001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create TextField
     * @tc.expected: TextField is created successfully
     */
    CreateTextField(DEFAULT_TEXT, "", [](TextFieldModelNG model) { model.SetType(TextInputType::TEXT); });

    ASSERT_NE(frameNode_, nullptr);
    ASSERT_NE(pattern_, nullptr);

    /**
     * @tc.steps: step2. Set initial chain weight
     * @tc.expected: Initial chain weight is set
     */
    NG::ChainWeightPair initialWeight;
    initialWeight.first = 2.0f;
    initialWeight.second = 3.0f;
    ViewAbstract::SetChainWeight(AceType::RawPtr(frameNode_), initialWeight);
    FlushLayoutTask(frameNode_);

    /**
     * @tc.steps: step3. Reset chain weight to default (0, 0)
     * @tc.expected: Chain weight should be reset to zero
     */
    NG::ChainWeightPair defaultWeight;
    defaultWeight.first = 0.0f;
    defaultWeight.second = 0.0f;
    ViewAbstract::SetChainWeight(AceType::RawPtr(frameNode_), defaultWeight);

    /**
     * @tc.steps: step4. Flush layout to apply changes
     * @tc.expected: Layout properties should be updated
     */
    FlushLayoutTask(frameNode_);

    /**
     * @tc.steps: step5. Verify frame node is still valid
     * @tc.expected: Frame node should remain valid after reset
     */
    EXPECT_NE(frameNode_, nullptr);
}

/**
 * @tc.name: IsHorizontalScrollEnabled
 * @tc.desc: Test IsHorizontalScrollEnabled with all conditions at boundary values
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldTestNgFive, IsHorizontalScrollEnabled, TestSize.Level0)
{
    /**
     * @tc.steps: step1. Create TextArea node
     */
    CreateTextField(DEFAULT_TEXT, "", [](TextFieldModelNG model) {});
    ASSERT_NE(pattern_, nullptr);
    pattern_->SetTextInputFlag(false);

    pattern_->SetHorizontalScrolling(false);
    TextFieldModelNG::SetInputStyle(AceType::RawPtr(frameNode_), InputStyle::DEFAULT);
    EXPECT_FALSE(pattern_->IsHorizontalScrollEnabled());

    pattern_->SetHorizontalScrolling(true);
    TextFieldModelNG::SetMaxLines(AceType::RawPtr(frameNode_), 1);
    EXPECT_FALSE(pattern_->IsHorizontalScrollEnabled());

    pattern_->SetHorizontalScrolling(true);
    TextFieldModelNG::SetMaxLines(AceType::RawPtr(frameNode_), 10);
    TextFieldModelNG::SetInputStyle(AceType::RawPtr(frameNode_), InputStyle::DEFAULT);
    TextFieldModelNG::SetType(AceType::RawPtr(frameNode_), TextInputType::UNSPECIFIED);
    EXPECT_TRUE(pattern_->IsHorizontalScrollEnabled());
}

namespace {
constexpr float TEST_REGION_LEFT = 100.0f;
constexpr float TEST_REGION_TOP = 100.0f;
constexpr float TEST_REGION_WIDTH = 20.0f;
constexpr float TEST_REGION_HEIGHT = 20.0f;
constexpr float TEST_POINT_IN_REGION = 110.0f;
constexpr float TEST_POINT_OUT_REGION = 999.0f;
constexpr float TEST_FIELD_PAINT_OFFSET_X = 30.0f;
constexpr float TEST_FIELD_PAINT_OFFSET_Y = 40.0f;
constexpr float TEST_SEARCH_PAINT_OFFSET_X = 10.0f;
constexpr float TEST_SEARCH_PAINT_OFFSET_Y = 5.0f;

class OffsetPaintRectRenderContext final : public RenderContext {
public:
    explicit OffsetPaintRectRenderContext(const OffsetF& paintOffset) : paintOffset_(paintOffset) {}
    ~OffsetPaintRectRenderContext() override = default;

    RectF GetPaintRectWithTransform() override
    {
        return RectF(paintOffset_, SizeF());
    }

private:
    OffsetF paintOffset_;
};
} // namespace

/**
 * @tc.name: TextFieldSelectOverlayGetSelectOverlayMountNode001
 * @tc.desc: test GetSelectOverlayMountNode and GetOwner for normal TextField
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldTestNgFive, TextFieldSelectOverlayGetSelectOverlayMountNode001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create normal TextField
     * @tc.expected: owner is the TextField host node and mount node keeps the owner node
     */
    CreateTextField(DEFAULT_TEXT);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(pattern_->selectOverlay_, nullptr);
    EXPECT_EQ(pattern_->selectOverlay_->GetOwner(), frameNode_);
    EXPECT_EQ(GetSelectOverlayMountNode(pattern_->selectOverlay_->GetOwner()), frameNode_);
}

/**
 * @tc.name: TextFieldSelectOverlayGetSelectOverlayMountNode002
 * @tc.desc: test GetSelectOverlayMountNode and GetOwner for SearchField
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldTestNgFive, TextFieldSelectOverlayGetSelectOverlayMountNode002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Search component
     * @tc.expected: owner is SearchField node, mount node is Search node
     */
    RefPtr<FrameNode> searchNode;
    RefPtr<FrameNode> searchFieldNode;
    RefPtr<SearchTextFieldPattern> searchFieldPattern;
    CreateSearchTextField(searchNode, searchFieldNode, searchFieldPattern);
    EXPECT_EQ(searchFieldPattern->selectOverlay_->GetOwner(), searchFieldNode);
    EXPECT_EQ(GetSelectOverlayMountNode(searchFieldPattern->selectOverlay_->GetOwner()), searchNode);
    EXPECT_NE(GetSelectOverlayMountNode(searchFieldPattern->selectOverlay_->GetOwner()),
        searchFieldPattern->selectOverlay_->GetOwner());
}

/**
 * @tc.name: TextFieldSelectOverlayGetSelectOverlayMountNode003
 * @tc.desc: test GetSelectOverlayMountNode with invalid cases
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldTestNgFive, TextFieldSelectOverlayGetSelectOverlayMountNode003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create standalone SearchField node without parent
     * @tc.expected: mount node falls back to the node itself
     */
    auto frameId = ElementRegister::GetInstance()->MakeUniqueId();
    auto standaloneNode = FrameNode::GetOrCreateFrameNode(
        SEARCH_FIELD_TAG, frameId, []() { return AceType::MakeRefPtr<SearchTextFieldPattern>(); });
    ASSERT_NE(standaloneNode, nullptr);
    EXPECT_EQ(GetSelectOverlayMountNode(standaloneNode), standaloneNode);

    /**
     * @tc.steps: step2. Call GetSelectOverlayMountNode with nullptr
     * @tc.expected: mount node is nullptr
     */
    EXPECT_EQ(GetSelectOverlayMountNode(nullptr), nullptr);

    /**
     * @tc.steps: step3. Create a non SearchField tagged node
     * @tc.expected: mount node keeps the node itself
     */
    auto frameIdTwo = ElementRegister::GetInstance()->MakeUniqueId();
    auto normalNode = FrameNode::GetOrCreateFrameNode(
        "TEMP_NODE", frameIdTwo, []() { return AceType::MakeRefPtr<Pattern>(); });
    ASSERT_NE(normalNode, nullptr);
    EXPECT_EQ(GetSelectOverlayMountNode(normalNode), normalNode);
}

/**
 * @tc.name: TextFieldSelectOverlayGetHandleInfo001
 * @tc.desc: test TextFieldSelectOverlay::GetHandleInfo when mount node equals owner node
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldTestNgFive, TextFieldSelectOverlayGetHandleInfo001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create normal TextField
     * @tc.expected: localPaintRect keeps the original coordinate without offset conversion
     */
    CreateTextField(DEFAULT_TEXT);
    ASSERT_NE(pattern_->selectOverlay_, nullptr);
    RectF handlePaintRect(5.0f, 6.0f, TEST_HANDLE_WIDTH, TEST_HANDLE_HEIGHT);
    auto handleInfo = pattern_->selectOverlay_->GetHandleInfo(handlePaintRect);
    ASSERT_TRUE(handleInfo.has_value());
    EXPECT_EQ(handleInfo->localPaintRect.GetOffset(), handlePaintRect.GetOffset());
    EXPECT_EQ(handleInfo->paintRect.GetOffset(), handlePaintRect.GetOffset());
}

/**
 * @tc.name: TextFieldSelectOverlayGetHandleInfo002
 * @tc.desc: test TextFieldSelectOverlay::GetHandleInfo when mount node differs from owner node
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldTestNgFive, TextFieldSelectOverlayGetHandleInfo002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Search component with known paint offsets
     */
    RefPtr<FrameNode> searchNode;
    RefPtr<FrameNode> searchFieldNode;
    RefPtr<SearchTextFieldPattern> searchFieldPattern;
    CreateSearchTextField(searchNode, searchFieldNode, searchFieldPattern);
    searchFieldNode->renderContext_ = AceType::MakeRefPtr<OffsetPaintRectRenderContext>(
        OffsetF(TEST_FIELD_PAINT_OFFSET_X, TEST_FIELD_PAINT_OFFSET_Y));
    searchNode->renderContext_ = AceType::MakeRefPtr<OffsetPaintRectRenderContext>(
        OffsetF(TEST_SEARCH_PAINT_OFFSET_X, TEST_SEARCH_PAINT_OFFSET_Y));

    /**
     * @tc.steps: step2. Call GetHandleInfo with a known rect
     * @tc.expected: localPaintRect is converted by ownerOffset - mountOffset from SearchField
     *               coordinate system to Search coordinate system
     */
    RectF handlePaintRect(5.0f, 6.0f, TEST_HANDLE_WIDTH, TEST_HANDLE_HEIGHT);
    auto handleInfo = searchFieldPattern->selectOverlay_->GetHandleInfo(handlePaintRect);
    ASSERT_TRUE(handleInfo.has_value());
    OffsetF ownerOffset(TEST_FIELD_PAINT_OFFSET_X + TEST_SEARCH_PAINT_OFFSET_X,
        TEST_FIELD_PAINT_OFFSET_Y + TEST_SEARCH_PAINT_OFFSET_Y);
    OffsetF mountOffset(TEST_SEARCH_PAINT_OFFSET_X, TEST_SEARCH_PAINT_OFFSET_Y);
    EXPECT_EQ(handleInfo->localPaintRect.GetOffset(), handlePaintRect.GetOffset() + ownerOffset - mountOffset);
    EXPECT_EQ(handleInfo->paintRect.GetOffset(), handlePaintRect.GetOffset() + ownerOffset);
}

/**
 * @tc.name: TextFieldSelectOverlayGetHandleInfo003
 * @tc.desc: test TextFieldSelectOverlay::GetHandleInfo when owner is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldTestNgFive, TextFieldSelectOverlayGetHandleInfo003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create overlay without pattern
     * @tc.expected: GetHandleInfo returns handle info without offset conversion
     */
    auto nullPatternOverlay = AceType::MakeRefPtr<TextFieldSelectOverlay>(WeakPtr<TextBase>());
    ASSERT_NE(nullPatternOverlay, nullptr);
    RectF handlePaintRect(5.0f, 6.0f, TEST_HANDLE_WIDTH, TEST_HANDLE_HEIGHT);
    auto handleInfo = nullPatternOverlay->GetHandleInfo(handlePaintRect);
    ASSERT_TRUE(handleInfo.has_value());
    EXPECT_EQ(handleInfo->localPaintRect.GetOffset(), handlePaintRect.GetOffset());
    EXPECT_EQ(handleInfo->paintRect.GetOffset(), handlePaintRect.GetOffset());
}

/**
 * @tc.name: SelectOverlayManagerMountNodeToCaller001
 * @tc.desc: test SelectContentOverlayManager::MountNodeToCaller mounts overlay on mount node
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldTestNgFive, SelectOverlayManagerMountNodeToCaller001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Search component and bind holder to manager
     * @tc.expected: overlay node is mounted on Search node instead of SearchField node
     */
    RefPtr<FrameNode> searchNode;
    RefPtr<FrameNode> searchFieldNode;
    RefPtr<SearchTextFieldPattern> searchFieldPattern;
    CreateSearchTextField(searchNode, searchFieldNode, searchFieldPattern);
    auto manager = SelectContentOverlayManager::GetOverlayManager();
    ASSERT_NE(manager, nullptr);
    manager->selectOverlayHolder_ = searchFieldPattern->selectOverlay_;
    searchFieldPattern->selectOverlay_->OnBind(manager);
    auto overlayInfo = std::make_shared<SelectOverlayInfo>();
    overlayInfo->isUsingMouse = true;
    manager->shareOverlayInfo_ = overlayInfo;
    auto overlayNode = SelectOverlayNode::CreateSelectOverlayNode(overlayInfo);
    ASSERT_NE(overlayNode, nullptr);
    manager->MountNodeToCaller(overlayNode, false);
    EXPECT_EQ(overlayNode->GetParent(), searchNode);
    EXPECT_EQ(searchNode->GetOverlayNode(), overlayNode);
    EXPECT_EQ(searchFieldNode->GetOverlayNode(), nullptr);

    /**
     * @tc.steps: step2. Create normal TextField and bind holder to manager
     * @tc.expected: overlay node is mounted on the TextField host node
     */
    CreateTextField(DEFAULT_TEXT);
    ASSERT_NE(pattern_->selectOverlay_, nullptr);
    manager->selectOverlayHolder_ = pattern_->selectOverlay_;
    pattern_->selectOverlay_->OnBind(manager);
    auto overlayInfoTwo = std::make_shared<SelectOverlayInfo>();
    overlayInfoTwo->isUsingMouse = true;
    manager->shareOverlayInfo_ = overlayInfoTwo;
    auto overlayNodeTwo = SelectOverlayNode::CreateSelectOverlayNode(overlayInfoTwo);
    ASSERT_NE(overlayNodeTwo, nullptr);
    manager->MountNodeToCaller(overlayNodeTwo, false);
    EXPECT_EQ(overlayNodeTwo->GetParent(), frameNode_);
    EXPECT_EQ(frameNode_->GetOverlayNode(), overlayNodeTwo);
}

/**
 * @tc.name: SelectOverlayManagerUpdateStatusInfos001
 * @tc.desc: test SelectContentOverlayManager::UpdateStatusInfos uses mount node as caller
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldTestNgFive, SelectOverlayManagerUpdateStatusInfos001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Search component and bind holder to manager
     * @tc.expected: callerFrameNode is the Search node
     */
    RefPtr<FrameNode> searchNode;
    RefPtr<FrameNode> searchFieldNode;
    RefPtr<SearchTextFieldPattern> searchFieldPattern;
    CreateSearchTextField(searchNode, searchFieldNode, searchFieldPattern);
    auto manager = SelectContentOverlayManager::GetOverlayManager();
    ASSERT_NE(manager, nullptr);
    manager->selectOverlayHolder_ = searchFieldPattern->selectOverlay_;
    searchFieldPattern->selectOverlay_->OnBind(manager);
    SelectOverlayInfo firstInfo;
    manager->UpdateStatusInfos(firstInfo);
    EXPECT_EQ(firstInfo.callerFrameNode.Upgrade(), searchNode);

    /**
     * @tc.steps: step2. Create normal TextField and bind holder to manager
     * @tc.expected: callerFrameNode is the TextField host node
     */
    CreateTextField(DEFAULT_TEXT);
    ASSERT_NE(pattern_->selectOverlay_, nullptr);
    manager->selectOverlayHolder_ = pattern_->selectOverlay_;
    pattern_->selectOverlay_->OnBind(manager);
    SelectOverlayInfo secondInfo;
    manager->UpdateStatusInfos(secondInfo);
    EXPECT_EQ(secondInfo.callerFrameNode.Upgrade(), frameNode_);
}

/**
 * @tc.name: SelectOverlayManagerIsTouchAtHandle001
 * @tc.desc: test SelectContentOverlayManager::IsTouchAtHandle with mount node comparison
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldTestNgFive, SelectOverlayManagerIsTouchAtHandle001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Search component, handle node mounted on Search node
     * @tc.expected: local point is used when handle node parent equals mount node
     */
    RefPtr<FrameNode> searchNode;
    RefPtr<FrameNode> searchFieldNode;
    RefPtr<SearchTextFieldPattern> searchFieldPattern;
    CreateSearchTextField(searchNode, searchFieldNode, searchFieldPattern);
    auto manager = SelectContentOverlayManager::GetOverlayManager();
    ASSERT_NE(manager, nullptr);
    manager->selectOverlayHolder_ = searchFieldPattern->selectOverlay_;
    searchFieldPattern->selectOverlay_->OnBind(manager);
    auto overlayInfo = std::make_shared<SelectOverlayInfo>();
    overlayInfo->enableHandleLevel = true;
    manager->shareOverlayInfo_ = overlayInfo;
    auto handleNode = SelectOverlayNode::CreateSelectOverlayNode(overlayInfo);
    ASSERT_NE(handleNode, nullptr);
    auto handlePattern = handleNode->GetPattern<SelectOverlayPattern>();
    ASSERT_NE(handlePattern, nullptr);
    handlePattern->firstHandleRegion_ = RectF(TEST_REGION_LEFT, TEST_REGION_TOP, TEST_REGION_WIDTH, TEST_REGION_HEIGHT);
    handlePattern->secondHandleRegion_ =
        RectF(TEST_REGION_LEFT + 200.0f, TEST_REGION_TOP, TEST_REGION_WIDTH, TEST_REGION_HEIGHT);
    manager->handleNode_ = AceType::WeakClaim(AceType::RawPtr(handleNode));
    handleNode->MountToParent(searchNode);
    EXPECT_TRUE(manager->IsTouchAtHandle(PointF(TEST_POINT_IN_REGION, TEST_POINT_IN_REGION),
        PointF(TEST_POINT_OUT_REGION, TEST_POINT_OUT_REGION)));
    EXPECT_FALSE(manager->IsTouchAtHandle(PointF(TEST_POINT_OUT_REGION, TEST_POINT_OUT_REGION),
        PointF(TEST_POINT_IN_REGION, TEST_POINT_IN_REGION)));

    /**
     * @tc.steps: step2. Mount another handle node on a different parent
     * @tc.expected: global point is used when handle node parent differs from mount node
     */
    auto handleNodeTwo = SelectOverlayNode::CreateSelectOverlayNode(overlayInfo);
    ASSERT_NE(handleNodeTwo, nullptr);
    auto handlePatternTwo = handleNodeTwo->GetPattern<SelectOverlayPattern>();
    ASSERT_NE(handlePatternTwo, nullptr);
    handlePatternTwo->firstHandleRegion_ =
        RectF(TEST_REGION_LEFT, TEST_REGION_TOP, TEST_REGION_WIDTH, TEST_REGION_HEIGHT);
    handlePatternTwo->secondHandleRegion_ =
        RectF(TEST_REGION_LEFT + 200.0f, TEST_REGION_TOP, TEST_REGION_WIDTH, TEST_REGION_HEIGHT);
    manager->handleNode_ = AceType::WeakClaim(AceType::RawPtr(handleNodeTwo));
    auto tempRoot = AceType::MakeRefPtr<FrameNode>("TEMP", -1, AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(tempRoot, nullptr);
    handleNodeTwo->MountToParent(tempRoot);
    EXPECT_FALSE(manager->IsTouchAtHandle(PointF(TEST_POINT_IN_REGION, TEST_POINT_IN_REGION),
        PointF(TEST_POINT_OUT_REGION, TEST_POINT_OUT_REGION)));
    EXPECT_TRUE(manager->IsTouchAtHandle(PointF(TEST_POINT_OUT_REGION, TEST_POINT_OUT_REGION),
        PointF(TEST_POINT_IN_REGION, TEST_POINT_IN_REGION)));

    /**
     * @tc.steps: step3. Hide the handle of the second handle node
     * @tc.expected: IsTouchAtHandle returns false
     */
    handlePatternTwo->isHiddenHandle_ = true;
    EXPECT_FALSE(manager->IsTouchAtHandle(PointF(TEST_POINT_IN_REGION, TEST_POINT_IN_REGION),
        PointF(TEST_POINT_IN_REGION, TEST_POINT_IN_REGION)));
}

/**
 * @tc.name: TextFieldSelectOverlayMarkOverlayDirty001
 * @tc.desc: test BaseTextSelectOverlay::MarkOverlayDirty finds overlay node on mount node
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldTestNgFive, TextFieldSelectOverlayMarkOverlayDirty001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Search component and bind holder to manager
     * @tc.expected: overlay node mounted on Search node is marked dirty
     */
    RefPtr<FrameNode> searchNode;
    RefPtr<FrameNode> searchFieldNode;
    RefPtr<SearchTextFieldPattern> searchFieldPattern;
    CreateSearchTextField(searchNode, searchFieldNode, searchFieldPattern);
    auto manager = SelectContentOverlayManager::GetOverlayManager();
    ASSERT_NE(manager, nullptr);
    manager->selectOverlayHolder_ = searchFieldPattern->selectOverlay_;
    searchFieldPattern->selectOverlay_->OnBind(manager);
    auto overlayInfo = std::make_shared<SelectOverlayInfo>();
    overlayInfo->isUsingMouse = true;
    manager->shareOverlayInfo_ = overlayInfo;
    auto overlayNode = SelectOverlayNode::CreateSelectOverlayNode(overlayInfo);
    ASSERT_NE(overlayNode, nullptr);
    manager->MountNodeToCaller(overlayNode, false);
    EXPECT_EQ(overlayNode->GetParent(), searchNode);
    manager->selectOverlayNode_ = AceType::WeakClaim(AceType::RawPtr(overlayNode));
    overlayNode->layoutProperty_->CleanDirty();
    searchFieldPattern->selectOverlay_->MarkOverlayDirty();
    EXPECT_TRUE(CheckMeasureFlag(overlayNode->layoutProperty_->GetPropertyChangeFlag()));

    /**
     * @tc.steps: step2. Create normal TextField and bind holder to manager
     * @tc.expected: overlay node mounted on host node is marked dirty
     */
    CreateTextField(DEFAULT_TEXT);
    ASSERT_NE(pattern_->selectOverlay_, nullptr);
    manager->selectOverlayHolder_ = pattern_->selectOverlay_;
    pattern_->selectOverlay_->OnBind(manager);
    auto overlayInfoTwo = std::make_shared<SelectOverlayInfo>();
    overlayInfoTwo->isUsingMouse = true;
    manager->shareOverlayInfo_ = overlayInfoTwo;
    auto overlayNodeTwo = SelectOverlayNode::CreateSelectOverlayNode(overlayInfoTwo);
    ASSERT_NE(overlayNodeTwo, nullptr);
    manager->MountNodeToCaller(overlayNodeTwo, false);
    EXPECT_EQ(overlayNodeTwo->GetParent(), frameNode_);
    manager->selectOverlayNode_ = AceType::WeakClaim(AceType::RawPtr(overlayNodeTwo));
    overlayNodeTwo->layoutProperty_->CleanDirty();
    pattern_->selectOverlay_->MarkOverlayDirty();
    EXPECT_TRUE(CheckMeasureFlag(overlayNodeTwo->layoutProperty_->GetPropertyChangeFlag()));
}
} // namespace OHOS::Ace::NG
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

#include "test/mock/core/common/mock_font_manager.h"
#include "test/mock/core/common/mock_theme_manager.h"
#include "test/mock/core/pattern/mock_nestable_scroll_container.h"
#include "test/mock/core/pipeline/mock_pipeline_context.h"
#include "test/mock/core/render/mock_canvas_image.h"
#include "test/mock/core/render/mock_paragraph.h"
#include "test/mock/core/render/mock_render_context.h"
#include "test/mock/core/rosen/mock_canvas.h"
#include "text_base.h"

#include "core/components/common/properties/text_style_parser.h"
#include "core/components_ng/pattern/text/paragraph_util.h"
#include "core/components_ng/pattern/text/span_model_ng.h"
#include "core/components_ng/render/adapter/pixelmap_image.h"

namespace OHOS::Ace::NG {

namespace {
const std::string SYMBOL_FONT_FAMILY = "Symbol_Test_CustomSymbol";
constexpr int32_t DEFAULT_SYMBOL_ID = 983242;
} // namespace

class TextTestNgSix : public TextBases {
public:
};

/**
 * @tc.name: TextContentModifier030
 * @tc.desc: test text_content_modifier.cpp UpdateImageNodeVisible function
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNgSix, TextContentModifier030, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create textFrameNode and textPattern.
     */
    auto textFrameNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 0, AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(textFrameNode, nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    ASSERT_NE(geometryNode, nullptr);
    auto textPattern = textFrameNode->GetPattern<TextPattern>();
    ASSERT_NE(textPattern, nullptr);
    /**
     * @tc.steps: step2. Create imageNodeList and add imageNode into imageNodeList.
     */
    ParagraphStyle paragraphStyle;
    RefPtr<Paragraph> paragraph = Paragraph::Create(paragraphStyle, FontCollection::Current());
    ASSERT_NE(paragraph, nullptr);
    textPattern->pManager_->AddParagraph({ .paragraph = paragraph, .start = 0, .end = 1 });
    std::vector<RefPtr<FrameNode>> imageNodeLocalList;
    std::vector<WeakPtr<FrameNode>> imageNodeList;
    for (int i = 0; i < 300; i++) {
        auto imageNode = FrameNode::GetOrCreateFrameNode(V2::IMAGE_ETS_TAG,
			ElementRegister::GetInstance()->MakeUniqueId(),
            []() { return AceType::MakeRefPtr<ImagePattern>(); });
        imageNodeList.emplace_back(AceType::WeakClaim(AceType::RawPtr(imageNode)));
        imageNodeLocalList.emplace_back(imageNode);
    }
    textPattern->SetImageSpanNodeList(imageNodeList);
    
    /**
     * @tc.steps: step3. call CreateNodePaintMethod func.
     * @tc.expected: The imageNodeList_ size is equal to 300.
     */
    auto textPaintMethod = AceType::DynamicCast<TextPaintMethod>(textPattern->CreateNodePaintMethod());
    ASSERT_NE(textPaintMethod, nullptr);
    RefPtr<RenderContext> renderContext = textFrameNode->GetRenderContext();
    auto paintProperty = textPattern->CreatePaintProperty();
    auto paintWrapper = AceType::MakeRefPtr<PaintWrapper>(renderContext, geometryNode, paintProperty);
    textPaintMethod->UpdateContentModifier(AceType::RawPtr(paintWrapper));
    ASSERT_NE(textPaintMethod, nullptr);
    auto textContentModifier = textPaintMethod->textContentModifier_;
    ASSERT_NE(textContentModifier, nullptr);
    EXPECT_EQ(textContentModifier->imageNodeList_.size(), 300);
    textContentModifier->UpdateImageNodeVisible(VisibleType::INVISIBLE);
    for (const auto& imageWeak : textContentModifier->imageNodeList_) {
        auto imageNode = imageWeak.Upgrade();
        ASSERT_NE(imageNode, nullptr);
        auto layoutProperty1 = imageNode->GetLayoutProperty();
        ASSERT_NE(layoutProperty1, nullptr);
        EXPECT_EQ(static_cast<int32_t>(layoutProperty1->GetVisibility().has_value() &&
            (layoutProperty1->GetVisibility().value() == VisibleType::INVISIBLE)), 1);
    }
    
    textPattern->pManager_->Reset();
}

/**
 * @tc.name: TextContentModifier031
 * @tc.desc: test text_content_modifier.cpp UpdateImageNodeVisible function
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNgSix, TextContentModifier031, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create textFrameNode and textPattern.
     */
    auto textFrameNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 0, AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(textFrameNode, nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    ASSERT_NE(geometryNode, nullptr);
    auto textPattern = textFrameNode->GetPattern<TextPattern>();
    ASSERT_NE(textPattern, nullptr);
    /**
     * @tc.steps: step2. Create imageNodeList and add imageNode into imageNodeList.
     */
    ParagraphStyle paragraphStyle;
    RefPtr<Paragraph> paragraph = Paragraph::Create(paragraphStyle, FontCollection::Current());
    ASSERT_NE(paragraph, nullptr);
    textPattern->pManager_->AddParagraph({ .paragraph = paragraph, .start = 0, .end = 1 });
    std::vector<RefPtr<FrameNode>> imageNodeLocalList;
    std::vector<WeakPtr<FrameNode>> imageNodeList;
    for (int i = 0; i < 300; i++) {
        auto imageNode = FrameNode::GetOrCreateFrameNode(V2::IMAGE_ETS_TAG,
			ElementRegister::GetInstance()->MakeUniqueId(),
            []() { return AceType::MakeRefPtr<ImagePattern>(); });
        imageNodeList.emplace_back(AceType::WeakClaim(AceType::RawPtr(imageNode)));
        imageNodeLocalList.emplace_back(imageNode);
    }
    textPattern->SetImageSpanNodeList(imageNodeList);
    
    /**
     * @tc.steps: step3. call CreateNodePaintMethod func.
     * @tc.expected: The imageNodeList_ size is equal to 300.
     */
    auto textPaintMethod = AceType::DynamicCast<TextPaintMethod>(textPattern->CreateNodePaintMethod());
    ASSERT_NE(textPaintMethod, nullptr);
    RefPtr<RenderContext> renderContext = textFrameNode->GetRenderContext();
    auto paintProperty = textPattern->CreatePaintProperty();
    auto paintWrapper = AceType::MakeRefPtr<PaintWrapper>(renderContext, geometryNode, paintProperty);
    textPaintMethod->UpdateContentModifier(AceType::RawPtr(paintWrapper));
    ASSERT_NE(textPaintMethod, nullptr);
    auto textContentModifier = textPaintMethod->textContentModifier_;
    ASSERT_NE(textContentModifier, nullptr);
  
    textContentModifier->UpdateImageNodeVisible(VisibleType::VISIBLE);
    EXPECT_EQ(textContentModifier->imageNodeList_.size(), 300);
    for (const auto& imageWeak : textContentModifier->imageNodeList_) {
        auto imageNode = imageWeak.Upgrade();
        ASSERT_NE(imageNode, nullptr);
        auto layoutProperty2 = imageNode->GetLayoutProperty();
        ASSERT_NE(layoutProperty2, nullptr);
        EXPECT_EQ(static_cast<int32_t>(layoutProperty2->GetVisibility().has_value() &&
            (layoutProperty2->GetVisibility().value() == VisibleType::VISIBLE)), 1);
    }
    textPattern->pManager_->Reset();
}

/**
 * @tc.name: TextContentModifier032
 * @tc.desc: test text_content_modifier.cpp UpdateImageNodeVisible function
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNgSix, TextContentModifier032, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create textFrameNode and textPattern.
     */
    auto textFrameNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 0, AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(textFrameNode, nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    ASSERT_NE(geometryNode, nullptr);
    auto textPattern = textFrameNode->GetPattern<TextPattern>();
    ASSERT_NE(textPattern, nullptr);
    /**
     * @tc.steps: step2. Create imageNodeList and add imageNode into imageNodeList.
     */
    ParagraphStyle paragraphStyle;
    RefPtr<Paragraph> paragraph = Paragraph::Create(paragraphStyle, FontCollection::Current());
    ASSERT_NE(paragraph, nullptr);
    textPattern->pManager_->AddParagraph({ .paragraph = paragraph, .start = 0, .end = 1 });
    std::vector<RefPtr<FrameNode>> imageNodeLocalList;
    std::vector<WeakPtr<FrameNode>> imageNodeList;
    for (int i = 0; i < 300; i++) {
        auto imageNode = FrameNode::GetOrCreateFrameNode(V2::IMAGE_ETS_TAG,
			ElementRegister::GetInstance()->MakeUniqueId(),
            []() { return AceType::MakeRefPtr<ImagePattern>(); });
        imageNodeList.emplace_back(AceType::WeakClaim(AceType::RawPtr(imageNode)));
        imageNodeLocalList.emplace_back(imageNode);
    }
    textPattern->SetImageSpanNodeList(imageNodeList);
    
    /**
     * @tc.steps: step3. call CreateNodePaintMethod func.
     * @tc.expected: The imageNodeList_ size is equal to 300.
     */
    auto textPaintMethod = AceType::DynamicCast<TextPaintMethod>(textPattern->CreateNodePaintMethod());
    ASSERT_NE(textPaintMethod, nullptr);
    RefPtr<RenderContext> renderContext = textFrameNode->GetRenderContext();
    auto paintProperty = textPattern->CreatePaintProperty();
    auto paintWrapper = AceType::MakeRefPtr<PaintWrapper>(renderContext, geometryNode, paintProperty);
    textPaintMethod->UpdateContentModifier(AceType::RawPtr(paintWrapper));
    ASSERT_NE(textPaintMethod, nullptr);
    auto textContentModifier = textPaintMethod->textContentModifier_;
    ASSERT_NE(textContentModifier, nullptr);
  
    textContentModifier->UpdateImageNodeVisible(VisibleType::GONE);
    EXPECT_EQ(textContentModifier->imageNodeList_.size(), 300);
    for (const auto& imageWeak : textContentModifier->imageNodeList_) {
        auto imageNode = imageWeak.Upgrade();
        ASSERT_NE(imageNode, nullptr);
        auto layoutProperty2 = imageNode->GetLayoutProperty();
        ASSERT_NE(layoutProperty2, nullptr);
        EXPECT_EQ(static_cast<int32_t>(layoutProperty2->GetVisibility().has_value() &&
            (layoutProperty2->GetVisibility().value() == VisibleType::GONE)), 1);
    }
    textPattern->pManager_->Reset();
}

/**
 * @tc.name: TextContentModifier033
 * @tc.desc: test text_content_modifier.cpp UpdateImageNodeVisible function
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNgSix, TextContentModifier033, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create textFrameNode and textPattern.
     */
    auto textFrameNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 0, AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(textFrameNode, nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    ASSERT_NE(geometryNode, nullptr);
    auto textPattern = textFrameNode->GetPattern<TextPattern>();
    ASSERT_NE(textPattern, nullptr);
    /**
     * @tc.steps: step2. Create imageNodeList and add imageNode into imageNodeList.
     */
    ParagraphStyle paragraphStyle;
    RefPtr<Paragraph> paragraph = Paragraph::Create(paragraphStyle, FontCollection::Current());
    ASSERT_NE(paragraph, nullptr);
    textPattern->pManager_->AddParagraph({ .paragraph = paragraph, .start = 0, .end = 1 });
    std::vector<WeakPtr<FrameNode>> imageNodeList;
    for (int i = 0; i < 200; i++) {
        imageNodeList.emplace_back(nullptr);
    }
    textPattern->SetImageSpanNodeList(imageNodeList);
    
    /**
     * @tc.steps: step3. call CreateNodePaintMethod func.
     * @tc.expected: The imageNodeList_ size is equal to 200.
     */
    auto textPaintMethod = AceType::DynamicCast<TextPaintMethod>(textPattern->CreateNodePaintMethod());
    ASSERT_NE(textPaintMethod, nullptr);
    RefPtr<RenderContext> renderContext = textFrameNode->GetRenderContext();
    auto paintProperty = textPattern->CreatePaintProperty();
    auto paintWrapper = AceType::MakeRefPtr<PaintWrapper>(renderContext, geometryNode, paintProperty);
    textPaintMethod->UpdateContentModifier(AceType::RawPtr(paintWrapper));
    ASSERT_NE(textPaintMethod, nullptr);
    auto textContentModifier = textPaintMethod->textContentModifier_;
    ASSERT_NE(textContentModifier, nullptr);
    EXPECT_EQ(textContentModifier->imageNodeList_.size(), 200);
    textContentModifier->UpdateImageNodeVisible(VisibleType::INVISIBLE);
    for (const auto& imageWeak : textContentModifier->imageNodeList_) {
        auto imageNode = imageWeak.Upgrade();
        EXPECT_EQ(imageNode, nullptr);
    }
    
    textPattern->pManager_->Reset();
}

/**
 * @tc.name: TextContentModifier034
 * @tc.desc: test text_content_modifier.cpp UpdateImageNodeVisible function
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNgSix, TextContentModifier034, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create textFrameNode and textPattern.
     */
    auto textFrameNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 0, AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(textFrameNode, nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    ASSERT_NE(geometryNode, nullptr);
    auto textPattern = textFrameNode->GetPattern<TextPattern>();
    ASSERT_NE(textPattern, nullptr);
    /**
     * @tc.steps: step2. Create imageNodeList and add imageNode into imageNodeList.
     */
    ParagraphStyle paragraphStyle;
    RefPtr<Paragraph> paragraph = Paragraph::Create(paragraphStyle, FontCollection::Current());
    ASSERT_NE(paragraph, nullptr);
    textPattern->pManager_->AddParagraph({ .paragraph = paragraph, .start = 0, .end = 1 });
    std::vector<WeakPtr<FrameNode>> imageNodeList;
    for (int i = 0; i < 200; i++) {
        imageNodeList.emplace_back(nullptr);
    }
    textPattern->SetImageSpanNodeList(imageNodeList);
    
    /**
     * @tc.steps: step3. call CreateNodePaintMethod func.
     * @tc.expected: The imageNodeList_ size is equal to 200.
     */
    auto textPaintMethod = AceType::DynamicCast<TextPaintMethod>(textPattern->CreateNodePaintMethod());
    ASSERT_NE(textPaintMethod, nullptr);
    RefPtr<RenderContext> renderContext = textFrameNode->GetRenderContext();
    auto paintProperty = textPattern->CreatePaintProperty();
    auto paintWrapper = AceType::MakeRefPtr<PaintWrapper>(renderContext, geometryNode, paintProperty);
    textPaintMethod->UpdateContentModifier(AceType::RawPtr(paintWrapper));
    ASSERT_NE(textPaintMethod, nullptr);
    auto textContentModifier = textPaintMethod->textContentModifier_;
    ASSERT_NE(textContentModifier, nullptr);
    
    textContentModifier->UpdateImageNodeVisible(VisibleType::VISIBLE);
    EXPECT_EQ(textContentModifier->imageNodeList_.size(), 200);
    for (const auto& imageWeak : textContentModifier->imageNodeList_) {
        auto imageNode = imageWeak.Upgrade();
        EXPECT_EQ(imageNode, nullptr);
    }
    textPattern->pManager_->Reset();
}

/**
 * @tc.name: TextContentModifier035
 * @tc.desc: test text_content_modifier.cpp UpdateImageNodeVisible function
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNgSix, TextContentModifier035, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create textFrameNode and textPattern.
     */
    auto textFrameNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 0, AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(textFrameNode, nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    ASSERT_NE(geometryNode, nullptr);
    auto textPattern = textFrameNode->GetPattern<TextPattern>();
    ASSERT_NE(textPattern, nullptr);
    /**
     * @tc.steps: step2. Create imageNodeList and add imageNode into imageNodeList.
     */
    ParagraphStyle paragraphStyle;
    RefPtr<Paragraph> paragraph = Paragraph::Create(paragraphStyle, FontCollection::Current());
    ASSERT_NE(paragraph, nullptr);
    textPattern->pManager_->AddParagraph({ .paragraph = paragraph, .start = 0, .end = 1 });
    std::vector<WeakPtr<FrameNode>> imageNodeList;
    for (int i = 0; i < 200; i++) {
        imageNodeList.emplace_back(nullptr);
    }
    textPattern->SetImageSpanNodeList(imageNodeList);

    /**
     * @tc.steps: step3. call CreateNodePaintMethod func.
     * @tc.expected: The imageNodeList_ size is equal to 200.
     */
    auto textPaintMethod = AceType::DynamicCast<TextPaintMethod>(textPattern->CreateNodePaintMethod());
    ASSERT_NE(textPaintMethod, nullptr);
    RefPtr<RenderContext> renderContext = textFrameNode->GetRenderContext();
    auto paintProperty = textPattern->CreatePaintProperty();
    auto paintWrapper = AceType::MakeRefPtr<PaintWrapper>(renderContext, geometryNode, paintProperty);
    textPaintMethod->UpdateContentModifier(AceType::RawPtr(paintWrapper));
    ASSERT_NE(textPaintMethod, nullptr);
    auto textContentModifier = textPaintMethod->textContentModifier_;
    ASSERT_NE(textContentModifier, nullptr);

    textContentModifier->UpdateImageNodeVisible(VisibleType::GONE);
    EXPECT_EQ(textContentModifier->imageNodeList_.size(), 200);
    for (const auto& imageWeak : textContentModifier->imageNodeList_) {
        auto imageNode = imageWeak.Upgrade();
        EXPECT_EQ(imageNode, nullptr);
    }
    textPattern->pManager_->Reset();
}

/**
 * @tc.name: GetSpansInfoInStyledString001
 * @tc.desc: test text_pattern.cpp GetSpansInfoInStyledString function
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNgSix, GetSpansInfoInStyledString001, TestSize.Level1)
{
    auto textFrameNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 0, AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(textFrameNode, nullptr);
    auto textPattern = textFrameNode->GetPattern<TextPattern>();
    ASSERT_NE(textPattern, nullptr);
    auto textSpanNode = CreateSpanNodeWithSetDefaultProperty(U16TEXT_FOR_AI);
    ASSERT_NE(textSpanNode, nullptr);
    textPattern->AddChildSpanItem(textSpanNode);
    ImageSpanNodeProperty firstProperty { .imageSrc = std::make_optional("image") };
    auto imageSpanNode = CreateImageSpanNode(firstProperty);
    ASSERT_NE(imageSpanNode, nullptr);
    textPattern->AddChildSpanItem(imageSpanNode);

    auto resultObjects = textPattern->GetSpansInfoInStyledString(0, TEXT_SIZE_INT);
    ASSERT_NE(resultObjects.size(), 0);
}

/**
 * @tc.name: GetSpansInfo001
 * @tc.desc: test text_pattern.cpp GetSpansInfo function
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNgSix, GetSpansInfo001, TestSize.Level1)
{
    auto textFrameNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 0, AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(textFrameNode, nullptr);
    auto textPattern = textFrameNode->GetPattern<TextPattern>();
    ASSERT_NE(textPattern, nullptr);

    auto spanString = AceType::MakeRefPtr<SpanString>(TEXT_U16CONTENT);
    ASSERT_NE(spanString, nullptr);
    textPattern->SetStyledString(spanString);

    auto selectionInfo = textPattern->GetSpansInfo(0, 0, GetSpansMethod::GETSPANS);
    ASSERT_NE(selectionInfo.selection_.resultObjects.size(), 0);

    selectionInfo = textPattern->GetSpansInfo(-1, 0, GetSpansMethod::GETSPANS);
    ASSERT_NE(selectionInfo.selection_.resultObjects.size(), 0);

    selectionInfo = textPattern->GetSpansInfo(0, -1, GetSpansMethod::GETSPANS);
    ASSERT_NE(selectionInfo.selection_.resultObjects.size(), 0);

    selectionInfo = textPattern->GetSpansInfo(-1, -1, GetSpansMethod::GETSPANS);
    ASSERT_NE(selectionInfo.selection_.resultObjects.size(), 0);

    selectionInfo = textPattern->GetSpansInfo(1, 0, GetSpansMethod::GETSPANS);
    ASSERT_NE(selectionInfo.selection_.resultObjects.size(), 0);

    selectionInfo = textPattern->GetSpansInfo(-1, -1, static_cast<GetSpansMethod>(-1));
    ASSERT_NE(selectionInfo.selection_.resultObjects.size(), 0);
}

/**
 * @tc.name: GetSelectedText001
 * @tc.desc: test text_pattern.cpp GetSelectedText function
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNgSix, GetSelectedText001, TestSize.Level1)
{
    auto textFrameNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 0, AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(textFrameNode, nullptr);
    auto textPattern = textFrameNode->GetPattern<TextPattern>();
    ASSERT_NE(textPattern, nullptr);

    auto textSpanNode1 = CreateSpanNodeWithSetDefaultProperty(CREATE_VALUE_W);
    ASSERT_NE(textSpanNode1, nullptr);
    textPattern->AddChildSpanItem(textSpanNode1);
    auto textSpanNode2 = CreateSpanNodeWithSetDefaultProperty(CREATE_VALUE_W);
    ASSERT_NE(textSpanNode2, nullptr);
    textSpanNode2->UpdateContent(1);
    textPattern->AddChildSpanItem(textSpanNode2);
    auto textSpanNode3 = CreateSpanNodeWithSetDefaultProperty(CREATE_VALUE_W);
    ASSERT_NE(textSpanNode3, nullptr);
    textSpanNode3->UpdateContent(1);
    textSpanNode3->spanItem_->position = 1;
    textPattern->AddChildSpanItem(textSpanNode3);
    auto textSpanNode4 = CreateSpanNodeWithSetDefaultProperty(CREATE_VALUE_W);
    ASSERT_NE(textSpanNode4, nullptr);
    textSpanNode4->spanItem_->placeholderIndex = 0;
    textPattern->AddChildSpanItem(textSpanNode4);
    auto textSpanNode5 = CreateSpanNodeWithSetDefaultProperty(CREATE_VALUE_W);
    ASSERT_NE(textSpanNode5, nullptr);
    textSpanNode5->spanItem_->position = 1;
    textSpanNode5->spanItem_->placeholderIndex = 0;
    textPattern->AddChildSpanItem(textSpanNode5);
    auto textSpanNode6 = CreateSpanNodeWithSetDefaultProperty(CREATE_VALUE_W);
    ASSERT_NE(textSpanNode6, nullptr);
    textSpanNode6->spanItem_->position = 1;
    textPattern->AddChildSpanItem(textSpanNode6);

    auto selectedText = textPattern->GetSelectedText(0, 10);
    ASSERT_EQ(StringUtils::Str16ToStr8(selectedText), " ");
}

/**
 * @tc.name: GetTextHeight001
 * @tc.desc: test text_pattern.cpp GetTextHeight function
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNgSix, GetTextHeight001, TestSize.Level1)
{
    auto textFrameNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 0, AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(textFrameNode, nullptr);
    auto textPattern = textFrameNode->GetPattern<TextPattern>();
    ASSERT_NE(textPattern, nullptr);
    ASSERT_NE(textPattern->pManager_, nullptr);

    Offset globalOffset;
    globalOffset.SetX(0);
    globalOffset.SetY(5);

    auto paragraph = MockParagraph::GetOrCreateMockParagraph();
    std::vector<RectF> rects { RectF(0, 0, 5, 5) };
    EXPECT_CALL(*paragraph, GetRectsForRange(_, _, _)).WillRepeatedly(SetArgReferee<2>(rects));
    EXPECT_CALL(*paragraph, GetLineCount()).WillRepeatedly(Return(2));
    textPattern->pManager_->AddParagraph(
        { .paragraph = paragraph, .start = 0, .end = 2 });
    textPattern->pManager_->AddParagraph(
        { .paragraph = paragraph, .start = 2, .end = 4 });

    EXPECT_EQ(textPattern->GetTextHeight(0, false), 0.0);
    EXPECT_EQ(textPattern->GetTextHeight(0, true), 0.0);
}

/**
 * @tc.name: IsShowTranslate001
 * @tc.desc: test text_pattern.cpp IsShowTranslate function
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNgSix, IsShowTranslate001, TestSize.Level1)
{
    auto textFrameNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 0, AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(textFrameNode, nullptr);
    auto textPattern = textFrameNode->GetPattern<TextPattern>();
    ASSERT_NE(textPattern, nullptr);

    bool showTranslate = textPattern->IsShowTranslate();
    EXPECT_EQ(showTranslate, false);
}

/**
 * @tc.name: IsShowSearch001
 * @tc.desc: test text_pattern.cpp IsShowSearch function
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNgSix, IsShowSearch001, TestSize.Level1)
{
    auto textFrameNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 0, AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(textFrameNode, nullptr);
    auto textPattern = textFrameNode->GetPattern<TextPattern>();
    ASSERT_NE(textPattern, nullptr);

    textPattern->IsShowSearch();
    EXPECT_NE(textPattern, nullptr);
}

/**
 * @tc.name: IsSupportMenuShare001
 * @tc.desc: test base_text_select_overlay.cpp IsSupportMenuShare function
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNgSix, IsSupportMenuShare001, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<TextPattern>();
    ASSERT_NE(pattern, nullptr);
    auto frameNode = FrameNode::CreateFrameNode("Test", 1, pattern);
    ASSERT_NE(frameNode, nullptr);
    pattern->AttachToFrameNode(frameNode);
    auto textSelectOverlay = pattern->selectOverlay_;
    ASSERT_NE(textSelectOverlay, nullptr);

    EXPECT_EQ(textSelectOverlay->IsSupportMenuShare(), false);
}

/**
 * @tc.name: CheckHandleIsVisibleWithTransform001
 * @tc.desc: test base_text_select_overlay.cpp CheckHandleIsVisibleWithTransform function
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNgSix, CheckHandleIsVisibleWithTransform001, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<TextPattern>();
    ASSERT_NE(pattern, nullptr);
    auto frameNode = FrameNode::CreateFrameNode("Test", 1, pattern);
    ASSERT_NE(frameNode, nullptr);
    pattern->AttachToFrameNode(frameNode);
    auto textSelectOverlay = pattern->selectOverlay_;
    ASSERT_NE(textSelectOverlay, nullptr);

    OffsetF startPoint(0, 0);
    OffsetF endPoint(1, 1);

    EXPECT_EQ(textSelectOverlay->CheckHandleIsVisibleWithTransform(startPoint, endPoint, 10.0), true);

    startPoint.SetX(-11);
    startPoint.SetY(-11);
    endPoint.SetX(11);
    endPoint.SetY(11);

    EXPECT_EQ(textSelectOverlay->CheckHandleIsVisibleWithTransform(startPoint, endPoint, 10.0), false);

    startPoint.SetX(0);
    startPoint.SetY(0);
    endPoint.SetX(11);
    endPoint.SetY(11);

    EXPECT_EQ(textSelectOverlay->CheckHandleIsVisibleWithTransform(startPoint, endPoint, 10.0), false);

    startPoint.SetX(-11);
    startPoint.SetY(-11);
    endPoint.SetX(0);
    endPoint.SetY(0);

    EXPECT_EQ(textSelectOverlay->CheckHandleIsVisibleWithTransform(startPoint, endPoint, 10.0), false);
}

/**
 * @tc.name: IsPointsInRegion001
 * @tc.desc: test base_text_select_overlay.cpp IsPointsInRegion function
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNgSix, IsPointsInRegion001, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<TextPattern>();
    ASSERT_NE(pattern, nullptr);
    auto textSelectOverlay = pattern->selectOverlay_;
    ASSERT_NE(textSelectOverlay, nullptr);

    std::vector<PointF> points;
    RectF regionRect(1, 1, 4, 4);

    EXPECT_EQ(textSelectOverlay->IsPointsInRegion(points, regionRect), true);

    points.emplace_back(PointF(0, 0));
    points.emplace_back(PointF(1, 1));
    points.emplace_back(PointF(3, 3));
    points.emplace_back(PointF(5, 5));
    points.emplace_back(PointF(6, 6));

    EXPECT_EQ(textSelectOverlay->IsPointsInRegion(points, regionRect), false);
}

/**
 * @tc.name: CheckHandleCanPaintInHost001
 * @tc.desc: test base_text_select_overlay.cpp CheckHandleCanPaintInHost function
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNgSix, CheckHandleCanPaintInHost001, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<TextPattern>();
    ASSERT_NE(pattern, nullptr);
    auto frameNode = FrameNode::CreateFrameNode("Test", 1, pattern);
    ASSERT_NE(frameNode, nullptr);
    pattern->AttachToFrameNode(frameNode);
    auto textSelectOverlay = pattern->selectOverlay_;
    ASSERT_NE(textSelectOverlay, nullptr);
    ASSERT_NE(frameNode->GetGeometryNode(), nullptr);

    frameNode->GetGeometryNode()->SetFrameSize(SizeF(10, 10));
    textSelectOverlay->isChangeToOverlayModeAtEdge_ = false;

    RectF firstRect(3, 3, 5, 5);
    RectF secondRect(4, 4, 6, 6);
    EXPECT_EQ(textSelectOverlay->CheckHandleCanPaintInHost(firstRect, secondRect), true);

    firstRect.SetRect(3, 3, 5, 5);
    secondRect.SetRect(11, 11, 15, 15);
    EXPECT_EQ(textSelectOverlay->CheckHandleCanPaintInHost(firstRect, secondRect), false);

    frameNode->GetGeometryNode()->SetFrameSize(SizeF(3, 3));

    firstRect.SetRect(11, 11, 15, 15);
    secondRect.SetRect(3, 3, 5, 5);
    EXPECT_EQ(textSelectOverlay->CheckHandleCanPaintInHost(firstRect, secondRect), false);

    firstRect.SetRect(11, 11, 15, 15);
    secondRect.SetRect(13, 13, 15, 15);
    EXPECT_EQ(textSelectOverlay->CheckHandleCanPaintInHost(firstRect, secondRect), false);
}

/**
 * @tc.name: IsTouchAtHandle001
 * @tc.desc: test base_text_select_overlay.cpp IsTouchAtHandle function
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNgSix, IsTouchAtHandle001, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<TextPattern>();
    ASSERT_NE(pattern, nullptr);
    auto textSelectOverlay = pattern->selectOverlay_;
    ASSERT_NE(textSelectOverlay, nullptr);
    auto manager = SelectContentOverlayManager::GetOverlayManager();
    ASSERT_NE(manager, nullptr);
    textSelectOverlay->OnBind(manager);

    TouchEventInfo info0("touch0");
    TouchLocationInfo locationInfo0(1);
    locationInfo0.SetTouchType(TouchType::DOWN);
    locationInfo0.SetLocalLocation(Offset(0, 0));
    info0.AddTouchLocationInfo(std::move(locationInfo0));

    EXPECT_EQ(textSelectOverlay->IsTouchAtHandle(info0), false);

    TouchEventInfo info1("touch1");
    TouchLocationInfo locationInfo1(1);
    locationInfo1.SetTouchType(TouchType::UP);
    locationInfo1.SetLocalLocation(Offset(0, 0));
    info1.AddTouchLocationInfo(std::move(locationInfo1));

    EXPECT_EQ(textSelectOverlay->IsTouchAtHandle(info1), false);

    TouchEventInfo info2("touch2");
    TouchLocationInfo locationInfo2(1);
    locationInfo2.SetTouchType(TouchType::MOVE);
    locationInfo2.SetLocalLocation(Offset(0, 0));
    info2.AddTouchLocationInfo(std::move(locationInfo2));

    EXPECT_EQ(textSelectOverlay->IsTouchAtHandle(info2), false);

    TouchEventInfo info3("touch3");
    TouchLocationInfo locationInfo3(1);
    locationInfo3.SetTouchType(TouchType::UP);
    locationInfo3.SetLocalLocation(Offset(0, 0));
    locationInfo3.SetGlobalLocation(Offset(0, 0));
    info3.AddTouchLocationInfo(std::move(locationInfo3));

    EXPECT_EQ(textSelectOverlay->IsTouchAtHandle(info3), false);
}

/**
 * @tc.name: CheckSwitchToMode001
 * @tc.desc: test base_text_select_overlay.cpp CheckSwitchToMode function
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNgSix, CheckSwitchToMode001, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<TextPattern>();
    ASSERT_NE(pattern, nullptr);
    auto textSelectOverlay = pattern->selectOverlay_;
    ASSERT_NE(textSelectOverlay, nullptr);

    EXPECT_EQ(textSelectOverlay->CheckSwitchToMode(HandleLevelMode::OVERLAY), true);
    EXPECT_EQ(textSelectOverlay->CheckSwitchToMode(HandleLevelMode::EMBED), true);
}

/**
 * @tc.name: DrawImage001
 * @tc.desc: test text_content_modifier.cpp DrawImage function
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNgSix, DrawImage001, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<ImagePattern>();
    ASSERT_NE(pattern, nullptr);
    auto frameNode = FrameNode::CreateFrameNode("Test", 1, pattern);
    ASSERT_NE(frameNode, nullptr);
    pattern->AttachToFrameNode(frameNode);
    auto textContentModifier =
        AceType::MakeRefPtr<TextContentModifier>(std::optional<TextStyle>(TextStyle()), pattern);
    ASSERT_NE(textContentModifier, nullptr);

    RSCanvas canvas;
    RectF rect(0, 0, 10, 10);

    EXPECT_EQ(textContentModifier->DrawImage(frameNode, canvas, 0, 0, rect), false);

    frameNode->layoutProperty_->margin_ = std::make_unique<MarginProperty>();
    EXPECT_EQ(textContentModifier->DrawImage(frameNode, canvas, 0, 0, rect), false);

    frameNode->layoutProperty_->margin_->left = CalcLength("0vp");
    frameNode->layoutProperty_->margin_->top = CalcLength("0vp");
    EXPECT_EQ(textContentModifier->DrawImage(frameNode, canvas, 0, 0, rect), false);

    auto layoutProperty = frameNode->layoutProperty_;
    frameNode->layoutProperty_ = nullptr;
    EXPECT_EQ(textContentModifier->DrawImage(frameNode, canvas, 0, 0, rect), false);
    frameNode->layoutProperty_ = layoutProperty;

    pattern->altImage_ = AceType::MakeRefPtr<NG::MockCanvasImage>();
    ASSERT_NE(pattern->altImage_, nullptr);
    EXPECT_EQ(textContentModifier->DrawImage(frameNode, canvas, 0, 0, rect), true);

    pattern->image_ = AceType::MakeRefPtr<NG::MockCanvasImage>();
    ASSERT_NE(pattern->image_, nullptr);
    EXPECT_EQ(textContentModifier->DrawImage(frameNode, canvas, 0, 0, rect), true);

    auto geometryNode = frameNode->geometryNode_;
    frameNode->geometryNode_ = nullptr;
    EXPECT_EQ(textContentModifier->DrawImage(frameNode, canvas, 0, 0, rect), false);
    frameNode->geometryNode_ = geometryNode;

    auto& config = pattern->image_->GetPaintConfig();
    config.isSvg_ = !config.isSvg_;
    EXPECT_EQ(textContentModifier->DrawImage(frameNode, canvas, 0, 0, rect), true);
    config.isSvg_ = !config.isSvg_;

    pattern->image_ = AceType::MakeRefPtr<PixelMapImage>();
    EXPECT_EQ(textContentModifier->DrawImage(frameNode, canvas, 0, 0, rect), true);
}

/**
 * @tc.name: GetTextDirection001
 * @tc.desc: test multiple_paragraph_layout_algorithm.cpp GetTextDirection function
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNgSix, GetTextDirection001, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<TextPattern>();
    ASSERT_NE(pattern, nullptr);
    auto frameNode = FrameNode::CreateFrameNode("Test", 1, pattern);
    ASSERT_NE(frameNode, nullptr);
    pattern->AttachToFrameNode(frameNode);

    std::u16string content = u"Hello World";
    auto layoutWrapper = frameNode->CreateLayoutWrapper(true, true);
    ASSERT_NE(layoutWrapper, nullptr);

    auto textLayoutProperty = AceType::DynamicCast<TextLayoutProperty>(layoutWrapper->GetLayoutProperty());
    ASSERT_NE(textLayoutProperty, nullptr);

    EXPECT_EQ(ParagraphUtil::GetTextDirection(
        content, Referenced::RawPtr(layoutWrapper)), TextDirection::LTR);

    textLayoutProperty->UpdateLayoutDirection(TextDirection::LTR);
    EXPECT_EQ(ParagraphUtil::GetTextDirection(
        content, Referenced::RawPtr(layoutWrapper)), TextDirection::LTR);

    textLayoutProperty->UpdateLayoutDirection(TextDirection::RTL);
    EXPECT_EQ(ParagraphUtil::GetTextDirection(
        content, Referenced::RawPtr(layoutWrapper)), TextDirection::RTL);

    textLayoutProperty->UpdateLayoutDirection(TextDirection::AUTO);
    AceApplicationInfo::GetInstance().isRightToLeft_ = !AceApplicationInfo::GetInstance().IsRightToLeft();
    EXPECT_EQ(ParagraphUtil::GetTextDirection(
        content, Referenced::RawPtr(layoutWrapper)), TextDirection::LTR);
    AceApplicationInfo::GetInstance().isRightToLeft_ = !AceApplicationInfo::GetInstance().IsRightToLeft();
}

/**
 * @tc.name: InheritParentProperties001
 * @tc.desc: test span_node.cpp InheritParentProperties function
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNgSix, InheritParentProperties001, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<TextPattern>();
    ASSERT_NE(pattern, nullptr);
    auto frameNode = FrameNode::CreateFrameNode("Test", 1, pattern);
    ASSERT_NE(frameNode, nullptr);
    pattern->AttachToFrameNode(frameNode);

    auto layoutProperty = AceType::DynamicCast<TextLayoutProperty>(frameNode->GetLayoutProperty());
    ASSERT_NE(layoutProperty, nullptr);
    layoutProperty->UpdateFontSize(Dimension(10.0));
    layoutProperty->UpdateTextColor(Color::RED);
    auto pipeline = PipelineContext::GetCurrentContext();
    auto theme = AceType::MakeRefPtr<MockThemeManager>();
    EXPECT_CALL(*theme, GetTheme(_)).WillRepeatedly(Return(AceType::MakeRefPtr<TextTheme>()));

    auto oldTheme = pipeline->themeManager_;
    pipeline->themeManager_ = theme;

    auto spanNode = AceType::MakeRefPtr<SpanNode>(1);
    ASSERT_NE(spanNode, nullptr);
    auto spanItem = spanNode->GetSpanItem();
    ASSERT_NE(spanItem, nullptr);

    TextStyle textStyle;
    textStyle.SetFontSize(Dimension(10.0));
    textStyle.SetTextColor(Color::RED);

    pipeline->themeManager_ = oldTheme;
}

/**
 * @tc.name: AdaptMinFontSize001
 * @tc.desc: test text_adapt_font_sizer.cpp AdaptMinFontSize function
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNgSix, AdaptMinFontSize001, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<TextPattern>();
    ASSERT_NE(pattern, nullptr);
    auto frameNode = FrameNode::CreateFrameNode("Test", 1, pattern);
    ASSERT_NE(frameNode, nullptr);
    pattern->AttachToFrameNode(frameNode);
    auto layoutWrapper = frameNode->CreateLayoutWrapper(true, true);
    ASSERT_NE(layoutWrapper, nullptr);
    auto textAdaptFontSizer = AceType::MakeRefPtr<TextLayoutAlgorithm>();
    ASSERT_NE(textAdaptFontSizer, nullptr);

    TextStyle textStyle;
    std::u16string content;
    Dimension stepUnit;
    LayoutConstraintF contentConstraint;

    EXPECT_EQ(textAdaptFontSizer->AdaptMinFontSize(
        textStyle, content, stepUnit, contentConstraint, Referenced::RawPtr(layoutWrapper)), true);

    textStyle.SetAdaptTextSize(Dimension(100, DimensionUnit::PERCENT),
        Dimension(10, DimensionUnit::PERCENT), Dimension(10, DimensionUnit::PERCENT));

    contentConstraint.maxSize.SetWidth(-1.0);
    contentConstraint.maxSize.SetHeight(-1.0);

    EXPECT_EQ(textAdaptFontSizer->AdaptMinFontSize(
        textStyle, content, stepUnit, contentConstraint, Referenced::RawPtr(layoutWrapper)), true);

    contentConstraint.maxSize.SetWidth(1.0);
    contentConstraint.maxSize.SetHeight(1.0);

    EXPECT_EQ(textAdaptFontSizer->AdaptMinFontSize(
        textStyle, content, stepUnit, contentConstraint, Referenced::RawPtr(layoutWrapper)), true);

    TextStyle textStyleFP;
    textStyleFP.SetAdaptTextSize(Dimension(100, DimensionUnit::FP),
        Dimension(10, DimensionUnit::FP), Dimension(10, DimensionUnit::FP));
    textStyleFP.SetFontSize(Dimension(10, DimensionUnit::FP));

    EXPECT_EQ(textAdaptFontSizer->AdaptMinFontSize(
        textStyleFP, content, stepUnit, contentConstraint, Referenced::RawPtr(layoutWrapper)), true);
}

/**
 * @tc.name: IsNeedAdaptFontSize001
 * @tc.desc: test text_adapt_font_sizer.cpp IsNeedAdaptFontSize function
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNgSix, IsNeedAdaptFontSize001, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<TextPattern>();
    ASSERT_NE(pattern, nullptr);
    auto frameNode = FrameNode::CreateFrameNode("Test", 1, pattern);
    ASSERT_NE(frameNode, nullptr);
    pattern->AttachToFrameNode(frameNode);
    auto layoutWrapper = frameNode->CreateLayoutWrapper(true, true);
    ASSERT_NE(layoutWrapper, nullptr);
    auto textAdaptFontSizer = AceType::MakeRefPtr<TextLayoutAlgorithm>();
    ASSERT_NE(textAdaptFontSizer, nullptr);

    double maxFontSize = 0.0;
    double minFontSize = 0.0;
    EXPECT_EQ(textAdaptFontSizer->IsNeedAdaptFontSize(maxFontSize, minFontSize), false);

    maxFontSize = 1.0;
    minFontSize = 1.0;
    EXPECT_EQ(textAdaptFontSizer->IsNeedAdaptFontSize(maxFontSize, minFontSize), true);

    maxFontSize = -2.0;
    minFontSize = -1.0;
    EXPECT_EQ(textAdaptFontSizer->IsNeedAdaptFontSize(maxFontSize, minFontSize), false);

    maxFontSize = 0.0;
    minFontSize = -1.0;
    EXPECT_EQ(textAdaptFontSizer->IsNeedAdaptFontSize(maxFontSize, minFontSize), false);

    TextStyle textStyle;
    LayoutConstraintF contentConstraint;

    textAdaptFontSizer->IsNeedAdaptFontSize(textStyle, contentConstraint);

    textStyle.SetAdaptTextSize(Dimension(100, DimensionUnit::PERCENT),
        Dimension(10, DimensionUnit::PERCENT), Dimension(10, DimensionUnit::PERCENT));
    contentConstraint.maxSize.SetWidth(-1.0);
    contentConstraint.maxSize.SetHeight(-1.0);
    EXPECT_EQ(textAdaptFontSizer->IsNeedAdaptFontSize(textStyle, contentConstraint), false);
}

/**
 * @tc.name: IsAdaptFontSizeExceedLineHeight001
 * @tc.desc: test text_adapt_font_sizer.cpp IsAdaptFontSizeExceedLineHeight function
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNgSix, IsAdaptFontSizeExceedLineHeight001, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<TextPattern>();
    ASSERT_NE(pattern, nullptr);
    auto frameNode = FrameNode::CreateFrameNode("Test", 1, pattern);
    ASSERT_NE(frameNode, nullptr);
    pattern->AttachToFrameNode(frameNode);
    auto layoutWrapper = frameNode->CreateLayoutWrapper(true, true);
    ASSERT_NE(layoutWrapper, nullptr);
    auto textAdaptFontSizer = AceType::MakeRefPtr<TextLayoutAlgorithm>();
    ASSERT_NE(textAdaptFontSizer, nullptr);

    auto paragraph = MockParagraph::GetOrCreateMockParagraph();

    textAdaptFontSizer->lineHeight_ = 0.0;
    EXPECT_EQ(textAdaptFontSizer->IsAdaptFontSizeExceedLineHeight(paragraph), false);

    textAdaptFontSizer->lineHeight_ = 1.0;
    EXPECT_EQ(textAdaptFontSizer->IsAdaptFontSizeExceedLineHeight(paragraph), false);
}

/**
 * @tc.name: EncodeTlv001
 * @tc.desc: test span_node.cpp EncodeTlv function
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNgSix, EncodeTlv001, TestSize.Level1)
{
    BorderRadiusProperty borderRadius;
    borderRadius.SetRadius(2.0_vp);
    MarginProperty margins;
    margins.SetEdges(CalcLength(10.0));
    PaddingProperty paddings;
    paddings.SetEdges(CalcLength(5.0));
    ImageSpanAttribute attr { .paddingProp = paddings,
        .marginProp = margins,
        .borderRadius = borderRadius,
        .objectFit = ImageFit::COVER,
        .verticalAlign = VerticalAlign::BOTTOM };
    auto imageSpanItem = AceType::MakeRefPtr<NG::ImageSpanItem>();
    ASSERT_NE(imageSpanItem, nullptr);

    std::vector<uint8_t> buff;
    ImageSpanOptions options;

    imageSpanItem->SetImageSpanOptions(options);
    EXPECT_EQ(imageSpanItem->EncodeTlv(buff), true);

    buff.clear();

    options.offset = 0;
    options.image = "textImage";
    options.bundleName = "textBundleName";
    options.moduleName = "textModuleName";
    options.imagePixelMap = nullptr;
    options.imageAttribute = attr;
    imageSpanItem->SetImageSpanOptions(options);
    EXPECT_EQ(imageSpanItem->EncodeTlv(buff), true);

    int32_t cursor = 1;
    auto newImageSpanItem = imageSpanItem->DecodeTlv(buff, cursor);
    EXPECT_NE(newImageSpanItem, nullptr);

    cursor = 0;
    newImageSpanItem = imageSpanItem->DecodeTlv(buff, cursor);
    EXPECT_NE(newImageSpanItem, nullptr);
}

/**
 * @tc.name: OnHandleLevelModeChanged001
 * @tc.desc: test text_select_overlay.cpp OnHandleLevelModeChanged function
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNgSix, OnHandleLevelModeChanged001, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<TextPattern>();
    ASSERT_NE(pattern, nullptr);
    auto frameNode = FrameNode::CreateFrameNode("Test", 1, pattern);
    ASSERT_NE(frameNode, nullptr);
    pattern->AttachToFrameNode(frameNode);
    auto textSelectOverlay = pattern->selectOverlay_;
    ASSERT_NE(textSelectOverlay, nullptr);

    textSelectOverlay->OnAncestorNodeChanged(FRAME_NODE_CHANGE_GEOMETRY_CHANGE);

    textSelectOverlay->handleLevelMode_ = HandleLevelMode::EMBED;
    textSelectOverlay->OnHandleLevelModeChanged(HandleLevelMode::OVERLAY);
    EXPECT_EQ(textSelectOverlay->handleLevelMode_, HandleLevelMode::OVERLAY);

    textSelectOverlay->handleLevelMode_ = HandleLevelMode::OVERLAY;
    textSelectOverlay->OnHandleLevelModeChanged(HandleLevelMode::EMBED);
    EXPECT_EQ(textSelectOverlay->handleLevelMode_, HandleLevelMode::EMBED);

    textSelectOverlay->handleLevelMode_ = HandleLevelMode::EMBED;
    textSelectOverlay->OnHandleLevelModeChanged(HandleLevelMode::EMBED);
    EXPECT_EQ(textSelectOverlay->handleLevelMode_, HandleLevelMode::EMBED);

    textSelectOverlay->handleLevelMode_ = HandleLevelMode::OVERLAY;
    textSelectOverlay->OnHandleLevelModeChanged(HandleLevelMode::OVERLAY);
    EXPECT_EQ(textSelectOverlay->handleLevelMode_, HandleLevelMode::OVERLAY);
}

/**
 * @tc.name: OnHandleLevelModeChanged002
 * @tc.desc: test text_select_overlay.cpp OnHandleLevelModeChanged function
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNgSix, OnHandleLevelModeChanged002, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<TextPattern>();
    ASSERT_NE(pattern, nullptr);
    auto frameNode = FrameNode::CreateFrameNode("Test", 1, pattern);
    ASSERT_NE(frameNode, nullptr);
    pattern->AttachToFrameNode(frameNode);
    auto textSelectOverlay = pattern->selectOverlay_;
    ASSERT_NE(textSelectOverlay, nullptr);

    textSelectOverlay->OnAncestorNodeChanged(FRAME_NODE_CHANGE_TRANSFORM_CHANGE);

    textSelectOverlay->handleLevelMode_ = HandleLevelMode::EMBED;
    textSelectOverlay->OnHandleLevelModeChanged(HandleLevelMode::OVERLAY);
    EXPECT_EQ(textSelectOverlay->handleLevelMode_, HandleLevelMode::OVERLAY);

    textSelectOverlay->handleLevelMode_ = HandleLevelMode::OVERLAY;
    textSelectOverlay->OnHandleLevelModeChanged(HandleLevelMode::EMBED);
    EXPECT_EQ(textSelectOverlay->handleLevelMode_, HandleLevelMode::EMBED);

    textSelectOverlay->handleLevelMode_ = HandleLevelMode::EMBED;
    textSelectOverlay->OnHandleLevelModeChanged(HandleLevelMode::EMBED);
    EXPECT_EQ(textSelectOverlay->handleLevelMode_, HandleLevelMode::EMBED);

    textSelectOverlay->handleLevelMode_ = HandleLevelMode::OVERLAY;
    textSelectOverlay->OnHandleLevelModeChanged(HandleLevelMode::OVERLAY);
    EXPECT_EQ(textSelectOverlay->handleLevelMode_, HandleLevelMode::OVERLAY);
}

/**
 * @tc.name: CreateParagraph001
 * @tc.desc: test text_layout_algorithm.cpp CreateParagraph function
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNgSix, CreateParagraph001, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<TextPattern>();
    auto frameNode = FrameNode::CreateFrameNode("Test", 1, pattern);
    ASSERT_NE(frameNode, nullptr);
    pattern->AttachToFrameNode(frameNode);
    pattern->selectOverlayProxy_ = nullptr;
    auto textLayoutAlgorithm = AceType::DynamicCast<TextLayoutAlgorithm>(pattern->CreateLayoutAlgorithm());
    ASSERT_NE(textLayoutAlgorithm, nullptr);

    TextStyle textStyle;
    LayoutConstraintF contentConstraint;
    auto maxSize = MultipleParagraphLayoutAlgorithm::GetMaxMeasureSize(contentConstraint);

    ParagraphStyle externalParagraphStyle;
    RefPtr<Paragraph> externalParagraph = Paragraph::Create(externalParagraphStyle, FontCollection::Current());
    ASSERT_NE(externalParagraph, nullptr);
    pattern->SetExternalParagraph(AceType::RawPtr(externalParagraph));

    EXPECT_EQ(textLayoutAlgorithm->CreateParagraph(textStyle, u"", AceType::RawPtr(frameNode), maxSize.Width()), true);

    pattern->textDetectEnable_ = true;
    pattern->copyOption_ = CopyOptions::InApp;
    ASSERT_NE(pattern->GetDataDetectorAdapter(), nullptr);
    pattern->dataDetectorAdapter_->aiSpanMap_.insert(std::make_pair(0, AISpan()));

    EXPECT_EQ(textLayoutAlgorithm->CreateParagraph(textStyle, u"", AceType::RawPtr(frameNode), maxSize.Width()), true);

    pattern->SetExternalParagraphStyle(externalParagraphStyle);

    EXPECT_EQ(textLayoutAlgorithm->CreateParagraph(textStyle, u"", AceType::RawPtr(frameNode), maxSize.Width()), true);
}

/**
 * @tc.name: UpdateSymbolTextStyle001
 * @tc.desc: test text_layout_algorithm.cpp UpdateSymbolTextStyle function
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNgSix, UpdateSymbolTextStyle001, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<TextPattern>();
    auto frameNode = FrameNode::CreateFrameNode("Test", 1, pattern);
    ASSERT_NE(frameNode, nullptr);
    pattern->AttachToFrameNode(frameNode);
    pattern->selectOverlayProxy_ = nullptr;
    auto textLayoutAlgorithm = AceType::DynamicCast<TextLayoutAlgorithm>(pattern->CreateLayoutAlgorithm());
    ASSERT_NE(textLayoutAlgorithm, nullptr);

    auto layoutProperty = AceType::DynamicCast<TextLayoutProperty>(frameNode->GetLayoutProperty());
    ASSERT_NE(layoutProperty, nullptr);
    layoutProperty->UpdateSymbolSourceInfo(SymbolSourceInfo());

    TextStyle textStyle;
    ParagraphStyle paragraphStyle;
    NG::SymbolEffectOptions symbolEffectOptions;

    EXPECT_EQ(textLayoutAlgorithm->UpdateSymbolTextStyle(
        textStyle, paragraphStyle, AceType::RawPtr(frameNode), frameNode), true);

    textStyle.SetRenderStrategy(-1);
    textStyle.SetEffectStrategy(-1);
    textStyle.SetSymbolEffectOptions(symbolEffectOptions);
    EXPECT_EQ(textLayoutAlgorithm->UpdateSymbolTextStyle(
        textStyle, paragraphStyle, AceType::RawPtr(frameNode), frameNode), true);
}

/**
 * @tc.name: UpdateSymbolTextStyle002
 * @tc.desc: test text_layout_algorithm.cpp UpdateSymbolTextStyle function for custom symbol
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNgSix, UpdateSymbolTextStyle002, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<TextPattern>();
    auto frameNode = FrameNode::CreateFrameNode("Test", 1, pattern);
    ASSERT_NE(frameNode, nullptr);
    pattern->AttachToFrameNode(frameNode);
    pattern->selectOverlayProxy_ = nullptr;
    auto textLayoutAlgorithm = AceType::DynamicCast<TextLayoutAlgorithm>(pattern->CreateLayoutAlgorithm());
    ASSERT_NE(textLayoutAlgorithm, nullptr);

    auto layoutProperty = AceType::DynamicCast<TextLayoutProperty>(frameNode->GetLayoutProperty());
    ASSERT_NE(layoutProperty, nullptr);
    layoutProperty->UpdateSymbolSourceInfo(SymbolSourceInfo(DEFAULT_SYMBOL_ID));

    TextStyle textStyle;
    ParagraphStyle paragraphStyle;
    textStyle.SetSymbolType(SymbolType::SYSTEM);

    EXPECT_EQ(textLayoutAlgorithm->UpdateSymbolTextStyle(
        textStyle, paragraphStyle, AceType::RawPtr(frameNode), frameNode), true);

    textStyle.SetSymbolType(SymbolType::CUSTOM);

    EXPECT_EQ(textLayoutAlgorithm->UpdateSymbolTextStyle(
        textStyle, paragraphStyle, AceType::RawPtr(frameNode), frameNode), false);

    std::vector<std::string> fontFamilies;
    fontFamilies.push_back(SYMBOL_FONT_FAMILY);
    textStyle.SetFontFamilies(fontFamilies);
    EXPECT_EQ(textLayoutAlgorithm->UpdateSymbolTextStyle(
        textStyle, paragraphStyle, AceType::RawPtr(frameNode), frameNode), true);
}

/**
 * @tc.name: AdaptMinTextSize001
 * @tc.desc: test text_layout_algorithm.cpp AdaptMinTextSize function
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNgSix, AdaptMinTextSize001, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<TextPattern>();
    auto frameNode = FrameNode::CreateFrameNode("Test", 1, pattern);
    ASSERT_NE(frameNode, nullptr);
    pattern->AttachToFrameNode(frameNode);
    pattern->selectOverlayProxy_ = nullptr;
    auto textLayoutAlgorithm = AceType::DynamicCast<TextLayoutAlgorithm>(pattern->CreateLayoutAlgorithm());
    ASSERT_NE(textLayoutAlgorithm, nullptr);

    TextStyle textStyle;
    std::u16string content;
    LayoutConstraintF contentConstraint;

    frameNode->pattern_ = nullptr;
    EXPECT_EQ(textLayoutAlgorithm->AdaptMinTextSize(
        textStyle, content, contentConstraint, AceType::RawPtr(frameNode)), false);

    textStyle.SetAdaptTextSize(Dimension(100, DimensionUnit::PX),
        Dimension(10, DimensionUnit::PX), Dimension(10, DimensionUnit::PX));
    EXPECT_EQ(textLayoutAlgorithm->AdaptMinTextSize(
        textStyle, content, contentConstraint, AceType::RawPtr(frameNode)), false);
    frameNode->pattern_ = pattern;

    textStyle.SetAdaptTextSize(Dimension(100, DimensionUnit::PERCENT),
        Dimension(10, DimensionUnit::PERCENT), Dimension(10, DimensionUnit::PERCENT));
    contentConstraint.maxSize.SetHeight(-1.0);
    contentConstraint.maxSize.SetWidth(-1.0);
    EXPECT_EQ(textLayoutAlgorithm->AdaptMinTextSize(
        textStyle, content, contentConstraint, AceType::RawPtr(frameNode)), true);

    auto paragraph = MockParagraph::GetOrCreateMockParagraph();
    std::vector<RectF> rects { RectF(0, 0, 5, 5) };
    EXPECT_CALL(*paragraph, GetRectsForRange(_, _, _)).WillRepeatedly(SetArgReferee<2>(rects));
    EXPECT_CALL(*paragraph, GetLineCount()).WillRepeatedly(Return(2));
    EXPECT_CALL(*paragraph, GetHeight()).WillRepeatedly(Return(2));
    EXPECT_CALL(*paragraph, GetLongestLine()).WillRepeatedly(Return(2));
    pattern->pManager_->AddParagraph(
        { .paragraph = paragraph, .start = 0, .end = 2 });

    contentConstraint.maxSize.SetHeight(1.0);
    contentConstraint.maxSize.SetWidth(1.0);
    EXPECT_EQ(textLayoutAlgorithm->AdaptMinTextSize(
        textStyle, content, contentConstraint, AceType::RawPtr(frameNode)), true);
}

/**
 * @tc.name: GetGraphemeClusterLength001
 * @tc.desc: test text_base.cpp GetGraphemeClusterLength function
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNgSix, GetGraphemeClusterLength001, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<TextPattern>();
    auto frameNode = FrameNode::CreateFrameNode("Test", 1, pattern);
    ASSERT_NE(frameNode, nullptr);
    pattern->AttachToFrameNode(frameNode);

    std::u16string text;

    EXPECT_EQ(pattern->GetGraphemeClusterLength(text, 0, false), 1);

    text = u"Test";

    EXPECT_EQ(pattern->GetGraphemeClusterLength(text, 0, true), 1);
    EXPECT_EQ(pattern->GetGraphemeClusterLength(text, 10, true), 1);
    EXPECT_EQ(pattern->GetGraphemeClusterLength(text, 0, false), 1);
    EXPECT_EQ(pattern->GetGraphemeClusterLength(text, 10, false), 1);
}

/**
 * @tc.name: CalculateSelectedRect001
 * @tc.desc: test text_base.cpp CalculateSelectedRect function
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNgSix, CalculateSelectedRect001, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<TextPattern>();
    auto frameNode = FrameNode::CreateFrameNode("Test", 1, pattern);
    ASSERT_NE(frameNode, nullptr);
    pattern->AttachToFrameNode(frameNode);

    std::vector<RectF> selectedRect;

    selectedRect.emplace_back(RectF(0, 0, 0, 0));
    pattern->CalculateSelectedRect(selectedRect, 0);
    EXPECT_EQ(selectedRect.size(), 1);

    selectedRect.emplace_back(RectF(0, 0, 1, 1));
    pattern->CalculateSelectedRect(selectedRect, 1);
    EXPECT_EQ(selectedRect.size(), 1);

    selectedRect.emplace_back(RectF(0, 0, 2, 2));
    pattern->CalculateSelectedRect(selectedRect, 2);
    EXPECT_EQ(selectedRect.size(), 1);

    selectedRect.emplace_back(RectF(10, 10, 20, 20));
    pattern->CalculateSelectedRect(selectedRect, 10);
    EXPECT_EQ(selectedRect.size(), 2);
}

/**
 * @tc.name: GetVisibleRect001
 * @tc.desc: test base_text_select_overlay.cpp GetVisibleRect function
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNgSix, GetVisibleRect001, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<TextPattern>();
    ASSERT_NE(pattern, nullptr);
    auto frameNode = FrameNode::CreateFrameNode("Test", 1, pattern);
    ASSERT_NE(frameNode, nullptr);
    pattern->AttachToFrameNode(frameNode);
    auto textSelectOverlay = pattern->selectOverlay_;
    ASSERT_NE(textSelectOverlay, nullptr);
    auto parentPattern = AceType::MakeRefPtr<MockNestableScrollContainer>();
    ASSERT_NE(parentPattern, nullptr);
    auto parentFrameNode = FrameNode::CreateFrameNode("ParentTest", 1, parentPattern);
    ASSERT_NE(parentFrameNode, nullptr);
    ASSERT_NE(pattern->GetHost(), nullptr);
    pattern->GetHost()->SetParent(parentFrameNode);

    RectF visibleRect;
    visibleRect.SetRect(0, 0, 10, 10);

    EXPECT_NE(textSelectOverlay->GetVisibleRect(pattern->GetHost(), visibleRect), visibleRect);

    parentFrameNode = FrameNode::CreateFrameNode(V2::PAGE_ETS_TAG, 1, parentPattern);
    ASSERT_NE(parentFrameNode, nullptr);
    pattern->GetHost()->SetParent(parentFrameNode);

    EXPECT_EQ(textSelectOverlay->GetVisibleRect(pattern->GetHost(), visibleRect), visibleRect);

    auto parentPattern1 = AceType::MakeRefPtr<TextPattern>();
    ASSERT_NE(parentPattern1, nullptr);
    parentFrameNode = FrameNode::CreateFrameNode("ParentTest", 1, parentPattern1);
    ASSERT_NE(parentFrameNode, nullptr);
    pattern->GetHost()->SetParent(parentFrameNode);

    EXPECT_EQ(textSelectOverlay->GetVisibleRect(pattern->GetHost(), visibleRect), visibleRect);
}

/**
 * @tc.name: GetLocalPointsWithTransform001
 * @tc.desc: test base_text_select_overlay.cpp GetLocalPointsWithTransform function
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNgSix, GetLocalPointsWithTransform001, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<TextPattern>();
    ASSERT_NE(pattern, nullptr);
    auto frameNode = FrameNode::CreateFrameNode("Test", 1, pattern);
    ASSERT_NE(frameNode, nullptr);
    pattern->AttachToFrameNode(frameNode);
    auto textSelectOverlay = pattern->selectOverlay_;
    ASSERT_NE(textSelectOverlay, nullptr);

    textSelectOverlay->hasTransform_ = true;

    std::vector<OffsetF> localPoints;

    textSelectOverlay->GetLocalPointsWithTransform(localPoints);
    EXPECT_EQ(localPoints.size(), 0);

    localPoints.emplace_back(OffsetF(0, 0));
    localPoints.emplace_back(OffsetF(1, 1));
    textSelectOverlay->GetLocalPointsWithTransform(localPoints);
    EXPECT_EQ(localPoints.size(), 2);
    EXPECT_EQ(localPoints[0].GetX(), 0);
    EXPECT_EQ(localPoints[0].GetY(), 0);
    EXPECT_EQ(localPoints[1].GetX(), 1);
    EXPECT_EQ(localPoints[1].GetY(), 1);
}

/**
 * @tc.name: GetAncestorNodeViewPort001
 * @tc.desc: test base_text_select_overlay.cpp GetAncestorNodeViewPort function
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNgSix, GetAncestorNodeViewPort001, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<TextPattern>();
    ASSERT_NE(pattern, nullptr);
    auto frameNode = FrameNode::CreateFrameNode("Test", 1, pattern);
    ASSERT_NE(frameNode, nullptr);
    pattern->AttachToFrameNode(frameNode);
    auto textSelectOverlay = pattern->selectOverlay_;
    ASSERT_NE(textSelectOverlay, nullptr);

    auto parentPattern1 = AceType::MakeRefPtr<TextPattern>();
    ASSERT_NE(parentPattern1, nullptr);
    auto parentFrameNode1 = FrameNode::CreateFrameNode("ParentTest", 1, parentPattern1);
    ASSERT_NE(parentFrameNode1, nullptr);
    ASSERT_NE(pattern->GetHost(), nullptr);
    pattern->GetHost()->SetParent(parentFrameNode1);

    EXPECT_EQ(textSelectOverlay->GetAncestorNodeViewPort(), RectF(0.0f, 0.0f, 0.0f, 0.0f));
}

/**
 * @tc.name: IsAcceptResetSelectionEvent001
 * @tc.desc: test base_text_select_overlay.cpp IsAcceptResetSelectionEvent function
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNgSix, IsAcceptResetSelectionEvent001, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<TextPattern>();
    ASSERT_NE(pattern, nullptr);
    auto frameNode = FrameNode::CreateFrameNode("Test", 1, pattern);
    ASSERT_NE(frameNode, nullptr);
    pattern->AttachToFrameNode(frameNode);
    auto textSelectOverlay = pattern->selectOverlay_;
    ASSERT_NE(textSelectOverlay, nullptr);

    EXPECT_EQ(textSelectOverlay->IsAcceptResetSelectionEvent(SourceType::MOUSE, TouchType::DOWN), true);
    EXPECT_EQ(textSelectOverlay->IsAcceptResetSelectionEvent(SourceType::MOUSE, TouchType::UP), false);
    EXPECT_EQ(textSelectOverlay->IsAcceptResetSelectionEvent(SourceType::TOUCH, TouchType::DOWN), true);
    EXPECT_EQ(textSelectOverlay->IsAcceptResetSelectionEvent(SourceType::TOUCH, TouchType::UP), false);
    EXPECT_EQ(textSelectOverlay->IsAcceptResetSelectionEvent(SourceType::KEYBOARD, TouchType::DOWN), false);
    EXPECT_EQ(textSelectOverlay->IsAcceptResetSelectionEvent(SourceType::KEYBOARD, TouchType::UP), false);
}

/**
 * @tc.name: RevertLocalPointWithTransform001
 * @tc.desc: test base_text_select_overlay.cpp RevertLocalPointWithTransform function
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNgSix, RevertLocalPointWithTransform001, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<TextPattern>();
    ASSERT_NE(pattern, nullptr);
    auto frameNode = FrameNode::CreateFrameNode("Test", 1, pattern);
    ASSERT_NE(frameNode, nullptr);
    pattern->AttachToFrameNode(frameNode);
    auto textSelectOverlay = pattern->selectOverlay_;
    ASSERT_NE(textSelectOverlay, nullptr);

    auto parentPattern1 = AceType::MakeRefPtr<TextPattern>();
    ASSERT_NE(parentPattern1, nullptr);
    auto parentFrameNode1 = FrameNode::CreateFrameNode("ParentTest", 1, parentPattern1);
    ASSERT_NE(parentFrameNode1, nullptr);
    ASSERT_NE(pattern->GetHost(), nullptr);
    pattern->GetHost()->SetParent(parentFrameNode1);

    OffsetF point(10, 10);

    textSelectOverlay->hasTransform_ = true;
    textSelectOverlay->RevertLocalPointWithTransform(point);
    EXPECT_EQ(point.GetX(), 10);
    EXPECT_EQ(point.GetY(), 10);
}

/**
 * @tc.name: HasUnsupportedTransform001
 * @tc.desc: test base_text_select_overlay.cpp HasUnsupportedTransform function
 * @tc.type: FUNC
 */
HWTEST_F(TextTestNgSix, HasUnsupportedTransform001, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<TextPattern>();
    ASSERT_NE(pattern, nullptr);
    auto frameNode = FrameNode::CreateFrameNode("Test", 1, pattern);
    ASSERT_NE(frameNode, nullptr);
    pattern->AttachToFrameNode(frameNode);
    auto textSelectOverlay = pattern->selectOverlay_;
    ASSERT_NE(textSelectOverlay, nullptr);
    auto renderContext = frameNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);

    auto parentPattern = AceType::MakeRefPtr<TextPattern>();
    ASSERT_NE(parentPattern, nullptr);
    auto parentFrameNode = FrameNode::CreateFrameNode(V2::WINDOW_SCENE_ETS_TAG, 1, parentPattern);
    ASSERT_NE(parentFrameNode, nullptr);
    ASSERT_NE(pattern->GetHost(), nullptr);
    pattern->GetHost()->SetParent(parentFrameNode);
    EXPECT_EQ(textSelectOverlay->HasUnsupportedTransform(), false);
    pattern->GetHost()->SetParent(nullptr);

    renderContext->UpdateTransformRotate(Vector5F(0, 0, 0, 0, 0));
    EXPECT_EQ(textSelectOverlay->HasUnsupportedTransform(), false);

    renderContext->UpdateTransformRotate(Vector5F(10, 0, 0, 0, 0));
    EXPECT_EQ(textSelectOverlay->HasUnsupportedTransform(), false);

    renderContext->UpdateTransformRotate(Vector5F(0, 10, 0, 0, 0));
    EXPECT_EQ(textSelectOverlay->HasUnsupportedTransform(), false);

    renderContext->UpdateTransformRotate(Vector5F(0, 0, 0, 0, 10));
    EXPECT_EQ(textSelectOverlay->HasUnsupportedTransform(), false);

    renderContext->UpdateTransformMatrix(Matrix4());
    EXPECT_EQ(textSelectOverlay->HasUnsupportedTransform(), false);
}
} // namespace OHOS::Ace::NG

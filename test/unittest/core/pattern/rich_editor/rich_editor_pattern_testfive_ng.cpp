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

#include "test/unittest/core/pattern/rich_editor/rich_editor_common_test_ng.h"
#include "test/mock/core/render/mock_paragraph.h"
#include "test/mock/core/pipeline/mock_pipeline_context.h"
#include "test/mock/core/common/mock_theme_manager.h"
#include "test/mock/core/common/mock_container.h"
#include "test/mock/base/mock_task_executor.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_theme.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
class RichEditorPatternTestFiveNg : public RichEditorCommonTestNg {
public:
    void SetUp() override;
    void TearDown() override;
    static void TearDownTestSuite();
};

void RichEditorPatternTestFiveNg::SetUp()
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

void RichEditorPatternTestFiveNg::TearDown()
{
    richEditorNode_ = nullptr;
    MockParagraph::TearDown();
}

void RichEditorPatternTestFiveNg::TearDownTestSuite()
{
    TestNG::TearDownTestSuite();
}

/**
 * @tc.name: BuilderSpanBindSelectionMenu001
 * @tc.desc: Test bind selection menu on BuilderSpan.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorPatternTestFiveNg, BuilderSpanBindSelectionMenu001, TestSize.Level1)
{
    auto richEditorNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(richEditorNode, nullptr);
    auto richEditorPattern = richEditorNode->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    SelectionInfo originalSelection;
    SelectionInfo adjustedSelection;

    // 0: Select BuilderSpan
    ResultObject objBuilder;
    objBuilder.type = SelectSpanType::TYPEIMAGE;
    objBuilder.valueString = u" ";
    objBuilder.valuePixelMap = nullptr;
    originalSelection.selection_.resultObjects.push_front(objBuilder);
    adjustedSelection = richEditorPattern->GetAdjustedSelectionInfo(originalSelection);
    richEditorPattern->UpdateSelectionType(adjustedSelection);
    EXPECT_EQ(richEditorPattern->selectedType_.value(), TextSpanType::BUILDER);

    // 1: Select BuilderSpan and TextSpan
    ResultObject objText;
    objText.type = SelectSpanType::TYPESPAN;
    originalSelection.selection_.resultObjects.push_front(objText);
    adjustedSelection = richEditorPattern->GetAdjustedSelectionInfo(originalSelection);
    richEditorPattern->UpdateSelectionType(adjustedSelection);
    EXPECT_EQ(richEditorPattern->selectedType_.value(), TextSpanType::MIXED);

    // 2: Select BuilderSpan、TextSpan and ImageSpan
    ResultObject objImage;
    objImage.type = SelectSpanType::TYPEIMAGE;
    objImage.valueString = u" ";
    objImage.valuePixelMap = PixelMap::CreatePixelMap(nullptr);
    originalSelection.selection_.resultObjects.push_front(objImage);
    adjustedSelection = richEditorPattern->GetAdjustedSelectionInfo(originalSelection);
    richEditorPattern->UpdateSelectionType(adjustedSelection);
    EXPECT_EQ(richEditorPattern->selectedType_.value(), TextSpanType::MIXED);
}

/**
 * @tc.name: SelectionMenuInteraction001
 * @tc.desc: Test the interaction between selection menu and click event.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorPatternTestFiveNg, SelectionMenuInteraction001, TestSize.Level1)
{
    auto richEditorNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(richEditorNode, nullptr);
    auto richEditorPattern = richEditorNode->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    std::vector<RectF> rects { RectF(0.0, 0.0, 200.0, 200.0) };
    auto paragraph = MockParagraph::GetOrCreateMockParagraph();
    ASSERT_NE(paragraph, nullptr);
    richEditorPattern->paragraphs_.AddParagraph({ .paragraph = paragraph, .start = 0, .end = 10 });
    EXPECT_CALL(*paragraph, GetRectsForRange(_, _, _)).WillRepeatedly(SetArgReferee<THIRD_PARAM>(rects));
    
    // 0: Show selectOverlay
    richEditorPattern->textSelector_.Update(0, 1);
    ASSERT_NE(richEditorPattern->selectOverlay_, nullptr);
    richEditorPattern->CalculateHandleOffsetAndShowOverlay();
    richEditorPattern->ShowSelectOverlay(
        richEditorPattern->textSelector_.firstHandle, richEditorPattern->textSelector_.secondHandle, false);
    ASSERT_TRUE(richEditorPattern->SelectOverlayIsOn());

    // 1: Toggle the menu when the selection area is clicked
    GestureEvent info;
    info.deviceType_ = SourceType::TOUCH;
    info.localLocation_ = Offset(50, 50);
    info.globalLocation_ = Offset(50, 50);
    auto manager = richEditorPattern->selectOverlay_->GetManager<SelectContentOverlayManager>();
    ASSERT_NE(manager, nullptr);
    manager->ShowOptionMenu();
    richEditorPattern->HandleClickSelection(info);
    EXPECT_FALSE(richEditorPattern->selectOverlay_->GetSelectOverlayInfo()->menuInfo.menuIsShow);
    richEditorPattern->HandleClickSelection(info);
    EXPECT_TRUE(richEditorPattern->selectOverlay_->GetSelectOverlayInfo()->menuInfo.menuIsShow);
}

/**
 * @tc.name: GetUrlSpanColor001
 * @tc.desc: Test get color in urlSpan theme.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorPatternTestFiveNg, GetUrlSpanColor001, TestSize.Level1)
{
    // 0: Get richEditor Node and richEditor Pattern
    auto richEditorNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(richEditorNode, nullptr);
    auto richEditorPattern = richEditorNode->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);

    // 1: Get richEditorTheme
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    ASSERT_NE(themeManager, nullptr);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(AceType::MakeRefPtr<RichEditorTheme>()));
    EXPECT_CALL(*themeManager, GetTheme(_, _)).WillRepeatedly(Return(AceType::MakeRefPtr<RichEditorTheme>()));

    // 2: Backup old richEditorTheme and set new richEditorTheme
    auto oldThemeManager = PipelineBase::GetCurrentContext()->themeManager_;
    PipelineBase::GetCurrentContext()->themeManager_ = themeManager;

    // 3: Test whether richEditor pattern->GetUrlHoverColor() and theme-GetUrlHoverColor() are the same color
    RichEditorTheme richEditorTheme;
    EXPECT_EQ(richEditorPattern->GetUrlHoverColor(), richEditorTheme.GetUrlHoverColor());

    // 4: Test whether richEditor pattern->GetUrlPressColor() and theme-GetUrlPressColor() are the same color
    EXPECT_EQ(richEditorPattern->GetUrlPressColor(), richEditorTheme.GetUrlPressColor());

    // 5: Restore old richEditorTheme
    PipelineBase::GetCurrentContext()->themeManager_ = oldThemeManager;
}

/**
 * @tc.name: GetUrlSpanShowShadow001
 * @tc.desc: Test set urlSpan showShadow
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorPatternTestFiveNg, GetUrlSpanShowShadow001, TestSize.Level1)
{
    // 0: Get richEditor Node and richEditor Pattern
    auto richEditorNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(richEditorNode, nullptr);
    auto richEditorPattern = richEditorNode->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);

    // 1: Mock location
    Offset localLocation = Offset(54.0, 20.0);
    Offset globalLocation = Offset(54.0, 20.0);

    // 2: Mock contentRect position and width and height
    richEditorPattern->contentRect_.x_ = 25.0f;
    richEditorPattern->contentRect_.y_ = 13.0f;
    richEditorPattern->contentRect_.width_ = 526.0f;
    richEditorPattern->contentRect_.height_ = 56.0f;

    // 3: Create urlSpan with hyperlink address
    std::string address = "https://www.example.com";
    auto urlSpan = AceType::MakeRefPtr<UrlSpan>(address);

    // 4: Create spanItem and set to spans list
    auto spanItem = AceType::MakeRefPtr<NG::SpanItem>();
    urlSpan->ApplyToSpanItem(spanItem, SpanOperation::ADD);
    spanItem->position = 2;
    std::list<RefPtr<NG::SpanItem>> spans;
    spans.push_back(spanItem);

    // 5: Create ParagraphManager and set to richEditorPattern
    auto pManager = AceType::MakeRefPtr<ParagraphManager>();
    ASSERT_NE(pManager, nullptr);
    richEditorPattern->pManager_ = pManager;

    // 6: Create MutableSpanString and set to richEditorPattern
    auto spanString = AceType::MakeRefPtr<MutableSpanString>(u"click here");
    spanString->AddSpan(AceType::MakeRefPtr<UrlSpan>(address, 0, 10));
    richEditorPattern->SetStyledString(spanString);

    // 7: Create TextOverlayModifier and set to richEditorPattern
    richEditorPattern->overlayMod_ = AceType::MakeRefPtr<TextOverlayModifier>();

    // 8: Create Paragraph and set to richEditorPattern
    auto paragraph = MockParagraph::GetOrCreateMockParagraph();
    richEditorPattern->paragraphs_.AddParagraph({ .paragraph = paragraph, .start = 0, .end = 100 });

    // 9: spans list set to richEditorPattern
    richEditorPattern->spans_ = spans;

    // 10: call HandleUrlSpanShowShadow method
    bool show = richEditorPattern->HandleUrlSpanShowShadow(localLocation, globalLocation, Color(Color::BLUE));

    // 11: Asserts that the ShowShadow result is true
    EXPECT_TRUE(show);
}

/**
 * @tc.name: HandleExtendAction001
 * @tc.desc: test HandleExtendAction
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorPatternTestFiveNg, HandleExtendAction001, TestSize.Level1)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    richEditorPattern->HandleExtendAction(ACTION_SELECT_ALL);
    EXPECT_TRUE(richEditorPattern->showSelect_);

    richEditorPattern->HandleExtendAction(ACTION_CUT);
    EXPECT_EQ(richEditorPattern->copyOption_, CopyOptions::None);

    richEditorPattern->HandleExtendAction(ACTION_COPY);
    EXPECT_EQ(richEditorPattern->copyOption_, CopyOptions::None);

    auto eventHub = richEditorNode_->GetOrCreateEventHub<RichEditorEventHub>();
    ASSERT_NE(richEditorPattern, nullptr);
    TextCommonEvent event;
    eventHub->FireOnPaste(event);
    richEditorPattern->HandleExtendAction(ACTION_PASTE);
    EXPECT_EQ(richEditorPattern->clipboard_, nullptr);
}

/**
 * @tc.name: HandleExtendAction002
 * @tc.desc: test HandleExtendAction
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorPatternTestFiveNg, HandleExtendAction002, TestSize.Level1)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    richEditorPattern->HandleExtendAction(2);
    EXPECT_TRUE(richEditorPattern->selectMenuInfo_.showCopyAll);
}

/**
 * @tc.name: UpdateMagnifierStateAfterLayout001
 * @tc.desc: test UpdateMagnifierStateAfterLayout
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorPatternTestFiveNg, UpdateMagnifierStateAfterLayout001, TestSize.Level1)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    WeakPtr<TextBase> textBase;
    richEditorPattern->selectOverlay_ = AceType::MakeRefPtr<RichEditorSelectOverlay>(textBase);
    richEditorPattern->magnifierController_.Reset();
    richEditorPattern->UpdateMagnifierStateAfterLayout(true);
    EXPECT_FALSE(richEditorPattern->caretVisible_);
}

/**
 * @tc.name: UpdateMagnifierStateAfterLayout002
 * @tc.desc: test UpdateMagnifierStateAfterLayout
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorPatternTestFiveNg, UpdateMagnifierStateAfterLayout002, TestSize.Level1)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    WeakPtr<TextBase> textBase;
    richEditorPattern->selectOverlay_ = AceType::MakeRefPtr<RichEditorSelectOverlay>(textBase);
    WeakPtr<Pattern> pattern;
    richEditorPattern->magnifierController_ = AceType::MakeRefPtr<MagnifierController>(pattern);
    richEditorPattern->magnifierController_->magnifierNodeExist_ = true;
    richEditorPattern->UpdateMagnifierStateAfterLayout(true);
    EXPECT_FALSE(richEditorPattern->caretVisible_);
}

/**
 * @tc.name: HandleSurfaceChanged001
 * @tc.desc: test HandleSurfaceChanged
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorPatternTestFiveNg, HandleSurfaceChanged001, TestSize.Level1)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    richEditorPattern->magnifierController_.Reset();
    richEditorPattern->HandleSurfaceChanged(1, 1, 2, 2, WindowSizeChangeReason::DRAG);
    EXPECT_FALSE(richEditorPattern->originIsMenuShow_);
}

/**
 * @tc.name: SetSubSpans001
 * @tc.desc: test SetSubSpans
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorPatternTestFiveNg, SetSubSpans001, TestSize.Level1)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    RefPtr<SpanItem> spanItem = AceType::MakeRefPtr<SpanItem>();
    RefPtr<SpanItem> spanItem1 = AceType::MakeRefPtr<SpanItem>();
    spanItem->spanItemType = SpanItemType::SYMBOL;
    spanItem1->spanItemType = SpanItemType::NORMAL;
    spanItem1->position = 0;
    richEditorPattern->spans_.push_back(spanItem);
    richEditorPattern->spans_.push_back(spanItem1);
    RefPtr<SpanString> spanString = AceType::MakeRefPtr<SpanString>(INIT_VALUE_1);
    richEditorPattern->SetSubSpans(spanString, 1, 1, spanString->spans_);
    EXPECT_EQ(spanString->spans_.size(), 0);
}

/**
 * @tc.name: HandlePointWithTransform001
 * @tc.desc: test HandlePointWithTransform
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorPatternTestFiveNg, HandlePointWithTransform001, TestSize.Level1)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    richEditorNode_->tag_ = V2::WINDOW_SCENE_ETS_TAG;
    OffsetF point(0, 0);
    richEditorPattern->HandlePointWithTransform(point);
    EXPECT_EQ(point.GetX(), 0);
    EXPECT_EQ(point.GetY(), 0);
}

/**
 * @tc.name: HandlePointWithTransform002
 * @tc.desc: test HandlePointWithTransform
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorPatternTestFiveNg, HandlePointWithTransform002, TestSize.Level1)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    richEditorNode_->tag_ = V2::SCREEN_ETS_TAG;
    OffsetF point(0, 0);
    richEditorPattern->HandlePointWithTransform(point);
    EXPECT_EQ(point.GetX(), 0);
    EXPECT_EQ(point.GetY(), 0);
}

/**
 * @tc.name: HandleKbVerticalSelection001
 * @tc.desc: test HandleKbVerticalSelection
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorPatternTestFiveNg, HandleKbVerticalSelection001, TestSize.Level1)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    richEditorPattern->textSelector_.baseOffset = 1;
    richEditorPattern->textSelector_.destinationOffset = 0;
    richEditorPattern->caretPosition_ = 1;
    EXPECT_EQ(richEditorPattern->HandleKbVerticalSelection(true), 0);
    richEditorPattern->caretPosition_ = 0;
    EXPECT_EQ(richEditorPattern->HandleKbVerticalSelection(true), 0);
}

/**
 * @tc.name: HandleKbVerticalSelection002
 * @tc.desc: test HandleKbVerticalSelection
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorPatternTestFiveNg, HandleKbVerticalSelection002, TestSize.Level1)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    richEditorPattern->textSelector_.baseOffset = 2;
    richEditorPattern->textSelector_.destinationOffset = 0;
    richEditorPattern->caretPosition_ = 1;
    EXPECT_EQ(richEditorPattern->HandleKbVerticalSelection(true), 0);
}

/**
 * @tc.name: DumpInfo001
 * @tc.desc: test DumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorPatternTestFiveNg, DumpInfo001, TestSize.Level1)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto context = richEditorNode_->GetContextRefPtr();
    auto pipe = richEditorNode_->GetContext();
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    ASSERT_NE(themeManager, nullptr);
    pipe->SetThemeManager(themeManager);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(AceType::MakeRefPtr<RichEditorTheme>()));
    std::function<void()> test;
    richEditorPattern->customKeyboardBuilder_ = test;
    std::unique_ptr<JsonValue> json = std::make_unique<JsonValue>();
    richEditorPattern->DumpInfo(json);
    EXPECT_FALSE(richEditorPattern->selectOverlay_->HasRenderTransform());

    test = []() {};
    richEditorPattern->customKeyboardBuilder_ = test;
    json = std::make_unique<JsonValue>();
    richEditorPattern->DumpInfo(json);
    EXPECT_FALSE(richEditorPattern->selectOverlay_->HasRenderTransform());
}


/**
 * @tc.name: TriggerAvoidOnCaretChange
 * @tc.desc: test rich_editor_pattern.cpp TriggerAvoidOnCaretChange function
 * @tc.type: FUNC
 */
 HWTEST_F(RichEditorPatternTestFiveNg, TriggerAvoidOnCaretChange, TestSize.Level1)
 {
     ASSERT_NE(richEditorNode_, nullptr);
     auto pattern_ = richEditorNode_->GetPattern<RichEditorPattern>();
     ASSERT_NE(pattern_, nullptr);
     auto focusHub = pattern_->GetFocusHub();
     CHECK_NULL_VOID(focusHub);
     focusHub->currentFocus_ = true;
     pattern_->HandleFocusEvent();
     auto host = pattern_->GetHost();
     CHECK_NULL_VOID(host);
     auto context = host->GetContext();
     CHECK_NULL_VOID(context);
     context->safeAreaManager_->keyboardAvoidMode_ = KeyBoardAvoidMode::OFFSET_WITH_CARET;
     pattern_->TriggerAvoidOnCaretChange();
     EXPECT_EQ(pattern_->GetLastCaretPos(), std::nullopt);
}
} // namespace OHOS::Ace::NG

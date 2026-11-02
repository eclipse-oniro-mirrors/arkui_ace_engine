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

#include "test/unittest/core/pattern/rich_editor/rich_editor_common_test_ng.h"
#include "core/accessibility/accessibility_manager.h"
#include "core/components/text_overlay/text_overlay_theme.h"
#include "core/common/text_field_manager_ng.h"
#include "test/mock/frameworks/core/rosen/mock_canvas.h"
#include "test/mock/frameworks/core/components_ng/render/mock_paragraph.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"
#include "test/mock/frameworks/core/common/mock_theme_manager.h"
#include "test/mock/frameworks/core/common/mock_container.h"
#include "test/mock/frameworks/base/thread/mock_task_executor.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_theme.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_model_ng.h"
#include "core/components_ng/pattern/scroll/inner/scroll_bar.h"
#include "test/mock/frameworks/core/common/mock_data_detector_mgr.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {
const std::u16string TEST_INSERT_LINE_SPACE = u" ";
} // namespace


class RichEditorSelectionTestNg : public RichEditorCommonTestNg {
public:
    void SetUp() override;
    void TearDown() override;
    RefPtr<RichEditorPattern> GetRichEditorPattern();
    static void TearDownTestSuite();
};

void RichEditorSelectionTestNg::SetUp()
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

void RichEditorSelectionTestNg::TearDown()
{
    richEditorNode_ = nullptr;
    MockParagraph::TearDown();
}

void RichEditorSelectionTestNg::TearDownTestSuite()
{
    TestNG::TearDownTestSuite();
}

RefPtr<RichEditorPattern> RichEditorSelectionTestNg::GetRichEditorPattern()
{
    CHECK_NULL_RETURN(richEditorNode_, nullptr);
    return richEditorNode_->GetPattern<RichEditorPattern>();
}

/**
 * @tc.name: SetSelection001
 * @tc.desc: test SetSelection and GetSelection
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSelectionTestNg, SetSelection001, TestSize.Level0)
{
    ASSERT_NE(richEditorNode_, nullptr);
    AddSpan(INIT_VALUE_1);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto richEditorController = richEditorPattern->GetRichEditorController();
    ASSERT_NE(richEditorController, nullptr);
    richEditorPattern->SetSelection(0, 1);
    EXPECT_EQ(richEditorPattern->textSelector_.baseOffset, -1);
    EXPECT_EQ(richEditorPattern->textSelector_.destinationOffset, -1);
    auto richEditorSelection = richEditorController->GetSelectionSpansInfo().GetSelection();
    EXPECT_EQ(richEditorSelection.selection[0], 0);
    EXPECT_EQ(richEditorSelection.selection[1], 0);

    auto focusHub = richEditorNode_->GetOrCreateFocusHub();
    ASSERT_NE(focusHub, nullptr);
    focusHub->RequestFocusImmediately();
    richEditorPattern->isEditing_ = true;
    richEditorPattern->SetSelection(0, 1);
    EXPECT_EQ(richEditorPattern->textSelector_.baseOffset, 0);
    EXPECT_EQ(richEditorPattern->textSelector_.destinationOffset, 1);
    EXPECT_EQ(richEditorPattern->caretPosition_, 1);
    auto richEditorSelection2 = richEditorController->GetSelectionSpansInfo().GetSelection();
    EXPECT_EQ(richEditorSelection2.selection[0], 0);
    EXPECT_EQ(richEditorSelection2.selection[1], 1);

    richEditorPattern->SetSelection(3, 1);
    EXPECT_EQ(richEditorPattern->textSelector_.baseOffset, -1);
    EXPECT_EQ(richEditorPattern->textSelector_.destinationOffset, -1);
    EXPECT_EQ(richEditorPattern->caretPosition_, 1);
    auto richEditorSelection3 = richEditorController->GetSelectionSpansInfo().GetSelection();
    EXPECT_EQ(richEditorSelection3.selection[0], 1);
    EXPECT_EQ(richEditorSelection3.selection[1], 1);

    richEditorPattern->SetSelection(-1, -1);
    EXPECT_EQ(richEditorPattern->textSelector_.baseOffset, 0);
    EXPECT_EQ(richEditorPattern->textSelector_.destinationOffset, 6);
    EXPECT_EQ(richEditorPattern->caretPosition_, 6);
    auto richEditorSelection4 = richEditorController->GetSelectionSpansInfo().GetSelection();
    EXPECT_EQ(richEditorSelection4.selection[0], 0);
    EXPECT_EQ(richEditorSelection4.selection[1], 6);
}

/**
 * @tc.name: SetSelection002
 * @tc.desc: test SetSelection and GetSelection
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSelectionTestNg, SetSelection002, TestSize.Level0)
{
    ASSERT_NE(richEditorNode_, nullptr);
    AddSpan(INIT_VALUE_1);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto richEditorController = richEditorPattern->GetRichEditorController();
    ASSERT_NE(richEditorController, nullptr);

    auto focusHub = richEditorNode_->GetOrCreateFocusHub();
    ASSERT_NE(focusHub, nullptr);
    focusHub->RequestFocusImmediately();
    richEditorPattern->isEditing_ = true;

    richEditorPattern->SetSelection(0, 10);
    EXPECT_EQ(richEditorPattern->textSelector_.baseOffset, 0);
    EXPECT_EQ(richEditorPattern->textSelector_.destinationOffset, 6);
    EXPECT_EQ(richEditorPattern->caretPosition_, 6);
    auto richEditorSelection5 = richEditorController->GetSelectionSpansInfo().GetSelection();
    EXPECT_EQ(richEditorSelection5.selection[0], 0);
    EXPECT_EQ(richEditorSelection5.selection[1], 6);

    richEditorPattern->SetSelection(-2, 3);
    EXPECT_EQ(richEditorPattern->textSelector_.baseOffset, 0);
    EXPECT_EQ(richEditorPattern->textSelector_.destinationOffset, 3);
    EXPECT_EQ(richEditorPattern->caretPosition_, 3);
    auto richEditorSelection6 = richEditorController->GetSelectionSpansInfo().GetSelection();
    EXPECT_EQ(richEditorSelection6.selection[0], 0);
    EXPECT_EQ(richEditorSelection6.selection[1], 3);

    richEditorPattern->SetSelection(-2, 8);
    EXPECT_EQ(richEditorPattern->textSelector_.baseOffset, 0);
    EXPECT_EQ(richEditorPattern->textSelector_.destinationOffset, 6);
    EXPECT_EQ(richEditorPattern->caretPosition_, 6);
    auto richEditorSelection7 = richEditorController->GetSelectionSpansInfo().GetSelection();
    EXPECT_EQ(richEditorSelection7.selection[0], 0);
    EXPECT_EQ(richEditorSelection7.selection[1], 6);

    richEditorPattern->SetSelection(-2, -1);
    EXPECT_EQ(richEditorPattern->textSelector_.baseOffset, -1);
    EXPECT_EQ(richEditorPattern->textSelector_.destinationOffset, -1);
    EXPECT_EQ(richEditorPattern->caretPosition_, 6);
    auto richEditorSelection8 = richEditorController->GetSelectionSpansInfo().GetSelection();
    EXPECT_EQ(richEditorSelection8.selection[0], 6);
    EXPECT_EQ(richEditorSelection8.selection[1], 6);

    richEditorPattern->SetSelection(1, 3);
    EXPECT_EQ(richEditorPattern->textSelector_.baseOffset, 1);
    EXPECT_EQ(richEditorPattern->textSelector_.destinationOffset, 3);
    EXPECT_EQ(richEditorPattern->caretPosition_, 3);
    auto richEditorSelection9 = richEditorController->GetSelectionSpansInfo().GetSelection();
    EXPECT_EQ(richEditorSelection9.selection[0], 1);
    EXPECT_EQ(richEditorSelection9.selection[1], 3);
}

/**
 * @tc.name: SetSelection003
 * @tc.desc: test SetSelection and GetSelection
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSelectionTestNg, SetSelection003, TestSize.Level0)
{
    ASSERT_NE(richEditorNode_, nullptr);
    AddSpan(INIT_VALUE_1);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto focusHub = richEditorNode_->GetOrCreateFocusHub();
    ASSERT_NE(focusHub, nullptr);
    focusHub->RequestFocusImmediately();
    richEditorPattern->isEditing_ = true;
    richEditorPattern->SetSelection(0, 1);
    auto richEditorController = richEditorPattern->GetRichEditorController();
    ASSERT_NE(richEditorController, nullptr);
    /**
     * @tc.step: step1. Empty text calls the setSelection interface.
     * @tc.expected: The interface exits normally, but it does not take effect
     */
    ClearSpan();
    richEditorPattern->SetSelection(1, 3);
    EXPECT_EQ(richEditorPattern->textSelector_.baseOffset, -1);
    EXPECT_EQ(richEditorPattern->textSelector_.destinationOffset, -1);
    EXPECT_EQ(richEditorPattern->caretPosition_, 0);
    auto richEditorSelection = richEditorController->GetSelectionSpansInfo().GetSelection();
    EXPECT_EQ(richEditorSelection.selection[0], 0);
    EXPECT_EQ(richEditorSelection.selection[1], 0);
    /**
     * @tc.step: step2. Extra-long text scenes.
     * @tc.expected: A portion of the selected text is not displayed, but the selection range can be updated
     * successfully
     */
    AddSpan(INIT_VALUE_3);
    SizeF sizeF(10.0f, 10.0f);
    richEditorNode_->GetGeometryNode()->SetContentSize(sizeF);
    richEditorPattern->SetSelection(15, 30);
    EXPECT_EQ(richEditorPattern->textSelector_.baseOffset, 15);
    EXPECT_EQ(richEditorPattern->textSelector_.destinationOffset, 30);
    EXPECT_EQ(richEditorPattern->caretPosition_, 30);
    auto richEditorSelection2 = richEditorController->GetSelectionSpansInfo().GetSelection();
    EXPECT_EQ(richEditorSelection2.selection[0], 15);
    EXPECT_EQ(richEditorSelection2.selection[1], 30);
    auto resultObject = richEditorSelection2.resultObjects.front();
    EXPECT_EQ(resultObject.valueString, INIT_VALUE_3);
    EXPECT_EQ(resultObject.offsetInSpan[0], 15);
    EXPECT_EQ(resultObject.offsetInSpan[1], 30);
}

/**
 * @tc.name: SetSelection004
 * @tc.desc: test SetSelection
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSelectionTestNg, SetSelection004, TestSize.Level0)
{
    /**
     * @tc.step: step1. Add text span and get richeditor pattern.
     */
    AddSpan(INIT_VALUE_1);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);

    /**
     * @tc.step: step2. Request focus.
     */
    auto focusHub = richEditorNode_->GetOrCreateFocusHub();
    focusHub->RequestFocusImmediately();
    richEditorPattern->isEditing_ = true;

    /**
     * @tc.step: step3. Create a scene where the text menu has popped up.
     */
    richEditorPattern->OnModifyDone();
    richEditorPattern->textSelector_.Update(0, 1);
    richEditorPattern->CalculateHandleOffsetAndShowOverlay();
    richEditorPattern->ShowSelectOverlay(
        richEditorPattern->textSelector_.firstHandle, richEditorPattern->textSelector_.secondHandle, false);
    EXPECT_TRUE(richEditorPattern->SelectOverlayIsOn());

    /**
     * @tc.step: step4. Call SetSelection with menu pop up
     * @tc.expected: Text is selected and the menu still pop up
     */
    int32_t start = -1;
    int32_t end = -1;
    SelectionOptions options;
    options.menuPolicy = MenuPolicy::DEFAULT;
    richEditorPattern->SetSelection(start, end, options);

    /**
     * @tc.step: step5. Call SetSelection with forward selection
     * @tc.expected: Cursor at start position
     */
    richEditorPattern->SetSelection(start, end, options, true);
    EXPECT_EQ(richEditorPattern->GetCaretPosition(), 0);

    /**
     * @tc.step: step6. Call SetSelection with backward selection
     * @tc.expected: Cursor at end position
     */
    richEditorPattern->SetSelection(start, end, options, false);
    EXPECT_EQ(richEditorPattern->GetCaretPosition(), INIT_VALUE_1.length());

    ClearSpan();
    EXPECT_FALSE(richEditorPattern->SelectOverlayIsOn());
    EXPECT_EQ(richEditorPattern->textSelector_.baseOffset, 0);
    EXPECT_EQ(richEditorPattern->textSelector_.destinationOffset, INIT_VALUE_1.length());
}

/**
 * @tc.name: SetSelection005
 * @tc.desc: test SetSelection
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSelectionTestNg, SetSelection005, TestSize.Level0)
{
    /**
     * @tc.step: step1. Add text span and get richeditor pattern.
     */
    AddSpan(INIT_VALUE_1);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);

    /**
     * @tc.step: step2. Request focus.
     */
    auto focusHub = richEditorNode_->GetOrCreateFocusHub();
    focusHub->RequestFocusImmediately();
    richEditorPattern->isEditing_ = true;

    /**
     * @tc.step: step3. Call SetSelection with no menu
     * @tc.expected: Text is selected and the menu doesn't pop up
     */
    int32_t start = 0;
    int32_t end = 1;
    SelectionOptions options;
    options.menuPolicy = MenuPolicy::DEFAULT;
    richEditorPattern->OnModifyDone();
    richEditorPattern->SetSelection(start, end, options);

    /**
     * @tc.step: step4. Call SetSelection with forward selection
     * @tc.expected: Cursor at start position
     */
    richEditorPattern->SetSelection(start, end, options, true);
    EXPECT_EQ(richEditorPattern->GetCaretPosition(), start);

    /**
     * @tc.step: step5. Call SetSelection with backward selection
     * @tc.expected: Cursor at end position
     */
    richEditorPattern->SetSelection(start, end, options, false);
    EXPECT_EQ(richEditorPattern->GetCaretPosition(), end);

    ClearSpan();
    EXPECT_FALSE(richEditorPattern->SelectOverlayIsOn());
    EXPECT_EQ(richEditorPattern->textSelector_.baseOffset, start);
    EXPECT_EQ(richEditorPattern->textSelector_.destinationOffset, end);
}

/**
 * @tc.name: SetSelection006
 * @tc.desc: test SetSelection
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSelectionTestNg, SetSelection006, TestSize.Level0)
{
    /**
     * @tc.step: step1. Add text span and get richeditor pattern.
     */
    AddSpan(INIT_VALUE_1);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);

    /**
     * @tc.step: step2. Request focus.
     */
    auto focusHub = richEditorNode_->GetOrCreateFocusHub();
    focusHub->RequestFocusImmediately();
    richEditorPattern->isEditing_ = true;

    /**
     * @tc.step: step3. Call SetSelection with no menu
     * @tc.expected: Text is selected and the menu doesn't pop up
     */
    int32_t start = 0;
    int32_t end = 1;
    SelectionOptions options;
    options.menuPolicy = MenuPolicy::HIDE;
    richEditorPattern->OnModifyDone();
    richEditorPattern->SetSelection(start, end, options);
    ClearSpan();
    EXPECT_FALSE(richEditorPattern->SelectOverlayIsOn());
    EXPECT_EQ(richEditorPattern->textSelector_.baseOffset, start);
    EXPECT_EQ(richEditorPattern->textSelector_.destinationOffset, end);
}

/**
 * @tc.name: SetSelection007
 * @tc.desc: test SetSelection
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSelectionTestNg, SetSelection007, TestSize.Level0)
{
    /**
     * @tc.step: step1. Add text span and get richeditor pattern.
     */
    AddSpan(INIT_VALUE_1);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);

    /**
     * @tc.step: step2. Request focus.
     */
    auto focusHub = richEditorNode_->GetOrCreateFocusHub();
    focusHub->RequestFocusImmediately();
    richEditorPattern->isEditing_ = true;

    /**
     * @tc.step: step3. Create a scene where the text menu has popped up.
     */
    richEditorPattern->OnModifyDone();
    richEditorPattern->textSelector_.Update(0, 1);
    richEditorPattern->CalculateHandleOffsetAndShowOverlay();
    richEditorPattern->ShowSelectOverlay(
        richEditorPattern->textSelector_.firstHandle, richEditorPattern->textSelector_.secondHandle, false);
    EXPECT_TRUE(richEditorPattern->SelectOverlayIsOn());

    /**
     * @tc.step: step4. Call SetSelection with menu pop up.
     * @tc.expected: Text is selected and menu doesn't pop up.
     */
    int32_t start = -1;
    int32_t end = -1;
    SelectionOptions options;
    options.menuPolicy = MenuPolicy::HIDE;
    richEditorPattern->SetSelection(start, end, options);
    ClearSpan();
    EXPECT_FALSE(richEditorPattern->SelectOverlayIsOn());
    EXPECT_EQ(richEditorPattern->textSelector_.baseOffset, 0);
    EXPECT_EQ(richEditorPattern->textSelector_.destinationOffset, INIT_VALUE_1.length());
}

/**
 * @tc.name: SetSelection008
 * @tc.desc: test SetSelection
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSelectionTestNg, SetSelection008, TestSize.Level0)
{
    /**
     * @tc.step: step1. Add text span and get richeditor pattern.
     */
    AddSpan(INIT_VALUE_1);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);

    /**
     * @tc.step: step2. Request focus.
     */
    auto focusHub = richEditorNode_->GetOrCreateFocusHub();
    focusHub->RequestFocusImmediately();
    richEditorPattern->isEditing_ = true;

    /**
     * @tc.step: step3. Call SetSelection with no menu
     * @tc.expected: Text is selected and the menu pop up
     */
    int32_t start = 0;
    int32_t end = 1;
    SelectionOptions options;
    options.menuPolicy = MenuPolicy::SHOW;
    richEditorPattern->OnModifyDone();
    richEditorPattern->SetSelection(start, end, options);
    ClearSpan();
    EXPECT_FALSE(richEditorPattern->SelectOverlayIsOn());
    EXPECT_EQ(richEditorPattern->textSelector_.baseOffset, start);
    EXPECT_EQ(richEditorPattern->textSelector_.destinationOffset, end);
}

/**
 * @tc.name: SetSelection009
 * @tc.desc: test Set Selection
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSelectionTestNg, SetSelection009, TestSize.Level0)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto richEditorController = richEditorPattern->GetRichEditorController();
    ASSERT_NE(richEditorController, nullptr);
    TextStyle style;
    style.SetLineHeight(LINE_HEIGHT_VALUE);
    style.SetLetterSpacing(LETTER_SPACING);
    style.SetFontFeatures(TEXT_FONTFEATURE);
    TextSpanOptions options;
    options.value = INIT_VALUE_1;
    options.style = style;
    richEditorController->AddTextSpan(options);
    AddSpan(INIT_VALUE_1);
    richEditorPattern->SetSelection(1, 3);
    auto info1 = richEditorController->GetSpansInfo(1, 2);
    ASSERT_NE(info1.selection_.resultObjects.size(), 0);
    EXPECT_EQ(info1.selection_.resultObjects.front().textStyle.lineHeight, LINE_HEIGHT_VALUE.ConvertToVp());
    EXPECT_EQ(info1.selection_.resultObjects.front().textStyle.letterSpacing, LETTER_SPACING.ConvertToVp());
    for (const auto& pair : info1.selection_.resultObjects.front().textStyle.fontFeature) {
        EXPECT_EQ(pair.first, "subs");
        EXPECT_EQ(pair.second, 1);
    }
    ClearSpan();
}

/**
 * @tc.name: SetSelection010
 * @tc.desc: test Set Selection
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSelectionTestNg, SetSelection010, TestSize.Level0)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto richEditorController = richEditorPattern->GetRichEditorController();
    ASSERT_NE(richEditorController, nullptr);
    TextStyle style;
    style.SetFontFeatures(TEXT_FONTFEATURE_2);
    TextSpanOptions options;
    options.value = INIT_VALUE_1;
    options.style = style;
    richEditorController->AddTextSpan(options);
    AddSpan(INIT_VALUE_1);
    richEditorPattern->SetSelection(1, 3);
    auto info1 = richEditorController->GetSpansInfo(1, 2);
    ASSERT_NE(info1.selection_.resultObjects.size(), 0);
    for (const auto& pair : info1.selection_.resultObjects.front().textStyle.fontFeature) {
        EXPECT_EQ(pair.first, "subs");
        EXPECT_EQ(pair.second, 0);
    }
    ClearSpan();
}

/**
 * @tc.name: SetSelection011
 * @tc.desc: test SetSelection
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSelectionTestNg, SetSelection011, TestSize.Level0)
{
    /**
     * @tc.step: step1. Add text span and get richeditor pattern.
     */
    AddSpan(INIT_VALUE_1);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);

    /**
     * @tc.step: step2. Request focus.
     */
    auto focusHub = richEditorNode_->GetOrCreateFocusHub();
    focusHub->RequestFocusImmediately();
    richEditorPattern->isEditing_ = true;

    /**
     * @tc.step: step3. Create a scene where the text menu has popped up.
     */
    richEditorPattern->OnModifyDone();
    richEditorPattern->textSelector_.Update(0, 1);
    richEditorPattern->CalculateHandleOffsetAndShowOverlay();
    richEditorPattern->ShowSelectOverlay(
        richEditorPattern->textSelector_.firstHandle, richEditorPattern->textSelector_.secondHandle, false);
    EXPECT_TRUE(richEditorPattern->SelectOverlayIsOn());

    /**
     * @tc.step: step4. Call SetSelection with menu pop up.
     * @tc.expected: Text is selected and menu pop up.
     */
    int32_t start = -1;
    int32_t end = -1;
    SelectionOptions options;
    options.menuPolicy = MenuPolicy::SHOW;
    richEditorPattern->SetSelection(start, end, options);
    ClearSpan();
    EXPECT_FALSE(richEditorPattern->SelectOverlayIsOn());
    EXPECT_EQ(richEditorPattern->textSelector_.baseOffset, 0);
    EXPECT_EQ(richEditorPattern->textSelector_.destinationOffset, INIT_VALUE_1.length());
}

/**
 * @tc.name: SetSelection012
 * @tc.desc: test SetSelection
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSelectionTestNg, SetSelection012, TestSize.Level0)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    richEditorPattern->DumpInfo();

    auto pipeline = PipelineContext::GetCurrentContext();
    auto theme = AceType::MakeRefPtr<MockThemeManager>();
    pipeline->SetThemeManager(theme);

    richEditorPattern->isTextChange_ = false;
    EXPECT_EQ(richEditorPattern->IsShowHandle(), false);

    auto manager = AceType::MakeRefPtr<TextFieldManagerNG>();
    richEditorPattern->ScrollToSafeArea();
    EXPECT_EQ(LessNotEqual(manager->GetHeight(), 800.0f), true);

    richEditorPattern->InitScrollablePattern();
    EXPECT_EQ(richEditorPattern->GetScrollBar(), true);

    richEditorPattern->overlayMod_ = AceType::MakeRefPtr<TextOverlayModifier>();
    richEditorPattern->InitScrollablePattern();
    EXPECT_EQ(richEditorPattern->GetScrollBar(), true);

    Offset Offset = {1, 4};
    richEditorPattern->isTextChange_ = true;
    richEditorPattern->UpdateTextFieldManager(Offset, 1.0f);
    EXPECT_EQ(richEditorPattern->HasFocus(), false);

    richEditorPattern->isTextChange_ = false;
    richEditorPattern->UpdateTextFieldManager(Offset, 1.0f);
    EXPECT_EQ(richEditorPattern->HasFocus(), false);

    richEditorPattern->caretUpdateType_ = CaretUpdateType::DOUBLE_CLICK;
    richEditorPattern->sourceType_ = SourceType::MOUSE;
    int32_t index = 1;
    richEditorPattern->MouseDoubleClickParagraphEnd(index);
    EXPECT_NE(richEditorPattern->GetParagraphEndPosition(index), index);

    SelectionOptions options;
    options.menuPolicy = MenuPolicy::SHOW;
    int32_t start = 1;
    int32_t end = 3;
    richEditorPattern->SetSelection(start, end, options);
    EXPECT_NE(richEditorPattern->textSelector_.GetStart(), start);

    options.menuPolicy = MenuPolicy::HIDE;
    richEditorPattern->SetSelection(start, end, options);
    EXPECT_NE(richEditorPattern->textSelector_.GetEnd(), end);

    options.menuPolicy = MenuPolicy::DEFAULT;
    richEditorPattern->SetSelection(start, end, options);
    EXPECT_NE(richEditorPattern->textSelector_.GetEnd(), end);
}

/**
 * @tc.name: SetSelection001
 * @tc.desc: test SetSelection
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSelectionTestNg, SetSelection013, TestSize.Level0)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    richEditorPattern->CreateNodePaintMethod();
    EXPECT_EQ(richEditorPattern->contentMod_, nullptr);
    EXPECT_NE(richEditorPattern->overlayMod_, nullptr);
    auto focusHub = richEditorPattern->GetFocusHub();
    EXPECT_NE(focusHub, nullptr);
    focusHub->currentFocus_ = true;

    richEditorPattern->previewTextRecord_.previewContent = u"test";
    richEditorPattern->previewTextRecord_.previewTextHasStarted = true;
    richEditorPattern->previewTextRecord_.startOffset = 1;
    richEditorPattern->previewTextRecord_.endOffset = 10;

    int32_t start = 1;
    int32_t end = 3;
    SelectionOptions options;
    options.menuPolicy = MenuPolicy::DEFAULT;
    richEditorPattern->SetSelection(start, end, options, true);
    EXPECT_NE(richEditorPattern->HasFocus(), false);
}

/**
 * @tc.name: SetSelection002
 * @tc.desc: test SetSelection
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSelectionTestNg, SetSelection014, TestSize.Level0)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    richEditorPattern->CreateNodePaintMethod();
    EXPECT_EQ(richEditorPattern->contentMod_, nullptr);
    EXPECT_NE(richEditorPattern->overlayMod_, nullptr);
    auto focusHub = richEditorPattern->GetFocusHub();
    EXPECT_NE(focusHub, nullptr);
    focusHub->currentFocus_ = true;

    richEditorPattern->isSpanStringMode_ = true;
    richEditorPattern->styledString_ = AccessibilityManager::MakeRefPtr<MutableSpanString>(u"test123456");
    int32_t start = -1;
    int32_t end = -1;
    SelectionOptions options;
    options.menuPolicy = MenuPolicy::DEFAULT;
    richEditorPattern->SetSelection(start, end, options, true);
    EXPECT_NE(richEditorPattern->HasFocus(), false);
}

/**
 * @tc.name: SetSelection003
 * @tc.desc: test SetSelection
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSelectionTestNg, SetSelection015, TestSize.Level0)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    richEditorPattern->CreateNodePaintMethod();
    EXPECT_EQ(richEditorPattern->contentMod_, nullptr);
    EXPECT_NE(richEditorPattern->overlayMod_, nullptr);
    auto focusHub = richEditorPattern->GetFocusHub();
    EXPECT_NE(focusHub, nullptr);
    focusHub->currentFocus_ = true;

    richEditorPattern->isSpanStringMode_ = true;
    richEditorPattern->styledString_ = AccessibilityManager::MakeRefPtr<MutableSpanString>(u"test123456");
    richEditorPattern->textSelector_ = TextSelector(0, 6);

    int32_t start = 1;
    int32_t end = 5;
    SelectionOptions options;
    options.menuPolicy = MenuPolicy::DEFAULT;
    richEditorPattern->SetSelection(start, end, options, true);
    EXPECT_NE(richEditorPattern->HasFocus(), false);
}

/**
 * @tc.name: SetSelection004
 * @tc.desc: test SetSelection
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSelectionTestNg, SetSelection016, TestSize.Level0)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    richEditorPattern->CreateNodePaintMethod();
    EXPECT_EQ(richEditorPattern->contentMod_, nullptr);
    EXPECT_NE(richEditorPattern->overlayMod_, nullptr);
    auto focusHub = richEditorPattern->GetFocusHub();
    EXPECT_NE(focusHub, nullptr);
    focusHub->currentFocus_ = true;

    richEditorPattern->isSpanStringMode_ = true;
    richEditorPattern->styledString_ = AccessibilityManager::MakeRefPtr<MutableSpanString>(u"test123456");
    richEditorPattern->textSelector_ = TextSelector(2, 4);
    auto pipeline = PipelineContext::GetCurrentContext();
    auto theme = AceType::MakeRefPtr<MockThemeManager>();
    EXPECT_CALL(*theme, GetTheme(_)).WillRepeatedly(Return(AceType::MakeRefPtr<RichEditorTheme>()));
    EXPECT_CALL(*theme, GetTheme(_, _)).WillRepeatedly(Return(AceType::MakeRefPtr<RichEditorTheme>()));
    pipeline->themeManager_ = theme;

    richEditorPattern->customKeyboardBuilder_ = []() {};
    richEditorPattern->DumpInfo();
    richEditorPattern->isTextChange_ = false;
    EXPECT_EQ(richEditorPattern->IsShowHandle(), true);

    int32_t start = 2;
    int32_t end = 5;
    SelectionOptions options;
    options.menuPolicy = MenuPolicy::DEFAULT;
    richEditorPattern->SetSelection(start, end, options, true);
    EXPECT_NE(richEditorPattern->HasFocus(), false);
}

/**
 * @tc.name: SetSelection005
 * @tc.desc: test SetSelection
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSelectionTestNg, SetSelection017, TestSize.Level0)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    richEditorPattern->CreateNodePaintMethod();
    EXPECT_EQ(richEditorPattern->contentMod_, nullptr);
    EXPECT_NE(richEditorPattern->overlayMod_, nullptr);
    auto focusHub = richEditorPattern->GetFocusHub();
    EXPECT_NE(focusHub, nullptr);
    focusHub->currentFocus_ = true;

    richEditorPattern->isSpanStringMode_ = true;
    richEditorPattern->styledString_ = AccessibilityManager::MakeRefPtr<MutableSpanString>(u"test123456");

    richEditorPattern->textSelector_ = TextSelector(2, 4);

    auto pipeline = PipelineContext::GetCurrentContext();
    auto theme = AceType::MakeRefPtr<MockThemeManager>();
    EXPECT_CALL(*theme, GetTheme(_)).WillRepeatedly(Return(AceType::MakeRefPtr<RichEditorTheme>()));
    EXPECT_CALL(*theme, GetTheme(_, _)).WillRepeatedly(Return(AceType::MakeRefPtr<RichEditorTheme>()));
    pipeline->themeManager_ = theme;

    richEditorPattern->customKeyboardBuilder_ = []() {};
    richEditorPattern->DumpInfo();
    richEditorPattern->isTextChange_ = false;
    EXPECT_EQ(richEditorPattern->IsShowHandle(), true);

    int32_t start = 2;
    int32_t end = 5;
    SelectionOptions options;
    options.menuPolicy = MenuPolicy::HIDE;
    richEditorPattern->SetSelection(start, end, options, true);
    EXPECT_NE(richEditorPattern->HasFocus(), false);
}

/**
 * @tc.name: SetSelection006
 * @tc.desc: test SetSelection
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSelectionTestNg, SetSelection018, TestSize.Level0)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    richEditorPattern->CreateNodePaintMethod();
    EXPECT_EQ(richEditorPattern->contentMod_, nullptr);
    EXPECT_NE(richEditorPattern->overlayMod_, nullptr);
    auto focusHub = richEditorPattern->GetFocusHub();
    EXPECT_NE(focusHub, nullptr);
    focusHub->currentFocus_ = true;

    richEditorPattern->isSpanStringMode_ = true;
    richEditorPattern->styledString_ = AccessibilityManager::MakeRefPtr<MutableSpanString>(u"test123456");
    richEditorPattern->textSelector_ = TextSelector(2, 4);

    auto pipeline = PipelineContext::GetCurrentContext();
    auto theme = AceType::MakeRefPtr<MockThemeManager>();
    EXPECT_CALL(*theme, GetTheme(_)).WillRepeatedly(Return(AceType::MakeRefPtr<RichEditorTheme>()));
    EXPECT_CALL(*theme, GetTheme(_, _)).WillRepeatedly(Return(AceType::MakeRefPtr<RichEditorTheme>()));
    pipeline->themeManager_ = theme;

    richEditorPattern->customKeyboardBuilder_ = []() {};
    richEditorPattern->DumpInfo();
    richEditorPattern->isTextChange_ = false;
    EXPECT_EQ(richEditorPattern->IsShowHandle(), true);

    int32_t start = 2;
    int32_t end = 5;
    SelectionOptions options;
    options.menuPolicy = MenuPolicy::SHOW;
    richEditorPattern->SetSelection(start, end, options, false);
    EXPECT_NE(richEditorPattern->HasFocus(), false);
}

/**
 * @tc.name: SetSelection019
 * @tc.desc: test SetSelection
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSelectionTestNg, SetSelection019, TestSize.Level0)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    richEditorPattern->CreateNodePaintMethod();
    ASSERT_EQ(richEditorPattern->contentMod_, nullptr);
    ASSERT_NE(richEditorPattern->overlayMod_, nullptr);
    auto focusHub = richEditorPattern->GetFocusHub();
    ASSERT_NE(focusHub, nullptr);
    focusHub->currentFocus_ = true;

    richEditorPattern->previewTextRecord_.previewContent = u"test";
    richEditorPattern->previewTextRecord_.startOffset = 1;
    richEditorPattern->previewTextRecord_.endOffset = 10;

    int32_t start = -1;
    int32_t end = 1;
    SelectionOptions options;
    options.menuPolicy = MenuPolicy::DEFAULT;
    richEditorPattern->SetSelection(start, end, options, true);
    EXPECT_TRUE(richEditorPattern->HasFocus());
    richEditorPattern->textSelector_.baseOffset = -1;
    richEditorPattern->SetSelection(start, end, options, true);
    EXPECT_TRUE(richEditorPattern->HasFocus());
}

/**
 * @tc.name: SetSelection020
 * @tc.desc: test SetSelection
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSelectionTestNg, SetSelection020, TestSize.Level0)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    richEditorPattern->CreateNodePaintMethod();
    ASSERT_EQ(richEditorPattern->contentMod_, nullptr);
    ASSERT_NE(richEditorPattern->overlayMod_, nullptr);
    auto focusHub = richEditorPattern->GetFocusHub();
    ASSERT_NE(focusHub, nullptr);
    focusHub->currentFocus_ = true;

    std::u16string content = u"TEST123";
    richEditorPattern->isSpanStringMode_ = true;
    richEditorPattern->styledString_ = AceType::MakeRefPtr<MutableSpanString>(content);
    richEditorPattern->previewTextRecord_.previewContent = u"test";
    richEditorPattern->previewTextRecord_.startOffset = -1;
    richEditorPattern->previewTextRecord_.endOffset = 10;

    int32_t start = -1;
    int32_t end = -1;
    SelectionOptions options;
    options.menuPolicy = MenuPolicy::DEFAULT;
    richEditorPattern->textSelector_.baseOffset = -1;
    richEditorPattern->textSelector_.destinationOffset = -1;
    richEditorPattern->SetSelection(start, end, options, true);
    EXPECT_TRUE(richEditorPattern->HasFocus());
    richEditorPattern->textSelector_.baseOffset = -1;
    richEditorPattern->textSelector_.destinationOffset = -1;
    richEditorPattern->SetSelection(start, end, options, true);
    EXPECT_TRUE(richEditorPattern->HasFocus());
}

/**
 * @tc.name: SetSelection021
 * @tc.desc: test set selection
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSelectionTestNg, SetSelection021, TestSize.Level0)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto richEditorController = richEditorPattern->GetRichEditorController();
    ASSERT_NE(richEditorController, nullptr);
    AddSpan(INIT_VALUE_1);
    auto focusHub = richEditorNode_->GetOrCreateFocusHub();
    ASSERT_NE(focusHub, nullptr);
    focusHub->RequestFocusImmediately();
    richEditorPattern->isEditing_ = true;
    richEditorController->SetSelection(0, 2);
    EXPECT_EQ(richEditorPattern->textSelector_.GetTextStart(), 0);
    EXPECT_EQ(richEditorPattern->textSelector_.GetTextEnd(), 2);
    richEditorController->SetSelection(-1, -1);
    EXPECT_EQ(richEditorPattern->textSelector_.GetTextStart(), 0);
    EXPECT_EQ(richEditorPattern->textSelector_.GetTextEnd(), 6);
}

/**
 * @tc.name: InitSelection001
 * @tc.desc: test InitSelection
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSelectionTestNg, InitSelection001, TestSize.Level0)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    richEditorPattern->textForDisplay_ = u"test";
    richEditorPattern->InitSelection(Offset(0, 0));
    EXPECT_EQ(richEditorPattern->textSelector_.baseOffset, 0);
    EXPECT_EQ(richEditorPattern->textSelector_.destinationOffset, 0);
}

/**
 * @tc.name: InitSelection002
 * @tc.desc: test InitSelection
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSelectionTestNg, InitSelection002, TestSize.Level0)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    richEditorPattern->textForDisplay_ = u"test";
    richEditorPattern->spans_.push_front(AceType::MakeRefPtr<SpanItem>());
    richEditorPattern->spans_.front()->position = 3;
    richEditorPattern->InitSelection(Offset(0, 0));
    EXPECT_EQ(richEditorPattern->textSelector_.baseOffset, 0);
    EXPECT_EQ(richEditorPattern->textSelector_.destinationOffset, 0);
}

/**
 * @tc.name: InitSelection003
 * @tc.desc: test InitSelection003
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSelectionTestNg, InitSelection003, TestSize.Level0)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    richEditorPattern->CreateNodePaintMethod();
    EXPECT_EQ(richEditorPattern->contentMod_, nullptr);
    EXPECT_NE(richEditorPattern->overlayMod_, nullptr);
    AddSpan("ab\n\nab");

    auto paragraph = MockParagraph::GetOrCreateMockParagraph();
    ASSERT_NE(paragraph, nullptr);

    PositionWithAffinity positionWithAffinity(3, TextAffinity::DOWNSTREAM);
    EXPECT_CALL(*paragraph, GetGlyphPositionAtCoordinate(_)).WillRepeatedly(Return(positionWithAffinity));

    Offset touchOffset = Offset(0.0f, 0.0f);
    richEditorPattern->previewLongPress_ = true;
    richEditorPattern->InitSelection(touchOffset);

    EXPECT_EQ(richEditorPattern->textSelector_.baseOffset, 0);
    EXPECT_EQ(richEditorPattern->textSelector_.destinationOffset, 0);
}

/**
 * @tc.name: InitSelection004
 * @tc.desc: test InitSelection
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSelectionTestNg, InitSelection004, TestSize.Level0)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto paragraph = MockParagraph::GetOrCreateMockParagraph();
    ASSERT_NE(paragraph, nullptr);
    richEditorPattern->previewLongPress_ = true;
    auto offset = Offset(0, 0);
    AddSpan("hello1");
    richEditorPattern->previewLongPress_ = false;
    richEditorPattern->editingLongPress_ = false;
    PositionWithAffinity positionWithAffinity(3, TextAffinity::DOWNSTREAM);
    EXPECT_CALL(*paragraph, GetGlyphPositionAtCoordinate(_)).WillRepeatedly(Return(positionWithAffinity));
    ParagraphManager::ParagraphInfo info;
    info.paragraph = paragraph;
    richEditorPattern->paragraphs_.AddParagraph(std::move(info));

    richEditorPattern->InitSelection(offset);
    EXPECT_NE(richEditorPattern->textSelector_.baseOffset, 0);
    EXPECT_NE(richEditorPattern->textSelector_.destinationOffset, 0);
}

/**
 * @tc.name: AdjustPlaceholderSelection001
 * @tc.desc: test AdjustPlaceholderSelection
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSelectionTestNg, AdjustPlaceholderSelection001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. init and call function.
     */
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    richEditorPattern->CreateNodePaintMethod();
    EXPECT_EQ(richEditorPattern->contentMod_, nullptr);
    EXPECT_NE(richEditorPattern->overlayMod_, nullptr);
    AddSpan(INIT_VALUE_1);
    OHOS::Ace::RefPtr<OHOS::Ace::NG::SpanItem> spanItem1 = AceType::MakeRefPtr<ImageSpanItem>();
    richEditorPattern->spans_.emplace_back(spanItem1);
    OHOS::Ace::RefPtr<OHOS::Ace::NG::SpanItem> spanItem2 = AceType::MakeRefPtr<PlaceholderSpanItem>();
    richEditorPattern->spans_.emplace_back(spanItem2);
    /**
     * @tc.steps: step2. change parameter and call function.
     */
    int32_t start = 10;
    int32_t end = 20;
    Offset touchPos(11.0f, 11.0f);
    richEditorPattern->AdjustPlaceholderSelection(start, end, touchPos);
    EXPECT_NE(start, end);
}

/**
 * @tc.name: AdjustPlaceholderSelection002
 * @tc.desc: test AdjustPlaceholderSelection
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSelectionTestNg, AdjustPlaceholderSelection002, TestSize.Level0)
{
    /**
     * @tc.steps: step1. init and call function.
     */
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    richEditorPattern->CreateNodePaintMethod();
    EXPECT_EQ(richEditorPattern->contentMod_, nullptr);
    EXPECT_NE(richEditorPattern->overlayMod_, nullptr);
    AddSpan(INIT_VALUE_1);
    OHOS::Ace::RefPtr<OHOS::Ace::NG::SpanItem> spanItem1 = AceType::MakeRefPtr<ImageSpanItem>();
    richEditorPattern->spans_.emplace_back(spanItem1);
    OHOS::Ace::RefPtr<OHOS::Ace::NG::SpanItem> spanItem2 = AceType::MakeRefPtr<PlaceholderSpanItem>();
    richEditorPattern->spans_.emplace_back(spanItem2);
    OHOS::Ace::RefPtr<OHOS::Ace::NG::SpanItem> spanItem3 = AceType::MakeRefPtr<PlaceholderSpanItem>();
    richEditorPattern->spans_.emplace_back(spanItem3);
    auto paragraph = MockParagraph::GetOrCreateMockParagraph();
    ASSERT_NE(paragraph, nullptr);
    EXPECT_CALL(*paragraph, GetHeight()).WillRepeatedly(Return(300.0f));
    EXPECT_CALL(*paragraph, GetGlyphIndexByCoordinate(_, _)).WillRepeatedly(Return(6));
    TestParagraphRect paragraphRect = { .start = 0, .end = 6, .rects = { { 10.0, 20.0, 30.0, 40.0 } } };
    TestParagraphItem paragraphItem = { .start = 0, .end = 6, .testParagraphRects = { paragraphRect } };
    AddParagraph(paragraphItem);
    CaretMetricsF metrics = { { 10.0f, 50.0f }, 20.0f };
    EXPECT_CALL(*paragraph, ComputeOffsetForCaretUpstream(_, _, _))
        .WillRepeatedly(DoAll(SetArgReferee<1>(metrics), Return(true)));
    EXPECT_CALL(*paragraph, ComputeOffsetForCaretDownstream(_, _, _))
        .WillRepeatedly(DoAll(SetArgReferee<1>(metrics), Return(true)));
    TestParagraphRect paragraphRectSec = { .start = 7, .end = 12, .rects = { { 40.0, 50.0, 60.0, 70.0 } } };
    TestParagraphItem paragraphItemSec = { .start = 7, .end = 12, .testParagraphRects = { paragraphRectSec } };
    AddParagraph(paragraphItemSec);
    TestParagraphRect paragraphRectThi = { .start = 13, .end = 18, .rects = { { 80.0, 90.0, 100.0, 110.0 } } };
    TestParagraphItem paragraphItemThi = { .start = 13, .end = 18, .testParagraphRects = { paragraphRectThi } };
    AddParagraph(paragraphItemThi);
    /**
     * @tc.steps: step2. change parameter and call function.
     */
    int32_t start = 8;
    int32_t end = 10;
    Offset touchPos(8.0f, 350.0f);
    richEditorPattern->AdjustPlaceholderSelection(start, end, touchPos);

    for (auto iter : richEditorPattern->spans_) {
        iter->position = start;
    }
    richEditorPattern->AdjustPlaceholderSelection(start, end, touchPos);
    EXPECT_NE(start, end);
}

/**
 * @tc.name: AdjustWordSelection001
 * @tc.desc: test AdjustWordSelection
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSelectionTestNg, AdjustWordSelection001, TestSize.Level0)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    int32_t start = 100;
    int32_t end = 200;
    bool ret = true;
    ret = richEditorPattern->AdjustWordSelection(start, end);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: HandleClickSelection001
 * @tc.desc: test HandleClickSelection
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSelectionTestNg, HandleClickSelection001, TestSize.Level0)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    richEditorPattern->selectOverlay_->bindManager_.Reset();
    OHOS::Ace::GestureEvent info;
    EXPECT_TRUE(richEditorPattern->HandleClickSelection(info));
}

/**
 * @tc.name: IsSelectAreaVisible001
 * @tc.desc: test selectArea inVisible
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSelectionTestNg, IsSelectAreaVisible001, TestSize.Level0)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    /**
     * @tc.steps: step1. add text and paragraph
     */
    TestParagraphRect paragraphRect = { .start = 0, .end = 6, .rects = { { 0.0, 0.0, 200.0, 200.0 } } };
    TestParagraphItem paragraphItem = { .start = 0, .end = 6, .testParagraphRects = { paragraphRect } };
    AddParagraph(paragraphItem);
    richEditorPattern->textSelector_.baseOffset = 0;
    richEditorPattern->textSelector_.destinationOffset = 6;
    richEditorPattern->contentRect_ = { 0.0, 0.0, 500.0, 500.0 };
    /**
     * @tc.steps: step2. test IsSelectAreaVisible
     */
    auto res = richEditorPattern->IsSelectAreaVisible();
    EXPECT_FALSE(res);
}

/**
 * @tc.name: IsSelectAreaVisible002
 * @tc.desc: test selectArea Visible
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSelectionTestNg, IsSelectAreaVisible002, TestSize.Level0)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    /**
     * @tc.steps: step1. add text and paragraph
     */
    TestParagraphRect paragraphRect = { .start = 0, .end = 6, .rects = { { -400.0, -400.0, 200.0, 200.0 } } };
    TestParagraphItem paragraphItem = { .start = 0, .end = 6, .testParagraphRects = { paragraphRect } };
    AddParagraph(paragraphItem);
    richEditorPattern->textSelector_.baseOffset = 0;
    richEditorPattern->textSelector_.destinationOffset = 6;
    richEditorPattern->contentRect_ = { -500.0, -500.0, 500.0, 500.0 };
    /**
     * @tc.steps: step2. test IsSelectAreaVisible
     */
    auto res = richEditorPattern->IsSelectAreaVisible();
    EXPECT_TRUE(res);
}

/**
 * @tc.name: GetSelectedSpanText001
 * @tc.desc: test GetSelectedSpanText
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSelectionTestNg, GetSelectedSpanText001, TestSize.Level0)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    std::u16string ori = u"12345";

    std::vector<int> start = { -1, 0, 15 };
    std::vector<int> end = { 10, -3 };

    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 2; ++j) {
            auto ret = richEditorPattern->GetSelectedSpanText(ori, start[i], end[j]);
            EXPECT_EQ(ret, u"");
        }
    }

    auto ret = richEditorPattern->GetSelectedSpanText(ori, 0, 1);
    EXPECT_EQ(ret, u"1");
}

/**
 * @tc.name: GetSelectedSpanText002
 * @tc.desc: test get select span text
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSelectionTestNg, GetSelectedSpanText002, TestSize.Level0)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);

    auto ret = richEditorPattern->GetSelectedSpanText(INIT_VALUE_1, -1, 1);
    ret = richEditorPattern->GetSelectedSpanText(INIT_VALUE_1, -1, 10);
    ret = richEditorPattern->GetSelectedSpanText(INIT_VALUE_1, 0, 1);
    EXPECT_EQ(ret, u"h");
}

/**
 * @tc.name: CaretPositionSelectEmoji001
 * @tc.desc: test CaretPositionSelectEmoji
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSelectionTestNg, CaretPositionSelectEmoji001, TestSize.Level0)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    CaretMoveIntent direction = CaretMoveIntent::Left;
    auto ret = richEditorPattern->CaretPositionSelectEmoji(direction);
    EXPECT_EQ(ret, richEditorPattern->caretPosition_ - 1);
}

/**
 * @tc.name: CaretPositionSelectEmoji002
 * @tc.desc: test CaretPositionSelectEmoji
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSelectionTestNg, CaretPositionSelectEmoji002, TestSize.Level0)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    CaretMoveIntent direction = CaretMoveIntent::Right;
    auto ret = richEditorPattern->CaretPositionSelectEmoji(direction);
    EXPECT_EQ(ret, richEditorPattern->caretPosition_ + 1);
}

/**
 * @tc.name: CaretPositionSelectEmoji003
 * @tc.desc: test CaretPositionSelectEmoji
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSelectionTestNg, CaretPositionSelectEmoji003, TestSize.Level0)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    CaretMoveIntent direction = CaretMoveIntent::Up;
    auto ret = richEditorPattern->CaretPositionSelectEmoji(direction);
    EXPECT_EQ(ret, richEditorPattern->caretPosition_);
}

/**
 * @tc.name: GetSelectSpansPositionInfo001
 * @tc.desc: test GetSelectSpansPositionInfo
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSelectionTestNg, GetSelectSpansPositionInfo001, TestSize.Level0)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    ClearSpan();
    AddSpan(INIT_VALUE_1);
    richEditorPattern->isSpanStringMode_ = false;
    richEditorPattern->textSelector_.baseOffset = 1;
    richEditorPattern->textSelector_.destinationOffset = 2;
    richEditorPattern->HandleSelectFontStyle(KeyCode::KEY_I);
    ClearSpan();
    AddSpan(INIT_VALUE_2);
    auto spanItem = richEditorPattern->spans_.back();
    spanItem->unicode = 1;
    richEditorPattern->textSelector_.baseOffset = 1;
    richEditorPattern->textSelector_.destinationOffset = 2;
    richEditorPattern->HandleSelectFontStyle(KeyCode::KEY_I);
    ClearSpan();
    AddImageSpan();
    richEditorPattern->textSelector_.baseOffset = 0;
    richEditorPattern->textSelector_.destinationOffset = 1;
    richEditorPattern->HandleSelectFontStyle(KeyCode::KEY_I);
    ClearSpan();
    AddSpan(INIT_VALUE_1);
    richEditorPattern->textSelector_.baseOffset = 7;
    richEditorPattern->textSelector_.destinationOffset = 8;
    richEditorPattern->HandleSelectFontStyle(KeyCode::KEY_I);
    EXPECT_FALSE(richEditorPattern->hasClicked_);
    ClearSpan();
    AddSpan(INIT_VALUE_1);
    richEditorPattern->textSelector_.baseOffset = 5;
    richEditorPattern->textSelector_.destinationOffset = 6;
    richEditorPattern->HandleSelectFontStyle(KeyCode::KEY_I);
    EXPECT_FALSE(richEditorPattern->hasClicked_);
}

/**
 * @tc.name: GetSelectSpansPositionInfo002
 * @tc.desc: test GetSelectSpansPositionInfo
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSelectionTestNg, GetSelectSpansPositionInfo002, TestSize.Level0)
{
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    int32_t start;
    int32_t end;
    SpanPositionInfo startPositionSpanInfo(-1, -1, -1, -1);
    SpanPositionInfo endPositionSpanInfo(-1, -1, -1, -1);
    ClearSpan();
    AddSpan(INIT_VALUE_2);
    auto spanItem = richEditorPattern->spans_.back();
    spanItem->unicode = 1;
    start = 1;
    end = 2;
    richEditorPattern->GetSelectSpansPositionInfo(start, end, startPositionSpanInfo, endPositionSpanInfo);
    EXPECT_NE(start, 1);
    ClearSpan();
    AddSpan(INIT_VALUE_2);
    spanItem = richEditorPattern->spans_.back();
    spanItem->spanItemType = SpanItemType::CustomSpan;
    start = 1;
    end = 2;
    richEditorPattern->GetSelectSpansPositionInfo(start, end, startPositionSpanInfo, endPositionSpanInfo);
    EXPECT_NE(start, 1);
    ClearSpan();
    AddSpan(INIT_VALUE_2);
    spanItem = richEditorPattern->spans_.back();
    spanItem->rangeStart = 2;
    start = 1;
    end = 2;
    richEditorPattern->GetSelectSpansPositionInfo(start, end, startPositionSpanInfo, endPositionSpanInfo);
    ClearSpan();
    AddSpan(INIT_VALUE_2);
    spanItem = richEditorPattern->spans_.back();
    spanItem->position = 0;
    start = 1;
    end = 2;
    richEditorPattern->GetSelectSpansPositionInfo(start, end, startPositionSpanInfo, endPositionSpanInfo);
}

/**
 * @tc.name: GetSelectSpansPositionInfo003
 * @tc.desc: test GetSelectSpansPositionInfo
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSelectionTestNg, GetSelectSpansPositionInfo003, TestSize.Level0)
{
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    int32_t start;
    int32_t end;
    SpanPositionInfo startPositionSpanInfo(-1, -1, -1, -1);
    SpanPositionInfo endPositionSpanInfo(-1, -1, -1, -1);
    ClearSpan();
    AddSpan(INIT_VALUE_2);
    start = 2;
    end = 1;
    richEditorPattern->GetSelectSpansPositionInfo(start, end, startPositionSpanInfo, endPositionSpanInfo);
    ClearSpan();
    AddSpan(INIT_VALUE_1);
    auto spanItem = richEditorPattern->spans_.back();
    spanItem->position = 2;
    start = 1;
    end = 3;
    richEditorPattern->GetSelectSpansPositionInfo(start, end, startPositionSpanInfo, endPositionSpanInfo);
    ClearSpan();
    AddSpan(INIT_VALUE_1);
    start = 5;
    end = 6;
    richEditorPattern->GetSelectSpansPositionInfo(start, end, startPositionSpanInfo, endPositionSpanInfo);
    EXPECT_EQ(endPositionSpanInfo.spanIndex_, startPositionSpanInfo.spanIndex_);
}

/**
 * @tc.name: JudgeSelectType001
 * @tc.desc: test RichEditorPattern JudgeSelectType
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSelectionTestNg, JudgeSelectType001, TestSize.Level0)
{
    ASSERT_NE(richEditorNode_, nullptr);
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto paragraph = MockParagraph::GetOrCreateMockParagraph();
    ASSERT_NE(paragraph, nullptr);

    richEditorPattern->previewLongPress_ = true;
    auto offset = Offset(50.0, -80.0);
    AddSpan("hello1");
    auto selectType = richEditorPattern->JudgeSelectType(offset).second;
    EXPECT_NE(selectType, SelectType::SELECT_FORWARD);
    richEditorPattern->previewLongPress_ = false;
    richEditorPattern->editingLongPress_ = true;
    selectType = richEditorPattern->JudgeSelectType(offset).second;
    EXPECT_NE(selectType, SelectType::SELECT_FORWARD);
}

/**
 * @tc.name: AdjustSelectorForEmoji001
 * @tc.desc: Test AdjustSelectorForEmoji returns false when index out of span range.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSelectionTestNg, AdjustSelectorForEmoji001, TestSize.Level0)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    ClearSpan();
    AddSpan(u"abc");
    int32_t index = 100;
    EXPECT_FALSE(pattern->AdjustSelectorForEmoji(index, HandleType::FIRST, SelectorAdjustPolicy::INCLUDE));
    EXPECT_FALSE(pattern->AdjustSelectorForEmoji(index, HandleType::SECOND, SelectorAdjustPolicy::EXCLUDE));
}

/**
 * @tc.name: AdjustSelectorForEmoji002
 * @tc.desc: Test AdjustSelectorForEmoji returns false when content has no emoji.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSelectionTestNg, AdjustSelectorForEmoji002, TestSize.Level0)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    ClearSpan();
    AddSpan(u"plain");
    int32_t index = 2;
    EXPECT_FALSE(pattern->AdjustSelectorForEmoji(index, HandleType::FIRST, SelectorAdjustPolicy::INCLUDE));
}

/**
 * @tc.name: GetSpanType001
 * @tc.desc: Test GetSpanType returns NORMAL when index is out of range.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSelectionTestNg, GetSpanType001, TestSize.Level0)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    ClearSpan();
    AddSpan(u"text");
    EXPECT_EQ(pattern->GetSpanType(1000), SpanItemType::NORMAL);
    EXPECT_EQ(pattern->GetSpanType(0), SpanItemType::NORMAL);
}

/**
 * @tc.name: ShouldSelectAllOnInit001
 * @tc.desc: Test ShouldSelectAllOnInit various && combinations.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSelectionTestNg, ShouldSelectAllOnInit001, TestSize.Level0)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    ClearSpan();
    pattern->selectAllOnInit_ = false;
    EXPECT_FALSE(pattern->ShouldSelectAllOnInit(FocusReason::DEFAULT));
    pattern->selectAllOnInit_ = true;
    EXPECT_FALSE(pattern->ShouldSelectAllOnInit(FocusReason::DEFAULT));
    AddSpan(u"x");
    pattern->editingLongPress_ = true;
    EXPECT_FALSE(pattern->ShouldSelectAllOnInit(FocusReason::DEFAULT));
    pattern->editingLongPress_ = false;
    EXPECT_FALSE(pattern->ShouldSelectAllOnInit(FocusReason::WINDOW_FOCUS));
    EXPECT_TRUE(pattern->ShouldSelectAllOnInit(FocusReason::DEFAULT));
}

/**
 * @tc.name: SetSelectAll001
 * @tc.desc: Test SetSelectAll IF_TRUE branch (value=false clears needSelectAll_).
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSelectionTestNg, SetSelectAll001, TestSize.Level0)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->needSelectAll_ = true;
    pattern->SetSelectAll(false);
    EXPECT_FALSE(pattern->GetSelectAll());
    EXPECT_FALSE(pattern->needSelectAll_);
    pattern->SetSelectAll(true);
    EXPECT_TRUE(pattern->GetSelectAll());
}

/**
 * @tc.name: TryExecuteSelectAll001
 * @tc.desc: Test TryExecuteSelectAll early return when needSelectAll_ false.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSelectionTestNg, TryExecuteSelectAll001, TestSize.Level0)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->needSelectAll_ = false;
    pattern->TryExecuteSelectAll();
    EXPECT_FALSE(pattern->needSelectAll_);
}

/**
 * @tc.name: ResetOnInvalidSelection001
 * @tc.desc: Test ResetOnInvalidSelection both branches.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSelectionTestNg, ResetOnInvalidSelection001, TestSize.Level0)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    EXPECT_FALSE(pattern->ResetOnInvalidSelection(0, 5));
    EXPECT_TRUE(pattern->ResetOnInvalidSelection(5, 5));
}

/**
 * @tc.name: IsSelectEmpty001
 * @tc.desc: Test IsSelectEmpty returns true for empty paragraphs.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSelectionTestNg, IsSelectEmpty001, TestSize.Level0)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    EXPECT_TRUE(pattern->IsSelectEmpty(0, 0));
}

/**
 * @tc.name: AdjustSelectorForSymbol001
 * @tc.desc: Test AdjustSelectorForSymbol returns false when span iter is end.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSelectionTestNg, AdjustSelectorForSymbol001, TestSize.Level0)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    ClearSpan();
    AddSpan(u"abc");
    int32_t index = 1000;
    EXPECT_FALSE(pattern->AdjustSelectorForSymbol(index, HandleType::FIRST, SelectorAdjustPolicy::INCLUDE));
    index = 0;
    EXPECT_FALSE(pattern->AdjustSelectorForSymbol(index, HandleType::FIRST, SelectorAdjustPolicy::INCLUDE));
}

/**
 * @tc.name: GetEmojiRelation001
 * @tc.desc: Test GetEmojiRelation returns NO_EMOJI when index out of range.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSelectionTestNg, GetEmojiRelation001, TestSize.Level0)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    ClearSpan();
    AddSpan(u"text");
    EXPECT_EQ(pattern->GetEmojiRelation(1000), EmojiRelation::NO_EMOJI);
    EXPECT_EQ(pattern->GetEmojiRelation(0), EmojiRelation::NO_EMOJI);
}

/**
 * @tc.name: GetSpanIter001
 * @tc.desc: Test GetSpanIter returns end iterator for out-of-range index.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSelectionTestNg, GetSpanIter001, TestSize.Level0)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    ClearSpan();
    AddSpan(u"abc");
    auto it = pattern->GetSpanIter(1000);
    EXPECT_TRUE(it == pattern->spans_.end());
    auto it2 = pattern->GetSpanIter(0);
    EXPECT_FALSE(it2 == pattern->spans_.end());
}

/**
 * @tc.name: AdjustSelector001
 * @tc.desc: Test AdjustSelector(start,end) delegates to per-handle variants.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSelectionTestNg, AdjustSelector001, TestSize.Level0)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    ClearSpan();
    AddSpan(u"hello");
    int32_t start = 0;
    int32_t end = 3;
    pattern->AdjustSelector(start, end, SelectorAdjustPolicy::INCLUDE);
    EXPECT_GE(start, 0);
}

/**
 * @tc.name: HandleSelectExtend001
 * @tc.desc: Test HandleSelectExtend delegates to HandleSelect.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSelectionTestNg, HandleSelectExtend001, TestSize.Level0)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->HandleSelectExtend(CaretMoveIntent::Left);
    EXPECT_FALSE(pattern->textSelector_.IsValid());
}

/**
 * @tc.name: SelectedBackgroundColor001
 * @tc.desc: Test SetSelectedBackgroundColor.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSelectionTestNg, SelectedBackgroundColor001, TestSize.Level0)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    Color semiTransparentBlue(0x800000ff); // alpha != 255, no opacity change in getter
    pattern->SetSelectedBackgroundColor(semiTransparentBlue);
    EXPECT_EQ(pattern->GetSelectedBackgroundColor(), semiTransparentBlue);
}

/**
 * @tc.name: GetFirstHandleOffset001
 * @tc.desc: Test GetFirstHandleOffset / GetSecondHandleOffset return selector offsets.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSelectionTestNg, GetFirstHandleOffset001, TestSize.Level0)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    auto first = pattern->GetFirstHandleOffset();
    auto second = pattern->GetSecondHandleOffset();
    EXPECT_EQ(first.GetX(), 0.0f);
    EXPECT_EQ(second.GetX(), 0.0f);
}

/**
 * @tc.name: GetSelectionMenuOffset001
 * @tc.desc: Test GetSelectionMenuOffset.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorSelectionTestNg, GetSelectionMenuOffset001, TestSize.Level0)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->selectionMenuOffsetByMouse_ = OffsetF(1, 2);
    EXPECT_EQ(pattern->GetSelectionMenuOffset(), OffsetF(1, 2));
}

} // namespace OHOS::Ace::NG

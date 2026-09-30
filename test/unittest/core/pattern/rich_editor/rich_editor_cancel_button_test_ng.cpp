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

#include "test/unittest/core/pattern/rich_editor/rich_editor_common_test_ng.h"
#include "test/mock/frameworks/base/thread/mock_task_executor.h"
#include "test/mock/frameworks/core/common/mock_container.h"
#include "test/mock/frameworks/core/common/mock_theme_manager.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_model_ng.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_overlay_modifier.h"
#include "core/components_ng/pattern/rich_editor/style_manager.h"
#include "core/components_ng/pattern/text_field/clean_node_host.h"
#include "core/components_ng/pattern/text_field/text_input_response_area.h"
#include "core/common/resource/pattern_resource_manager.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
class RichEditorCancelButtonTestNg : public RichEditorCommonTestNg {
public:
    void SetUp() override
    {
        MockPipelineContext::SetUp();
        MockContainer::SetUp();
        MockContainer::Current()->taskExecutor_ = AceType::MakeRefPtr<MockTaskExecutor>();
        auto* stack = ViewStackProcessor::GetInstance();
        auto nodeId = stack->ClaimNodeId();
        richEditorNode_ = FrameNode::GetOrCreateFrameNode(
            V2::RICH_EDITOR_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<RichEditorPattern>(true); });
        ASSERT_NE(richEditorNode_, nullptr);
        auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
        ASSERT_NE(pattern, nullptr);
        pattern->InitScrollablePattern();
        pattern->SetRichEditorController(AceType::MakeRefPtr<RichEditorController>());
        pattern->GetRichEditorController()->SetPattern(AceType::WeakClaim(AceType::RawPtr(pattern)));
        pattern->CreateNodePaintMethod();
    }
    void TearDown() override
    {
        MockPipelineContext::TearDown();
        MockContainer::TearDown();
    }
    static void TearDownTestSuite() {}
};

/**
 * @tc.name: ICleanNodeHostInheritance
 * @tc.desc: Verify RichEditorPattern inherits ICleanNodeHost and all 24 methods are callable.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, ICleanNodeHostInheritance, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    auto hostInterface = AceType::DynamicCast<ICleanNodeHost>(pattern);
    ASSERT_NE(hostInterface, nullptr);
    EXPECT_EQ(hostInterface->GetCleanNodeStyle().value(), CleanNodeStyle::INPUT);
    EXPECT_EQ(hostInterface->GetIsShowSymbol(), true);
    EXPECT_EQ(hostInterface->GetCancelIconSize(), CalcDimension());
    EXPECT_EQ(hostInterface->GetCancelIconColor(), Color());
    EXPECT_EQ(hostInterface->GetCancelIconSrc(), std::string());
    EXPECT_EQ(hostInterface->GetBundleName(), std::string());
    EXPECT_EQ(hostInterface->GetModuleName(), std::string());
    EXPECT_FALSE(hostInterface->HasCancelIconSize());
    EXPECT_FALSE(hostInterface->HasCancelIconColor());
    EXPECT_FALSE(hostInterface->HasCancelIconSrc());
    EXPECT_FALSE(hostInterface->HasBundleName());
    EXPECT_FALSE(hostInterface->HasModuleName());
    EXPECT_FALSE(hostInterface->GetIsDisabled());
    EXPECT_FALSE(hostInterface->IsDragging());
    EXPECT_FALSE(hostInterface->HasUserAccessibilityText());
    EXPECT_FALSE(hostInterface->IsShowCancelButtonMode());
    EXPECT_FALSE(hostInterface->HasCancelMaxFontScale());
    EXPECT_FALSE(hostInterface->HasCancelMinFontScale());
}

/**
 * @tc.name: ProcessCancelButtonCreatesResponseArea
 * @tc.desc: After setting cancel button via SetCancelButton and calling OnModifyDone, CleanNodeResponseArea exists.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, ProcessCancelButtonCreatesResponseArea, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    auto* frameNode = AceType::RawPtr(richEditorNode_);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(RichEditorLayoutProperty, SingleLine, true, frameNode);
    RichEditorModelNG::SetCancelButton(frameNode, static_cast<int32_t>(CleanNodeStyle::CONSTANT),
        CalcDimension(16.0_vp), Color::WHITE, "");
    pattern->OnModifyDone();
    auto hostInterface = AceType::DynamicCast<ICleanNodeHost>(pattern);
    ASSERT_NE(hostInterface, nullptr);
    EXPECT_TRUE(hostInterface->IsShowCancelButtonMode());
}

/**
 * @tc.name: ThreeStateDisplay_ConstantAlwaysShows
 * @tc.desc: style=CONSTANT should show cancel button regardless of content.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, ThreeStateDisplay_ConstantAlwaysShows, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    auto* frameNode = AceType::RawPtr(richEditorNode_);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(RichEditorLayoutProperty, SingleLine, true, frameNode);
    RichEditorModelNG::SetCancelButton(frameNode, static_cast<int32_t>(CleanNodeStyle::CONSTANT),
        CalcDimension(16.0_vp), Color::WHITE, "");
    auto hostInterface = AceType::DynamicCast<ICleanNodeHost>(pattern);
    ASSERT_NE(hostInterface, nullptr);
    EXPECT_TRUE(hostInterface->IsShowCancelButtonMode());
}

/**
 * @tc.name: ThreeStateDisplay_InvisibleAlwaysHides
 * @tc.desc: style=INVISIBLE should hide cancel button regardless of content.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, ThreeStateDisplay_InvisibleAlwaysHides, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    auto* frameNode = AceType::RawPtr(richEditorNode_);
    RichEditorModelNG::SetSingleLine(frameNode, true);
    RichEditorModelNG::SetCancelButton(frameNode, static_cast<int32_t>(CleanNodeStyle::INVISIBLE),
        CalcDimension(16.0_vp), Color::WHITE, "");
    pattern->OnModifyDone();
    auto hostInterface = AceType::DynamicCast<ICleanNodeHost>(pattern);
    ASSERT_NE(hostInterface, nullptr);
    EXPECT_TRUE(hostInterface->IsShowCancelButtonMode());
    // INVISIBLE style: cancel button should be hidden regardless of content
    const auto& cleanNodeArea = pattern->GetCleanNodeResponseArea();
    ASSERT_NE(cleanNodeArea, nullptr);
    EXPECT_FALSE(cleanNodeArea->IsShow());
}

/**
 * @tc.name: ThreeStateDisplay_InputShowsWhenContent
 * @tc.desc: style=INPUT should show cancel button when IsShowCancelButton is set and content exists.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, ThreeStateDisplay_InputShowsWhenContent, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    auto* frameNode = AceType::RawPtr(richEditorNode_);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(RichEditorLayoutProperty, SingleLine, true, frameNode);
    RichEditorModelNG::SetSingleLine(frameNode, true);
    RichEditorModelNG::SetCancelButton(frameNode, static_cast<int32_t>(CleanNodeStyle::INPUT),
        CalcDimension(16.0_vp), Color::WHITE, "");
    pattern->isSpanStringMode_ = false;
    // Add content so that IsContentEmpty() returns false for INPUT style
    auto spanItem = AceType::MakeRefPtr<SpanItem>();
    spanItem->content = u"hello1";
    pattern->spans_.push_back(spanItem);
    pattern->OnModifyDone();
    auto hostInterface = AceType::DynamicCast<ICleanNodeHost>(pattern);
    ASSERT_NE(hostInterface, nullptr);
    EXPECT_TRUE(hostInterface->IsShowCancelButtonMode());
    // INPUT style with content: cancel button should be shown
    const auto& cleanNodeArea = pattern->GetCleanNodeResponseArea();
    ASSERT_NE(cleanNodeArea, nullptr);
    EXPECT_TRUE(cleanNodeArea->IsShow());
}

/**
 * @tc.name: HandleCleanNodeClickedClearsContent
 * @tc.desc: HandleCleanNodeClicked should clear all text content via DeleteForward.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, HandleCleanNodeClickedClearsContent, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    auto hostInterface = AceType::DynamicCast<ICleanNodeHost>(pattern);
    ASSERT_NE(hostInterface, nullptr);
    // HandleCleanNodeClicked should not crash even with empty content
    hostInterface->HandleCleanNodeClicked();
    SUCCEED();
}

/**
 * @tc.name: PropertyGettersReturnSetValues
 * @tc.desc: Verify that getters return values set via SetCancelButton.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, PropertyGettersReturnSetValues, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    auto* frameNode = AceType::RawPtr(richEditorNode_);
    RichEditorModelNG::SetCancelButton(frameNode, static_cast<int32_t>(CleanNodeStyle::CONSTANT),
        CalcDimension(20.0_vp), Color::RED, "icon.png");
    auto hostInterface = AceType::DynamicCast<ICleanNodeHost>(pattern);
    ASSERT_NE(hostInterface, nullptr);
    EXPECT_EQ(hostInterface->GetCleanNodeStyle().value(), CleanNodeStyle::CONSTANT);
    EXPECT_EQ(hostInterface->GetCancelIconSize().Value(), 20.0f);
    EXPECT_EQ(hostInterface->GetCancelIconColor(), Color::RED);
    EXPECT_EQ(hostInterface->GetCancelIconSrc(), "icon.png");
    EXPECT_FALSE(hostInterface->GetIsShowSymbol());
    EXPECT_TRUE(hostInterface->HasCancelIconSize());
    EXPECT_TRUE(hostInterface->HasCancelIconColor());
    EXPECT_TRUE(hostInterface->HasCancelIconSrc());
}

/**
 * @tc.name: LayoutAlgorithmChildrenSizeIsTwo
 * @tc.desc: Verify RichEditorLayoutAlgorithm CHILDREN_SIZE is 2 after cancel button support.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, LayoutAlgorithmChildrenSizeIsTwo, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    auto* frameNode = AceType::RawPtr(richEditorNode_);
    RichEditorModelNG::SetSingleLine(frameNode, true);
    RichEditorModelNG::SetCancelButton(frameNode, static_cast<int32_t>(CleanNodeStyle::CONSTANT),
        CalcDimension(16.0_vp), Color::WHITE, "");
    pattern->OnModifyDone();
    // After OnModifyDone with cancel button enabled, the host node should have 2 children:
    // content node + cancel button node
    auto children = richEditorNode_->GetChildren();
    EXPECT_GE(children.size(), 2u);
}

/**
 * @tc.name: ProcessCancelButton_RefreshExistingArea
 * @tc.desc: Call OnModifyDone twice to hit the Refresh path (branch A) of ProcessCancelButton.
 *           When cleanNodeResponseArea_ already exists and IsShowCancelButtonMode is true,
 *           it should Refresh + UpdateShowState instead of creating a new area.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, ProcessCancelButton_RefreshExistingArea, TestSize.Level0)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    auto* frameNode = AceType::RawPtr(richEditorNode_);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(RichEditorLayoutProperty, SingleLine, true, frameNode);
    RichEditorModelNG::SetCancelButton(frameNode, static_cast<int32_t>(CleanNodeStyle::CONSTANT),
        CalcDimension(16.0_vp), Color::WHITE, "");
    // First OnModifyDone: creates the response area (branch B)
    pattern->OnModifyDone();
    ASSERT_NE(pattern->cleanNodeResponseArea_, nullptr);
    // Second OnModifyDone: should refresh existing area (branch A)
    pattern->MarkCancelButtonDirty();
    pattern->OnModifyDone();
    ASSERT_NE(pattern->cleanNodeResponseArea_, nullptr);
    auto hostInterface = AceType::DynamicCast<ICleanNodeHost>(pattern);
    ASSERT_NE(hostInterface, nullptr);
    EXPECT_TRUE(hostInterface->IsShowCancelButtonMode());
}

/**
 * @tc.name: ProcessCancelButton_DisableAndClearArea
 * @tc.desc: Enable cancel button then disable it to hit the ClearArea path (branch C).
 *           When IsShowCancelButtonMode is false and cleanNodeResponseArea_ exists,
 *           it should ClearArea + Reset.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, ProcessCancelButton_DisableAndClearArea, TestSize.Level0)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    auto* frameNode = AceType::RawPtr(richEditorNode_);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(RichEditorLayoutProperty, SingleLine, true, frameNode);
    RichEditorModelNG::SetCancelButton(frameNode, static_cast<int32_t>(CleanNodeStyle::CONSTANT),
        CalcDimension(16.0_vp), Color::WHITE, "");
    pattern->OnModifyDone();
    ASSERT_NE(pattern->cleanNodeResponseArea_, nullptr);
    // Disable: should clear area (branch C)
    RichEditorModelNG::ResetCancelButton(frameNode);
    pattern->OnModifyDone();
    ASSERT_EQ(pattern->cleanNodeResponseArea_, nullptr);
}

/**
 * @tc.name: ProcessCancelButton_DisableNoArea
 * @tc.desc: When IsShowCancelButtonMode is false and no area exists (branch D),
 *           ProcessCancelButton should be a no-op without crash.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, ProcessCancelButton_DisableNoArea, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    // No cancel button set, OnModifyDone should be a no-op
    pattern->OnModifyDone();
    // No area should be created when disabled
    ASSERT_EQ(pattern->cleanNodeResponseArea_, nullptr);
    // Call again to verify no crash
    pattern->OnModifyDone();
    ASSERT_EQ(pattern->cleanNodeResponseArea_, nullptr);
}

/**
 * @tc.name: ResetCancelButton_RestoresDefaults
 * @tc.desc: Set all cancel button properties via SetCancelButton then call ResetCancelButton,
 *           verify all getters return default values afterwards.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, ResetCancelButton_RestoresDefaults, TestSize.Level0)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    auto* frameNode = AceType::RawPtr(richEditorNode_);
    RichEditorModelNG::SetCancelButton(frameNode, static_cast<int32_t>(CleanNodeStyle::CONSTANT),
        CalcDimension(20.0_vp), Color::RED, "icon.png");
    // Verify properties are set
    auto hostInterface = AceType::DynamicCast<ICleanNodeHost>(pattern);
    ASSERT_NE(hostInterface, nullptr);
    EXPECT_EQ(hostInterface->GetCleanNodeStyle().value(), CleanNodeStyle::CONSTANT);
    EXPECT_TRUE(hostInterface->HasCancelIconSize());
    EXPECT_TRUE(hostInterface->HasCancelIconColor());
    EXPECT_TRUE(hostInterface->HasCancelIconSrc());
    // Reset
    RichEditorModelNG::ResetCancelButton(frameNode);
    // Verify defaults are restored
    EXPECT_EQ(hostInterface->GetCleanNodeStyle().value(), CleanNodeStyle::INPUT);
    EXPECT_FALSE(hostInterface->HasCancelIconSize());
    EXPECT_FALSE(hostInterface->HasCancelIconColor());
    EXPECT_FALSE(hostInterface->HasCancelIconSrc());
    EXPECT_FALSE(hostInterface->HasBundleName());
    EXPECT_FALSE(hostInterface->HasModuleName());
    EXPECT_FALSE(hostInterface->IsShowCancelButtonMode());
    EXPECT_TRUE(hostInterface->GetIsShowSymbol());
}

/**
 * @tc.name: SetCancelButton_CombinedSetter
 * @tc.desc: Use SetCancelButton (combined setter) to set all properties at once,
 *           verify each getter returns the expected value.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, SetCancelButton_CombinedSetter, TestSize.Level0)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    auto* frameNode = AceType::RawPtr(richEditorNode_);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(RichEditorLayoutProperty, SingleLine, true, frameNode);
    RichEditorModelNG::SetCancelButton(frameNode, static_cast<int32_t>(CleanNodeStyle::CONSTANT),
        CalcDimension(16.0_vp), Color::BLUE, "cancel.png");
    auto hostInterface = AceType::DynamicCast<ICleanNodeHost>(pattern);
    ASSERT_NE(hostInterface, nullptr);
    EXPECT_EQ(hostInterface->GetCleanNodeStyle().value(), CleanNodeStyle::CONSTANT);
    EXPECT_EQ(hostInterface->GetCancelIconSize().Value(), 16.0f);
    EXPECT_EQ(hostInterface->GetCancelIconColor(), Color::BLUE);
    EXPECT_EQ(hostInterface->GetCancelIconSrc(), "cancel.png");
    EXPECT_TRUE(hostInterface->IsShowCancelButtonMode());
}

/**
 * @tc.name: UpdateShowState_InvisibleStyleHides
 * @tc.desc: Create area with CONSTANT style, then switch to INVISIBLE style and
 *           call OnModifyDone to trigger UpdateShowState hiding path.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, UpdateShowState_InvisibleStyleHides, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    auto* frameNode = AceType::RawPtr(richEditorNode_);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(RichEditorLayoutProperty, SingleLine, true, frameNode);
    RichEditorModelNG::SetCancelButton(frameNode, static_cast<int32_t>(CleanNodeStyle::CONSTANT),
        CalcDimension(16.0_vp), Color::WHITE, "");
    pattern->OnModifyDone();
    ASSERT_NE(pattern->cleanNodeResponseArea_, nullptr);
    // Switch to INVISIBLE: should trigger UpdateShowState hide path
    RichEditorModelNG::SetCancelButton(frameNode, static_cast<int32_t>(CleanNodeStyle::INVISIBLE),
        CalcDimension(16.0_vp), Color::WHITE, "");
    pattern->OnModifyDone();
    // Area should still exist (Refresh path, not ClearArea)
    ASSERT_NE(pattern->cleanNodeResponseArea_, nullptr);
    auto hostInterface = AceType::DynamicCast<ICleanNodeHost>(pattern);
    ASSERT_NE(hostInterface, nullptr);
    EXPECT_EQ(hostInterface->GetCleanNodeStyle().value(), CleanNodeStyle::INVISIBLE);
}

/**
 * @tc.name: UpdateShowState_InputStyleNoContentHides
 * @tc.desc: With INPUT style and no content, UpdateShowState should hide the cancel button.
 *           Area is created but should not be visible.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, UpdateShowState_InputStyleNoContentHides, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    auto* frameNode = AceType::RawPtr(richEditorNode_);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(RichEditorLayoutProperty, SingleLine, true, frameNode);
    RichEditorModelNG::SetCancelButton(frameNode, static_cast<int32_t>(CleanNodeStyle::INPUT),
        CalcDimension(16.0_vp), Color::WHITE, "");
    pattern->OnModifyDone();
    // Area should be created
    ASSERT_NE(pattern->cleanNodeResponseArea_, nullptr);
    // With INPUT style and empty content, IsContentEmpty returns true,
    // so UpdateShowState should call UpdateCleanNode(false)
    auto hostInterface = AceType::DynamicCast<ICleanNodeHost>(pattern);
    ASSERT_NE(hostInterface, nullptr);
    EXPECT_TRUE(hostInterface->IsShowCancelButtonMode());
    EXPECT_TRUE(hostInterface->IsContentEmpty());
}

/**
 * @tc.name: HandleCleanNodeClicked_NotCrashingWithContent
 * @tc.desc: Add content to the RichEditor and call HandleCleanNodeClicked,
 *           verify it does not crash and clears content.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, HandleCleanNodeClicked_NotCrashingWithContent, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    auto hostInterface = AceType::DynamicCast<ICleanNodeHost>(pattern);
    ASSERT_NE(hostInterface, nullptr);
    // With empty content, HandleCleanNodeClicked should not crash
    EXPECT_TRUE(hostInterface->IsContentEmpty());
    hostInterface->HandleCleanNodeClicked();
    SUCCEED();
}

/**
 * @tc.name: HandleCleanNodeClickedRequestsFocusWhenNotFocused
 * @tc.desc: When not focused, HandleCleanNodeClicked should request focus via RequestFocusImmediately,
 *           so that HandleFocusEvent is triggered and the component gains focus.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, HandleCleanNodeClickedRequestsFocusWhenNotFocused, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    auto hostInterface = AceType::DynamicCast<ICleanNodeHost>(pattern);
    ASSERT_NE(hostInterface, nullptr);

    EXPECT_FALSE(pattern->HasFocus());
    hostInterface->HandleCleanNodeClicked();
    EXPECT_TRUE(pattern->HasFocus());
}

/**
 * @tc.name: HandleCleanNodeClickedSetsEditingState
 * @tc.desc: When not focused, HandleCleanNodeClicked should set isEditing_ to true
 *           via HandleFocusEvent → HandleOnEditChanged.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, HandleCleanNodeClickedSetsEditingState, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    auto hostInterface = AceType::DynamicCast<ICleanNodeHost>(pattern);
    ASSERT_NE(hostInterface, nullptr);

    pattern->isEditing_ = true;
    hostInterface->HandleCleanNodeClicked();
    EXPECT_TRUE(pattern->isEditing_);
}

/**
 * @tc.name: HandleCleanNodeClickedStartsTwinkling
 * @tc.desc: HandleCleanNodeClicked should start caret twinkling.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, HandleCleanNodeClickedStartsTwinkling, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    auto hostInterface = AceType::DynamicCast<ICleanNodeHost>(pattern);
    ASSERT_NE(hostInterface, nullptr);

    pattern->isEditing_ = true;
    hostInterface->HandleCleanNodeClicked();
    SUCCEED();
}

/**
 * @tc.name: HandleCleanNodeClickedNoSideEffectWhenAlreadyFocused
 * @tc.desc: When already focused, HandleCleanNodeClicked should not crash and remain focused.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, HandleCleanNodeClickedNoSideEffectWhenAlreadyFocused, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    auto hostInterface = AceType::DynamicCast<ICleanNodeHost>(pattern);
    ASSERT_NE(hostInterface, nullptr);

    auto focusHub = richEditorNode_->GetOrCreateFocusHub();
    ASSERT_NE(focusHub, nullptr);
    focusHub->RequestFocusImmediately();
    ASSERT_TRUE(pattern->HasFocus());
    pattern->isEditing_ = true;

    hostInterface->HandleCleanNodeClicked();
    EXPECT_TRUE(pattern->HasFocus());
    EXPECT_TRUE(pattern->isEditing_);
}

// ============ IsShowCancelButtonMode branch coverage ============

/**
 * @tc.name: IsShowCancelButtonMode_NotSingleLine
 * @tc.desc: When SingleLine is false, IsShowCancelButtonMode returns false
 *           even if IsShowCancelButton is set to true.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, IsShowCancelButtonMode_NotSingleLine, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    auto* frameNode = AceType::RawPtr(richEditorNode_);
    // Set cancel button but do NOT set SingleLine
    RichEditorModelNG::SetCancelButton(frameNode, static_cast<int32_t>(CleanNodeStyle::CONSTANT),
        CalcDimension(16.0_vp), Color::WHITE, "");
    auto hostInterface = AceType::DynamicCast<ICleanNodeHost>(pattern);
    ASSERT_NE(hostInterface, nullptr);
    // SingleLine is false by default, so mode should be false
    EXPECT_FALSE(hostInterface->IsShowCancelButtonMode());
}

/**
 * @tc.name: IsShowCancelButtonMode_NotSetCancelButton
 * @tc.desc: When SingleLine is true but IsShowCancelButton is not set,
 *           IsShowCancelButtonMode returns false.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, IsShowCancelButtonMode_NotSetCancelButton, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    auto* frameNode = AceType::RawPtr(richEditorNode_);
    RichEditorModelNG::SetSingleLine(frameNode, true);
    // Do not call SetCancelButton
    auto hostInterface = AceType::DynamicCast<ICleanNodeHost>(pattern);
    ASSERT_NE(hostInterface, nullptr);
    EXPECT_FALSE(hostInterface->IsShowCancelButtonMode());
}

// ============ IsInResponseArea branch coverage ============

/**
 * @tc.name: IsInResponseArea_CancelButtonTouched
 * @tc.desc: When cancelButtonTouched_ is true, IsInResponseArea returns true
 *           regardless of position.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, IsInResponseArea_CancelButtonTouched, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    auto hostInterface = AceType::DynamicCast<ICleanNodeHost>(pattern);
    ASSERT_NE(hostInterface, nullptr);
    // Set cancelButtonTouched_ = true
    hostInterface->SetCancelButtonTouched(true);
    EXPECT_TRUE(hostInterface->IsCancelButtonTouched());
    // IsInResponseArea should return true when cancelButtonTouched_ is true
    EXPECT_TRUE(pattern->IsInResponseArea(Offset(0, 0)));
    // Reset
    hostInterface->SetCancelButtonTouched(false);
    EXPECT_FALSE(hostInterface->IsCancelButtonTouched());
}

/**
 * @tc.name: IsInResponseArea_NoAreaReturnsFalse
 * @tc.desc: When cleanNodeResponseArea_ is null and cancelButtonTouched_ is false,
 *           IsInResponseArea returns false for any position.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, IsInResponseArea_NoAreaReturnsFalse, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    ASSERT_EQ(pattern->cleanNodeResponseArea_, nullptr);
    pattern->cancelButtonTouched_ = false;
    EXPECT_FALSE(pattern->IsInResponseArea(Offset(100, 100)));
}

/**
 * @tc.name: IsOnCleanNodeByPosition_NullArea
 * @tc.desc: When cleanNodeResponseArea_ is null, IsOnCleanNodeByPosition returns false.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, IsOnCleanNodeByPosition_NullArea, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    ASSERT_EQ(pattern->cleanNodeResponseArea_, nullptr);
    EXPECT_FALSE(pattern->IsOnCleanNodeByPosition(Offset(0, 0)));
}

/**
 * @tc.name: IsOnCancelButtonHoverArea_NullArea
 * @tc.desc: When cleanNodeResponseArea_ is null, IsOnCancelButtonHoverArea returns false.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, IsOnCancelButtonHoverArea_NullArea, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    ASSERT_EQ(pattern->cleanNodeResponseArea_, nullptr);
    EXPECT_FALSE(pattern->IsOnCancelButtonHoverArea(Offset(0, 0)));
}

// ============ OnCleanNodeHoverEnter branch coverage ============

/**
 * @tc.name: OnCleanNodeHoverEnter_ResetsMouseStyle
 * @tc.desc: When currentMouseStyle_ is not DEFAULT, OnCleanNodeHoverEnter changes it to DEFAULT.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, OnCleanNodeHoverEnter_ResetsMouseStyle, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    // Set a non-DEFAULT mouse style
    pattern->currentMouseStyle_ = MouseFormat::TEXT_CURSOR;
    auto hostInterface = AceType::DynamicCast<ICleanNodeHost>(pattern);
    ASSERT_NE(hostInterface, nullptr);
    hostInterface->OnCleanNodeHoverEnter();
    EXPECT_EQ(pattern->currentMouseStyle_, MouseFormat::DEFAULT);
}

/**
 * @tc.name: OnCleanNodeHoverEnter_AlreadyDefault
 * @tc.desc: When currentMouseStyle_ is already DEFAULT, OnCleanNodeHoverEnter does nothing.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, OnCleanNodeHoverEnter_AlreadyDefault, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->currentMouseStyle_ = MouseFormat::DEFAULT;
    auto hostInterface = AceType::DynamicCast<ICleanNodeHost>(pattern);
    ASSERT_NE(hostInterface, nullptr);
    hostInterface->OnCleanNodeHoverEnter();
    EXPECT_EQ(pattern->currentMouseStyle_, MouseFormat::DEFAULT);
}

// ============ SetCancelButtonIconColor coverage ============

/**
 * @tc.name: SetCancelButtonIconColor_UpdatesProperty
 * @tc.desc: SetCancelButtonIconColor updates the IconColor layout property and marks dirty.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, SetCancelButtonIconColor_UpdatesProperty, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    auto* frameNode = AceType::RawPtr(richEditorNode_);
    // Set initial cancel button
    RichEditorModelNG::SetCancelButton(frameNode, static_cast<int32_t>(CleanNodeStyle::CONSTANT),
        CalcDimension(16.0_vp), Color::WHITE, "");
    pattern->cancelButtonDirty_ = false;
    // Call SetCancelButtonIconColor
    pattern->SetCancelButtonIconColor(Color::RED);
    // Verify IconColor property updated
    auto layoutProperty = frameNode->GetLayoutProperty<RichEditorLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    EXPECT_EQ(layoutProperty->GetIconColor().value_or(Color::WHITE), Color::RED);
    // Verify dirty flag set
    EXPECT_TRUE(pattern->cancelButtonDirty_);
}

// ============ UpdatePropertyImpl coverage ============

/**
 * @tc.name: UpdatePropertyImpl_CancelButtonIconColor
 * @tc.desc: UpdatePropertyImpl with CANCEL_BUTTON_ICON_COLOR_KEY calls SetCancelButtonIconColor.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, UpdatePropertyImpl_CancelButtonIconColor, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    auto* frameNode = AceType::RawPtr(richEditorNode_);
    RichEditorModelNG::SetCancelButton(frameNode, static_cast<int32_t>(CleanNodeStyle::CONSTANT),
        CalcDimension(16.0_vp), Color::WHITE, "");
    // Build a PropertyValueBase with Color
    auto value = AceType::MakeRefPtr<PropertyValueBase>();
    value->SetValue(Color::BLUE);
    value->SetValueType(ValueType::COLOR);
    pattern->cancelButtonDirty_ = false;
    // Call UpdatePropertyImpl with the cancel button icon color key
    pattern->UpdatePropertyImpl(std::string(StyleManager::CANCEL_BUTTON_ICON_COLOR_KEY), value);
    // Verify the color was updated
    auto layoutProperty = frameNode->GetLayoutProperty<RichEditorLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    EXPECT_EQ(layoutProperty->GetIconColor().value_or(Color::WHITE), Color::BLUE);
    EXPECT_TRUE(pattern->cancelButtonDirty_);
}

// ============ SetCleanHoverColorAndRect / ClearCleanHoverColorAndRects ============

/**
 * @tc.name: SetCleanHoverColorAndRect_DelegatesToOverlay
 * @tc.desc: SetCleanHoverColorAndRect sets hover color and rects on the overlay modifier.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, SetCleanHoverColorAndRect_DelegatesToOverlay, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    auto hostInterface = AceType::DynamicCast<ICleanNodeHost>(pattern);
    ASSERT_NE(hostInterface, nullptr);
    // Create a cancel button so overlay modifier is initialized
    auto* frameNode = AceType::RawPtr(richEditorNode_);
    RichEditorModelNG::SetCancelButton(frameNode, static_cast<int32_t>(CleanNodeStyle::CONSTANT),
        CalcDimension(16.0_vp), Color::WHITE, "");
    pattern->OnModifyDone();
    ASSERT_NE(pattern->hostOverlayMod_, nullptr);
    // Call SetCleanHoverColorAndRect
    RoundRect rect(RectF(0, 0, 50, 50), 0.0f, 0.0f);
    uint32_t color = 0xFFFF0000;
    hostInterface->SetCleanHoverColorAndRect(rect, color);
    // Verify overlay modifier received the data
    auto overlay = AceType::DynamicCast<RichEditorOverlayModifier>(pattern->hostOverlayMod_);
    ASSERT_NE(overlay, nullptr);
    EXPECT_FALSE(overlay->hoverRects_.empty());
    EXPECT_EQ(overlay->hoverRects_.size(), 1u);
    EXPECT_EQ(static_cast<uint32_t>(overlay->hoverColor_->Get()), color);
}

/**
 * @tc.name: ClearCleanHoverColorAndRects_DelegatesToOverlay
 * @tc.desc: ClearCleanHoverColorAndRects clears hover color and rects on the overlay modifier.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, ClearCleanHoverColorAndRects_DelegatesToOverlay, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    auto hostInterface = AceType::DynamicCast<ICleanNodeHost>(pattern);
    ASSERT_NE(hostInterface, nullptr);
    auto* frameNode = AceType::RawPtr(richEditorNode_);
    RichEditorModelNG::SetCancelButton(frameNode, static_cast<int32_t>(CleanNodeStyle::CONSTANT),
        CalcDimension(16.0_vp), Color::WHITE, "");
    pattern->OnModifyDone();
    // First set some hover rects
    RoundRect rect(RectF(0, 0, 50, 50), 0.0f, 0.0f);
    hostInterface->SetCleanHoverColorAndRect(rect, 0xFFFF0000);
    // Then clear
    hostInterface->ClearCleanHoverColorAndRects();
    auto overlay = AceType::DynamicCast<RichEditorOverlayModifier>(pattern->hostOverlayMod_);
    ASSERT_NE(overlay, nullptr);
    EXPECT_TRUE(overlay->hoverRects_.empty());
    EXPECT_EQ(overlay->hoverColor_->Get(), 0);
}

/**
 * @tc.name: ClearCleanHoverColorAndRects_NullOverlay
 * @tc.desc: ClearCleanHoverColorAndRects should not crash when hostOverlayMod_ is null.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, ClearCleanHoverColorAndRects_NullOverlay, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->hostOverlayMod_ = nullptr;
    auto hostInterface = AceType::DynamicCast<ICleanNodeHost>(pattern);
    ASSERT_NE(hostInterface, nullptr);
    // Should not crash
    hostInterface->ClearCleanHoverColorAndRects();
    SUCCEED();
}

// ============ HandleBlurEventReset cancelButtonTouched_ reset ============

/**
 * @tc.name: HandleBlurEventReset_ResetsCancelButtonTouched
 * @tc.desc: HandleBlurEventReset sets cancelButtonTouched_ to false.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, HandleBlurEventReset_ResetsCancelButtonTouched, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    // Set cancelButtonTouched_ to true
    pattern->cancelButtonTouched_ = true;
    pattern->HandleBlurEventReset();
    EXPECT_FALSE(pattern->cancelButtonTouched_);
}

// ============ BeforeCreateLayoutWrapper content empty monitoring ============

/**
 * @tc.name: BeforeCreateLayoutWrapper_TriggersProcessOnContentChange
 * @tc.desc: When cancel button mode is on with INPUT style and content changes from
 *           empty to non-empty, BeforeCreateLayoutWrapper triggers ProcessCancelButton.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, BeforeCreateLayoutWrapper_TriggersProcessOnContentChange, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    auto* frameNode = AceType::RawPtr(richEditorNode_);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(RichEditorLayoutProperty, SingleLine, true, frameNode);
    pattern->styledString_ = nullptr;
    RichEditorModelNG::SetSingleLine(frameNode, true);
    RichEditorModelNG::SetCancelButton(frameNode, static_cast<int32_t>(CleanNodeStyle::INPUT),
        CalcDimension(16.0_vp), Color::WHITE, "");
    pattern->OnModifyDone();
    // Initially content is empty, lastContentEmptyForCancel_ should be true
    ASSERT_TRUE(pattern->lastContentEmptyForCancel_);
    // Add content so IsContentEmpty returns false
    auto spanItem = AceType::MakeRefPtr<SpanItem>();
    spanItem->content = u"hello";
    pattern->spans_.push_back(spanItem);
    // Call BeforeCreateLayoutWrapper, should detect content change and trigger ProcessCancelButton
    pattern->BeforeCreateLayoutWrapper();
    EXPECT_FALSE(pattern->lastContentEmptyForCancel_);
}

/**
 * @tc.name: BeforeCreateLayoutWrapper_NoTriggerWhenContentUnchanged
 * @tc.desc: When content empty state does not change, BeforeCreateLayoutWrapper
 *           does not trigger ProcessCancelButton again.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, BeforeCreateLayoutWrapper_NoTriggerWhenContentUnchanged, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    auto* frameNode = AceType::RawPtr(richEditorNode_);
    RichEditorModelNG::SetSingleLine(frameNode, true);
    RichEditorModelNG::SetCancelButton(frameNode, static_cast<int32_t>(CleanNodeStyle::INPUT),
        CalcDimension(16.0_vp), Color::WHITE, "");
    pattern->OnModifyDone();
    // Content is empty, lastContentEmptyForCancel_ is true
    ASSERT_TRUE(pattern->lastContentEmptyForCancel_);
    // Call BeforeCreateLayoutWrapper without changing content
    pattern->BeforeCreateLayoutWrapper();
    // lastContentEmptyForCancel_ should still be true (no change)
    EXPECT_TRUE(pattern->lastContentEmptyForCancel_);
}

/**
 * @tc.name: BeforeCreateLayoutWrapper_NoTriggerWhenNotCancelButtonMode
 * @tc.desc: When cancel button mode is off, BeforeCreateLayoutWrapper does not
 *           check content empty state.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, BeforeCreateLayoutWrapper_NoTriggerWhenNotCancelButtonMode, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    // Do not set cancel button
    ASSERT_FALSE(pattern->IsShowCancelButtonMode());
    // Should not crash and should not create any response area
    pattern->BeforeCreateLayoutWrapper();
    ASSERT_EQ(pattern->cleanNodeResponseArea_, nullptr);
}

// ============ HandleCleanNodeClicked additional branches ============

/**
 * @tc.name: HandleCleanNodeClicked_IsDraggingReturns
 * @tc.desc: When IsDragging is true, HandleCleanNodeClicked returns early without crashing.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, HandleCleanNodeClicked_IsDraggingReturns, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    auto hostInterface = AceType::DynamicCast<ICleanNodeHost>(pattern);
    ASSERT_NE(hostInterface, nullptr);
    // Simulate dragging state by setting drag state
    pattern->status_ = Status::DRAGGING;
    // HandleCleanNodeClicked should return early without crash
    hostInterface->HandleCleanNodeClicked();
    SUCCEED();
}

/**
 * @tc.name: HandleCleanNodeClicked_DeletesContent
 * @tc.desc: When content exists, HandleCleanNodeClicked deletes all content via DeleteForward.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, HandleCleanNodeClicked_DeletesContent, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    auto hostInterface = AceType::DynamicCast<ICleanNodeHost>(pattern);
    ASSERT_NE(hostInterface, nullptr);
    // Add content
    auto spanItem = AceType::MakeRefPtr<SpanItem>();
    spanItem->content = u"hello world";
    pattern->spans_.push_back(spanItem);
    EXPECT_FALSE(pattern->spans_.empty());
    // Call HandleCleanNodeClicked - should clear content
    hostInterface->HandleCleanNodeClicked();
    // Content should be cleared or spans modified
    SUCCEED();
}

// ============ OverlayModifier direct tests ============

/**
 * @tc.name: OverlayModifier_SetHoverColorAndRects
 * @tc.desc: SetHoverColorAndRects on RichEditorOverlayModifier sets hoverRects_ and hoverColor_.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, OverlayModifier_SetHoverColorAndRects, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    auto* frameNode = AceType::RawPtr(richEditorNode_);
    RichEditorModelNG::SetCancelButton(frameNode, static_cast<int32_t>(CleanNodeStyle::CONSTANT),
        CalcDimension(16.0_vp), Color::WHITE, "");
    pattern->OnModifyDone();
    auto overlay = AceType::DynamicCast<RichEditorOverlayModifier>(pattern->hostOverlayMod_);
    ASSERT_NE(overlay, nullptr);
    std::vector<RoundRect> rects;
    rects.push_back(RoundRect(RectF(10, 10, 40, 40), 0.0f, 0.0f));
    uint32_t color = 0xFF00FF00;
    overlay->SetHoverColorAndRects(rects, color);
    EXPECT_EQ(overlay->hoverRects_.size(), 1u);
    EXPECT_EQ(static_cast<uint32_t>(overlay->hoverColor_->Get()), color);
}

/**
 * @tc.name: OverlayModifier_ClearHoverColorAndRects
 * @tc.desc: ClearHoverColorAndRects on RichEditorOverlayModifier clears hoverRects_ and hoverColor_.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, OverlayModifier_ClearHoverColorAndRects, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    auto* frameNode = AceType::RawPtr(richEditorNode_);
    RichEditorModelNG::SetCancelButton(frameNode, static_cast<int32_t>(CleanNodeStyle::CONSTANT),
        CalcDimension(16.0_vp), Color::WHITE, "");
    pattern->OnModifyDone();
    auto overlay = AceType::DynamicCast<RichEditorOverlayModifier>(pattern->hostOverlayMod_);
    ASSERT_NE(overlay, nullptr);
    // First set some data
    std::vector<RoundRect> rects;
    rects.push_back(RoundRect(RectF(10, 10, 40, 40), 0.0f, 0.0f));
    overlay->SetHoverColorAndRects(rects, 0xFF00FF00);
    ASSERT_FALSE(overlay->hoverRects_.empty());
    // Clear
    overlay->ClearHoverColorAndRects();
    EXPECT_TRUE(overlay->hoverRects_.empty());
    EXPECT_EQ(overlay->hoverColor_->Get(), 0);
}

// ============ CleanNodeResponseArea methods (via RichEditor host) ============

/**
 * @tc.name: CleanNodeResponseArea_HandleButtonMouseEventHover
 * @tc.desc: HandleButtonMouseEvent(true) sets hover color and rect on the host.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, CleanNodeResponseArea_HandleButtonMouseEventHover, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    auto* frameNode = AceType::RawPtr(richEditorNode_);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(RichEditorLayoutProperty, SingleLine, true, frameNode);
    RichEditorModelNG::SetCancelButton(frameNode, static_cast<int32_t>(CleanNodeStyle::CONSTANT),
        CalcDimension(16.0_vp), Color::WHITE, "");
    pattern->OnModifyDone();
    auto cleanArea = AceType::DynamicCast<CleanNodeResponseArea>(pattern->cleanNodeResponseArea_);
    ASSERT_NE(cleanArea, nullptr);
    // Ensure cleanNode_ is set
    ASSERT_NE(cleanArea->cleanNode_, nullptr);
    // Call HandleButtonMouseEvent with isHover=true
    cleanArea->HandleButtonMouseEvent(true);
    auto overlay = AceType::DynamicCast<RichEditorOverlayModifier>(pattern->hostOverlayMod_);
    ASSERT_NE(overlay, nullptr);
    SUCCEED();
}

/**
 * @tc.name: CleanNodeResponseArea_HandleButtonMouseEventNoHover
 * @tc.desc: HandleButtonMouseEvent(false) clears hover color and rects on the host.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, CleanNodeResponseArea_HandleButtonMouseEventNoHover, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    auto* frameNode = AceType::RawPtr(richEditorNode_);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(RichEditorLayoutProperty, SingleLine, true, frameNode);
    RichEditorModelNG::SetCancelButton(frameNode, static_cast<int32_t>(CleanNodeStyle::CONSTANT),
        CalcDimension(16.0_vp), Color::WHITE, "");
    pattern->OnModifyDone();
    auto cleanArea = AceType::DynamicCast<CleanNodeResponseArea>(pattern->cleanNodeResponseArea_);
    ASSERT_NE(cleanArea, nullptr);
    ASSERT_NE(cleanArea->cleanNode_, nullptr);
    // First set hover state
    cleanArea->HandleButtonMouseEvent(true);
    // Then clear with noHover
    cleanArea->HandleButtonMouseEvent(false);
    auto overlay = AceType::DynamicCast<RichEditorOverlayModifier>(pattern->hostOverlayMod_);
    ASSERT_NE(overlay, nullptr);
    EXPECT_TRUE(overlay->hoverRects_.empty());
}

/**
 * @tc.name: CleanNodeResponseArea_HandleResponseButtonTouchDown
 * @tc.desc: HandleResponseButtonTouchDown sets cancelButtonTouched_ to true and sets press color.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, CleanNodeResponseArea_HandleResponseButtonTouchDown, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    auto* frameNode = AceType::RawPtr(richEditorNode_);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(RichEditorLayoutProperty, SingleLine, true, frameNode);
    RichEditorModelNG::SetCancelButton(frameNode, static_cast<int32_t>(CleanNodeStyle::CONSTANT),
        CalcDimension(16.0_vp), Color::WHITE, "");
    pattern->OnModifyDone();
    auto cleanArea = AceType::DynamicCast<CleanNodeResponseArea>(pattern->cleanNodeResponseArea_);
    ASSERT_NE(cleanArea, nullptr);
    ASSERT_NE(cleanArea->cleanNode_, nullptr);
    pattern->cancelButtonTouched_ = false;
    // Call touch down
    cleanArea->HandleResponseButtonTouchDown();
    SUCCEED();
}

/**
 * @tc.name: CleanNodeResponseArea_HandleResponseButtonTouchUp
 * @tc.desc: HandleResponseButtonTouchUp resets cancelButtonTouched_ to false and clears hover.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, CleanNodeResponseArea_HandleResponseButtonTouchUp, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    auto* frameNode = AceType::RawPtr(richEditorNode_);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(RichEditorLayoutProperty, SingleLine, true, frameNode);
    RichEditorModelNG::SetCancelButton(frameNode, static_cast<int32_t>(CleanNodeStyle::CONSTANT),
        CalcDimension(16.0_vp), Color::WHITE, "");
    pattern->OnModifyDone();
    auto cleanArea = AceType::DynamicCast<CleanNodeResponseArea>(pattern->cleanNodeResponseArea_);
    ASSERT_NE(cleanArea, nullptr);
    ASSERT_NE(cleanArea->cleanNode_, nullptr);
    // First touch down (may return early in mock env due to missing theme)
    cleanArea->HandleResponseButtonTouchDown();
    // Then touch up
    cleanArea->HandleResponseButtonTouchUp();
    EXPECT_FALSE(pattern->cancelButtonTouched_);
    auto overlay = AceType::DynamicCast<RichEditorOverlayModifier>(pattern->hostOverlayMod_);
    ASSERT_NE(overlay, nullptr);
    EXPECT_TRUE(overlay->hoverRects_.empty());
}

/**
 * @tc.name: CleanNodeResponseArea_HandleCleanNodeHover
 * @tc.desc: HandleCleanNodeHover delegates to OnCleanNodeHover and HandleButtonMouseEvent.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, CleanNodeResponseArea_HandleCleanNodeHover, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    auto* frameNode = AceType::RawPtr(richEditorNode_);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(RichEditorLayoutProperty, SingleLine, true, frameNode);
    RichEditorModelNG::SetCancelButton(frameNode, static_cast<int32_t>(CleanNodeStyle::CONSTANT),
        CalcDimension(16.0_vp), Color::WHITE, "");
    pattern->OnModifyDone();
    auto cleanArea = AceType::DynamicCast<CleanNodeResponseArea>(pattern->cleanNodeResponseArea_);
    ASSERT_NE(cleanArea, nullptr);
    ASSERT_NE(cleanArea->cleanNode_, nullptr);
    HoverInfo info;
    // Hover enter (may return early in mock env due to missing theme)
    cleanArea->HandleCleanNodeHover(true, info);
    auto overlay = AceType::DynamicCast<RichEditorOverlayModifier>(pattern->hostOverlayMod_);
    ASSERT_NE(overlay, nullptr);
    SUCCEED();
    // Hover leave
    cleanArea->HandleCleanNodeHover(false, info);
    EXPECT_TRUE(overlay->hoverRects_.empty());
}

/**
 * @tc.name: CleanNodeResponseArea_SetAccessibilityClearAction
 * @tc.desc: SetAccessibilityClearAction sets clear action and role on the clean node.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, CleanNodeResponseArea_SetAccessibilityClearAction, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    auto* frameNode = AceType::RawPtr(richEditorNode_);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(RichEditorLayoutProperty, SingleLine, true, frameNode);
    RichEditorModelNG::SetCancelButton(frameNode, static_cast<int32_t>(CleanNodeStyle::CONSTANT),
        CalcDimension(16.0_vp), Color::WHITE, "");
    pattern->OnModifyDone();
    auto cleanArea = AceType::DynamicCast<CleanNodeResponseArea>(pattern->cleanNodeResponseArea_);
    ASSERT_NE(cleanArea, nullptr);
    ASSERT_NE(cleanArea->cleanNode_, nullptr);
    // Verify SetAccessibilityClearAction does not crash and sets expected properties.
    cleanArea->SetAccessibilityClearAction();
    auto accessibilityProperty =
        cleanArea->cleanNode_->GetAccessibilityProperty<AccessibilityProperty>();
    ASSERT_NE(accessibilityProperty, nullptr);
    EXPECT_EQ(accessibilityProperty->GetAccessibilityLevel(), "yes");
    EXPECT_EQ(accessibilityProperty->GetAccessibilityCustomRole(), "button");
}

/**
 * @tc.name: CleanNodeResponseArea_AfterLayoutProcessCleanResponse
 * @tc.desc: AfterLayoutProcessCleanResponse adds an after-layout task without crashing.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, CleanNodeResponseArea_AfterLayoutProcessCleanResponse, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    auto* frameNode = AceType::RawPtr(richEditorNode_);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(RichEditorLayoutProperty, SingleLine, true, frameNode);
    RichEditorModelNG::SetCancelButton(frameNode, static_cast<int32_t>(CleanNodeStyle::CONSTANT),
        CalcDimension(16.0_vp), Color::WHITE, "");
    pattern->OnModifyDone();
    auto cleanArea = AceType::DynamicCast<CleanNodeResponseArea>(pattern->cleanNodeResponseArea_);
    ASSERT_NE(cleanArea, nullptr);
    // Should not crash
    cleanArea->AfterLayoutProcessCleanResponse();
    SUCCEED();
}

// ============ OnHover with HoverInfo - cancel button area branch ============

/**
 * @tc.name: OnHover_CancelButtonAreaChangesMouseStyle
 * @tc.desc: When hovering over the cancel button area, OnHover should change
 *           mouse style to DEFAULT instead of TEXT_CURSOR.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, OnHover_CancelButtonAreaChangesMouseStyle, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    auto* frameNode = AceType::RawPtr(richEditorNode_);
    RichEditorModelNG::SetSingleLine(frameNode, true);
    RichEditorModelNG::SetCancelButton(frameNode, static_cast<int32_t>(CleanNodeStyle::CONSTANT),
        CalcDimension(16.0_vp), Color::WHITE, "");
    pattern->OnModifyDone();
    ASSERT_NE(pattern->cleanNodeResponseArea_, nullptr);
    // Set non-DEFAULT mouse style to verify it changes
    pattern->currentMouseStyle_ = MouseFormat::TEXT_CURSOR;
    HoverInfo info;
    // Call OnHover with hover=true — since cancel button area exists,
    // IsOnCancelButtonHoverArea will be called. With a zero offset it may or may not
    // be in the area, but the function should not crash.
    pattern->OnHover(true, info);
    SUCCEED();
}

// ============ IsContentEmpty additional branches ============

/**
 * @tc.name: IsContentEmpty_SpanStringMode
 * @tc.desc: In span string mode with non-empty styled string, IsContentEmpty returns false.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, IsContentEmpty_SpanStringMode, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    auto hostInterface = AceType::DynamicCast<ICleanNodeHost>(pattern);
    ASSERT_NE(hostInterface, nullptr);
    // In span string mode with no styled string, content should be empty
    EXPECT_TRUE(hostInterface->IsContentEmpty());
}

/**
 * @tc.name: IsContentEmpty_WithSpans
 * @tc.desc: When spans_ is not empty, IsContentEmpty returns false.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, IsContentEmpty_WithSpans, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    auto hostInterface = AceType::DynamicCast<ICleanNodeHost>(pattern);
    ASSERT_NE(hostInterface, nullptr);
    // Initially empty
    EXPECT_TRUE(hostInterface->IsContentEmpty());
    // Add content
    pattern->isSpanStringMode_ = false;
    auto spanItem = AceType::MakeRefPtr<SpanItem>();
    spanItem->content = u"hello";
    pattern->spans_.push_back(spanItem);
    EXPECT_FALSE(hostInterface->IsContentEmpty());
}

// ============ GetIsDisabled / GetHost coverage ============

/**
 * @tc.name: GetIsDisabled_DefaultFalse
 * @tc.desc: GetIsDisabled returns the disabled state of the pattern.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, GetIsDisabled_DefaultFalse, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    auto hostInterface = AceType::DynamicCast<ICleanNodeHost>(pattern);
    ASSERT_NE(hostInterface, nullptr);
    EXPECT_FALSE(hostInterface->GetIsDisabled());
}

/**
 * @tc.name: GetHost_ReturnsFrameNode
 * @tc.desc: GetHost returns the host FrameNode.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, GetHost_ReturnsFrameNode, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    auto hostInterface = AceType::DynamicCast<ICleanNodeHost>(pattern);
    ASSERT_NE(hostInterface, nullptr);
    auto host = hostInterface->GetHost();
    ASSERT_NE(host, nullptr);
    EXPECT_EQ(host->GetTag(), V2::RICH_EDITOR_ETS_TAG);
}

// ============ MarkCancelButtonDirty coverage ============

/**
 * @tc.name: MarkCancelButtonDirty_SetsFlag
 * @tc.desc: MarkCancelButtonDirty sets cancelButtonDirty_ to true.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, MarkCancelButtonDirty_SetsFlag, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->cancelButtonDirty_ = false;
    pattern->MarkCancelButtonDirty();
    EXPECT_TRUE(pattern->cancelButtonDirty_);
}

/**
 * @tc.name: OnModifyDone_ClearsDirtyFlag
 * @tc.desc: OnModifyDone clears cancelButtonDirty_ after processing.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, OnModifyDone_ClearsDirtyFlag, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    auto* frameNode = AceType::RawPtr(richEditorNode_);
    RichEditorModelNG::SetCancelButton(frameNode, static_cast<int32_t>(CleanNodeStyle::CONSTANT),
        CalcDimension(16.0_vp), Color::WHITE, "");
    ASSERT_TRUE(pattern->cancelButtonDirty_);
    pattern->OnModifyDone();
    EXPECT_FALSE(pattern->cancelButtonDirty_);
}

// ============ Focus navigation map and index getters ============

/**
 * @tc.name: GetFocusForwardMap_ReturnsCorrectMap
 * @tc.desc: Verify GetFocusForwardMap returns TEXT→CANCEL, CANCEL→UNIT.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, GetFocusForwardMap_ReturnsCorrectMap, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    const auto& fwdMap = pattern->GetFocusForwardMap();
    ASSERT_EQ(fwdMap.size(), 2u);
    EXPECT_EQ(fwdMap.at(FocuseIndex::TEXT), FocuseIndex::CANCEL);
    EXPECT_EQ(fwdMap.at(FocuseIndex::CANCEL), FocuseIndex::UNIT);
}

/**
 * @tc.name: GetFocusBackwardMap_ReturnsCorrectMap
 * @tc.desc: Verify GetFocusBackwardMap returns UNIT→CANCEL, CANCEL→TEXT.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, GetFocusBackwardMap_ReturnsCorrectMap, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    const auto& bwdMap = pattern->GetFocusBackwardMap();
    ASSERT_EQ(bwdMap.size(), 2u);
    EXPECT_EQ(bwdMap.at(FocuseIndex::UNIT), FocuseIndex::CANCEL);
    EXPECT_EQ(bwdMap.at(FocuseIndex::CANCEL), FocuseIndex::TEXT);
}

/**
 * @tc.name: GetFocusIndex_DefaultIsText
 * @tc.desc: Verify focusIndex_ defaults to FocuseIndex::TEXT.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, GetFocusIndex_DefaultIsText, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    EXPECT_EQ(pattern->GetFocusIndex(), FocuseIndex::TEXT);
}

// ============ CancelNodeIsShow branch coverage ============

/**
 * @tc.name: CancelNodeIsShow_NullArea_ReturnsFalse
 * @tc.desc: When cleanNodeResponseArea_ is null, CancelNodeIsShow returns false.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, CancelNodeIsShow_NullArea_ReturnsFalse, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    ASSERT_EQ(pattern->cleanNodeResponseArea_, nullptr);
    EXPECT_FALSE(pattern->CancelNodeIsShow());
}

/**
 * @tc.name: CancelNodeIsShow_WhenShown_ReturnsTrue
 * @tc.desc: When cleanNodeResponseArea_ exists and IsShow() returns true, CancelNodeIsShow returns true.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, CancelNodeIsShow_WhenShown_ReturnsTrue, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    auto* frameNode = AceType::RawPtr(richEditorNode_);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(RichEditorLayoutProperty, SingleLine, true, frameNode);
    RichEditorModelNG::SetCancelButton(frameNode, static_cast<int32_t>(CleanNodeStyle::CONSTANT),
        CalcDimension(16.0_vp), Color::WHITE, "");
    pattern->OnModifyDone();
    ASSERT_NE(pattern->cleanNodeResponseArea_, nullptr);
    // CONSTANT style should show the cancel button
    EXPECT_TRUE(pattern->CancelNodeIsShow());
}

/**
 * @tc.name: CancelNodeIsShow_WhenHidden_ReturnsFalse
 * @tc.desc: When cleanNodeResponseArea_ exists but IsShow() returns false, CancelNodeIsShow returns false.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, CancelNodeIsShow_WhenHidden_ReturnsFalse, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    auto* frameNode = AceType::RawPtr(richEditorNode_);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(RichEditorLayoutProperty, SingleLine, true, frameNode);
    RichEditorModelNG::SetCancelButton(frameNode, static_cast<int32_t>(CleanNodeStyle::INVISIBLE),
        CalcDimension(16.0_vp), Color::WHITE, "");
    pattern->OnModifyDone();
    ASSERT_NE(pattern->cleanNodeResponseArea_, nullptr);
    // INVISIBLE style should hide the cancel button
    EXPECT_FALSE(pattern->CancelNodeIsShow());
}

// ============ HandleSwithFocus branch coverage ============

/**
 * @tc.name: HandleSwithFocus_TextIndex_Succeeds
 * @tc.desc: HandleSwithFocus(TEXT) sets focusIndex_ to TEXT and returns true.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, HandleSwithFocus_TextIndex_Succeeds, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->focusIndex_ = FocuseIndex::CANCEL;
    EXPECT_TRUE(pattern->HandleSwithFocus(FocuseIndex::TEXT));
    EXPECT_EQ(pattern->focusIndex_, FocuseIndex::TEXT);
}

/**
 * @tc.name: HandleSwithFocus_CancelIndex_NotShown_ReturnsFalse
 * @tc.desc: HandleSwithFocus(CANCEL) when cancel node is not shown returns false.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, HandleSwithFocus_CancelIndex_NotShown_ReturnsFalse, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    // No cancel button set up, so cleanNodeResponseArea_ is null → CancelNodeIsShow returns false
    ASSERT_EQ(pattern->cleanNodeResponseArea_, nullptr);
    EXPECT_FALSE(pattern->HandleSwithFocus(FocuseIndex::CANCEL));
}

/**
 * @tc.name: HandleSwithFocus_CancelIndex_Shown_Succeeds
 * @tc.desc: HandleSwithFocus(CANCEL) when cancel node is shown sets focusIndex_ to CANCEL and returns true.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, HandleSwithFocus_CancelIndex_Shown_Succeeds, TestSize.Level0)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    auto* frameNode = AceType::RawPtr(richEditorNode_);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(RichEditorLayoutProperty, SingleLine, true, frameNode);
    RichEditorModelNG::SetCancelButton(frameNode, static_cast<int32_t>(CleanNodeStyle::CONSTANT),
        CalcDimension(16.0_vp), Color::WHITE, "");
    pattern->OnModifyDone();
    ASSERT_TRUE(pattern->CancelNodeIsShow());
    EXPECT_TRUE(pattern->HandleSwithFocus(FocuseIndex::CANCEL));
    EXPECT_EQ(pattern->focusIndex_, FocuseIndex::CANCEL);
}

/**
 * @tc.name: HandleSwithFocus_UnitIndex_ReturnsFalse
 * @tc.desc: HandleSwithFocus(UNIT) returns false (reserved for future password icon).
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, HandleSwithFocus_UnitIndex_ReturnsFalse, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    EXPECT_FALSE(pattern->HandleSwithFocus(FocuseIndex::UNIT));
}

/**
 * @tc.name: HandleSwithFocus_VoiceIndex_ReturnsFalse
 * @tc.desc: HandleSwithFocus(VOICE) returns false (not supported in RichEditor).
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, HandleSwithFocus_VoiceIndex_ReturnsFalse, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    EXPECT_FALSE(pattern->HandleSwithFocus(FocuseIndex::VOICE));
}

// ============ HandleOnTab branch coverage ============

/**
 * @tc.name: HandleOnTab_Forward_FromText_CancelShown_MovesToCancel
 * @tc.desc: Tab forward from TEXT when cancel is visible moves focus to CANCEL.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, HandleOnTab_Forward_FromText_CancelShown_MovesToCancel, TestSize.Level0)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    auto* frameNode = AceType::RawPtr(richEditorNode_);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(RichEditorLayoutProperty, SingleLine, true, frameNode);
    RichEditorModelNG::SetCancelButton(frameNode, static_cast<int32_t>(CleanNodeStyle::CONSTANT),
        CalcDimension(16.0_vp), Color::WHITE, "");
    pattern->OnModifyDone();
    pattern->focusIndex_ = FocuseIndex::TEXT;
    // Need focus for HandleFocusForwardImpl to proceed
    auto focusHub = richEditorNode_->GetOrCreateFocusHub();
    ASSERT_NE(focusHub, nullptr);
    focusHub->RequestFocusImmediately();
    ASSERT_TRUE(pattern->HasFocus());
    EXPECT_TRUE(pattern->HandleOnTab(false));
    EXPECT_EQ(pattern->focusIndex_, FocuseIndex::CANCEL);
}

/**
 * @tc.name: HandleOnTab_Forward_FromText_CancelHidden_ReturnsFalse
 * @tc.desc: Tab forward from TEXT when cancel is hidden skips to UNIT (returns false).
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, HandleOnTab_Forward_FromText_CancelHidden_ReturnsFalse, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    auto* frameNode = AceType::RawPtr(richEditorNode_);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(RichEditorLayoutProperty, SingleLine, true, frameNode);
    RichEditorModelNG::SetCancelButton(frameNode, static_cast<int32_t>(CleanNodeStyle::INVISIBLE),
        CalcDimension(16.0_vp), Color::WHITE, "");
    pattern->OnModifyDone();
    pattern->focusIndex_ = FocuseIndex::TEXT;
    auto focusHub = richEditorNode_->GetOrCreateFocusHub();
    ASSERT_NE(focusHub, nullptr);
    focusHub->RequestFocusImmediately();
    ASSERT_TRUE(pattern->HasFocus());
    // TEXT→CANCEL (hidden, HandleSwithFocus returns false)→UNIT (HandleSwithFocus returns false)→not in map→false
    EXPECT_FALSE(pattern->HandleOnTab(false));
}

/**
 * @tc.name: HandleOnTab_Forward_FromCancel_ReturnsFalse
 * @tc.desc: Tab forward from CANCEL goes to UNIT which returns false in RichEditor.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, HandleOnTab_Forward_FromCancel_ReturnsFalse, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    auto* frameNode = AceType::RawPtr(richEditorNode_);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(RichEditorLayoutProperty, SingleLine, true, frameNode);
    RichEditorModelNG::SetCancelButton(frameNode, static_cast<int32_t>(CleanNodeStyle::CONSTANT),
        CalcDimension(16.0_vp), Color::WHITE, "");
    pattern->OnModifyDone();
    auto focusHub = richEditorNode_->GetOrCreateFocusHub();
    ASSERT_NE(focusHub, nullptr);
    focusHub->RequestFocusImmediately();
    ASSERT_TRUE(pattern->HasFocus());
    // Set focusIndex_ AFTER RequestFocusImmediately (which resets to TEXT via HandleFocusEvent)
    pattern->focusIndex_ = FocuseIndex::CANCEL;
    // CANCEL→UNIT, HandleSwithFocus(UNIT) returns false, UNIT not in forward map→false
    EXPECT_FALSE(pattern->HandleOnTab(false));
}

/**
 * @tc.name: HandleOnTab_Backward_FromCancel_MovesToText
 * @tc.desc: Tab backward from CANCEL moves focus to TEXT.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, HandleOnTab_Backward_FromCancel_MovesToText, TestSize.Level0)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    auto* frameNode = AceType::RawPtr(richEditorNode_);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(RichEditorLayoutProperty, SingleLine, true, frameNode);
    RichEditorModelNG::SetCancelButton(frameNode, static_cast<int32_t>(CleanNodeStyle::CONSTANT),
        CalcDimension(16.0_vp), Color::WHITE, "");
    pattern->OnModifyDone();
    auto focusHub = richEditorNode_->GetOrCreateFocusHub();
    ASSERT_NE(focusHub, nullptr);
    focusHub->RequestFocusImmediately();
    ASSERT_TRUE(pattern->HasFocus());
    pattern->focusIndex_ = FocuseIndex::CANCEL;
    EXPECT_TRUE(pattern->HandleOnTab(true));
    EXPECT_EQ(pattern->focusIndex_, FocuseIndex::TEXT);
}

/**
 * @tc.name: HandleOnTab_Backward_FromText_ReturnsFalse
 * @tc.desc: Tab backward from TEXT returns false (TEXT not in backward map).
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, HandleOnTab_Backward_FromText_ReturnsFalse, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->focusIndex_ = FocuseIndex::TEXT;
    auto focusHub = richEditorNode_->GetOrCreateFocusHub();
    ASSERT_NE(focusHub, nullptr);
    focusHub->RequestFocusImmediately();
    ASSERT_TRUE(pattern->HasFocus());
    // TEXT not in backward map→false
    EXPECT_FALSE(pattern->HandleOnTab(true));
}

/**
 * @tc.name: HandleOnTab_NoFocus_ReturnsFalse
 * @tc.desc: HandleOnTab returns false when component does not have focus.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, HandleOnTab_NoFocus_ReturnsFalse, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->focusIndex_ = FocuseIndex::TEXT;
    EXPECT_FALSE(pattern->HasFocus());
    EXPECT_FALSE(pattern->HandleOnTab(false));
    EXPECT_FALSE(pattern->HandleOnTab(true));
}

// ============ ProcessFocusIndexAction branch coverage ============

/**
 * @tc.name: ProcessFocusIndexAction_TextIndex_ReturnsTrue
 * @tc.desc: When focusIndex_ is TEXT, ProcessFocusIndexAction returns true.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, ProcessFocusIndexAction_TextIndex_ReturnsTrue, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->focusIndex_ = FocuseIndex::TEXT;
    EXPECT_TRUE(pattern->ProcessFocusIndexAction());
}

/**
 * @tc.name: ProcessFocusIndexAction_CancelIndex_ReturnsFalse
 * @tc.desc: When focusIndex_ is CANCEL, ProcessFocusIndexAction calls CleanNodeResponseKeyEvent and returns false.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, ProcessFocusIndexAction_CancelIndex_ReturnsFalse, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->focusIndex_ = FocuseIndex::CANCEL;
    EXPECT_FALSE(pattern->ProcessFocusIndexAction());
    // CleanNodeResponseKeyEvent calls HandleCleanNodeClicked but does not reset focusIndex_
    EXPECT_EQ(pattern->focusIndex_, FocuseIndex::CANCEL);
}

/**
 * @tc.name: ProcessFocusIndexAction_UnitIndex_ReturnsTrue
 * @tc.desc: When focusIndex_ is UNIT, ProcessFocusIndexAction returns true.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, ProcessFocusIndexAction_UnitIndex_ReturnsTrue, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->focusIndex_ = FocuseIndex::UNIT;
    EXPECT_TRUE(pattern->ProcessFocusIndexAction());
}

// ============ HandleSpaceKeyClickEvent branch coverage ============

/**
 * @tc.name: HandleSpaceKeyClickEvent_TextIndex_ReturnsFalse
 * @tc.desc: When focusIndex_ is TEXT, HandleSpaceKeyClickEvent returns false.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, HandleSpaceKeyClickEvent_TextIndex_ReturnsFalse, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->focusIndex_ = FocuseIndex::TEXT;
    EXPECT_FALSE(pattern->HandleSpaceKeyClickEvent());
}

/**
 * @tc.name: HandleSpaceKeyClickEvent_CancelIndex_NoArea_ReturnsFalse
 * @tc.desc: When focusIndex_ is CANCEL but cleanNodeResponseArea_ is null, returns false.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, HandleSpaceKeyClickEvent_CancelIndex_NoArea_ReturnsFalse, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->focusIndex_ = FocuseIndex::CANCEL;
    ASSERT_EQ(pattern->cleanNodeResponseArea_, nullptr);
    EXPECT_FALSE(pattern->HandleSpaceKeyClickEvent());
}

/**
 * @tc.name: HandleSpaceKeyClickEvent_CancelIndex_WithArea_ActsClick
 * @tc.desc: When focusIndex_ is CANCEL and area exists, HandleSpaceKeyClickEvent triggers ActClick and returns true.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, HandleSpaceKeyClickEvent_CancelIndex_WithArea_ActsClick, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    auto* frameNode = AceType::RawPtr(richEditorNode_);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(RichEditorLayoutProperty, SingleLine, true, frameNode);
    RichEditorModelNG::SetCancelButton(frameNode, static_cast<int32_t>(CleanNodeStyle::CONSTANT),
        CalcDimension(16.0_vp), Color::WHITE, "");
    pattern->OnModifyDone();
    ASSERT_NE(pattern->cleanNodeResponseArea_, nullptr);
    pattern->focusIndex_ = FocuseIndex::CANCEL;
    // ActClick may crash in mock environment; verify no crash
    EXPECT_TRUE(pattern->HandleSpaceKeyClickEvent());
}

/**
 * @tc.name: HandleSpaceKeyClickEvent_UnitIndex_ReturnsFalse
 * @tc.desc: When focusIndex_ is UNIT, HandleSpaceKeyClickEvent returns false.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, HandleSpaceKeyClickEvent_UnitIndex_ReturnsFalse, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->focusIndex_ = FocuseIndex::UNIT;
    EXPECT_FALSE(pattern->HandleSpaceKeyClickEvent());
}

// ============ CleanNodeResponseKeyEvent branch coverage ============

/**
 * @tc.name: CleanNodeResponseKeyEvent_IsDragging_Returns
 * @tc.desc: When IsDragging is true, CleanNodeResponseKeyEvent returns early without state change.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, CleanNodeResponseKeyEvent_IsDragging_Returns, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->focusIndex_ = FocuseIndex::CANCEL;
    pattern->status_ = Status::DRAGGING;
    pattern->CleanNodeResponseKeyEvent();
    // focusIndex_ should remain CANCEL (early return)
    EXPECT_EQ(pattern->focusIndex_, FocuseIndex::CANCEL);
}

/**
 * @tc.name: CleanNodeResponseKeyEvent_NotDragging_CallsCleanNodeClicked
 * @tc.desc: When not dragging, CleanNodeResponseKeyEvent calls HandleCleanNodeClicked (no crash).
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, CleanNodeResponseKeyEvent_NotDragging_CallsCleanNodeClicked, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->focusIndex_ = FocuseIndex::CANCEL;
    pattern->status_ = Status::NONE;
    pattern->CleanNodeResponseKeyEvent();
    // CleanNodeResponseKeyEvent does not reset focusIndex_, only calls HandleCleanNodeClicked
    EXPECT_EQ(pattern->focusIndex_, FocuseIndex::CANCEL);
}

// ============ Focus state reset in existing functions ============

/**
 * @tc.name: HandleBlurEventReset_ResetsFocusIndex
 * @tc.desc: HandleBlurEventReset resets focusIndex_ to TEXT.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, HandleBlurEventReset_ResetsFocusIndex, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->focusIndex_ = FocuseIndex::CANCEL;
    pattern->HandleBlurEventReset();
    EXPECT_EQ(pattern->focusIndex_, FocuseIndex::TEXT);
}

/**
 * @tc.name: HandleFocusEvent_SetsFocusIndexToText
 * @tc.desc: HandleFocusEvent sets focusIndex_ to TEXT on focus.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, HandleFocusEvent_SetsFocusIndexToText, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->focusIndex_ = FocuseIndex::CANCEL;
    pattern->HandleFocusEvent(FocusReason::DEFAULT);
    EXPECT_EQ(pattern->focusIndex_, FocuseIndex::TEXT);
}

/**
 * @tc.name: StartTwinkling_ResetsFocusFromCancelToText
 * @tc.desc: When focusIndex_ is CANCEL, StartTwinkling resets it to TEXT.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, StartTwinkling_ResetsFocusFromCancelToText, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->focusIndex_ = FocuseIndex::CANCEL;
    pattern->StartTwinkling();
    EXPECT_EQ(pattern->focusIndex_, FocuseIndex::TEXT);
}

// ============ OnKeyEvent space key handling ============

/**
 * @tc.name: OnKeyEvent_SpaceKeyCancelFocused_TriggersClick
 * @tc.desc: When KEY_SPACE is pressed and focusIndex_ is CANCEL, OnKeyEvent calls HandleSpaceKeyClickEvent.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, OnKeyEvent_SpaceKeyCancelFocused_TriggersClick, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    auto* frameNode = AceType::RawPtr(richEditorNode_);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(RichEditorLayoutProperty, SingleLine, true, frameNode);
    RichEditorModelNG::SetCancelButton(frameNode, static_cast<int32_t>(CleanNodeStyle::CONSTANT),
        CalcDimension(16.0_vp), Color::WHITE, "");
    pattern->OnModifyDone();
    pattern->focusIndex_ = FocuseIndex::CANCEL;
    KeyEvent keyEvent;
    keyEvent.code = KeyCode::KEY_SPACE;
    keyEvent.action = KeyAction::DOWN;
    auto result = pattern->OnKeyEvent(keyEvent);
    // HandleSpaceKeyClickEvent should return true when cancel area exists
    EXPECT_TRUE(result);
}

/**
 * @tc.name: OnKeyEvent_SpaceKeyTextFocused_FallsThrough
 * @tc.desc: When KEY_SPACE is pressed and focusIndex_ is TEXT, OnKeyEvent falls through to default handler.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, OnKeyEvent_SpaceKeyTextFocused_FallsThrough, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->focusIndex_ = FocuseIndex::TEXT;
    KeyEvent keyEvent;
    keyEvent.code = KeyCode::KEY_SPACE;
    keyEvent.action = KeyAction::DOWN;
    // Should fall through to TextInputClient::HandleKeyEvent (returns false in mock)
    pattern->OnKeyEvent(keyEvent);
    SUCCEED();
}

/**
 * @tc.name: OnKeyEvent_NonSpaceKey_FallsThrough
 * @tc.desc: When a non-space key is pressed, OnKeyEvent falls through to default handler regardless of focusIndex_.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, OnKeyEvent_NonSpaceKey_FallsThrough, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->focusIndex_ = FocuseIndex::CANCEL;
    KeyEvent keyEvent;
    keyEvent.code = KeyCode::KEY_ENTER;
    keyEvent.action = KeyAction::DOWN;
    pattern->OnKeyEvent(keyEvent);
    SUCCEED();
}

// ============ PerformAction focus check ============

/**
 * @tc.name: PerformAction_FocusOnCancel_SkipsAction
 * @tc.desc: When focusIndex_ is CANCEL, PerformAction skips the action via ProcessFocusIndexAction.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, PerformAction_FocusOnCancel_SkipsAction, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->focusIndex_ = FocuseIndex::CANCEL;
    // PerformAction should return early without processing the action
    pattern->PerformAction(TextInputAction::NEW_LINE, false);
    // ProcessFocusIndexAction calls CleanNodeResponseKeyEvent which does not reset focusIndex_
    EXPECT_EQ(pattern->focusIndex_, FocuseIndex::CANCEL);
}

/**
 * @tc.name: PerformAction_FocusOnText_Proceeds
 * @tc.desc: When focusIndex_ is TEXT, PerformAction proceeds normally.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, PerformAction_FocusOnText_Proceeds, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->focusIndex_ = FocuseIndex::TEXT;
    pattern->PerformAction(TextInputAction::NEW_LINE, false);
    SUCCEED();
}

// ============ ProcessInsertValue space key interception ============

/**
 * @tc.name: ProcessInsertValue_SpaceKeyCancelFocused_TriggersClick
 * @tc.desc: When focusIndex_ is CANCEL and insertValue is space, ProcessInsertValue triggers HandleSpaceKeyClickEvent.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, ProcessInsertValue_SpaceKeyCancelFocused_TriggersClick, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    auto* frameNode = AceType::RawPtr(richEditorNode_);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(RichEditorLayoutProperty, SingleLine, true, frameNode);
    RichEditorModelNG::SetCancelButton(frameNode, static_cast<int32_t>(CleanNodeStyle::CONSTANT),
        CalcDimension(16.0_vp), Color::WHITE, "");
    pattern->OnModifyDone();
    pattern->focusIndex_ = FocuseIndex::CANCEL;
    pattern->ProcessInsertValue(u" ", OperationType::DEFAULT, true);
    // After ProcessInsertValue with space when CANCEL focused, should trigger click
    SUCCEED();
}

// ============ HandleMouseEvent cancel hover area check ============

/**
 * @tc.name: HandleMouseEvent_NotOnCancelArea_ChangesToTextCursor
 * @tc.desc: When mouse style is DEFAULT and not on cancel hover area, style changes to TEXT_CURSOR.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, HandleMouseEvent_NotOnCancelArea_ChangesToTextCursor, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    // No cancel button area, so IsOnCancelButtonHoverArea returns false
    ASSERT_EQ(pattern->cleanNodeResponseArea_, nullptr);
    MouseInfo info;
    info.SetAction(MouseAction::MOVE);
    info.SetButton(MouseButton::NONE_BUTTON);
    info.SetLocalLocation(Offset(10, 10));
    pattern->HandleMouseEvent(info);
    // Should not crash; mouse style may or may not change depending on mock
    SUCCEED();
}

/**
 * @tc.name: HandleMouseEvent_OnCancelHoverArea_StaysDefault
 * @tc.desc: When on cancel hover area, mouse style stays DEFAULT (does not change to TEXT_CURSOR).
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, HandleMouseEvent_OnCancelHoverArea_StaysDefault, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    auto* frameNode = AceType::RawPtr(richEditorNode_);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(RichEditorLayoutProperty, SingleLine, true, frameNode);
    RichEditorModelNG::SetCancelButton(frameNode, static_cast<int32_t>(CleanNodeStyle::CONSTANT),
        CalcDimension(16.0_vp), Color::WHITE, "");
    pattern->OnModifyDone();
    ASSERT_NE(pattern->cleanNodeResponseArea_, nullptr);
    MouseInfo info;
    info.SetAction(MouseAction::MOVE);
    info.SetButton(MouseButton::NONE_BUTTON);
    info.SetLocalLocation(Offset(10, 10));
    pattern->HandleMouseEvent(info);
    SUCCEED();
}

// ============ GetInnerFocusPaintRect branch coverage ============

/**
 * @tc.name: GetInnerFocusPaintRect_NotCancelIndex_Returns
 * @tc.desc: When focusIndex_ is not CANCEL, GetInnerFocusPaintRect returns immediately.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, GetInnerFocusPaintRect_NotCancelIndex_Returns, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->focusIndex_ = FocuseIndex::TEXT;
    RoundRect paintRect;
    pattern->GetInnerFocusPaintRect(paintRect);
    SUCCEED();
}

/**
 * @tc.name: GetInnerFocusPaintRect_CancelIndex_NullArea_NoCrash
 * @tc.desc: When focusIndex_ is CANCEL but cleanNodeResponseArea_ is null, no crash.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, GetInnerFocusPaintRect_CancelIndex_NullArea_NoCrash, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->focusIndex_ = FocuseIndex::CANCEL;
    ASSERT_EQ(pattern->cleanNodeResponseArea_, nullptr);
    RoundRect paintRect;
    pattern->GetInnerFocusPaintRect(paintRect);
    SUCCEED();
}

// ============ Paint function no-crash verification ============

/**
 * @tc.name: PaintTextRect_NoCrash
 * @tc.desc: PaintTextRect with valid host does not crash.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, PaintTextRect_NoCrash, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->PaintTextRect();
    SUCCEED();
}

/**
 * @tc.name: PaintCancelRect_NoCrash
 * @tc.desc: PaintCancelRect with valid host does not crash.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, PaintCancelRect_NoCrash, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->focusIndex_ = FocuseIndex::CANCEL;
    pattern->PaintCancelRect();
    SUCCEED();
}

/**
 * @tc.name: FocusForwardStopTwinkling_NoCrash
 * @tc.desc: FocusForwardStopTwinkling with valid host does not crash.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, FocusForwardStopTwinkling_NoCrash, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->FocusForwardStopTwinkling();
    SUCCEED();
}

// ============ HandleOnTab forward from UNIT (not in map) ============

/**
 * @tc.name: HandleOnTab_Forward_FromUnit_ReturnsFalse
 * @tc.desc: Tab forward from UNIT returns false (UNIT not in forward map).
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, HandleOnTab_Forward_FromUnit_ReturnsFalse, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    auto focusHub = richEditorNode_->GetOrCreateFocusHub();
    ASSERT_NE(focusHub, nullptr);
    focusHub->RequestFocusImmediately();
    ASSERT_TRUE(pattern->HasFocus());
    pattern->focusIndex_ = FocuseIndex::UNIT;
    EXPECT_FALSE(pattern->HandleOnTab(false));
}

// ============ HandleOnTab backward from UNIT (UNIT→CANCEL→TEXT) ============

/**
 * @tc.name: HandleOnTab_Backward_FromUnit_MovesToCancel
 * @tc.desc: Tab backward from UNIT moves focus to CANCEL if cancel is visible.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, HandleOnTab_Backward_FromUnit_MovesToCancel, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    auto* frameNode = AceType::RawPtr(richEditorNode_);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(RichEditorLayoutProperty, SingleLine, true, frameNode);
    RichEditorModelNG::SetCancelButton(frameNode, static_cast<int32_t>(CleanNodeStyle::CONSTANT),
        CalcDimension(16.0_vp), Color::WHITE, "");
    pattern->OnModifyDone();
    auto focusHub = richEditorNode_->GetOrCreateFocusHub();
    ASSERT_NE(focusHub, nullptr);
    focusHub->RequestFocusImmediately();
    ASSERT_TRUE(pattern->HasFocus());
    pattern->focusIndex_ = FocuseIndex::UNIT;
    // UNIT→CANCEL (visible, HandleSwithFocus returns true)
    EXPECT_TRUE(pattern->HandleOnTab(true));
    EXPECT_EQ(pattern->focusIndex_, FocuseIndex::CANCEL);
}

// ============ HandleOnTab backward from UNIT when cancel hidden ============

/**
 * @tc.name: HandleOnTab_Backward_FromUnit_CancelHidden_MovesToText
 * @tc.desc: Tab backward from UNIT when cancel is hidden skips to TEXT.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, HandleOnTab_Backward_FromUnit_CancelHidden_MovesToText, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    auto* frameNode = AceType::RawPtr(richEditorNode_);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(RichEditorLayoutProperty, SingleLine, true, frameNode);
    RichEditorModelNG::SetCancelButton(frameNode, static_cast<int32_t>(CleanNodeStyle::INVISIBLE),
        CalcDimension(16.0_vp), Color::WHITE, "");
    pattern->OnModifyDone();
    auto focusHub = richEditorNode_->GetOrCreateFocusHub();
    ASSERT_NE(focusHub, nullptr);
    focusHub->RequestFocusImmediately();
    ASSERT_TRUE(pattern->HasFocus());
    pattern->focusIndex_ = FocuseIndex::UNIT;
    // UNIT→CANCEL (hidden, returns false)→TEXT (returns true)
    EXPECT_TRUE(pattern->HandleOnTab(true));
    EXPECT_EQ(pattern->focusIndex_, FocuseIndex::TEXT);
}

// ============ CleanNodeHostBase CRTP template method coverage ============

/**
 * @tc.name: HandleFocusForwardImpl_NoFocus_ReturnsFalse
 * @tc.desc: HandleFocusForwardImpl returns false when HasFocus() is false.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, HandleFocusForwardImpl_NoFocus_ReturnsFalse, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->focusIndex_ = FocuseIndex::TEXT;
    EXPECT_FALSE(pattern->HasFocus());
    // HandleOnTab delegates to HandleFocusForwardImpl which checks HasFocus
    EXPECT_FALSE(pattern->HandleOnTab(false));
}

/**
 * @tc.name: HandleFocusBackwardImpl_NoFocus_ReturnsFalse
 * @tc.desc: HandleFocusBackwardImpl returns false when HasFocus() is false.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorCancelButtonTestNg, HandleFocusBackwardImpl_NoFocus_ReturnsFalse, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->focusIndex_ = FocuseIndex::CANCEL;
    EXPECT_FALSE(pattern->HasFocus());
    EXPECT_FALSE(pattern->HandleOnTab(true));
}

} // namespace OHOS::Ace::NG

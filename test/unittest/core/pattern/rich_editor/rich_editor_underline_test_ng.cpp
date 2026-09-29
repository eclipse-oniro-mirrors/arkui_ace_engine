/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "core/components_ng/pattern/rich_editor/rich_editor_model_static.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_model_ng.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_theme.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_overlay_modifier.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_paint_method.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_content_pattern.h"
#include "test/mock/frameworks/core/rosen/mock_canvas.h"
#include "test/mock/frameworks/core/components_ng/render/mock_paragraph.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"
#include "test/mock/frameworks/core/common/mock_container.h"
#include "test/mock/frameworks/core/common/mock_theme_manager.h"
#include "test/mock/frameworks/core/common/mock_resource_adapter_v2.h"
#include "test/mock/frameworks/base/thread/mock_task_executor.h"
#include "core/components_ng/pattern/text_field/text_field_model.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {
constexpr Dimension EXPECTED_TYPING_WIDTH = 2.0_px;
constexpr Dimension EXPECTED_NORMAL_WIDTH = 1.0_px;
const Color USER_TYPING_COLOR = Color(0xFF111111);
const Color USER_NORMAL_COLOR = Color(0xFF222222);
const Color USER_DISABLE_COLOR = Color(0xFF333333);
const Color USER_ERROR_COLOR = Color(0xFF444444);
} // namespace

// Extern declarations for dynamic modifier bridge functions (global C functions)
struct _ArkUINode;
typedef _ArkUINode* ArkUINodeHandle;
typedef int ArkUI_Int32;
typedef unsigned int ArkUI_Uint32;
constexpr uint32_t BRIDGE_ERROR_UINT_CODE = static_cast<uint32_t>(-1);
extern void SetRichEditorShowUnderline(ArkUINodeHandle node, ArkUI_Int32 showUnderline);
extern ArkUI_Int32 GetRichEditorShowUnderline(ArkUINodeHandle node);
extern void ResetRichEditorShowUnderline(ArkUINodeHandle node);
extern void SetRichEditorUnderlineColor(ArkUINodeHandle node, const ArkUI_Uint32* colors, ArkUI_Uint32 size);
extern const ArkUI_Uint32* GetRichEditorUnderlineColor(ArkUINodeHandle node, ArkUI_Uint32* size);
extern void ResetRichEditorUnderlineColor(ArkUINodeHandle node);

class RichEditorUnderlineTestNg : public RichEditorCommonTestNg {
public:
    void SetUp() override;
    void TearDown() override;
    static void TearDownTestSuite();
    RefPtr<RichEditorTheme> theme_;
    RefPtr<MockThemeManager> themeManager_;

    void SetupTheme();
    void SetupFocus(bool hasFocus);
    void SetupEnabled(bool enabled);
};

void RichEditorUnderlineTestNg::SetUp()
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
    richEditorNode_->GetGeometryNode()->SetContentSize({ 300.0f, 300.0f });
    SetupTheme();
}

void RichEditorUnderlineTestNg::TearDown()
{
    richEditorNode_.Reset();
    MockParagraph::TearDown();
}

void RichEditorUnderlineTestNg::TearDownTestSuite()
{
    TestNG::TearDownTestSuite();
}

void RichEditorUnderlineTestNg::SetupTheme()
{
    theme_ = AceType::MakeRefPtr<RichEditorTheme>();
    themeManager_ = AceType::MakeRefPtr<MockThemeManager>();
    EXPECT_CALL(*themeManager_, GetTheme(_)).WillRepeatedly(Return(theme_));
    EXPECT_CALL(*themeManager_, GetTheme(_, _)).WillRepeatedly(Return(theme_));
    PipelineBase::GetCurrentContext()->themeManager_ = themeManager_;
}

void RichEditorUnderlineTestNg::SetupFocus(bool hasFocus)
{
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto focusHub = richEditorPattern->GetFocusHub();
    ASSERT_NE(focusHub, nullptr);
    if (hasFocus) {
        focusHub->RequestFocusImmediately();
    } else {
        focusHub->UnFocusImmediately();
    }
}

void RichEditorUnderlineTestNg::SetupEnabled(bool enabled)
{
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto eventHub = richEditorPattern->GetEventHub<RichEditorEventHub>();
    ASSERT_NE(eventHub, nullptr);
    eventHub->SetEnabled(enabled);
}

// =================== ModelNG Tests ===================

/**
 * @tc.name: RichEditorModelNGShowUnderline001
 * @tc.desc: Test SetShowUnderline / GetShowUnderline / ResetShowUnderline via ModelNG
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, RichEditorModelNGShowUnderline001, TestSize.Level1)
{
    auto* frameNode = AceType::RawPtr(richEditorNode_);
    ASSERT_NE(frameNode, nullptr);

    // Default value is false
    EXPECT_FALSE(RichEditorModelNG::GetShowUnderline(frameNode));

    // Set true
    RichEditorModelNG::SetShowUnderline(frameNode, true);
    EXPECT_TRUE(RichEditorModelNG::GetShowUnderline(frameNode));

    // Set false
    RichEditorModelNG::SetShowUnderline(frameNode, false);
    EXPECT_FALSE(RichEditorModelNG::GetShowUnderline(frameNode));

    // Reset
    RichEditorModelNG::SetShowUnderline(frameNode, true);
    RichEditorModelNG::ResetShowUnderline(frameNode);
    EXPECT_FALSE(RichEditorModelNG::GetShowUnderline(frameNode));
}

/**
 * @tc.name: RichEditorModelNGUnderlineColor001
 * @tc.desc: Test SetUnderlineColor / GetUnderlineColor / ResetUnderlineColor via ModelNG
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, RichEditorModelNGUnderlineColor001, TestSize.Level1)
{
    auto* frameNode = AceType::RawPtr(richEditorNode_);
    ASSERT_NE(frameNode, nullptr);

    // Default: all nullopt
    auto defaultColor = RichEditorModelNG::GetUnderlineColor(frameNode);
    EXPECT_FALSE(defaultColor.typing.has_value());
    EXPECT_FALSE(defaultColor.normal.has_value());
    EXPECT_FALSE(defaultColor.error.has_value());
    EXPECT_FALSE(defaultColor.disable.has_value());

    // Set user colors
    UserUnderlineColor userColor;
    userColor.typing = USER_TYPING_COLOR;
    userColor.normal = USER_NORMAL_COLOR;
    userColor.error = USER_ERROR_COLOR;
    userColor.disable = USER_DISABLE_COLOR;
    RichEditorModelNG::SetUnderlineColor(frameNode, userColor);

    auto got = RichEditorModelNG::GetUnderlineColor(frameNode);
    EXPECT_TRUE(got.typing.has_value());
    EXPECT_EQ(got.typing.value(), USER_TYPING_COLOR);
    EXPECT_TRUE(got.normal.has_value());
    EXPECT_EQ(got.normal.value(), USER_NORMAL_COLOR);
    EXPECT_TRUE(got.error.has_value());
    EXPECT_EQ(got.error.value(), USER_ERROR_COLOR);
    EXPECT_TRUE(got.disable.has_value());
    EXPECT_EQ(got.disable.value(), USER_DISABLE_COLOR);

    // Reset
    RichEditorModelNG::ResetUnderlineColor(frameNode);
    auto resetColor = RichEditorModelNG::GetUnderlineColor(frameNode);
    EXPECT_FALSE(resetColor.typing.has_value());
    EXPECT_FALSE(resetColor.normal.has_value());
    EXPECT_FALSE(resetColor.error.has_value());
    EXPECT_FALSE(resetColor.disable.has_value());
}

// =================== Pattern: IsUnderlineMode Tests ===================

/**
 * @tc.name: IsUnderlineMode001
 * @tc.desc: Test IsUnderlineMode returns true when ShowUnderline is set
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, IsUnderlineMode001, TestSize.Level1)
{
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);

    // Default false
    EXPECT_FALSE(richEditorPattern->IsUnderlineMode());

    // Set true
    RichEditorModelNG::SetShowUnderline(AceType::RawPtr(richEditorNode_), true);
    EXPECT_TRUE(richEditorPattern->IsUnderlineMode());

    // Set false
    RichEditorModelNG::SetShowUnderline(AceType::RawPtr(richEditorNode_), false);
    EXPECT_FALSE(richEditorPattern->IsUnderlineMode());
}

// =================== Pattern: IsUnderlineAndButtonMode Tests ===================

/**
 * @tc.name: IsUnderlineAndButtonMode001
 * @tc.desc: Test IsUnderlineAndButtonMode returns correct value
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, IsUnderlineAndButtonMode001, TestSize.Level1)
{
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);

    // Default: not underline mode, so false
    EXPECT_FALSE(richEditorPattern->IsUnderlineAndButtonMode());

    // Set underline mode but not button mode
    RichEditorModelNG::SetShowUnderline(AceType::RawPtr(richEditorNode_), true);
    // SingleLine must be true for IsShowCancelButtonMode to potentially return true
    // Without SingleLine, IsShowCancelButtonMode returns false
    EXPECT_FALSE(richEditorPattern->IsUnderlineAndButtonMode());
}

// =================== Pattern: UpdateUnderlineStyle Tests ===================

/**
 * @tc.name: UpdateUnderlineStyle001
 * @tc.desc: Test UpdateUnderlineStyle returns early when not underline mode
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, UpdateUnderlineStyle001, TestSize.Level1)
{
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);

    // Not underline mode -> should not change default values
    richEditorPattern->underlineColor_ = Color::TRANSPARENT;
    richEditorPattern->underlineWidth_ = 0.0_vp;
    richEditorPattern->UpdateUnderlineStyle();
    EXPECT_EQ(richEditorPattern->underlineColor_, Color::TRANSPARENT);
    EXPECT_EQ(richEditorPattern->underlineWidth_, 0.0_vp);
}

/**
 * @tc.name: UpdateUnderlineStyle002
 * @tc.desc: Test UpdateUnderlineStyle with focus and no user colors -> typing color from theme
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, UpdateUnderlineStyle002, TestSize.Level1)
{
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);

    RichEditorModelNG::SetShowUnderline(AceType::RawPtr(richEditorNode_), true);
    SetupEnabled(true);
    SetupFocus(true);

    richEditorPattern->UpdateUnderlineStyle();
    EXPECT_EQ(richEditorPattern->underlineColor_, theme_->GetUnderlineTypingColor());
    EXPECT_EQ(richEditorPattern->underlineWidth_, EXPECTED_TYPING_WIDTH);
}

/**
 * @tc.name: UpdateUnderlineStyle003
 * @tc.desc: Test UpdateUnderlineStyle without focus and no user colors -> normal color from theme
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, UpdateUnderlineStyle003, TestSize.Level1)
{
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);

    RichEditorModelNG::SetShowUnderline(AceType::RawPtr(richEditorNode_), true);
    SetupEnabled(true);
    SetupFocus(false);

    richEditorPattern->UpdateUnderlineStyle();
    EXPECT_EQ(richEditorPattern->underlineColor_, theme_->GetUnderlineColor());
    EXPECT_EQ(richEditorPattern->underlineWidth_, EXPECTED_NORMAL_WIDTH);
}

/**
 * @tc.name: UpdateUnderlineStyle004
 * @tc.desc: Test UpdateUnderlineStyle with focus and user typing color
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, UpdateUnderlineStyle004, TestSize.Level1)
{
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);

    RichEditorModelNG::SetShowUnderline(AceType::RawPtr(richEditorNode_), true);
    UserUnderlineColor userColor;
    userColor.typing = USER_TYPING_COLOR;
    userColor.normal = USER_NORMAL_COLOR;
    RichEditorModelNG::SetUnderlineColor(AceType::RawPtr(richEditorNode_), userColor);
    SetupEnabled(true);
    SetupFocus(true);

    richEditorPattern->UpdateUnderlineStyle();
    EXPECT_EQ(richEditorPattern->underlineColor_, USER_TYPING_COLOR);
    EXPECT_EQ(richEditorPattern->underlineWidth_, EXPECTED_TYPING_WIDTH);
}

/**
 * @tc.name: UpdateUnderlineStyle005
 * @tc.desc: Test UpdateUnderlineStyle without focus and user normal color
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, UpdateUnderlineStyle005, TestSize.Level1)
{
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);

    RichEditorModelNG::SetShowUnderline(AceType::RawPtr(richEditorNode_), true);
    UserUnderlineColor userColor;
    userColor.typing = USER_TYPING_COLOR;
    userColor.normal = USER_NORMAL_COLOR;
    RichEditorModelNG::SetUnderlineColor(AceType::RawPtr(richEditorNode_), userColor);
    SetupEnabled(true);
    SetupFocus(false);

    richEditorPattern->UpdateUnderlineStyle();
    EXPECT_EQ(richEditorPattern->underlineColor_, USER_NORMAL_COLOR);
    EXPECT_EQ(richEditorPattern->underlineWidth_, EXPECTED_NORMAL_WIDTH);
}

/**
 * @tc.name: UpdateUnderlineStyle006
 * @tc.desc: Test UpdateUnderlineStyle disabled with user disable color set
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, UpdateUnderlineStyle006, TestSize.Level1)
{
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);

    RichEditorModelNG::SetShowUnderline(AceType::RawPtr(richEditorNode_), true);
    UserUnderlineColor userColor;
    userColor.disable = USER_DISABLE_COLOR;
    RichEditorModelNG::SetUnderlineColor(AceType::RawPtr(richEditorNode_), userColor);
    SetupEnabled(false); // IsDisabled() returns true
    SetupFocus(false);

    richEditorPattern->UpdateUnderlineStyle();
    // userColor.disable has value and IsDisabled -> userColor.disable.value()
    EXPECT_EQ(richEditorPattern->underlineColor_, USER_DISABLE_COLOR);
}

/**
 * @tc.name: UpdateUnderlineStyle007
 * @tc.desc: Test UpdateUnderlineStyle disabled without user disable color -> theme disable color
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, UpdateUnderlineStyle007, TestSize.Level1)
{
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);

    RichEditorModelNG::SetShowUnderline(AceType::RawPtr(richEditorNode_), true);
    // No user colors set (disable is nullopt)
    SetupEnabled(false); // IsDisabled() returns true
    SetupFocus(false);

    richEditorPattern->UpdateUnderlineStyle();
    EXPECT_EQ(richEditorPattern->underlineColor_, theme_->GetDisableUnderlineColor());
}

/**
 * @tc.name: UpdateUnderlineStyle008
 * @tc.desc: Test UpdateUnderlineStyle not disabled with user disable color set -> underline color from focus
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, UpdateUnderlineStyle008, TestSize.Level1)
{
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);

    RichEditorModelNG::SetShowUnderline(AceType::RawPtr(richEditorNode_), true);
    UserUnderlineColor userColor;
    userColor.disable = USER_DISABLE_COLOR;
    userColor.normal = USER_NORMAL_COLOR;
    RichEditorModelNG::SetUnderlineColor(AceType::RawPtr(richEditorNode_), userColor);
    SetupEnabled(true); // not disabled
    SetupFocus(false);

    richEditorPattern->UpdateUnderlineStyle();
    // userColor.disable has value but !IsDisabled -> underlineColor from focus state (normal)
    EXPECT_EQ(richEditorPattern->underlineColor_, USER_NORMAL_COLOR);
}

// =================== Pattern: GetUnderlineColor/Width Tests ===================

/**
 * @tc.name: GetUnderlineColorWidth001
 * @tc.desc: Test GetUnderlineColor and GetUnderlineWidth default values
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, GetUnderlineColorWidth001, TestSize.Level1)
{
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);

    // Defaults
    EXPECT_EQ(richEditorPattern->GetUnderlineColor(), Color::TRANSPARENT);
    EXPECT_EQ(richEditorPattern->GetUnderlineWidth(), 0.0_vp);

    // After UpdateUnderlineStyle with focus
    RichEditorModelNG::SetShowUnderline(AceType::RawPtr(richEditorNode_), true);
    SetupEnabled(true);
    SetupFocus(true);
    richEditorPattern->UpdateUnderlineStyle();
    EXPECT_EQ(richEditorPattern->GetUnderlineColor(), theme_->GetUnderlineTypingColor());
    EXPECT_EQ(richEditorPattern->GetUnderlineWidth(), EXPECTED_TYPING_WIDTH);
}

// =================== OverlayModifier: Set/Get Tests ===================

/**
 * @tc.name: OverlayModifierUnderlineProperties001
 * @tc.desc: Test SetUnderlineColor and SetUnderlineWidth on overlay modifier
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, OverlayModifierUnderlineProperties001, TestSize.Level1)
{
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);

    richEditorPattern->CreateRichEditorOverlayModifier();
    auto overlayMod = AceType::DynamicCast<RichEditorOverlayModifier>(richEditorPattern->overlayMod_);
    ASSERT_NE(overlayMod, nullptr);

    // Set underline color
    Color testColor(0xFFAABBCC);
    overlayMod->SetUnderlineColor(testColor);
    EXPECT_EQ(overlayMod->underlineColor_->Get(), testColor);

    // Set underline width
    float testWidth = 5.0f;
    overlayMod->SetUnderlineWidth(testWidth);
    EXPECT_FLOAT_EQ(overlayMod->underlineWidth_->Get(), testWidth);

    // Change values
    Color testColor2(0xFF112233);
    overlayMod->SetUnderlineColor(testColor2);
    EXPECT_EQ(overlayMod->underlineColor_->Get(), testColor2);

    float testWidth2 = 10.0f;
    overlayMod->SetUnderlineWidth(testWidth2);
    EXPECT_FLOAT_EQ(overlayMod->underlineWidth_->Get(), testWidth2);
}

// =================== OverlayModifier: PaintUnderline Tests ===================

/**
 * @tc.name: PaintUnderline001
 * @tc.desc: Test PaintUnderline returns early when not underline mode
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, PaintUnderline001, TestSize.Level1)
{
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);

    richEditorPattern->CreateRichEditorOverlayModifier();
    auto overlayMod = AceType::DynamicCast<RichEditorOverlayModifier>(richEditorPattern->overlayMod_);
    ASSERT_NE(overlayMod, nullptr);

    // Not underline mode -> PaintUnderline should return early without crash
    Testing::MockCanvas canvas;
    overlayMod->SetFrameSize(SizeF(300.0f, 100.0f));
    overlayMod->PaintUnderline(canvas);
    // No crash, no draw call
}

/**
 * @tc.name: PaintUnderline002
 * @tc.desc: Test PaintUnderline draws when underline mode is enabled
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, PaintUnderline002, TestSize.Level1)
{
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);

    RichEditorModelNG::SetShowUnderline(AceType::RawPtr(richEditorNode_), true);

    richEditorPattern->CreateRichEditorOverlayModifier();
    auto overlayMod = AceType::DynamicCast<RichEditorOverlayModifier>(richEditorPattern->overlayMod_);
    ASSERT_NE(overlayMod, nullptr);

    overlayMod->SetFrameSize(SizeF(300.0f, 100.0f));
    overlayMod->SetUnderlineColor(Color(0xFF007DFF));
    overlayMod->SetUnderlineWidth(2.0f);

    Testing::MockCanvas canvas;
    // Should draw without crash
    overlayMod->PaintUnderline(canvas);
}

/**
 * @tc.name: PaintUnderline003
 * @tc.desc: Test PaintUnderline with null pattern (CHECK_NULL_VOID)
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, PaintUnderline003, TestSize.Level1)
{
    // Create overlay modifier with null pattern
    WeakPtr<Pattern> nullPattern;
    WeakPtr<ScrollBarOverlayModifier> nullScrollBar;
    WeakPtr<ScrollEdgeEffect> nullEdge;
    auto overlayMod = AceType::MakeRefPtr<RichEditorOverlayModifier>(
        nullPattern, nullScrollBar, std::move(nullEdge));
    ASSERT_NE(overlayMod, nullptr);

    Testing::MockCanvas canvas;
    // Pattern is null -> should return early without crash
    overlayMod->SetFrameSize(SizeF(300.0f, 100.0f));
    overlayMod->PaintUnderline(canvas);
}

// =================== Theme Tests ===================

/**
 * @tc.name: RichEditorThemeUnderlineColors001
 * @tc.desc: Test RichEditorTheme underline color getters
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, RichEditorThemeUnderlineColors001, TestSize.Level1)
{
    ASSERT_NE(theme_, nullptr);

    // Verify default theme colors
    EXPECT_EQ(theme_->GetUnderlineColor(), Color(0x33182431));
    EXPECT_EQ(theme_->GetUnderlineTypingColor(), Color(0xFF007DFF));
    EXPECT_EQ(theme_->GetDisableUnderlineColor(), Color(0x33182431));
    EXPECT_EQ(theme_->GetErrorUnderlineColor(), Color(0xFF000000));
}

// =================== Integration: OnModifyDone / HandleFocus / HandleBlur ===================

/**
 * @tc.name: UpdateUnderlineStyleOnModifyDone001
 * @tc.desc: Test that UpdateUnderlineStyle is called during OnModifyDone
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, UpdateUnderlineStyleOnModifyDone001, TestSize.Level1)
{
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);

    RichEditorModelNG::SetShowUnderline(AceType::RawPtr(richEditorNode_), true);
    SetupEnabled(true);
    SetupFocus(false);

    // Reset to defaults before OnModifyDone
    richEditorPattern->underlineColor_ = Color::TRANSPARENT;
    richEditorPattern->underlineWidth_ = 0.0_vp;

    // OnModifyDone should call UpdateUnderlineStyle
    richEditorPattern->OnModifyDone();

    // After OnModifyDone, underline style should be updated
    EXPECT_EQ(richEditorPattern->underlineColor_, theme_->GetUnderlineColor());
    EXPECT_EQ(richEditorPattern->underlineWidth_, EXPECTED_NORMAL_WIDTH);
}

/**
 * @tc.name: UpdateUnderlineStyleOnFocusBlur001
 * @tc.desc: Test that UpdateUnderlineStyle is called during HandleFocusEvent and HandleBlurEvent
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, UpdateUnderlineStyleOnFocusBlur001, TestSize.Level1)
{
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);

    RichEditorModelNG::SetShowUnderline(AceType::RawPtr(richEditorNode_), true);
    SetupEnabled(true);

    // Focus -> typing color
    SetupFocus(true);
    richEditorPattern->HandleFocusEvent(FocusReason::DEFAULT);
    EXPECT_EQ(richEditorPattern->underlineColor_, theme_->GetUnderlineTypingColor());
    EXPECT_EQ(richEditorPattern->underlineWidth_, EXPECTED_TYPING_WIDTH);

    // Blur -> normal color
    SetupFocus(false);
    richEditorPattern->HandleBlurEvent();
    EXPECT_EQ(richEditorPattern->underlineColor_, theme_->GetUnderlineColor());
    EXPECT_EQ(richEditorPattern->underlineWidth_, EXPECTED_NORMAL_WIDTH);
}

/**
 * @tc.name: UpdateUnderlineStyleHandleEnabled001
 * @tc.desc: Test that UpdateUnderlineStyle is called during HandleEnabled
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, UpdateUnderlineStyleHandleEnabled001, TestSize.Level1)
{
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);

    RichEditorModelNG::SetShowUnderline(AceType::RawPtr(richEditorNode_), true);
    SetupEnabled(false);
    SetupFocus(false);

    // When disabled, color should be theme disable color
    richEditorPattern->HandleEnabled();
    EXPECT_EQ(richEditorPattern->underlineColor_, theme_->GetDisableUnderlineColor());

    // When enabled, color should be normal theme color
    SetupEnabled(true);
    richEditorPattern->HandleEnabled();
    EXPECT_EQ(richEditorPattern->underlineColor_, theme_->GetUnderlineColor());
}

// =================== PaintMethod: SetShowUnderlineWidth Direct Tests ===================

/**
 * @tc.name: SetShowUnderlineWidthDirect001
 * @tc.desc: Directly test RichEditorPaintMethod::SetShowUnderlineWidth with underline mode enabled
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, SetShowUnderlineWidthDirect001, TestSize.Level1)
{
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);

    RichEditorModelNG::SetShowUnderline(AceType::RawPtr(richEditorNode_), true);
    SetupEnabled(true);
    SetupFocus(true);
    richEditorPattern->UpdateUnderlineStyle();

    richEditorPattern->CreateRichEditorOverlayModifier();
    auto overlayMod = AceType::DynamicCast<RichEditorOverlayModifier>(richEditorPattern->GetOverlayModifier());
    ASSERT_NE(overlayMod, nullptr);

    // Create a content pattern and paint method from it
    auto contentPattern = AceType::MakeRefPtr<RichEditorContentPattern>(WeakClaim(richEditorPattern));
    ASSERT_NE(contentPattern, nullptr);
    auto paintMethod = contentPattern->CreateNodePaintMethod();
    ASSERT_NE(paintMethod, nullptr);
    auto richEditorPaintMethod = AceType::DynamicCast<RichEditorPaintMethod>(paintMethod);
    ASSERT_NE(richEditorPaintMethod, nullptr);

    // Call SetShowUnderlineWidth directly (nullptr paintWrapper is safe since
    // GetOverlayModifier returns stored textOverlayModifier_ and ignores the parameter)
    richEditorPaintMethod->SetShowUnderlineWidth(nullptr);

    // Verify the overlay modifier received the correct color and width
    EXPECT_EQ(overlayMod->underlineColor_->Get(), richEditorPattern->GetUnderlineColor());
    EXPECT_FLOAT_EQ(overlayMod->underlineWidth_->Get(),
        static_cast<float>(richEditorPattern->GetUnderlineWidth().ConvertToPx()));
}

/**
 * @tc.name: SetShowUnderlineWidthDirect002
 * @tc.desc: Directly test RichEditorPaintMethod::SetShowUnderlineWidth with underline mode disabled
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, SetShowUnderlineWidthDirect002, TestSize.Level1)
{
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);

    // Not underline mode
    EXPECT_FALSE(richEditorPattern->IsUnderlineMode());

    richEditorPattern->CreateRichEditorOverlayModifier();
    auto overlayMod = AceType::DynamicCast<RichEditorOverlayModifier>(richEditorPattern->GetOverlayModifier());
    ASSERT_NE(overlayMod, nullptr);

    // Set a non-zero width to verify it gets reset to 0
    overlayMod->SetUnderlineWidth(5.0f);
    EXPECT_FLOAT_EQ(overlayMod->underlineWidth_->Get(), 5.0f);

    // Create a content pattern and paint method from it
    auto contentPattern = AceType::MakeRefPtr<RichEditorContentPattern>(WeakClaim(richEditorPattern));
    ASSERT_NE(contentPattern, nullptr);
    auto paintMethod = contentPattern->CreateNodePaintMethod();
    ASSERT_NE(paintMethod, nullptr);
    auto richEditorPaintMethod = AceType::DynamicCast<RichEditorPaintMethod>(paintMethod);
    ASSERT_NE(richEditorPaintMethod, nullptr);

    // Call SetShowUnderlineWidth directly - should set width to 0 since not underline mode
    richEditorPaintMethod->SetShowUnderlineWidth(nullptr);

    // Verify the overlay modifier width was set to 0
    EXPECT_FLOAT_EQ(overlayMod->underlineWidth_->Get(), 0.0f);
}

// =================== IsUnderlineAndButtonMode B3 Branch Test ===================

/**
 * @tc.name: IsUnderlineAndButtonMode002
 * @tc.desc: Test IsUnderlineAndButtonMode returns true when both underline and button mode are active
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, IsUnderlineAndButtonMode002, TestSize.Level1)
{
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);

    auto* frameNode = AceType::RawPtr(richEditorNode_);

    // Set underline mode
    RichEditorModelNG::SetShowUnderline(frameNode, true);
    EXPECT_TRUE(richEditorPattern->IsUnderlineMode());

    // Set single line mode (required for IsShowCancelButtonMode to pass the single line check)
    RichEditorModelNG::SetSingleLine(frameNode, true);
    // SingleLine is true but IsShowCancelButton not set yet -> false
    EXPECT_FALSE(richEditorPattern->IsShowCancelButtonMode());

    // Without cancel button, still false
    EXPECT_FALSE(richEditorPattern->IsUnderlineAndButtonMode());

    // Set cancel button (sets IsShowCancelButton = true)
    RichEditorModelNG::SetCancelButton(frameNode, static_cast<int32_t>(CleanNodeStyle::CONSTANT),
        CalcDimension(16), Color(0xFF000000), "");
    EXPECT_TRUE(richEditorPattern->IsShowCancelButtonMode());

    // Now both underline and button mode are true
    EXPECT_TRUE(richEditorPattern->IsUnderlineAndButtonMode());
}

// =================== Dynamic Modifier Bridge: Null and Size Check Tests ===================

/**
 * @tc.name: BridgeShowUnderlineNullNode001
 * @tc.desc: Test bridge Set/Get/Reset RichEditorShowUnderline with null node (CHECK_NULL)
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, BridgeShowUnderlineNullNode001, TestSize.Level1)
{
    // Null node should not crash on any of the three operations
    SetRichEditorShowUnderline(nullptr, 1);
    ResetRichEditorShowUnderline(nullptr);
    auto result = GetRichEditorShowUnderline(nullptr);
    EXPECT_EQ(static_cast<uint32_t>(result), BRIDGE_ERROR_UINT_CODE);
}

/**
 * @tc.name: BridgeShowUnderlineValidNode001
 * @tc.desc: Test bridge Set/Get/Reset RichEditorShowUnderline with valid node
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, BridgeShowUnderlineValidNode001, TestSize.Level1)
{
    auto* frameNode = AceType::RawPtr(richEditorNode_);
    ASSERT_NE(frameNode, nullptr);
    auto nodeHandle = reinterpret_cast<ArkUINodeHandle>(frameNode);

    // Set true
    SetRichEditorShowUnderline(nodeHandle, 1);
    EXPECT_EQ(GetRichEditorShowUnderline(nodeHandle), 1);

    // Set false
    SetRichEditorShowUnderline(nodeHandle, 0);
    EXPECT_EQ(GetRichEditorShowUnderline(nodeHandle), 0);

    // Reset
    SetRichEditorShowUnderline(nodeHandle, 1);
    ResetRichEditorShowUnderline(nodeHandle);
    EXPECT_EQ(GetRichEditorShowUnderline(nodeHandle), 0);
}

/**
 * @tc.name: BridgeUnderlineColorNullNode001
 * @tc.desc: Test bridge SetRichEditorUnderlineColor with null node (CHECK_NULL_VOID)
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, BridgeUnderlineColorNullNode001, TestSize.Level1)
{
    ArkUI_Uint32 colors[4] = { 0xFF111111, 0xFF222222, 0xFF333333, 0xFF444444 };
    // Null node should not crash
    SetRichEditorUnderlineColor(nullptr, colors, 4);
    SUCCEED();
}

/**
 * @tc.name: BridgeUnderlineColorNullColors001
 * @tc.desc: Test bridge SetRichEditorUnderlineColor with null colors pointer (CHECK_NULL_VOID)
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, BridgeUnderlineColorNullColors001, TestSize.Level1)
{
    auto* frameNode = AceType::RawPtr(richEditorNode_);
    ASSERT_NE(frameNode, nullptr);
    auto nodeHandle = reinterpret_cast<ArkUINodeHandle>(frameNode);

    // Null colors should not crash
    SetRichEditorUnderlineColor(nodeHandle, nullptr, 4);
    SUCCEED();
}

/**
 * @tc.name: BridgeUnderlineColorSmallSize001
 * @tc.desc: Test bridge SetRichEditorUnderlineColor with size < 4 (early return)
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, BridgeUnderlineColorSmallSize001, TestSize.Level1)
{
    auto* frameNode = AceType::RawPtr(richEditorNode_);
    ASSERT_NE(frameNode, nullptr);
    auto nodeHandle = reinterpret_cast<ArkUINodeHandle>(frameNode);

    ArkUI_Uint32 colors[4] = { 0xFF111111, 0xFF222222, 0xFF333333, 0xFF444444 };

    // Set a known color first
    SetRichEditorUnderlineColor(nodeHandle, colors, 4);

    // Now call with size < 4 - should return early without changing colors
    ArkUI_Uint32 smallColors[3] = { 0xFFAAAAAAAA, 0xFFBBBBBBBB, 0xFFCCCCCCCC };
    SetRichEditorUnderlineColor(nodeHandle, smallColors, 3);

    // Verify colors were NOT changed by the small-size call
    ArkUI_Uint32 outSize = 0;
    auto* result = GetRichEditorUnderlineColor(nodeHandle, &outSize);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(outSize, 4);
    // typing color should still be the original value, not the small-size attempt
    EXPECT_EQ(result[0], 0xFF111111);
}

/**
 * @tc.name: BridgeUnderlineColorValid001
 * @tc.desc: Test bridge Set/Get/Reset RichEditorUnderlineColor with valid node and colors
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, BridgeUnderlineColorValid001, TestSize.Level1)
{
    auto* frameNode = AceType::RawPtr(richEditorNode_);
    ASSERT_NE(frameNode, nullptr);
    auto nodeHandle = reinterpret_cast<ArkUINodeHandle>(frameNode);

    // Set colors via bridge
    ArkUI_Uint32 colors[4] = { USER_TYPING_COLOR.GetValue(), USER_NORMAL_COLOR.GetValue(),
        USER_ERROR_COLOR.GetValue(), USER_DISABLE_COLOR.GetValue() };
    SetRichEditorUnderlineColor(nodeHandle, colors, 4);

    // Get colors via bridge
    ArkUI_Uint32 outSize = 0;
    auto* result = GetRichEditorUnderlineColor(nodeHandle, &outSize);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(outSize, 4);
    EXPECT_EQ(result[0], USER_TYPING_COLOR.GetValue());
    EXPECT_EQ(result[1], USER_NORMAL_COLOR.GetValue());
    EXPECT_EQ(result[2], USER_ERROR_COLOR.GetValue());
    EXPECT_EQ(result[3], USER_DISABLE_COLOR.GetValue());

    // Reset
    ResetRichEditorUnderlineColor(nodeHandle);
    result = GetRichEditorUnderlineColor(nodeHandle, &outSize);
    ASSERT_NE(result, nullptr);
    // After reset, all colors should be 0 (Color() default)
    EXPECT_EQ(result[0], Color().GetValue());
    EXPECT_EQ(result[1], Color().GetValue());
    EXPECT_EQ(result[2], Color().GetValue());
    EXPECT_EQ(result[3], Color().GetValue());
}

/**
 * @tc.name: BridgeGetUnderlineColorNullNode001
 * @tc.desc: Test bridge GetRichEditorUnderlineColor with null node (CHECK_NULL_RETURN)
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, BridgeGetUnderlineColorNullNode001, TestSize.Level1)
{
    auto* result = GetRichEditorUnderlineColor(nullptr, nullptr);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: BridgeGetUnderlineColorNullSize001
 * @tc.desc: Test bridge GetRichEditorUnderlineColor with null size pointer (CHECK_NULL_RETURN)
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, BridgeGetUnderlineColorNullSize001, TestSize.Level1)
{
    auto* frameNode = AceType::RawPtr(richEditorNode_);
    ASSERT_NE(frameNode, nullptr);
    auto nodeHandle = reinterpret_cast<ArkUINodeHandle>(frameNode);

    auto* result = GetRichEditorUnderlineColor(nodeHandle, nullptr);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: BridgeResetUnderlineColorNullNode001
 * @tc.desc: Test bridge ResetRichEditorUnderlineColor with null node (CHECK_NULL_VOID)
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, BridgeResetUnderlineColorNullNode001, TestSize.Level1)
{
    // Null node should not crash
    ResetRichEditorUnderlineColor(nullptr);
    SUCCEED();
}
} // namespace OHOS::Ace::NG

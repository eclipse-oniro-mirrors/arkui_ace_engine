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
#include "core/components_ng/pattern/rich_editor/napi_rich_editor_pattern.h"
#include "test/mock/frameworks/core/rosen/mock_canvas.h"
#include "test/mock/frameworks/core/components_ng/render/mock_paragraph.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"
#include "test/mock/frameworks/core/common/mock_container.h"
#include "test/mock/frameworks/core/common/mock_theme_manager.h"
#include "test/mock/frameworks/core/common/mock_resource_adapter_v2.h"
#include "test/mock/frameworks/base/thread/mock_task_executor.h"
#include "core/components_ng/pattern/text_field/text_field_model.h"
#include "core/components/text_field/textfield_theme.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {
constexpr Dimension EXPECTED_TYPING_WIDTH = 2.0_px;
constexpr Dimension EXPECTED_NORMAL_WIDTH = 1.0_px;
constexpr Dimension EXPECTED_ERROR_WIDTH = 2.0_px;
const Color USER_TYPING_COLOR = Color(0xFF111111);
const Color USER_NORMAL_COLOR = Color(0xFF222222);
const Color USER_DISABLE_COLOR = Color(0xFF333333);
const Color USER_ERROR_COLOR = Color(0xFF444444);
constexpr int32_t TEST_MAX_LENGTH = 10;
constexpr float INITIAL_BOUNDS_HEIGHT = 100.0f;
constexpr float INITIAL_BOUNDS_WIDTH = 200.0f;
} // namespace

// Bridge function implementations for dynamic modifier (linked from rich_editor_dynamic_modifier.cpp)
struct _ArkUINode;
typedef _ArkUINode* ArkUINodeHandle;
typedef int ArkUI_Int32;
typedef unsigned int ArkUI_Uint32;
constexpr uint32_t BRIDGE_ERROR_UINT_CODE = static_cast<uint32_t>(-1);
void SetRichEditorShowUnderline(ArkUINodeHandle node, ArkUI_Int32 showUnderline)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    RichEditorModelNG::SetShowUnderline(frameNode, static_cast<bool>(showUnderline));
}
ArkUI_Int32 GetRichEditorShowUnderline(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_RETURN(frameNode, static_cast<ArkUI_Int32>(BRIDGE_ERROR_UINT_CODE));
    return static_cast<ArkUI_Int32>(RichEditorModelNG::GetShowUnderline(frameNode));
}
void ResetRichEditorShowUnderline(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    RichEditorModelNG::ResetShowUnderline(frameNode);
}
void SetRichEditorUnderlineColor(ArkUINodeHandle node, const ArkUI_Uint32* colors, ArkUI_Uint32 size)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(colors);
    if (size < 4) {
        return;
    }
    UserUnderlineColor userColor {
        Color(colors[0]),
        Color(colors[1]),
        Color(colors[2]),
        Color(colors[3]),
    };
    RichEditorModelNG::SetUnderlineColor(frameNode, userColor);
}
const ArkUI_Uint32* GetRichEditorUnderlineColor(ArkUINodeHandle node, ArkUI_Uint32* size)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_RETURN(frameNode, nullptr);
    CHECK_NULL_RETURN(size, nullptr);
    auto userColor = RichEditorModelNG::GetUnderlineColor(frameNode);
    static ArkUI_Uint32 underlineColors[4] = { 0 };
    underlineColors[0] = userColor.typing.value_or(Color()).GetValue();
    underlineColors[1] = userColor.normal.value_or(Color()).GetValue();
    underlineColors[2] = userColor.error.value_or(Color()).GetValue();
    underlineColors[3] = userColor.disable.value_or(Color()).GetValue();
    *size = 4;
    return underlineColors;
}
void ResetRichEditorUnderlineColor(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    RichEditorModelNG::ResetUnderlineColor(frameNode);
}

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
    RefPtr<FrameNode> CreateNapiNode();
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
    auto textFieldTheme = AceType::MakeRefPtr<TextFieldTheme>();
    textFieldTheme->underlineTypingColor_ = theme_->GetUnderlineTypingColor();
    textFieldTheme->underlineColor_ = theme_->GetUnderlineColor();
    textFieldTheme->disableUnderlineColor_ = theme_->GetDisableUnderlineColor();
    textFieldTheme->errorUnderlineColor_ = theme_->GetErrorUnderlineColor();
    themeManager_ = AceType::MakeRefPtr<MockThemeManager>();
    EXPECT_CALL(*themeManager_, GetTheme(_))
        .WillRepeatedly([textFieldTheme, richEditorTheme = theme_](ThemeType type) -> RefPtr<Theme> {
            if (type == TextFieldTheme::TypeId()) {
                return textFieldTheme;
            }
            return richEditorTheme;
        });
    EXPECT_CALL(*themeManager_, GetTheme(_, _))
        .WillRepeatedly([textFieldTheme, richEditorTheme = theme_](ThemeType type, int32_t) -> RefPtr<Theme> {
            if (type == TextFieldTheme::TypeId()) {
                return textFieldTheme;
            }
            return richEditorTheme;
        });
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
        focusHub->LostFocus();
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

RefPtr<FrameNode> RichEditorUnderlineTestNg::CreateNapiNode()
{
    auto* stack = ViewStackProcessor::GetInstance();
    auto nodeId = stack->ClaimNodeId();
    auto node = FrameNode::GetOrCreateFrameNode(
        V2::RICH_EDITOR_ETS_TAG, nodeId,
        []() { return AceType::MakeRefPtr<NapiRichEditorPattern>(); });
    auto pattern = node->GetPattern<RichEditorPattern>();
    pattern->InitScrollablePattern();
    pattern->SetRichEditorController(AceType::MakeRefPtr<RichEditorController>());
    pattern->GetRichEditorController()->SetPattern(AceType::WeakClaim(AceType::RawPtr(pattern)));
    pattern->CreateNodePaintMethod();
    node->GetGeometryNode()->SetContentSize({ 300.0f, 300.0f });
    return node;
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

    EXPECT_FALSE(RichEditorModelNG::GetShowUnderline(frameNode));
    RichEditorModelNG::SetShowUnderline(frameNode, true);
    EXPECT_TRUE(RichEditorModelNG::GetShowUnderline(frameNode));
    RichEditorModelNG::SetShowUnderline(frameNode, false);
    EXPECT_FALSE(RichEditorModelNG::GetShowUnderline(frameNode));
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

    auto defaultColor = RichEditorModelNG::GetUnderlineColor(frameNode);
    EXPECT_FALSE(defaultColor.typing.has_value());
    EXPECT_FALSE(defaultColor.normal.has_value());
    EXPECT_FALSE(defaultColor.error.has_value());
    EXPECT_FALSE(defaultColor.disable.has_value());

    UserUnderlineColor userColor;
    userColor.typing = USER_TYPING_COLOR;
    userColor.normal = USER_NORMAL_COLOR;
    userColor.error = USER_ERROR_COLOR;
    userColor.disable = USER_DISABLE_COLOR;
    RichEditorModelNG::SetUnderlineColor(frameNode, userColor);

    auto got = RichEditorModelNG::GetUnderlineColor(frameNode);
    EXPECT_EQ(got.typing.value(), USER_TYPING_COLOR);
    EXPECT_EQ(got.normal.value(), USER_NORMAL_COLOR);
    EXPECT_EQ(got.error.value(), USER_ERROR_COLOR);
    EXPECT_EQ(got.disable.value(), USER_DISABLE_COLOR);

    RichEditorModelNG::ResetUnderlineColor(frameNode);
    auto resetColor = RichEditorModelNG::GetUnderlineColor(frameNode);
    EXPECT_FALSE(resetColor.typing.has_value());
    EXPECT_FALSE(resetColor.normal.has_value());
    EXPECT_FALSE(resetColor.error.has_value());
    EXPECT_FALSE(resetColor.disable.has_value());
}

// =================== Pattern: IsUnderlineMode / IsUnderlineAndButtonMode ===================

/**
 * @tc.name: IsUnderlineMode001
 * @tc.desc: Test IsUnderlineMode and IsUnderlineAndButtonMode for all combinations
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, IsUnderlineMode001, TestSize.Level1)
{
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto* frameNode = AceType::RawPtr(richEditorNode_);

    // Default: not underline mode
    EXPECT_FALSE(richEditorPattern->IsUnderlineMode());
    EXPECT_FALSE(richEditorPattern->IsUnderlineAndButtonMode());

    // Underline only, no button
    RichEditorModelNG::SetShowUnderline(frameNode, true);
    EXPECT_TRUE(richEditorPattern->IsUnderlineMode());
    EXPECT_FALSE(richEditorPattern->IsUnderlineAndButtonMode());

    // Underline + button (requires SingleLine + CancelButton)
    RichEditorModelNG::SetSingleLine(frameNode, true);
    EXPECT_FALSE(richEditorPattern->IsShowCancelButtonMode());
    RichEditorModelNG::SetCancelButton(frameNode, static_cast<int32_t>(CleanNodeStyle::CONSTANT),
        CalcDimension(16), Color(0xFF000000), "");
    EXPECT_TRUE(richEditorPattern->IsShowCancelButtonMode());
    EXPECT_TRUE(richEditorPattern->IsUnderlineAndButtonMode());

    // Turn off underline
    RichEditorModelNG::SetShowUnderline(frameNode, false);
    EXPECT_FALSE(richEditorPattern->IsUnderlineMode());
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

    richEditorPattern->underlineColor_ = Color::TRANSPARENT;
    richEditorPattern->underlineWidth_ = 0.0_vp;
    richEditorPattern->UpdateUnderlineStyle();
    EXPECT_EQ(richEditorPattern->underlineColor_, Color::TRANSPARENT);
    EXPECT_EQ(richEditorPattern->underlineWidth_, 0.0_vp);
}

/**
 * @tc.name: UpdateUnderlineStyle002
 * @tc.desc: Test UpdateUnderlineStyle focus/no-focus with and without user colors
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, UpdateUnderlineStyle002, TestSize.Level1)
{
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    RichEditorModelNG::SetShowUnderline(AceType::RawPtr(richEditorNode_), true);
    SetupEnabled(true);

    // Focus, no user color -> theme typing
    SetupFocus(true);
    richEditorPattern->UpdateUnderlineStyle();
    EXPECT_EQ(richEditorPattern->underlineColor_, theme_->GetUnderlineTypingColor());
    EXPECT_EQ(richEditorPattern->underlineWidth_, EXPECTED_TYPING_WIDTH);

    // No focus, no user color -> theme normal
    SetupFocus(false);
    richEditorPattern->UpdateUnderlineStyle();
    EXPECT_EQ(richEditorPattern->underlineColor_, theme_->GetUnderlineColor());
    EXPECT_EQ(richEditorPattern->underlineWidth_, EXPECTED_NORMAL_WIDTH);

    // Focus, user typing color
    UserUnderlineColor userColor;
    userColor.typing = USER_TYPING_COLOR;
    userColor.normal = USER_NORMAL_COLOR;
    RichEditorModelNG::SetUnderlineColor(AceType::RawPtr(richEditorNode_), userColor);
    SetupFocus(true);
    richEditorPattern->UpdateUnderlineStyle();
    EXPECT_EQ(richEditorPattern->underlineColor_, USER_TYPING_COLOR);
    EXPECT_EQ(richEditorPattern->underlineWidth_, EXPECTED_TYPING_WIDTH);

    // No focus, user normal color
    SetupFocus(false);
    richEditorPattern->UpdateUnderlineStyle();
    EXPECT_EQ(richEditorPattern->underlineColor_, USER_NORMAL_COLOR);
    EXPECT_EQ(richEditorPattern->underlineWidth_, EXPECTED_NORMAL_WIDTH);
}

/**
 * @tc.name: UpdateUnderlineStyle003
 * @tc.desc: Test UpdateUnderlineStyle disable variations (user disable color / theme disable / not disabled)
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, UpdateUnderlineStyle003, TestSize.Level1)
{
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    RichEditorModelNG::SetShowUnderline(AceType::RawPtr(richEditorNode_), true);
    SetupFocus(false);

    // Disabled, no user disable color -> theme disable color
    SetupEnabled(false);
    richEditorPattern->UpdateUnderlineStyle();
    EXPECT_EQ(richEditorPattern->underlineColor_, theme_->GetDisableUnderlineColor());

    // Disabled, user disable color -> user disable color
    UserUnderlineColor userColor;
    userColor.disable = USER_DISABLE_COLOR;
    RichEditorModelNG::SetUnderlineColor(AceType::RawPtr(richEditorNode_), userColor);
    richEditorPattern->UpdateUnderlineStyle();
    EXPECT_EQ(richEditorPattern->underlineColor_, USER_DISABLE_COLOR);

    // Not disabled, user disable color set -> focus state color (normal)
    userColor.normal = USER_NORMAL_COLOR;
    RichEditorModelNG::SetUnderlineColor(AceType::RawPtr(richEditorNode_), userColor);
    SetupEnabled(true);
    richEditorPattern->UpdateUnderlineStyle();
    EXPECT_EQ(richEditorPattern->underlineColor_, USER_NORMAL_COLOR);
}

// =================== Pattern: GetUnderlineColor/Width + UpdateUnderlineErrorStyle ===================

/**
 * @tc.name: GetUnderlineColorWidth001
 * @tc.desc: Test GetUnderlineColor/Width defaults and after style update
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, GetUnderlineColorWidth001, TestSize.Level1)
{
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);

    EXPECT_EQ(richEditorPattern->GetUnderlineColor(), Color::TRANSPARENT);
    EXPECT_EQ(richEditorPattern->GetUnderlineWidth(), 0.0_vp);

    RichEditorModelNG::SetShowUnderline(AceType::RawPtr(richEditorNode_), true);
    SetupEnabled(true);
    SetupFocus(true);
    richEditorPattern->UpdateUnderlineStyle();
    EXPECT_EQ(richEditorPattern->GetUnderlineColor(), theme_->GetUnderlineTypingColor());
    EXPECT_EQ(richEditorPattern->GetUnderlineWidth(), EXPECTED_TYPING_WIDTH);
}

/**
 * @tc.name: UpdateUnderlineErrorStyle001
 * @tc.desc: Test UpdateUnderlineErrorStyle: early return, user error color, theme error color
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, UpdateUnderlineErrorStyle001, TestSize.Level1)
{
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);

    // Not underline mode -> no change
    richEditorPattern->underlineColor_ = Color::TRANSPARENT;
    richEditorPattern->underlineWidth_ = 0.0_vp;
    richEditorPattern->UpdateUnderlineErrorStyle();
    EXPECT_EQ(richEditorPattern->underlineColor_, Color::TRANSPARENT);
    EXPECT_EQ(richEditorPattern->underlineWidth_, 0.0_vp);

    // Underline mode, user error color
    RichEditorModelNG::SetShowUnderline(AceType::RawPtr(richEditorNode_), true);
    UserUnderlineColor userColor;
    userColor.error = USER_ERROR_COLOR;
    RichEditorModelNG::SetUnderlineColor(AceType::RawPtr(richEditorNode_), userColor);
    richEditorPattern->UpdateUnderlineErrorStyle();
    EXPECT_EQ(richEditorPattern->underlineColor_, USER_ERROR_COLOR);
    EXPECT_EQ(richEditorPattern->underlineWidth_, EXPECTED_ERROR_WIDTH);

    // Underline mode, no user error color -> theme error color
    RichEditorModelNG::ResetUnderlineColor(AceType::RawPtr(richEditorNode_));
    richEditorPattern->UpdateUnderlineErrorStyle();
    EXPECT_EQ(richEditorPattern->underlineColor_, theme_->GetErrorUnderlineColor());
    EXPECT_EQ(richEditorPattern->underlineWidth_, EXPECTED_ERROR_WIDTH);
}

// =================== OverlayModifier: Set/Get + PaintUnderline ===================

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

    Color testColor(0xFFAABBCC);
    overlayMod->SetUnderlineColor(testColor);
    EXPECT_EQ(overlayMod->underlineColor_->Get(), testColor);

    float testWidth = 5.0f;
    overlayMod->SetUnderlineWidth(testWidth);
    EXPECT_FLOAT_EQ(overlayMod->underlineWidth_->Get(), testWidth);

    overlayMod->SetUnderlineColor(Color(0xFF112233));
    EXPECT_EQ(overlayMod->underlineColor_->Get(), Color(0xFF112233));
    overlayMod->SetUnderlineWidth(10.0f);
    EXPECT_FLOAT_EQ(overlayMod->underlineWidth_->Get(), 10.0f);
}

/**
 * @tc.name: PaintUnderline001
 * @tc.desc: Test PaintUnderline early-return paths: null pattern, !IsUnderlineMode, !isContentNode_
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, PaintUnderline001, TestSize.Level1)
{
    Testing::MockCanvas canvas;

    // Null pattern -> return early
    WeakPtr<Pattern> nullPattern;
    WeakPtr<ScrollBarOverlayModifier> nullScrollBar;
    WeakPtr<ScrollEdgeEffect> nullEdge;
    auto nullOverlayMod = AceType::MakeRefPtr<RichEditorOverlayModifier>(
        nullPattern, nullScrollBar, std::move(nullEdge));
    ASSERT_NE(nullOverlayMod, nullptr);
    nullOverlayMod->SetFrameSize(SizeF(300.0f, 100.0f));
    nullOverlayMod->PaintUnderline(canvas);

    // Valid pattern, not underline mode -> return early
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    richEditorPattern->CreateRichEditorOverlayModifier();
    auto overlayMod = AceType::DynamicCast<RichEditorOverlayModifier>(richEditorPattern->overlayMod_);
    ASSERT_NE(overlayMod, nullptr);
    overlayMod->SetFrameSize(SizeF(300.0f, 100.0f));
    overlayMod->PaintUnderline(canvas);

    // Underline mode but isContentNode_ = false -> return early
    RichEditorModelNG::SetShowUnderline(AceType::RawPtr(richEditorNode_), true);
    WeakPtr<Pattern> pattern = AceType::WeakClaim(AceType::RawPtr(richEditorPattern));
    auto nonContentOverlay = AceType::MakeRefPtr<RichEditorOverlayModifier>(
        pattern, nullScrollBar, std::move(nullEdge), false);
    ASSERT_NE(nonContentOverlay, nullptr);
    nonContentOverlay->SetFrameSize(SizeF(300.0f, 100.0f));
    nonContentOverlay->SetUnderlineColor(Color(0xFF007DFF));
    nonContentOverlay->SetUnderlineWidth(2.0f);
    nonContentOverlay->PaintUnderline(canvas);
}

/**
 * @tc.name: PaintUnderline002
 * @tc.desc: Test PaintUnderline drawing: LTR/RTL x isNeedToAddPadding
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, PaintUnderline002, TestSize.Level1)
{
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto* frameNode = AceType::RawPtr(richEditorNode_);

    auto setupAndDraw = [&](bool rtl, bool withButton) {
        RichEditorModelNG::SetShowUnderline(frameNode, true);
        if (withButton) {
            RichEditorModelNG::SetSingleLine(frameNode, true);
            RichEditorModelNG::SetCancelButton(frameNode, static_cast<int32_t>(CleanNodeStyle::CONSTANT),
                CalcDimension(16), Color(0xFF000000), "");
        }
        auto layoutProperty = richEditorNode_->GetLayoutProperty<RichEditorLayoutProperty>();
        layoutProperty->UpdateLayoutDirection(rtl ? TextDirection::RTL : TextDirection::LTR);

        richEditorPattern->CreateRichEditorOverlayModifier();
        auto overlayMod = AceType::DynamicCast<RichEditorOverlayModifier>(richEditorPattern->overlayMod_);
        overlayMod->SetFrameSize(SizeF(300.0f, 100.0f));
        overlayMod->SetUnderlineColor(Color(0xFF007DFF));
        overlayMod->SetUnderlineWidth(2.0f);

        Testing::MockCanvas canvas;
        EXPECT_CALL(canvas, AttachPen(_)).WillRepeatedly(ReturnRef(canvas));
        EXPECT_CALL(canvas, DetachPen()).WillRepeatedly(ReturnRef(canvas));
        EXPECT_CALL(canvas, DrawLine(_, _)).Times(1);
        overlayMod->PaintUnderline(canvas);
    };

    // LTR, no padding
    setupAndDraw(false, false);
    // LTR, with padding (isNeedToAddPadding=true)
    setupAndDraw(false, true);
    // RTL, no padding
    setupAndDraw(true, false);
    // RTL, with padding
    setupAndDraw(true, true);
}

// =================== Theme + Integration Tests ===================

/**
 * @tc.name: RichEditorThemeUnderlineColors001
 * @tc.desc: Test RichEditorTheme underline color getters
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, RichEditorThemeUnderlineColors001, TestSize.Level1)
{
    ASSERT_NE(theme_, nullptr);
    EXPECT_EQ(theme_->GetUnderlineColor(), Color(0x33182431));
    EXPECT_EQ(theme_->GetUnderlineTypingColor(), Color(0xFF007DFF));
    EXPECT_EQ(theme_->GetDisableUnderlineColor(), Color(0x33182431));
    EXPECT_EQ(theme_->GetErrorUnderlineColor(), Color(0xFF000000));
}

/**
 * @tc.name: UpdateUnderlineStyleIntegration001
 * @tc.desc: Test UpdateUnderlineStyle is called during OnModifyDone, HandleFocus/Blur, HandleEnabled
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, UpdateUnderlineStyleIntegration001, TestSize.Level1)
{
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    RichEditorModelNG::SetShowUnderline(AceType::RawPtr(richEditorNode_), true);
    SetupEnabled(true);

    // OnModifyDone -> normal color (no focus)
    SetupFocus(false);
    richEditorPattern->underlineColor_ = Color::TRANSPARENT;
    richEditorPattern->underlineWidth_ = 0.0_vp;
    richEditorPattern->OnModifyDone();
    EXPECT_EQ(richEditorPattern->underlineColor_, theme_->GetUnderlineColor());
    EXPECT_EQ(richEditorPattern->underlineWidth_, EXPECTED_NORMAL_WIDTH);

    // HandleFocusEvent -> typing color
    SetupFocus(true);
    richEditorPattern->HandleFocusEvent(FocusReason::DEFAULT);
    EXPECT_EQ(richEditorPattern->underlineColor_, theme_->GetUnderlineTypingColor());
    EXPECT_EQ(richEditorPattern->underlineWidth_, EXPECTED_TYPING_WIDTH);

    // HandleBlurEvent -> normal color
    SetupFocus(false);
    richEditorPattern->HandleBlurEvent();
    EXPECT_EQ(richEditorPattern->underlineColor_, theme_->GetUnderlineColor());
    EXPECT_EQ(richEditorPattern->underlineWidth_, EXPECTED_NORMAL_WIDTH);

    // HandleEnabled disabled -> disable color
    SetupEnabled(false);
    richEditorPattern->HandleEnabled();
    EXPECT_EQ(richEditorPattern->underlineColor_, theme_->GetDisableUnderlineColor());

    // HandleEnabled enabled -> normal color
    SetupEnabled(true);
    richEditorPattern->HandleEnabled();
    EXPECT_EQ(richEditorPattern->underlineColor_, theme_->GetUnderlineColor());
}

// =================== PaintMethod: SetShowUnderlineWidth + AdjustBoundsRect ===================

/**
 * @tc.name: SetShowUnderlineWidth001
 * @tc.desc: Test SetShowUnderlineWidth with underline mode on/off
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, SetShowUnderlineWidth001, TestSize.Level1)
{
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);

    richEditorPattern->CreateRichEditorOverlayModifier();
    auto overlayMod = AceType::DynamicCast<RichEditorOverlayModifier>(richEditorPattern->GetOverlayModifier());
    ASSERT_NE(overlayMod, nullptr);

    auto contentPattern = AceType::MakeRefPtr<RichEditorContentPattern>(AceType::WeakClaim(AceType::RawPtr(richEditorPattern)));
    auto paintMethod = contentPattern->CreateNodePaintMethod();
    auto richEditorPaintMethod = AceType::DynamicCast<RichEditorPaintMethod>(paintMethod);
    ASSERT_NE(richEditorPaintMethod, nullptr);

    // Not underline mode -> width set to 0
    overlayMod->SetUnderlineWidth(5.0f);
    richEditorPaintMethod->SetShowUnderlineWidth(nullptr);
    EXPECT_FLOAT_EQ(overlayMod->underlineWidth_->Get(), 0.0f);

    // Underline mode -> color and width set from pattern
    RichEditorModelNG::SetShowUnderline(AceType::RawPtr(richEditorNode_), true);
    SetupEnabled(true);
    SetupFocus(true);
    richEditorPattern->UpdateUnderlineStyle();
    richEditorPaintMethod->SetShowUnderlineWidth(nullptr);
    EXPECT_EQ(overlayMod->underlineColor_->Get(), richEditorPattern->GetUnderlineColor());
    EXPECT_FLOAT_EQ(overlayMod->underlineWidth_->Get(),
        static_cast<float>(richEditorPattern->GetUnderlineWidth().ConvertToPx()));
}

/**
 * @tc.name: AdjustBoundsRect001
 * @tc.desc: Test AdjustBoundsRect: underline mode, counter enabled, neither
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, AdjustBoundsRect001, TestSize.Level1)
{
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);

    // Neither counter nor underline -> no change
    EXPECT_FALSE(richEditorPattern->IsUnderlineMode());
    EXPECT_FALSE(richEditorPattern->IsShowCounterEnabled());
    RectF bounds(0.0f, 0.0f, INITIAL_BOUNDS_WIDTH, INITIAL_BOUNDS_HEIGHT);
    richEditorPattern->AdjustBoundsRect(bounds);
    EXPECT_FLOAT_EQ(bounds.Height(), INITIAL_BOUNDS_HEIGHT);

    // IsUnderlineMode -> height increases by underlineWidth
    RichEditorModelNG::SetShowUnderline(AceType::RawPtr(richEditorNode_), true);
    richEditorPattern->underlineWidth_ = EXPECTED_NORMAL_WIDTH;
    RectF bounds2(0.0f, 0.0f, INITIAL_BOUNDS_WIDTH, INITIAL_BOUNDS_HEIGHT);
    richEditorPattern->AdjustBoundsRect(bounds2);
    float expectedHeight = INITIAL_BOUNDS_HEIGHT +
        static_cast<float>(EXPECTED_NORMAL_WIDTH.ConvertToPx());
    EXPECT_FLOAT_EQ(bounds2.Height(), expectedHeight);

    // IsShowCounterEnabled (counterDecorator_ null -> countHeight 0)
    richEditorPattern->isSpanStringMode_ = true;
    richEditorPattern->maxLength_ = TEST_MAX_LENGTH;
    richEditorPattern->styledString_ = AceType::MakeRefPtr<MutableSpanString>(u"hello");
    auto layoutProperty = richEditorNode_->GetLayoutProperty<RichEditorLayoutProperty>();
    layoutProperty->UpdateShowCounter(true);
    EXPECT_TRUE(richEditorPattern->IsShowCounterEnabled());
    RectF bounds3(0.0f, 0.0f, INITIAL_BOUNDS_WIDTH, INITIAL_BOUNDS_HEIGHT);
    richEditorPattern->AdjustBoundsRect(bounds3);
    EXPECT_FLOAT_EQ(bounds3.Height(), INITIAL_BOUNDS_HEIGHT);
}

// =================== GetAllResponseAreaWidth ===================

/**
 * @tc.name: GetAllResponseAreaWidth001
 * @tc.desc: Test GetAllResponseAreaWidth returns 0 when no response areas
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, GetAllResponseAreaWidth001, TestSize.Level1)
{
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    EXPECT_FLOAT_EQ(richEditorPattern->GetAllResponseAreaWidth(), 0.0f);
}

// =================== HandleCountStyle + HandleCounterBorder with Underline ===================

/**
 * @tc.name: HandleCountStyleUnderline001
 * @tc.desc: Test HandleCountStyle with underline mode: showCountBorderStyle_ true/false
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, HandleCountStyleUnderline001, TestSize.Level1)
{
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    RichEditorModelNG::SetShowUnderline(AceType::RawPtr(richEditorNode_), true);
    SetupEnabled(true);
    SetupFocus(false);
    EXPECT_FALSE(richEditorPattern->IsShowCounterEnabled());

    // showCountBorderStyle_ true -> UpdateUnderlineErrorStyle
    richEditorPattern->showCountBorderStyle_ = true;
    richEditorPattern->underlineColor_ = Color::TRANSPARENT;
    richEditorPattern->underlineWidth_ = 0.0_vp;
    richEditorPattern->HandleCountStyle();
    EXPECT_EQ(richEditorPattern->underlineColor_, theme_->GetErrorUnderlineColor());
    EXPECT_EQ(richEditorPattern->underlineWidth_, EXPECTED_ERROR_WIDTH);

    // showCountBorderStyle_ false -> UpdateUnderlineStyle (normal)
    richEditorPattern->showCountBorderStyle_ = false;
    richEditorPattern->underlineColor_ = Color::TRANSPARENT;
    richEditorPattern->underlineWidth_ = 0.0_vp;
    richEditorPattern->HandleCountStyle();
    EXPECT_EQ(richEditorPattern->underlineColor_, theme_->GetUnderlineColor());
    EXPECT_EQ(richEditorPattern->underlineWidth_, EXPECTED_NORMAL_WIDTH);
}

/**
 * @tc.name: HandleCounterBorderUnderline001
 * @tc.desc: Test HandleCounterBorder all 4 combos: showCountBorderStyle_ x IsUnderlineMode
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, HandleCounterBorderUnderline001, TestSize.Level1)
{
    auto node = CreateNapiNode();
    ASSERT_NE(node, nullptr);
    auto pattern = node->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    EXPECT_TRUE(pattern->IsNapiRichEditorPattern());

    // 1) showCountBorderStyle_=true + IsUnderlineMode -> UpdateUnderlineErrorStyle
    RichEditorModelNG::SetShowUnderline(AceType::RawPtr(node), true);
    pattern->showCountBorderStyle_ = true;
    pattern->underlineColor_ = Color::TRANSPARENT;
    pattern->underlineWidth_ = 0.0_vp;
    pattern->HandleCounterBorder();
    EXPECT_EQ(pattern->underlineColor_, theme_->GetErrorUnderlineColor());
    EXPECT_EQ(pattern->underlineWidth_, EXPECTED_ERROR_WIDTH);

    // 2) showCountBorderStyle_=false + IsUnderlineMode -> UpdateUnderlineStyle
    pattern->showCountBorderStyle_ = false;
    pattern->underlineColor_ = Color::TRANSPARENT;
    pattern->underlineWidth_ = 0.0_vp;
    pattern->HandleCounterBorder();
    EXPECT_EQ(pattern->underlineColor_, theme_->GetUnderlineColor());
    EXPECT_EQ(pattern->underlineWidth_, EXPECTED_NORMAL_WIDTH);

    // 3) showCountBorderStyle_=true + !IsUnderlineMode -> ApplyInnerBorderColor (no crash)
    RichEditorModelNG::SetShowUnderline(AceType::RawPtr(node), false);
    pattern->showCountBorderStyle_ = true;
    pattern->HandleCounterBorder();

    // 4) showCountBorderStyle_=false + !IsUnderlineMode -> SetThemeBorderAttr (no crash)
    pattern->showCountBorderStyle_ = false;
    pattern->HandleCounterBorder();
}

// =================== Dynamic Modifier Bridge Tests ===================

/**
 * @tc.name: BridgeShowUnderline001
 * @tc.desc: Test bridge Set/Get/Reset RichEditorShowUnderline with null and valid nodes
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, BridgeShowUnderline001, TestSize.Level1)
{
    // Null node
    SetRichEditorShowUnderline(nullptr, 1);
    ResetRichEditorShowUnderline(nullptr);
    EXPECT_EQ(static_cast<uint32_t>(GetRichEditorShowUnderline(nullptr)), BRIDGE_ERROR_UINT_CODE);

    // Valid node
    auto* frameNode = AceType::RawPtr(richEditorNode_);
    auto nodeHandle = reinterpret_cast<ArkUINodeHandle>(frameNode);
    SetRichEditorShowUnderline(nodeHandle, 1);
    EXPECT_EQ(GetRichEditorShowUnderline(nodeHandle), 1);
    SetRichEditorShowUnderline(nodeHandle, 0);
    EXPECT_EQ(GetRichEditorShowUnderline(nodeHandle), 0);
    SetRichEditorShowUnderline(nodeHandle, 1);
    ResetRichEditorShowUnderline(nodeHandle);
    EXPECT_EQ(GetRichEditorShowUnderline(nodeHandle), 0);
}

/**
 * @tc.name: BridgeUnderlineColor001
 * @tc.desc: Test bridge UnderlineColor error cases: null node, null colors, small size
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, BridgeUnderlineColor001, TestSize.Level1)
{
    ArkUI_Uint32 colors[4] = { 0xFF111111, 0xFF222222, 0xFF333333, 0xFF444444 };

    // Null node
    SetRichEditorUnderlineColor(nullptr, colors, 4);
    ResetRichEditorUnderlineColor(nullptr);
    EXPECT_EQ(GetRichEditorUnderlineColor(nullptr, nullptr), nullptr);

    auto* frameNode = AceType::RawPtr(richEditorNode_);
    auto nodeHandle = reinterpret_cast<ArkUINodeHandle>(frameNode);

    // Null colors
    SetRichEditorUnderlineColor(nodeHandle, nullptr, 4);

    // Null size pointer for Get
    EXPECT_EQ(GetRichEditorUnderlineColor(nodeHandle, nullptr), nullptr);

    // Small size (< 4) -> should not change existing colors
    SetRichEditorUnderlineColor(nodeHandle, colors, 4);
    ArkUI_Uint32 smallColors[3] = { 0xAAAAAAAA, 0xBBBBBBBB, 0xCCCCCCCC };
    SetRichEditorUnderlineColor(nodeHandle, smallColors, 3);
    ArkUI_Uint32 outSize = 0;
    auto* result = GetRichEditorUnderlineColor(nodeHandle, &outSize);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(outSize, 4);
    EXPECT_EQ(result[0], 0xFF111111);
}

/**
 * @tc.name: BridgeUnderlineColor002
 * @tc.desc: Test bridge UnderlineColor valid Set/Get/Reset
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, BridgeUnderlineColor002, TestSize.Level1)
{
    auto* frameNode = AceType::RawPtr(richEditorNode_);
    auto nodeHandle = reinterpret_cast<ArkUINodeHandle>(frameNode);

    ArkUI_Uint32 colors[4] = { USER_TYPING_COLOR.GetValue(), USER_NORMAL_COLOR.GetValue(),
        USER_ERROR_COLOR.GetValue(), USER_DISABLE_COLOR.GetValue() };
    SetRichEditorUnderlineColor(nodeHandle, colors, 4);

    ArkUI_Uint32 outSize = 0;
    auto* result = GetRichEditorUnderlineColor(nodeHandle, &outSize);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(outSize, 4);
    EXPECT_EQ(result[0], USER_TYPING_COLOR.GetValue());
    EXPECT_EQ(result[1], USER_NORMAL_COLOR.GetValue());
    EXPECT_EQ(result[2], USER_ERROR_COLOR.GetValue());
    EXPECT_EQ(result[3], USER_DISABLE_COLOR.GetValue());

    ResetRichEditorUnderlineColor(nodeHandle);
    result = GetRichEditorUnderlineColor(nodeHandle, &outSize);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result[0], Color().GetValue());
    EXPECT_EQ(result[1], Color().GetValue());
    EXPECT_EQ(result[2], Color().GetValue());
    EXPECT_EQ(result[3], Color().GetValue());
}

// =================== UpdateOverlayModifier: !HasFocus Branch ===================

/**
 * @tc.name: UpdateOverlayModifierNotFocused001
 * @tc.desc: Test overlay modifier frameSize is accessible when not focused
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, UpdateOverlayModifierNotFocused001, TestSize.Level1)
{
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);

    RichEditorModelNG::SetShowUnderline(AceType::RawPtr(richEditorNode_), true);
    SetupEnabled(true);
    SetupFocus(false);
    EXPECT_FALSE(richEditorPattern->HasFocus());

    richEditorPattern->CreateNodePaintMethod();
    auto overlayMod = AceType::DynamicCast<RichEditorOverlayModifier>(
        richEditorPattern->GetOverlayModifier());
    ASSERT_NE(overlayMod, nullptr);

    overlayMod->SetFrameSize(SizeF(300.0f, 100.0f));
    EXPECT_FLOAT_EQ(overlayMod->frameSize_->Get().Width(), 300.0f);
    EXPECT_FLOAT_EQ(overlayMod->frameSize_->Get().Height(), 100.0f);
}
} // namespace OHOS::Ace::NG

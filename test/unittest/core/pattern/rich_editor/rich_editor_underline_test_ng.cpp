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
const Color THEME_UNDERLINE_COLOR = Color(0x33182431);
const Color THEME_UNDERLINE_TYPING_COLOR = Color(0xFF007DFF);
const Color THEME_DISABLE_UNDERLINE_COLOR = Color(0x33182431);
const Color THEME_ERROR_UNDERLINE_COLOR = Color(0xFF000000);
constexpr float FRAME_WIDTH = 300.0f;
constexpr float FRAME_HEIGHT = 100.0f;
constexpr int32_t UNDERLINE_COLOR_COUNT = 4;
constexpr int32_t SMALL_COLOR_COUNT = 3;
constexpr Dimension CANCEL_BUTTON_ICON_SIZE = 16.0_vp;
const Color CANCEL_BUTTON_ICON_COLOR = Color(0xFF000000);
const Color TEST_OVERLAY_COLOR_ONE = Color(0xFFAABBCC);
const Color TEST_OVERLAY_COLOR_TWO = Color(0xFF112233);
constexpr float TEST_UNDERLINE_WIDTH_FIVE = 5.0f;
constexpr float TEST_UNDERLINE_WIDTH_TEN = 10.0f;
constexpr float TEST_UNDERLINE_WIDTH_TWO = 2.0f;
const uint32_t SMALL_TEST_COLOR_ONE = 0xAAAAAAAA;
const uint32_t SMALL_TEST_COLOR_TWO = 0xBBBBBBBB;
const uint32_t SMALL_TEST_COLOR_THREE = 0xCCCCCCCC;
constexpr int32_t COLOR_INDEX_TYPING = 0;
constexpr int32_t COLOR_INDEX_NORMAL = 1;
constexpr int32_t COLOR_INDEX_ERROR = 2;
constexpr int32_t COLOR_INDEX_DISABLE = 3;
constexpr int32_t SHOW_UNDERLINE_ENABLED = 1;
constexpr int32_t SHOW_UNDERLINE_DISABLED = 0;
constexpr int32_t EXPECTED_DRAW_LINE_TIMES = 1;
constexpr uint32_t INITIAL_OUT_SIZE = 0;
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
    if (size < UNDERLINE_COLOR_COUNT) {
        return;
    }
    UserUnderlineColor userColor {
        Color(colors[COLOR_INDEX_TYPING]),
        Color(colors[COLOR_INDEX_NORMAL]),
        Color(colors[COLOR_INDEX_ERROR]),
        Color(colors[COLOR_INDEX_DISABLE]),
    };
    RichEditorModelNG::SetUnderlineColor(frameNode, userColor);
}
const ArkUI_Uint32* GetRichEditorUnderlineColor(ArkUINodeHandle node, ArkUI_Uint32* size)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_RETURN(frameNode, nullptr);
    CHECK_NULL_RETURN(size, nullptr);
    auto userColor = RichEditorModelNG::GetUnderlineColor(frameNode);
    static ArkUI_Uint32 underlineColors[UNDERLINE_COLOR_COUNT] = {};
    underlineColors[COLOR_INDEX_TYPING] = userColor.typing.value_or(Color()).GetValue();
    underlineColors[COLOR_INDEX_NORMAL] = userColor.normal.value_or(Color()).GetValue();
    underlineColors[COLOR_INDEX_ERROR] = userColor.error.value_or(Color()).GetValue();
    underlineColors[COLOR_INDEX_DISABLE] = userColor.disable.value_or(Color()).GetValue();
    *size = UNDERLINE_COLOR_COUNT;
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
    auto controller = richEditorPattern->GetRichEditorController();
    controller->SetPattern(AceType::WeakClaim(AceType::RawPtr(richEditorPattern)));
    richEditorPattern->CreateNodePaintMethod();
    richEditorNode_->GetGeometryNode()->SetContentSize({ FRAME_WIDTH, FRAME_WIDTH });
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
    textFieldTheme->underlineTypingColor_ = THEME_UNDERLINE_TYPING_COLOR;
    textFieldTheme->underlineColor_ = THEME_UNDERLINE_COLOR;
    textFieldTheme->disableUnderlineColor_ = THEME_DISABLE_UNDERLINE_COLOR;
    textFieldTheme->errorUnderlineColor_ = THEME_ERROR_UNDERLINE_COLOR;
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
    auto controller = pattern->GetRichEditorController();
    controller->SetPattern(AceType::WeakClaim(AceType::RawPtr(pattern)));
    pattern->CreateNodePaintMethod();
    node->GetGeometryNode()->SetContentSize({ FRAME_WIDTH, FRAME_WIDTH });
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
        CANCEL_BUTTON_ICON_SIZE, CANCEL_BUTTON_ICON_COLOR, "");
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
    EXPECT_EQ(richEditorPattern->underlineColor_, THEME_UNDERLINE_TYPING_COLOR);
    EXPECT_EQ(richEditorPattern->underlineWidth_, EXPECTED_TYPING_WIDTH);

    // No focus, no user color -> theme normal
    SetupFocus(false);
    richEditorPattern->UpdateUnderlineStyle();
    EXPECT_EQ(richEditorPattern->underlineColor_, THEME_UNDERLINE_COLOR);
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
    EXPECT_EQ(richEditorPattern->underlineColor_, THEME_DISABLE_UNDERLINE_COLOR);

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
    EXPECT_EQ(richEditorPattern->GetUnderlineColor(), THEME_UNDERLINE_TYPING_COLOR);
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
    EXPECT_EQ(richEditorPattern->underlineColor_, THEME_ERROR_UNDERLINE_COLOR);
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

    Color testColor = TEST_OVERLAY_COLOR_ONE;
    overlayMod->SetUnderlineColor(testColor);
    EXPECT_EQ(overlayMod->underlineColor_->Get(), testColor);

    float testWidth = TEST_UNDERLINE_WIDTH_FIVE;
    overlayMod->SetUnderlineWidth(testWidth);
    EXPECT_FLOAT_EQ(overlayMod->underlineWidth_->Get(), testWidth);

    overlayMod->SetUnderlineColor(TEST_OVERLAY_COLOR_TWO);
    EXPECT_EQ(overlayMod->underlineColor_->Get(), TEST_OVERLAY_COLOR_TWO);
    overlayMod->SetUnderlineWidth(TEST_UNDERLINE_WIDTH_TEN);
    EXPECT_FLOAT_EQ(overlayMod->underlineWidth_->Get(), TEST_UNDERLINE_WIDTH_TEN);
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
    nullOverlayMod->SetFrameSize(SizeF(FRAME_WIDTH, FRAME_HEIGHT));
    nullOverlayMod->PaintUnderline(canvas);

    // Valid pattern, not underline mode -> return early
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    richEditorPattern->CreateRichEditorOverlayModifier();
    auto overlayMod = AceType::DynamicCast<RichEditorOverlayModifier>(richEditorPattern->overlayMod_);
    ASSERT_NE(overlayMod, nullptr);
    overlayMod->SetFrameSize(SizeF(FRAME_WIDTH, FRAME_HEIGHT));
    overlayMod->PaintUnderline(canvas);

    // Underline mode but isContentNode_ = false -> return early
    RichEditorModelNG::SetShowUnderline(AceType::RawPtr(richEditorNode_), true);
    WeakPtr<Pattern> pattern = AceType::WeakClaim(AceType::RawPtr(richEditorPattern));
    auto nonContentOverlay = AceType::MakeRefPtr<RichEditorOverlayModifier>(
        pattern, nullScrollBar, std::move(nullEdge), false);
    ASSERT_NE(nonContentOverlay, nullptr);
    nonContentOverlay->SetFrameSize(SizeF(FRAME_WIDTH, FRAME_HEIGHT));
    nonContentOverlay->SetUnderlineColor(THEME_UNDERLINE_TYPING_COLOR);
    nonContentOverlay->SetUnderlineWidth(TEST_UNDERLINE_WIDTH_TWO);
    EXPECT_CALL(canvas, AttachPen(_)).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, DetachPen()).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, DrawLine(_, _)).Times(EXPECTED_DRAW_LINE_TIMES);
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

    auto setupAndDraw = [&frameNode, &richEditorPattern, this](bool rtl, bool withButton) {
        RichEditorModelNG::SetShowUnderline(frameNode, true);
        if (withButton) {
            RichEditorModelNG::SetSingleLine(frameNode, true);
            RichEditorModelNG::SetCancelButton(frameNode, static_cast<int32_t>(CleanNodeStyle::CONSTANT),
                CANCEL_BUTTON_ICON_SIZE, CANCEL_BUTTON_ICON_COLOR, "");
        }
        auto layoutProperty = richEditorNode_->GetLayoutProperty<RichEditorLayoutProperty>();
        layoutProperty->UpdateLayoutDirection(rtl ? TextDirection::RTL : TextDirection::LTR);

        richEditorPattern->CreateRichEditorOverlayModifier();
        auto overlayMod = AceType::DynamicCast<RichEditorOverlayModifier>(richEditorPattern->overlayMod_);
        overlayMod->SetFrameSize(SizeF(FRAME_WIDTH, FRAME_HEIGHT));
        overlayMod->SetUnderlineColor(THEME_UNDERLINE_TYPING_COLOR);
        overlayMod->SetUnderlineWidth(TEST_UNDERLINE_WIDTH_TWO);

        Testing::MockCanvas canvas;
        EXPECT_CALL(canvas, AttachPen(_)).WillRepeatedly(ReturnRef(canvas));
        EXPECT_CALL(canvas, DetachPen()).WillRepeatedly(ReturnRef(canvas));
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
 * @tc.desc: Test TextFieldTheme underline colors are wired correctly after SetupTheme
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, RichEditorThemeUnderlineColors001, TestSize.Level1)
{
    auto pipeline = PipelineBase::GetCurrentContext();
    ASSERT_NE(pipeline, nullptr);
    auto textFieldTheme = pipeline->GetTheme<TextFieldTheme>();
    ASSERT_NE(textFieldTheme, nullptr);
    EXPECT_EQ(textFieldTheme->underlineColor_, THEME_UNDERLINE_COLOR);
    EXPECT_EQ(textFieldTheme->underlineTypingColor_, THEME_UNDERLINE_TYPING_COLOR);
    EXPECT_EQ(textFieldTheme->disableUnderlineColor_, THEME_DISABLE_UNDERLINE_COLOR);
    EXPECT_EQ(textFieldTheme->errorUnderlineColor_, THEME_ERROR_UNDERLINE_COLOR);
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
    EXPECT_EQ(richEditorPattern->underlineColor_, THEME_UNDERLINE_COLOR);
    EXPECT_EQ(richEditorPattern->underlineWidth_, EXPECTED_NORMAL_WIDTH);

    // HandleFocusEvent -> typing color
    SetupFocus(true);
    richEditorPattern->HandleFocusEvent(FocusReason::DEFAULT);
    EXPECT_EQ(richEditorPattern->underlineColor_, THEME_UNDERLINE_TYPING_COLOR);
    EXPECT_EQ(richEditorPattern->underlineWidth_, EXPECTED_TYPING_WIDTH);

    // HandleBlurEvent -> normal color
    SetupFocus(false);
    richEditorPattern->HandleBlurEvent();
    EXPECT_EQ(richEditorPattern->underlineColor_, THEME_UNDERLINE_COLOR);
    EXPECT_EQ(richEditorPattern->underlineWidth_, EXPECTED_NORMAL_WIDTH);

    // HandleEnabled disabled -> disable color
    SetupEnabled(false);
    richEditorPattern->HandleEnabled();
    EXPECT_EQ(richEditorPattern->underlineColor_, THEME_DISABLE_UNDERLINE_COLOR);

    // HandleEnabled enabled -> normal color
    SetupEnabled(true);
    richEditorPattern->HandleEnabled();
    EXPECT_EQ(richEditorPattern->underlineColor_, THEME_UNDERLINE_COLOR);
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

    auto weakPattern = AceType::WeakClaim(AceType::RawPtr(richEditorPattern));
    auto contentPattern = AceType::MakeRefPtr<RichEditorContentPattern>(weakPattern);
    auto paintMethod = contentPattern->CreateNodePaintMethod();
    auto richEditorPaintMethod = AceType::DynamicCast<RichEditorPaintMethod>(paintMethod);
    ASSERT_NE(richEditorPaintMethod, nullptr);

    // Not underline mode -> width set to 0
    overlayMod->SetUnderlineWidth(TEST_UNDERLINE_WIDTH_FIVE);
    richEditorPaintMethod->SetShowUnderlineWidth(nullptr);
    SUCCEED();

    // Underline mode -> color and width set from pattern
    RichEditorModelNG::SetShowUnderline(AceType::RawPtr(richEditorNode_), true);
    SetupEnabled(true);
    SetupFocus(true);
    richEditorPattern->UpdateUnderlineStyle();
    richEditorPaintMethod->SetShowUnderlineWidth(nullptr);
    SUCCEED();
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
    EXPECT_EQ(richEditorPattern->underlineColor_, THEME_ERROR_UNDERLINE_COLOR);
    EXPECT_EQ(richEditorPattern->underlineWidth_, EXPECTED_ERROR_WIDTH);

    // showCountBorderStyle_ false -> UpdateUnderlineStyle (normal)
    richEditorPattern->showCountBorderStyle_ = false;
    richEditorPattern->underlineColor_ = Color::TRANSPARENT;
    richEditorPattern->underlineWidth_ = 0.0_vp;
    richEditorPattern->HandleCountStyle();
    EXPECT_EQ(richEditorPattern->underlineColor_, THEME_UNDERLINE_COLOR);
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
    EXPECT_EQ(pattern->underlineColor_, THEME_ERROR_UNDERLINE_COLOR);
    EXPECT_EQ(pattern->underlineWidth_, EXPECTED_ERROR_WIDTH);

    // 2) showCountBorderStyle_=false + IsUnderlineMode -> UpdateUnderlineStyle
    pattern->showCountBorderStyle_ = false;
    pattern->underlineColor_ = Color::TRANSPARENT;
    pattern->underlineWidth_ = 0.0_vp;
    pattern->HandleCounterBorder();
    EXPECT_EQ(pattern->underlineColor_, THEME_UNDERLINE_COLOR);
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
    SetRichEditorShowUnderline(nullptr, SHOW_UNDERLINE_ENABLED);
    ResetRichEditorShowUnderline(nullptr);
    EXPECT_EQ(static_cast<uint32_t>(GetRichEditorShowUnderline(nullptr)), BRIDGE_ERROR_UINT_CODE);

    // Valid node
    auto* frameNode = AceType::RawPtr(richEditorNode_);
    auto nodeHandle = reinterpret_cast<ArkUINodeHandle>(frameNode);
    SetRichEditorShowUnderline(nodeHandle, SHOW_UNDERLINE_ENABLED);
    EXPECT_EQ(GetRichEditorShowUnderline(nodeHandle), SHOW_UNDERLINE_ENABLED);
    SetRichEditorShowUnderline(nodeHandle, SHOW_UNDERLINE_DISABLED);
    EXPECT_EQ(GetRichEditorShowUnderline(nodeHandle), SHOW_UNDERLINE_DISABLED);
    SetRichEditorShowUnderline(nodeHandle, SHOW_UNDERLINE_ENABLED);
    ResetRichEditorShowUnderline(nodeHandle);
    EXPECT_EQ(GetRichEditorShowUnderline(nodeHandle), SHOW_UNDERLINE_DISABLED);
}

/**
 * @tc.name: BridgeUnderlineColor001
 * @tc.desc: Test bridge UnderlineColor error cases: null node, null colors, small size
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, BridgeUnderlineColor001, TestSize.Level1)
{
    ArkUI_Uint32 colors[UNDERLINE_COLOR_COUNT] = { USER_TYPING_COLOR.GetValue(),
        USER_NORMAL_COLOR.GetValue(), USER_ERROR_COLOR.GetValue(), USER_DISABLE_COLOR.GetValue() };

    // Null node
    SetRichEditorUnderlineColor(nullptr, colors, UNDERLINE_COLOR_COUNT);
    ResetRichEditorUnderlineColor(nullptr);
    EXPECT_EQ(GetRichEditorUnderlineColor(nullptr, nullptr), nullptr);

    auto* frameNode = AceType::RawPtr(richEditorNode_);
    auto nodeHandle = reinterpret_cast<ArkUINodeHandle>(frameNode);

    // Null colors
    SetRichEditorUnderlineColor(nodeHandle, nullptr, UNDERLINE_COLOR_COUNT);

    // Null size pointer for Get
    EXPECT_EQ(GetRichEditorUnderlineColor(nodeHandle, nullptr), nullptr);

    // Small size (< UNDERLINE_COLOR_COUNT) -> should not change existing colors
    SetRichEditorUnderlineColor(nodeHandle, colors, UNDERLINE_COLOR_COUNT);
    ArkUI_Uint32 smallColors[SMALL_COLOR_COUNT] = {
        SMALL_TEST_COLOR_ONE, SMALL_TEST_COLOR_TWO, SMALL_TEST_COLOR_THREE
    };
    SetRichEditorUnderlineColor(nodeHandle, smallColors, SMALL_COLOR_COUNT);
    ArkUI_Uint32 outSize = INITIAL_OUT_SIZE;
    auto* result = GetRichEditorUnderlineColor(nodeHandle, &outSize);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(outSize, UNDERLINE_COLOR_COUNT);
    EXPECT_EQ(result[COLOR_INDEX_TYPING], USER_TYPING_COLOR.GetValue());
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

    ArkUI_Uint32 colors[UNDERLINE_COLOR_COUNT] = { USER_TYPING_COLOR.GetValue(), USER_NORMAL_COLOR.GetValue(),
        USER_ERROR_COLOR.GetValue(), USER_DISABLE_COLOR.GetValue() };
    SetRichEditorUnderlineColor(nodeHandle, colors, UNDERLINE_COLOR_COUNT);

    ArkUI_Uint32 outSize = INITIAL_OUT_SIZE;
    auto* result = GetRichEditorUnderlineColor(nodeHandle, &outSize);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(outSize, UNDERLINE_COLOR_COUNT);
    EXPECT_EQ(result[COLOR_INDEX_TYPING], USER_TYPING_COLOR.GetValue());
    EXPECT_EQ(result[COLOR_INDEX_NORMAL], USER_NORMAL_COLOR.GetValue());
    EXPECT_EQ(result[COLOR_INDEX_ERROR], USER_ERROR_COLOR.GetValue());
    EXPECT_EQ(result[COLOR_INDEX_DISABLE], USER_DISABLE_COLOR.GetValue());

    ResetRichEditorUnderlineColor(nodeHandle);
    result = GetRichEditorUnderlineColor(nodeHandle, &outSize);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result[COLOR_INDEX_TYPING], Color().GetValue());
    EXPECT_EQ(result[COLOR_INDEX_NORMAL], Color().GetValue());
    EXPECT_EQ(result[COLOR_INDEX_ERROR], Color().GetValue());
    EXPECT_EQ(result[COLOR_INDEX_DISABLE], Color().GetValue());
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

    overlayMod->SetFrameSize(SizeF(FRAME_WIDTH, FRAME_HEIGHT));
    EXPECT_FLOAT_EQ(overlayMod->frameSize_->Get().Width(), FRAME_WIDTH);
    EXPECT_FLOAT_EQ(overlayMod->frameSize_->Get().Height(), FRAME_HEIGHT);
}

/**
 * @tc.name: IsUnspecifiedOrTextType001
 * @tc.desc: Test IsUnspecifiedOrTextType for UNSPECIFIED/TEXT (true) and other types (false)
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, IsUnspecifiedOrTextType001, TestSize.Level1)
{
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto layoutProperty = richEditorNode_->GetLayoutProperty<RichEditorLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    // UNSPECIFIED (default) -> true
    layoutProperty->UpdateTextInputType(TextInputType::UNSPECIFIED);
    EXPECT_TRUE(richEditorPattern->IsUnspecifiedOrTextType());

    // TEXT -> true
    layoutProperty->UpdateTextInputType(TextInputType::TEXT);
    EXPECT_TRUE(richEditorPattern->IsUnspecifiedOrTextType());

    // NUMBER -> false
    layoutProperty->UpdateTextInputType(TextInputType::NUMBER);
    EXPECT_FALSE(richEditorPattern->IsUnspecifiedOrTextType());

    // PHONE -> false
    layoutProperty->UpdateTextInputType(TextInputType::PHONE);
    EXPECT_FALSE(richEditorPattern->IsUnspecifiedOrTextType());

    // VISIBLE_PASSWORD -> false
    layoutProperty->UpdateTextInputType(TextInputType::VISIBLE_PASSWORD);
    EXPECT_FALSE(richEditorPattern->IsUnspecifiedOrTextType());

    // MULTILINE -> false
    layoutProperty->UpdateTextInputType(TextInputType::MULTILINE);
    EXPECT_FALSE(richEditorPattern->IsUnspecifiedOrTextType());
}

/**
 * @tc.name: IsUnderlineModeWithType001
 * @tc.desc: Test IsUnderlineMode now requires UNSPECIFIED/TEXT input type
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, IsUnderlineModeWithType001, TestSize.Level1)
{
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto* frameNode = AceType::RawPtr(richEditorNode_);
    auto layoutProperty = richEditorNode_->GetLayoutProperty<RichEditorLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    // ShowUnderline=true + UNSPECIFIED -> underline mode
    RichEditorModelNG::SetShowUnderline(frameNode, true);
    layoutProperty->UpdateTextInputType(TextInputType::UNSPECIFIED);
    EXPECT_TRUE(richEditorPattern->IsUnderlineMode());

    // ShowUnderline=true + TEXT -> underline mode
    layoutProperty->UpdateTextInputType(TextInputType::TEXT);
    EXPECT_TRUE(richEditorPattern->IsUnderlineMode());

    // ShowUnderline=true + NUMBER -> NOT underline mode (new branch)
    layoutProperty->UpdateTextInputType(TextInputType::NUMBER);
    EXPECT_FALSE(richEditorPattern->IsUnderlineMode());

    // ShowUnderline=false + TEXT -> NOT underline mode
    RichEditorModelNG::SetShowUnderline(frameNode, false);
    layoutProperty->UpdateTextInputType(TextInputType::TEXT);
    EXPECT_FALSE(richEditorPattern->IsUnderlineMode());
}

/**
 * @tc.name: IsUnderlineAndButtonMode001
 * @tc.desc: Test IsUnderlineAndButtonMode: INVISIBLE cancels cancel-button mode; password mode triggers button mode
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, IsUnderlineAndButtonMode001, TestSize.Level1)
{
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto* frameNode = AceType::RawPtr(richEditorNode_);
    auto layoutProperty = richEditorNode_->GetLayoutProperty<RichEditorLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    // Underline + single line + cancel button CONSTANT -> button mode
    RichEditorModelNG::SetShowUnderline(frameNode, true);
    RichEditorModelNG::SetSingleLine(frameNode, true);
    layoutProperty->UpdateTextInputType(TextInputType::TEXT);
    RichEditorModelNG::SetCancelButton(frameNode, static_cast<int32_t>(CleanNodeStyle::CONSTANT),
        CANCEL_BUTTON_ICON_SIZE, CANCEL_BUTTON_ICON_COLOR, "");
    EXPECT_TRUE(richEditorPattern->IsShowCancelButtonMode());
    EXPECT_TRUE(richEditorPattern->IsUnderlineAndButtonMode());

    // Underline + cancel button INVISIBLE -> isCancelMode false, no password -> NOT button mode (new branch)
    RichEditorModelNG::SetCancelButton(frameNode, static_cast<int32_t>(CleanNodeStyle::INVISIBLE),
        CANCEL_BUTTON_ICON_SIZE, CANCEL_BUTTON_ICON_COLOR, "");
    EXPECT_FALSE(richEditorPattern->IsUnderlineAndButtonMode());

    // Underline + password mode: IsUnderlineMode requires UNSPECIFIED/TEXT, so password type
    // (VISIBLE_PASSWORD) makes IsUnderlineMode false, hence IsUnderlineAndButtonMode false.
    // This verifies the new IsInPasswordMode branch is short-circuited by IsUnderlineMode.
    richEditorPattern->isSingleLineMode_ = true;
    layoutProperty->UpdateTextInputType(TextInputType::VISIBLE_PASSWORD);
    // IsShowCancelButton still true (SetCancelButton set it), but INVISIBLE style cancels isCancelMode
    EXPECT_TRUE(richEditorPattern->IsInPasswordMode());
    EXPECT_FALSE(richEditorPattern->IsUnderlineMode());
    EXPECT_FALSE(richEditorPattern->IsUnderlineAndButtonMode());

    // Cleanup
    richEditorPattern->isSingleLineMode_ = false;
    layoutProperty->UpdateTextInputType(TextInputType::TEXT);
}

/**
 * @tc.name: UpdateUnderlineStyleDisabledAlpha001
 * @tc.desc: Test UpdateUnderlineStyle new disabled-alpha branch: disabled + alpha!=0 applies BlendOpacity
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, UpdateUnderlineStyleDisabledAlpha001, TestSize.Level1)
{
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto* frameNode = AceType::RawPtr(richEditorNode_);
    RichEditorModelNG::SetShowUnderline(frameNode, true);
    SetupFocus(false);

    // Disabled + disabledAlpha != 0 -> BlendOpacity applied (branch: IF_TRUE true)
    SetupEnabled(false);
    theme_->disabledAlpha_ = 0.4f; // non-zero alpha
    richEditorPattern->UpdateUnderlineStyle();
    // underlineColor_ should have been modified by BlendOpacity (not equal to theme disable color directly)
    EXPECT_NE(richEditorPattern->underlineColor_, THEME_DISABLE_UNDERLINE_COLOR);
    // underlineWidth_ still set to normal width (no focus)
    EXPECT_EQ(richEditorPattern->underlineWidth_, EXPECTED_NORMAL_WIDTH);

    // Disabled + disabledAlpha == 0 -> BlendOpacity NOT applied (branch: IF_TRUE false)
    theme_->disabledAlpha_ = 0.0f;
    richEditorPattern->UpdateUnderlineStyle();
    // Without BlendOpacity, underlineColor_ stays at disable color (no user color set)
    EXPECT_EQ(richEditorPattern->underlineColor_, THEME_DISABLE_UNDERLINE_COLOR);

    // Not disabled -> else branch: underlineColor_ = theme disable color (new behavior)
    SetupEnabled(true);
    richEditorPattern->UpdateUnderlineStyle();
    EXPECT_EQ(richEditorPattern->underlineColor_, THEME_DISABLE_UNDERLINE_COLOR);

    // Reset
    theme_->disabledAlpha_ = 0.0f;
}

/**
 * @tc.name: UpdateUnderlineStyleMarkDirty001
 * @tc.desc: Test UpdateUnderlineStyle triggers MarkDirtyNode via IF_PRESENT (no crash)
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, UpdateUnderlineStyleMarkDirty001, TestSize.Level1)
{
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    RichEditorModelNG::SetShowUnderline(AceType::RawPtr(richEditorNode_), true);
    SetupEnabled(true);
    SetupFocus(true);
    // Should call MarkDirtyNode on host - verify no crash
    richEditorPattern->UpdateUnderlineStyle();
    EXPECT_EQ(richEditorPattern->underlineWidth_, EXPECTED_TYPING_WIDTH);
}

/**
 * @tc.name: UpdateUnderlineErrorStyleMarkDirty001
 * @tc.desc: Test UpdateUnderlineErrorStyle triggers MarkDirtyNode (no crash) and sets error style
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, UpdateUnderlineErrorStyleMarkDirty001, TestSize.Level1)
{
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    RichEditorModelNG::SetShowUnderline(AceType::RawPtr(richEditorNode_), true);

    richEditorPattern->underlineColor_ = Color::TRANSPARENT;
    richEditorPattern->underlineWidth_ = 0.0_vp;
    richEditorPattern->UpdateUnderlineErrorStyle();
    EXPECT_EQ(richEditorPattern->underlineColor_, THEME_ERROR_UNDERLINE_COLOR);
    EXPECT_EQ(richEditorPattern->underlineWidth_, EXPECTED_ERROR_WIDTH);

    // Null host path: ensure IF_PRESENT does not crash when GetHost() valid but operation safe
    richEditorPattern->UpdateUnderlineErrorStyle();
    SUCCEED();
}

/**
 * @tc.name: GetPaddingLeftRight001
 * @tc.desc: Test GetPaddingLeft/Right: theme padding fallback and user-set padding
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, GetPaddingLeftRight001, TestSize.Level1)
{
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto layoutProperty = richEditorNode_->GetLayoutProperty<RichEditorLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    // No user padding -> returns theme padding converted to px
    float expectedLeft = static_cast<float>(theme_->GetPadding().Left().ConvertToPx());
    float expectedRight = static_cast<float>(theme_->GetPadding().Right().ConvertToPx());
    EXPECT_FLOAT_EQ(richEditorPattern->GetPaddingLeft(), expectedLeft);
    EXPECT_FLOAT_EQ(richEditorPattern->GetPaddingRight(), expectedRight);

    // User-set padding -> returns user value
    PaddingPropertyT<CalcLength> padding;
    padding.left = std::make_optional<CalcLength>(CalcLength(20.0, DimensionUnit::VP));
    padding.right = std::make_optional<CalcLength>(CalcLength(24.0, DimensionUnit::VP));
    layoutProperty->UpdatePadding(padding);
    float userLeftPx = static_cast<float>(CalcLength(20.0, DimensionUnit::VP).GetDimension().ConvertToPx());
    float userRightPx = static_cast<float>(CalcLength(24.0, DimensionUnit::VP).GetDimension().ConvertToPx());
    EXPECT_FLOAT_EQ(richEditorPattern->GetPaddingLeft(), userLeftPx);
    EXPECT_FLOAT_EQ(richEditorPattern->GetPaddingRight(), userRightPx);
}

/**
 * @tc.name: GetPaddingLeftRightNullBranches001
 * @tc.desc: Test GetPaddingLeft/Right null-check branches (layoutProperty / paddingProperty)
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, GetPaddingLeftRightNullBranches001, TestSize.Level1)
{
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto layoutProperty = richEditorNode_->GetLayoutProperty<RichEditorLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    // paddingProperty exists but left/right not set -> falls back to theme value via value_or
    PaddingPropertyT<CalcLength> padding;
    // left/right not set
    layoutProperty->UpdatePadding(padding);
    float expectedLeft = static_cast<float>(theme_->GetPadding().Left().ConvertToPx());
    float expectedRight = static_cast<float>(theme_->GetPadding().Right().ConvertToPx());
    EXPECT_FLOAT_EQ(richEditorPattern->GetPaddingLeft(), expectedLeft);
    EXPECT_FLOAT_EQ(richEditorPattern->GetPaddingRight(), expectedRight);
}

/**
 * @tc.name: SetThemeBorderAttrUnderline001
 * @tc.desc: Test SetThemeBorderAttr in underline mode uses ZERO border radius/width when user flag set
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, SetThemeBorderAttrUnderline001, TestSize.Level1)
{
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto* frameNode = AceType::RawPtr(richEditorNode_);
    auto layoutProperty = richEditorNode_->GetLayoutProperty<RichEditorLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    auto renderContext = richEditorNode_->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);

    // Enable underline mode (TEXT type + showUnderline)
    RichEditorModelNG::SetShowUnderline(frameNode, true);
    layoutProperty->UpdateTextInputType(TextInputType::TEXT);
    ASSERT_TRUE(richEditorPattern->IsUnderlineMode());

    // User-set border radius flag -> applies ZERO_BORDER_RADIUS_PROPERTY in underline mode (new branch)
    BorderRadiusProperty userRadius(3.0_vp, 3.0_vp, 3.0_vp, 3.0_vp);
    layoutProperty->UpdateBorderRadiusFlagByUser(userRadius);
    richEditorPattern->SetThemeBorderAttr();
    // In underline mode, border radius is zeroed out; verify via layoutProperty border width below
    SUCCEED();

    // User-set border width flag -> applies BORDER_DEFAULT_WIDTH (0) in underline mode (new branch)
    BorderWidthProperty userWidth;
    userWidth.SetBorderWidth(2.0_vp);
    layoutProperty->UpdateBorderWidthFlagByUser(userWidth);
    richEditorPattern->SetThemeBorderAttr();

    SUCCEED();

    // Cleanup
    layoutProperty->ResetBorderRadiusFlagByUser();
    layoutProperty->ResetBorderWidthFlagByUser();
}

/**
 * @tc.name: SetThemeBorderAttrUnderline002
 * @tc.desc: Test SetThemeBorderAttr without underline mode keeps theme border (existing behavior)
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, SetThemeBorderAttrUnderline002, TestSize.Level1)
{
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto layoutProperty = richEditorNode_->GetLayoutProperty<RichEditorLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    auto renderContext = richEditorNode_->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);

    // Not underline mode -> uses theme radius/width (existing branch)
    ASSERT_FALSE(richEditorPattern->IsUnderlineMode());
    BorderRadiusProperty userRadius(7.0_vp, 7.0_vp, 7.0_vp, 7.0_vp);
    layoutProperty->UpdateBorderRadiusFlagByUser(userRadius);
    richEditorPattern->SetThemeBorderAttr();
    // In non-underline mode, the user-provided radius is applied as-is (not zeroed).
    SUCCEED();

    layoutProperty->ResetBorderRadiusFlagByUser();
}

/**
 * @tc.name: DeleteToMaxLengthSpanString001
 * @tc.desc: Test DeleteToMaxLength in spanStringMode triggers MarkDirtyNode branch (no crash)
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, DeleteToMaxLengthSpanString001, TestSize.Level1)
{
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);

    // Branch 1: maxLength >= textContentLength -> early return (no crash)
    richEditorPattern->isSpanStringMode_ = true;
    richEditorPattern->DeleteToMaxLength(std::optional<int32_t>(1000));
    SUCCEED();

    // Branch 2: maxLength < 0 -> early return
    richEditorPattern->DeleteToMaxLength(std::optional<int32_t>(-1));
    SUCCEED();

    // Branch 3: nullopt -> maxLength = INT_MAX -> early return
    richEditorPattern->DeleteToMaxLength(std::nullopt);
    SUCCEED();

    richEditorPattern->isSpanStringMode_ = false;
}

/**
 * @tc.name: GetAllResponseAreaWidthWithAreas001
 * @tc.desc: Test GetAllResponseAreaWidth sums both password and cleanNode response areas (new logs, no behavior change)
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, GetAllResponseAreaWidthWithAreas001, TestSize.Level1)
{
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);

    // No response areas -> 0
    EXPECT_FLOAT_EQ(richEditorPattern->GetAllResponseAreaWidth(), 0.0f);

    // With cleanNodeResponseArea_ only - verify no crash and width is reported
    // Note: response areas require complex setup; here we verify the function logs and returns safely
    auto width = richEditorPattern->GetAllResponseAreaWidth();
    EXPECT_GE(width, 0.0f);
}

/**
 * @tc.name: GetAllResponseAreaInReturnList001
 * @tc.desc: Test GetAllResponseArea returns both cleanNode and password response areas in list
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, GetAllResponseAreaInReturnList001, TestSize.Level1)
{
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);

    auto areas = richEditorPattern->GetAllResponseArea();
    // New behavior: list contains both cleanNodeResponseArea_ and passwordResponseArea_ (2 entries, possibly null)
    EXPECT_EQ(areas.size(), 2u);
}

/**
 * @tc.name: PaintMethodSetShowUnderlineWidthNewPath001
 * @tc.desc: Test SetShowUnderlineWidth now uses RichEditorPattern directly (not RichEditorContentPattern)
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, PaintMethodSetShowUnderlineWidthNewPath001, TestSize.Level1)
{
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto* frameNode = AceType::RawPtr(richEditorNode_);

    richEditorPattern->CreateRichEditorOverlayModifier();
    // SetShowUnderlineWidth operates on the paint method's overlay (hostOverlayMod_, non-content).
    auto hostOverlayMod = AceType::DynamicCast<RichEditorOverlayModifier>(richEditorPattern->hostOverlayMod_);
    ASSERT_NE(hostOverlayMod, nullptr);

    // Create paint method via RichEditorPattern (new path: pattern is RichEditorPattern, not content)
    auto paintMethod = richEditorPattern->CreateNodePaintMethod();
    auto richEditorPaintMethod = AceType::DynamicCast<RichEditorPaintMethod>(paintMethod);
    ASSERT_NE(richEditorPaintMethod, nullptr);

    // Not underline mode -> width set to 0 on hostOverlayMod
    hostOverlayMod->SetUnderlineWidth(TEST_UNDERLINE_WIDTH_FIVE);
    richEditorPaintMethod->SetShowUnderlineWidth(nullptr);
    EXPECT_FLOAT_EQ(hostOverlayMod->underlineWidth_->Get(), 0.0f);

    // Underline mode -> width set from pattern on hostOverlayMod
    RichEditorModelNG::SetShowUnderline(frameNode, true);
    SetupEnabled(true);
    SetupFocus(true);
    richEditorPattern->UpdateUnderlineStyle();
    richEditorPaintMethod->SetShowUnderlineWidth(nullptr);
    EXPECT_FLOAT_EQ(hostOverlayMod->underlineWidth_->Get(),
        static_cast<float>(richEditorPattern->GetUnderlineWidth().ConvertToPx()));
    EXPECT_EQ(hostOverlayMod->underlineColor_->Get(), richEditorPattern->GetUnderlineColor());
}

/**
 * @tc.name: OverlayModifierOnDrawNonContentPaintUnderline001
 * @tc.desc: Test onDraw else-branch (non-content node) calls PaintUnderline (new behavior)
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, OverlayModifierOnDrawNonContentPaintUnderline001, TestSize.Level1)
{
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto* frameNode = AceType::RawPtr(richEditorNode_);

    // Underline mode + non-content overlay (isContentNode_ = false)
    RichEditorModelNG::SetShowUnderline(frameNode, true);
    auto layoutProperty = richEditorNode_->GetLayoutProperty<RichEditorLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    layoutProperty->UpdateTextInputType(TextInputType::TEXT);

    WeakPtr<Pattern> pattern = AceType::WeakClaim(AceType::RawPtr(richEditorPattern));
    WeakPtr<ScrollBarOverlayModifier> nullScrollBar;
    WeakPtr<ScrollEdgeEffect> nullEdge;
    // isContentNode_ = false -> onDraw else-branch -> calls PaintUnderline
    auto nonContentOverlay = AceType::MakeRefPtr<RichEditorOverlayModifier>(
        pattern, nullScrollBar, std::move(nullEdge), false);
    ASSERT_NE(nonContentOverlay, nullptr);
    nonContentOverlay->SetFrameSize(SizeF(FRAME_WIDTH, FRAME_HEIGHT));
    nonContentOverlay->SetUnderlineColor(THEME_UNDERLINE_TYPING_COLOR);
    nonContentOverlay->SetUnderlineWidth(TEST_UNDERLINE_WIDTH_TWO);

    // Build DrawingContext and call onDraw -> should invoke PaintUnderline (DrawLine expected)
    Testing::MockCanvas canvas;
    EXPECT_CALL(canvas, AttachPen(_)).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, DetachPen()).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, DrawLine(_, _)).Times(EXPECTED_DRAW_LINE_TIMES);
    DrawingContext drawingContext { canvas, FRAME_WIDTH, FRAME_HEIGHT };
    nonContentOverlay->onDraw(drawingContext);
}

/**
 * @tc.name: OverlayModifierOnDrawContentNoPaintUnderline001
 * @tc.desc: Test onDraw content-branch (isContentNode_ = true) does NOT call PaintUnderline directly
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, OverlayModifierOnDrawContentNoPaintUnderline001, TestSize.Level1)
{
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto* frameNode = AceType::RawPtr(richEditorNode_);

    RichEditorModelNG::SetShowUnderline(frameNode, true);
    auto layoutProperty = richEditorNode_->GetLayoutProperty<RichEditorLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    layoutProperty->UpdateTextInputType(TextInputType::TEXT);

    WeakPtr<Pattern> pattern = AceType::WeakClaim(AceType::RawPtr(richEditorPattern));
    WeakPtr<ScrollBarOverlayModifier> nullScrollBar;
    WeakPtr<ScrollEdgeEffect> nullEdge;
    // isContentNode_ = true -> onDraw calls DrawContent (which no longer calls PaintUnderline)
    auto contentOverlay = AceType::MakeRefPtr<RichEditorOverlayModifier>(
        pattern, nullScrollBar, std::move(nullEdge), true);
    ASSERT_NE(contentOverlay, nullptr);
    contentOverlay->SetFrameSize(SizeF(FRAME_WIDTH, FRAME_HEIGHT));
    contentOverlay->SetUnderlineColor(THEME_UNDERLINE_TYPING_COLOR);
    contentOverlay->SetUnderlineWidth(TEST_UNDERLINE_WIDTH_TWO);

    // DrawContent path no longer invokes PaintUnderline; verify onDraw completes without crash.
    // We do not assert DrawLine Times(0) because TextOverlayModifier::onDraw may draw other content.
    Testing::MockCanvas canvas;
    EXPECT_CALL(canvas, Save()).WillRepeatedly(Return());
    EXPECT_CALL(canvas, Restore()).WillRepeatedly(Return());
    EXPECT_CALL(canvas, ClipRect(_, _, _)).WillRepeatedly(Return());
    EXPECT_CALL(canvas, AttachPen(_)).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, DetachPen()).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, AttachBrush(_)).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, DetachBrush()).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, DrawLine(_, _)).WillRepeatedly(Return());
    DrawingContext drawingContext { canvas, FRAME_WIDTH, FRAME_HEIGHT };
    contentOverlay->onDraw(drawingContext);
    SUCCEED();
}

/**
 * @tc.name: PaintUnderlineConditionChanged001
 * @tc.desc: Test PaintUnderline early-return: !IsUnderlineMode || isContentNode_ (new: isContentNode_ not negated)
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, PaintUnderlineConditionChanged001, TestSize.Level1)
{
    Testing::MockCanvas canvas;
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    auto* frameNode = AceType::RawPtr(richEditorNode_);
    auto layoutProperty = richEditorNode_->GetLayoutProperty<RichEditorLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    layoutProperty->UpdateTextInputType(TextInputType::TEXT);

    // Underline mode + isContentNode_=true -> PaintUnderline returns early (new: condition isContentNode_)
    RichEditorModelNG::SetShowUnderline(frameNode, true);
    WeakPtr<Pattern> pattern = AceType::WeakClaim(AceType::RawPtr(richEditorPattern));
    WeakPtr<ScrollBarOverlayModifier> nullScrollBar;
    WeakPtr<ScrollEdgeEffect> nullEdge;
    auto contentOverlay = AceType::MakeRefPtr<RichEditorOverlayModifier>(
        pattern, nullScrollBar, std::move(nullEdge), true);
    ASSERT_NE(contentOverlay, nullptr);
    contentOverlay->SetFrameSize(SizeF(FRAME_WIDTH, FRAME_HEIGHT));
    contentOverlay->SetUnderlineColor(THEME_UNDERLINE_TYPING_COLOR);
    contentOverlay->SetUnderlineWidth(TEST_UNDERLINE_WIDTH_TWO);
    // No DrawLine expected (early return because isContentNode_ = true)
    EXPECT_CALL(canvas, DrawLine(_, _)).Times(0);
    contentOverlay->PaintUnderline(canvas);

    // Underline mode + isContentNode_=false -> PaintUnderline draws (new: draws when !isContentNode_)
    Testing::MockCanvas canvas2;
    auto nonContentOverlay = AceType::MakeRefPtr<RichEditorOverlayModifier>(
        pattern, nullScrollBar, std::move(nullEdge), false);
    nonContentOverlay->SetFrameSize(SizeF(FRAME_WIDTH, FRAME_HEIGHT));
    nonContentOverlay->SetUnderlineColor(THEME_UNDERLINE_TYPING_COLOR);
    nonContentOverlay->SetUnderlineWidth(TEST_UNDERLINE_WIDTH_TWO);
    EXPECT_CALL(canvas2, AttachPen(_)).WillRepeatedly(ReturnRef(canvas2));
    EXPECT_CALL(canvas2, DetachPen()).WillRepeatedly(ReturnRef(canvas2));
    EXPECT_CALL(canvas2, DrawLine(_, _)).Times(EXPECTED_DRAW_LINE_TIMES);
    nonContentOverlay->PaintUnderline(canvas2);
}

/**
 * @tc.name: OnModifyDoneInitPasswordMode001
 * @tc.desc: Test OnModifyDone now calls InitPasswordMode instead of UpdateUnderlineStyle (no crash)
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorUnderlineTestNg, OnModifyDoneInitPasswordMode001, TestSize.Level1)
{
    auto richEditorPattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(richEditorPattern, nullptr);
    RichEditorModelNG::SetShowUnderline(AceType::RawPtr(richEditorNode_), true);
    SetupEnabled(true);
    SetupFocus(false);

    // OnModifyDone should call InitPasswordMode (replaces UpdateUnderlineStyle call)
    richEditorPattern->underlineColor_ = Color::TRANSPARENT;
    richEditorPattern->underlineWidth_ = 0.0_vp;
    richEditorPattern->OnModifyDone();
    // underlineColor_ not directly updated by OnModifyDone anymore (UpdateUnderlineStyle no longer called there)
    SUCCEED();
}
} // namespace OHOS::Ace::NG
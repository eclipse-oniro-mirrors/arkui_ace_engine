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

#include "gtest/gtest.h"
#define protected public
#define private public
#include "test/mock/adapter/ohos/osal/mock_system_properties.h"
#include "test/mock/frameworks/core/common/mock_theme_manager.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"

#include "base/json/json_util.h"
#include "core/components/button/button_theme.h"
#include "core/components/toggle/toggle_theme.h"
#include "core/components_ng/base/common_configuration.h"
#include "core/components_ng/base/inspector_filter.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/event/event_hub.h"
#include "core/components_ng/event/focus_hub.h"
#include "core/components_ng/event/input_event_hub.h"
#include "core/components_ng/layout/layout_property.h"
#include "core/components_ng/manager/focus/focus_manager.h"
#include "core/components_ng/pattern/button/button_event_hub.h"
#include "core/components_ng/pattern/button/toggle_button_event_hub.h"
#include "core/components_ng/pattern/button/toggle_button_model_ng.h"
#include "core/components_ng/pattern/button/toggle_button_paint_property.h"
#include "core/components_ng/pattern/button/toggle_button_pattern.h"
#include "core/components_ng/pattern/text/text_layout_property.h"
#include "core/components_ng/pattern/text/text_pattern.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/event/touch_event.h"
#include "core/pipeline/base/element_register.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS::Ace::NG {
namespace {
constexpr bool IS_ON = true;
constexpr bool IS_OFF = false;
const char TOGGLE_ETS_TAG[] = "Toggle";
constexpr int32_t DEFAULT_BUILDER_ID = -1;
constexpr int32_t THEME_SCOPE_ID_DEFAULT = 0;
constexpr int32_t TOUCH_POINT_ID = 0;
constexpr float SCALE_HOVER_OR_FOCUS_NON_UNIT = 1.05;
const std::string TOUCH_EVENT_TAG = "touch";
// Injection command JSON payloads consumed by ParseCommand / OnInjectionEvent.
const std::string INJECTION_CMD_ON = R"({"cmd":"onToggleChange","params":{"isOn":true}})";
const std::string INJECTION_CMD_OFF = R"({"cmd":"onToggleChange","params":{"isOn":false}})";
const std::string INJECTION_CMD_BAD_FORMAT = "invalid_json_string";
const std::string INJECTION_CMD_WRONG_CMD = R"({"cmd":"other","params":{"isOn":true}})";
const std::string INJECTION_CMD_MISSING_PARAM = R"({"cmd":"onToggleChange","params":{}})";
const std::string INJECTION_CMD_NON_BOOL = R"({"cmd":"onToggleChange","params":{"isOn":"yes"}})";

// Install a ToggleTheme-backed theme manager on the current mock pipeline so that
// ToggleButtonPattern::InitParameters can resolve a non-null toggleTheme_.
RefPtr<ToggleTheme> InstallToggleTheme()
{
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    auto toggleTheme = AceType::MakeRefPtr<ToggleTheme>();
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(toggleTheme));
    EXPECT_CALL(*themeManager, GetTheme(_, _)).WillRepeatedly(Return(toggleTheme));
    return toggleTheme;
}

// Attach a text child frame node so methods that read host->GetFirstChild() exercise their full path.
void AttachTextChild(const RefPtr<FrameNode>& host)
{
    CHECK_NULL_VOID(host);
    auto textChild =
        FrameNode::GetOrCreateFrameNode("childTag", ElementRegister::GetInstance()->MakeUniqueId(),
            []() { return AceType::MakeRefPtr<TextPattern>(); });
    host->children_.emplace_back(textChild);
}
} // namespace

class ToggleButtonPatternTestNg : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    static RefPtr<FrameNode> CreateToggleButton(bool isOn);
    static RefPtr<ToggleButtonPattern> GetPattern(const RefPtr<FrameNode>& node);
};

void ToggleButtonPatternTestNg::SetUpTestCase()
{
    MockPipelineContext::SetUp();
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(AceType::MakeRefPtr<ButtonTheme>()));
    EXPECT_CALL(*themeManager, GetTheme(_, _)).WillRepeatedly(Return(AceType::MakeRefPtr<ButtonTheme>()));
}

void ToggleButtonPatternTestNg::TearDownTestCase()
{
    MockPipelineContext::TearDown();
}

RefPtr<FrameNode> ToggleButtonPatternTestNg::CreateToggleButton(bool isOn)
{
    ToggleButtonModelNG toggleButtonModelNG;
    toggleButtonModelNG.Create(TOGGLE_ETS_TAG);
    toggleButtonModelNG.SetIsOn(isOn);
    auto frameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    if (frameNode) {
        frameNode->MarkModifyDone();
    }
    return frameNode;
}

RefPtr<ToggleButtonPattern> ToggleButtonPatternTestNg::GetPattern(const RefPtr<FrameNode>& node)
{
    CHECK_NULL_RETURN(node, nullptr);
    return AceType::DynamicCast<ToggleButtonPattern>(node->GetPattern());
}

// ============================================================
// MarkIsSelected tests
// ============================================================

/**
 * @tc.name: ToggleButton_MarkIsSelected_On
 * @tc.desc: Test MarkIsSelected flips the toggle on when it is currently off.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleButtonPatternTestNg, ToggleButton_MarkIsSelected_On, TestSize.Level1)
{
    auto node = CreateToggleButton(IS_OFF);
    ASSERT_NE(node, nullptr);
    auto pattern = GetPattern(node);
    ASSERT_NE(pattern, nullptr);
    EXPECT_FALSE(pattern->isOn_.value_or(IS_OFF));
    // Selecting when currently off flips the state to on.
    pattern->MarkIsSelected(IS_ON);
    EXPECT_TRUE(pattern->isOn_.value_or(IS_OFF));
}

/**
 * @tc.name: ToggleButton_MarkIsSelected_Off
 * @tc.desc: Test MarkIsSelected flips the toggle off when it is currently on.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleButtonPatternTestNg, ToggleButton_MarkIsSelected_Off, TestSize.Level1)
{
    auto node = CreateToggleButton(IS_ON);
    ASSERT_NE(node, nullptr);
    auto pattern = GetPattern(node);
    ASSERT_NE(pattern, nullptr);
    EXPECT_TRUE(pattern->isOn_.value_or(IS_OFF));
    // Clearing selection when currently on flips the state to off.
    pattern->MarkIsSelected(IS_OFF);
    EXPECT_FALSE(pattern->isOn_.value_or(IS_OFF));
}

/**
 * @tc.name: ToggleButton_MarkIsSelected_SameState_NoEvent
 * @tc.desc: Test MarkIsSelected takes the early-return branch and does not fire the change event
 *           when the target state equals the current state.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleButtonPatternTestNg, ToggleButton_MarkIsSelected_SameState_NoEvent, TestSize.Level1)
{
    auto node = CreateToggleButton(IS_ON);
    ASSERT_NE(node, nullptr);
    auto pattern = GetPattern(node);
    ASSERT_NE(pattern, nullptr);
    bool eventFired = false;
    auto eventHub = pattern->GetEventHub<ToggleButtonEventHub>();
    ASSERT_NE(eventHub, nullptr);
    eventHub->SetOnChange([&eventFired](const bool) { eventFired = true; });
    // Already on; selecting on again hits the early-return branch, so no event is fired.
    pattern->MarkIsSelected(IS_ON);
    EXPECT_FALSE(eventFired);
    // A real state change (on -> off) does fire the registered change event.
    pattern->MarkIsSelected(IS_OFF);
    EXPECT_TRUE(eventFired);
}

// ============================================================
// SetButtonPress tests
// ============================================================

/**
 * @tc.name: ToggleButton_SetButtonPress_Enabled
 * @tc.desc: Test SetButtonPress updates the IsOn paint property when the toggle is enabled.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleButtonPatternTestNg, ToggleButton_SetButtonPress_Enabled, TestSize.Level1)
{
    auto node = CreateToggleButton(IS_OFF);
    ASSERT_NE(node, nullptr);
    auto pattern = GetPattern(node);
    ASSERT_NE(pattern, nullptr);
    auto paintProperty = node->GetPaintProperty<ToggleButtonPaintProperty>();
    ASSERT_NE(paintProperty, nullptr);
    EXPECT_FALSE(paintProperty->GetIsOnValue(IS_OFF));
    pattern->SetButtonPress(IS_ON);
    EXPECT_TRUE(paintProperty->GetIsOnValue(IS_OFF));
}

/**
 * @tc.name: ToggleButton_SetButtonPress_Disabled
 * @tc.desc: Test SetButtonPress returns early and leaves IsOn unchanged when the toggle is disabled.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleButtonPatternTestNg, ToggleButton_SetButtonPress_Disabled, TestSize.Level1)
{
    auto node = CreateToggleButton(IS_OFF);
    ASSERT_NE(node, nullptr);
    auto pattern = GetPattern(node);
    ASSERT_NE(pattern, nullptr);
    auto eventHub = node->GetEventHub<EventHub>();
    ASSERT_NE(eventHub, nullptr);
    eventHub->SetEnabled(IS_OFF);
    auto paintProperty = node->GetPaintProperty<ToggleButtonPaintProperty>();
    ASSERT_NE(paintProperty, nullptr);
    // Disabled: the early-return branch keeps IsOn unchanged.
    pattern->SetButtonPress(IS_ON);
    EXPECT_FALSE(paintProperty->GetIsOnValue(IS_OFF));
}

// ============================================================
// OnInjectionEvent / ParseCommand tests
// ============================================================

/**
 * @tc.name: ToggleButton_OnInjectionEvent_Valid
 * @tc.desc: Test OnInjectionEvent applies a well-formed on command and reports success.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleButtonPatternTestNg, ToggleButton_OnInjectionEvent_Valid, TestSize.Level1)
{
    auto node = CreateToggleButton(IS_OFF);
    ASSERT_NE(node, nullptr);
    auto pattern = GetPattern(node);
    ASSERT_NE(pattern, nullptr);
    EXPECT_EQ(pattern->OnInjectionEvent(INJECTION_CMD_ON), RET_SUCCESS);
    auto paintProperty = node->GetPaintProperty<ToggleButtonPaintProperty>();
    ASSERT_NE(paintProperty, nullptr);
    EXPECT_TRUE(paintProperty->GetIsOnValue(IS_OFF));
}

/**
 * @tc.name: ToggleButton_OnInjectionEvent_BadFormat
 * @tc.desc: Test OnInjectionEvent rejects a malformed command with RET_FAILED.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleButtonPatternTestNg, ToggleButton_OnInjectionEvent_BadFormat, TestSize.Level1)
{
    auto node = CreateToggleButton(IS_OFF);
    ASSERT_NE(node, nullptr);
    auto pattern = GetPattern(node);
    ASSERT_NE(pattern, nullptr);
    EXPECT_EQ(pattern->OnInjectionEvent(INJECTION_CMD_BAD_FORMAT), RET_FAILED);
}

/**
 * @tc.name: ToggleButton_OnInjectionEvent_WrongCmd
 * @tc.desc: Test OnInjectionEvent rejects a command whose cmd field is not onToggleChange.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleButtonPatternTestNg, ToggleButton_OnInjectionEvent_WrongCmd, TestSize.Level1)
{
    auto node = CreateToggleButton(IS_OFF);
    ASSERT_NE(node, nullptr);
    auto pattern = GetPattern(node);
    ASSERT_NE(pattern, nullptr);
    EXPECT_EQ(pattern->OnInjectionEvent(INJECTION_CMD_WRONG_CMD), RET_FAILED);
}

/**
 * @tc.name: ToggleButton_OnInjectionEvent_ReadOnly
 * @tc.desc: Test OnInjectionEvent rejects the command when the toggle is disabled (read-only).
 * @tc.type: FUNC
 */
HWTEST_F(ToggleButtonPatternTestNg, ToggleButton_OnInjectionEvent_ReadOnly, TestSize.Level1)
{
    auto node = CreateToggleButton(IS_OFF);
    ASSERT_NE(node, nullptr);
    auto pattern = GetPattern(node);
    ASSERT_NE(pattern, nullptr);
    auto eventHub = node->GetEventHub<EventHub>();
    ASSERT_NE(eventHub, nullptr);
    eventHub->SetEnabled(IS_OFF);
    EXPECT_EQ(pattern->OnInjectionEvent(INJECTION_CMD_ON), RET_FAILED);
}

/**
 * @tc.name: ToggleButton_ParseCommand_Valid
 * @tc.desc: Test ParseCommand parses a well-formed command and extracts the isOn flag.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleButtonPatternTestNg, ToggleButton_ParseCommand_Valid, TestSize.Level1)
{
    auto node = CreateToggleButton(IS_OFF);
    ASSERT_NE(node, nullptr);
    auto pattern = GetPattern(node);
    ASSERT_NE(pattern, nullptr);
    bool isOn = IS_OFF;
    EXPECT_TRUE(pattern->ParseCommand(INJECTION_CMD_ON, isOn));
    EXPECT_TRUE(isOn);
}

/**
 * @tc.name: ToggleButton_ParseCommand_Invalid
 * @tc.desc: Test ParseCommand rejects malformed and incomplete commands.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleButtonPatternTestNg, ToggleButton_ParseCommand_Invalid, TestSize.Level1)
{
    auto node = CreateToggleButton(IS_OFF);
    ASSERT_NE(node, nullptr);
    auto pattern = GetPattern(node);
    ASSERT_NE(pattern, nullptr);
    bool isOn = IS_OFF;
    // Malformed JSON, wrong cmd, missing param, and non-bool param are all rejected.
    EXPECT_FALSE(pattern->ParseCommand(INJECTION_CMD_BAD_FORMAT, isOn));
    EXPECT_FALSE(pattern->ParseCommand(INJECTION_CMD_WRONG_CMD, isOn));
    EXPECT_FALSE(pattern->ParseCommand(INJECTION_CMD_MISSING_PARAM, isOn));
    EXPECT_FALSE(pattern->ParseCommand(INJECTION_CMD_NON_BOOL, isOn));
}

// ============================================================
// ProvideRestoreInfo / OnRestoreInfo tests
// ============================================================

/**
 * @tc.name: ToggleButton_ProvideRestoreInfo_On
 * @tc.desc: Test ProvideRestoreInfo serializes the on state as {"IsOn":true}.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleButtonPatternTestNg, ToggleButton_ProvideRestoreInfo_On, TestSize.Level1)
{
    auto node = CreateToggleButton(IS_ON);
    ASSERT_NE(node, nullptr);
    auto pattern = GetPattern(node);
    ASSERT_NE(pattern, nullptr);
    EXPECT_EQ(pattern->ProvideRestoreInfo(), R"({"IsOn":true})");
}

/**
 * @tc.name: ToggleButton_ProvideRestoreInfo_Off
 * @tc.desc: Test ProvideRestoreInfo serializes the off state as {"IsOn":false}.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleButtonPatternTestNg, ToggleButton_ProvideRestoreInfo_Off, TestSize.Level1)
{
    auto node = CreateToggleButton(IS_OFF);
    ASSERT_NE(node, nullptr);
    auto pattern = GetPattern(node);
    ASSERT_NE(pattern, nullptr);
    EXPECT_EQ(pattern->ProvideRestoreInfo(), R"({"IsOn":false})");
}

/**
 * @tc.name: ToggleButton_OnRestoreInfo_RoundTrip
 * @tc.desc: Test OnRestoreInfo applies a valid restore info and flips IsOn to true.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleButtonPatternTestNg, ToggleButton_OnRestoreInfo_RoundTrip, TestSize.Level1)
{
    auto node = CreateToggleButton(IS_OFF);
    ASSERT_NE(node, nullptr);
    auto pattern = GetPattern(node);
    ASSERT_NE(pattern, nullptr);
    pattern->OnRestoreInfo(R"({"IsOn":true})");
    auto paintProperty = node->GetPaintProperty<ToggleButtonPaintProperty>();
    ASSERT_NE(paintProperty, nullptr);
    EXPECT_TRUE(paintProperty->GetIsOnValue(IS_OFF));
}

/**
 * @tc.name: ToggleButton_OnRestoreInfo_InvalidJson
 * @tc.desc: Test OnRestoreInfo ignores malformed restore info and leaves IsOn unchanged.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleButtonPatternTestNg, ToggleButton_OnRestoreInfo_InvalidJson, TestSize.Level1)
{
    auto node = CreateToggleButton(IS_OFF);
    ASSERT_NE(node, nullptr);
    auto pattern = GetPattern(node);
    ASSERT_NE(pattern, nullptr);
    auto paintProperty = node->GetPaintProperty<ToggleButtonPaintProperty>();
    ASSERT_NE(paintProperty, nullptr);
    pattern->OnRestoreInfo("invalid");
    EXPECT_FALSE(paintProperty->GetIsOnValue(IS_OFF));
}

// ============================================================
// ToTreeJson / GetBuilderId tests
// ============================================================

/**
 * @tc.name: ToggleButton_ToTreeJson_Checked
 * @tc.desc: Test ToTreeJson emits the checked flag as "true" when the toggle is on.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleButtonPatternTestNg, ToggleButton_ToTreeJson_Checked, TestSize.Level1)
{
    auto node = CreateToggleButton(IS_ON);
    ASSERT_NE(node, nullptr);
    auto pattern = GetPattern(node);
    ASSERT_NE(pattern, nullptr);
    std::unique_ptr<JsonValue> json = JsonUtil::Create(true);
    ASSERT_NE(json, nullptr);
    InspectorConfig config;
    pattern->ToTreeJson(json, config);
    EXPECT_EQ(json->GetValue(TreeKey::CHECKED)->GetString(), "true");
}

/**
 * @tc.name: ToggleButton_ToTreeJson_Unchecked
 * @tc.desc: Test ToTreeJson emits the checked flag as "false" when the on-state is not initialized.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleButtonPatternTestNg, ToggleButton_ToTreeJson_Unchecked, TestSize.Level1)
{
    auto node = CreateToggleButton(IS_OFF);
    ASSERT_NE(node, nullptr);
    auto pattern = GetPattern(node);
    ASSERT_NE(pattern, nullptr);
    // ToTreeJson reads the optional isOn_ in a boolean context, so it reports "false" only when
    // the on-state is disengaged. Reset it to exercise that branch.
    pattern->isOn_ = std::nullopt;
    std::unique_ptr<JsonValue> json = JsonUtil::Create(true);
    ASSERT_NE(json, nullptr);
    InspectorConfig config;
    pattern->ToTreeJson(json, config);
    EXPECT_EQ(json->GetValue(TreeKey::CHECKED)->GetString(), "false");
}

/**
 * @tc.name: ToggleButton_GetBuilderId_Default
 * @tc.desc: Test GetBuilderId returns the default invalid id when no content modifier is attached.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleButtonPatternTestNg, ToggleButton_GetBuilderId_Default, TestSize.Level1)
{
    auto node = CreateToggleButton(IS_ON);
    ASSERT_NE(node, nullptr);
    auto pattern = GetPattern(node);
    ASSERT_NE(pattern, nullptr);
    // No builder has been attached, so the builder id stays at its default value.
    EXPECT_EQ(pattern->GetBuilderId(), DEFAULT_BUILDER_ID);
}

// ============================================================
// HandleHoverEvent tests
// ============================================================

/**
 * @tc.name: ToggleButton_HandleHoverEvent_HoverOn
 * @tc.desc: Test HandleHoverEvent with hover on and a non-blocking hover effect applies the focus scale.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleButtonPatternTestNg, ToggleButton_HandleHoverEvent_HoverOn, TestSize.Level1)
{
    auto node = CreateToggleButton(IS_ON);
    ASSERT_NE(node, nullptr);
    AttachTextChild(node);
    auto pattern = GetPattern(node);
    ASSERT_NE(pattern, nullptr);
    auto toggleTheme = InstallToggleTheme();
    toggleTheme->scaleHoverOrFocus_ = SCALE_HOVER_OR_FOCUS_NON_UNIT;
    pattern->OnModifyDone();

    auto inputEventHub = node->GetOrCreateInputEventHub();
    ASSERT_NE(inputEventHub, nullptr);
    // AUTO avoids the NONE/SCALE early-return branch so the hover body executes.
    inputEventHub->SetHoverEffect(HoverEffectType::AUTO);
    pattern->isPress_ = IS_OFF;

    pattern->HandleHoverEvent(IS_ON);
    EXPECT_TRUE(pattern->isHover_);
    EXPECT_TRUE(pattern->isScale_);
}

/**
 * @tc.name: ToggleButton_HandleHoverEvent_HoverOffResetsScale
 * @tc.desc: Test HandleHoverEvent with hover off resets a previously applied scale.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleButtonPatternTestNg, ToggleButton_HandleHoverEvent_HoverOffResetsScale, TestSize.Level1)
{
    auto node = CreateToggleButton(IS_ON);
    ASSERT_NE(node, nullptr);
    AttachTextChild(node);
    auto pattern = GetPattern(node);
    ASSERT_NE(pattern, nullptr);
    auto toggleTheme = InstallToggleTheme();
    toggleTheme->scaleHoverOrFocus_ = SCALE_HOVER_OR_FOCUS_NON_UNIT;
    pattern->OnModifyDone();

    auto inputEventHub = node->GetOrCreateInputEventHub();
    ASSERT_NE(inputEventHub, nullptr);
    inputEventHub->SetHoverEffect(HoverEffectType::AUTO);
    // Pre-arm the scale so the hover-off branch can reset it.
    pattern->isScale_ = IS_ON;
    pattern->isPress_ = IS_OFF;

    pattern->HandleHoverEvent(IS_OFF);
    EXPECT_FALSE(pattern->isHover_);
    EXPECT_FALSE(pattern->isScale_);
}

/**
 * @tc.name: ToggleButton_HandleHoverEvent_NoneEffectEarlyReturn
 * @tc.desc: Test HandleHoverEvent takes the early-return branch when the hover effect is NONE.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleButtonPatternTestNg, ToggleButton_HandleHoverEvent_NoneEffectEarlyReturn, TestSize.Level1)
{
    auto node = CreateToggleButton(IS_ON);
    ASSERT_NE(node, nullptr);
    auto pattern = GetPattern(node);
    ASSERT_NE(pattern, nullptr);
    InstallToggleTheme();
    pattern->OnModifyDone();

    auto inputEventHub = node->GetOrCreateInputEventHub();
    ASSERT_NE(inputEventHub, nullptr);
    inputEventHub->SetHoverEffect(HoverEffectType::NONE);
    pattern->isScale_ = IS_OFF;

    // NONE hover effect returns early: isHover_ is still recorded, but scale stays untouched.
    pattern->HandleHoverEvent(IS_ON);
    EXPECT_TRUE(pattern->isHover_);
    EXPECT_FALSE(pattern->isScale_);
}

// ============================================================
// HandleBorderAndShadow tests
// ============================================================

/**
 * @tc.name: ToggleButton_HandleBorderAndShadow_SetsBorderColor
 * @tc.desc: Test HandleBorderAndShadow enters the !HasBorderColor branch and applies the checked border color.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleButtonPatternTestNg, ToggleButton_HandleBorderAndShadow_SetsBorderColor, TestSize.Level1)
{
    auto node = CreateToggleButton(IS_ON);
    ASSERT_NE(node, nullptr);
    auto pattern = GetPattern(node);
    ASSERT_NE(pattern, nullptr);
    auto toggleTheme = InstallToggleTheme();
    pattern->InitParameters();

    auto renderContext = node->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    auto layoutProperty = node->GetLayoutProperty();
    ASSERT_NE(layoutProperty, nullptr);
    // Prior tests in this fixture install a live ToggleTheme on the shared pipeline, so the
    // initial MarkModifyDone inside CreateToggleButton already ran HandleBorderAndShadow and
    // populated border state. Reset both the render-context border and the layout border-width
    // property so the !HasBorderColor branch is reachable on this call.
    renderContext->ResetBorderColor();
    renderContext->ResetBorderWidth();
    layoutProperty->borderWidth_.reset();
    EXPECT_FALSE(renderContext->HasBorderColor());

    pattern->HandleBorderAndShadow();
    EXPECT_TRUE(renderContext->HasBorderColor());
    auto expectedColor = toggleTheme->GetBorderColorChecked();
    EXPECT_EQ(renderContext->GetBorderColor()->leftColor.value_or(Color::TRANSPARENT), expectedColor);
}

/**
 * @tc.name: ToggleButton_HandleBorderAndShadow_SkipsBorderColorWhenPresent
 * @tc.desc: Test HandleBorderAndShadow skips the !HasBorderColor branch when a border color already exists.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleButtonPatternTestNg, ToggleButton_HandleBorderAndShadow_SkipsBorderColorWhenPresent, TestSize.Level1)
{
    auto node = CreateToggleButton(IS_OFF);
    ASSERT_NE(node, nullptr);
    auto pattern = GetPattern(node);
    ASSERT_NE(pattern, nullptr);
    InstallToggleTheme();
    pattern->InitParameters();

    auto renderContext = node->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    auto layoutProperty = node->GetLayoutProperty();
    ASSERT_NE(layoutProperty, nullptr);
    // Reset border state so the outer !GetBorderWidthProperty() block is entered, then pre-set a
    // distinct border color so the inner !HasBorderColor() condition is false and SetColor is skipped.
    renderContext->ResetBorderColor();
    renderContext->ResetBorderWidth();
    layoutProperty->borderWidth_.reset();
    const Color presetValue = Color(0xFF00FF00);
    BorderColorProperty presetColor;
    presetColor.SetColor(presetValue);
    renderContext->UpdateBorderColor(presetColor);
    ASSERT_TRUE(renderContext->HasBorderColor());

    pattern->HandleBorderAndShadow();
    // Pre-set color is preserved because the !HasBorderColor branch was skipped.
    EXPECT_EQ(renderContext->GetBorderColor()->leftColor.value_or(Color::TRANSPARENT), presetValue);
}

// ============================================================
// AddIsFocusActiveUpdateEvent tests
// ============================================================

/**
 * @tc.name: ToggleButton_AddIsFocusActiveUpdateEvent_CreatesCallback
 * @tc.desc: Test AddIsFocusActiveUpdateEvent creates the focus-active callback when it is not yet set.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleButtonPatternTestNg, ToggleButton_AddIsFocusActiveUpdateEvent_CreatesCallback, TestSize.Level1)
{
    auto node = CreateToggleButton(IS_ON);
    ASSERT_NE(node, nullptr);
    AttachTextChild(node);
    auto pattern = GetPattern(node);
    ASSERT_NE(pattern, nullptr);
    InstallToggleTheme();
    pattern->OnModifyDone();

    EXPECT_FALSE(pattern->isFocusActiveUpdateEvent_);
    // First call enters the !isFocusActiveUpdateEvent_ branch and builds the callback.
    pattern->AddIsFocusActiveUpdateEvent();
    EXPECT_TRUE(pattern->isFocusActiveUpdateEvent_);
}

/**
 * @tc.name: ToggleButton_AddIsFocusActiveUpdateEvent_ReusesExistingCallback
 * @tc.desc: Test AddIsFocusActiveUpdateEvent reuses the existing callback instead of rebuilding it.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleButtonPatternTestNg, ToggleButton_AddIsFocusActiveUpdateEvent_ReusesExistingCallback, TestSize.Level1)
{
    auto node = CreateToggleButton(IS_ON);
    ASSERT_NE(node, nullptr);
    AttachTextChild(node);
    auto pattern = GetPattern(node);
    ASSERT_NE(pattern, nullptr);
    InstallToggleTheme();
    pattern->OnModifyDone();

    pattern->AddIsFocusActiveUpdateEvent();
    ASSERT_TRUE(pattern->isFocusActiveUpdateEvent_);
    // Capture the stored callback so we can tell the second call did not clear it.
    auto firstCallback = pattern->isFocusActiveUpdateEvent_;
    ASSERT_TRUE(firstCallback);

    // Second call takes the early-return branch (isFocusActiveUpdateEvent_ already set):
    // the callback is reused rather than rebuilt, and registration is re-invoked safely.
    EXPECT_NO_FATAL_FAILURE(pattern->AddIsFocusActiveUpdateEvent());
    EXPECT_TRUE(pattern->isFocusActiveUpdateEvent_);
}

// ============================================================
// UpdateTexOverflow tests
// ============================================================

/**
 * @tc.name: ToggleButton_UpdateTexOverflow_FadeOutAppliesMarquee
 * @tc.desc: Test UpdateTexOverflow applies MARQUEE overflow when text fade-out is enabled.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleButtonPatternTestNg, ToggleButton_UpdateTexOverflow_FadeOutAppliesMarquee, TestSize.Level1)
{
    auto node = CreateToggleButton(IS_ON);
    ASSERT_NE(node, nullptr);
    auto pattern = GetPattern(node);
    ASSERT_NE(pattern, nullptr);
    InstallToggleTheme();
    pattern->OnModifyDone();

    auto textNode = FrameNode::GetOrCreateFrameNode(V2::TEXT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextPattern>(); });
    ASSERT_NE(textNode, nullptr);
    auto textLayoutProperty = textNode->GetLayoutProperty<TextLayoutProperty>();
    ASSERT_NE(textLayoutProperty, nullptr);

    pattern->isTextFadeOut_ = IS_ON;
    pattern->UpdateTexOverflow(IS_ON, textLayoutProperty);
    EXPECT_EQ(textLayoutProperty->GetTextOverflow().value_or(TextOverflow::CLIP), TextOverflow::MARQUEE);
    EXPECT_TRUE(textLayoutProperty->GetTextMarqueeFadeoutValue(IS_OFF));
}

/**
 * @tc.name: ToggleButton_UpdateTexOverflow_FadeOutDisabledSkips
 * @tc.desc: Test UpdateTexOverflow leaves overflow untouched when text fade-out is disabled.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleButtonPatternTestNg, ToggleButton_UpdateTexOverflow_FadeOutDisabledSkips, TestSize.Level1)
{
    auto node = CreateToggleButton(IS_ON);
    ASSERT_NE(node, nullptr);
    auto pattern = GetPattern(node);
    ASSERT_NE(pattern, nullptr);
    InstallToggleTheme();
    pattern->OnModifyDone();

    auto textNode = FrameNode::GetOrCreateFrameNode(V2::TEXT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextPattern>(); });
    ASSERT_NE(textNode, nullptr);
    auto textLayoutProperty = textNode->GetLayoutProperty<TextLayoutProperty>();
    ASSERT_NE(textLayoutProperty, nullptr);

    pattern->isTextFadeOut_ = IS_OFF;
    pattern->UpdateTexOverflow(IS_ON, textLayoutProperty);
    // MARQUEE should not be applied when fade-out is disabled.
    EXPECT_FALSE(textLayoutProperty->GetTextOverflow().has_value());
}

/**
 * @tc.name: ToggleButton_UpdateTexOverflow_NullPropertyNoCrash
 * @tc.desc: Test UpdateTexOverflow is safe when the text layout property ref is null.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleButtonPatternTestNg, ToggleButton_UpdateTexOverflow_NullPropertyNoCrash, TestSize.Level1)
{
    auto node = CreateToggleButton(IS_ON);
    ASSERT_NE(node, nullptr);
    auto pattern = GetPattern(node);
    ASSERT_NE(pattern, nullptr);
    InstallToggleTheme();
    pattern->OnModifyDone();

    RefPtr<TextLayoutProperty> nullProperty;
    pattern->isTextFadeOut_ = IS_ON;
    // Null property hits the falsy short-circuit of the guard; no crash, no work done.
    EXPECT_NO_FATAL_FAILURE(pattern->UpdateTexOverflow(IS_ON, nullProperty));
}

// ============================================================
// HandleFocusStyle tests
// ============================================================

/**
 * @tc.name: ToggleButton_HandleFocusStyle_FocusAndBlurCallbacks
 * @tc.desc: Test HandleFocusStyle registers focus/blur callbacks that toggle the focus state.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleButtonPatternTestNg, ToggleButton_HandleFocusStyle_FocusAndBlurCallbacks, TestSize.Level1)
{
    auto node = CreateToggleButton(IS_ON);
    ASSERT_NE(node, nullptr);
    AttachTextChild(node);
    auto pattern = GetPattern(node);
    ASSERT_NE(pattern, nullptr);
    auto toggleTheme = InstallToggleTheme();
    toggleTheme->scaleHoverOrFocus_ = SCALE_HOVER_OR_FOCUS_NON_UNIT;
    pattern->OnModifyDone();

    pattern->HandleFocusStyle();
    auto focusHub = node->GetOrCreateFocusHub();
    ASSERT_NE(focusHub, nullptr);
    ASSERT_TRUE(focusHub->onFocusInternal_);
    ASSERT_TRUE(focusHub->onBlurInternal_);

    // Make focus active so the focus callback's UpdateButtonStyle path is reachable.
    auto pipeline = node->GetContextRefPtr();
    ASSERT_NE(pipeline, nullptr);
    auto focusManager = pipeline->GetOrCreateFocusManager();
    ASSERT_NE(focusManager, nullptr);
    focusManager->SetIsFocusActive(IS_ON);

    // Invoking the registered focus lambda should mark the pattern focused.
    focusHub->onFocusInternal_(FocusReason::DEFAULT);
    EXPECT_TRUE(pattern->isFocus_);

    // Invoking the registered blur lambda should clear the focus state.
    focusHub->onBlurInternal_();
    EXPECT_FALSE(pattern->isFocus_);
}

// ============================================================
// HandleFocusEvent tests
// ============================================================

/**
 * @tc.name: ToggleButton_HandleFocusEvent_FocusActive
 * @tc.desc: Test HandleFocusEvent sets focus when pipeline focus is active.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleButtonPatternTestNg, ToggleButton_HandleFocusEvent_FocusActive, TestSize.Level1)
{
    auto node = CreateToggleButton(IS_ON);
    ASSERT_NE(node, nullptr);
    AttachTextChild(node);
    auto pattern = GetPattern(node);
    ASSERT_NE(pattern, nullptr);
    auto toggleTheme = InstallToggleTheme();
    toggleTheme->scaleHoverOrFocus_ = SCALE_HOVER_OR_FOCUS_NON_UNIT;
    pattern->OnModifyDone();

    auto pipeline = node->GetContextRefPtr();
    ASSERT_NE(pipeline, nullptr);
    auto focusManager = pipeline->GetOrCreateFocusManager();
    ASSERT_NE(focusManager, nullptr);
    focusManager->SetIsFocusActive(IS_ON);

    pattern->isFocus_ = IS_OFF;
    // Focus active enters the if branch and applies focus styling.
    pattern->HandleFocusEvent();
    EXPECT_TRUE(pattern->isFocus_);
}

/**
 * @tc.name: ToggleButton_HandleFocusEvent_FocusInactive
 * @tc.desc: Test HandleFocusEvent skips focus styling when pipeline focus is inactive.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleButtonPatternTestNg, ToggleButton_HandleFocusEvent_FocusInactive, TestSize.Level1)
{
    auto node = CreateToggleButton(IS_ON);
    ASSERT_NE(node, nullptr);
    auto pattern = GetPattern(node);
    ASSERT_NE(pattern, nullptr);
    InstallToggleTheme();
    pattern->OnModifyDone();

    auto pipeline = node->GetContextRefPtr();
    ASSERT_NE(pipeline, nullptr);
    auto focusManager = pipeline->GetOrCreateFocusManager();
    ASSERT_NE(focusManager, nullptr);
    focusManager->SetIsFocusActive(IS_OFF);

    pattern->isFocus_ = IS_OFF;
    // Focus inactive skips the if branch; isFocus_ stays unchanged.
    pattern->HandleFocusEvent();
    EXPECT_FALSE(pattern->isFocus_);
    // The focus-active callback is still registered on the pipeline.
    EXPECT_TRUE(pattern->isFocusActiveUpdateEvent_);
}

// ============================================================
// InitTouchEvent tests
// ============================================================

/**
 * @tc.name: ToggleButton_InitTouchEvent_RegistersListener
 * @tc.desc: Test InitTouchEvent registers the touch listener on the gesture hub.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleButtonPatternTestNg, ToggleButton_InitTouchEvent_RegistersListener, TestSize.Level1)
{
    auto node = CreateToggleButton(IS_ON);
    ASSERT_NE(node, nullptr);
    auto pattern = GetPattern(node);
    ASSERT_NE(pattern, nullptr);
    InstallToggleTheme();
    pattern->OnModifyDone();

    pattern->touchListener_ = nullptr;
    pattern->InitTouchEvent();
    EXPECT_NE(pattern->touchListener_, nullptr);
}

/**
 * @tc.name: ToggleButton_InitTouchEvent_SkipsWhenListenerExists
 * @tc.desc: Test InitTouchEvent takes the early-return branch when a listener already exists.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleButtonPatternTestNg, ToggleButton_InitTouchEvent_SkipsWhenListenerExists, TestSize.Level1)
{
    auto node = CreateToggleButton(IS_ON);
    ASSERT_NE(node, nullptr);
    auto pattern = GetPattern(node);
    ASSERT_NE(pattern, nullptr);
    InstallToggleTheme();
    pattern->OnModifyDone();

    auto sentinel = AceType::MakeRefPtr<TouchEventImpl>([](const TouchEventInfo&) {});
    pattern->touchListener_ = sentinel;
    // Existing listener triggers the early return; the sentinel is preserved.
    pattern->InitTouchEvent();
    EXPECT_EQ(pattern->touchListener_, sentinel);
}

/**
 * @tc.name: ToggleButton_InitTouchEvent_TouchDownCallback
 * @tc.desc: Test the InitTouchEvent closure marks the button pressed on a DOWN touch.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleButtonPatternTestNg, ToggleButton_InitTouchEvent_TouchDownCallback, TestSize.Level1)
{
    auto node = CreateToggleButton(IS_ON);
    ASSERT_NE(node, nullptr);
    auto pattern = GetPattern(node);
    ASSERT_NE(pattern, nullptr);
    InstallToggleTheme();
    pattern->OnModifyDone();

    pattern->touchListener_ = nullptr;
    pattern->InitTouchEvent();
    ASSERT_NE(pattern->touchListener_, nullptr);

    TouchEventInfo touchInfo(TOUCH_EVENT_TAG);
    TouchLocationInfo touchLocationInfo(TOUCH_POINT_ID);
    touchLocationInfo.SetTouchType(TouchType::DOWN);
    touchInfo.AddTouchLocationInfo(std::move(touchLocationInfo));
    auto callback = pattern->touchListener_->GetTouchEventCallback();
    EXPECT_NO_FATAL_FAILURE(callback(touchInfo));
    EXPECT_TRUE(pattern->isPress_);
}

/**
 * @tc.name: ToggleButton_InitTouchEvent_TouchUpCallback
 * @tc.desc: Test the InitTouchEvent closure clears the pressed state on an UP touch.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleButtonPatternTestNg, ToggleButton_InitTouchEvent_TouchUpCallback, TestSize.Level1)
{
    auto node = CreateToggleButton(IS_ON);
    ASSERT_NE(node, nullptr);
    auto pattern = GetPattern(node);
    ASSERT_NE(pattern, nullptr);
    InstallToggleTheme();
    pattern->OnModifyDone();

    pattern->touchListener_ = nullptr;
    pattern->InitTouchEvent();
    ASSERT_NE(pattern->touchListener_, nullptr);

    TouchEventInfo touchInfo(TOUCH_EVENT_TAG);
    TouchLocationInfo touchLocationInfo(TOUCH_POINT_ID);
    touchLocationInfo.SetTouchType(TouchType::UP);
    touchInfo.AddTouchLocationInfo(std::move(touchLocationInfo));
    auto callback = pattern->touchListener_->GetTouchEventCallback();
    pattern->isPress_ = IS_ON;
    EXPECT_NO_FATAL_FAILURE(callback(touchInfo));
    EXPECT_FALSE(pattern->isPress_);
}

/**
 * @tc.name: ToggleButton_InitTouchEvent_EmptyTouchesCallback
 * @tc.desc: Test the InitTouchEvent closure is a no-op when the touch list is empty.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleButtonPatternTestNg, ToggleButton_InitTouchEvent_EmptyTouchesCallback, TestSize.Level1)
{
    auto node = CreateToggleButton(IS_ON);
    ASSERT_NE(node, nullptr);
    auto pattern = GetPattern(node);
    ASSERT_NE(pattern, nullptr);
    InstallToggleTheme();
    pattern->OnModifyDone();

    pattern->touchListener_ = nullptr;
    pattern->InitTouchEvent();
    ASSERT_NE(pattern->touchListener_, nullptr);

    TouchEventInfo touchInfo(TOUCH_EVENT_TAG);
    auto callback = pattern->touchListener_->GetTouchEventCallback();
    pattern->isPress_ = IS_OFF;
    EXPECT_NO_FATAL_FAILURE(callback(touchInfo));
    EXPECT_FALSE(pattern->isPress_);
}

// ============================================================
// OnThemeScopeUpdate tests
// ============================================================

/**
 * @tc.name: ToggleButton_OnThemeScopeUpdate_OnWithoutSelectedColor
 * @tc.desc: Test OnThemeScopeUpdate reapplies the checked color when on and no user selected color is set.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleButtonPatternTestNg, ToggleButton_OnThemeScopeUpdate_OnWithoutSelectedColor, TestSize.Level1)
{
    auto node = CreateToggleButton(IS_ON);
    ASSERT_NE(node, nullptr);
    auto pattern = GetPattern(node);
    ASSERT_NE(pattern, nullptr);
    auto toggleTheme = InstallToggleTheme();
    pattern->OnModifyDone();

    auto renderContext = node->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    renderContext->UpdateBackgroundColor(Color::TRANSPARENT);

    auto result = pattern->OnThemeScopeUpdate(THEME_SCOPE_ID_DEFAULT);
    EXPECT_TRUE(result);
    EXPECT_EQ(renderContext->GetBackgroundColorValue(Color::TRANSPARENT), toggleTheme->GetCheckedColor());
}

/**
 * @tc.name: ToggleButton_OnThemeScopeUpdate_OffWithoutBackgroundColor
 * @tc.desc: Test OnThemeScopeUpdate reapplies the unchecked color when off and no user background color is set.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleButtonPatternTestNg, ToggleButton_OnThemeScopeUpdate_OffWithoutBackgroundColor, TestSize.Level1)
{
    auto node = CreateToggleButton(IS_OFF);
    ASSERT_NE(node, nullptr);
    auto pattern = GetPattern(node);
    ASSERT_NE(pattern, nullptr);
    auto toggleTheme = InstallToggleTheme();
    pattern->OnModifyDone();

    auto renderContext = node->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    renderContext->UpdateBackgroundColor(Color::TRANSPARENT);

    auto result = pattern->OnThemeScopeUpdate(THEME_SCOPE_ID_DEFAULT);
    EXPECT_TRUE(result);
    EXPECT_EQ(renderContext->GetBackgroundColorValue(Color::TRANSPARENT), toggleTheme->GetBackgroundColor());
}

/**
 * @tc.name: ToggleButton_OnThemeScopeUpdate_UserColorSetNoChange
 * @tc.desc: Test OnThemeScopeUpdate reports no change when the user already supplied the colors.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleButtonPatternTestNg, ToggleButton_OnThemeScopeUpdate_UserColorSetNoChange, TestSize.Level1)
{
    auto node = CreateToggleButton(IS_ON);
    ASSERT_NE(node, nullptr);
    auto pattern = GetPattern(node);
    ASSERT_NE(pattern, nullptr);
    InstallToggleTheme();
    pattern->OnModifyDone();

    auto paintProperty = node->GetPaintProperty<ToggleButtonPaintProperty>();
    ASSERT_NE(paintProperty, nullptr);
    // User-provided selected color means the on-branch color reapply is skipped.
    paintProperty->UpdateSelectedColor(Color(0xFF0000FF));

    auto result = pattern->OnThemeScopeUpdate(THEME_SCOPE_ID_DEFAULT);
    EXPECT_FALSE(result);
}

// ============================================================
// FireBuilder tests
// ============================================================

/**
 * @tc.name: ToggleButton_FireBuilder_AttachesContentModifier
 * @tc.desc: Test FireBuilder builds and attaches the content modifier node when a builder is set.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleButtonPatternTestNg, ToggleButton_FireBuilder_AttachesContentModifier, TestSize.Level1)
{
    auto node = CreateToggleButton(IS_ON);
    ASSERT_NE(node, nullptr);
    auto pattern = GetPattern(node);
    ASSERT_NE(pattern, nullptr);
    InstallToggleTheme();
    pattern->OnModifyDone();

    auto builtNode = FrameNode::GetOrCreateFrameNode(V2::TEXT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextPattern>(); });
    ASSERT_NE(builtNode, nullptr);
    SwitchMakeCallback makeFunc = [builtNode](const ToggleConfiguration&) { return builtNode; };
    pattern->SetToggleBuilderFunc(std::move(makeFunc));

    // FireBuilder builds the modifier node, clears the host background, and attaches it.
    pattern->FireBuilder();
    EXPECT_EQ(pattern->contentModifierNode_, builtNode);
    EXPECT_EQ(pattern->nodeId_, builtNode->GetId());
    auto renderContext = node->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    EXPECT_EQ(renderContext->GetBackgroundColorValue(Color::BLACK), Color::TRANSPARENT);
}

/**
 * @tc.name: ToggleButton_FireBuilder_NoBuilderRemovesStaleChild
 * @tc.desc: Test FireBuilder removes a previously attached modifier child when no builder is set.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleButtonPatternTestNg, ToggleButton_FireBuilder_NoBuilderRemovesStaleChild, TestSize.Level1)
{
    auto node = CreateToggleButton(IS_ON);
    ASSERT_NE(node, nullptr);
    auto pattern = GetPattern(node);
    ASSERT_NE(pattern, nullptr);
    InstallToggleTheme();
    pattern->OnModifyDone();

    // Attach a stale child whose id matches the cached builder id to exercise the removal branch.
    auto staleChild = FrameNode::GetOrCreateFrameNode(V2::TEXT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextPattern>(); });
    ASSERT_NE(staleChild, nullptr);
    pattern->nodeId_ = staleChild->GetId();
    node->children_.emplace_back(staleChild);

    // No builder set: FireBuilder takes the !toggleMakeFunc_ branch and removes the stale child.
    EXPECT_NO_FATAL_FAILURE(pattern->FireBuilder());
    EXPECT_FALSE(pattern->toggleMakeFunc_.has_value());
}

/**
 * @tc.name: ToggleButton_FireBuilder_SameNodeSkipsReattach
 * @tc.desc: Test FireBuilder skips re-attaching when the built node equals the cached modifier node.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleButtonPatternTestNg, ToggleButton_FireBuilder_SameNodeSkipsReattach, TestSize.Level1)
{
    auto node = CreateToggleButton(IS_ON);
    ASSERT_NE(node, nullptr);
    auto pattern = GetPattern(node);
    ASSERT_NE(pattern, nullptr);
    InstallToggleTheme();
    pattern->OnModifyDone();

    auto builtNode = FrameNode::GetOrCreateFrameNode(V2::TEXT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextPattern>(); });
    ASSERT_NE(builtNode, nullptr);
    SwitchMakeCallback makeFunc = [builtNode](const ToggleConfiguration&) { return builtNode; };
    pattern->SetToggleBuilderFunc(std::move(makeFunc));
    pattern->FireBuilder();
    ASSERT_EQ(pattern->contentModifierNode_, builtNode);

    // Second call builds the same node and takes the equality early-return branch.
    EXPECT_NO_FATAL_FAILURE(pattern->FireBuilder());
    EXPECT_EQ(pattern->contentModifierNode_, builtNode);
}

// ============================================================
// SetToggleBuilderFunc tests
// ============================================================

/**
 * @tc.name: ToggleButton_SetToggleBuilderFunc_NullptrClearsBuilder
 * @tc.desc: Test SetToggleBuilderFunc with nullptr clears the builder and the cached modifier node.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleButtonPatternTestNg, ToggleButton_SetToggleBuilderFunc_NullptrClearsBuilder, TestSize.Level1)
{
    auto node = CreateToggleButton(IS_ON);
    ASSERT_NE(node, nullptr);
    AttachTextChild(node);
    auto pattern = GetPattern(node);
    ASSERT_NE(pattern, nullptr);
    InstallToggleTheme();
    pattern->OnModifyDone();

    // First attach a real builder so there is state to clear.
    auto builtNode = FrameNode::GetOrCreateFrameNode(V2::TEXT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextPattern>(); });
    ASSERT_NE(builtNode, nullptr);
    SwitchMakeCallback makeFunc = [builtNode](const ToggleConfiguration&) { return builtNode; };
    pattern->SetToggleBuilderFunc(std::move(makeFunc));
    pattern->FireBuilder();
    ASSERT_TRUE(pattern->toggleMakeFunc_.has_value());
    ASSERT_NE(pattern->contentModifierNode_, nullptr);

    // Passing nullptr enters the nullptr branch: builder reset, modifier node cleared.
    pattern->SetToggleBuilderFunc(nullptr);
    EXPECT_FALSE(pattern->toggleMakeFunc_.has_value());
    EXPECT_EQ(pattern->contentModifierNode_, nullptr);
}

/**
 * @tc.name: ToggleButton_SetToggleBuilderFunc_ValidCallbackEngagesBuilder
 * @tc.desc: Test SetToggleBuilderFunc with a valid callback stores the builder for later FireBuilder use.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleButtonPatternTestNg, ToggleButton_SetToggleBuilderFunc_ValidCallbackEngagesBuilder, TestSize.Level1)
{
    auto node = CreateToggleButton(IS_ON);
    ASSERT_NE(node, nullptr);
    auto pattern = GetPattern(node);
    ASSERT_NE(pattern, nullptr);
    InstallToggleTheme();
    pattern->OnModifyDone();

    auto builtNode = FrameNode::GetOrCreateFrameNode(V2::TEXT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextPattern>(); });
    ASSERT_NE(builtNode, nullptr);
    SwitchMakeCallback makeFunc = [builtNode](const ToggleConfiguration&) { return builtNode; };

    pattern->SetToggleBuilderFunc(std::move(makeFunc));
    EXPECT_TRUE(pattern->toggleMakeFunc_.has_value());
    // FireBuilder consumes the engaged builder to attach the modifier node.
    pattern->FireBuilder();
    EXPECT_EQ(pattern->contentModifierNode_, builtNode);
}

// ============================================================
// ToTreeJson tests (branch coverage for the checked flag emission)
// ============================================================

/**
 * @tc.name: ToggleButton_ToTreeJson_CheckedTrue
 * @tc.desc: Test ToTreeJson emits checked as "true" when the toggle is on.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleButtonPatternTestNg, ToggleButton_ToTreeJson_CheckedTrue, TestSize.Level1)
{
    auto node = CreateToggleButton(IS_ON);
    ASSERT_NE(node, nullptr);
    auto pattern = GetPattern(node);
    ASSERT_NE(pattern, nullptr);
    pattern->isOn_ = IS_ON;
    std::unique_ptr<JsonValue> json = JsonUtil::Create(true);
    ASSERT_NE(json, nullptr);
    InspectorConfig config;
    pattern->ToTreeJson(json, config);
    EXPECT_EQ(json->GetValue(TreeKey::CHECKED)->GetString(), "true");
}

/**
 * @tc.name: ToggleButton_ToTreeJson_CheckedFalse
 * @tc.desc: Test ToTreeJson emits checked as "false" when the on-state is disengaged.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleButtonPatternTestNg, ToggleButton_ToTreeJson_CheckedFalse, TestSize.Level1)
{
    auto node = CreateToggleButton(IS_ON);
    ASSERT_NE(node, nullptr);
    auto pattern = GetPattern(node);
    ASSERT_NE(pattern, nullptr);
    // Disengage the optional so the boolean context reports false.
    pattern->isOn_ = std::nullopt;
    std::unique_ptr<JsonValue> json = JsonUtil::Create(true);
    ASSERT_NE(json, nullptr);
    InspectorConfig config;
    pattern->ToTreeJson(json, config);
    EXPECT_EQ(json->GetValue(TreeKey::CHECKED)->GetString(), "false");
}
} // namespace OHOS::Ace::NG

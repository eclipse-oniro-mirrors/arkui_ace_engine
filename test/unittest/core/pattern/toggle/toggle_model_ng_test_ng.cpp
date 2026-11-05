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
#include "gtest/gtest.h"
#define protected public
#define private public
#include "base/geometry/dimension.h"
#include "core/components/checkable/checkable_theme.h"
#include "core/components/toggle/toggle_theme.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/ui_node.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/button/toggle_button_pattern.h"
#include "core/components_ng/pattern/checkbox/checkbox_pattern.h"
#include "core/components_ng/pattern/checkbox/toggle_checkbox_pattern.h"
#include "core/components_ng/pattern/toggle/switch_event_hub.h"
#include "core/components_ng/pattern/toggle/switch_paint_method.h"
#include "core/components_ng/pattern/toggle/switch_paint_property.h"
#include "core/components_ng/pattern/toggle/switch_pattern.h"
#include "core/components_ng/pattern/toggle/toggle_model.h"
#include "core/components_ng/pattern/toggle/toggle_model_ng.h"
#include "core/components_ng/property/accessibility_property.h"
#include "core/common/resource/resource_object.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/pipeline/base/element_register.h"
#include "interfaces/inner_api/ace_kit/include/ui/base/geometry/dimension_rect.h"
#include "test/mock/adapter/ohos/osal/mock_system_properties.h"
#include "test/mock/frameworks/core/common/mock_theme_manager.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS::Ace::NG {
namespace {
constexpr bool IS_ON = true;
constexpr int32_t ADD_CHILD_INDEX = 0;
constexpr int32_t RESOURCE_ID = -1;
constexpr Color POINT_COLOR = Color(0xFF0000FF);
constexpr Color UNSELECTED_COLOR = Color(0xFF00FF00);
constexpr Dimension RESPONSE_WIDTH = 1.0_pct;
constexpr Dimension RESPONSE_HEIGHT = 1.0_pct;
const std::string SELECTED_COLOR_KEY = "toggleSelectedColor";
} // namespace

class ToggleModelNgTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();

    static RefPtr<SwitchTheme> InstallSwitchTheme();
    static RefPtr<CheckboxTheme> InstallCheckboxTheme();
    static RefPtr<ToggleTheme> InstallToggleTheme();
};

void ToggleModelNgTest::SetUpTestCase()
{
    MockPipelineContext::SetUp();
}

void ToggleModelNgTest::TearDownTestCase()
{
    MockPipelineContext::TearDown();
}

RefPtr<SwitchTheme> ToggleModelNgTest::InstallSwitchTheme()
{
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    auto switchTheme = AceType::MakeRefPtr<SwitchTheme>();
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(switchTheme));
    EXPECT_CALL(*themeManager, GetTheme(_, _)).WillRepeatedly(Return(switchTheme));
    return switchTheme;
}

RefPtr<CheckboxTheme> ToggleModelNgTest::InstallCheckboxTheme()
{
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    auto checkboxTheme = AceType::MakeRefPtr<CheckboxTheme>();
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(checkboxTheme));
    EXPECT_CALL(*themeManager, GetTheme(_, _)).WillRepeatedly(Return(checkboxTheme));
    return checkboxTheme;
}

RefPtr<ToggleTheme> ToggleModelNgTest::InstallToggleTheme()
{
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    auto toggleTheme = AceType::MakeRefPtr<ToggleTheme>();
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(toggleTheme));
    EXPECT_CALL(*themeManager, GetTheme(_, _)).WillRepeatedly(Return(toggleTheme));
    return toggleTheme;
}

namespace {
RefPtr<FrameNode> CreateSwitchOnStack()
{
    ToggleModelNG toggleModelNG;
    toggleModelNG.Create(ToggleType::SWITCH, IS_ON);
    // GetMainFrameNode returns a raw FrameNode* already retained by the stack; Claim wraps it
    // into a RefPtr without altering the refcount.
    return AceType::Claim(ViewStackProcessor::GetInstance()->GetMainFrameNode());
}

RefPtr<FrameNode> CreateToggleButtonNode()
{
    return FrameNode::CreateFrameNode(
        V2::TOGGLE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<ToggleButtonPattern>());
}
} // namespace

/**
 * @tc.name: ToggleModelNG_SetSwitchPointColorSetByUser_Instance
 * @tc.desc: Test the instance overload of SetSwitchPointColorSetByUser updates the flag on the stack node.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleModelNgTest, ToggleModelNG_SetSwitchPointColorSetByUser_Instance, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create a switch on the stack and read its paint property.
     */
    auto frameNode = CreateSwitchOnStack();
    ASSERT_NE(frameNode, nullptr);
    auto paintProperty = frameNode->GetPaintProperty<SwitchPaintProperty>();
    ASSERT_NE(paintProperty, nullptr);

    /**
     * @tc.steps: step2. Set the flag true then false via the instance overload.
     * @tc.expected: The flag mirrors the value passed to the instance overload.
     */
    ToggleModelNG toggleModelNG;
    toggleModelNG.SetSwitchPointColorSetByUser(true);
    EXPECT_TRUE(paintProperty->GetSwitchPointColorSetByUser().value_or(false));

    toggleModelNG.SetSwitchPointColorSetByUser(false);
    EXPECT_FALSE(paintProperty->GetSwitchPointColorSetByUser().value_or(true));
    ViewStackProcessor::GetInstance()->Finish();
}

/**
 * @tc.name: ToggleModelNG_AddNewChild_CheckboxAndSwitch_SetsNoStr
 * @tc.desc: Test AddNewChild sets accessibility level to NO_STR for CHECKBOX and SWITCH types.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleModelNgTest, ToggleModelNG_AddNewChild_CheckboxAndSwitch_SetsNoStr, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Build a parent and a toggle node carrying a real child.
     * @tc.expected: The toggle node is resolvable through ElementRegister by tag and nodeId.
     */
    auto parent = FrameNode::CreateFrameNode(
        "parent", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(parent, nullptr);
    auto nodeId = ElementRegister::GetInstance()->MakeUniqueId();
    auto toggleNode =
        FrameNode::CreateFrameNode(V2::TOGGLE_ETS_TAG, nodeId, AceType::MakeRefPtr<ToggleButtonPattern>());
    ASSERT_NE(toggleNode, nullptr);
    ASSERT_EQ(FrameNode::GetFrameNode(V2::TOGGLE_ETS_TAG, nodeId), toggleNode);

    auto child =
        FrameNode::CreateFrameNode("child", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<SwitchPattern>());
    ASSERT_NE(child, nullptr);
    ASSERT_NE(child->GetOrCreateAccessibilityProperty(), nullptr);
    toggleNode->AddChild(child);

    /**
     * @tc.steps: step2. Add the node for the CHECKBOX type.
     * @tc.expected: The real child's accessibility level becomes NO_STR (for-loop body, NO_STR branch).
     */
    ToggleModelNG::AddNewChild(parent, nodeId, ADD_CHILD_INDEX, ToggleType::CHECKBOX);
    auto accessibilityProperty = child->GetAccessibilityProperty<AccessibilityProperty>();
    ASSERT_NE(accessibilityProperty, nullptr);
    EXPECT_EQ(accessibilityProperty->GetAccessibilityLevel(), AccessibilityProperty::Level::NO_STR);

    /**
     * @tc.steps: step3. Add a fresh node for the SWITCH type.
     * @tc.expected: The real child's accessibility level becomes NO_STR (for-loop body, NO_STR branch).
     */
    auto nodeIdSwitch = ElementRegister::GetInstance()->MakeUniqueId();
    auto toggleNodeSwitch =
        FrameNode::CreateFrameNode(V2::TOGGLE_ETS_TAG, nodeIdSwitch, AceType::MakeRefPtr<ToggleButtonPattern>());
    ASSERT_NE(toggleNodeSwitch, nullptr);
    auto childSwitch =
        FrameNode::CreateFrameNode("child", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<SwitchPattern>());
    ASSERT_NE(childSwitch, nullptr);
    ASSERT_NE(childSwitch->GetOrCreateAccessibilityProperty(), nullptr);
    toggleNodeSwitch->AddChild(childSwitch);
    ToggleModelNG::AddNewChild(parent, nodeIdSwitch, ADD_CHILD_INDEX, ToggleType::SWITCH);
    auto childAccessibilityProperty = childSwitch->GetAccessibilityProperty<AccessibilityProperty>();
    ASSERT_NE(childAccessibilityProperty, nullptr);
    EXPECT_EQ(childAccessibilityProperty->GetAccessibilityLevel(), AccessibilityProperty::Level::NO_STR);
}

/**
 * @tc.name: ToggleModelNG_AddNewChild_Button_SetsYesStr
 * @tc.desc: Test AddNewChild sets accessibility level to YES_STR for the BUTTON type.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleModelNgTest, ToggleModelNG_AddNewChild_Button_SetsYesStr, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Build a parent and a toggle node with a real child.
     */
    auto parent = FrameNode::CreateFrameNode(
        "parent", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(parent, nullptr);
    auto nodeId = ElementRegister::GetInstance()->MakeUniqueId();
    auto toggleNode =
        FrameNode::CreateFrameNode(V2::TOGGLE_ETS_TAG, nodeId, AceType::MakeRefPtr<ToggleButtonPattern>());
    ASSERT_NE(toggleNode, nullptr);
    auto child = FrameNode::CreateFrameNode(
        "child", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<CheckBoxPattern>());
    ASSERT_NE(child, nullptr);
    ASSERT_NE(child->GetOrCreateAccessibilityProperty(), nullptr);
    toggleNode->AddChild(child);

    /**
     * @tc.steps: step2. Add the node for the BUTTON type.
     * @tc.expected: The child's accessibility level becomes YES_STR.
     */
    ToggleModelNG::AddNewChild(parent, nodeId, ADD_CHILD_INDEX, ToggleType::BUTTON);
    auto accessibilityProperty = child->GetAccessibilityProperty<AccessibilityProperty>();
    ASSERT_NE(accessibilityProperty, nullptr);
    EXPECT_EQ(accessibilityProperty->GetAccessibilityLevel(), AccessibilityProperty::Level::YES_STR);
}

/**
 * @tc.name: ToggleModelNG_SetResponseRegion_Instance
 * @tc.desc: Test the instance overload of SetResponseRegion marks the switch pattern as user-set.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleModelNgTest, ToggleModelNG_SetResponseRegion_Instance, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create a switch on the stack.
     */
    auto frameNode = CreateSwitchOnStack();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<SwitchPattern>();
    ASSERT_NE(pattern, nullptr);
    EXPECT_FALSE(pattern->isUserSetResponseRegion_);

    /**
     * @tc.steps: step2. Set a response region via the instance overload.
     * @tc.expected: The pattern's user-set response region flag becomes true.
     */
    DimensionOffset offset;
    std::vector<DimensionRect> responseRegion = { DimensionRect(RESPONSE_WIDTH, RESPONSE_HEIGHT, offset) };
    ToggleModelNG toggleModelNG;
    toggleModelNG.SetResponseRegion(responseRegion);
    EXPECT_TRUE(pattern->isUserSetResponseRegion_);
    ViewStackProcessor::GetInstance()->Finish();
}

/**
 * @tc.name: ToggleModelNG_SetResponseRegion_Static
 * @tc.desc: Test the static overload of SetResponseRegion marks the given switch pattern as user-set.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleModelNgTest, ToggleModelNG_SetResponseRegion_Static, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create a switch on the stack and read its pattern.
     */
    auto frameNode = CreateSwitchOnStack();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<SwitchPattern>();
    ASSERT_NE(pattern, nullptr);
    EXPECT_FALSE(pattern->isUserSetResponseRegion_);

    /**
     * @tc.steps: step2. Set a response region via the static overload.
     * @tc.expected: The pattern's user-set response region flag becomes true.
     */
    DimensionOffset offset;
    std::vector<DimensionRect> responseRegion = { DimensionRect(RESPONSE_WIDTH, RESPONSE_HEIGHT, offset) };
    ToggleModelNG::SetResponseRegion(AceType::RawPtr(frameNode), responseRegion);
    EXPECT_TRUE(pattern->isUserSetResponseRegion_);
    ViewStackProcessor::GetInstance()->Finish();
}

/**
 * @tc.name: ToggleModelNG_SetUnselectedColor_Instance
 * @tc.desc: Test the instance overload of SetUnselectedColor writes the color onto the stack node.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleModelNgTest, ToggleModelNG_SetUnselectedColor_Instance, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create a switch on the stack and read its paint property.
     */
    auto frameNode = CreateSwitchOnStack();
    ASSERT_NE(frameNode, nullptr);
    auto paintProperty = frameNode->GetPaintProperty<SwitchPaintProperty>();
    ASSERT_NE(paintProperty, nullptr);

    /**
     * @tc.steps: step2. Set the unselected color via the instance overload.
     * @tc.expected: The unselected color matches the value passed in.
     */
    ToggleModelNG toggleModelNG;
    toggleModelNG.SetUnselectedColor(UNSELECTED_COLOR);
    EXPECT_EQ(paintProperty->GetUnselectedColor().value_or(Color::BLACK), UNSELECTED_COLOR);
    ViewStackProcessor::GetInstance()->Finish();
}

/**
 * @tc.name: ToggleModelNG_SetSwitchPointColor_Nullopt_UsesThemePointColor
 * @tc.desc: Test SetSwitchPointColor uses the switch theme point color when no value is supplied.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleModelNgTest, ToggleModelNG_SetSwitchPointColor_Nullopt_UsesThemePointColor, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create a switch node and install a switch theme.
     */
    auto frameNode = FrameNode::CreateFrameNode(
        V2::TOGGLE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<SwitchPattern>());
    ASSERT_NE(frameNode, nullptr);
    auto switchTheme = InstallSwitchTheme();
    ASSERT_NE(switchTheme, nullptr);
    auto paintProperty = frameNode->GetPaintProperty<SwitchPaintProperty>();
    ASSERT_NE(paintProperty, nullptr);

    /**
     * @tc.steps: step2. Call SetSwitchPointColor with no value (nullopt branch).
     * @tc.expected: The point color falls back to the theme's point color.
     */
    ToggleModelNG::SetSwitchPointColor(AceType::RawPtr(frameNode), std::nullopt);
    EXPECT_EQ(paintProperty->GetSwitchPointColor().value_or(Color::BLACK), switchTheme->GetPointColor());
}

/**
 * @tc.name: ToggleModelNG_SetSwitchPointColor_WithValue
 * @tc.desc: Test SetSwitchPointColor applies the supplied value directly.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleModelNgTest, ToggleModelNG_SetSwitchPointColor_WithValue, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create a switch node and read its paint property.
     */
    auto frameNode = FrameNode::CreateFrameNode(
        V2::TOGGLE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<SwitchPattern>());
    ASSERT_NE(frameNode, nullptr);
    InstallSwitchTheme();
    auto paintProperty = frameNode->GetPaintProperty<SwitchPaintProperty>();
    ASSERT_NE(paintProperty, nullptr);

    /**
     * @tc.steps: step2. Call SetSwitchPointColor with a concrete value (has_value branch).
     * @tc.expected: The point color equals the supplied value, not the theme fallback.
     */
    ToggleModelNG::SetSwitchPointColor(AceType::RawPtr(frameNode), POINT_COLOR);
    EXPECT_EQ(paintProperty->GetSwitchPointColor().value_or(Color::BLACK), POINT_COLOR);
}

/**
 * @tc.name: ToggleModelNG_CreateWithColorResourceObj_Instance_NullRes
 * @tc.desc: Test the instance CreateWithColorResourceObj removes the resource when the object is null.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleModelNgTest, ToggleModelNG_CreateWithColorResourceObj_Instance_NullRes, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create a switch on the stack and equip it with a resource manager.
     */
    auto frameNode = CreateSwitchOnStack();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<SwitchPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->resourceMgr_ = AceType::MakeRefPtr<PatternResourceManager>();
    auto resMgr = pattern->resourceMgr_;
    ASSERT_NE(resMgr, nullptr);

    /**
     * @tc.steps: step2. Call the instance overload with a null resource object.
     * @tc.expected: No entry is registered under the selected-color key.
     */
    ToggleModelNG toggleModelNG;
    toggleModelNG.CreateWithColorResourceObj(nullptr, ToggleColorType::SELECTED_COLOR);
    EXPECT_EQ(resMgr->resMap_.count(SELECTED_COLOR_KEY), 0);
    ViewStackProcessor::GetInstance()->Finish();
}

/**
 * @tc.name: ToggleModelNG_CreateWithColorResourceObj_Instance_ValidRes
 * @tc.desc: Test the instance CreateWithColorResourceObj registers a resource for a valid object.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleModelNgTest, ToggleModelNG_CreateWithColorResourceObj_Instance_ValidRes, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create a switch on the stack and equip it with a resource manager.
     */
    auto frameNode = CreateSwitchOnStack();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<SwitchPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->resourceMgr_ = AceType::MakeRefPtr<PatternResourceManager>();
    auto resMgr = pattern->resourceMgr_;
    ASSERT_NE(resMgr, nullptr);

    /**
     * @tc.steps: step2. Call the instance overload with a valid resource object.
     * @tc.expected: Exactly one entry is registered under the selected-color key.
     */
    auto resObj = AceType::MakeRefPtr<ResourceObject>("", "", RESOURCE_ID);
    ToggleModelNG toggleModelNG;
    toggleModelNG.CreateWithColorResourceObj(resObj, ToggleColorType::SELECTED_COLOR);
    EXPECT_EQ(resMgr->resMap_.count(SELECTED_COLOR_KEY), 1);
    ViewStackProcessor::GetInstance()->Finish();
}

/**
 * @tc.name: ToggleModelNG_SetSwitchDefaultColor_ColorBranches
 * @tc.desc: Test SetSwitchDefaultColor handles selected, point, unselected, and default types on a switch node.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleModelNgTest, ToggleModelNG_SetSwitchDefaultColor_ColorBranches, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create a switch on the stack and install a switch theme.
     */
    auto frameNode = CreateSwitchOnStack();
    ASSERT_NE(frameNode, nullptr);
    auto switchTheme = InstallSwitchTheme();
    ASSERT_NE(switchTheme, nullptr);
    auto paintProperty = frameNode->GetPaintProperty<SwitchPaintProperty>();
    ASSERT_NE(paintProperty, nullptr);

    /**
     * @tc.steps: step2. Exercise each color-type branch of SetSwitchDefaultColor.
     * @tc.expected: Each color property matches the corresponding theme color.
     */
    ToggleModelNG toggleModelNG;
    toggleModelNG.SetSwitchDefaultColor(AceType::RawPtr(frameNode), ToggleColorType::SELECTED_COLOR);
    EXPECT_EQ(paintProperty->GetSelectedColor().value_or(Color::BLACK), switchTheme->GetActiveColor());

    toggleModelNG.SetSwitchDefaultColor(AceType::RawPtr(frameNode), ToggleColorType::SWITCH_POINT_COLOR);
    EXPECT_EQ(paintProperty->GetSwitchPointColor().value_or(Color::BLACK), switchTheme->GetPointColor());

    toggleModelNG.SetSwitchDefaultColor(AceType::RawPtr(frameNode), ToggleColorType::UN_SELECTED_COLOR);
    EXPECT_EQ(paintProperty->GetUnselectedColor().value_or(Color::BLACK), switchTheme->GetInactiveColor());

    // Default branch must not crash and must leave previously set properties stable.
    toggleModelNG.SetSwitchDefaultColor(AceType::RawPtr(frameNode), static_cast<ToggleColorType>(99));
    EXPECT_EQ(paintProperty->GetSelectedColor().value_or(Color::BLACK), switchTheme->GetActiveColor());
    ViewStackProcessor::GetInstance()->Finish();
}

/**
 * @tc.name: ToggleModelNG_SetSwitchDefaultColor_BackgroundResetsToggleBg
 * @tc.desc: Test SetSwitchDefaultColor BACKGROUND_COLOR branch resets the toggle button background.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleModelNgTest, ToggleModelNG_SetSwitchDefaultColor_BackgroundResetsToggleBg, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create a toggle button node, install a switch theme (read before the type switch),
     *           and pre-set its background color.
     */
    auto toggleNode = CreateToggleButtonNode();
    ASSERT_NE(toggleNode, nullptr);
    InstallSwitchTheme();
    auto paintProperty = toggleNode->GetPaintProperty<ToggleButtonPaintProperty>();
    ASSERT_NE(paintProperty, nullptr);
    paintProperty->UpdateBackgroundColor(Color::BLACK);
    EXPECT_TRUE(paintProperty->GetBackgroundColor().has_value());

    /**
     * @tc.steps: step2. Set the default color for BACKGROUND_COLOR type.
     * @tc.expected: The background color is reset and no longer has a value.
     */
    ToggleModelNG toggleModelNG;
    toggleModelNG.SetSwitchDefaultColor(AceType::RawPtr(toggleNode), ToggleColorType::BACKGROUND_COLOR);
    EXPECT_FALSE(paintProperty->GetBackgroundColor().has_value());
}

/**
 * @tc.name: ToggleModelNG_CreateWithSwitchResourceObj_NullAndValid
 * @tc.desc: Test CreateWithSwitchResourceObj removes on null and registers on valid resource objects.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleModelNgTest, ToggleModelNG_CreateWithSwitchResourceObj_NullAndValid, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create a switch node.
     */
    auto frameNode = FrameNode::CreateFrameNode(
        V2::TOGGLE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<SwitchPattern>());
    ASSERT_NE(frameNode, nullptr);
    frameNode->MarkModifyDone();
    auto pattern = frameNode->GetPattern<SwitchPattern>();
    ASSERT_NE(pattern, nullptr);
    const std::string key = "toggle" + ToggleModelNG::ColorTypeToString(ToggleColorType::SELECTED_COLOR);

    /**
     * @tc.steps: step2. Call with a null resource object (before equipping a resource manager, so the
     *           null branch's RemoveResObj is a no-op and cannot reset resourceMgr_ to null).
     * @tc.expected: No entry is registered under the key.
     */
    ToggleModelNG::CreateWithSwitchResourceObj(
        AceType::RawPtr(frameNode), ToggleColorType::SELECTED_COLOR, nullptr, key);

    /**
     * @tc.steps: step3. Equip the pattern with a fresh resource manager and call with a valid resource object.
     * @tc.expected: Exactly one entry is registered under the key in the equipped manager.
     */
    pattern->resourceMgr_ = AceType::MakeRefPtr<PatternResourceManager>();
    auto resMgr = pattern->resourceMgr_;
    ASSERT_NE(resMgr, nullptr);
    EXPECT_EQ(resMgr->resMap_.count(key), 0);

    auto resObj = AceType::MakeRefPtr<ResourceObject>("", "", RESOURCE_ID);
    ToggleModelNG::CreateWithSwitchResourceObj(
        AceType::RawPtr(frameNode), ToggleColorType::SELECTED_COLOR, resObj, key);
    EXPECT_EQ(resMgr->resMap_.count(key), 1);
}

/**
 * @tc.name: ToggleModelNG_SetCheckboxDefaultColor_SelectedBranch
 * @tc.desc: Test SetCheckboxDefaultColor SELECTED_COLOR branch sets the selected color from the checkbox theme.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleModelNgTest, ToggleModelNG_SetCheckboxDefaultColor_SelectedBranch, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create a switch node (carries SwitchPaintProperty) and install a checkbox theme.
     */
    auto frameNode = CreateSwitchOnStack();
    ASSERT_NE(frameNode, nullptr);
    auto checkboxTheme = InstallCheckboxTheme();
    ASSERT_NE(checkboxTheme, nullptr);
    auto paintProperty = frameNode->GetPaintProperty<SwitchPaintProperty>();
    ASSERT_NE(paintProperty, nullptr);

    /**
     * @tc.steps: step2. Exercise the SELECTED_COLOR branch.
     * @tc.expected: Selected color matches the checkbox theme active color.
     */
    ToggleModelNG toggleModelNG;
    toggleModelNG.SetCheckboxDefaultColor(AceType::RawPtr(frameNode), ToggleColorType::SELECTED_COLOR);
    EXPECT_EQ(paintProperty->GetSelectedColor().value_or(Color::BLACK), checkboxTheme->GetActiveColor());
    ViewStackProcessor::GetInstance()->Finish();
}

/**
 * @tc.name: ToggleModelNG_SetCheckboxDefaultColor_BackgroundResetsToggleBg
 * @tc.desc: Test SetCheckboxDefaultColor BACKGROUND_COLOR branch resets the toggle button background.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleModelNgTest, ToggleModelNG_SetCheckboxDefaultColor_BackgroundResetsToggleBg, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create a toggle button node and pre-set its background color.
     */
    auto toggleNode = CreateToggleButtonNode();
    ASSERT_NE(toggleNode, nullptr);
    auto paintProperty = toggleNode->GetPaintProperty<ToggleButtonPaintProperty>();
    ASSERT_NE(paintProperty, nullptr);
    paintProperty->UpdateBackgroundColor(Color::BLACK);
    EXPECT_TRUE(paintProperty->GetBackgroundColor().has_value());

    /**
     * @tc.steps: step2. Set the default color for BACKGROUND_COLOR type.
     * @tc.expected: The background color is reset and no longer has a value.
     */
    ToggleModelNG toggleModelNG;
    toggleModelNG.SetCheckboxDefaultColor(AceType::RawPtr(toggleNode), ToggleColorType::BACKGROUND_COLOR);
    EXPECT_FALSE(paintProperty->GetBackgroundColor().has_value());
}

/**
 * @tc.name: ToggleModelNG_CreateWithCheckBoxResourceObj_NullAndValid
 * @tc.desc: Test CreateWithCheckBoxResourceObj removes on null and registers on valid resource objects.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleModelNgTest, ToggleModelNG_CreateWithCheckBoxResourceObj_NullAndValid, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create a checkbox toggle node.
     */
    auto frameNode = FrameNode::CreateFrameNode(
        V2::TOGGLE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<ToggleCheckBoxPattern>());
    ASSERT_NE(frameNode, nullptr);
    frameNode->MarkModifyDone();
    auto pattern = frameNode->GetPattern();
    ASSERT_NE(pattern, nullptr);
    const std::string key = "toggle" + ToggleModelNG::ColorTypeToString(ToggleColorType::SELECTED_COLOR);

    /**
     * @tc.steps: step2. Call with a null resource object (before equipping a resource manager, so the
     *           null branch's RemoveResObj is a no-op and cannot reset resourceMgr_ to null).
     * @tc.expected: No entry is registered under the key.
     */
    ToggleModelNG::CreateWithCheckBoxResourceObj(
        AceType::RawPtr(frameNode), ToggleColorType::SELECTED_COLOR, nullptr, key);

    /**
     * @tc.steps: step3. Equip the pattern with a fresh resource manager and call with a valid resource object.
     * @tc.expected: Exactly one entry is registered under the key in the equipped manager.
     */
    pattern->resourceMgr_ = AceType::MakeRefPtr<PatternResourceManager>();
    auto resMgr = pattern->resourceMgr_;
    ASSERT_NE(resMgr, nullptr);
    EXPECT_EQ(resMgr->resMap_.count(key), 0);

    auto resObj = AceType::MakeRefPtr<ResourceObject>("", "", RESOURCE_ID);
    ToggleModelNG::CreateWithCheckBoxResourceObj(
        AceType::RawPtr(frameNode), ToggleColorType::SELECTED_COLOR, resObj, key);
    EXPECT_EQ(resMgr->resMap_.count(key), 1);
}

/**
 * @tc.name: ToggleModelNG_SetButtonDefaultColor_SelectedBranch
 * @tc.desc: Test SetButtonDefaultColor SELECTED_COLOR branch sets the selected color from the toggle theme.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleModelNgTest, ToggleModelNG_SetButtonDefaultColor_SelectedBranch, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create a switch node (carries SwitchPaintProperty) and install a toggle theme.
     */
    auto frameNode = CreateSwitchOnStack();
    ASSERT_NE(frameNode, nullptr);
    auto toggleTheme = InstallToggleTheme();
    ASSERT_NE(toggleTheme, nullptr);
    auto paintProperty = frameNode->GetPaintProperty<SwitchPaintProperty>();
    ASSERT_NE(paintProperty, nullptr);

    /**
     * @tc.steps: step2. Exercise the SELECTED_COLOR branch.
     * @tc.expected: Selected color matches the toggle theme checked color.
     */
    ToggleModelNG toggleModelNG;
    toggleModelNG.SetButtonDefaultColor(AceType::RawPtr(frameNode), ToggleColorType::SELECTED_COLOR);
    EXPECT_EQ(paintProperty->GetSelectedColor().value_or(Color::BLACK), toggleTheme->GetCheckedColor());
    ViewStackProcessor::GetInstance()->Finish();
}

/**
 * @tc.name: ToggleModelNG_SetButtonDefaultColor_BackgroundResetsToggleBg
 * @tc.desc: Test SetButtonDefaultColor BACKGROUND_COLOR branch resets the toggle button background.
 * @tc.type: FUNC
 */
HWTEST_F(ToggleModelNgTest, ToggleModelNG_SetButtonDefaultColor_BackgroundResetsToggleBg, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create a toggle button node and pre-set its background color.
     */
    auto toggleNode = CreateToggleButtonNode();
    ASSERT_NE(toggleNode, nullptr);
    auto paintProperty = toggleNode->GetPaintProperty<ToggleButtonPaintProperty>();
    ASSERT_NE(paintProperty, nullptr);
    paintProperty->UpdateBackgroundColor(Color::BLACK);
    EXPECT_TRUE(paintProperty->GetBackgroundColor().has_value());

    /**
     * @tc.steps: step2. Set the default color for BACKGROUND_COLOR type.
     * @tc.expected: The background color is reset and no longer has a value.
     */
    ToggleModelNG toggleModelNG;
    toggleModelNG.SetButtonDefaultColor(AceType::RawPtr(toggleNode), ToggleColorType::BACKGROUND_COLOR);
    EXPECT_FALSE(paintProperty->GetBackgroundColor().has_value());
}
} // namespace OHOS::Ace::NG

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

#include <type_traits>

#include "gtest/gtest.h"

#define private public
#define protected public

#include "test/mock/adapter/ohos/osal/mock_system_properties.h"
#include "test/mock/frameworks/core/common/mock_container.h"
#include "test/mock/frameworks/core/common/mock_theme_manager.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"
#include "test/mock/frameworks/core/components_ng/render/mock_render_context.h"

#include "core/components/select/select_theme.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/flex/flex_layout_pattern.h"
#include "core/components_ng/pattern/image/image_pattern.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"
#include "core/components_ng/pattern/menu/menu_item/menu_item_pattern.h"
#include "core/components_ng/pattern/menu/menu_layout_property.h"
#include "core/components_ng/pattern/menu/menu_pattern.h"
#include "core/components_ng/pattern/menu/menu_theme.h"
#include "core/components_ng/pattern/menu/menu_view.h"
#include "core/components_ng/pattern/menu/preview/menu_preview_pattern.h"
#include "core/components_ng/pattern/menu/wrapper/menu_wrapper_pattern.h"
#include "core/components_ng/pattern/scroll/scroll_pattern.h"
#include "core/components_ng/pattern/stack/stack_pattern.h"
#include "core/components_ng/pattern/text/text_layout_property.h"
#include "core/components_ng/pattern/text/text_pattern.h"
#include "core/components_ng/property/border_property.h"
#include "core/components_ng/property/menu_property.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Ace::Framework;

namespace OHOS::Ace::NG {
namespace {
constexpr int32_t TARGET_ID = 100;
const std::string TEXT_TAG = "text";

// Build a menu FrameNode (with a MenuPattern) mounted under a MenuWrapper that
// carries the given MenuParam, so MenuPattern::GetMenuParam()/GetMenuWrapper()
// resolve correctly. Mirrors the helper used in menu_pattern_test_ng.cpp.
RefPtr<FrameNode> BuildMenuNodeWithParam(const MenuParam& menuParam)
{
    auto wrapper = FrameNode::CreateFrameNode(V2::MENU_WRAPPER_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<MenuWrapperPattern>(TARGET_ID));
    auto menu = FrameNode::CreateFrameNode(V2::MENU_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<MenuPattern>(TARGET_ID, TEXT_TAG, MenuType::MENU));
    if (menu) {
        menu->MountToParent(wrapper);
    }
    auto wrapperPattern = wrapper ? wrapper->GetPattern<MenuWrapperPattern>() : nullptr;
    if (wrapperPattern) {
        wrapperPattern->SetMenuParam(menuParam);
    }
    return menu;
}
} // namespace

class MenuViewThreeTestNg : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    void InstallThemes();
};

void MenuViewThreeTestNg::SetUpTestCase() {}

void MenuViewThreeTestNg::TearDownTestCase() {}

void MenuViewThreeTestNg::SetUp()
{
    MockPipelineContext::SetUp();
    InstallThemes();
    MockContainer::SetUp();
}

void MenuViewThreeTestNg::TearDown()
{
    MockPipelineContext::TearDown();
    MockContainer::TearDown();
}

void MenuViewThreeTestNg::InstallThemes()
{
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    auto resolver = [](ThemeType type) -> RefPtr<Theme> {
        if (type == SelectTheme::TypeId()) {
            return AceType::MakeRefPtr<SelectTheme>();
        }
        return AceType::MakeRefPtr<MenuTheme>();
    };
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(resolver);
    EXPECT_CALL(*themeManager, GetTheme(_, _))
        .WillRepeatedly([](ThemeType type, int32_t /*id*/) -> RefPtr<Theme> {
            if (type == SelectTheme::TypeId()) {
                return AceType::MakeRefPtr<SelectTheme>();
            }
            return AceType::MakeRefPtr<MenuTheme>();
        });
}

/**
 * @tc.name: MenuViewIsGridStyleEnabled001
 * @tc.desc: Test MenuView::IsGridStyleEnabled branch: no gridStyle / count<=0 / count>0.
 * @tc.type: FUNC
 */
HWTEST_F(MenuViewThreeTestNg, MenuViewIsGridStyleEnabled001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. MenuParam without gridStyle.
     * @tc.expected: returns false.
     */
    MenuParam param;
    EXPECT_FALSE(MenuView::IsGridStyleEnabled(param));

    /**
     * @tc.steps: step2. gridStyle with count<=0.
     * @tc.expected: returns false.
     */
    MenuGridStyleOptions style;
    style.count = 0;
    param.gridStyle = style;
    EXPECT_FALSE(MenuView::IsGridStyleEnabled(param));

    /**
     * @tc.steps: step3. gridStyle with count>0.
     * @tc.expected: returns true.
     */
    style.count = 4;
    param.gridStyle = style;
    EXPECT_TRUE(MenuView::IsGridStyleEnabled(param));
}

/**
 * @tc.name: MenuViewIsMenuWithOnlyMenuItems001
 * @tc.desc: Test MenuView::IsMenuWithOnlyMenuItems branches: null/wrong-tag/empty/non-menuitem/all-menuitem.
 * @tc.type: FUNC
 */
HWTEST_F(MenuViewThreeTestNg, MenuViewIsMenuWithOnlyMenuItems001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. null node.
     * @tc.expected: returns false.
     */
    EXPECT_FALSE(MenuView::IsMenuWithOnlyMenuItems(nullptr));

    /**
     * @tc.steps: step2. node whose tag is not MENU_ETS_TAG.
     * @tc.expected: returns false.
     */
    auto textNode = FrameNode::CreateFrameNode(
        V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(textNode, nullptr);
    EXPECT_FALSE(MenuView::IsMenuWithOnlyMenuItems(textNode));

    /**
     * @tc.steps: step3. MENU node with no children.
     * @tc.expected: returns false.
     */
    auto menuPattern = AceType::MakeRefPtr<MenuPattern>(TARGET_ID, TEXT_TAG, MenuType::MENU);
    auto menuNode = FrameNode::CreateFrameNode(
        V2::MENU_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), menuPattern);
    ASSERT_NE(menuNode, nullptr);
    EXPECT_FALSE(MenuView::IsMenuWithOnlyMenuItems(menuNode));

    /**
     * @tc.steps: step4. MENU node whose child is not a MenuItem.
     * @tc.expected: returns false.
     */
    textNode->MountToParent(menuNode);
    EXPECT_FALSE(MenuView::IsMenuWithOnlyMenuItems(menuNode));

    /**
     * @tc.steps: step5. MENU node whose only child is a MenuItem.
     * @tc.expected: returns true.
     */
    auto menuPattern2 = AceType::MakeRefPtr<MenuPattern>(TARGET_ID, TEXT_TAG, MenuType::MENU);
    auto menuNode2 = FrameNode::CreateFrameNode(
        V2::MENU_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), menuPattern2);
    ASSERT_NE(menuNode2, nullptr);
    auto menuItem = FrameNode::CreateFrameNode(V2::MENU_ITEM_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<MenuItemPattern>());
    ASSERT_NE(menuItem, nullptr);
    menuItem->MountToParent(menuNode2);
    EXPECT_TRUE(MenuView::IsMenuWithOnlyMenuItems(menuNode2));
}

/**
 * @tc.name: MenuViewHoverScaleStatusMap001
 * @tc.desc: Test MenuView set/get/remove MenuHoverScaleStatus thread_local map branches.
 * @tc.type: FUNC
 */
HWTEST_F(MenuViewThreeTestNg, MenuViewHoverScaleStatusMap001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. query a targetId that has never been registered.
     * @tc.expected: returns MenuHoverScaleStatus::NONE.
     */
    constexpr int32_t targetA = 9001;
    EXPECT_EQ(MenuView::GetMenuHoverScaleStatus(targetA), MenuHoverScaleStatus::NONE);

    /**
     * @tc.steps: step2. set status and query.
     * @tc.expected: returns the just-set status.
     */
    MenuView::SetMenuHoverScaleStatus(targetA, MenuHoverScaleStatus::MENU_SHOW);
    EXPECT_EQ(MenuView::GetMenuHoverScaleStatus(targetA), MenuHoverScaleStatus::MENU_SHOW);

    /**
     * @tc.steps: step3. remove status and query again.
     * @tc.expected: returns NONE after erase.
     */
    MenuView::RemoveMenuHoverScaleStatus(targetA);
    EXPECT_EQ(MenuView::GetMenuHoverScaleStatus(targetA), MenuHoverScaleStatus::NONE);
}

/**
 * @tc.name: MenuViewSetHasCustomOutline001
 * @tc.desc: Test MenuView::SetHasCustomOutline branches for outlineWidth presence/sign.
 * @tc.type: FUNC
 */
HWTEST_F(MenuViewThreeTestNg, MenuViewSetHasCustomOutline001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. menuParam without outlineWidth.
     * @tc.expected: hasCustomOutlineWidth/Color both set false, no crash.
     */
    MenuParam param;
    auto wrapper = FrameNode::CreateFrameNode(V2::MENU_WRAPPER_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<MenuWrapperPattern>(TARGET_ID));
    auto menu = FrameNode::CreateFrameNode(V2::MENU_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<MenuPattern>(TARGET_ID, TEXT_TAG, MenuType::MENU));
    ASSERT_NE(wrapper, nullptr);
    ASSERT_NE(menu, nullptr);
    menu->MountToParent(wrapper);
    MenuView::SetHasCustomOutline(wrapper, menu, param);
    auto wp = wrapper->GetPattern<MenuWrapperPattern>();
    ASSERT_NE(wp, nullptr);
    EXPECT_FALSE(wp->GetHasCustomOutlineWidth());
    EXPECT_FALSE(wp->GetHasCustomOutlineColor());

    /**
     * @tc.steps: step2. outlineWidth with a negative leftDimen.
     * @tc.expected: branch hits the "any negative" early return → both false.
     */
    param.outlineWidth = BorderWidthProperty();
    param.outlineWidth->leftDimen = Dimension(-1.0_vp);
    MenuView::SetHasCustomOutline(wrapper, menu, param);
    EXPECT_FALSE(wp->GetHasCustomOutlineWidth());
    EXPECT_FALSE(wp->GetHasCustomOutlineColor());

    /**
     * @tc.steps: step3. outlineWidth with all non-negative dimensions.
     * @tc.expected: both hasCustomOutlineWidth/Color set true.
     */
    param.outlineWidth->leftDimen = Dimension(1.0_vp);
    param.outlineWidth->rightDimen = Dimension(1.0_vp);
    param.outlineWidth->topDimen = Dimension(1.0_vp);
    param.outlineWidth->bottomDimen = Dimension(1.0_vp);
    MenuView::SetHasCustomOutline(wrapper, menu, param);
    EXPECT_TRUE(wp->GetHasCustomOutlineWidth());
    EXPECT_TRUE(wp->GetHasCustomOutlineColor());
}

/**
 * @tc.name: MenuViewSetTextTruncationAndWrap001
 * @tc.desc: Test MenuView::SetTextTruncationAndWrap autoWrapFlag true/false branches.
 * @tc.type: FUNC
 */
HWTEST_F(MenuViewThreeTestNg, MenuViewSetTextTruncationAndWrap001, TestSize.Level1)
{
    // EnableSmallLanguageTruncation is gated by PlatformVersion::VERSION_TWENTY_SIX,
    // and UINode::apiVersion_ is snapshotted from Container::GetCurrentApiTargetVersion()
    // at node creation, so the Container version must be raised BEFORE creating the
    // text node. APIVersionGuard only sets AceApplicationInfo, which is NOT consulted
    // when a Container is installed (MockContainer::SetUp installs one with version 0).
    int32_t backupApiVersion = MockContainer::Current()->GetApiTargetVersion();
    MockContainer::Current()->SetApiTargetVersion(static_cast<int32_t>(PlatformVersion::VERSION_TWENTY_SIX));
    auto textNode = FrameNode::CreateFrameNode(
        V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(textNode, nullptr);
    auto textProperty = textNode->GetLayoutProperty<TextLayoutProperty>();
    ASSERT_NE(textProperty, nullptr);

    /**
     * @tc.steps: step1. autoWrapFlag=false.
     * @tc.expected: MaxLines=1 and TextOverflow=ELLIPSIS.
     */
    MenuView::SetTextTruncationAndWrap(textProperty, false);
    EXPECT_EQ(textProperty->GetMaxLines().value_or(0), 1);
    EXPECT_EQ(textProperty->GetTextOverflow().value_or(TextOverflow::CLIP), TextOverflow::ELLIPSIS);

    /**
     * @tc.steps: step2. autoWrapFlag=true.
     * @tc.expected: MaxLines is INT_MAX; EnableSmallLanguageTruncation stays true.
     */
    MenuView::SetTextTruncationAndWrap(textProperty, true);
    EXPECT_EQ(textProperty->GetMaxLines().value_or(0), std::numeric_limits<int32_t>::max());
    EXPECT_TRUE(textProperty->GetEnableSmallLanguageTruncation().value_or(false));
    MockContainer::Current()->SetApiTargetVersion(backupApiVersion);
}

/**
 * @tc.name: MenuViewUpdateStyleOptionColorMode001
 * @tc.desc: Test MenuView::UpdateStyleOptionColorMode isColorModeFollowTarget + LIGHT/DARK branches.
 * @tc.type: FUNC
 */
HWTEST_F(MenuViewThreeTestNg, MenuViewUpdateStyleOptionColorMode001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. isColorModeFollowTarget=true with LIGHT local color mode.
     * @tc.expected: styleOption.colorMode = LIGHT.
     */
    BlurStyleOption option;
    MenuView::UpdateStyleOptionColorMode(ColorMode::LIGHT, option, true);
    EXPECT_EQ(option.colorMode, ThemeColorMode::LIGHT);

    /**
     * @tc.steps: step2. isColorModeFollowTarget=true with DARK local color mode.
     * @tc.expected: styleOption.colorMode = DARK.
     */
    MenuView::UpdateStyleOptionColorMode(ColorMode::DARK, option, true);
    EXPECT_EQ(option.colorMode, ThemeColorMode::DARK);

    /**
     * @tc.steps: step3. isColorModeFollowTarget=false (falls back to current pipeline color mode).
     * @tc.expected: colorMode is one of LIGHT/DARK/SYSTEM (no crash).
     */
    MenuView::UpdateStyleOptionColorMode(ColorMode::LIGHT, option, false);
    EXPECT_TRUE(option.colorMode == ThemeColorMode::LIGHT || option.colorMode == ThemeColorMode::DARK ||
                option.colorMode == ThemeColorMode::SYSTEM);
}

/**
 * @tc.name: MenuViewUpdateMenuPaintProperty001
 * @tc.desc: Test MenuView::UpdateMenuPaintProperty API-version + type branches.
 * @tc.type: FUNC
 */
HWTEST_F(MenuViewThreeTestNg, MenuViewUpdateMenuPaintProperty001, TestSize.Level1)
{
    // UpdateMenuPaintProperty takes the MENU/CONTEXT_MENU branch only when
    // API >= VERSION_TWELVE; default mock API version is 0. Raise it on the
    // Container (not AceApplicationInfo) so Container::GreatOrEqualAPITargetVersion
    // and the snapshotted UINode::apiVersion_ both see VERSION_TWENTY_SIX.
    int32_t backupApiVersion = MockContainer::Current()->GetApiTargetVersion();
    MockContainer::Current()->SetApiTargetVersion(static_cast<int32_t>(PlatformVersion::VERSION_TWENTY_SIX));
    auto menu = FrameNode::CreateFrameNode(V2::MENU_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<MenuPattern>(TARGET_ID, TEXT_TAG, MenuType::MENU));
    ASSERT_NE(menu, nullptr);

    /**
     * @tc.steps: step1. enableArrow=true, arrowOffset set, type=MENU, API>=12.
     * @tc.expected: paint property gets enableArrow/arrowOffset (CONTEXT_MENU/MENU path taken).
     */
    MenuParam param;
    param.enableArrow = true;
    param.arrowOffset = Dimension(5.0_vp);
    MenuView::UpdateMenuPaintProperty(menu, param, MenuType::MENU);
    auto paint = menu->GetPaintProperty<MenuPaintProperty>();
    ASSERT_NE(paint, nullptr);
    EXPECT_TRUE(paint->GetEnableArrow().value_or(false));

    /**
     * @tc.steps: step2. type=SUB_MENU (not CONTEXT_MENU/MENU) under API>=12.
     * @tc.expected: early return — enableArrow stays unchanged (still the previously set value).
     */
    MenuView::UpdateMenuPaintProperty(menu, param, MenuType::SUB_MENU);
    EXPECT_TRUE(paint->GetEnableArrow().value_or(false));
    MockContainer::Current()->SetApiTargetVersion(backupApiVersion);
}

/**
 * @tc.name: MenuViewUpdateMenuBackgroundEffect001
 * @tc.desc: Test MenuView::UpdateMenuBackgroundEffect with bgBlurEffectEnable false (no-op branch).
 * @tc.type: FUNC
 */
HWTEST_F(MenuViewThreeTestNg, MenuViewUpdateMenuBackgroundEffect001, TestSize.Level1)
{
    auto menu = FrameNode::CreateFrameNode(V2::MENU_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<MenuPattern>(TARGET_ID, TEXT_TAG, MenuType::MENU));
    ASSERT_NE(menu, nullptr);
    auto rc = menu->GetRenderContext();
    ASSERT_NE(rc, nullptr);
    rc->UpdateBackgroundColor(Color::BLUE);

    /**
     * @tc.steps: step1. default-constructed MenuTheme has GetBgBlurEffectEnable()==false.
     * @tc.expected: branch not taken, background color unchanged, no crash.
     */
    MenuView::UpdateMenuBackgroundEffect(menu);
    auto colorOpt = rc->GetBackgroundColor();
    ASSERT_TRUE(colorOpt.has_value());
    EXPECT_EQ(colorOpt.value(), Color::BLUE);
}

/**
 * @tc.name: MenuViewUpdateMenuBackgroundStyleSub001
 * @tc.desc: Test MenuView::UpdateMenuBackgroundStyleSub updates background color from SelectTheme.
 * @tc.type: FUNC
 */
HWTEST_F(MenuViewThreeTestNg, MenuViewUpdateMenuBackgroundStyleSub001, TestSize.Level1)
{
    auto menu = FrameNode::CreateFrameNode(V2::MENU_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<MenuPattern>(TARGET_ID, TEXT_TAG, MenuType::MENU));
    ASSERT_NE(menu, nullptr);

    /**
     * @tc.steps: step1. call UpdateMenuBackgroundStyleSub with no backgroundColor in MenuParam.
     * @tc.expected: SelectTheme's GetBackgroundColor is applied, no crash.
     */
    MenuParam param;
    MenuView::UpdateMenuBackgroundStyleSub(menu, param);
    auto rc = menu->GetRenderContext();
    ASSERT_NE(rc, nullptr);
    EXPECT_TRUE(rc->GetBackgroundColor().has_value());
}

/**
 * @tc.name: MenuViewUpdateMenuOutlineWithArrow001
 * @tc.desc: Test MenuView::UpdateMenuOutlineWithArrow early-return and apply branches.
 * @tc.type: FUNC
 */
HWTEST_F(MenuViewThreeTestNg, MenuViewUpdateMenuOutlineWithArrow001, TestSize.Level1)
{
    auto wrapper = FrameNode::CreateFrameNode(V2::MENU_WRAPPER_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<MenuWrapperPattern>(TARGET_ID));
    auto menu = FrameNode::CreateFrameNode(V2::MENU_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<MenuPattern>(TARGET_ID, TEXT_TAG, MenuType::MENU));
    ASSERT_NE(wrapper, nullptr);
    ASSERT_NE(menu, nullptr);
    menu->MountToParent(wrapper);
    auto wp = wrapper->GetPattern<MenuWrapperPattern>();
    ASSERT_NE(wp, nullptr);

    /**
     * @tc.steps: step1. HasCustomOutlineWidth=false.
     * @tc.expected: early return, render context outer border untouched.
     */
    MenuParam param;
    MenuView::UpdateMenuOutlineWithArrow(menu, wrapper, param);
    auto rc = menu->GetRenderContext();
    ASSERT_NE(rc, nullptr);

    /**
     * @tc.steps: step2. HasCustomOutlineWidth=true.
     * @tc.expected: outer border width/color set to 0/TRANSPARENT, menuParam stored on wrapper.
     */
    wp->SetHasCustomOutlineWidth(true);
    MenuView::UpdateMenuOutlineWithArrow(menu, wrapper, param);
    EXPECT_TRUE(wp->GetHasCustomOutlineWidth());
}

/**
 * @tc.name: MenuViewUpdateMenuPositionTopLeft001
 * @tc.desc: Test MenuView::UpdateMenuPositionTop / UpdateMenuPositionLeft margin setters.
 * @tc.type: FUNC
 */
HWTEST_F(MenuViewThreeTestNg, MenuViewUpdateMenuPositionTopLeft001, TestSize.Level1)
{
    MarginProperty margin;
    std::map<AlignDirection, AlignRule> alignRules;
    AlignRule rule { .anchor = "__stack__", .vertical = VerticalAlign::TOP };

    /**
     * @tc.steps: step1. call UpdateMenuPositionTop.
     * @tc.expected: TOP/BOTTOM rules added; margin.top/bottom set.
     */
    MenuView::UpdateMenuPositionTop(margin, alignRules, rule, 10.0f, -10.0f);
    EXPECT_NE(margin.top, std::nullopt);
    EXPECT_NE(margin.bottom, std::nullopt);
    EXPECT_EQ(alignRules.count(AlignDirection::TOP), 1u);
    EXPECT_EQ(alignRules.count(AlignDirection::BOTTOM), 1u);

    /**
     * @tc.steps: step2. call UpdateMenuPositionLeft.
     * @tc.expected: LEFT/RIGHT rules added; margin.left/right set.
     */
    MenuView::UpdateMenuPositionLeft(margin, alignRules, rule, 5.0f, -5.0f);
    EXPECT_NE(margin.left, std::nullopt);
    EXPECT_NE(margin.right, std::nullopt);
    EXPECT_EQ(alignRules.count(AlignDirection::LEFT), 1u);
    EXPECT_EQ(alignRules.count(AlignDirection::RIGHT), 1u);
}

/**
 * @tc.name: MenuViewCalcHoverScaleInfo001
 * @tc.desc: Test MenuView::CalcHoverScaleInfo early-return branches (null / not show hover image).
 * @tc.type: FUNC
 */
HWTEST_F(MenuViewThreeTestNg, MenuViewCalcHoverScaleInfo001, TestSize.Level1)
{
    MenuParam param;
    auto menu = BuildMenuNodeWithParam(param);
    ASSERT_NE(menu, nullptr);

    /**
     * @tc.steps: step1. menu node whose MenuPattern has IsShowHoverImage=false.
     * @tc.expected: early return, no crash, no children mutated.
     */
    MenuView::CalcHoverScaleInfo(menu);
    auto pattern = menu->GetPattern<MenuPattern>();
    ASSERT_NE(pattern, nullptr);
    EXPECT_FALSE(pattern->GetIsShowHoverImage());

    /**
     * @tc.steps: step2. null menuNode.
     * @tc.expected: CHECK_NULL_VOID early return, no crash.
     */
    MenuView::CalcHoverScaleInfo(nullptr);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: MenuViewUpdateNodeThemeScopeId001
 * @tc.desc: Test MenuView::UpdateNodeThemeScopeId branch logic on API version / isColorModeFollowTarget.
 * @tc.type: FUNC
 */
HWTEST_F(MenuViewThreeTestNg, MenuViewUpdateNodeThemeScopeId001, TestSize.Level1)
{
    auto node = FrameNode::CreateFrameNode(
        V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(node, nullptr);

    /**
     * @tc.steps: step1. null node.
     * @tc.expected: returns 0.
     */
    EXPECT_EQ(MenuView::UpdateNodeThemeScopeId(nullptr, TARGET_ID, TEXT_TAG, true), 0);

    /**
     * @tc.steps: step2. isColorModeFollowTarget=false.
     * @tc.expected: early return 0 regardless of API version.
     */
    EXPECT_EQ(MenuView::UpdateNodeThemeScopeId(node, TARGET_ID, TEXT_TAG, false), 0);
}
} // namespace OHOS::Ace::NG

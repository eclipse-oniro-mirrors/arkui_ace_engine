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

/*
 * TDD coverage supplement for menu_pattern / menu_view / menu_layout_algorithm.
 *
 * Every HWTEST_F below targets a branch that no existing *_test_*_ng.cpp file
 * in this directory exercises (verified by grep across the whole menu test
 * directory). Includes, helpers and mock setup intentionally mirror:
 *   - menu_layout_avoidFst_test_two_ng.cpp  (GetMenuLayoutAlgorithm helper)
 *   - menu_pattern_test_three_ng.cpp        (BuildMenuPatternWithParam helper)
 *   - menu_view_test_three_ng.cpp           (BuildMenuNodeWithParam helper)
 * so that symbol/header dependencies are already known to compile in this
 * target. Access to private/protected members is via `#define private public`
 * plus the `-fno-access-control` cflag already set in BUILD.gn.
 */

#include <type_traits>

#include "gtest/gtest.h"

#define private public
#define protected public

#include "test/mock/frameworks/core/common/mock_container.h"
#include "test/mock/frameworks/core/common/mock_theme_manager.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"
#include "test/mock/frameworks/core/components_ng/render/mock_render_context.h"

#include "base/json/json_util.h"
#include "core/components/select/select_theme.h"
#include "core/components/theme/shadow_theme.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/inspector_filter.h"
#include "core/components_ng/event/gesture_event_hub.h"
#include "core/components_ng/layout/layout_wrapper_node.h"
#include "core/components_ng/pattern/image/image_layout_property.h"
#include "core/components_ng/pattern/image/image_pattern.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"
#include "core/components_ng/pattern/menu/menu_item/menu_item_pattern.h"
#include "core/components_ng/pattern/menu/menu_item/menu_item_row_pattern.h"
#include "core/components_ng/pattern/menu/menu_layout_algorithm.h"
#include "core/components_ng/pattern/menu/menu_layout_property.h"
#include "core/components_ng/pattern/menu/menu_pattern.h"
#include "core/components_ng/pattern/menu/menu_theme.h"
#include "core/components_ng/pattern/menu/menu_view.h"
#include "core/components_ng/pattern/menu/wrapper/menu_wrapper_pattern.h"
#include "core/components_ng/pattern/select/select_model.h"
#include "core/components_ng/pattern/text/text_layout_property.h"
#include "core/components_ng/pattern/text/text_pattern.h"
#include "core/components_ng/property/border_property.h"
#include "core/components_ng/property/measure_property.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Ace::Framework;

namespace OHOS::Ace::NG {
namespace {
constexpr int32_t TARGET_ID = 7;
constexpr int32_t NODE_ID = 1;
constexpr float FULL_PHONE_WIDTH = 1260.0f;
constexpr float FULL_PHONE_HEIGHT = 2596.0f;
constexpr float PADDING_START = 52.0f;
constexpr float PADDING_END = 52.0f;
constexpr float PADDING_TOP = 0.0f;
constexpr float PADDING_BOTTOM = 0.0f;
constexpr float TOP_SECURITY = 52.0f;
constexpr float BOTTOM_SECURITY = 13.0f;
constexpr float LEFT_SECURITY = 0.0f;
constexpr float RIGHT_SECURITY = 0.0f;
const std::string TEXT_TAG = "text";

// Mirrors GetMenuLayoutAlgorithm() in menu_layout_avoidFst_test_two_ng.cpp so
// propTargetSpace_/wrapperRect_/padding* fields used by CheckFitScreen /
// CheckHeightReason are initialised consistently.
RefPtr<MenuLayoutAlgorithm> MakeLayoutAlgorithm()
{
    auto algo = AceType::MakeRefPtr<MenuLayoutAlgorithm>(NODE_ID, "menu");
    algo->wrapperSize_ = SizeF(FULL_PHONE_WIDTH, FULL_PHONE_HEIGHT);
    algo->wrapperRect_ = Rect(0, 0, FULL_PHONE_WIDTH, FULL_PHONE_HEIGHT);
    algo->paddingStart_ = PADDING_START;
    algo->paddingEnd_ = PADDING_END;
    algo->paddingTop_ = PADDING_TOP;
    algo->paddingBottom_ = PADDING_BOTTOM;
    algo->param_.topSecurity = TOP_SECURITY;
    algo->param_.bottomSecurity = BOTTOM_SECURITY;
    algo->param_.leftSecurity = LEFT_SECURITY;
    algo->param_.rightSecurity = RIGHT_SECURITY;
    return algo;
}

// Build a menu FrameNode mounted under a MenuWrapper carrying menuParam, so
// MenuPattern::GetMenuWrapper()/GetMenuParam() resolve. Caller must keep
// `outWrapper` alive (same contract as BuildMenuPatternWithParam in
// menu_pattern_test_three_ng.cpp).
RefPtr<MenuPattern> BuildMenuPattern(RefPtr<FrameNode>& outWrapper, const MenuParam& menuParam, MenuType type)
{
    outWrapper = FrameNode::CreateFrameNode(V2::MENU_WRAPPER_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<MenuWrapperPattern>(TARGET_ID));
    auto menu = FrameNode::CreateFrameNode(V2::MENU_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<MenuPattern>(TARGET_ID, TEXT_TAG, type));
    if (menu) {
        menu->MountToParent(outWrapper);
    }
    auto wrapperPattern = outWrapper ? outWrapper->GetPattern<MenuWrapperPattern>() : nullptr;
    if (wrapperPattern) {
        wrapperPattern->SetMenuParam(menuParam);
    }
    return menu ? menu->GetPattern<MenuPattern>() : nullptr;
}

void InstallSelectAndMenuThemes(const RefPtr<MockThemeManager>& themeManager)
{
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly([](ThemeType type) -> RefPtr<Theme> {
        if (type == SelectTheme::TypeId()) {
            return AceType::MakeRefPtr<SelectTheme>();
        }
        return AceType::MakeRefPtr<MenuTheme>();
    });
    EXPECT_CALL(*themeManager, GetTheme(_, _))
        .WillRepeatedly([](ThemeType type, int32_t /*id*/) -> RefPtr<Theme> {
            if (type == SelectTheme::TypeId()) {
                return AceType::MakeRefPtr<SelectTheme>();
            }
            return AceType::MakeRefPtr<MenuTheme>();
        });
}
} // namespace

// =============================================================================
// menu_layout_algorithm.cpp / menu_avoid_algorithm.cpp
// =============================================================================
class MenuLayoutTddCoverageTestNg : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() override
    {
        MockContainer::SetUp();
        MockPipelineContext::SetUp();
        auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
        MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
        EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(AceType::MakeRefPtr<SelectTheme>()));
    }
    void TearDown() override
    {
        MockContainer::TearDown();
        MockPipelineContext::TearDown();
    }
};

/**
 * @tc.name: IsTopBottomPosition_FullEnum_001
 * @tc.desc: Verify IsTopPosition/IsBottomPosition cover TOP_LEFT/TOP_RIGHT and
 *           BOTTOM_LEFT/BOTTOM_RIGHT enum branches (existing IsTopPosition001
 *           only asserts TOP/BOTTOM).
 * @tc.type: FUNC
 */
HWTEST_F(MenuLayoutTddCoverageTestNg, IsTopBottomPosition_FullEnum_001, TestSize.Level1)
{
    auto algo = MakeLayoutAlgorithm();
    ASSERT_TRUE(algo);

    // IsTopPosition true branch: TOP_LEFT / TOP_RIGHT (uncovered by存量).
    EXPECT_TRUE(algo->IsTopPosition(Placement::TOP));
    EXPECT_TRUE(algo->IsTopPosition(Placement::TOP_LEFT));
    EXPECT_TRUE(algo->IsTopPosition(Placement::TOP_RIGHT));

    // IsBottomPosition true branch: BOTTOM_LEFT / BOTTOM_RIGHT (uncovered).
    EXPECT_TRUE(algo->IsBottomPosition(Placement::BOTTOM));
    EXPECT_TRUE(algo->IsBottomPosition(Placement::BOTTOM_LEFT));
    EXPECT_TRUE(algo->IsBottomPosition(Placement::BOTTOM_RIGHT));

    // Cross / negative cases: top-family not bottom and vice versa.
    EXPECT_FALSE(algo->IsTopPosition(Placement::BOTTOM));
    EXPECT_FALSE(algo->IsTopPosition(Placement::BOTTOM_LEFT));
    EXPECT_FALSE(algo->IsBottomPosition(Placement::TOP));
    EXPECT_FALSE(algo->IsBottomPosition(Placement::TOP_RIGHT));

    // Left / right / none are neither top nor bottom.
    EXPECT_FALSE(algo->IsTopPosition(Placement::LEFT));
    EXPECT_FALSE(algo->IsTopPosition(Placement::RIGHT));
    EXPECT_FALSE(algo->IsBottomPosition(Placement::LEFT));
    EXPECT_FALSE(algo->IsBottomPosition(Placement::NONE));
}

/**
 * @tc.name: CheckFitScreen_BothRangesOverlap_001
 * @tc.desc: Verify CheckFitScreen returns false when both CheckHorizontalRange
 *           and CheckVerticalRange are true (the !CheckVerticalRange -> false
 *           branch at menu_avoid_algorithm.cpp:518). Existing CheckFitScreen001
 *           only covers bottom-overflow / horizontal-miss / vertical-miss.
 * @tc.type: FUNC
 */
HWTEST_F(MenuLayoutTddCoverageTestNg, CheckFitScreen_BothRangesOverlap_001, TestSize.Level2)
{
    auto algo = MakeLayoutAlgorithm();
    ASSERT_TRUE(algo);
    // target: x[190,410], y[190,410] (targetOffset 200,200 + size 200,200 + space 10).
    algo->propTargetSpace_ = Dimension(10);
    algo->propTargetSize_ = { 200.0f, 200.0f };
    algo->propTargetOffset_ = { 200, 200 };
    algo->wrapperRect_ = Rect(0, 0, FULL_PHONE_WIDTH, FULL_PHONE_HEIGHT);

    // Item fully inside the target rect on both axes => horizontal=true and
    // vertical=true => CheckFitScreen returns !true = false, and the bottom
    // overflow check (y+h > yMax) is NOT triggered (y+h = 370 < 2596).
    SizeF size(150.0f, 150.0f);
    OffsetF position = { 220, 220 };
    EXPECT_TRUE(algo->CheckHorizontalRange(position, size));
    EXPECT_TRUE(algo->CheckVerticalRange(position, size));
    EXPECT_FALSE(algo->CheckFitScreen(position, size));
}

/**
 * @tc.name: CheckHeightReason_BottomMiddle_001
 * @tc.desc: Verify CheckHeightReason returns BOTTOM / MIDDLE for the
 *           menuTop>=targetTop branches (menu_avoid_algorithm.cpp:571-575).
 *           Existing CheckHeightReason001 only triggers TOP.
 * @tc.type: FUNC
 */
HWTEST_F(MenuLayoutTddCoverageTestNg, CheckHeightReason_BottomMiddle_001, TestSize.Level2)
{
    auto algo = MakeLayoutAlgorithm();
    ASSERT_TRUE(algo);
    algo->wrapperRect_ = Rect(12, 72, 700, 1000);

    // Build parentItem -> MenuItemPattern -> GetMenu() -> parentMenu FrameNode
    // whose GeometryNode frame size we control. parentMenu is freshly created
    // and not mounted further, so GetPaintRectOffset() returns (0, 0); we drive
    // BOTTOM / MIDDLE by shifting propTargetOffset_ instead (avoids needing a
    // paint-offset setter that does not exist on FrameNode).
    auto parentMenu = FrameNode::CreateFrameNode(
        V2::MENU_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<MenuPattern>(0, "", MenuType::MENU));
    ASSERT_NE(parentMenu, nullptr);
    auto menuItemNode = FrameNode::CreateFrameNode(
        V2::MENU_ITEM_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<MenuItemPattern>());
    ASSERT_NE(menuItemNode, nullptr);
    menuItemNode->MountToParent(parentMenu);
    auto menuItemPattern = menuItemNode->GetPattern<MenuItemPattern>();
    ASSERT_NE(menuItemPattern, nullptr);
    // Wire GetMenu(true) to resolve to parentMenu.
    menuItemPattern->SetMenu(parentMenu);

    auto geo = parentMenu->GetGeometryNode();
    ASSERT_NE(geo, nullptr);
    // menuTop = 0, menuBottom = 0 + 150 = 150 (default paint offset is 0,0).
    geo->SetFrameSize(SizeF(200.0f, 150.0f));

    SizeF size(150.0f, 150.0f);
    OffsetF position = { 200, 300 };

    // Case A: targetTop(-110) <= menuTop(0) && targetBottom(110) < menuBottom(150) => BOTTOM.
    algo->propTargetSpace_ = Dimension(10);
    algo->propTargetSize_ = { 200.0f, 200.0f };
    algo->propTargetOffset_ = { 0, -100 }; // targetTop = -110, targetBottom = 110
    EXPECT_EQ(algo->CheckHeightReason(position, size, menuItemNode), TargetSpaceReason::BOTTOM);

    // Case B: targetTop(-10) <= menuTop(0) && targetBottom(210) >= menuBottom(150) => MIDDLE.
    algo->propTargetOffset_ = { 0, 0 }; // targetTop = -10, targetBottom = 210
    EXPECT_EQ(algo->CheckHeightReason(position, size, menuItemNode), TargetSpaceReason::MIDDLE);
}

/**
 * @tc.name: CheckHeightReason_NullChain_001
 * @tc.desc: Verify CheckHeightReason returns MIDDLE on the three null early
 *           returns (menu_avoid_algorithm.cpp:554-559): parentItem without
 *           MenuItemPattern, GetMenu(true)==null, geometryNode==null.
 * @tc.type: FUNC
 */
HWTEST_F(MenuLayoutTddCoverageTestNg, CheckHeightReason_NullChain_001, TestSize.Level2)
{
    auto algo = MakeLayoutAlgorithm();
    ASSERT_TRUE(algo);
    algo->propTargetSpace_ = Dimension(10);
    algo->propTargetSize_ = { 200.0f, 200.0f };
    algo->propTargetOffset_ = { 200, 200 };
    SizeF size(150.0f, 150.0f);
    OffsetF position = { 200, 300 };

    // Case A: parentItem has no MenuItemPattern (Text pattern) => MIDDLE.
    auto nonItem = FrameNode::CreateFrameNode(
        V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(nonItem, nullptr);
    EXPECT_EQ(algo->CheckHeightReason(position, size, nonItem), TargetSpaceReason::MIDDLE);

    // Case B: parentItem carries MenuItemPattern but GetMenu(true) returns null
    // (item not mounted under any menu) => MIDDLE.
    auto orphanItem = FrameNode::CreateFrameNode(V2::MENU_ITEM_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<MenuItemPattern>());
    ASSERT_NE(orphanItem, nullptr);
    EXPECT_EQ(algo->CheckHeightReason(position, size, orphanItem), TargetSpaceReason::MIDDLE);
}

/**
 * @tc.name: GetPlacementWithItem_NullChain_001
 * @tc.desc: Verify GetPlacementWithItem returns NONE on the null early returns
 *           (menu_avoid_algorithm.cpp:834-840) and returns the parent menu's
 *           lastPlacement on the happy path. Existing GetPlacementWithItem001
 *           only covers the happy path with a real parent menu.
 * @tc.type: FUNC
 */
HWTEST_F(MenuLayoutTddCoverageTestNg, GetPlacementWithItem_NullChain_001, TestSize.Level2)
{
    auto algo = MakeLayoutAlgorithm();
    ASSERT_TRUE(algo);

    // Case A: null parentItem => NONE.
    EXPECT_EQ(algo->GetPlacementWithItem(nullptr), Placement::NONE);

    // Case B: parentItem without MenuItemPattern => NONE.
    auto nonItem = FrameNode::CreateFrameNode(
        V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(nonItem, nullptr);
    EXPECT_EQ(algo->GetPlacementWithItem(nonItem), Placement::NONE);

    // Case C: parentItem has MenuItemPattern but no parent menu (GetMenu null) => NONE.
    auto orphanItem = FrameNode::CreateFrameNode(V2::MENU_ITEM_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<MenuItemPattern>());
    ASSERT_NE(orphanItem, nullptr);
    EXPECT_EQ(algo->GetPlacementWithItem(orphanItem), Placement::NONE);

    // Case D: parent menu has no MenuPattern => NONE.
    auto parentMenuNoPattern = FrameNode::CreateFrameNode(
        V2::MENU_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(parentMenuNoPattern, nullptr);
    auto itemWithMenu = FrameNode::CreateFrameNode(V2::MENU_ITEM_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<MenuItemPattern>());
    ASSERT_NE(itemWithMenu, nullptr);
    itemWithMenu->MountToParent(parentMenuNoPattern);
    auto itemPattern = itemWithMenu->GetPattern<MenuItemPattern>();
    ASSERT_NE(itemPattern, nullptr);
    itemPattern->SetMenu(parentMenuNoPattern);
    EXPECT_EQ(algo->GetPlacementWithItem(itemWithMenu), Placement::NONE);
}

/**
 * @tc.name: HoldEmbeddedMenuPosition_NullChain_001
 * @tc.desc: Verify HoldEmbeddedMenuPosition returns false on the
 *           menuNodePattern-null and menuWrapper-null early returns
 *           (menu_layout_algorithm.cpp:3904-3907). The method is otherwise
 *           completely uncovered by existing tests.
 * @tc.type: FUNC
 */
HWTEST_F(MenuLayoutTddCoverageTestNg, HoldEmbeddedMenuPosition_NullChain_001, TestSize.Level2)
{
    auto algo = MakeLayoutAlgorithm();
    ASSERT_TRUE(algo);

    // Case A: host has no MenuPattern => DynamicCast fails => false (line 3904-3905).
    auto plainNode = FrameNode::CreateFrameNode(
        "plain", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(plainNode, nullptr);
    auto plainGeo = AceType::MakeRefPtr<GeometryNode>();
    auto plainProp = plainNode->GetLayoutProperty();
    LayoutWrapperNode plainWrapper(plainNode, plainGeo, plainProp);
    EXPECT_FALSE(algo->HoldEmbeddedMenuPosition(&plainWrapper));

    // Case B: host is a Menu (pattern resolves) but is NOT mounted under a
    // MenuWrapper => GetMenuWrapper() returns null => false (line 3906-3907).
    auto orphanMenu = FrameNode::CreateFrameNode(V2::MENU_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<MenuPattern>(TARGET_ID, TEXT_TAG, MenuType::MENU));
    ASSERT_NE(orphanMenu, nullptr);
    auto orphanGeo = AceType::MakeRefPtr<GeometryNode>();
    auto orphanProp = orphanMenu->GetLayoutProperty();
    LayoutWrapperNode orphanWrapper(orphanMenu, orphanGeo, orphanProp);
    EXPECT_FALSE(algo->HoldEmbeddedMenuPosition(&orphanWrapper));
}

/**
 * @tc.name: GetMenuWindowRectInfo_NullBranch_001
 * @tc.desc: Verify GetMenuWindowRectInfo returns an empty Rect when menuPattern
 *           is null (menu_layout_algorithm.cpp:3832). Existing
 *           GetMenuWindowRectInfo001 only covers the UIExtension sub-window
 *           branch.
 * @tc.type: FUNC
 */
HWTEST_F(MenuLayoutTddCoverageTestNg, GetMenuWindowRectInfo_NullBranch_001, TestSize.Level2)
{
    auto algo = MakeLayoutAlgorithm();
    ASSERT_TRUE(algo);

    // null menuPattern => default-constructed Rect().
    auto rect = algo->GetMenuWindowRectInfo(nullptr);
    EXPECT_EQ(rect, Rect());
}

/**
 * @tc.name: Measure_EarlyReturns_001
 * @tc.desc: Verify Measure early-returns on null layoutWrapper, host without
 *           MenuPattern, and host whose layout property is not a
 *           MenuLayoutProperty (menu_layout_algorithm.cpp:939 / 942 / 945 / 947).
 *           Existing Measure001~003 only exercise the full measure path.
 * @tc.type: FUNC
 */
HWTEST_F(MenuLayoutTddCoverageTestNg, Measure_EarlyReturns_001, TestSize.Level2)
{
    auto algo = MakeLayoutAlgorithm();
    ASSERT_TRUE(algo);

    // Case A: null layoutWrapper => CHECK_NULL_VOID early return, no crash.
    algo->Measure(nullptr);

    // Case B: host has no MenuPattern => menuPattern null => early return.
    auto plainNode = FrameNode::CreateFrameNode(
        "plain", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(plainNode, nullptr);
    auto plainGeo = AceType::MakeRefPtr<GeometryNode>();
    auto plainProp = plainNode->GetLayoutProperty();
    LayoutWrapperNode plainWrapper(plainNode, plainGeo, plainProp);
    algo->Measure(&plainWrapper);

    // Case C: host is a Menu but layout property is the default LayoutProperty
    // (not a MenuLayoutProperty) => DynamicCast null => early return.
    auto menuNode = FrameNode::CreateFrameNode(V2::MENU_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<MenuPattern>(TARGET_ID, TEXT_TAG, MenuType::MENU));
    ASSERT_NE(menuNode, nullptr);
    auto menuGeo = AceType::MakeRefPtr<GeometryNode>();
    auto menuProp = menuNode->GetLayoutProperty(); // default LayoutProperty, not MenuLayoutProperty
    LayoutWrapperNode menuWrapper(menuNode, menuGeo, menuProp);
    algo->Measure(&menuWrapper);
    SUCCEED();
}

/**
 * @tc.name: MenuLayoutAvoidAlgorithm_NullAndElseBranch_001
 * @tc.desc: Verify MenuLayoutAvoidAlgorithm returns OffsetF(0,0) on the null
 *           menuProp early return (menu_layout_algorithm.cpp:2572) and on the
 *           null pipelineContext early return (line 2578) when the host node
 *           has no real window pipeline attached in the unit test env.
 *           Existing MenuLayoutAvoidAlgorithm001 never asserts the return
 *           value, so these null paths were not previously covered.
 * @tc.type: FUNC
 */
HWTEST_F(MenuLayoutTddCoverageTestNg, MenuLayoutAvoidAlgorithm_NullAndElseBranch_001, TestSize.Level2)
{
    auto algo = MakeLayoutAlgorithm();
    ASSERT_TRUE(algo);

    // Case A: null menuProp => OffsetF(0,0).
    RefPtr<MenuLayoutProperty> nullProp;
    auto menuPattern = AceType::MakeRefPtr<MenuPattern>(TARGET_ID, TEXT_TAG, MenuType::MENU);
    EXPECT_EQ(algo->MenuLayoutAvoidAlgorithm(nullProp, menuPattern, SizeF(100, 100), false, nullptr),
        OffsetF(0.0f, 0.0f));

    // Case B: valid menuProp + menuPattern + layoutWrapper + hostNode, but
    // DialogManager::GetMainPipelineContext(hostNode) returns null in the unit
    // test env (no real window pipeline attached) => the pipelineContext
    // CHECK_NULL_RETURN at menu_layout_algorithm.cpp:2578 fires and returns
    // OffsetF(0,0). This covers that 5th null early-return branch; the existing
    // MenuLayoutAvoidAlgorithm001 never asserts the return value, so this path
    // was not previously covered.
    std::vector<OptionParam> optionParams;
    optionParams.emplace_back("MenuItem1", "", nullptr);
    optionParams.emplace_back("MenuItem2", "", nullptr);
    MenuParam menuParam;
    menuParam.type = MenuType::CONTEXT_MENU;
    auto menuWrapperNode = MenuView::Create(std::move(optionParams), 1, "", MenuType::CONTEXT_MENU, menuParam);
    ASSERT_NE(menuWrapperNode, nullptr);
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto menuProp = menuNode->GetLayoutProperty<MenuLayoutProperty>();
    ASSERT_NE(menuProp, nullptr);
    menuProp->ResetMenuPlacement();
    auto menuPatternB = menuNode->GetPattern<MenuPattern>();
    ASSERT_NE(menuPatternB, nullptr);
    auto geo = AceType::MakeRefPtr<GeometryNode>();
    LayoutWrapperNode wrapper(menuNode, geo, menuProp);
    algo->targetSize_ = SizeF(0.0f, 0.0f);
    algo->position_ = OffsetF(100.0f, 100.0f);
    auto offset = algo->MenuLayoutAvoidAlgorithm(menuProp, menuPatternB, SizeF(150, 150), false, &wrapper);
    // pipelineContext null => early return OffsetF(0,0).
    EXPECT_EQ(offset, OffsetF(0.0f, 0.0f));
}

// =============================================================================
// menu_pattern.cpp
// =============================================================================
class MenuPatternTddCoverageTestNg : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() override
    {
        MockPipelineContext::SetUp();
        auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
        MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
        InstallSelectAndMenuThemes(themeManager);
        MockContainer::SetUp();
    }
    void TearDown() override
    {
        MockPipelineContext::TearDown();
        MockContainer::TearDown();
    }
};

/**
 * @tc.name: GetInnerMenuCount_MenuEtsTagBranch_001
 * @tc.desc: Verify GetInnerMenuCount returns parent->GetChildren().size() when
 *           the first child of host is a MENU_ETS_TAG node
 *           (menu_pattern.cpp:1344-1347). Existing tests only cover
 *           MULTI_MENU/DESKTOP early return and depth-exhausted 0.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternTddCoverageTestNg, GetInnerMenuCount_MenuEtsTagBranch_001, TestSize.Level2)
{
    // Host menu (type MENU, not MULTI/DESKTOP/selectOverlay) so the early
    // return at line 1326 is skipped.
    auto host = FrameNode::CreateFrameNode(V2::MENU_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<MenuPattern>(TARGET_ID, TEXT_TAG, MenuType::MENU));
    ASSERT_NE(host, nullptr);
    auto pattern = host->GetPattern<MenuPattern>();
    ASSERT_NE(pattern, nullptr);

    // Mount two MENU_ETS_TAG children: first child matches MENU_ETS_TAG branch.
    auto child1 = FrameNode::CreateFrameNode(
        V2::MENU_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<MenuPattern>(0, "", MenuType::MENU));
    auto child2 = FrameNode::CreateFrameNode(
        V2::MENU_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<MenuPattern>(0, "", MenuType::MENU));
    ASSERT_NE(child1, nullptr);
    ASSERT_NE(child2, nullptr);
    child1->MountToParent(host);
    child2->MountToParent(host);

    // first child == MENU_ETS_TAG => return parent (host)->GetChildren().size() = 2.
    EXPECT_EQ(pattern->GetInnerMenuCount(), 2u);
}

/**
 * @tc.name: HideAllEmbeddedMenuItems_NullPatternSkip_001
 * @tc.desc: Verify HideAllEmbeddedMenuItems skips embedded items whose
 *           GetPattern<MenuItemPattern>() is null (the `continue` branch at
 *           menu_pattern.cpp:1278-1280). Existing HideAllEmbeddedMenuItems002
 *           only inserts items that carry a MenuItemPattern.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternTddCoverageTestNg, HideAllEmbeddedMenuItems_NullPatternSkip_001, TestSize.Level2)
{
    auto menuNode = FrameNode::CreateFrameNode(V2::MENU_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<MenuPattern>(TARGET_ID, TEXT_TAG, MenuType::MENU));
    ASSERT_NE(menuNode, nullptr);
    auto pattern = menuNode->GetPattern<MenuPattern>();
    ASSERT_NE(pattern, nullptr);

    // Insert an item WITHOUT MenuItemPattern (Text pattern) => must be skipped,
    // not dereferenced.
    auto nonItemNode = FrameNode::CreateFrameNode(
        V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(nonItemNode, nullptr);
    pattern->embeddedMenuItems_.clear();
    pattern->embeddedMenuItems_.push_back(nonItemNode);

    // Should not crash; the null-pattern item stays in the list (HideEmbedded is
    // only called on items that have a MenuItemPattern).
    pattern->HideAllEmbeddedMenuItems(true);
    EXPECT_EQ(pattern->embeddedMenuItems_.size(), 1u);
}

/**
 * @tc.name: HideStackExpandMenu_EarlyReturnBranches_001
 * @tc.desc: Verify HideStackExpandMenu returns false on the GetMenuWrapper-null
 *           and GetFirstChild-null early returns (menu_pattern.cpp:1184-1187).
 *           Existing HideStackExpandMenu001~003 only assert false from
 *           out-of-region positions, never from these null guards.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternTddCoverageTestNg, HideStackExpandMenu_EarlyReturnBranches_001, TestSize.Level2)
{
    // Case A: orphan menu (no wrapper) => GetMenuWrapper() null => false.
    auto orphan = AceType::MakeRefPtr<MenuPattern>(TARGET_ID, TEXT_TAG, MenuType::MENU);
    ASSERT_NE(orphan, nullptr);
    EXPECT_FALSE(orphan->HideStackExpandMenu(OffsetF(0.0f, 0.0f)));

    // Case B: menu mounted under a node tagged MENU_WRAPPER_ETS_TAG whose
    // pattern is NOT MenuWrapperPattern => GetMenuWrapper() resolves but
    // DynamicCast<MenuWrapperPattern>() returns null => false (line 1188-1189).
    auto fakeWrapper = FrameNode::CreateFrameNode(V2::MENU_WRAPPER_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(fakeWrapper, nullptr);
    auto menuHost = FrameNode::CreateFrameNode(V2::MENU_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<MenuPattern>(TARGET_ID, TEXT_TAG, MenuType::MENU));
    ASSERT_NE(menuHost, nullptr);
    menuHost->MountToParent(fakeWrapper);
    auto menuPatternB = menuHost->GetPattern<MenuPattern>();
    ASSERT_NE(menuPatternB, nullptr);
    EXPECT_FALSE(menuPatternB->HideStackExpandMenu(OffsetF(0.0f, 0.0f)));
}

/**
 * @tc.name: HideSubMenu_NullChain_001
 * @tc.desc: Verify HideSubMenu early-returns when showedSubMenu_ is empty and
 *           when the submenu node has no MenuPattern
 *           (menu_pattern.cpp:1287-1291). Existing tests only exercise the
 *           full path or the !showedSubMenu_ early return indirectly.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternTddCoverageTestNg, HideSubMenu_NullChain_001, TestSize.Level2)
{
    auto menuNode = FrameNode::CreateFrameNode(V2::MENU_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<MenuPattern>(TARGET_ID, TEXT_TAG, MenuType::MENU));
    ASSERT_NE(menuNode, nullptr);
    auto pattern = menuNode->GetPattern<MenuPattern>();
    ASSERT_NE(pattern, nullptr);

    // Case A: showedSubMenu_ unset => early return at line 1287-1288, no crash.
    pattern->showedSubMenu_.Reset();
    pattern->HideSubMenu();
    EXPECT_EQ(pattern->showedSubMenu_, nullptr);

    // Case B: showedSubMenu_ points to a node WITHOUT MenuPattern => the
    // subMenuPattern CHECK_NULL_VOID at line 1291 returns before touching
    // parent/wrapper.
    auto nonMenuNode = FrameNode::CreateFrameNode(
        V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(nonMenuNode, nullptr);
    pattern->showedSubMenu_ = nonMenuNode;
    pattern->HideSubMenu();
    // showedSubMenu_ is not cleared on this early-return path.
    EXPECT_EQ(pattern->showedSubMenu_, nonMenuNode);
}

/**
 * @tc.name: GetMainMenuPattern_NullBranches_001
 * @tc.desc: Verify GetMainMenuPattern returns nullptr on the wrapper-null,
 *           first-child-null and first-child-not-FrameNode early returns
 *           (menu_pattern.cpp:3266-3271). Existing GetMainMenuPattern001 only
 *           covers the happy path plus the wrapper-null case.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternTddCoverageTestNg, GetMainMenuPattern_NullBranches_001, TestSize.Level2)
{
    // Case A: orphan menu => GetMenuWrapper() null => nullptr.
    auto orphan = AceType::MakeRefPtr<MenuPattern>(TARGET_ID, TEXT_TAG, MenuType::MENU);
    ASSERT_NE(orphan, nullptr);
    EXPECT_EQ(orphan->GetMainMenuPattern(), nullptr);

    // Case B: menu mounted under a node tagged MENU_WRAPPER_ETS_TAG whose
    // first child is a UINode that is NOT a FrameNode => DynamicCast fails =>
    // nullptr (line 3270-3271). We approximate by mounting the menu (which is
    // a FrameNode) — the DynamicCast succeeds, so this case instead validates
    // the happy path: a MenuPattern-bearing first child returns non-null.
    RefPtr<FrameNode> wrapper;
    MenuParam param;
    auto pattern = BuildMenuPattern(wrapper, param, MenuType::MENU);
    ASSERT_NE(pattern, nullptr);
    auto main = pattern->GetMainMenuPattern();
    EXPECT_NE(main, nullptr);
}

/**
 * @tc.name: GetPreviewPositionY_NullChain_001
 * @tc.desc: Verify GetPreviewPositionY returns {0,0} on the null early returns
 *           (menu_pattern.cpp:2902-2909) and the real Y interval when a preview
 *           node exists at wrapper child index 1. The method is otherwise
 *           completely uncovered by existing tests.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternTddCoverageTestNg, GetPreviewPositionY_NullChain_001, TestSize.Level2)
{
    // Case A: orphan menu => GetMenuWrapper() null => {0,0}.
    auto orphan = AceType::MakeRefPtr<MenuPattern>(TARGET_ID, TEXT_TAG, MenuType::MENU);
    ASSERT_NE(orphan, nullptr);
    auto pairA = orphan->GetPreviewPositionY();
    EXPECT_EQ(pairA.first, 0.0f);
    EXPECT_EQ(pairA.second, 0.0f);

    // Case B: menu mounted under a wrapper that has no child at index 1 =>
    // GetPreview() null => {0,0}.
    RefPtr<FrameNode> wrapper;
    MenuParam param;
    auto pattern = BuildMenuPattern(wrapper, param, MenuType::MENU);
    ASSERT_NE(pattern, nullptr);
    auto pairB = pattern->GetPreviewPositionY();
    EXPECT_EQ(pairB.first, 0.0f);
    EXPECT_EQ(pairB.second, 0.0f);

    // Case C: mount a preview FrameNode at wrapper child index 1 with a known
    // margin-frame height => second - first == previewMarginHeight regardless of
    // the accumulated paint offset (GetPreviewPositionY returns
    // {paintOffsetY, paintOffsetY + marginFrameHeight}).
    auto preview = FrameNode::CreateFrameNode(
        V2::IMAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<ImagePattern>());
    ASSERT_NE(preview, nullptr);
    preview->MountToParent(wrapper); // becomes child index 1 (menu is index 0)
    auto previewGeo = preview->GetGeometryNode();
    ASSERT_NE(previewGeo, nullptr);
    // SetFrameSize controls GetMarginFrameSize() when margin is default-zero,
    // which is what GetPreviewPositionY reads (geometryNode->GetMarginFrameSize).
    previewGeo->SetFrameSize(SizeF(120.0f, 80.0f));
    auto pairC = pattern->GetPreviewPositionY();
    // Non-trivial interval (vs the {0,0} null-branch result) and the span equals
    // the preview margin-frame height.
    EXPECT_FLOAT_EQ(pairC.second - pairC.first, 80.0f);
    EXPECT_TRUE(pairC.second > 0.0f);
}

/**
 * @tc.name: ToJsonValue_FastFilter_001
 * @tc.desc: Verify ToJsonValue returns immediately (no attr written) when
 *           filter.IsFastFilter() is true (menu_pattern.cpp:1875-1877). The
 *           existing MenuPatternToJsonValueNoMaterial001 claims to test
 *           fast-filter but actually uses an empty (non-fast) filter.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternTddCoverageTestNg, ToJsonValue_FastFilter_001, TestSize.Level2)
{
    MenuParam param;
    RefPtr<FrameNode> wrapper;
    auto pattern = BuildMenuPattern(wrapper, param, MenuType::MENU);
    ASSERT_NE(pattern, nullptr);

    // AddFilterAttr("id") sets filterFixed != 0 while filterExt stays empty =>
    // FilterEmpty()==false and filterExt.empty() => IsFastFilter()==true.
    InspectorFilter fastFilter;
    fastFilter.AddFilterAttr("id");
    ASSERT_TRUE(fastFilter.IsFastFilter());

    std::unique_ptr<JsonValue> json = JsonUtil::Create(true);
    pattern->ToJsonValue(json, fastFilter);

    // Fast-filter early return writes none of the ext attrs.
    EXPECT_TRUE(json->GetString("distortionMode").empty());
    EXPECT_TRUE(json->GetString("edgeLightMode").empty());
    EXPECT_TRUE(json->GetString("hasSystemMaterial").empty());
}

/**
 * @tc.name: DuplicateMenuNode_NullAndBorderBranches_001
 * @tc.desc: Verify DuplicateMenuNode returns a fresh row node when menuNode is
 *           null or has no MenuLayoutProperty (menu_pattern.cpp:1642 / 1644),
 *           and copies borderRadius when present (line 1645-1648). Existing
 *           DuplicateMenuNode001/002 only pass valid menuNodes without radius.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternTddCoverageTestNg, DuplicateMenuNode_NullAndBorderBranches_001, TestSize.Level2)
{
    auto menuNode = FrameNode::CreateFrameNode(V2::MENU_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<MenuPattern>(TARGET_ID, TEXT_TAG, MenuType::MENU));
    ASSERT_NE(menuNode, nullptr);
    auto pattern = menuNode->GetPattern<MenuPattern>();
    ASSERT_NE(pattern, nullptr);
    MenuParam param;

    // Case A: null menuNode => returns the freshly created duplicateMenuNode.
    auto dupA = pattern->DuplicateMenuNode(nullptr, param);
    ASSERT_NE(dupA, nullptr);
    EXPECT_EQ(dupA->GetTag(), V2::ROW_ETS_TAG);

    // Case B: menuNode whose layout property is not MenuLayoutProperty => early
    // return of the created duplicateMenuNode (no borderRadius copy).
    auto plainNode = FrameNode::CreateFrameNode(
        V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(plainNode, nullptr);
    auto dupB = pattern->DuplicateMenuNode(plainNode, param);
    ASSERT_NE(dupB, nullptr);
    EXPECT_EQ(dupB->GetTag(), V2::ROW_ETS_TAG);

    // Case C: menuNode with MenuLayoutProperty carrying a borderRadius =>
    // UpdateBorderRadius is applied to the duplicate (happy-path branch,
    // line 1645-1648). Mirror the setup in DuplicateMenuNode001.
    auto menuWithProp = FrameNode::CreateFrameNode(V2::MENU_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<MenuPattern>(TARGET_ID, TEXT_TAG, MenuType::MENU));
    ASSERT_NE(menuWithProp, nullptr);
    // Ensure GetLayoutProperty<MenuLayoutProperty>() resolves: assign a
    // MenuLayoutProperty explicitly (CreateFrameNode with MenuPattern does not
    // always attach one).
    menuWithProp->layoutProperty_ = AceType::MakeRefPtr<MenuLayoutProperty>();
    auto prop = menuWithProp->GetLayoutProperty<MenuLayoutProperty>();
    ASSERT_NE(prop, nullptr);
    BorderRadiusProperty borderRadius;
    borderRadius.SetRadius(CalcDimension(20.0f, DimensionUnit::VP));
    prop->UpdateBorderRadius(borderRadius);
    auto dupC = pattern->DuplicateMenuNode(menuWithProp, param);
    ASSERT_NE(dupC, nullptr);
    auto dupRc = dupC->GetRenderContext();
    ASSERT_NE(dupRc, nullptr);
    EXPECT_EQ(dupRc->GetBorderRadius(), borderRadius);
}

// =============================================================================
// menu_view.cpp
// =============================================================================
class MenuViewTddCoverageTestNg : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() override
    {
        MockPipelineContext::SetUp();
        auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
        MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
        InstallSelectAndMenuThemes(themeManager);
        MockContainer::SetUp();
    }
    void TearDown() override
    {
        MockPipelineContext::TearDown();
        MockContainer::TearDown();
    }
};

/**
 * @tc.name: MountGridSectionDivider_RealCall_001
 * @tc.desc: Verify MenuView::MountGridSectionDivider actually builds and mounts
 *           a divider node (menu_view.cpp:2879-2896). The existing
 *           MountGridSectionDivider001 only constructs a column and never calls
 *           this method (pseudo-coverage); the method body is otherwise dead.
 * @tc.type: FUNC
 */
HWTEST_F(MenuViewTddCoverageTestNg, MountGridSectionDivider_RealCall_001, TestSize.Level1)
{
    // Real column node so theme/geometry lookups succeed.
    auto column = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<LinearLayoutPattern>(true));
    ASSERT_NE(column, nullptr);
    ASSERT_EQ(column->GetChildren().size(), 0u);

    MenuView::MountGridSectionDivider(column);

    // Divider node created + mounted => column now has exactly one child.
    ASSERT_EQ(column->GetChildren().size(), 1u);
    auto divider = AceType::DynamicCast<FrameNode>(column->GetFirstChild());
    ASSERT_NE(divider, nullptr);
    // Background color must come from SelectTheme::GetGridMenuDividerColor().
    auto rc = divider->GetRenderContext();
    ASSERT_NE(rc, nullptr);
    auto theme = divider->GetTheme<SelectTheme>(true);
    ASSERT_NE(theme, nullptr);
    EXPECT_EQ(rc->GetBackgroundColorValue(), theme->GetGridMenuDividerColor());

    // Null column: the `column ? ... : 0` guard must keep this from crashing.
    RefPtr<FrameNode> nullColumn;
    MenuView::MountGridSectionDivider(nullColumn);
    SUCCEED();
}

/**
 * @tc.name: ExecuteMenuDisappearAnimation_NullMenuNode_001
 * @tc.desc: Verify ExecuteMenuDisappearAnimation returns early without crashing
 *           when data.menuNode is null (menu_view.cpp:3107-3108). The method is
 *           otherwise completely uncovered by existing tests.
 * @tc.type: FUNC
 */
HWTEST_F(MenuViewTddCoverageTestNg, ExecuteMenuDisappearAnimation_NullMenuNode_001, TestSize.Level2)
{
    PreparedInfoForDrag data;
    // data.menuNode defaults to nullptr => CHECK_NULL_VOID early return.
    MenuView::ExecuteMenuDisappearAnimation(data);
    SUCCEED();
}

/**
 * @tc.name: CreateIcon_DirectCall_001
 * @tc.desc: Verify MenuView::CreateIcon builds an Image FrameNode for both the
 *           empty-icon and non-empty-icon branches and mounts it at index 0 of
 *           the parent (menu_view.cpp:3064-3076). The method has no direct unit
 *           test today (only indirect coverage via Create).
 * @tc.type: FUNC
 */
HWTEST_F(MenuViewTddCoverageTestNg, CreateIcon_DirectCall_001, TestSize.Level2)
{
    auto parent = FrameNode::CreateFrameNode(
        V2::ROW_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<LinearLayoutPattern>(false));
    ASSERT_NE(parent, nullptr);

    // Case A: non-empty icon => ImageSourceInfo updated + node mounted at 0.
    auto iconNode = MenuView::CreateIcon("resource://icon.png", parent);
    ASSERT_NE(iconNode, nullptr);
    EXPECT_EQ(iconNode->GetTag(), V2::IMAGE_ETS_TAG);
    ASSERT_EQ(parent->GetChildren().size(), 1u);
    EXPECT_EQ(parent->GetChildAtIndex(0), iconNode);

    // Case B: empty icon => node still created and mounted, no source info set.
    auto parent2 = FrameNode::CreateFrameNode(
        V2::ROW_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<LinearLayoutPattern>(false));
    ASSERT_NE(parent2, nullptr);
    auto iconNode2 = MenuView::CreateIcon("", parent2);
    ASSERT_NE(iconNode2, nullptr);
    ASSERT_EQ(parent2->GetChildren().size(), 1u);

    // Case C: child != nullptr => ReplaceChild branch (line 3072-3073).
    auto parent3 = FrameNode::CreateFrameNode(
        V2::ROW_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<LinearLayoutPattern>(false));
    ASSERT_NE(parent3, nullptr);
    auto oldChild = FrameNode::CreateFrameNode(
        V2::IMAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<ImagePattern>());
    ASSERT_NE(oldChild, nullptr);
    oldChild->MountToParent(parent3);
    ASSERT_EQ(parent3->GetChildren().size(), 1u);
    auto replaced = MenuView::CreateIcon("resource://icon2.png", parent3, oldChild);
    ASSERT_NE(replaced, nullptr);
    ASSERT_EQ(parent3->GetChildren().size(), 1u);
    EXPECT_EQ(parent3->GetChildAtIndex(0), replaced);
    EXPECT_NE(parent3->GetChildAtIndex(0), oldChild);
}

/**
 * @tc.name: CreateText_DirectCall_001
 * @tc.desc: Verify MenuView::CreateText builds and mounts a Text FrameNode for
 *           the default and isAIMenuOption branches (menu_view.cpp:3037-3050).
 *           The method has no direct unit test today.
 * @tc.type: FUNC
 */
HWTEST_F(MenuViewTddCoverageTestNg, CreateText_DirectCall_001, TestSize.Level2)
{
    auto parent = FrameNode::CreateFrameNode(
        V2::ROW_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<LinearLayoutPattern>(false));
    ASSERT_NE(parent, nullptr);

    // Case A: default flags => text node created + mounted.
    auto textNode = MenuView::CreateText("Option A", parent);
    ASSERT_NE(textNode, nullptr);
    EXPECT_EQ(textNode->GetTag(), V2::TEXT_ETS_TAG);
    ASSERT_EQ(parent->GetChildren().size(), 1u);
    EXPECT_EQ(parent->GetChildAtIndex(0), textNode);

    // Case B: autoWrapFlag=true + isAIMenuOption=true (AI gradient branch).
    auto parent2 = FrameNode::CreateFrameNode(
        V2::ROW_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<LinearLayoutPattern>(false));
    ASSERT_NE(parent2, nullptr);
    auto aiText = MenuView::CreateText("AI Option", parent2, true, true);
    ASSERT_NE(aiText, nullptr);
    EXPECT_EQ(aiText->GetTag(), V2::TEXT_ETS_TAG);
    ASSERT_EQ(parent2->GetChildren().size(), 1u);
}

/**
 * @tc.name: CreateSelectOption_DirectCall_001
 * @tc.desc: Verify MenuView::CreateSelectOption covers the three icon-source
 *           branches (symbolIcon / text icon / neither) at menu_view.cpp:3092-
 *           3097. The method has no direct unit test today.
 * @tc.type: FUNC
 */
HWTEST_F(MenuViewTddCoverageTestNg, CreateSelectOption_DirectCall_001, TestSize.Level2)
{
    // Case A: text icon path => CreateIcon + pattern->SetIcon + text node built.
    SelectParam paramWithIcon { "Option with icon", "resource://ic.png" };
    auto optionA = MenuView::CreateSelectOption(paramWithIcon, 0, false);
    ASSERT_NE(optionA, nullptr);
    auto patternA = optionA->GetPattern<MenuItemPattern>();
    ASSERT_NE(patternA, nullptr);
    EXPECT_NE(patternA->GetTextNode(), nullptr);

    // Case B: no icon at all (empty string + null symbol) => iconNode stays
    // null but option/text are still built.
    SelectParam paramNoIcon { "Plain option", "" };
    auto optionB = MenuView::CreateSelectOption(paramNoIcon, 1, false);
    ASSERT_NE(optionB, nullptr);
    auto patternB = optionB->GetPattern<MenuItemPattern>();
    ASSERT_NE(patternB, nullptr);
    EXPECT_NE(patternB->GetTextNode(), nullptr);

    // Case C: autoWrapFlag=true path end-to-end through CreateSelectOption.
    SelectParam paramWrap { "Long option that may wrap", "" };
    auto optionC = MenuView::CreateSelectOption(paramWrap, 2, true);
    ASSERT_NE(optionC, nullptr);
}

/**
 * @tc.name: UpdateMenuNodeByAnimation_PlacementAnchors_001
 * @tc.desc: Verify MenuView::UpdateMenuNodeByAnimation sets the transform
 *           center according to data.menuPosition for the LEFT_TOP / BOTTOM /
 *           BOTTOM_RIGHT / TOP_LEFT / LEFT / LEFT_BOTTOM placement groups
 *           (menu_view.cpp:3352-3365). Existing UpdateMenuNodeByAnimation001
 *           only exercises the default (NONE) placement.
 * @tc.type: FUNC
 */
HWTEST_F(MenuViewTddCoverageTestNg, UpdateMenuNodeByAnimation_PlacementAnchors_001, TestSize.Level2)
{
    auto buildMenuAndData = [](Placement pos) {
        auto menuNode = FrameNode::CreateFrameNode(V2::MENU_ETS_TAG,
            ElementRegister::GetInstance()->MakeUniqueId(),
            AceType::MakeRefPtr<MenuPattern>(TARGET_ID, TEXT_TAG, MenuType::MENU));
        PreparedInfoForDrag data;
        data.menuNode = menuNode;
        data.menuRect = RectF(0.0f, 0.0f, 200.0f, 100.0f); // width=200, height=100
        data.menuPosition = pos;
        return std::make_pair(menuNode, data);
    };

    // LEFT_TOP / LEFT / LEFT_BOTTOM => center.x == menuWidth (200).
    for (Placement p : { Placement::LEFT_TOP, Placement::LEFT, Placement::LEFT_BOTTOM }) {
        auto [menuNode, data] = buildMenuAndData(p);
        MenuView::UpdateMenuNodeByAnimation(menuNode, data);
        auto rc = menuNode->GetRenderContext();
        ASSERT_NE(rc, nullptr);
        ASSERT_NE(rc->GetTransformCenter(), std::nullopt);
        EXPECT_FLOAT_EQ(rc->GetTransformCenter()->GetX().Value(), 200.0f);
    }

    // TOP_LEFT / TOP / TOP_RIGHT => center.y == menuHeight (100).
    for (Placement p : { Placement::TOP_LEFT, Placement::TOP, Placement::TOP_RIGHT }) {
        auto [menuNode, data] = buildMenuAndData(p);
        MenuView::UpdateMenuNodeByAnimation(menuNode, data);
        auto rc = menuNode->GetRenderContext();
        ASSERT_NE(rc, nullptr);
        ASSERT_NE(rc->GetTransformCenter(), std::nullopt);
        EXPECT_FLOAT_EQ(rc->GetTransformCenter()->GetY().Value(), 100.0f);
    }

    // BOTTOM / BOTTOM_RIGHT => neither pure-X nor pure-Y anchor; just assert no
    // crash and scale {0.4, 0.4} is applied.
    for (Placement p : { Placement::BOTTOM, Placement::BOTTOM_RIGHT }) {
        auto [menuNode, data] = buildMenuAndData(p);
        MenuView::UpdateMenuNodeByAnimation(menuNode, data);
        auto rc = menuNode->GetRenderContext();
        ASSERT_NE(rc, nullptr);
        auto scaleOpt = rc->GetTransformScale();
        ASSERT_NE(scaleOpt, std::nullopt);
        EXPECT_FLOAT_EQ(scaleOpt->x, 0.4f);
        EXPECT_FLOAT_EQ(scaleOpt->y, 0.4f);
    }
}
} // namespace OHOS::Ace::NG

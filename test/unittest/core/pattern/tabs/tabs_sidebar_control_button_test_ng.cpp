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

#include "tabs_test_ng.h"
#include "test/mock/frameworks/core/common/mock_theme_manager.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"

#include "core/components/dialog/dialog_theme.h"
#include "core/components/tab_bar/tab_theme.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/components_ng/pattern/tabs/tabs_layout_algorithm.h"
#include "core/components_ng/pattern/tabs/tabs_layout_property.h"
#include "core/components_ng/pattern/tabs/tabs_model_ng.h"
#include "core/components_ng/pattern/tabs/tabs_node.h"
#include "core/components_ng/pattern/tabs/tabs_pattern.h"
#include "core/components_ng/pattern/tabs/tabs_side_bar_pattern.h"
#include "core/components_ng/pattern/shape/polygon_paint_property.h"
#include "core/components_ng/pattern/text/text_layout_property.h"

namespace OHOS::Ace::NG {
namespace {
RefPtr<Theme> GetControlButtonTheme(ThemeType type)
{
    if (type == TabTheme::TypeId()) {
        auto themeConstants = TestNG::CreateThemeConstants(THEME_PATTERN_TAB);
        return TabTheme::Builder().Build(themeConstants);
    }
    return AceType::MakeRefPtr<DialogTheme>();
}

RefPtr<FrameNode> CreateTestNodeCB(const std::string& tag = V2::COLUMN_ETS_TAG)
{
    return FrameNode::CreateFrameNode(tag, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<Pattern>());
}

void SetupSideBarPatternCB(RefPtr<FrameNode>& sideBarNode, RefPtr<TabsSideBarPattern>& pattern)
{
    sideBarNode = FrameNode::CreateFrameNode(V2::TABS_SIDE_BAR_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TabsSideBarPattern>());
    pattern = sideBarNode ? sideBarNode->GetPattern<TabsSideBarPattern>() : nullptr;
    if (!pattern) {
        return;
    }
    auto footerContainer = CreateTestNodeCB();
    pattern->footerContainerNode_ = footerContainer;
    footerContainer->MountToParent(sideBarNode);

    auto bottomBarContainer = CreateTestNodeCB();
    pattern->bottomBarContainerNode_ = bottomBarContainer;
    bottomBarContainer->MountToParent(sideBarNode);

    auto headerContainer = CreateTestNodeCB();
    pattern->headerContainerNode_ = headerContainer;
    headerContainer->MountToParent(sideBarNode);

    auto tabList = CreateTestNodeCB(V2::TABS_SIDE_BAR_TAB_LIST_TAG);
    pattern->tabListNode_ = tabList;
    tabList->MountToParent(sideBarNode);
}
} // namespace

class TabsSidebarControlButtonTestNg : public TabsTestNg {
public:
    static void SetUpTestSuite()
    {
        TestNG::SetUpTestSuite();
        MockPipelineContext::GetCurrent()->SetUseFlushUITasks(true);
        auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
        EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly([](ThemeType type) -> RefPtr<Theme> {
            return GetControlButtonTheme(type);
        });
        EXPECT_CALL(*themeManager, GetTheme(_, _))
            .WillRepeatedly([](ThemeType type, int32_t themeScopeId) -> RefPtr<Theme> {
                return GetControlButtonTheme(type);
            });
        MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    }
    static void TearDownTestSuite()
    {
        TestNG::TearDownTestSuite();
    }
    void FlushLayoutTask(const RefPtr<FrameNode>& frameNode)
    {
        CHECK_NULL_VOID(frameNode);
        frameNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
        frameNode->SetActive();
        frameNode->isLayoutDirtyMarked_ = true;
        frameNode->CreateLayoutTask();
        auto paintProperty = frameNode->GetPaintProperty<PaintProperty>();
        if (paintProperty) {
            auto wrapper = frameNode->CreatePaintWrapper();
            if (wrapper != nullptr) {
                wrapper->FlushRender();
            }
            paintProperty->CleanDirty();
        }
        frameNode->SetActive(false);
    }

    /**
     * Helper: Create Tabs with sidebar layout style and optional control button.
     */
    void CreateTabsWithSidebar(bool showControlButton = false)
    {
        TabsModelNG model = CreateTabs();
        CreateTabContents(TABCONTENT_NUMBER);
        model.SetBarLayoutStyle(TabBarLayoutStyle::SIDEBAR);
        if (showControlButton) {
            model.SetShowSidebarControlButton(true);
        }
        CreateTabsDone(model);
        FlushLayoutTask(frameNode_);
    }

    RefPtr<TabsSideBarPattern> GetSideBarPattern()
    {
        auto sideBarNode = pattern_->GetSideBarNode();
        CHECK_NULL_RETURN(sideBarNode, nullptr);
        return sideBarNode->GetPattern<TabsSideBarPattern>();
    }
};

// =========================================================================
// Part 1: Tests for commit 10c7cba93c — showSidebarControlButton API
// =========================================================================

/**
 * @tc.name: ShowSidebarControlButtonTest001
 * @tc.desc: Test SetShowSidebarControlButton default value is false
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarControlButtonTestNg, ShowSidebarControlButtonTest001, TestSize.Level1)
{
    CreateTabsWithSidebar(false);

    /**
     * @tc.expected: Default showSidebarControlButton_ is false
     */
    EXPECT_FALSE(pattern_->GetShowSidebarControlButton());
    auto sideBarPattern = GetSideBarPattern();
    ASSERT_NE(sideBarPattern, nullptr);
    EXPECT_FALSE(sideBarPattern->showControlButton_);
}

/**
 * @tc.name: ShowSidebarControlButtonTest002
 * @tc.desc: Test SetShowSidebarControlButton(true) creates collapseButton node
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarControlButtonTestNg, ShowSidebarControlButtonTest002, TestSize.Level1)
{
    CreateTabsWithSidebar(true);

    /**
     * @tc.expected: showSidebarControlButton_ is true and collapseButtonNode_ is created
     */
    EXPECT_TRUE(pattern_->GetShowSidebarControlButton());
    auto collapseButton = pattern_->GetCollapseButtonNode();
    EXPECT_NE(collapseButton, nullptr);

    auto sideBarPattern = GetSideBarPattern();
    ASSERT_NE(sideBarPattern, nullptr);
    EXPECT_TRUE(sideBarPattern->showControlButton_);
    EXPECT_NE(sideBarPattern->GetCollapseButtonNode(), nullptr);
}

/**
 * @tc.name: ShowSidebarControlButtonTest003
 * @tc.desc: Test dynamic toggle: set true then false removes collapseButton
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarControlButtonTestNg, ShowSidebarControlButtonTest003, TestSize.Level1)
{
    CreateTabsWithSidebar(true);
    EXPECT_NE(pattern_->GetCollapseButtonNode(), nullptr);

    /**
     * @tc.steps: Toggle to false
     */
    TabsModelNG::SetShowSidebarControlButton(AceType::RawPtr(frameNode_), false);
    FlushUITasks();

    /**
     * @tc.expected: collapseButtonNode_ is null, sideBar state cleared
     */
    EXPECT_FALSE(pattern_->GetShowSidebarControlButton());
    EXPECT_EQ(pattern_->GetCollapseButtonNode(), nullptr);

    auto sideBarPattern = GetSideBarPattern();
    ASSERT_NE(sideBarPattern, nullptr);
    EXPECT_EQ(sideBarPattern->GetCollapseButtonNode(), nullptr);
}

/**
 * @tc.name: ShowSidebarControlButtonTest004
 * @tc.desc: Test dynamic toggle: set false then true creates collapseButton
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarControlButtonTestNg, ShowSidebarControlButtonTest004, TestSize.Level1)
{
    CreateTabsWithSidebar(false);
    EXPECT_EQ(pattern_->GetCollapseButtonNode(), nullptr);

    /**
     * @tc.steps: Toggle to true
     */
    TabsModelNG::SetShowSidebarControlButton(AceType::RawPtr(frameNode_), true);
    FlushUITasks();

    /**
     * @tc.expected: collapseButtonNode_ is created
     */
    EXPECT_TRUE(pattern_->GetShowSidebarControlButton());
    EXPECT_NE(pattern_->GetCollapseButtonNode(), nullptr);
}

/**
 * @tc.name: ShowSidebarControlButtonTest005
 * @tc.desc: Test SetShowSidebarControlButton with same value is a no-op
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarControlButtonTestNg, ShowSidebarControlButtonTest005, TestSize.Level1)
{
    CreateTabsWithSidebar(true);
    auto buttonBefore = pattern_->GetCollapseButtonNode();
    ASSERT_NE(buttonBefore, nullptr);

    /**
     * @tc.steps: Set same value again
     */
    pattern_->SetShowSidebarControlButton(true);

    /**
     * @tc.expected: Same button node (no recreation)
     */
    EXPECT_EQ(pattern_->GetCollapseButtonNode(), buttonBefore);
}

/**
 * @tc.name: ShowSidebarControlButtonTest006
 * @tc.desc: Test SetShowSidebarControlButton with nullptr frameNode (no crash)
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarControlButtonTestNg, ShowSidebarControlButtonTest006, TestSize.Level1)
{
    TabsModelNG::SetShowSidebarControlButton(nullptr, true);
    TabsModelNG::SetShowSidebarControlButton(nullptr, false);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: CollapseButtonNodeCreationTest001
 * @tc.desc: Test CreateCollapseButton mounts button to tabsNode and creates placeholder
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarControlButtonTestNg, CollapseButtonNodeCreationTest001, TestSize.Level1)
{
    CreateTabsWithSidebar(true);

    auto sideBarPattern = GetSideBarPattern();
    ASSERT_NE(sideBarPattern, nullptr);

    /**
     * @tc.expected: Button is mounted as child of tabsNode
     */
    auto collapseButton = sideBarPattern->GetCollapseButtonNode();
    ASSERT_NE(collapseButton, nullptr);
    auto buttonParent = collapseButton->GetParent();
    ASSERT_NE(buttonParent, nullptr);
    EXPECT_EQ(buttonParent, AceType::DynamicCast<UINode>(frameNode_));

    /**
     * @tc.expected: Icon node is created and mounted inside button
     */
    EXPECT_NE(sideBarPattern->collapseButtonIconNode_, nullptr);
    auto iconParent = sideBarPattern->collapseButtonIconNode_->GetParent();
    ASSERT_NE(iconParent, nullptr);
    EXPECT_EQ(iconParent, AceType::DynamicCast<UINode>(collapseButton));
}

/**
 * @tc.name: CollapseButtonNodeCreationTest002
 * @tc.desc: Test CreateCollapseButton with null tabsNode returns null
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarControlButtonTestNg, CollapseButtonNodeCreationTest002, TestSize.Level1)
{
    RefPtr<FrameNode> sideBarNode;
    RefPtr<TabsSideBarPattern> sideBarPattern;
    SetupSideBarPatternCB(sideBarNode, sideBarPattern);
    ASSERT_NE(sideBarPattern, nullptr);

    auto result = sideBarPattern->CreateCollapseButton(nullptr);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: ClearCollapseButtonStateTest001
 * @tc.desc: Test ClearCollapseButtonState clears nodes and removes placeholder
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarControlButtonTestNg, ClearCollapseButtonStateTest001, TestSize.Level1)
{
    CreateTabsWithSidebar(true);
    auto sideBarPattern = GetSideBarPattern();
    ASSERT_NE(sideBarPattern, nullptr);
    ASSERT_NE(sideBarPattern->GetCollapseButtonNode(), nullptr);
    ASSERT_NE(sideBarPattern->collapseButtonIconNode_, nullptr);

    /**
     * @tc.steps: Clear state
     */
    sideBarPattern->ClearCollapseButtonState();

    /**
     * @tc.expected: All button-related nodes are null
     */
    EXPECT_EQ(sideBarPattern->collapseButtonNode_, nullptr);
    EXPECT_EQ(sideBarPattern->collapseButtonIconNode_, nullptr);
}

/**
 * @tc.name: ClearCollapseButtonStateTest002
 * @tc.desc: Test ClearCollapseButtonState is safe to call when already cleared
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarControlButtonTestNg, ClearCollapseButtonStateTest002, TestSize.Level1)
{
    RefPtr<FrameNode> sideBarNode;
    RefPtr<TabsSideBarPattern> sideBarPattern;
    SetupSideBarPatternCB(sideBarNode, sideBarPattern);
    ASSERT_NE(sideBarPattern, nullptr);

    /**
     * @tc.steps: Call ClearCollapseButtonState when no button exists
     * @tc.expected: No crash
     */
    sideBarPattern->ClearCollapseButtonState();
    EXPECT_EQ(sideBarPattern->collapseButtonNode_, nullptr);
    EXPECT_EQ(sideBarPattern->collapseButtonIconNode_, nullptr);
}

/**
 * @tc.name: UpdateCollapseButtonIfNeededTest001
 * @tc.desc: Test UpdateCollapseButtonIfNeeded creates button when show=true and no button exists
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarControlButtonTestNg, UpdateCollapseButtonIfNeededTest001, TestSize.Level1)
{
    CreateTabsWithSidebar(false);
    EXPECT_EQ(pattern_->GetCollapseButtonNode(), nullptr);

    /**
     * @tc.steps: Set show=true and call UpdateCollapseButtonIfNeeded
     */
    pattern_->showSidebarControlButton_ = true;
    pattern_->UpdateCollapseButtonIfNeeded();

    /**
     * @tc.expected: Button is created
     */
    EXPECT_NE(pattern_->GetCollapseButtonNode(), nullptr);
}

/**
 * @tc.name: UpdateCollapseButtonIfNeededTest002
 * @tc.desc: Test UpdateCollapseButtonIfNeeded removes button when show=false and button exists
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarControlButtonTestNg, UpdateCollapseButtonIfNeededTest002, TestSize.Level1)
{
    CreateTabsWithSidebar(true);
    EXPECT_NE(pattern_->GetCollapseButtonNode(), nullptr);

    /**
     * @tc.steps: Set show=false and call UpdateCollapseButtonIfNeeded
     */
    pattern_->showSidebarControlButton_ = false;
    pattern_->UpdateCollapseButtonIfNeeded();

    /**
     * @tc.expected: Button is removed
     */
    EXPECT_EQ(pattern_->GetCollapseButtonNode(), nullptr);
}

/**
 * @tc.name: UpdateCollapseButtonIfNeededTest003
 * @tc.desc: Test UpdateCollapseButtonIfNeeded is no-op when show=true and button already exists
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarControlButtonTestNg, UpdateCollapseButtonIfNeededTest003, TestSize.Level1)
{
    CreateTabsWithSidebar(true);
    auto buttonBefore = pattern_->GetCollapseButtonNode();
    ASSERT_NE(buttonBefore, nullptr);

    /**
     * @tc.steps: Call again with show=true
     */
    pattern_->showSidebarControlButton_ = true;
    pattern_->UpdateCollapseButtonIfNeeded();

    /**
     * @tc.expected: Same button (no recreation)
     */
    EXPECT_EQ(pattern_->GetCollapseButtonNode(), buttonBefore);
}

/**
 * @tc.name: UpdateCollapseButtonIfNeededTest004
 * @tc.desc: Test UpdateCollapseButtonIfNeeded is no-op when show=false and no button
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarControlButtonTestNg, UpdateCollapseButtonIfNeededTest004, TestSize.Level1)
{
    CreateTabsWithSidebar(false);
    EXPECT_EQ(pattern_->GetCollapseButtonNode(), nullptr);

    /**
     * @tc.steps: Call with show=false and no button
     * @tc.expected: Still null, no crash
     */
    pattern_->showSidebarControlButton_ = false;
    pattern_->UpdateCollapseButtonIfNeeded();
    EXPECT_EQ(pattern_->GetCollapseButtonNode(), nullptr);
}

/**
 * @tc.name: UpdateCollapseButtonIconTest001
 * @tc.desc: Test UpdateCollapseButtonIcon does not crash with valid icon node.
 *   Note: In test env, ResourceManager::GetResourceAdapter returns nullptr, so
 *   UpdateCollapseButtonIcon early-returns before setting symbol properties.
 *   We verify the call is safe and iconNode remains valid.
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarControlButtonTestNg, UpdateCollapseButtonIconTest001, TestSize.Level1)
{
    CreateTabsWithSidebar(true);
    auto sideBarPattern = GetSideBarPattern();
    ASSERT_NE(sideBarPattern, nullptr);
    ASSERT_NE(sideBarPattern->collapseButtonIconNode_, nullptr);

    /**
     * @tc.steps: Call UpdateCollapseButtonIcon with different collapsed states
     * @tc.expected: No crash, iconNode still valid
     */
    sideBarPattern->isCollapsed_ = false;
    sideBarPattern->UpdateCollapseButtonIcon();
    EXPECT_NE(sideBarPattern->collapseButtonIconNode_, nullptr);

    sideBarPattern->isCollapsed_ = true;
    sideBarPattern->UpdateCollapseButtonIcon();
    EXPECT_NE(sideBarPattern->collapseButtonIconNode_, nullptr);
}

/**
 * @tc.name: UpdateCollapseButtonIconTest002
 * @tc.desc: Test UpdateCollapseButtonIcon with null iconNode (no crash)
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarControlButtonTestNg, UpdateCollapseButtonIconTest002, TestSize.Level1)
{
    RefPtr<FrameNode> sideBarNode;
    RefPtr<TabsSideBarPattern> sideBarPattern;
    SetupSideBarPatternCB(sideBarNode, sideBarPattern);
    ASSERT_NE(sideBarPattern, nullptr);

    /**
     * @tc.steps: Call UpdateCollapseButtonIcon with null iconNode
     * @tc.expected: No crash
     */
    sideBarPattern->collapseButtonIconNode_ = nullptr;
    sideBarPattern->UpdateCollapseButtonIcon();
    EXPECT_TRUE(true);
}

// =========================================================================
// Part 2: Tests for animation fix — reentrancy, state, visibility
// =========================================================================

/**
 * @tc.name: OnCollapseButtonClickReentrancyTest001
 * @tc.desc: Test OnCollapseButtonClick is blocked while the TabsPattern collapse-switch
 *   state machine is busy (COLLAPSE_ANIMATING). The shadowed isCollapseAnimating_ flag
 *   was removed; the debounce now reads the single SidebarCollapseSwitchState machine.
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarControlButtonTestNg, OnCollapseButtonClickReentrancyTest001, TestSize.Level1)
{
    CreateTabsWithSidebar(true);
    auto sideBarPattern = GetSideBarPattern();
    ASSERT_NE(sideBarPattern, nullptr);
    ASSERT_NE(pattern_, nullptr);

    /**
     * @tc.steps: Make the switch state machine busy (animating), then call OnCollapseButtonClick
     * @tc.expected: isCollapsed_ does not change (click is blocked), state stays busy
     */
    bool beforeCollapsed = sideBarPattern->IsCollapsed();
    pattern_->sidebarCollapseSwitchState_ = TabsPattern::SidebarCollapseSwitchState::COLLAPSE_ANIMATING;
    sideBarPattern->OnCollapseButtonClick();

    EXPECT_EQ(sideBarPattern->IsCollapsed(), beforeCollapsed);
    EXPECT_TRUE(pattern_->IsSidebarCollapseSwitchBusy());
    EXPECT_TRUE(pattern_->IsOnSidebarCollapseSwitchAnimation());

    /**
     * @tc.steps: Restore the state machine to idle (cleanup)
     */
    pattern_->sidebarCollapseSwitchState_ = TabsPattern::SidebarCollapseSwitchState::NONE;
}

/**
 * @tc.name: OnCollapseButtonClickReentrancyTest002
 * @tc.desc: Test OnCollapseButtonClick toggles isCollapsed_ and registers a collapse
 *   switch request on the TabsPattern state machine when it is idle.
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarControlButtonTestNg, OnCollapseButtonClickReentrancyTest002, TestSize.Level1)
{
    CreateTabsWithSidebar(true);
    auto sideBarPattern = GetSideBarPattern();
    ASSERT_NE(sideBarPattern, nullptr);
    ASSERT_NE(pattern_, nullptr);

    /**
     * @tc.steps: Not busy, call OnCollapseButtonClick
     * @tc.expected: isCollapsed_ toggles to true (collapse) and the switch request
     * is registered (COLLAPSE_REQUESTED)
     */
    EXPECT_FALSE(sideBarPattern->IsCollapsed());
    EXPECT_FALSE(pattern_->IsSidebarCollapseSwitchBusy());

    sideBarPattern->OnCollapseButtonClick();

    EXPECT_TRUE(sideBarPattern->IsCollapsed());
    EXPECT_TRUE(pattern_->IsSidebarCollapseSwitchRequested());
    EXPECT_TRUE(pattern_->IsSidebarCollapseSwitchToCollapsed());
    EXPECT_FALSE(pattern_->IsOnSidebarCollapseSwitchAnimation());

    /**
     * @tc.steps: Restore the state machine to idle (cleanup)
     */
    pattern_->sidebarCollapseSwitchState_ = TabsPattern::SidebarCollapseSwitchState::NONE;
}

/**
 * @tc.name: IsCollapsedTest001
 * @tc.desc: Test IsCollapsed initial value and after collapse/expand
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarControlButtonTestNg, IsCollapsedTest001, TestSize.Level1)
{
    RefPtr<FrameNode> sideBarNode;
    RefPtr<TabsSideBarPattern> sideBarPattern;
    SetupSideBarPatternCB(sideBarNode, sideBarPattern);
    ASSERT_NE(sideBarPattern, nullptr);

    /**
     * @tc.expected: Initial isCollapsed_ is false
     */
    EXPECT_FALSE(sideBarPattern->IsCollapsed());

    /**
     * @tc.steps: Set isCollapsed_ = true
     */
    sideBarPattern->isCollapsed_ = true;
    EXPECT_TRUE(sideBarPattern->IsCollapsed());

    /**
     * @tc.steps: Set isCollapsed_ = false
     */
    sideBarPattern->isCollapsed_ = false;
    EXPECT_FALSE(sideBarPattern->IsCollapsed());
}

/**
 * @tc.name: RealSideBarWidthPxTest001
 * @tc.desc: Test SetRealSideBarWidthPx / ClearRealSideBarWidthPx
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarControlButtonTestNg, RealSideBarWidthPxTest001, TestSize.Level1)
{
    CreateTabsWithSidebar(false);

    /**
     * @tc.steps: Set a value
     */
    pattern_->SetRealSideBarWidthPx(250.0f);
    auto val = pattern_->GetRealSideBarWidthPx();
    ASSERT_TRUE(val.has_value());
    EXPECT_FLOAT_EQ(val.value(), 250.0f);

    /**
     * @tc.steps: Clear
     */
    pattern_->ClearRealSideBarWidthPx();
    EXPECT_FALSE(pattern_->GetRealSideBarWidthPx().has_value());
}

/**
 * @tc.name: PreSideBarWidthPxTest001
 * @tc.desc: Test SetPreSideBarWidthPx / GetPreSideBarWidthPx
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarControlButtonTestNg, PreSideBarWidthPxTest001, TestSize.Level1)
{
    CreateTabsWithSidebar(false);

    /**
     * @tc.steps: Set and get
     */
    pattern_->SetPreSideBarWidthPx(400.0f);
    EXPECT_FLOAT_EQ(pattern_->GetPreSideBarWidthPx(), 400.0f);

    pattern_->SetPreSideBarWidthPx(0.0f);
    EXPECT_FLOAT_EQ(pattern_->GetPreSideBarWidthPx(), 0.0f);
}

/**
 * @tc.name: SideBarVisibilityDuringAnimationTest001
 * @tc.desc: Test UpdateSideBarAndSideBarDividerVisibility keeps VISIBLE when isCollapseAnimating=true
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarControlButtonTestNg, SideBarVisibilityDuringAnimationTest001, TestSize.Level1)
{
    CreateTabsWithSidebar(true);
    auto sideBarNode = pattern_->GetSideBarNode();
    ASSERT_NE(sideBarNode, nullptr);
    auto sideBarPattern = GetSideBarPattern();
    ASSERT_NE(sideBarPattern, nullptr);

    /**
     * @tc.steps: Simulate animation state: isCollapsed=true but the switch state machine
     * is busy (COLLAPSE_ANIMATING). Call layout visibility update with isVisible=true
     */
    sideBarPattern->isCollapsed_ = true;
    ASSERT_NE(pattern_, nullptr);
    pattern_->sidebarCollapseSwitchState_ = TabsPattern::SidebarCollapseSwitchState::COLLAPSE_ANIMATING;

    auto layoutWrapper = frameNode_->CreateLayoutWrapper();
    ASSERT_NE(layoutWrapper, nullptr);
    auto layoutAlgorithm = AceType::MakeRefPtr<TabsLayoutAlgorithm>();
    layoutAlgorithm->UpdateSideBarAndSideBarDividerVisibility(AceType::RawPtr(layoutWrapper), true);

    /**
     * @tc.expected: Sidebar is VISIBLE (busy switch overrides isCollapsed)
     */
    auto sideBarProp = sideBarNode->GetLayoutProperty();
    ASSERT_NE(sideBarProp, nullptr);
    EXPECT_EQ(sideBarProp->GetVisibilityValue(VisibleType::GONE), VisibleType::VISIBLE);

    /**
     * @tc.steps: Restore the state machine to idle (cleanup)
     */
    pattern_->sidebarCollapseSwitchState_ = TabsPattern::SidebarCollapseSwitchState::NONE;
}

/**
 * @tc.name: SideBarVisibilityDuringAnimationTest002
 * @tc.desc: Test visibility is GONE when isCollapsed=true and not animating
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarControlButtonTestNg, SideBarVisibilityDuringAnimationTest002, TestSize.Level1)
{
    CreateTabsWithSidebar(true);
    auto sideBarNode = pattern_->GetSideBarNode();
    ASSERT_NE(sideBarNode, nullptr);
    auto sideBarPattern = GetSideBarPattern();
    ASSERT_NE(sideBarPattern, nullptr);

    /**
     * @tc.steps: isCollapsed=true, switch state machine idle, isVisible=true
     */
    sideBarPattern->isCollapsed_ = true;
    ASSERT_NE(pattern_, nullptr);
    pattern_->sidebarCollapseSwitchState_ = TabsPattern::SidebarCollapseSwitchState::NONE;

    auto layoutWrapper = frameNode_->CreateLayoutWrapper();
    ASSERT_NE(layoutWrapper, nullptr);
    auto layoutAlgorithm = AceType::MakeRefPtr<TabsLayoutAlgorithm>();
    layoutAlgorithm->UpdateSideBarAndSideBarDividerVisibility(AceType::RawPtr(layoutWrapper), true);

    /**
     * @tc.expected: Sidebar is GONE (collapsed state, not animating)
     */
    auto sideBarProp = sideBarNode->GetLayoutProperty();
    ASSERT_NE(sideBarProp, nullptr);
    EXPECT_EQ(sideBarProp->GetVisibilityValue(VisibleType::VISIBLE), VisibleType::GONE);
}

/**
 * @tc.name: SideBarVisibilityDuringAnimationTest003
 * @tc.desc: Test visibility is VISIBLE when not collapsed and not animating
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarControlButtonTestNg, SideBarVisibilityDuringAnimationTest003, TestSize.Level1)
{
    CreateTabsWithSidebar(true);
    auto sideBarNode = pattern_->GetSideBarNode();
    ASSERT_NE(sideBarNode, nullptr);
    auto sideBarPattern = GetSideBarPattern();
    ASSERT_NE(sideBarPattern, nullptr);

    /**
     * @tc.steps: isCollapsed=false, switch state machine idle, isVisible=true
     */
    sideBarPattern->isCollapsed_ = false;
    ASSERT_NE(pattern_, nullptr);
    pattern_->sidebarCollapseSwitchState_ = TabsPattern::SidebarCollapseSwitchState::NONE;

    auto layoutWrapper = frameNode_->CreateLayoutWrapper();
    ASSERT_NE(layoutWrapper, nullptr);
    auto layoutAlgorithm = AceType::MakeRefPtr<TabsLayoutAlgorithm>();
    layoutAlgorithm->UpdateSideBarAndSideBarDividerVisibility(AceType::RawPtr(layoutWrapper), true);

    /**
     * @tc.expected: Sidebar is VISIBLE
     */
    auto sideBarProp = sideBarNode->GetLayoutProperty();
    ASSERT_NE(sideBarProp, nullptr);
    EXPECT_EQ(sideBarProp->GetVisibilityValue(VisibleType::GONE), VisibleType::VISIBLE);
}

/**
 * @tc.name: SideBarVisibilityDuringAnimationTest004
 * @tc.desc: Test visibility is GONE when isVisible=false (regardless of collapse state)
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarControlButtonTestNg, SideBarVisibilityDuringAnimationTest004, TestSize.Level1)
{
    CreateTabsWithSidebar(true);
    auto sideBarNode = pattern_->GetSideBarNode();
    ASSERT_NE(sideBarNode, nullptr);
    auto sideBarPattern = GetSideBarPattern();
    ASSERT_NE(sideBarPattern, nullptr);

    /**
     * @tc.steps: isCollapsed=false, switch state machine idle, but isVisible=false
     * (e.g. bottom tab bar mode)
     */
    sideBarPattern->isCollapsed_ = false;
    ASSERT_NE(pattern_, nullptr);
    pattern_->sidebarCollapseSwitchState_ = TabsPattern::SidebarCollapseSwitchState::NONE;

    auto layoutWrapper = frameNode_->CreateLayoutWrapper();
    ASSERT_NE(layoutWrapper, nullptr);
    auto layoutAlgorithm = AceType::MakeRefPtr<TabsLayoutAlgorithm>();
    layoutAlgorithm->UpdateSideBarAndSideBarDividerVisibility(AceType::RawPtr(layoutWrapper), false);

    /**
     * @tc.expected: Sidebar is GONE (not visible)
     */
    auto sideBarProp = sideBarNode->GetLayoutProperty();
    ASSERT_NE(sideBarProp, nullptr);
    EXPECT_EQ(sideBarProp->GetVisibilityValue(VisibleType::VISIBLE), VisibleType::GONE);
}

/**
 * @tc.name: SideBarCollapsedStatePersistTest001
 * @tc.desc: Test collapsed state persists when isVisible=true and not animating
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarControlButtonTestNg, SideBarCollapsedStatePersistTest001, TestSize.Level1)
{
    CreateTabsWithSidebar(true);
    auto sideBarNode = pattern_->GetSideBarNode();
    ASSERT_NE(sideBarNode, nullptr);
    auto sideBarPattern = GetSideBarPattern();
    ASSERT_NE(sideBarPattern, nullptr);

    /**
     * @tc.steps: Set isCollapsed=true, not animating. This simulates switching back
     * from bottom tab bar mode to sidebar mode when sidebar was previously collapsed.
     */
    sideBarPattern->isCollapsed_ = true;
    ASSERT_NE(pattern_, nullptr);
    pattern_->sidebarCollapseSwitchState_ = TabsPattern::SidebarCollapseSwitchState::NONE;

    auto layoutWrapper = frameNode_->CreateLayoutWrapper();
    ASSERT_NE(layoutWrapper, nullptr);
    auto layoutAlgorithm = AceType::MakeRefPtr<TabsLayoutAlgorithm>();
    layoutAlgorithm->UpdateSideBarAndSideBarDividerVisibility(AceType::RawPtr(layoutWrapper), true);

    /**
     * @tc.expected: Sidebar stays GONE (collapsed state preserved)
     */
    auto sideBarProp = sideBarNode->GetLayoutProperty();
    ASSERT_NE(sideBarProp, nullptr);
    EXPECT_EQ(sideBarProp->GetVisibilityValue(VisibleType::VISIBLE), VisibleType::GONE);
}

/**
 * @tc.name: DividerVisibilityDuringAnimationTest001
 * @tc.desc: Test divider visibility follows effectiveVisible during animation
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarControlButtonTestNg, DividerVisibilityDuringAnimationTest001, TestSize.Level1)
{
    CreateTabsWithSidebar(true);
    auto sideBarPattern = GetSideBarPattern();
    ASSERT_NE(sideBarPattern, nullptr);
    auto dividerNode = pattern_->GetSideBarDividerNode();
    ASSERT_NE(dividerNode, nullptr);

    /**
     * @tc.steps: isCollapsed=true, switch state machine busy (animating) → divider should be VISIBLE
     */
    sideBarPattern->isCollapsed_ = true;
    ASSERT_NE(pattern_, nullptr);
    pattern_->sidebarCollapseSwitchState_ = TabsPattern::SidebarCollapseSwitchState::COLLAPSE_ANIMATING;

    auto layoutWrapper = frameNode_->CreateLayoutWrapper();
    ASSERT_NE(layoutWrapper, nullptr);
    auto layoutAlgorithm = AceType::MakeRefPtr<TabsLayoutAlgorithm>();
    layoutAlgorithm->UpdateSideBarAndSideBarDividerVisibility(AceType::RawPtr(layoutWrapper), true);

    auto dividerProp = dividerNode->GetLayoutProperty();
    ASSERT_NE(dividerProp, nullptr);
    EXPECT_EQ(dividerProp->GetVisibilityValue(VisibleType::GONE), VisibleType::VISIBLE);

    /**
     * @tc.steps: isCollapsed=true, switch state machine idle → divider should be GONE
     */
    pattern_->sidebarCollapseSwitchState_ = TabsPattern::SidebarCollapseSwitchState::NONE;
    layoutAlgorithm->UpdateSideBarAndSideBarDividerVisibility(AceType::RawPtr(layoutWrapper), true);

    EXPECT_EQ(dividerProp->GetVisibilityValue(VisibleType::VISIBLE), VisibleType::GONE);
}

/**
 * @tc.name: CollapseButtonBackShadowTest001
 * @tc.desc: Test collapse button has default shadow applied
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarControlButtonTestNg, CollapseButtonBackShadowTest001, TestSize.Level1)
{
    CreateTabsWithSidebar(true);
    auto sideBarPattern = GetSideBarPattern();
    ASSERT_NE(sideBarPattern, nullptr);
    auto collapseButton = sideBarPattern->GetCollapseButtonNode();
    ASSERT_NE(collapseButton, nullptr);

    /**
     * @tc.expected: Button render context has back shadow
     */
    auto renderContext = collapseButton->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    EXPECT_TRUE(renderContext->HasBackShadow());
}

/**
 * @tc.name: CollapseButtonIconInitTest001
 * @tc.desc: Test collapse button icon node is created after CreateCollapseButton.
 *   The icon is a STACK container (CreateCollapseButtonIcon in
 *   tabs_side_bar_pattern.cpp): three static hamburger bar FrameNodes plus one
 *   morphable arrow triangle Polygon node, so the icon node carries a stack
 *   layout property instead of the legacy text/symbol one.
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarControlButtonTestNg, CollapseButtonIconInitTest001, TestSize.Level1)
{
    CreateTabsWithSidebar(true);
    auto sideBarPattern = GetSideBarPattern();
    ASSERT_NE(sideBarPattern, nullptr);

    /**
     * @tc.expected: Icon node exists and is a STACK with 3 bars + 1 triangle
     */
    auto iconNode = sideBarPattern->collapseButtonIconNode_;
    ASSERT_NE(iconNode, nullptr);
    EXPECT_EQ(iconNode->GetTag(), V2::STACK_ETS_TAG);
    auto iconProp = iconNode->GetLayoutProperty();
    ASSERT_NE(iconProp, nullptr);
    EXPECT_EQ(iconNode->GetChildren().size(), 4u);
}

/**
 * @tc.name: CollapseButtonIconInitTest002
 * @tc.desc: Test UpdateCollapseButtonIcon is safe to call with different isCollapsed_ states.
 *   Note: In test env, ResourceAdapter is null so symbol properties are not set;
 *   we verify no crash and iconNode persists across state changes.
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarControlButtonTestNg, CollapseButtonIconInitTest002, TestSize.Level1)
{
    CreateTabsWithSidebar(true);
    auto sideBarPattern = GetSideBarPattern();
    ASSERT_NE(sideBarPattern, nullptr);

    /**
     * @tc.steps: Toggle isCollapsed_ and call UpdateCollapseButtonIcon
     * @tc.expected: No crash, iconNode remains valid
     */
    sideBarPattern->isCollapsed_ = true;
    sideBarPattern->UpdateCollapseButtonIcon();
    EXPECT_NE(sideBarPattern->collapseButtonIconNode_, nullptr);

    sideBarPattern->isCollapsed_ = false;
    sideBarPattern->UpdateCollapseButtonIcon();
    EXPECT_NE(sideBarPattern->collapseButtonIconNode_, nullptr);
}

/**
 * @tc.name: ToggleCollapseExpandCycleTest001
 * @tc.desc: Test full collapse→expand cycle toggles isCollapsed_ correctly.
 *   Note: In test env no layout pass runs between clicks, so the switch state
 *   machine stays in the REQUESTED phase; we simulate the animation finishing
 *   (state back to NONE) between clicks, matching the on-device lifecycle.
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarControlButtonTestNg, ToggleCollapseExpandCycleTest001, TestSize.Level1)
{
    CreateTabsWithSidebar(true);
    auto sideBarPattern = GetSideBarPattern();
    ASSERT_NE(sideBarPattern, nullptr);
    ASSERT_NE(pattern_, nullptr);

    /**
     * @tc.steps: step1. Collapse — isCollapsed_ should become true
     */
    EXPECT_FALSE(sideBarPattern->IsCollapsed());
    sideBarPattern->OnCollapseButtonClick();
    EXPECT_TRUE(sideBarPattern->IsCollapsed());
    EXPECT_TRUE(pattern_->IsSidebarCollapseSwitchRequested());

    /**
     * @tc.steps: step2. Simulate switch finished, then expand — isCollapsed_
     * should become false
     */
    pattern_->sidebarCollapseSwitchState_ = TabsPattern::SidebarCollapseSwitchState::NONE;
    sideBarPattern->OnCollapseButtonClick();
    EXPECT_FALSE(sideBarPattern->IsCollapsed());

    /**
     * @tc.steps: Restore the state machine to idle (cleanup)
     */
    pattern_->sidebarCollapseSwitchState_ = TabsPattern::SidebarCollapseSwitchState::NONE;
}

/**
 * @tc.name: ToggleCollapseExpandCycleTest002
 * @tc.desc: Test a click landing while the previous switch request is still pending
 *   is debounced by the state machine: isCollapsed_ keeps the first click's value.
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarControlButtonTestNg, ToggleCollapseExpandCycleTest002, TestSize.Level1)
{
    CreateTabsWithSidebar(true);
    auto sideBarPattern = GetSideBarPattern();
    ASSERT_NE(sideBarPattern, nullptr);
    ASSERT_NE(pattern_, nullptr);

    /**
     * @tc.steps: Two rapid clicks without a layout pass in between
     * @tc.expected: The second click is rejected by the busy state machine;
     * isCollapsed_ stays true (collapsed by the first click)
     */
    sideBarPattern->OnCollapseButtonClick();
    EXPECT_TRUE(sideBarPattern->IsCollapsed());
    EXPECT_EQ(pattern_->sidebarCollapseSwitchState_, TabsPattern::SidebarCollapseSwitchState::COLLAPSE_REQUESTED);

    sideBarPattern->OnCollapseButtonClick();
    EXPECT_TRUE(sideBarPattern->IsCollapsed());
    EXPECT_EQ(pattern_->sidebarCollapseSwitchState_, TabsPattern::SidebarCollapseSwitchState::COLLAPSE_REQUESTED);

    /**
     * @tc.steps: Restore the state machine to idle (cleanup)
     */
    pattern_->sidebarCollapseSwitchState_ = TabsPattern::SidebarCollapseSwitchState::NONE;
}

/**
 * @tc.name: ShowSidebarControlButtonDynamicToggleTest001
 * @tc.desc: Test toggling control button off clears sidebar collapsed state
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarControlButtonTestNg, ShowSidebarControlButtonDynamicToggleTest001, TestSize.Level1)
{
    CreateTabsWithSidebar(true);
    auto sideBarPattern = GetSideBarPattern();
    ASSERT_NE(sideBarPattern, nullptr);

    /**
     * @tc.steps: Collapse sidebar, then hide control button
     */
    sideBarPattern->isCollapsed_ = true;
    EXPECT_TRUE(sideBarPattern->IsCollapsed());

    TabsModelNG::SetShowSidebarControlButton(AceType::RawPtr(frameNode_), false);
    FlushUITasks();

    /**
     * @tc.expected: Button is removed, collapseButtonNode_ and icon cleared
     */
    EXPECT_EQ(pattern_->GetCollapseButtonNode(), nullptr);
    auto sbPattern = GetSideBarPattern();
    ASSERT_NE(sbPattern, nullptr);
    EXPECT_EQ(sbPattern->GetCollapseButtonNode(), nullptr);
    EXPECT_EQ(sbPattern->collapseButtonIconNode_, nullptr);
}

/**
 * @tc.name: ShowSidebarControlButtonDynamicToggleTest002
 * @tc.desc: Test re-enabling control button after disabling creates fresh button
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarControlButtonTestNg, ShowSidebarControlButtonDynamicToggleTest002, TestSize.Level1)
{
    CreateTabsWithSidebar(true);
    EXPECT_NE(pattern_->GetCollapseButtonNode(), nullptr);

    /**
     * @tc.steps: Disable, then re-enable
     */
    TabsModelNG::SetShowSidebarControlButton(AceType::RawPtr(frameNode_), false);
    FlushUITasks();
    EXPECT_EQ(pattern_->GetCollapseButtonNode(), nullptr);

    TabsModelNG::SetShowSidebarControlButton(AceType::RawPtr(frameNode_), true);
    FlushUITasks();

    /**
     * @tc.expected: New button is created
     */
    EXPECT_NE(pattern_->GetCollapseButtonNode(), nullptr);
    auto sideBarPattern = GetSideBarPattern();
    ASSERT_NE(sideBarPattern, nullptr);
    EXPECT_NE(sideBarPattern->GetCollapseButtonNode(), nullptr);
    EXPECT_NE(sideBarPattern->collapseButtonIconNode_, nullptr);
}

/**
 * @tc.name: SidebarCollapseSwitchSideBarOnLeft001
 * @tc.desc: Unified two-pass collapse switch with sideBarOnLeft (START + LTR) → outsideX
 *   negative. OnCollapseButtonClick registers the request, StartSidebarCollapseSwitchAnimation
 *   drives the slide-out, and the finish bookkeeping sets the leaving sidebar GONE.
 *   (Replace of the removed CollapseSideBarContent path.)
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarControlButtonTestNg, SidebarCollapseSwitchSideBarOnLeft001, TestSize.Level1)
{
    CreateTabsWithSidebar(true);
    auto sideBarPattern = GetSideBarPattern();
    ASSERT_NE(sideBarPattern, nullptr);
    auto sideBarNode = pattern_->GetSideBarNode();
    ASSERT_NE(sideBarNode, nullptr);
    auto dividerNode = pattern_->GetSideBarDividerNode();
    ASSERT_NE(dividerNode, nullptr);

    /**
     * @tc.steps: Default BarPosition::START + LTR → sideBarOnLeft=true → outsideX < 0
     */
    ASSERT_NE(layoutProperty_, nullptr);
    layoutProperty_->UpdateSidebarPosition(BarPosition::START);
    layoutProperty_->UpdateLayoutDirection(TextDirection::LTR);
    FlushLayoutTask(frameNode_);

    /**
     * @tc.steps: Click (request) then run the two-pass switch animation
     */
    EXPECT_FALSE(sideBarPattern->IsCollapsed());
    sideBarPattern->OnCollapseButtonClick();
    EXPECT_TRUE(pattern_->IsSidebarCollapseSwitchRequested());
    pattern_->StartSidebarCollapseSwitchAnimation();
    // In test env the animation closure/finish may not run synchronously;
    // drive the finish bookkeeping deterministically when still busy.
    if (pattern_->IsSidebarCollapseSwitchBusy()) {
        pattern_->OnSidebarCollapseSwitchAnimationFinished(true, false);
    }

    /**
     * @tc.expected: isCollapsed_=true; switch finished (state back to NONE);
     * sidebar and divider set GONE by the finish bookkeeping; the translate
     * render property was written (exit-start reset + finish reset).
     */
    EXPECT_TRUE(sideBarPattern->IsCollapsed());
    EXPECT_FALSE(pattern_->IsSidebarCollapseSwitchBusy());
    auto sideBarRender = sideBarNode->GetRenderContext();
    ASSERT_NE(sideBarRender, nullptr);
    EXPECT_TRUE(sideBarRender->HasTransformTranslate());
    auto dividerRender = dividerNode->GetRenderContext();
    ASSERT_NE(dividerRender, nullptr);
    EXPECT_TRUE(dividerRender->HasTransformTranslate());
    EXPECT_EQ(sideBarNode->GetLayoutProperty()->GetVisibilityValue(VisibleType::VISIBLE), VisibleType::GONE);
    EXPECT_EQ(dividerNode->GetLayoutProperty()->GetVisibilityValue(VisibleType::VISIBLE), VisibleType::GONE);
}

/**
 * @tc.name: SidebarCollapseSwitchSideBarOnRight001
 * @tc.desc: Unified two-pass collapse switch with sideBarOnRight (END + LTR) → outsideX
 *   positive. Same switch pipeline as the left case; verifies the positive-X slide-out
 *   branch lands on the same steady state.
 *   (Replace of the removed CollapseSideBarContent path.)
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarControlButtonTestNg, SidebarCollapseSwitchSideBarOnRight001, TestSize.Level1)
{
    CreateTabsWithSidebar(true);
    auto sideBarPattern = GetSideBarPattern();
    ASSERT_NE(sideBarPattern, nullptr);
    auto sideBarNode = pattern_->GetSideBarNode();
    ASSERT_NE(sideBarNode, nullptr);
    auto dividerNode = pattern_->GetSideBarDividerNode();
    ASSERT_NE(dividerNode, nullptr);

    /**
     * @tc.steps: BarPosition::END + LTR → sideBarOnLeft=false → outsideX > 0
     */
    ASSERT_NE(layoutProperty_, nullptr);
    layoutProperty_->UpdateSidebarPosition(BarPosition::END);
    layoutProperty_->UpdateLayoutDirection(TextDirection::LTR);
    FlushLayoutTask(frameNode_);

    /**
     * @tc.steps: Click (request) then run the two-pass switch animation
     */
    EXPECT_FALSE(sideBarPattern->IsCollapsed());
    sideBarPattern->OnCollapseButtonClick();
    EXPECT_TRUE(pattern_->IsSidebarCollapseSwitchRequested());
    pattern_->StartSidebarCollapseSwitchAnimation();
    // In test env the animation closure/finish may not run synchronously;
    // drive the finish bookkeeping deterministically when still busy.
    if (pattern_->IsSidebarCollapseSwitchBusy()) {
        pattern_->OnSidebarCollapseSwitchAnimationFinished(true, false);
    }

    /**
     * @tc.expected: isCollapsed_=true; switch finished (state back to NONE);
     * translate pipeline ran on both nodes; visibility GONE after finish.
     */
    EXPECT_TRUE(sideBarPattern->IsCollapsed());
    EXPECT_FALSE(pattern_->IsSidebarCollapseSwitchBusy());
    auto sideBarRender = sideBarNode->GetRenderContext();
    ASSERT_NE(sideBarRender, nullptr);
    EXPECT_TRUE(sideBarRender->HasTransformTranslate());
    auto dividerRender = dividerNode->GetRenderContext();
    ASSERT_NE(dividerRender, nullptr);
    EXPECT_TRUE(dividerRender->HasTransformTranslate());
    EXPECT_EQ(sideBarNode->GetLayoutProperty()->GetVisibilityValue(VisibleType::VISIBLE), VisibleType::GONE);
    EXPECT_EQ(dividerNode->GetLayoutProperty()->GetVisibilityValue(VisibleType::VISIBLE), VisibleType::GONE);
}

// =========================================================================
// Part 5: Tests for sidebarTitleBar API (title bar row + icon/text nodes)
// =========================================================================

namespace {
constexpr float CONTAINER_MODAL_TITLE_HEIGHT_PX = 100.0f;
} // namespace

/**
 * @tc.name: SidebarTitleBarTest001
 * @tc.desc: Default state — no titleBar row when sidebarTitleBar never set
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarControlButtonTestNg, SidebarTitleBarTest001, TestSize.Level1)
{
    CreateTabsWithSidebar(false);
    auto sideBarPattern = GetSideBarPattern();
    ASSERT_NE(sideBarPattern, nullptr);

    EXPECT_FALSE(pattern_->HasSidebarTitleBar());
    // titleBarRowNode_ is always created in CreateTitleBarRowIfNeeded (visibility GONE
    // when no titleBar options and no control button), so it should exist but be GONE.
    auto rowNode = sideBarPattern->GetTitleBarRowNode();
    ASSERT_NE(rowNode, nullptr);
    EXPECT_EQ(rowNode->GetLayoutProperty()->GetVisibilityValue(VisibleType::VISIBLE), VisibleType::GONE);
}

/**
 * @tc.name: SidebarTitleBarTest002
 * @tc.desc: Reset sidebarTitleBar with isNull=true → removes titleBar content
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarControlButtonTestNg, SidebarTitleBarTest002, TestSize.Level1)
{
    CreateTabsWithSidebar(false);
    auto sideBarPattern = GetSideBarPattern();
    ASSERT_NE(sideBarPattern, nullptr);

    TabsSidebarTitleBarOptions options;
    options.text = "Title";
    options.isNull = false;
    pattern_->SetSidebarTitleBar(options);
    FlushUITasks();
    EXPECT_TRUE(pattern_->HasSidebarTitleBar());

    TabsSidebarTitleBarOptions resetOptions;
    resetOptions.isNull = true;
    pattern_->SetSidebarTitleBar(resetOptions);
    FlushUITasks();

    EXPECT_FALSE(pattern_->HasSidebarTitleBar());
}

/**
 * @tc.name: SidebarTitleBarTest003
 * @tc.desc: ComputeTitleBarRowHeight default — controlButtonVisible=false → default height
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarControlButtonTestNg, SidebarTitleBarTest003, TestSize.Level1)
{
    CreateTabsWithSidebar(false);
    auto sideBarPattern = GetSideBarPattern();
    ASSERT_NE(sideBarPattern, nullptr);

    MockPipelineContext::GetCurrent()->SetContainerControlButtonVisible(false);
    MockPipelineContext::GetCurrent()->SetContainerCustomTitleVisible(false);
    MockPipelineContext::GetCurrent()->SetContainerModalTitleHeight(0);

    Dimension height = sideBarPattern->ComputeTitleBarRowHeight();
    // controlButtonVisible=false → returns default TITLE_BAR_ROW_HEIGHT
    // TITLE_BAR_ROW_HEIGHT = COLLAPSE_BUTTON_SIZE + 2*COLLAPSE_BUTTON_HORIZONTAL_PADDING = 40 + 16 = 56vp
    EXPECT_EQ(height.Value(), 56.0f);
}

/**
 * @tc.name: SidebarTitleBarTest004
 * @tc.desc: ComputeTitleBarRowHeight with customTitleVisible=true → default height
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarControlButtonTestNg, SidebarTitleBarTest004, TestSize.Level1)
{
    CreateTabsWithSidebar(false);
    auto sideBarPattern = GetSideBarPattern();
    ASSERT_NE(sideBarPattern, nullptr);

    MockPipelineContext::GetCurrent()->SetContainerCustomTitleVisible(true);
    MockPipelineContext::GetCurrent()->SetContainerControlButtonVisible(true);
    MockPipelineContext::GetCurrent()->SetContainerModalTitleHeight(static_cast<int32_t>(CONTAINER_MODAL_TITLE_HEIGHT_PX));

    Dimension height = sideBarPattern->ComputeTitleBarRowHeight();
    // customTitleVisible=true → returns default TITLE_BAR_ROW_HEIGHT
    EXPECT_EQ(height.Value(), 56.0f);
}

/**
 * @tc.name: SidebarTitleBarTest005
 * @tc.desc: ComputeTitleBarRowHeight with three-button present → returns containerModalTitleHeight
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarControlButtonTestNg, SidebarTitleBarTest005, TestSize.Level1)
{
    CreateTabsWithSidebar(false);
    auto sideBarPattern = GetSideBarPattern();
    ASSERT_NE(sideBarPattern, nullptr);

    MockPipelineContext::GetCurrent()->SetContainerCustomTitleVisible(false);
    MockPipelineContext::GetCurrent()->SetContainerControlButtonVisible(true);
    MockPipelineContext::GetCurrent()->SetContainerModalTitleHeight(static_cast<int32_t>(CONTAINER_MODAL_TITLE_HEIGHT_PX));

    Dimension height = sideBarPattern->ComputeTitleBarRowHeight();
    // !customTitleVisible && controlButtonVisible → returns titleHeight in PX
    EXPECT_FLOAT_EQ(height.Value(), CONTAINER_MODAL_TITLE_HEIGHT_PX);
    EXPECT_EQ(height.Unit(), DimensionUnit::PX);
}

/**
 * @tc.name: SidebarTitleBarTest006
 * @tc.desc: ComputeTitleBarRowHeight with titleHeight<=0 → default height
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarControlButtonTestNg, SidebarTitleBarTest006, TestSize.Level1)
{
    CreateTabsWithSidebar(false);
    auto sideBarPattern = GetSideBarPattern();
    ASSERT_NE(sideBarPattern, nullptr);

    MockPipelineContext::GetCurrent()->SetContainerCustomTitleVisible(false);
    MockPipelineContext::GetCurrent()->SetContainerControlButtonVisible(true);
    MockPipelineContext::GetCurrent()->SetContainerModalTitleHeight(0);

    Dimension height = sideBarPattern->ComputeTitleBarRowHeight();
    // titleHeight<=0 → returns default TITLE_BAR_ROW_HEIGHT
    EXPECT_EQ(height.Value(), 56.0f);
}

/**
 * @tc.name: SidebarTitleBarTest07
 * @tc.desc: TitleBar row padding uses start/end fields for RTL mirroring
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarControlButtonTestNg, SidebarTitleBarTest07, TestSize.Level1)
{
    CreateTabsWithSidebar(false);
    auto sideBarPattern = GetSideBarPattern();
    ASSERT_NE(sideBarPattern, nullptr);

    TabsSidebarTitleBarOptions options;
    options.text = "Title";
    options.isNull = false;
    pattern_->SetSidebarTitleBar(options);
    FlushUITasks();

    auto rowNode = sideBarPattern->GetTitleBarRowNode();
    ASSERT_NE(rowNode, nullptr);
    auto rowProp = rowNode->GetLayoutProperty<LinearLayoutProperty>();
    ASSERT_NE(rowProp, nullptr);
    const auto& padding = rowProp->GetPaddingProperty();
    ASSERT_NE(padding, nullptr);
    // start/end fields used (not left/right) for RTL-aware layout
    EXPECT_TRUE(padding->start.has_value());
    EXPECT_TRUE(padding->end.has_value());
}

/**
 * @tc.name: SidebarTitleBarTest08
 * @tc.desc: Button Y vertically centered with three-button control when title bar hidden
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarControlButtonTestNg, SidebarTitleBarTest08, TestSize.Level1)
{
    CreateTabsWithSidebar(true);
    auto sideBarPattern = GetSideBarPattern();
    ASSERT_NE(sideBarPattern, nullptr);
    auto sideBarNode = pattern_->GetSideBarNode();
    ASSERT_NE(sideBarNode, nullptr);

    MockPipelineContext::GetCurrent()->SetContainerCustomTitleVisible(false);
    MockPipelineContext::GetCurrent()->SetContainerControlButtonVisible(true);
    MockPipelineContext::GetCurrent()->SetContainerModalTitleHeight(static_cast<int32_t>(CONTAINER_MODAL_TITLE_HEIGHT_PX));
    FlushLayoutTask(frameNode_);

    auto collapseButton = pattern_->GetCollapseButtonNode();
    ASSERT_NE(collapseButton, nullptr);
    auto buttonGeo = collapseButton->GetGeometryNode();
    ASSERT_NE(buttonGeo, nullptr);
    float buttonY = buttonGeo->GetMarginFrameOffset().GetY();
    float buttonSize = 40.0f; // COLLAPSE_BUTTON_SIZE = 40vp
    // buttonY should be (titleHeight - buttonSize) / 2 + paddingOffset.Y
    // With dpi ≈ 1.0 in test, titleHeight=100px, buttonSize=40px
    // buttonY ≈ (100 - 40) / 2 = 30 (plus paddingOffset.Y which is 0 in test)
    EXPECT_GT(buttonY, 0.0f);
    EXPECT_FLOAT_EQ(buttonY, (CONTAINER_MODAL_TITLE_HEIGHT_PX - buttonSize) / 2.0f);
}

// =========================================================================
// Part 6: Tests for the two-pass sidebar collapse/expand switch state machine
// =========================================================================

/**
 * @tc.name: IsTabBarBottomStyleTest001
 * @tc.desc: Test TabsPattern::IsTabBarBottomStyle — a floating bar always counts as
 *   bottom-style; a normal bar only when horizontal + barPosition END + non-TOP style.
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarControlButtonTestNg, IsTabBarBottomStyleTest001, TestSize.Level1)
{
    CreateTabsWithSidebar(false);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    /**
     * @tc.steps: step1. Default sidebar tabs: barPosition START → not bottom style.
     */
    EXPECT_FALSE(pattern_->IsTabBarBottomStyle());

    /**
     * @tc.steps: step2. Normal bar placed at the bottom: horizontal + END → bottom style.
     */
    layoutProperty_->UpdateTabBarPosition(BarPosition::END);
    EXPECT_TRUE(pattern_->IsTabBarBottomStyle());

    /**
     * @tc.steps: step3. Vertical tabs never join the switch even with END position.
     */
    layoutProperty_->UpdateAxis(Axis::VERTICAL);
    EXPECT_FALSE(pattern_->IsTabBarBottomStyle());

    /**
     * @tc.steps: step4. Floating bar overrides placement and always qualifies.
     */
    layoutProperty_->UpdateAxis(Axis::HORIZONTAL);
    pattern_->isFloatingBar_ = true;
    EXPECT_TRUE(pattern_->IsTabBarBottomStyle());
    pattern_->isFloatingBar_ = false;
}

/**
 * @tc.name: WillBottomBarJoinCollapseSwitchTest001
 * @tc.desc: Test TabsPattern::WillBottomBarJoinCollapseSwitch — collapse direction reads
 *   showBottomBarOnSidebarCollapse at use time; expand direction is gated by
 *   keepTabBarInSideBar (never joins) and then by the tabBar node visibility
 *   (a visible bar joins so its exit is animated; a GONE bar does not).
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarControlButtonTestNg, WillBottomBarJoinCollapseSwitchTest001, TestSize.Level1)
{
    CreateTabsWithSidebar(false);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);
    auto tabBar = AceType::DynamicCast<FrameNode>(frameNode_->GetTabBar());
    ASSERT_NE(tabBar, nullptr);

    /**
     * @tc.steps: step1. Not a bottom-style bar (START) → never joins either direction.
     */
    EXPECT_FALSE(pattern_->WillBottomBarJoinCollapseSwitch(true));
    EXPECT_FALSE(pattern_->WillBottomBarJoinCollapseSwitch(false));

    /**
     * @tc.steps: step2. Bottom-style bar + property unset → collapse does not join.
     */
    layoutProperty_->UpdateTabBarPosition(BarPosition::END);
    EXPECT_FALSE(pattern_->WillBottomBarJoinCollapseSwitch(true));

    /**
     * @tc.steps: step3. Property enabled → collapse direction joins; expand direction
     * joins too when the tabBar node is visible. The sidebar-arrangement fixture
     * leaves the tabBar GONE by default (SetElementVisibilityForMode hides it in
     * the sidebar mode), so make it visible first: keepMode is off and the
     * visible bar joins the expand switch.
     */
    layoutProperty_->UpdateShowBottomBarOnSidebarCollapse(true);
    EXPECT_TRUE(pattern_->WillBottomBarJoinCollapseSwitch(true));
    tabBar->GetLayoutProperty()->UpdateVisibility(VisibleType::VISIBLE);
    EXPECT_TRUE(pattern_->WillBottomBarJoinCollapseSwitch(false));

    /**
     * @tc.steps: step4. keepTabBarInSideBar keeps the bar in the sidebar → expand
     * never joins, regardless of bar visibility.
     */
    layoutProperty_->UpdateKeepTabBarInSideBar(true);
    EXPECT_FALSE(pattern_->WillBottomBarJoinCollapseSwitch(false));
    layoutProperty_->UpdateKeepTabBarInSideBar(false);

    /**
     * @tc.steps: step5. keepMode off + tabBar GONE → expand direction does not join.
     */
    tabBar->GetLayoutProperty()->UpdateVisibility(VisibleType::GONE);
    EXPECT_FALSE(pattern_->WillBottomBarJoinCollapseSwitch(false));
    tabBar->GetLayoutProperty()->UpdateVisibility(VisibleType::VISIBLE);
}

/**
 * @tc.name: IsTabBarNodeVisibleTest001
 * @tc.desc: Test TabsPattern::IsTabBarNodeVisible — false in the sidebar
 *   arrangement by default (SetElementVisibilityForMode hides the tabBar),
 *   true when the tabBar layout property visibility is explicitly not GONE,
 *   false when GONE, false without a host node.
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarControlButtonTestNg, IsTabBarNodeVisibleTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Pattern without host → false (no crash).
     */
    auto orphanPattern = AceType::MakeRefPtr<TabsPattern>();
    EXPECT_FALSE(orphanPattern->IsTabBarNodeVisible());

    /**
     * @tc.steps: step2. Sidebar-arrangement fixture default: the tabBar was set
     * GONE by SetElementVisibilityForMode (sidebar mode hides the bottom tab
     * bar) → false.
     */
    CreateTabsWithSidebar(false);
    ASSERT_NE(pattern_, nullptr);
    auto tabBar = AceType::DynamicCast<FrameNode>(frameNode_->GetTabBar());
    ASSERT_NE(tabBar, nullptr);
    EXPECT_FALSE(pattern_->IsTabBarNodeVisible());

    /**
     * @tc.steps: step3. tabBar explicitly VISIBLE → true; GONE again → false.
     */
    tabBar->GetLayoutProperty()->UpdateVisibility(VisibleType::VISIBLE);
    EXPECT_TRUE(pattern_->IsTabBarNodeVisible());
    tabBar->GetLayoutProperty()->UpdateVisibility(VisibleType::GONE);
    EXPECT_FALSE(pattern_->IsTabBarNodeVisible());
}

/**
 * @tc.name: IsTabBarDividerNullTest001
 * @tc.desc: Test TabsPattern::IsTabBarDividerNull — false for an unset divider
 *   (default TabsItemDivider), false for a real divider, true only for a divider
 *   explicitly set to null; false without a host node.
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarControlButtonTestNg, IsTabBarDividerNullTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Pattern without host → false (no crash).
     */
    auto orphanPattern = AceType::MakeRefPtr<TabsPattern>();
    EXPECT_FALSE(orphanPattern->IsTabBarDividerNull());

    /**
     * @tc.steps: step2. Divider never set → default TabsItemDivider, isNull == false.
     */
    CreateTabsWithSidebar(false);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);
    EXPECT_FALSE(pattern_->IsTabBarDividerNull());

    /**
     * @tc.steps: step3. Real divider set → not null.
     */
    TabsItemDivider realDivider;
    realDivider.isNull = false;
    realDivider.strokeWidth = 1.0_vp;
    layoutProperty_->UpdateDivider(realDivider);
    EXPECT_FALSE(pattern_->IsTabBarDividerNull());

    /**
     * @tc.steps: step4. Divider explicitly set to null → true.
     */
    TabsItemDivider nullDivider;
    nullDivider.isNull = true;
    layoutProperty_->UpdateDivider(nullDivider);
    EXPECT_TRUE(pattern_->IsTabBarDividerNull());

    layoutProperty_->ResetDivider();
}

/**
 * @tc.name: HideTabBarForSidebarRestoreTest001
 * @tc.desc: Test TabsPattern::HideTabBarForSidebarRestore — keepMode and an
 *   already-GONE tabBar return early without touching render state; otherwise
 *   the tabBar/divider/background-mask are instantly hidden (visibility GONE)
 *   with translate/opacity reset.
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarControlButtonTestNg, HideTabBarForSidebarRestoreTest001, TestSize.Level1)
{
    CreateTabsWithSidebar(false);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);
    auto tabBar = AceType::DynamicCast<FrameNode>(frameNode_->GetTabBar());
    ASSERT_NE(tabBar, nullptr);
    auto tabBarLayoutProperty = tabBar->GetLayoutProperty();
    ASSERT_NE(tabBarLayoutProperty, nullptr);
    auto divider = AceType::DynamicCast<FrameNode>(frameNode_->GetDivider());
    ASSERT_NE(divider, nullptr);

    /**
     * @tc.steps: step1. keepMode on → early return: tabBar stays VISIBLE and its
     * render state is untouched. The sidebar-arrangement fixture leaves the
     * tabBar GONE by default (SetElementVisibilityForMode hides it in the
     * sidebar mode); make it explicitly VISIBLE so the early return below is
     * really triggered by keepMode, not by the already-GONE check.
     */
    auto tabBarRC = tabBar->GetRenderContext();
    ASSERT_NE(tabBarRC, nullptr);
    tabBarLayoutProperty->UpdateVisibility(VisibleType::VISIBLE);
    tabBarRC->UpdateOpacity(0.3f);
    layoutProperty_->UpdateKeepTabBarInSideBar(true);
    pattern_->HideTabBarForSidebarRestore();
    EXPECT_EQ(tabBarLayoutProperty->GetVisibilityValue(VisibleType::VISIBLE), VisibleType::VISIBLE);
    ASSERT_TRUE(tabBarRC->GetOpacity().has_value());
    EXPECT_FLOAT_EQ(tabBarRC->GetOpacity().value(), 0.3f);
    layoutProperty_->UpdateKeepTabBarInSideBar(false);

    /**
     * @tc.steps: step2. keepMode off + tabBar already GONE → early return, nothing
     * to hide.
     */
    tabBarLayoutProperty->UpdateVisibility(VisibleType::GONE);
    pattern_->HideTabBarForSidebarRestore();
    EXPECT_EQ(tabBarLayoutProperty->GetVisibilityValue(VisibleType::VISIBLE), VisibleType::GONE);

    /**
     * @tc.steps: step3. Visible bar → instant hide: tabBar GONE, translate reset,
     * opacity reset to 1.0; divider GONE; background mask GONE when present.
     */
    tabBarLayoutProperty->UpdateVisibility(VisibleType::VISIBLE);
    tabBarRC->UpdateTransformTranslate({ 10.0f, 20.0f, 0.0f });
    pattern_->HideTabBarForSidebarRestore();
    EXPECT_EQ(tabBarLayoutProperty->GetVisibilityValue(VisibleType::VISIBLE), VisibleType::GONE);
    ASSERT_TRUE(tabBarRC->GetTransformTranslate().has_value());
    EXPECT_FLOAT_EQ(tabBarRC->GetTransformTranslate()->x.ConvertToPx(), 0.0f);
    EXPECT_FLOAT_EQ(tabBarRC->GetTransformTranslate()->y.ConvertToPx(), 0.0f);
    ASSERT_TRUE(tabBarRC->GetOpacity().has_value());
    EXPECT_FLOAT_EQ(tabBarRC->GetOpacity().value(), 1.0f);
    EXPECT_EQ(divider->GetLayoutProperty()->GetVisibilityValue(VisibleType::VISIBLE), VisibleType::GONE);
    auto dividerRC = divider->GetRenderContext();
    ASSERT_NE(dividerRC, nullptr);
    ASSERT_TRUE(dividerRC->GetOpacity().has_value());
    EXPECT_FLOAT_EQ(dividerRC->GetOpacity().value(), 1.0f);
    if (frameNode_->HasBackgroundMaskNode()) {
        auto bgMaskNode = AceType::DynamicCast<FrameNode>(frameNode_->GetBackgroundMask());
        ASSERT_NE(bgMaskNode, nullptr);
        EXPECT_EQ(bgMaskNode->GetLayoutProperty()->GetVisibilityValue(VisibleType::VISIBLE), VisibleType::GONE);
    }

    /**
     * @tc.steps: cleanup: restore visibility for subsequent cases on the same node.
     */
    tabBarLayoutProperty->UpdateVisibility(VisibleType::VISIBLE);
    divider->GetLayoutProperty()->UpdateVisibility(VisibleType::VISIBLE);
}

/**
 * @tc.name: ApplyFloatingTabBarShownTargetsDividerGuardTest001
 * @tc.desc: Test the !IsTabBarDividerNull() guard in
 *   TabsPattern::ApplyFloatingTabBarShownTargets — with a default (unset) divider
 *   the divider opacity is restored to 1.0; with a divider explicitly set to null
 *   the opacity restore is skipped, while the tabBar itself is still restored.
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarControlButtonTestNg, ApplyFloatingTabBarShownTargetsDividerGuardTest001, TestSize.Level1)
{
    CreateTabsWithSidebar(false);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);
    auto tabBar = AceType::DynamicCast<FrameNode>(frameNode_->GetTabBar());
    ASSERT_NE(tabBar, nullptr);
    auto tabBarRC = tabBar->GetRenderContext();
    ASSERT_NE(tabBarRC, nullptr);
    auto divider = AceType::DynamicCast<FrameNode>(frameNode_->GetDivider());
    ASSERT_NE(divider, nullptr);
    auto dividerRC = divider->GetRenderContext();
    ASSERT_NE(dividerRC, nullptr);

    /**
     * @tc.steps: step1. Divider never set (default TabsItemDivider, isNull == false):
     * shown targets restore both tabBar and divider opacity.
     */
    tabBarRC->UpdateOpacity(0.2f);
    dividerRC->UpdateOpacity(0.3f);
    pattern_->ApplyFloatingTabBarShownTargets();
    ASSERT_TRUE(tabBarRC->GetOpacity().has_value());
    EXPECT_FLOAT_EQ(tabBarRC->GetOpacity().value(), 1.0f);
    ASSERT_TRUE(dividerRC->GetOpacity().has_value());
    EXPECT_FLOAT_EQ(dividerRC->GetOpacity().value(), 1.0f);

    /**
     * @tc.steps: step2. Divider explicitly set to null: the tabBar opacity is still
     * restored, but the divider opacity restore is skipped (stays as set).
     */
    TabsItemDivider nullDivider;
    nullDivider.isNull = true;
    layoutProperty_->UpdateDivider(nullDivider);
    tabBarRC->UpdateOpacity(0.2f);
    dividerRC->UpdateOpacity(0.3f);
    pattern_->ApplyFloatingTabBarShownTargets();
    ASSERT_TRUE(tabBarRC->GetOpacity().has_value());
    EXPECT_FLOAT_EQ(tabBarRC->GetOpacity().value(), 1.0f);
    ASSERT_TRUE(dividerRC->GetOpacity().has_value());
    EXPECT_FLOAT_EQ(dividerRC->GetOpacity().value(), 0.3f);

    layoutProperty_->ResetDivider();
}

/**
 * @tc.name: RequestSidebarCollapseSwitchAnimationTest001
 * @tc.desc: Test TabsPattern::RequestSidebarCollapseSwitchAnimation — registers the
 *   requested phase of the state machine and rejects requests while busy.
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarControlButtonTestNg, RequestSidebarCollapseSwitchAnimationTest001, TestSize.Level1)
{
    CreateTabsWithSidebar(false);
    ASSERT_NE(pattern_, nullptr);

    /**
     * @tc.steps: step1. Idle machine: request a collapse switch.
     * @tc.expected: COLLAPSE_REQUESTED phase, busy/requested/toCollapsed all correct.
     */
    EXPECT_FALSE(pattern_->IsSidebarCollapseSwitchBusy());
    pattern_->RequestSidebarCollapseSwitchAnimation(true);
    EXPECT_EQ(pattern_->sidebarCollapseSwitchState_, TabsPattern::SidebarCollapseSwitchState::COLLAPSE_REQUESTED);
    EXPECT_TRUE(pattern_->IsSidebarCollapseSwitchBusy());
    EXPECT_TRUE(pattern_->IsSidebarCollapseSwitchRequested());
    EXPECT_TRUE(pattern_->IsSidebarCollapseSwitchToCollapsed());
    EXPECT_FALSE(pattern_->IsOnSidebarCollapseSwitchAnimation());

    /**
     * @tc.steps: step2. Request again while busy → rejected, state unchanged.
     */
    pattern_->RequestSidebarCollapseSwitchAnimation(false);
    EXPECT_EQ(pattern_->sidebarCollapseSwitchState_, TabsPattern::SidebarCollapseSwitchState::COLLAPSE_REQUESTED);

    /**
     * @tc.steps: step3. Back to idle: request an expand switch.
     * @tc.expected: EXPAND_REQUESTED phase, toCollapsed=false.
     */
    pattern_->sidebarCollapseSwitchState_ = TabsPattern::SidebarCollapseSwitchState::NONE;
    pattern_->RequestSidebarCollapseSwitchAnimation(false);
    EXPECT_EQ(pattern_->sidebarCollapseSwitchState_, TabsPattern::SidebarCollapseSwitchState::EXPAND_REQUESTED);
    EXPECT_TRUE(pattern_->IsSidebarCollapseSwitchRequested());
    EXPECT_FALSE(pattern_->IsSidebarCollapseSwitchToCollapsed());

    /**
     * @tc.steps: Restore the state machine to idle (cleanup)
     */
    pattern_->sidebarCollapseSwitchState_ = TabsPattern::SidebarCollapseSwitchState::NONE;
}

/**
 * @tc.name: SidebarCollapseSwitchStateHelpersTest001
 * @tc.desc: Test the SidebarCollapseSwitchState helper predicates across the animating
 *   phases: COLLAPSE_ANIMATING / EXPAND_ANIMATING are "on animation" but not
 *   "requested"; every non-NONE state is busy.
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarControlButtonTestNg, SidebarCollapseSwitchStateHelpersTest001, TestSize.Level1)
{
    CreateTabsWithSidebar(false);
    ASSERT_NE(pattern_, nullptr);

    /**
     * @tc.steps: step1. COLLAPSE_ANIMATING phase.
     */
    pattern_->sidebarCollapseSwitchState_ = TabsPattern::SidebarCollapseSwitchState::COLLAPSE_ANIMATING;
    EXPECT_TRUE(pattern_->IsSidebarCollapseSwitchBusy());
    EXPECT_FALSE(pattern_->IsSidebarCollapseSwitchRequested());
    EXPECT_TRUE(pattern_->IsSidebarCollapseSwitchToCollapsed());
    EXPECT_TRUE(pattern_->IsOnSidebarCollapseSwitchAnimation());

    /**
     * @tc.steps: step2. EXPAND_ANIMATING phase.
     */
    pattern_->sidebarCollapseSwitchState_ = TabsPattern::SidebarCollapseSwitchState::EXPAND_ANIMATING;
    EXPECT_TRUE(pattern_->IsSidebarCollapseSwitchBusy());
    EXPECT_FALSE(pattern_->IsSidebarCollapseSwitchRequested());
    EXPECT_FALSE(pattern_->IsSidebarCollapseSwitchToCollapsed());
    EXPECT_TRUE(pattern_->IsOnSidebarCollapseSwitchAnimation());

    /**
     * @tc.steps: step3. Back to NONE: idle again.
     */
    pattern_->sidebarCollapseSwitchState_ = TabsPattern::SidebarCollapseSwitchState::NONE;
    EXPECT_FALSE(pattern_->IsSidebarCollapseSwitchBusy());
    EXPECT_FALSE(pattern_->IsOnSidebarCollapseSwitchAnimation());
}

/**
 * @tc.name: UpdateCollapseButtonIconColorTest001
 * @tc.desc: Test TabsSideBarPattern::UpdateCollapseButtonIconColor re-applies the theme
 *   icon color to all hamburger bars and the morphable triangle (used on dark/light
 *   mode switches where the Shape fill colors are baked at creation time).
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarControlButtonTestNg, UpdateCollapseButtonIconColorTest001, TestSize.Level1)
{
    CreateTabsWithSidebar(true);
    auto sideBarPattern = GetSideBarPattern();
    ASSERT_NE(sideBarPattern, nullptr);
    auto sideBarNode = pattern_->GetSideBarNode();
    ASSERT_NE(sideBarNode, nullptr);
    auto iconNode = sideBarPattern->collapseButtonIconNode_;
    ASSERT_NE(iconNode, nullptr);
    auto triangleNode = sideBarPattern->collapseButtonTriangleNode_;
    ASSERT_NE(triangleNode, nullptr);

    // Expected color: the theme icon color the function must re-apply.
    auto tabTheme = sideBarNode->GetTheme<TabTheme>();
    ASSERT_NE(tabTheme, nullptr);
    Color expectedColor = tabTheme->GetIconPrimary();

    /**
     * @tc.steps: step1. Simulate a stale color configuration: overwrite the
     * background color of every bar and the fill of the triangle with a wrong color.
     */
    const auto& iconChildren = iconNode->GetChildren();
    ASSERT_EQ(iconChildren.size(), 4u);
    for (const auto& child : iconChildren) {
        if (child == triangleNode) {
            continue;
        }
        auto barNode = AceType::DynamicCast<FrameNode>(child);
        ASSERT_NE(barNode, nullptr);
        auto barRenderContext = barNode->GetRenderContext();
        ASSERT_NE(barRenderContext, nullptr);
        barRenderContext->UpdateBackgroundColor(Color::RED);
        EXPECT_EQ(barRenderContext->GetBackgroundColor().value_or(Color::TRANSPARENT), Color::RED);
    }
    auto trianglePaintProperty = triangleNode->GetPaintProperty<PolygonPaintProperty>();
    ASSERT_NE(trianglePaintProperty, nullptr);
    trianglePaintProperty->UpdateFill(Color::RED);

    /**
     * @tc.steps: step2. Re-apply the theme color.
     * @tc.expected: Every bar background and the triangle fill carry the theme icon color again.
     */
    sideBarPattern->UpdateCollapseButtonIconColor();
    for (const auto& child : iconChildren) {
        if (child == triangleNode) {
            continue;
        }
        auto barNode = AceType::DynamicCast<FrameNode>(child);
        ASSERT_NE(barNode, nullptr);
        auto barRenderContext = barNode->GetRenderContext();
        ASSERT_NE(barRenderContext, nullptr);
        EXPECT_EQ(barRenderContext->GetBackgroundColor().value_or(Color::TRANSPARENT), expectedColor);
    }
    EXPECT_EQ(trianglePaintProperty->GetFillValue(Color::TRANSPARENT), expectedColor);
}
} // namespace OHOS::Ace::NG

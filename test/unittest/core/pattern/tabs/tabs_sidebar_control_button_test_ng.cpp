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
#include "core/components_ng/pattern/tabs/tabs_layout_algorithm.h"
#include "core/components_ng/pattern/tabs/tabs_layout_property.h"
#include "core/components_ng/pattern/tabs/tabs_model_ng.h"
#include "core/components_ng/pattern/tabs/tabs_node.h"
#include "core/components_ng/pattern/tabs/tabs_pattern.h"
#include "core/components_ng/pattern/tabs/tabs_side_bar_pattern.h"
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
     * @tc.expected: Placeholder is created in headerContainer
     */
    EXPECT_NE(sideBarPattern->buttonPlaceholderNode_, nullptr);
    auto placeholderParent = sideBarPattern->buttonPlaceholderNode_->GetParent();
    ASSERT_NE(placeholderParent, nullptr);
    EXPECT_EQ(placeholderParent, AceType::DynamicCast<UINode>(sideBarPattern->headerContainerNode_));

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
    ASSERT_NE(sideBarPattern->buttonPlaceholderNode_, nullptr);

    /**
     * @tc.steps: Clear state
     */
    sideBarPattern->ClearCollapseButtonState();

    /**
     * @tc.expected: All button-related nodes are null
     */
    EXPECT_EQ(sideBarPattern->collapseButtonNode_, nullptr);
    EXPECT_EQ(sideBarPattern->collapseButtonIconNode_, nullptr);
    EXPECT_EQ(sideBarPattern->buttonPlaceholderNode_, nullptr);
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
    EXPECT_EQ(sideBarPattern->buttonPlaceholderNode_, nullptr);
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
 * @tc.desc: Test OnCollapseButtonClick is blocked during animation (isCollapseAnimating_=true)
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarControlButtonTestNg, OnCollapseButtonClickReentrancyTest001, TestSize.Level1)
{
    CreateTabsWithSidebar(true);
    auto sideBarPattern = GetSideBarPattern();
    ASSERT_NE(sideBarPattern, nullptr);

    /**
     * @tc.steps: Set isCollapseAnimating_ = true, then call OnCollapseButtonClick
     * @tc.expected: isCollapsed_ does not change (click is blocked)
     */
    bool beforeCollapsed = sideBarPattern->IsCollapsed();
    sideBarPattern->SetCollapseAnimating(true);
    sideBarPattern->OnCollapseButtonClick();

    EXPECT_EQ(sideBarPattern->IsCollapsed(), beforeCollapsed);
    EXPECT_TRUE(sideBarPattern->IsCollapseAnimating());
}

/**
 * @tc.name: OnCollapseButtonClickReentrancyTest002
 * @tc.desc: Test OnCollapseButtonClick toggles isCollapsed_ when not animating.
 *   Note: In test env, AnimationUtils::StartAnimation executes the finish callback
 *   synchronously, so isCollapseAnimating_ is reset to false immediately.
 *   We only verify isCollapsed_ toggles correctly.
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarControlButtonTestNg, OnCollapseButtonClickReentrancyTest002, TestSize.Level1)
{
    CreateTabsWithSidebar(true);
    auto sideBarPattern = GetSideBarPattern();
    ASSERT_NE(sideBarPattern, nullptr);

    /**
     * @tc.steps: Not animating, call OnCollapseButtonClick
     * @tc.expected: isCollapsed_ toggles to true (collapse)
     */
    EXPECT_FALSE(sideBarPattern->IsCollapsed());

    sideBarPattern->OnCollapseButtonClick();

    EXPECT_TRUE(sideBarPattern->IsCollapsed());
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
 * @tc.name: IsCollapseAnimatingTest001
 * @tc.desc: Test IsCollapseAnimating and SetCollapseAnimating
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarControlButtonTestNg, IsCollapseAnimatingTest001, TestSize.Level1)
{
    RefPtr<FrameNode> sideBarNode;
    RefPtr<TabsSideBarPattern> sideBarPattern;
    SetupSideBarPatternCB(sideBarNode, sideBarPattern);
    ASSERT_NE(sideBarPattern, nullptr);

    /**
     * @tc.expected: Initial isCollapseAnimating_ is false
     */
    EXPECT_FALSE(sideBarPattern->IsCollapseAnimating());

    /**
     * @tc.steps: Set to true
     */
    sideBarPattern->SetCollapseAnimating(true);
    EXPECT_TRUE(sideBarPattern->IsCollapseAnimating());

    /**
     * @tc.steps: Set to false
     */
    sideBarPattern->SetCollapseAnimating(false);
    EXPECT_FALSE(sideBarPattern->IsCollapseAnimating());
}

/**
 * @tc.name: CollapseSideBarContentTest001
 * @tc.desc: Test CollapseSideBarContent sets isCollapsed_ and isCollapseAnimating_
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarControlButtonTestNg, CollapseSideBarContentTest001, TestSize.Level1)
{
    CreateTabsWithSidebar(true);
    auto sideBarPattern = GetSideBarPattern();
    ASSERT_NE(sideBarPattern, nullptr);

    /**
     * @tc.steps: Call CollapseSideBarContent
     */
    EXPECT_FALSE(sideBarPattern->IsCollapsed());
    sideBarPattern->CollapseSideBarContent();

    /**
     * @tc.expected: isCollapsed_ = true.
     * Note: isCollapseAnimating_ is reset synchronously by the finish callback
     * in test env, so we don't assert it here.
     */
    EXPECT_TRUE(sideBarPattern->IsCollapsed());
}

/**
 * @tc.name: CollapseSideBarContentTest002
 * @tc.desc: Test CollapseSideBarContent saves preSideBarWidthPx_ synchronously
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarControlButtonTestNg, CollapseSideBarContentTest002, TestSize.Level1)
{
    CreateTabsWithSidebar(true);
    auto sideBarPattern = GetSideBarPattern();
    ASSERT_NE(sideBarPattern, nullptr);

    /**
     * @tc.steps: Call CollapseSideBarContent
     */
    sideBarPattern->CollapseSideBarContent();

    /**
     * @tc.expected: preSideBarWidthPx_ is saved synchronously (non-negative).
     * realSideBarWidthPx_ is set inside the animation closure and cleared by
     * the finish callback; in test env both run synchronously so the value
     * is already cleared — we only check preSideBarWidthPx_.
     */
    float savedWidth = pattern_->GetPreSideBarWidthPx();
    EXPECT_GE(savedWidth, 0.0f);
}

/**
 * @tc.name: ExpandSideBarContentTest001
 * @tc.desc: Test ExpandSideBarContent clears isCollapsed_ and sets isCollapseAnimating_
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarControlButtonTestNg, ExpandSideBarContentTest001, TestSize.Level1)
{
    CreateTabsWithSidebar(true);
    auto sideBarPattern = GetSideBarPattern();
    ASSERT_NE(sideBarPattern, nullptr);

    /**
     * @tc.steps: First collapse, then expand
     */
    sideBarPattern->isCollapsed_ = true;
    sideBarPattern->ExpandSideBarContent();

    /**
     * @tc.expected: isCollapsed_ = false.
     * Note: isCollapseAnimating_ is reset synchronously by the finish callback
     * in test env, so we don't assert it here.
     */
    EXPECT_FALSE(sideBarPattern->IsCollapsed());
}

/**
 * @tc.name: ExpandSideBarContentTest002
 * @tc.desc: Test ExpandSideBarContent sets realSideBarWidthPx_ to 0 synchronously before animation
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarControlButtonTestNg, ExpandSideBarContentTest002, TestSize.Level1)
{
    CreateTabsWithSidebar(true);
    auto sideBarPattern = GetSideBarPattern();
    ASSERT_NE(sideBarPattern, nullptr);

    /**
     * @tc.steps: Set a known preSideBarWidthPx_, then expand
     */
    constexpr float TEST_WIDTH = 300.0f;
    pattern_->SetPreSideBarWidthPx(TEST_WIDTH);
    sideBarPattern->isCollapsed_ = true;
    sideBarPattern->ExpandSideBarContent();

    /**
     * @tc.expected: preSideBarWidthPx_ is still saved. realSideBarWidthPx_ is set
     * to 0 synchronously, then restored inside the animation closure, then cleared
     * by the finish callback. In test env all run synchronously, so it's already
     * cleared — we only verify preSideBarWidthPx_ survives.
     */
    EXPECT_EQ(pattern_->GetPreSideBarWidthPx(), TEST_WIDTH);
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
     * @tc.steps: Simulate animation state: isCollapsed=true but isCollapseAnimating=true
     * Call layout visibility update with isVisible=true
     */
    sideBarPattern->isCollapsed_ = true;
    sideBarPattern->SetCollapseAnimating(true);

    auto layoutWrapper = frameNode_->CreateLayoutWrapper();
    ASSERT_NE(layoutWrapper, nullptr);
    auto layoutAlgorithm = AceType::MakeRefPtr<TabsLayoutAlgorithm>();
    layoutAlgorithm->UpdateSideBarAndSideBarDividerVisibility(AceType::RawPtr(layoutWrapper), true);

    /**
     * @tc.expected: Sidebar is VISIBLE (because isCollapseAnimating overrides isCollapsed)
     */
    auto sideBarProp = sideBarNode->GetLayoutProperty();
    ASSERT_NE(sideBarProp, nullptr);
    EXPECT_EQ(sideBarProp->GetVisibilityValue(VisibleType::GONE), VisibleType::VISIBLE);
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
     * @tc.steps: isCollapsed=true, not animating, isVisible=true
     */
    sideBarPattern->isCollapsed_ = true;
    sideBarPattern->SetCollapseAnimating(false);

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
     * @tc.steps: isCollapsed=false, not animating, isVisible=true
     */
    sideBarPattern->isCollapsed_ = false;
    sideBarPattern->SetCollapseAnimating(false);

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
     * @tc.steps: isCollapsed=false, not animating, but isVisible=false (e.g. bottom tab bar mode)
     */
    sideBarPattern->isCollapsed_ = false;
    sideBarPattern->SetCollapseAnimating(false);

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
    sideBarPattern->SetCollapseAnimating(false);

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
     * @tc.steps: isCollapsed=true, isCollapseAnimating=true → divider should be VISIBLE
     */
    sideBarPattern->isCollapsed_ = true;
    sideBarPattern->SetCollapseAnimating(true);

    auto layoutWrapper = frameNode_->CreateLayoutWrapper();
    ASSERT_NE(layoutWrapper, nullptr);
    auto layoutAlgorithm = AceType::MakeRefPtr<TabsLayoutAlgorithm>();
    layoutAlgorithm->UpdateSideBarAndSideBarDividerVisibility(AceType::RawPtr(layoutWrapper), true);

    auto dividerProp = dividerNode->GetLayoutProperty();
    ASSERT_NE(dividerProp, nullptr);
    EXPECT_EQ(dividerProp->GetVisibilityValue(VisibleType::GONE), VisibleType::VISIBLE);

    /**
     * @tc.steps: isCollapsed=true, isCollapseAnimating=false → divider should be GONE
     */
    sideBarPattern->SetCollapseAnimating(false);
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
 *   Note: Symbol properties are not set in test env due to null ResourceAdapter,
 *   so we only verify the icon node exists.
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarControlButtonTestNg, CollapseButtonIconInitTest001, TestSize.Level1)
{
    CreateTabsWithSidebar(true);
    auto sideBarPattern = GetSideBarPattern();
    ASSERT_NE(sideBarPattern, nullptr);

    /**
     * @tc.expected: Icon node exists
     */
    auto iconNode = sideBarPattern->collapseButtonIconNode_;
    ASSERT_NE(iconNode, nullptr);
    auto iconProp = iconNode->GetLayoutProperty<TextLayoutProperty>();
    ASSERT_NE(iconProp, nullptr);
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
 *   Note: In test env, animation finish callback runs synchronously inside
 *   AnimationUtils::StartAnimation, so isCollapseAnimating_ is already false
 *   after each call. We verify isCollapsed_ toggles correctly across the cycle.
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarControlButtonTestNg, ToggleCollapseExpandCycleTest001, TestSize.Level1)
{
    CreateTabsWithSidebar(true);
    auto sideBarPattern = GetSideBarPattern();
    ASSERT_NE(sideBarPattern, nullptr);

    /**
     * @tc.steps: step1. Collapse — isCollapsed_ should become true
     */
    EXPECT_FALSE(sideBarPattern->IsCollapsed());
    sideBarPattern->OnCollapseButtonClick();
    EXPECT_TRUE(sideBarPattern->IsCollapsed());

    /**
     * @tc.steps: step2. Expand — isCollapsed_ should become false
     */
    sideBarPattern->OnCollapseButtonClick();
    EXPECT_FALSE(sideBarPattern->IsCollapsed());
}

/**
 * @tc.name: ToggleCollapseExpandCycleTest002
 * @tc.desc: Test rapid double-click does not crash and ends in a consistent state.
 *   Note: In test env, the animation finish callback runs synchronously, so
 *   isCollapseAnimating_ is false by the time the second click arrives — the
 *   reentrancy guard cannot be tested here. We verify no crash and consistent
 *   final state.
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarControlButtonTestNg, ToggleCollapseExpandCycleTest002, TestSize.Level1)
{
    CreateTabsWithSidebar(true);
    auto sideBarPattern = GetSideBarPattern();
    ASSERT_NE(sideBarPattern, nullptr);

    /**
     * @tc.steps: Two rapid clicks — in test env both execute (no animation guard)
     * @tc.expected: No crash, isCollapsed_ is false (collapse then expand)
     */
    sideBarPattern->OnCollapseButtonClick();
    sideBarPattern->OnCollapseButtonClick();
    EXPECT_FALSE(sideBarPattern->IsCollapsed());
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
    EXPECT_EQ(sbPattern->buttonPlaceholderNode_, nullptr);
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

} // namespace OHOS::Ace::NG

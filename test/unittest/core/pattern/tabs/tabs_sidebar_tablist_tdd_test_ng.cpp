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
#include "tabs_test_ng.h"

#include "test/mock/base/mock_system_properties.h"
#include "test/mock/core/common/mock_theme_manager.h"
#include "test/mock/core/pipeline/mock_pipeline_context.h"

#include "core/components/dialog/dialog_theme.h"
#include "core/components/tab_bar/tab_theme.h"
#include "core/components_ng/pattern/image/image_pattern.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"
#include "core/components_ng/pattern/swiper/swiper_pattern.h"
#include "core/components_ng/pattern/tabs/tab_content_model_ng.h"
#include "core/components_ng/pattern/tabs/tab_content_pattern.h"
#include "core/components_ng/pattern/tabs/tab_content_layout_property.h"
#include "core/components_ng/pattern/tabs/tabs_declaration.h"
#include "core/components_ng/pattern/tabs/tabs_layout_property.h"
#include "core/components_ng/pattern/tabs/tabs_model_ng.h"
#include "core/components_ng/pattern/tabs/tabs_node.h"
#include "core/components_ng/pattern/tabs/tabs_pattern.h"
#include "core/components_ng/pattern/tabs/tabs_side_bar_tab_list_pattern.h"
#include "core/components_ng/pattern/text/text_layout_property.h"
#include "core/components_ng/pattern/text/text_pattern.h"
#include "core/components_ng/syntax/shallow_builder.h"
#include "core/components_v2/inspector/inspector_constants.h"

namespace OHOS::Ace::NG {
namespace {
RefPtr<Theme> GetSidebarTabListTddTheme(ThemeType type)
{
    if (type == TabTheme::TypeId()) {
        auto themeConstants = TestNG::CreateThemeConstants(THEME_PATTERN_TAB);
        return TabTheme::Builder().Build(themeConstants);
    }
    return AceType::MakeRefPtr<DialogTheme>();
}

// Build a sidebar tab list tree: host > column > tabItem > row > (icon, text)
struct SidebarTabListTree {
    RefPtr<FrameNode> hostNode;
    RefPtr<FrameNode> columnNode;
    RefPtr<FrameNode> tabItemNode;
    RefPtr<FrameNode> rowNode;
    RefPtr<FrameNode> iconNode;
    RefPtr<FrameNode> textNode;
    RefPtr<TabsSideBarTabListPattern> pattern;

    void Build()
    {
        pattern = AceType::MakeRefPtr<TabsSideBarTabListPattern>();
        hostNode = FrameNode::CreateFrameNode(
            "SideBarTabList", ElementRegister::GetInstance()->MakeUniqueId(), pattern);
        columnNode = FrameNode::CreateFrameNode(
            V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
            AceType::MakeRefPtr<LinearLayoutPattern>(false));
        columnNode->MountToParent(hostNode);
        tabItemNode = FrameNode::CreateFrameNode(
            "TabsSideBarTabItem", ElementRegister::GetInstance()->MakeUniqueId(),
            AceType::MakeRefPtr<LinearLayoutPattern>(false));
        tabItemNode->MountToParent(columnNode);
        rowNode = FrameNode::CreateFrameNode(
            V2::ROW_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
            AceType::MakeRefPtr<LinearLayoutPattern>(true));
        rowNode->MountToParent(tabItemNode);
        iconNode = FrameNode::CreateFrameNode(
            V2::IMAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
            AceType::MakeRefPtr<ImagePattern>());
        iconNode->MountToParent(rowNode);
        textNode = FrameNode::CreateFrameNode(
            V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
            AceType::MakeRefPtr<TextPattern>());
        textNode->MountToParent(rowNode);
        pattern->SetColumnNode(columnNode);
    }
};

// Build a TabsNode with a swiper containing tabContentCount TabContentNodes,
// optionally configuring default visibility per item. When no visibilities are
// provided, the TabContentPattern's default visibility stays at its default.
struct SidebarTabsTree {
    RefPtr<TabsNode> tabsNode;
    RefPtr<FrameNode> swiperNode;
    RefPtr<TabsSideBarTabListPattern> tabListPattern;
    RefPtr<TabsLayoutProperty> tabsLayoutProperty;

    void Build(int32_t tabContentCount,
        const std::vector<TabContentDefaultVisibility>& visibilities = {})
    {
        tabsNode = AceType::MakeRefPtr<TabsNode>(
            V2::TABS_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
            AceType::MakeRefPtr<TabsPattern>(), false);
        tabsNode->InitializePatternAndContext();
        ElementRegister::GetInstance()->AddUINode(tabsNode);
        swiperNode = FrameNode::GetOrCreateFrameNode(
            V2::SWIPER_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
            []() { return AceType::MakeRefPtr<SwiperPattern>(); });
        swiperNode->MountToParent(tabsNode);
        for (int32_t i = 0; i < tabContentCount; ++i) {
            auto shallowBuilder = AceType::MakeRefPtr<ShallowBuilder>(nullptr);
            auto tabContentNode = FrameNode::CreateFrameNode(
                V2::TAB_CONTENT_ITEM_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
                AceType::MakeRefPtr<TabContentPattern>(shallowBuilder));
            if (i < static_cast<int32_t>(visibilities.size())) {
                tabContentNode->GetPattern<TabContentPattern>()->SetDefaultVisibility(visibilities[i]);
            }
            tabContentNode->MountToParent(swiperNode);
        }
        tabListPattern = AceType::MakeRefPtr<TabsSideBarTabListPattern>();
        tabListPattern->SetTabsNode(tabsNode);
        tabsLayoutProperty = tabsNode->GetLayoutProperty<TabsLayoutProperty>();
    }
};
} // namespace

class TabsSidebarTabListTddTestNg : public TabsTestNg {
public:
    static void SetUpTestSuite()
    {
        TestNG::SetUpTestSuite();
        MockPipelineContext::GetCurrent()->SetUseFlushUITasks(true);
        auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
        EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly([](ThemeType type) -> RefPtr<Theme> {
            return GetSidebarTabListTddTheme(type);
        });
        EXPECT_CALL(*themeManager, GetTheme(_, _))
            .WillRepeatedly([](ThemeType type, int32_t themeScopeId) -> RefPtr<Theme> {
                return GetSidebarTabListTddTheme(type);
            });
        MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    }
    static void TearDownTestSuite()
    {
        TestNG::TearDownTestSuite();
        g_isConfigChangePerform = false;
    }
};

// ===================== IsHiddenBySearchFilter (calc) =====================

/**
 * @tc.name: IsHiddenBySearchFilterEmptyText001
 * @tc.desc: When activeSearchText_ is empty, returns false regardless of filter
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarTabListTddTestNg, IsHiddenBySearchFilterEmptyText001, TestSize.Level1)
{
    SidebarTabsTree tree;
    tree.Build(1, { TabContentDefaultVisibility() });
    ASSERT_NE(tree.tabListPattern, nullptr);
    EXPECT_FALSE(tree.tabListPattern->IsHiddenBySearchFilter(0));
}

/**
 * @tc.name: IsHiddenBySearchFilterWithCustomFilter001
 * @tc.desc: When activeSearchText_ is set and filter returns true, tab is visible (not hidden)
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarTabListTddTestNg, IsHiddenBySearchFilterWithCustomFilter001, TestSize.Level1)
{
    SidebarTabsTree tree;
    tree.Build(1, { TabContentDefaultVisibility() });
    ASSERT_NE(tree.tabListPattern, nullptr);
    tree.tabListPattern->activeSearchText_ = u"test";
    tree.tabListPattern->activeSearchFilter_ = [](int32_t, const std::string&) { return true; };
    EXPECT_FALSE(tree.tabListPattern->IsHiddenBySearchFilter(0));
}

/**
 * @tc.name: IsHiddenBySearchFilterWithCustomFilterHidden001
 * @tc.desc: When filter returns false, tab is hidden
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarTabListTddTestNg, IsHiddenBySearchFilterWithCustomFilterHidden001, TestSize.Level1)
{
    SidebarTabsTree tree;
    tree.Build(1, { TabContentDefaultVisibility() });
    ASSERT_NE(tree.tabListPattern, nullptr);
    tree.tabListPattern->activeSearchText_ = u"test";
    tree.tabListPattern->activeSearchFilter_ = [](int32_t, const std::string&) { return false; };
    EXPECT_TRUE(tree.tabListPattern->IsHiddenBySearchFilter(0));
}

/**
 * @tc.name: IsHiddenBySearchFilterOutOfRange001
 * @tc.desc: When tabIndex is out of range with default filter, returns false
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarTabListTddTestNg, IsHiddenBySearchFilterOutOfRange001, TestSize.Level1)
{
    SidebarTabsTree tree;
    tree.Build(1, { TabContentDefaultVisibility() });
    ASSERT_NE(tree.tabListPattern, nullptr);
    tree.tabListPattern->activeSearchText_ = u"test";
    EXPECT_FALSE(tree.tabListPattern->IsHiddenBySearchFilter(99));
}

/**
 * @tc.name: IsHiddenBySearchFilterDefaultMatch001
 * @tc.desc: Default filter with matching text returns false (visible)
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarTabListTddTestNg, IsHiddenBySearchFilterDefaultMatch001, TestSize.Level1)
{
    SidebarTabsTree tree;
    tree.Build(1, { TabContentDefaultVisibility() });
    ASSERT_NE(tree.tabListPattern, nullptr);
    // Set tab text to something containing "abc"
    auto tabContentNode = AceType::DynamicCast<FrameNode>(tree.swiperNode->GetChildByIndex(0));
    ASSERT_NE(tabContentNode, nullptr);
    auto tabContentPattern = tabContentNode->GetPattern<TabContentPattern>();
    ASSERT_NE(tabContentPattern, nullptr);
    tabContentPattern->UpdateTabBarParamText("abcdef");
    tree.tabListPattern->activeSearchText_ = u"abc";
    // Default filter: fuzzy match by tab text → "abcdef" contains "abc" → visible
    EXPECT_FALSE(tree.tabListPattern->IsHiddenBySearchFilter(0));
}

/**
 * @tc.name: IsHiddenBySearchFilterDefaultNoMatch001
 * @tc.desc: Default filter with non-matching text returns true (hidden)
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarTabListTddTestNg, IsHiddenBySearchFilterDefaultNoMatch001, TestSize.Level1)
{
    SidebarTabsTree tree;
    tree.Build(1, { TabContentDefaultVisibility() });
    ASSERT_NE(tree.tabListPattern, nullptr);
    auto tabContentNode = AceType::DynamicCast<FrameNode>(tree.swiperNode->GetChildByIndex(0));
    ASSERT_NE(tabContentNode, nullptr);
    auto tabContentPattern = tabContentNode->GetPattern<TabContentPattern>();
    ASSERT_NE(tabContentPattern, nullptr);
    tabContentPattern->UpdateTabBarParamText("hello");
    tree.tabListPattern->activeSearchText_ = u"world";
    // Default filter: "hello" does not contain "world" → hidden
    EXPECT_TRUE(tree.tabListPattern->IsHiddenBySearchFilter(0));
}

// ===================== IsHiddenByDefaultVisibility =====================

/**
 * @tc.name: IsHiddenByDefaultVisibilityOutOfRange001
 * @tc.desc: IsHiddenByDefaultVisibility with out-of-range index returns false
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarTabListTddTestNg, IsHiddenByDefaultVisibilityOutOfRange001, TestSize.Level1)
{
    SidebarTabsTree tree;
    tree.Build(1, { TabContentDefaultVisibility() });
    ASSERT_NE(tree.tabListPattern, nullptr);
    EXPECT_FALSE(tree.tabListPattern->IsHiddenByDefaultVisibility(99));
}

/**
 * @tc.name: IsHiddenByDefaultVisibilityNullVisibility001
 * @tc.desc: When defaultVisibility is null, returns false
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarTabListTddTestNg, IsHiddenByDefaultVisibilityNullVisibility001, TestSize.Level1)
{
    SidebarTabsTree tree;
    TabContentDefaultVisibility vis;
    vis.isNull = true;
    tree.Build(1, { vis });
    ASSERT_NE(tree.tabListPattern, nullptr);
    EXPECT_FALSE(tree.tabListPattern->IsHiddenByDefaultVisibility(0));
}

/**
 * @tc.name: IsHiddenByDefaultVisibilityHiddenInSidebar001
 * @tc.desc: When visibility=HIDDEN and displayMode=SIDEBAR, returns true
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarTabListTddTestNg, IsHiddenByDefaultVisibilityHiddenInSidebar001, TestSize.Level1)
{
    SidebarTabsTree tree;
    TabContentDefaultVisibility vis;
    vis.isNull = false;
    vis.visibility = TabVisibility::HIDDEN;
    vis.displayMode = TabBarDisplayMode::SIDEBAR;
    tree.Build(1, { vis });
    ASSERT_NE(tree.tabListPattern, nullptr);
    EXPECT_TRUE(tree.tabListPattern->IsHiddenByDefaultVisibility(0));
}

/**
 * @tc.name: IsHiddenByDefaultVisibilityVisibleInSidebar001
 * @tc.desc: When visibility=VISIBLE in SIDEBAR mode, returns false
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarTabListTddTestNg, IsHiddenByDefaultVisibilityVisibleInSidebar001, TestSize.Level1)
{
    SidebarTabsTree tree;
    TabContentDefaultVisibility vis;
    vis.isNull = false;
    vis.visibility = TabVisibility::VISIBLE;
    vis.displayMode = TabBarDisplayMode::SIDEBAR;
    tree.Build(1, { vis });
    ASSERT_NE(tree.tabListPattern, nullptr);
    EXPECT_FALSE(tree.tabListPattern->IsHiddenByDefaultVisibility(0));
}

/**
 * @tc.name: IsHiddenByDefaultVisibilityHiddenInBottom001
 * @tc.desc: When HIDDEN in BOTTOMTABBAR (active=SIDEBAR), returns false
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarTabListTddTestNg, IsHiddenByDefaultVisibilityHiddenInBottom001, TestSize.Level1)
{
    SidebarTabsTree tree;
    TabContentDefaultVisibility vis;
    vis.isNull = false;
    vis.visibility = TabVisibility::HIDDEN;
    vis.displayMode = TabBarDisplayMode::BOTTOMTABBAR;
    tree.Build(1, { vis });
    ASSERT_NE(tree.tabListPattern, nullptr);
    EXPECT_FALSE(tree.tabListPattern->IsHiddenByDefaultVisibility(0));
}

// ===================== GetBuilderByContentId =====================

/**
 * @tc.name: GetBuilderByContentIdNew001
 * @tc.desc: First call for a contentId returns nullptr and stores the builder
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarTabListTddTestNg, GetBuilderByContentIdNew001, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<TabsSideBarTabListPattern>();
    ASSERT_NE(pattern, nullptr);
    auto builder = AceType::MakeRefPtr<FrameNode>("test", 1, AceType::MakeRefPtr<Pattern>());
    auto result = pattern->GetBuilderByContentId(100, builder);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: GetBuilderByContentIdExisting001
 * @tc.desc: Second call for the same contentId returns the previous builder
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarTabListTddTestNg, GetBuilderByContentIdExisting001, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<TabsSideBarTabListPattern>();
    ASSERT_NE(pattern, nullptr);
    auto builder1 = AceType::MakeRefPtr<FrameNode>("test1", 1, AceType::MakeRefPtr<Pattern>());
    auto builder2 = AceType::MakeRefPtr<FrameNode>("test2", 2, AceType::MakeRefPtr<Pattern>());
    pattern->GetBuilderByContentId(200, builder1);
    auto result = pattern->GetBuilderByContentId(200, builder2);
    EXPECT_EQ(result, builder1);
}

// ===================== RemoveBuilderByContentId =====================

/**
 * @tc.name: RemoveBuilderByContentId001
 * @tc.desc: After removing, GetBuilderByContentId returns nullptr for new entry
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarTabListTddTestNg, RemoveBuilderByContentId001, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<TabsSideBarTabListPattern>();
    ASSERT_NE(pattern, nullptr);
    auto builder = AceType::MakeRefPtr<FrameNode>("test", 1, AceType::MakeRefPtr<Pattern>());
    pattern->GetBuilderByContentId(300, builder);
    pattern->RemoveBuilderByContentId(300);
    auto builder2 = AceType::MakeRefPtr<FrameNode>("test2", 2, AceType::MakeRefPtr<Pattern>());
    auto result = pattern->GetBuilderByContentId(300, builder2);
    EXPECT_EQ(result, nullptr);
}

// ===================== SetCurrentIndex / InitCurrentIndex =====================

/**
 * @tc.name: SetCurrentIndexUpdates001
 * @tc.desc: SetCurrentIndex updates the current index value
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarTabListTddTestNg, SetCurrentIndexUpdates001, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<TabsSideBarTabListPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->InitCurrentIndex(0);
    pattern->SetCurrentIndex(3);
    EXPECT_EQ(pattern->GetCurrentIndex(), 3);
}

/**
 * @tc.name: SetCurrentIndexNegative001
 * @tc.desc: SetCurrentIndex with negative value still sets
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarTabListTddTestNg, SetCurrentIndexNegative001, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<TabsSideBarTabListPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->InitCurrentIndex(0);
    pattern->SetCurrentIndex(-1);
    EXPECT_EQ(pattern->GetCurrentIndex(), -1);
}

/**
 * @tc.name: SetCurrentIndexSameValue001
 * @tc.desc: SetCurrentIndex with same value is a no-op
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarTabListTddTestNg, SetCurrentIndexSameValue001, TestSize.Level1)
{
    SidebarTabListTree tree;
    tree.Build();
    tree.pattern->currentIndex_ = 2;
    tree.pattern->SetCurrentIndex(2);
    EXPECT_EQ(tree.pattern->currentIndex_, 2);
}

/**
 * @tc.name: InitCurrentIndexSameValue001
 * @tc.desc: InitCurrentIndex with same value is a no-op
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarTabListTddTestNg, InitCurrentIndexSameValue001, TestSize.Level1)
{
    SidebarTabListTree tree;
    tree.Build();
    tree.pattern->currentIndex_ = 0;
    tree.pattern->InitCurrentIndex(0);
    EXPECT_EQ(tree.pattern->currentIndex_, 0);
}

/**
 * @tc.name: InitCurrentIndexDifferentValue001
 * @tc.desc: InitCurrentIndex with different value updates currentIndex_
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarTabListTddTestNg, InitCurrentIndexDifferentValue001, TestSize.Level1)
{
    SidebarTabListTree tree;
    tree.Build();
    tree.pattern->currentIndex_ = 0;
    tree.pattern->InitCurrentIndex(1);
    EXPECT_EQ(tree.pattern->currentIndex_, 1);
}

// ===================== UpdateTabItemStyle =====================

/**
 * @tc.name: UpdateTabItemStyleSelected001
 * @tc.desc: UpdateTabItemStyle with selected=true sets background color
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarTabListTddTestNg, UpdateTabItemStyleSelected001, TestSize.Level1)
{
    SidebarTabListTree tree;
    tree.Build();
    tree.pattern->InitCurrentIndex(0);
    auto renderContext = tree.tabItemNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    renderContext->UpdateBackgroundColor(Color::TRANSPARENT);
    tree.pattern->UpdateTabItemStyle(0, true);
    // Selected → background color should be set to theme's actived color (not transparent)
    auto bgColor = renderContext->GetBackgroundColor();
    ASSERT_TRUE(bgColor.has_value());
    EXPECT_NE(bgColor.value(), Color::TRANSPARENT);
}

/**
 * @tc.name: UpdateTabItemStyleUnselected001
 * @tc.desc: UpdateTabItemStyle with selected=false sets transparent background
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarTabListTddTestNg, UpdateTabItemStyleUnselected001, TestSize.Level1)
{
    SidebarTabListTree tree;
    tree.Build();
    tree.pattern->InitCurrentIndex(0);
    auto renderContext = tree.tabItemNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    // First set to selected (non-transparent)
    tree.pattern->UpdateTabItemStyle(0, true);
    // Then set to unselected
    tree.pattern->UpdateTabItemStyle(0, false);
    auto bgColor = renderContext->GetBackgroundColor();
    ASSERT_TRUE(bgColor.has_value());
    EXPECT_EQ(bgColor.value(), Color::TRANSPARENT);
}

/**
 * @tc.name: UpdateTabItemStyleSelectedWithBoardColor001
 * @tc.desc: UpdateTabItemStyle with sidebarSelectedBoardColor set uses that color
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarTabListTddTestNg, UpdateTabItemStyleSelectedWithBoardColor001, TestSize.Level1)
{
    SidebarTabListTree tree;
    tree.Build();
    SidebarTabsTree tabsTree;
    tabsTree.Build(1);
    tree.pattern->SetTabsNode(tabsTree.tabsNode);
    tabsTree.tabsLayoutProperty->UpdateSidebarSelectedBoardColor(Color::RED);
    tree.pattern->InitCurrentIndex(0);
    tree.pattern->UpdateTabItemStyle(0, true);
    auto renderContext = tree.tabItemNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    auto bgColor = renderContext->GetBackgroundColor();
    ASSERT_TRUE(bgColor.has_value());
    EXPECT_EQ(bgColor.value(), Color::RED);
}

// ===================== RefreshAllTabItemColors =====================

/**
 * @tc.name: RefreshAllTabItemColorsNegativeIndex001
 * @tc.desc: RefreshAllTabItemColors with negative currentIndex does not call UpdateTabItemStyle
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarTabListTddTestNg, RefreshAllTabItemColorsNegativeIndex001, TestSize.Level1)
{
    SidebarTabListTree tree;
    tree.Build();
    tree.pattern->currentIndex_ = -1;
    tree.pattern->RefreshAllTabItemColors();
    EXPECT_EQ(tree.pattern->currentIndex_, -1);
}

// ===================== UpdateTextColorAndIconColor =====================

/**
 * @tc.name: UpdateTextColorAndIconColorSelectedNoTabsNode001
 * @tc.desc: With selected=true but no tabsNode set, UpdateTextColorAndIconColor early-returns
 *           (CHECK_NULL_VOID(tabsNode)) and does not write textColor; verify no side effect.
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarTabListTddTestNg, UpdateTextColorAndIconColorSelectedNoTabsNode001, TestSize.Level1)
{
    SidebarTabListTree tree;
    tree.Build();
    auto shallowBuilder = AceType::MakeRefPtr<ShallowBuilder>(nullptr);
    auto tabContentPattern = AceType::MakeRefPtr<TabContentPattern>(shallowBuilder);
    tree.pattern->UpdateTextColorAndIconColor(
        tree.rowNode, Color::RED, Color::BLUE, true, tabContentPattern);
    auto textLayoutProp = tree.textNode->GetLayoutProperty<TextLayoutProperty>();
    ASSERT_NE(textLayoutProp, nullptr);
    // tabsNode_ is null -> production code CHECK_NULL_VOID-returns before UpdateTextColor.
    EXPECT_FALSE(textLayoutProp->GetTextColor().has_value());
}

/**
 * @tc.name: UpdateTextColorAndIconColorUnselectedNoTabsNode001
 * @tc.desc: With selected=false and no tabsNode, UpdateTextColorAndIconColor early-returns
 *           (CHECK_NULL_VOID(tabsNode)) and does not write textColor; verify no side effect.
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarTabListTddTestNg, UpdateTextColorAndIconColorUnselectedNoTabsNode001, TestSize.Level1)
{
    SidebarTabListTree tree;
    tree.Build();
    auto shallowBuilder = AceType::MakeRefPtr<ShallowBuilder>(nullptr);
    auto tabContentPattern = AceType::MakeRefPtr<TabContentPattern>(shallowBuilder);
    tree.pattern->UpdateTextColorAndIconColor(
        tree.rowNode, Color::GREEN, Color::BLACK, false, tabContentPattern);
    auto textLayoutProp = tree.textNode->GetLayoutProperty<TextLayoutProperty>();
    ASSERT_NE(textLayoutProp, nullptr);
    // tabsNode_ is null -> production code CHECK_NULL_VOID-returns before UpdateTextColor.
    EXPECT_FALSE(textLayoutProp->GetTextColor().has_value());
}

/**
 * @tc.name: UpdateTextColorAndIconColorSelectedWithLabelColor001
 * @tc.desc: With selected=true and labelSelectedColor set by user, uses labelStyle color
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarTabListTddTestNg, UpdateTextColorAndIconColorSelectedWithLabelColor001, TestSize.Level1)
{
    SidebarTabListTree tree;
    tree.Build();
    // tabsNode_ must be set, otherwise UpdateTextColorAndIconColor CHECK_NULL_VOID-returns
    // before writing textColor. Do not set sidebar selected text color so the labelStyle
    // branch (set-by-user) is the one that supplies the color.
    SidebarTabsTree tabsTree;
    tabsTree.Build(1);
    tree.pattern->SetTabsNode(tabsTree.tabsNode);

    auto shallowBuilder = AceType::MakeRefPtr<ShallowBuilder>(nullptr);
    auto tabContentPattern = AceType::MakeRefPtr<TabContentPattern>(shallowBuilder);
    LabelStyle labelStyle;
    labelStyle.selectedColor = Color::GREEN;
    tabContentPattern->labelStyle_ = labelStyle;
    // Mount to a FrameNode with TabContentLayoutProperty so GetLayoutProperty works
    auto contentHost = FrameNode::CreateFrameNode(
        V2::TAB_CONTENT_ITEM_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), tabContentPattern);
    ASSERT_NE(contentHost, nullptr);
    auto contentProperty = contentHost->GetLayoutProperty<TabContentLayoutProperty>();
    ASSERT_NE(contentProperty, nullptr);
    contentProperty->UpdateLabelSelectedColorSetByUser(true);

    tree.pattern->UpdateTextColorAndIconColor(
        tree.rowNode, Color::RED, Color::BLUE, true, tabContentPattern);
    auto textLayoutProp = tree.textNode->GetLayoutProperty<TextLayoutProperty>();
    ASSERT_NE(textLayoutProp, nullptr);
    ASSERT_TRUE(textLayoutProp->GetTextColor().has_value());
    EXPECT_EQ(textLayoutProp->GetTextColor().value(), Color::GREEN);
}

/**
 * @tc.name: UpdateTextColorAndIconColorUnselectedWithLabelColor001
 * @tc.desc: With selected=false and labelUnselectedColor set by user, uses labelStyle color
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarTabListTddTestNg, UpdateTextColorAndIconColorUnselectedWithLabelColor001, TestSize.Level1)
{
    SidebarTabListTree tree;
    tree.Build();
    // tabsNode_ must be set, otherwise UpdateTextColorAndIconColor CHECK_NULL_VOID-returns
    // before writing textColor. Do not set sidebar unselected text color so the labelStyle
    // branch (set-by-user) is the one that supplies the color.
    SidebarTabsTree tabsTree;
    tabsTree.Build(1);
    tree.pattern->SetTabsNode(tabsTree.tabsNode);

    auto shallowBuilder = AceType::MakeRefPtr<ShallowBuilder>(nullptr);
    auto tabContentPattern = AceType::MakeRefPtr<TabContentPattern>(shallowBuilder);
    LabelStyle labelStyle;
    labelStyle.unselectedColor = Color::FromRGB(255, 255, 0);
    tabContentPattern->labelStyle_ = labelStyle;
    auto contentHost = FrameNode::CreateFrameNode(
        V2::TAB_CONTENT_ITEM_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), tabContentPattern);
    ASSERT_NE(contentHost, nullptr);
    auto contentProperty = contentHost->GetLayoutProperty<TabContentLayoutProperty>();
    ASSERT_NE(contentProperty, nullptr);
    contentProperty->UpdateLabelUnselectedColorSetByUser(true);

    tree.pattern->UpdateTextColorAndIconColor(
        tree.rowNode, Color::RED, Color::BLUE, false, tabContentPattern);
    auto textLayoutProp = tree.textNode->GetLayoutProperty<TextLayoutProperty>();
    ASSERT_NE(textLayoutProp, nullptr);
    ASSERT_TRUE(textLayoutProp->GetTextColor().has_value());
    EXPECT_EQ(textLayoutProp->GetTextColor().value(), Color::FromRGB(255, 255, 0));
}

/**
 * @tc.name: UpdateTextColorAndIconColorSelectedSidebarTextColor001
 * @tc.desc: With selected=true and no user label color, uses sidebarSelectedTextColor
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarTabListTddTestNg, UpdateTextColorAndIconColorSelectedSidebarTextColor001, TestSize.Level1)
{
    SidebarTabListTree tree;
    tree.Build();
    SidebarTabsTree tabsTree;
    tabsTree.Build(1);
    tree.pattern->SetTabsNode(tabsTree.tabsNode);
    tabsTree.tabsLayoutProperty->UpdateSidebarSelectedTextColor(Color::FromRGB(255, 0, 255));

    auto shallowBuilder = AceType::MakeRefPtr<ShallowBuilder>(nullptr);
    auto tabContentPattern = AceType::MakeRefPtr<TabContentPattern>(shallowBuilder);

    tree.pattern->UpdateTextColorAndIconColor(
        tree.rowNode, Color::RED, Color::BLUE, true, tabContentPattern);
    auto textLayoutProp = tree.textNode->GetLayoutProperty<TextLayoutProperty>();
    ASSERT_NE(textLayoutProp, nullptr);
    ASSERT_TRUE(textLayoutProp->GetTextColor().has_value());
    EXPECT_EQ(textLayoutProp->GetTextColor().value(), Color::FromRGB(255, 0, 255));
}

/**
 * @tc.name: UpdateTextColorAndIconColorUnselectedSidebarTextColor001
 * @tc.desc: With selected=false and no user label color, uses sidebarUnselectedTextColor
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarTabListTddTestNg, UpdateTextColorAndIconColorUnselectedSidebarTextColor001, TestSize.Level1)
{
    SidebarTabListTree tree;
    tree.Build();
    SidebarTabsTree tabsTree;
    tabsTree.Build(1);
    tree.pattern->SetTabsNode(tabsTree.tabsNode);
    tabsTree.tabsLayoutProperty->UpdateSidebarUnselectedTextColor(Color::FromRGB(0, 255, 255));

    auto shallowBuilder = AceType::MakeRefPtr<ShallowBuilder>(nullptr);
    auto tabContentPattern = AceType::MakeRefPtr<TabContentPattern>(shallowBuilder);

    tree.pattern->UpdateTextColorAndIconColor(
        tree.rowNode, Color::RED, Color::BLUE, false, tabContentPattern);
    auto textLayoutProp = tree.textNode->GetLayoutProperty<TextLayoutProperty>();
    ASSERT_NE(textLayoutProp, nullptr);
    ASSERT_TRUE(textLayoutProp->GetTextColor().has_value());
    EXPECT_EQ(textLayoutProp->GetTextColor().value(), Color::FromRGB(0, 255, 255));
}

// ===================== ApplyDefaultVisibility =====================

/**
 * @tc.name: ApplyDefaultVisibilitySidebarStyleHidden001
 * @tc.desc: ApplyDefaultVisibility with SIDEBAR style and HIDDEN visibility sets GONE
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarTabListTddTestNg, ApplyDefaultVisibilitySidebarStyleHidden001, TestSize.Level1)
{
    SidebarTabListTree tree;
    tree.Build();
    TabContentDefaultVisibility vis;
    vis.isNull = false;
    vis.visibility = TabVisibility::HIDDEN;
    vis.displayMode = TabBarDisplayMode::SIDEBAR;
    SidebarTabsTree tabsTree;
    tabsTree.Build(1, { vis });
    tree.pattern->SetTabsNode(tabsTree.tabsNode);
    tabsTree.tabsLayoutProperty->UpdateBarLayoutStyle(TabBarLayoutStyle::SIDEBAR);
    tree.pattern->ApplyDefaultVisibility();
    auto property = tree.tabItemNode->GetLayoutProperty();
    ASSERT_NE(property, nullptr);
    EXPECT_EQ(property->GetVisibilityValue(VisibleType::VISIBLE), VisibleType::GONE);
}

/**
 * @tc.name: ApplyDefaultVisibilitySidebarStyleVisible001
 * @tc.desc: ApplyDefaultVisibility with SIDEBAR style and VISIBLE visibility sets VISIBLE
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarTabListTddTestNg, ApplyDefaultVisibilitySidebarStyleVisible001, TestSize.Level1)
{
    SidebarTabListTree tree;
    tree.Build();
    TabContentDefaultVisibility vis;
    vis.isNull = false;
    vis.visibility = TabVisibility::VISIBLE;
    vis.displayMode = TabBarDisplayMode::SIDEBAR;
    SidebarTabsTree tabsTree;
    tabsTree.Build(1, { vis });
    tree.pattern->SetTabsNode(tabsTree.tabsNode);
    tabsTree.tabsLayoutProperty->UpdateBarLayoutStyle(TabBarLayoutStyle::SIDEBAR);
    tree.pattern->ApplyDefaultVisibility();
    auto property = tree.tabItemNode->GetLayoutProperty();
    ASSERT_NE(property, nullptr);
    EXPECT_EQ(property->GetVisibilityValue(VisibleType::GONE), VisibleType::VISIBLE);
}

/**
 * @tc.name: ApplyDefaultVisibilityWithSearchFilter001
 * @tc.desc: ApplyDefaultVisibility with search filter set hides non-matching items
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarTabListTddTestNg, ApplyDefaultVisibilityWithSearchFilter001, TestSize.Level1)
{
    SidebarTabListTree tree;
    tree.Build();
    TabContentDefaultVisibility vis;
    vis.isNull = true;
    SidebarTabsTree tabsTree;
    tabsTree.Build(1, { vis });
    tree.pattern->SetTabsNode(tabsTree.tabsNode);
    tabsTree.tabsLayoutProperty->UpdateBarLayoutStyle(TabBarLayoutStyle::SIDEBAR);
    // Set a search filter that hides all items
    tree.pattern->activeSearchText_ = u"xyz";
    tree.pattern->activeSearchFilter_ = [](int32_t, const std::string&) { return false; };
    tree.pattern->ApplyDefaultVisibility();
    auto property = tree.tabItemNode->GetLayoutProperty();
    ASSERT_NE(property, nullptr);
    EXPECT_EQ(property->GetVisibilityValue(VisibleType::VISIBLE), VisibleType::GONE);
}

// ===================== ApplySearchFilter =====================

/**
 * @tc.name: ApplySearchFilterEmptyText001
 * @tc.desc: ApplySearchFilter with empty search text makes all tabs visible
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarTabListTddTestNg, ApplySearchFilterEmptyText001, TestSize.Level1)
{
    SidebarTabListTree tree;
    tree.Build();
    SidebarTabsTree tabsTree;
    tabsTree.Build(1);
    tree.pattern->SetTabsNode(tabsTree.tabsNode);
    tree.pattern->ApplySearchFilter(nullptr, u"");
    EXPECT_TRUE(tree.pattern->activeSearchText_.empty());
}

/**
 * @tc.name: ApplySearchFilterWithText001
 * @tc.desc: ApplySearchFilter with search text stores the filter state
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarTabListTddTestNg, ApplySearchFilterWithText001, TestSize.Level1)
{
    SidebarTabListTree tree;
    tree.Build();
    SidebarTabsTree tabsTree;
    tabsTree.Build(1);
    tree.pattern->SetTabsNode(tabsTree.tabsNode);
    tree.pattern->ApplySearchFilter(
        [](int32_t, const std::string&) { return true; }, u"abc");
    EXPECT_EQ(tree.pattern->activeSearchText_, u"abc");
    EXPECT_TRUE(tree.pattern->activeSearchFilter_ != nullptr);
}

// ===================== SetColumnNode / GetTabItemContainerNode =====================

/**
 * @tc.name: GetTabItemContainerNodeWithColumn001
 * @tc.desc: GetTabItemContainerNode with column returns the column node
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarTabListTddTestNg, GetTabItemContainerNodeWithColumn001, TestSize.Level1)
{
    SidebarTabListTree tree;
    tree.Build();
    auto container = tree.pattern->GetTabItemContainerNode();
    EXPECT_EQ(container, tree.columnNode);
}
} // namespace OHOS::Ace::NG

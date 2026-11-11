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
#include "test/mock/adapter/ohos/osal/mock_system_properties.h"
#include "test/mock/frameworks/core/common/mock_theme_manager.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"

#include "base/utils/string_utils.h"
#include "core/components/dialog/dialog_theme.h"
#include "core/components/tab_bar/tab_theme.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/image/image_layout_property.h"
#include "core/components_ng/pattern/image/image_pattern.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_property.h"
#include "core/components_ng/pattern/swiper/swiper_pattern.h"
#include "core/components_ng/pattern/tabs/tab_content_model_ng.h"
#include "core/components_ng/pattern/tabs/tab_content_node.h"
#include "core/components_ng/pattern/tabs/tab_content_pattern.h"
#include "core/components_ng/pattern/tabs/tabs_declaration.h"
#include "core/components_ng/syntax/shallow_builder.h"
#include "core/components_ng/pattern/tabs/tabs_layout_property.h"
#include "core/components_ng/pattern/tabs/tabs_model_ng.h"
#include "core/components_ng/pattern/tabs/tabs_node.h"
#include "core/components_ng/pattern/tabs/tabs_pattern.h"
#include "core/components_ng/pattern/tabs/tabs_side_bar_tab_list_pattern.h"
#include "core/components_ng/pattern/text/text_layout_property.h"
#include "core/components_ng/pattern/text/text_pattern.h"
#include "core/pipeline_ng/pipeline_context.h"
#include "core/components_v2/inspector/inspector_constants.h"

namespace OHOS::Ace::NG {
namespace {
RefPtr<Theme> GetTabListTheme(ThemeType type)
{
    if (type == TabTheme::TypeId()) {
        auto themeConstants = TestNG::CreateThemeConstants(THEME_PATTERN_TAB);
        return TabTheme::Builder().Build(themeConstants);
    }
    return AceType::MakeRefPtr<DialogTheme>();
}

// Build a minimal sidebar tab list node tree:
//   hostNode (TabsSideBarTabListPattern)
//     └── columnNode
//           └── tabItemNode
//                 └── rowNode
//                       ├── iconNode (Image)
//                       └── textNode (Text)
struct SideBarTabListTestTree {
    RefPtr<FrameNode> hostNode;
    RefPtr<FrameNode> columnNode;
    RefPtr<FrameNode> tabItemNode;
    RefPtr<FrameNode> rowNode;
    RefPtr<FrameNode> iconNode;
    RefPtr<FrameNode> textNode;
    RefPtr<TabsSideBarTabListPattern> pattern;

    void Build()
    {
        // Create host node with TabsSideBarTabListPattern
        pattern = AceType::MakeRefPtr<TabsSideBarTabListPattern>();
        hostNode = FrameNode::CreateFrameNode(
            "SideBarTabList", ElementRegister::GetInstance()->MakeUniqueId(), pattern);
        ASSERT_NE(hostNode, nullptr);

        // Create column node
        columnNode = FrameNode::CreateFrameNode(
            V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
            AceType::MakeRefPtr<LinearLayoutPattern>(false));
        ASSERT_NE(columnNode, nullptr);
        columnNode->MountToParent(hostNode);

        // Create tab item node (pattern type doesn't matter —
        // OnFontScaleConfigurationUpdate only checks child tag == ROW_ETS_TAG)
        tabItemNode = FrameNode::CreateFrameNode(
            "TabsSideBarTabItem", ElementRegister::GetInstance()->MakeUniqueId(),
            AceType::MakeRefPtr<LinearLayoutPattern>(false));
        ASSERT_NE(tabItemNode, nullptr);
        tabItemNode->MountToParent(columnNode);

        // Create row node
        rowNode = FrameNode::CreateFrameNode(
            V2::ROW_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
            AceType::MakeRefPtr<LinearLayoutPattern>(true));
        ASSERT_NE(rowNode, nullptr);
        rowNode->MountToParent(tabItemNode);

        // Create icon node
        iconNode = FrameNode::CreateFrameNode(
            V2::IMAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
            AceType::MakeRefPtr<ImagePattern>());
        ASSERT_NE(iconNode, nullptr);
        iconNode->MountToParent(rowNode);

        // Create text node
        textNode = FrameNode::CreateFrameNode(
            V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
            AceType::MakeRefPtr<TextPattern>());
        ASSERT_NE(textNode, nullptr);
        textNode->MountToParent(rowNode);

        // Set columnNode to pattern
        pattern->SetColumnNode(columnNode);
    }
};

// Helper: get the top margin CalcLength from a text node, or nullopt if absent
std::optional<CalcLength> GetTextNodeMarginTop(const RefPtr<FrameNode>& textNode)
{
    auto textLayoutProperty = textNode->GetLayoutProperty<TextLayoutProperty>();
    if (!textLayoutProperty) {
        return std::nullopt;
    }
    const auto& margin = textLayoutProperty->GetMarginProperty();
    if (!margin) {
        return std::nullopt;
    }
    return margin->top;
}

// Build a minimal TabsNode with a swiper containing tabContentCount TabContentNodes.
// Each TabContentPattern gets the provided defaultVisibility.
// Returns the TabsNode and the tab list pattern wired to it.
struct SideBarVisibilityTestTree {
    RefPtr<TabsNode> tabsNode;
    RefPtr<FrameNode> swiperNode;
    RefPtr<TabsSideBarTabListPattern> tabListPattern;

    void Build(int32_t tabContentCount, const std::vector<TabContentDefaultVisibility>& visibilities)
    {
        auto nodeId = ElementRegister::GetInstance()->MakeUniqueId();
        tabsNode = AceType::MakeRefPtr<TabsNode>(
            V2::TABS_ETS_TAG, nodeId, AceType::MakeRefPtr<TabsPattern>(), false);
        tabsNode->InitializePatternAndContext();
        ElementRegister::GetInstance()->AddUINode(tabsNode);

        swiperNode = FrameNode::GetOrCreateFrameNode(
            V2::SWIPER_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
            []() { return AceType::MakeRefPtr<SwiperPattern>(); });
        swiperNode->MountToParent(tabsNode);

        for (int32_t i = 0; i < tabContentCount && i < static_cast<int32_t>(visibilities.size()); ++i) {
            auto shallowBuilder = AceType::MakeRefPtr<ShallowBuilder>(nullptr);
            auto tabContentNode = FrameNode::CreateFrameNode(
                V2::TAB_CONTENT_ITEM_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
                AceType::MakeRefPtr<TabContentPattern>(shallowBuilder));
            tabContentNode->GetPattern<TabContentPattern>()->SetDefaultVisibility(visibilities[i]);
            tabContentNode->MountToParent(swiperNode);
        }

        tabListPattern = AceType::MakeRefPtr<TabsSideBarTabListPattern>();
        tabListPattern->SetTabsNode(tabsNode);
    }
};
} // namespace

class TabsSideBarTabListTestNg : public TabsTestNg {
public:
    static void SetUpTestSuite()
    {
        TestNG::SetUpTestSuite();
        MockPipelineContext::GetCurrent()->SetUseFlushUITasks(true);
        auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
        EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly([](ThemeType type) -> RefPtr<Theme> {
            return GetTabListTheme(type);
        });
        EXPECT_CALL(*themeManager, GetTheme(_, _))
            .WillRepeatedly([](ThemeType type, int32_t themeScopeId) -> RefPtr<Theme> {
                return GetTabListTheme(type);
            });
        MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    }
    static void TearDownTestSuite()
    {
        TestNG::TearDownTestSuite();
        g_isConfigChangePerform = false;
    }
};

/**
 * @tc.name: OnFontScaleConfigurationUpdate001
 * @tc.desc: Test OnFontScaleConfigurationUpdate with default font scale (1.0)
 *           verticalMargin should be paddingLevel4 (8vp)
 * @tc.type: FUNC
 */
HWTEST_F(TabsSideBarTabListTestNg, OnFontScaleConfigurationUpdate001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Build sidebar tab list node tree
     */
    SideBarTabListTestTree tree;
    tree.Build();

    /**
     * @tc.steps: step2. Set font scale to 1.0 and call OnFontScaleConfigurationUpdate
     */
    auto pipeline = tree.hostNode->GetContext();
    ASSERT_NE(pipeline, nullptr);
    pipeline->fontScale_ = 1.0f;

    tree.pattern->OnFontScaleConfigurationUpdate();

    /**
     * @tc.steps: step3. Verify text node margin top equals paddingLevel4 (8vp)
     */
    auto marginTop = GetTextNodeMarginTop(tree.textNode);
    ASSERT_TRUE(marginTop.has_value());
    EXPECT_EQ(marginTop.value(), CalcLength(Dimension(8.0_vp)));
}

/**
 * @tc.name: OnFontScaleConfigurationUpdate002
 * @tc.desc: Test OnFontScaleConfigurationUpdate with font scale 1.75
 *           verticalMargin should be paddingLevel8 (16vp)
 * @tc.type: FUNC
 */
HWTEST_F(TabsSideBarTabListTestNg, OnFontScaleConfigurationUpdate002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Build sidebar tab list node tree
     */
    SideBarTabListTestTree tree;
    tree.Build();

    /**
     * @tc.steps: step2. Set font scale to 1.75 and call OnFontScaleConfigurationUpdate
     */
    auto pipeline = tree.hostNode->GetContext();
    ASSERT_NE(pipeline, nullptr);
    pipeline->fontScale_ = 1.75f;

    tree.pattern->OnFontScaleConfigurationUpdate();

    /**
     * @tc.steps: step3. Verify text node margin top equals paddingLevel8 (16vp)
     */
    auto marginTop = GetTextNodeMarginTop(tree.textNode);
    ASSERT_TRUE(marginTop.has_value());
    EXPECT_EQ(marginTop.value(), CalcLength(Dimension(16.0_vp)));
}

/**
 * @tc.name: OnFontScaleConfigurationUpdate003
 * @tc.desc: Test OnFontScaleConfigurationUpdate with font scale 2.0
 *           verticalMargin should be paddingLevel10 (20vp)
 * @tc.type: FUNC
 */
HWTEST_F(TabsSideBarTabListTestNg, OnFontScaleConfigurationUpdate003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Build sidebar tab list node tree
     */
    SideBarTabListTestTree tree;
    tree.Build();

    /**
     * @tc.steps: step2. Set font scale to 2.0 and call OnFontScaleConfigurationUpdate
     */
    auto pipeline = tree.hostNode->GetContext();
    ASSERT_NE(pipeline, nullptr);
    pipeline->fontScale_ = 2.0f;

    tree.pattern->OnFontScaleConfigurationUpdate();

    /**
     * @tc.steps: step3. Verify text node margin top equals paddingLevel10 (20vp)
     */
    auto marginTop = GetTextNodeMarginTop(tree.textNode);
    ASSERT_TRUE(marginTop.has_value());
    EXPECT_EQ(marginTop.value(), CalcLength(Dimension(20.0_vp)));
}

/**
 * @tc.name: SetCurrentIndex001
 * @tc.desc: Test SetCurrentIndex updates the current index
 * @tc.type: FUNC
 */
HWTEST_F(TabsSideBarTabListTestNg, SetCurrentIndex001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create a TabsSideBarTabListPattern directly
     */
    auto pattern = AceType::MakeRefPtr<TabsSideBarTabListPattern>();
    ASSERT_NE(pattern, nullptr);

    /**
     * @tc.steps: step2. Init to index 0 then set to index 2
     * @tc.expected: GetCurrentIndex returns the new index
     */
    pattern->InitCurrentIndex(0);
    EXPECT_EQ(pattern->GetCurrentIndex(), 0);

    pattern->SetCurrentIndex(2);
    EXPECT_EQ(pattern->GetCurrentIndex(), 2);
}

/**
 * @tc.name: SetCurrentIndexSameValue001
 * @tc.desc: Test SetCurrentIndex with same value exercises early return
 * @tc.type: FUNC
 */
HWTEST_F(TabsSideBarTabListTestNg, SetCurrentIndexSameValue001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create a TabsSideBarTabListPattern directly
     */
    auto pattern = AceType::MakeRefPtr<TabsSideBarTabListPattern>();
    ASSERT_NE(pattern, nullptr);

    /**
     * @tc.steps: step2. Init to index 1, then SetCurrentIndex to same value
     * @tc.expected: Index remains 1 (early return path)
     */
    pattern->InitCurrentIndex(1);
    EXPECT_EQ(pattern->GetCurrentIndex(), 1);

    pattern->SetCurrentIndex(1);
    EXPECT_EQ(pattern->GetCurrentIndex(), 1);
}

/**
 * @tc.name: InitCurrentIndexSameValue001
 * @tc.desc: Test InitCurrentIndex with same value exercises early return
 * @tc.type: FUNC
 */
HWTEST_F(TabsSideBarTabListTestNg, InitCurrentIndexSameValue001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create a TabsSideBarTabListPattern directly
     */
    auto pattern = AceType::MakeRefPtr<TabsSideBarTabListPattern>();
    ASSERT_NE(pattern, nullptr);

    /**
     * @tc.steps: step2. Init to index 0, then InitCurrentIndex again with 0
     * @tc.expected: Index remains 0 (early return path)
     */
    pattern->InitCurrentIndex(0);
    EXPECT_EQ(pattern->GetCurrentIndex(), 0);

    pattern->InitCurrentIndex(0);
    EXPECT_EQ(pattern->GetCurrentIndex(), 0);
}

/**
 * @tc.name: InitCurrentIndexSequential001
 * @tc.desc: Test InitCurrentIndex with sequential different values
 * @tc.type: FUNC
 */
HWTEST_F(TabsSideBarTabListTestNg, InitCurrentIndexSequential001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create a TabsSideBarTabListPattern directly
     */
    auto pattern = AceType::MakeRefPtr<TabsSideBarTabListPattern>();
    ASSERT_NE(pattern, nullptr);

    /**
     * @tc.steps: step2. Init to different indices sequentially
     * @tc.expected: Each time GetCurrentIndex returns the latest value
     */
    EXPECT_EQ(pattern->GetCurrentIndex(), -1); // default value

    pattern->InitCurrentIndex(0);
    EXPECT_EQ(pattern->GetCurrentIndex(), 0);

    pattern->InitCurrentIndex(3);
    EXPECT_EQ(pattern->GetCurrentIndex(), 3);
}

/**
 * @tc.name: IsHiddenByDefaultVisibility001
 * @tc.desc: Test IsHiddenByDefaultVisibility returns true when tab is HIDDEN in SIDEBAR mode
 * @tc.type: FUNC
 */
HWTEST_F(TabsSideBarTabListTestNg, IsHiddenByDefaultVisibility001, TestSize.Level1)
{
    SideBarVisibilityTestTree tree;
    TabContentDefaultVisibility hiddenInSidebar;
    hiddenInSidebar.isNull = false;
    hiddenInSidebar.visibility = TabVisibility::HIDDEN;
    hiddenInSidebar.displayMode = TabBarDisplayMode::SIDEBAR;
    tree.Build(1, { hiddenInSidebar });
    ASSERT_NE(tree.tabListPattern, nullptr);

    EXPECT_TRUE(tree.tabListPattern->IsHiddenByDefaultVisibility(0));
}

/**
 * @tc.name: IsHiddenByDefaultVisibility002
 * @tc.desc: Test IsHiddenByDefaultVisibility returns false when tab is HIDDEN in BOTTOMTABBAR but active is SIDEBAR
 * @tc.type: FUNC
 */
HWTEST_F(TabsSideBarTabListTestNg, IsHiddenByDefaultVisibility002, TestSize.Level1)
{
    SideBarVisibilityTestTree tree;
    TabContentDefaultVisibility hiddenInBottom;
    hiddenInBottom.isNull = false;
    hiddenInBottom.visibility = TabVisibility::HIDDEN;
    hiddenInBottom.displayMode = TabBarDisplayMode::BOTTOMTABBAR;
    tree.Build(1, { hiddenInBottom });
    ASSERT_NE(tree.tabListPattern, nullptr);

    EXPECT_FALSE(tree.tabListPattern->IsHiddenByDefaultVisibility(0));
}

/**
 * @tc.name: IsHiddenByDefaultVisibility003
 * @tc.desc: Test IsHiddenByDefaultVisibility returns false when tab is VISIBLE
 * @tc.type: FUNC
 */
HWTEST_F(TabsSideBarTabListTestNg, IsHiddenByDefaultVisibility003, TestSize.Level1)
{
    SideBarVisibilityTestTree tree;
    TabContentDefaultVisibility visible;
    visible.isNull = false;
    visible.visibility = TabVisibility::VISIBLE;
    tree.Build(1, { visible });
    ASSERT_NE(tree.tabListPattern, nullptr);

    EXPECT_FALSE(tree.tabListPattern->IsHiddenByDefaultVisibility(0));
}

/**
 * @tc.name: IsHiddenByDefaultVisibility004
 * @tc.desc: Test IsHiddenByDefaultVisibility returns false for out-of-range tabIndex
 * @tc.type: FUNC
 */
HWTEST_F(TabsSideBarTabListTestNg, IsHiddenByDefaultVisibility004, TestSize.Level1)
{
    SideBarVisibilityTestTree tree;
    TabContentDefaultVisibility vis;
    vis.isNull = true;
    tree.Build(1, { vis });
    ASSERT_NE(tree.tabListPattern, nullptr);

    EXPECT_FALSE(tree.tabListPattern->IsHiddenByDefaultVisibility(5));
}

/**
 * @tc.name: SetCurrentIndexWithoutAnimation001
 * @tc.desc: When isCustomAnimation is false (default), SetCurrentIndex routes to
 *           SwipeToWithoutAnimation and does NOT call SwipeTo
 * @tc.type: FUNC
 */
HWTEST_F(TabsSideBarTabListTestNg, SetCurrentIndexWithoutAnimation001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Build a sidebar tabs tree with one tab content
     */
    SideBarVisibilityTestTree tree;
    tree.Build(1, { TabContentDefaultVisibility() });
    ASSERT_NE(tree.tabListPattern, nullptr);
    ASSERT_NE(tree.tabsNode, nullptr);

    /**
     * @tc.steps: step2. Inject a SwiperController with recording impls
     */
    auto controller = AceType::MakeRefPtr<SwiperController>();
    ASSERT_NE(controller, nullptr);
    int32_t swipeToIndex = -1;
    int32_t swipeToWithoutAnimIndex = -1;
    int32_t swipeToCallCount = 0;
    int32_t swipeToWithoutAnimCallCount = 0;
    controller->SetSwipeToImpl([&swipeToIndex, &swipeToCallCount](int32_t index, bool reverse) {
        swipeToIndex = index;
        ++swipeToCallCount;
    });
    controller->SetSwipeToWithoutAnimationImpl(
        [&swipeToWithoutAnimIndex, &swipeToWithoutAnimCallCount](int32_t index) {
            swipeToWithoutAnimIndex = index;
            ++swipeToWithoutAnimCallCount;
        });
    tree.tabListPattern->SetSwiperController(controller);

    /**
     * @tc.steps: step3. Ensure isCustomAnimation is false (the default)
     */
    auto tabsPattern = tree.tabsNode->GetPattern<TabsPattern>();
    ASSERT_NE(tabsPattern, nullptr);
    tabsPattern->SetIsCustomAnimation(false);
    EXPECT_FALSE(tabsPattern->GetIsCustomAnimation());

    /**
     * @tc.steps: step4. Init to 0 then SetCurrentIndex to 2
     * @tc.expected: SwipeToWithoutAnimation called with 2; SwipeTo NOT called
     */
    tree.tabListPattern->InitCurrentIndex(0);
    EXPECT_EQ(tree.tabListPattern->GetCurrentIndex(), 0);
    tree.tabListPattern->SetCurrentIndex(2);
    EXPECT_EQ(tree.tabListPattern->GetCurrentIndex(), 2);

    EXPECT_EQ(swipeToWithoutAnimCallCount, 1);
    EXPECT_EQ(swipeToWithoutAnimIndex, 2);
    EXPECT_EQ(swipeToCallCount, 0);
}

/**
 * @tc.name: SetCurrentIndexWithCustomAnimation001
 * @tc.desc: When isCustomAnimation is true, SetCurrentIndex routes to SwipeTo
 *           and does NOT call SwipeToWithoutAnimation
 * @tc.type: FUNC
 */
HWTEST_F(TabsSideBarTabListTestNg, SetCurrentIndexWithCustomAnimation001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Build a sidebar tabs tree with one tab content
     */
    SideBarVisibilityTestTree tree;
    tree.Build(1, { TabContentDefaultVisibility() });
    ASSERT_NE(tree.tabListPattern, nullptr);
    ASSERT_NE(tree.tabsNode, nullptr);

    /**
     * @tc.steps: step2. Inject a SwiperController with recording impls
     */
    auto controller = AceType::MakeRefPtr<SwiperController>();
    ASSERT_NE(controller, nullptr);
    int32_t swipeToIndex = -1;
    int32_t swipeToWithoutAnimIndex = -1;
    int32_t swipeToCallCount = 0;
    int32_t swipeToWithoutAnimCallCount = 0;
    controller->SetSwipeToImpl([&swipeToIndex, &swipeToCallCount](int32_t index, bool reverse) {
        swipeToIndex = index;
        ++swipeToCallCount;
    });
    controller->SetSwipeToWithoutAnimationImpl(
        [&swipeToWithoutAnimIndex, &swipeToWithoutAnimCallCount](int32_t index) {
            swipeToWithoutAnimIndex = index;
            ++swipeToWithoutAnimCallCount;
        });
    tree.tabListPattern->SetSwiperController(controller);

    /**
     * @tc.steps: step3. Enable custom animation on TabsPattern
     */
    auto tabsPattern = tree.tabsNode->GetPattern<TabsPattern>();
    ASSERT_NE(tabsPattern, nullptr);
    tabsPattern->SetIsCustomAnimation(true);
    EXPECT_TRUE(tabsPattern->GetIsCustomAnimation());

    /**
     * @tc.steps: step4. Init to 0 then SetCurrentIndex to 2
     * @tc.expected: SwipeTo called with 2; SwipeToWithoutAnimation NOT called
     */
    tree.tabListPattern->InitCurrentIndex(0);
    EXPECT_EQ(tree.tabListPattern->GetCurrentIndex(), 0);
    tree.tabListPattern->SetCurrentIndex(2);
    EXPECT_EQ(tree.tabListPattern->GetCurrentIndex(), 2);

    EXPECT_EQ(swipeToCallCount, 1);
    EXPECT_EQ(swipeToIndex, 2);
    EXPECT_EQ(swipeToWithoutAnimCallCount, 0);
}

// Helper for section-related tests: builds a TabsNode tree with real TabContentNode
// children (so GetSideBarTabBarItemId works), an attached columnNode with matching
// tab item FrameNodes, and configurable barLayoutStyle + per-tab section/text.
// Each entry in `tabConfigs` configures one TabContentNode: its tab text and an
// optional SidebarSection. A matching tab item FrameNode is created in columnNode
// using the TabContentNode's sideBarTabBarItemId so FindSwiperIndexForTabItem can
// resolve it during RebuildSections/ApplySectionLayout.
struct SideBarSectionTestTree {
    struct TabConfig {
        std::string text;
        std::optional<OHOS::Ace::SidebarSection> section;
    };

    RefPtr<TabsNode> tabsNode;
    RefPtr<FrameNode> swiperNode;
    RefPtr<FrameNode> hostNode;
    RefPtr<FrameNode> columnNode;
    RefPtr<TabsSideBarTabListPattern> tabListPattern;
    std::vector<RefPtr<TabContentNode>> tabContentNodes;
    std::vector<RefPtr<FrameNode>> tabItemNodes;

    void Build(const std::vector<TabConfig>& tabConfigs, TabBarLayoutStyle style = TabBarLayoutStyle::SIDEBAR)
    {
        // 1. TabsNode
        auto nodeId = ElementRegister::GetInstance()->MakeUniqueId();
        tabsNode = AceType::MakeRefPtr<TabsNode>(
            V2::TABS_ETS_TAG, nodeId, AceType::MakeRefPtr<TabsPattern>(), false);
        tabsNode->InitializePatternAndContext();
        ElementRegister::GetInstance()->AddUINode(tabsNode);

        // Set barLayoutStyle on the TabsNode's TabsLayoutProperty
        auto tabsLayoutProperty = tabsNode->GetLayoutProperty<TabsLayoutProperty>();
        ASSERT_NE(tabsLayoutProperty, nullptr);
        tabsLayoutProperty->UpdateBarLayoutStyle(style);

        // 2. Swiper node
        swiperNode = FrameNode::GetOrCreateFrameNode(
            V2::SWIPER_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
            []() { return AceType::MakeRefPtr<SwiperPattern>(); });
        swiperNode->MountToParent(tabsNode);

        // 3. Host node carrying TabsSideBarTabListPattern + columnNode
        tabListPattern = AceType::MakeRefPtr<TabsSideBarTabListPattern>();
        hostNode = FrameNode::CreateFrameNode(
            "SideBarTabList", ElementRegister::GetInstance()->MakeUniqueId(), tabListPattern);
        ASSERT_NE(hostNode, nullptr);
        columnNode = FrameNode::CreateFrameNode(
            V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
            AceType::MakeRefPtr<LinearLayoutPattern>(false));
        ASSERT_NE(columnNode, nullptr);
        columnNode->MountToParent(hostNode);
        tabListPattern->SetColumnNode(columnNode);
        tabListPattern->SetTabsNode(tabsNode);

        // 4. Per-tab: TabContentNode in swiper + matching tab item in columnNode
        for (const auto& cfg : tabConfigs) {
            auto tabContentNode = TabContentNode::GetOrCreateTabContentNode(
                V2::TAB_CONTENT_ITEM_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
                []() -> RefPtr<Pattern> {
                    auto shallowBuilder = AceType::MakeRefPtr<ShallowBuilder>(nullptr);
                    return AceType::MakeRefPtr<TabContentPattern>(shallowBuilder);
                });
            ASSERT_NE(tabContentNode, nullptr);
            auto tabContentPattern = tabContentNode->GetPattern<TabContentPattern>();
            ASSERT_NE(tabContentPattern, nullptr);
            tabContentPattern->UpdateTabBarParamText(cfg.text);
            if (cfg.section.has_value()) {
                tabContentPattern->SetSidebarSection(cfg.section.value());
            }
            tabContentNode->MountToParent(swiperNode);
            tabContentNodes.push_back(tabContentNode);

            // Create a matching tab item node whose FrameNode id == sideBarTabBarItemId
            int32_t sideBarTabBarItemId = tabContentNode->GetSideBarTabBarItemId();
            auto tabItemNode = FrameNode::GetOrCreateFrameNode(
                "TabsSideBarTabItem", sideBarTabBarItemId,
                []() { return AceType::MakeRefPtr<LinearLayoutPattern>(false); });
            ASSERT_NE(tabItemNode, nullptr);
            tabItemNode->MountToParent(columnNode);
            tabItemNodes.push_back(tabItemNode);
        }
    }
};

/**
 * @tc.name: IsTabVisible001
 * @tc.desc: Test IsTabVisible returns false when tab is hidden by defaultVisibility (SIDEBAR mode)
 * @tc.type: FUNC
 */
HWTEST_F(TabsSideBarTabListTestNg, IsTabVisible001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Build a sidebar tree with one tab hidden by defaultVisibility in SIDEBAR mode
     */
    SideBarVisibilityTestTree tree;
    TabContentDefaultVisibility hiddenInSidebar;
    hiddenInSidebar.isNull = false;
    hiddenInSidebar.visibility = TabVisibility::HIDDEN;
    hiddenInSidebar.displayMode = TabBarDisplayMode::SIDEBAR;
    tree.Build(1, { hiddenInSidebar });
    ASSERT_NE(tree.tabListPattern, nullptr);

    /**
     * @tc.steps: step2. Query IsTabVisible for index 0
     * @tc.expected: Returns false because the tab is hidden by defaultVisibility
     */
    EXPECT_FALSE(tree.tabListPattern->IsTabVisible(0));
}

/**
 * @tc.name: IsTabVisible002
 * @tc.desc: Test IsTabVisible returns false when tab is hidden by an active search filter
 * @tc.type: FUNC
 */
HWTEST_F(TabsSideBarTabListTestNg, IsTabVisible002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Build a sidebar section tree with one tab whose text is "Contacts"
     */
    SideBarSectionTestTree tree;
    SideBarSectionTestTree::TabConfig cfg;
    cfg.text = "Contacts";
    tree.Build({ cfg });
    ASSERT_NE(tree.tabListPattern, nullptr);

    /**
     * @tc.steps: step2. Apply a search filter with non-matching text "Settings"
     */
    tree.tabListPattern->ApplySearchFilter(nullptr, u"Settings");

    /**
     * @tc.steps: step3. Query IsTabVisible for index 0
     * @tc.expected: Returns false because the search text does not match the tab text
     */
    EXPECT_FALSE(tree.tabListPattern->IsTabVisible(0));
}

/**
 * @tc.name: IsTabVisible003
 * @tc.desc: Test IsTabVisible returns true when tab is visible (no visibility/search restrictions)
 * @tc.type: FUNC
 */
HWTEST_F(TabsSideBarTabListTestNg, IsTabVisible003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Build a sidebar section tree with one visible tab and no search filter
     */
    SideBarSectionTestTree tree;
    SideBarSectionTestTree::TabConfig cfg;
    cfg.text = "Messages";
    tree.Build({ cfg });
    ASSERT_NE(tree.tabListPattern, nullptr);

    /**
     * @tc.steps: step2. Query IsTabVisible for index 0
     * @tc.expected: Returns true because the tab is not hidden by defaultVisibility or search filter
     */
    EXPECT_TRUE(tree.tabListPattern->IsTabVisible(0));
}

/**
 * @tc.name: IsTabVisible004
 * @tc.desc: Test IsTabVisible returns false for out-of-range tabIndex (negative, beyond child count)
 * @tc.type: FUNC
 */
HWTEST_F(TabsSideBarTabListTestNg, IsTabVisible004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Build a sidebar section tree with one tab
     */
    SideBarSectionTestTree tree;
    SideBarSectionTestTree::TabConfig cfg;
    cfg.text = "Phone";
    tree.Build({ cfg });
    ASSERT_NE(tree.tabListPattern, nullptr);

    /**
     * @tc.steps: step2. Query IsTabVisible with negative and out-of-range indices
     * @tc.expected: Returns false for both negative (-1) and beyond-count (5) indices
     */
    EXPECT_FALSE(tree.tabListPattern->IsTabVisible(-1));
    EXPECT_FALSE(tree.tabListPattern->IsTabVisible(5));
}

/**
 * @tc.name: RebuildSections001
 * @tc.desc: Test RebuildSections is a no-op when barLayoutStyle is not SIDEBAR (default BOTTOM)
 * @tc.type: FUNC
 */
HWTEST_F(TabsSideBarTabListTestNg, RebuildSections001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Build a tree with BOTTOM bar layout style and one sectioned tab
     */
    SideBarSectionTestTree tree;
    SideBarSectionTestTree::TabConfig cfg;
    cfg.text = "Phone";
    OHOS::Ace::SidebarSection section;
    section.title = "Favorites";
    cfg.section = section;
    // Explicitly use BOTTOM style — RebuildSections should early-return
    tree.Build({ cfg }, TabBarLayoutStyle::BOTTOM);
    ASSERT_NE(tree.tabListPattern, nullptr);
    ASSERT_NE(tree.columnNode, nullptr);
    auto childrenBefore = tree.columnNode->GetChildren();
    size_t childCountBefore = childrenBefore.size();

    /**
     * @tc.steps: step2. Call RebuildSections
     * @tc.expected: No section header nodes are created (columnNode unchanged)
     */
    tree.tabListPattern->RebuildSections();
    auto childrenAfter = tree.columnNode->GetChildren();
    size_t childCountAfter = childrenAfter.size();

    // No new "TabsSideBarSectionHeader" nodes should exist
    bool hasSectionHeader = false;
    for (const auto& child : childrenAfter) {
        if (child && child->GetTag() == "TabsSideBarSectionHeader") {
            hasSectionHeader = true;
            break;
        }
    }
    EXPECT_FALSE(hasSectionHeader);
    EXPECT_EQ(childCountAfter, childCountBefore);
}

/**
 * @tc.name: RebuildSections002
 * @tc.desc: Test RebuildSections builds section headers when sections are configured and style is SIDEBAR
 * @tc.type: FUNC
 */
HWTEST_F(TabsSideBarTabListTestNg, RebuildSections002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Build a SIDEBAR tree with two sectioned tabs under the same section title
     */
    SideBarSectionTestTree tree;
    SideBarSectionTestTree::TabConfig cfg1;
    cfg1.text = "Alice";
    OHOS::Ace::SidebarSection section1;
    section1.title = "Favorites";
    section1.titleStyle = SidebarSectionTitleStyle::STANDARD;
    cfg1.section = section1;

    SideBarSectionTestTree::TabConfig cfg2;
    cfg2.text = "Bob";
    OHOS::Ace::SidebarSection section2;
    section2.title = "Favorites";
    section2.titleStyle = SidebarSectionTitleStyle::STANDARD;
    cfg2.section = section2;

    tree.Build({ cfg1, cfg2 });
    ASSERT_NE(tree.tabListPattern, nullptr);
    ASSERT_NE(tree.columnNode, nullptr);

    /**
     * @tc.steps: step2. Call RebuildSections
     */
    tree.tabListPattern->RebuildSections();

    /**
     * @tc.steps: step3. Verify a single "Favorites" section header was created in columnNode
     */
    int32_t sectionHeaderCount = 0;
    RefPtr<FrameNode> foundHeader = nullptr;
    for (const auto& child : tree.columnNode->GetChildren()) {
        if (child && child->GetTag() == "TabsSideBarSectionHeader") {
            ++sectionHeaderCount;
            foundHeader = AceType::DynamicCast<FrameNode>(child);
        }
    }
    EXPECT_EQ(sectionHeaderCount, 1);
    ASSERT_NE(foundHeader, nullptr);

    /**
     * @tc.steps: step4. Verify the header's text child carries the configured section title
     */
    ASSERT_FALSE(foundHeader->GetChildren().empty());
    auto textChild = AceType::DynamicCast<FrameNode>(foundHeader->GetChildren().front());
    ASSERT_NE(textChild, nullptr);
    auto textProp = textChild->GetLayoutProperty<TextLayoutProperty>();
    ASSERT_NE(textProp, nullptr);
    ASSERT_TRUE(textProp->HasContent());
    EXPECT_EQ(StringUtils::Str16ToStr8(textProp->GetContentValue(u"")), std::string("Favorites"));
}

/**
 * @tc.name: RebuildSections003
 * @tc.desc: Test RebuildSections handles empty swiper (no tab content) gracefully
 * @tc.type: FUNC
 */
HWTEST_F(TabsSideBarTabListTestNg, RebuildSections003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Build a SIDEBAR tree with no tab content nodes
     */
    SideBarSectionTestTree tree;
    tree.Build({}, TabBarLayoutStyle::SIDEBAR);
    ASSERT_NE(tree.tabListPattern, nullptr);
    ASSERT_NE(tree.columnNode, nullptr);
    size_t childCountBefore = tree.columnNode->GetChildren().size();

    /**
     * @tc.steps: step2. Call RebuildSections on an empty swiper
     * @tc.expected: No crash and columnNode remains unchanged (no section headers created)
     */
    tree.tabListPattern->RebuildSections();
    size_t childCountAfter = tree.columnNode->GetChildren().size();
    EXPECT_EQ(childCountAfter, childCountBefore);

    bool hasSectionHeader = false;
    for (const auto& child : tree.columnNode->GetChildren()) {
        if (child && child->GetTag() == "TabsSideBarSectionHeader") {
            hasSectionHeader = true;
            break;
        }
    }
    EXPECT_FALSE(hasSectionHeader);
}

/**
 * @tc.name: IsHiddenBySearchFilter001
 * @tc.desc: Test IsHiddenBySearchFilter returns false when no search text is active
 * @tc.type: FUNC
 */
HWTEST_F(TabsSideBarTabListTestNg, IsHiddenBySearchFilter001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Build a sidebar tree without applying any search filter
     */
    SideBarVisibilityTestTree tree;
    TabContentDefaultVisibility vis;
    vis.isNull = true;
    tree.Build(1, { vis });
    ASSERT_NE(tree.tabListPattern, nullptr);

    /**
     * @tc.steps: step2. Query IsHiddenBySearchFilter for index 0
     * @tc.expected: Returns false because no search text is active
     */
    EXPECT_FALSE(tree.tabListPattern->IsHiddenBySearchFilter(0));
}

/**
 * @tc.name: IsHiddenBySearchFilter002
 * @tc.desc: Test IsHiddenBySearchFilter returns true when search text does not match tab text
 * @tc.type: FUNC
 */
HWTEST_F(TabsSideBarTabListTestNg, IsHiddenBySearchFilter002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Build a sidebar section tree with one tab whose text is "Contacts"
     */
    SideBarSectionTestTree tree;
    SideBarSectionTestTree::TabConfig cfg;
    cfg.text = "Contacts";
    tree.Build({ cfg });
    ASSERT_NE(tree.tabListPattern, nullptr);

    /**
     * @tc.steps: step2. Apply a search with non-matching text "Settings" (default fuzzy matcher)
     */
    tree.tabListPattern->ApplySearchFilter(nullptr, u"Settings");

    /**
     * @tc.steps: step3. Query IsHiddenBySearchFilter for index 0
     * @tc.expected: Returns true because "Settings" is not a substring of "Contacts"
     */
    EXPECT_TRUE(tree.tabListPattern->IsHiddenBySearchFilter(0));
}

/**
 * @tc.name: IsHiddenBySearchFilter003
 * @tc.desc: Test IsHiddenBySearchFilter returns false when search text matches tab text
 * @tc.type: FUNC
 */
HWTEST_F(TabsSideBarTabListTestNg, IsHiddenBySearchFilter003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Build a sidebar section tree with one tab whose text is "Contacts"
     */
    SideBarSectionTestTree tree;
    SideBarSectionTestTree::TabConfig cfg;
    cfg.text = "Contacts";
    tree.Build({ cfg });
    ASSERT_NE(tree.tabListPattern, nullptr);

    /**
     * @tc.steps: step2. Apply a search with matching text "tact" (default fuzzy, case-insensitive)
     */
    tree.tabListPattern->ApplySearchFilter(nullptr, u"tact");

    /**
     * @tc.steps: step3. Query IsHiddenBySearchFilter for index 0
     * @tc.expected: Returns false because "tact" is a substring of "Contacts"
     */
    EXPECT_FALSE(tree.tabListPattern->IsHiddenBySearchFilter(0));
}
} // namespace OHOS::Ace::NG

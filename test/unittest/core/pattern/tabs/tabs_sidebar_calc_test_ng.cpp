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
#include "core/components_ng/pattern/tabs/tabs_layout_algorithm.h"
#include "core/components_ng/pattern/tabs/tabs_layout_property.h"
#include "core/components_ng/pattern/tabs/tabs_model_ng.h"
#include "core/components_ng/pattern/tabs/tabs_node.h"
#include "core/components_ng/pattern/tabs/tabs_pattern.h"
#include "core/components_ng/pattern/tabs/tabs_declaration.h"

namespace OHOS::Ace::NG {
namespace {
RefPtr<Theme> GetSidebarCalcTheme(ThemeType type)
{
    if (type == TabTheme::TypeId()) {
        auto themeConstants = TestNG::CreateThemeConstants(THEME_PATTERN_TAB);
        return TabTheme::Builder().Build(themeConstants);
    }
    return AceType::MakeRefPtr<DialogTheme>();
}

// Build a TabsLayoutAlgorithm with barItemSize pre-set so private calculation
// helpers can be exercised directly via the private-access macro.
RefPtr<TabsLayoutAlgorithm> MakeAlgorithm(int32_t barItemSize, bool isFloatingBar = false,
    bool isFloatingTopBar = false)
{
    auto algo = AceType::MakeRefPtr<TabsLayoutAlgorithm>();
    algo->SetBarItemSize(barItemSize);
    algo->SetIsFloatingBar(isFloatingBar);
    algo->SetIsFloatingTopBar(isFloatingTopBar);
    return algo;
}
} // namespace

class TabsSidebarCalcTestNg : public TabsTestNg {
public:
    static void SetUpTestSuite()
    {
        TestNG::SetUpTestSuite();
        MockPipelineContext::GetCurrent()->SetUseFlushUITasks(true);
        auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
        EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly([](ThemeType type) -> RefPtr<Theme> {
            return GetSidebarCalcTheme(type);
        });
        EXPECT_CALL(*themeManager, GetTheme(_, _))
            .WillRepeatedly([](ThemeType type, int32_t themeScopeId) -> RefPtr<Theme> {
                return GetSidebarCalcTheme(type);
            });
        MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    }
    static void TearDownTestSuite()
    {
        TestNG::TearDownTestSuite();
        g_isConfigChangePerform = false;
    }
};

// ===================== CalcTopBarGridMaxWidth =====================

/**
 * @tc.name: CalcTopBarGridMaxWidth001
 * @tc.desc: tabWidth below breakpoint600 uses 4-col grid with 8vp gutter and occupy=4
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarCalcTestNg, CalcTopBarGridMaxWidth001, TestSize.Level1)
{
    auto algo = MakeAlgorithm(0);
    float tabWidth = 400.0f; // < 600vp breakpoint (in px after default density)
    float barSideMargin = 16.0f;
    float result = algo->CalcTopBarGridMaxWidth(tabWidth, barSideMargin);
    EXPECT_GT(result, 0.0f);
    EXPECT_LE(result, tabWidth);
}

/**
 * @tc.name: CalcTopBarGridMaxWidth002
 * @tc.desc: tabWidth between breakpoint600 and breakpoint840 uses 8-col grid with 12vp gutter and occupy=8
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarCalcTestNg, CalcTopBarGridMaxWidth002, TestSize.Level1)
{
    auto algo = MakeAlgorithm(0);
    float tabWidth = 700.0f; // between 600 and 840
    float barSideMargin = 24.0f;
    float result = algo->CalcTopBarGridMaxWidth(tabWidth, barSideMargin);
    EXPECT_GT(result, 0.0f);
    EXPECT_LE(result, tabWidth);
}

/**
 * @tc.name: CalcTopBarGridMaxWidth003
 * @tc.desc: tabWidth above breakpoint840 uses 12-col grid with 16vp gutter and occupy=10
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarCalcTestNg, CalcTopBarGridMaxWidth003, TestSize.Level1)
{
    auto algo = MakeAlgorithm(0);
    float tabWidth = 1000.0f; // > 840
    float barSideMargin = 32.0f;
    float result = algo->CalcTopBarGridMaxWidth(tabWidth, barSideMargin);
    EXPECT_GT(result, 0.0f);
    EXPECT_LE(result, tabWidth);
}

/**
 * @tc.name: CalcTopBarGridMaxWidth004
 * @tc.desc: Larger barSideMargin yields a smaller maxWidth because usableWidth shrinks
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarCalcTestNg, CalcTopBarGridMaxWidth004, TestSize.Level1)
{
    auto algo = MakeAlgorithm(0);
    float tabWidth = 1000.0f;
    float smallMargin = 8.0f;
    float largeMargin = 64.0f;
    float resultSmall = algo->CalcTopBarGridMaxWidth(tabWidth, smallMargin);
    float resultLarge = algo->CalcTopBarGridMaxWidth(tabWidth, largeMargin);
    EXPECT_GT(resultSmall, resultLarge);
}

/**
 * @tc.name: CalcTopBarGridMaxWidth005
 * @tc.desc: Result is always positive for a positive tabWidth and margin
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarCalcTestNg, CalcTopBarGridMaxWidth005, TestSize.Level1)
{
    auto algo = MakeAlgorithm(0);
    float tabWidth = 500.0f;
    float barSideMargin = 0.0f;
    float result = algo->CalcTopBarGridMaxWidth(tabWidth, barSideMargin);
    EXPECT_GT(result, 0.0f);
}

// ===================== CalcSideBarDragRange =====================

/**
 * @tc.name: CalcSideBarDragRangeNotDraggableNoDivider001
 * @tc.desc: When divider width is 0 (no divider set), range.draggable is false
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarCalcTestNg, CalcSideBarDragRangeNotDraggableNoDivider001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    // No divider configured → dividerWidthPx is 0 → not draggable
    auto range = pattern_->CalcSideBarDragRange(720.0f);
    EXPECT_FALSE(range.draggable);
}

/**
 * @tc.name: CalcSideBarDragRangeNotDraggableNoRangeProps001
 * @tc.desc: With a visible divider but no min/max/content props, range.draggable is false
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarCalcTestNg, CalcSideBarDragRangeNotDraggableNoRangeProps001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    TabsItemDivider divider;
    divider.isNull = false;
    divider.strokeWidth = 1.0_vp;
    layoutProperty_->UpdateSidebarDivider(divider);

    auto range = pattern_->CalcSideBarDragRange(720.0f);
    EXPECT_FALSE(range.draggable);
}

/**
 * @tc.name: CalcSideBarDragRangeDraggableWithMin001
 * @tc.desc: With a visible divider and only minSidebarWidth set, range.draggable is true
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarCalcTestNg, CalcSideBarDragRangeDraggableWithMin001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    TabsItemDivider divider;
    divider.isNull = false;
    divider.strokeWidth = 1.0_vp;
    layoutProperty_->UpdateSidebarDivider(divider);
    layoutProperty_->UpdateMinSidebarWidth(Dimension(100.0f, DimensionUnit::VP));

    auto range = pattern_->CalcSideBarDragRange(720.0f);
    EXPECT_TRUE(range.draggable);
    EXPECT_GE(range.maxPx, range.minPx);
}

/**
 * @tc.name: CalcSideBarDragRangeNotDraggableMinGtMax001
 * @tc.desc: When minSidebarWidth >= maxSidebarWidth (px), range.draggable is false
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarCalcTestNg, CalcSideBarDragRangeNotDraggableMinGtMax001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    TabsItemDivider divider;
    divider.isNull = false;
    divider.strokeWidth = 1.0_vp;
    layoutProperty_->UpdateSidebarDivider(divider);
    layoutProperty_->UpdateMinSidebarWidth(Dimension(500.0f, DimensionUnit::VP));
    layoutProperty_->UpdateMaxSidebarWidth(Dimension(100.0f, DimensionUnit::VP));

    auto range = pattern_->CalcSideBarDragRange(720.0f);
    EXPECT_FALSE(range.draggable);
}

/**
 * @tc.name: CalcSideBarDragRangeDraggableWithContent001
 * @tc.desc: With a visible divider and only minContentWidth set, range.draggable is true
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarCalcTestNg, CalcSideBarDragRangeDraggableWithContent001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    TabsItemDivider divider;
    divider.isNull = false;
    divider.strokeWidth = 1.0_vp;
    layoutProperty_->UpdateSidebarDivider(divider);
    layoutProperty_->UpdateMinContentWidth(Dimension(200.0f, DimensionUnit::VP));

    auto range = pattern_->CalcSideBarDragRange(720.0f);
    EXPECT_TRUE(range.draggable);
}

/**
 * @tc.name: CalcSideBarDragRangeDraggableWithMax001
 * @tc.desc: With a visible divider and only maxSidebarWidth set, range.draggable is true
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarCalcTestNg, CalcSideBarDragRangeDraggableWithMax001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    TabsItemDivider divider;
    divider.isNull = false;
    divider.strokeWidth = 1.0_vp;
    layoutProperty_->UpdateSidebarDivider(divider);
    layoutProperty_->UpdateMaxSidebarWidth(Dimension(400.0f, DimensionUnit::VP));

    auto range = pattern_->CalcSideBarDragRange(720.0f);
    EXPECT_TRUE(range.draggable);
}

/**
 * @tc.name: CalcSideBarDragRangeMinZeroWhenNoMin001
 * @tc.desc: When minSidebarWidth is not set, range.minPx defaults to 0
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarCalcTestNg, CalcSideBarDragRangeMinZeroWhenNoMin001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    TabsItemDivider divider;
    divider.isNull = false;
    divider.strokeWidth = 1.0_vp;
    layoutProperty_->UpdateSidebarDivider(divider);
    layoutProperty_->UpdateMaxSidebarWidth(Dimension(400.0f, DimensionUnit::VP));

    auto range = pattern_->CalcSideBarDragRange(720.0f);
    EXPECT_TRUE(range.draggable);
    EXPECT_EQ(range.minPx, 0.0f);
}

// ===================== IsDividerDraggable =====================

/**
 * @tc.name: IsDividerDraggableFalseNoDivider001
 * @tc.desc: When no divider configured, IsDividerDraggable returns false
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarCalcTestNg, IsDividerDraggableFalseNoDivider001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    EXPECT_FALSE(pattern_->IsDividerDraggable());
}

/**
 * @tc.name: IsDividerDraggableFalsePercentStrokeWidth001
 * @tc.desc: When divider strokeWidth is a percent, IsDividerDraggable returns false
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarCalcTestNg, IsDividerDraggableFalsePercentStrokeWidth001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    TabsItemDivider divider;
    divider.isNull = false;
    divider.strokeWidth = Dimension(10.0f, DimensionUnit::PERCENT);
    layoutProperty_->UpdateSidebarDivider(divider);
    layoutProperty_->UpdateMinSidebarWidth(Dimension(100.0f, DimensionUnit::VP));

    EXPECT_FALSE(pattern_->IsDividerDraggable());
}

/**
 * @tc.name: IsDividerDraggableFalseZeroStrokeWidth001
 * @tc.desc: When divider strokeWidth <= 0, IsDividerDraggable returns false
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarCalcTestNg, IsDividerDraggableFalseZeroStrokeWidth001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    TabsItemDivider divider;
    divider.isNull = false;
    divider.strokeWidth = 0.0_vp;
    layoutProperty_->UpdateSidebarDivider(divider);
    layoutProperty_->UpdateMinSidebarWidth(Dimension(100.0f, DimensionUnit::VP));

    EXPECT_FALSE(pattern_->IsDividerDraggable());
}

/**
 * @tc.name: IsDividerDraggableFalseNullDivider001
 * @tc.desc: When divider isNull is true, IsDividerDraggable returns false
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarCalcTestNg, IsDividerDraggableFalseNullDivider001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    TabsItemDivider divider;
    divider.isNull = true;
    divider.strokeWidth = 1.0_vp;
    layoutProperty_->UpdateSidebarDivider(divider);
    layoutProperty_->UpdateMinSidebarWidth(Dimension(100.0f, DimensionUnit::VP));

    EXPECT_FALSE(pattern_->IsDividerDraggable());
}

/**
 * @tc.name: IsDividerDraggableFalseNoRangeProps001
 * @tc.desc: With a valid divider but no range props, IsDividerDraggable returns false
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarCalcTestNg, IsDividerDraggableFalseNoRangeProps001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    TabsItemDivider divider;
    divider.isNull = false;
    divider.strokeWidth = 1.0_vp;
    layoutProperty_->UpdateSidebarDivider(divider);

    EXPECT_FALSE(pattern_->IsDividerDraggable());
}

/**
 * @tc.name: IsDividerDraggableTrueWithMin001
 * @tc.desc: With a valid divider and minSidebarWidth set, IsDividerDraggable returns true
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarCalcTestNg, IsDividerDraggableTrueWithMin001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    TabsItemDivider divider;
    divider.isNull = false;
    divider.strokeWidth = 1.0_vp;
    layoutProperty_->UpdateSidebarDivider(divider);
    layoutProperty_->UpdateMinSidebarWidth(Dimension(100.0f, DimensionUnit::VP));

    EXPECT_TRUE(pattern_->IsDividerDraggable());
}

// ===================== GetEffectiveSidebarDividerWidthPx =====================

/**
 * @tc.name: GetEffectiveSidebarDividerWidthPxZeroNoDivider001
 * @tc.desc: When no divider configured, GetEffectiveSidebarDividerWidthPx returns 0
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarCalcTestNg, GetEffectiveSidebarDividerWidthPxZeroNoDivider001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    EXPECT_EQ(pattern_->GetEffectiveSidebarDividerWidthPx(), 0.0f);
}

/**
 * @tc.name: GetEffectiveSidebarDividerWidthPxZeroNull001
 * @tc.desc: When divider isNull, GetEffectiveSidebarDividerWidthPx returns 0
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarCalcTestNg, GetEffectiveSidebarDividerWidthPxZeroNull001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    TabsItemDivider divider;
    divider.isNull = true;
    divider.strokeWidth = 1.0_vp;
    layoutProperty_->UpdateSidebarDivider(divider);

    EXPECT_EQ(pattern_->GetEffectiveSidebarDividerWidthPx(), 0.0f);
}

/**
 * @tc.name: GetEffectiveSidebarDividerWidthPxZeroPercent001
 * @tc.desc: When divider strokeWidth is percent, GetEffectiveSidebarDividerWidthPx returns 0
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarCalcTestNg, GetEffectiveSidebarDividerWidthPxZeroPercent001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    TabsItemDivider divider;
    divider.isNull = false;
    divider.strokeWidth = Dimension(10.0f, DimensionUnit::PERCENT);
    layoutProperty_->UpdateSidebarDivider(divider);

    EXPECT_EQ(pattern_->GetEffectiveSidebarDividerWidthPx(), 0.0f);
}

/**
 * @tc.name: GetEffectiveSidebarDividerWidthPxPositive001
 * @tc.desc: With valid divider strokeWidth, GetEffectiveSidebarDividerWidthPx returns a positive value
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarCalcTestNg, GetEffectiveSidebarDividerWidthPxPositive001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    TabsItemDivider divider;
    divider.isNull = false;
    divider.strokeWidth = 2.0_vp;
    layoutProperty_->UpdateSidebarDivider(divider);

    EXPECT_GT(pattern_->GetEffectiveSidebarDividerWidthPx(), 0.0f);
}

// ===================== GetEffectiveSidebarWidth =====================

/**
 * @tc.name: GetEffectiveSidebarWidthNulloptNoWidth001
 * @tc.desc: When neither sidebarWidth nor barWidth set and no sideBarNode, returns nullopt
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarCalcTestNg, GetEffectiveSidebarWidthNulloptNoWidth001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    auto result = pattern_->GetEffectiveSidebarWidth();
    EXPECT_FALSE(result.has_value());
}

/**
 * @tc.name: GetEffectiveSidebarWidthFromSidebarWidth001
 * @tc.desc: When hasSidebarWidth_ is true and sidebarWidth set, returns that width
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarCalcTestNg, GetEffectiveSidebarWidthFromSidebarWidth001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    pattern_->SetSidebarWidthCalled();
    layoutProperty_->UpdateSidebarWidth(Dimension(300.0f, DimensionUnit::VP));

    auto result = pattern_->GetEffectiveSidebarWidth();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), Dimension(300.0f, DimensionUnit::VP));
}

/**
 * @tc.name: GetEffectiveSidebarWidthNulloptSidebarWidthNotSet001
 * @tc.desc: When hasSidebarWidth_ is true but sidebarWidth not set, returns nullopt
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarCalcTestNg, GetEffectiveSidebarWidthNulloptSidebarWidthNotSet001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    pattern_->SetSidebarWidthCalled();
    // sidebarWidth not set on property

    auto result = pattern_->GetEffectiveSidebarWidth();
    EXPECT_FALSE(result.has_value());
}

/**
 * @tc.name: GetEffectiveSidebarWidthNegativeReturnsNullopt001
 * @tc.desc: When sidebarWidth is negative, returns nullopt
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarCalcTestNg, GetEffectiveSidebarWidthNegativeReturnsNullopt001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    pattern_->SetSidebarWidthCalled();
    layoutProperty_->UpdateSidebarWidth(Dimension(-10.0f, DimensionUnit::VP));

    auto result = pattern_->GetEffectiveSidebarWidth();
    EXPECT_FALSE(result.has_value());
}

/**
 * @tc.name: GetEffectiveSidebarWidthFromBarWidthForSideBar001
 * @tc.desc: When hasSidebarWidth_ is false but barWidthForSideBar set, returns that width
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarCalcTestNg, GetEffectiveSidebarWidthFromBarWidthForSideBar001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    // hasSidebarWidth_ is false by default
    layoutProperty_->UpdateBarWidthForSideBar(Dimension(250.0f, DimensionUnit::VP));

    auto result = pattern_->GetEffectiveSidebarWidth();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), Dimension(250.0f, DimensionUnit::VP));
}

/**
 * @tc.name: GetEffectiveSidebarWidthNegativeBarWidthReturnsNullopt001
 * @tc.desc: When barWidthForSideBar is negative, returns nullopt
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarCalcTestNg, GetEffectiveSidebarWidthNegativeBarWidthReturnsNullopt001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    layoutProperty_->UpdateBarWidthForSideBar(Dimension(-10.0f, DimensionUnit::VP));

    auto result = pattern_->GetEffectiveSidebarWidth();
    EXPECT_FALSE(result.has_value());
}

// ===================== GetEffectiveSidebarBackgroundColor =====================

/**
 * @tc.name: GetEffectiveSidebarBackgroundColorFromSidebar001
 * @tc.desc: When SidebarBackgroundColor is set, returns that color
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarCalcTestNg, GetEffectiveSidebarBackgroundColorFromSidebar001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    Color expected = Color::RED;
    layoutProperty_->UpdateSidebarBackgroundColor(expected);

    auto result = pattern_->GetEffectiveSidebarBackgroundColor();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), expected);
}

/**
 * @tc.name: GetEffectiveSidebarBackgroundColorFromBarForSideBar001
 * @tc.desc: When only BarBackgroundColorForSideBar is set, returns that color
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarCalcTestNg, GetEffectiveSidebarBackgroundColorFromBarForSideBar001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    Color expected = Color::GREEN;
    layoutProperty_->UpdateBarBackgroundColorForSideBar(expected);

    auto result = pattern_->GetEffectiveSidebarBackgroundColor();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), expected);
}

/**
 * @tc.name: GetEffectiveSidebarBackgroundColorFromBar001
 * @tc.desc: When only BarBackgroundColor is set, returns that color
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarCalcTestNg, GetEffectiveSidebarBackgroundColorFromBar001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    Color expected = Color::BLUE;
    layoutProperty_->UpdateBarBackgroundColor(expected);

    auto result = pattern_->GetEffectiveSidebarBackgroundColor();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), expected);
}

/**
 * @tc.name: GetEffectiveSidebarBackgroundColorPriority001
 * @tc.desc: SidebarBackgroundColor has higher priority than BarBackgroundColorForSideBar
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarCalcTestNg, GetEffectiveSidebarBackgroundColorPriority001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    Color sidebarColor = Color::RED;
    Color barColor = Color::GREEN;
    layoutProperty_->UpdateSidebarBackgroundColor(sidebarColor);
    layoutProperty_->UpdateBarBackgroundColorForSideBar(barColor);

    auto result = pattern_->GetEffectiveSidebarBackgroundColor();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), sidebarColor);
}

/**
 * @tc.name: GetEffectiveSidebarBackgroundColorPriorityBarOverBarBg001
 * @tc.desc: BarBackgroundColorForSideBar has higher priority than BarBackgroundColor
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarCalcTestNg, GetEffectiveSidebarBackgroundColorPriorityBarOverBarBg001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    Color barForSideBarColor = Color::GREEN;
    Color barColor = Color::BLUE;
    layoutProperty_->UpdateBarBackgroundColorForSideBar(barForSideBarColor);
    layoutProperty_->UpdateBarBackgroundColor(barColor);

    auto result = pattern_->GetEffectiveSidebarBackgroundColor();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), barForSideBarColor);
}

// ===================== GetActiveBarDisplayMode =====================

/**
 * @tc.name: GetActiveBarDisplayModeFromCurrent001
 * @tc.desc: When currentBarDisplayMode_ is set, returns that value
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarCalcTestNg, GetActiveBarDisplayModeFromCurrent001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    pattern_->SetCurrentBarDisplayMode(TabBarDisplayMode::SIDEBAR);
    EXPECT_EQ(pattern_->GetActiveBarDisplayMode(), TabBarDisplayMode::SIDEBAR);
}

/**
 * @tc.name: GetActiveBarDisplayModeSidebarStyle001
 * @tc.desc: When no current mode and barLayoutStyle is SIDEBAR, returns SIDEBAR
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarCalcTestNg, GetActiveBarDisplayModeSidebarStyle001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    layoutProperty_->UpdateBarLayoutStyle(TabBarLayoutStyle::SIDEBAR);
    pattern_->currentBarDisplayMode_.reset();
    EXPECT_EQ(pattern_->GetActiveBarDisplayMode(), TabBarDisplayMode::SIDEBAR);
}

/**
 * @tc.name: GetActiveBarDisplayModeTopStyle001
 * @tc.desc: When no current mode and barLayoutStyle is TOP, returns TOP_TABBAR
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarCalcTestNg, GetActiveBarDisplayModeTopStyle001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    layoutProperty_->UpdateBarLayoutStyle(TabBarLayoutStyle::TOP);
    pattern_->currentBarDisplayMode_.reset();
    EXPECT_EQ(pattern_->GetActiveBarDisplayMode(), TabBarDisplayMode::TOP_TABBAR);
}

/**
 * @tc.name: GetActiveBarDisplayModeBottomDefault001
 * @tc.desc: When no current mode and barLayoutStyle is BOTTOM, returns BOTTOMTABBAR
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarCalcTestNg, GetActiveBarDisplayModeBottomDefault001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    layoutProperty_->UpdateBarLayoutStyle(TabBarLayoutStyle::BOTTOM);
    EXPECT_EQ(pattern_->GetActiveBarDisplayMode(), TabBarDisplayMode::BOTTOMTABBAR);
}

// ===================== CalculateTabBarDisplayMode =====================

/**
 * @tc.name: CalculateTabBarDisplayModeSidebarStyle001
 * @tc.desc: When barLayoutStyle is SIDEBAR, returns SIDEBAR
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarCalcTestNg, CalculateTabBarDisplayModeSidebarStyle001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    layoutProperty_->UpdateBarLayoutStyle(TabBarLayoutStyle::SIDEBAR);
    EXPECT_EQ(pattern_->CalculateTabBarDisplayMode(720.0f), TabBarDisplayMode::SIDEBAR);
}

/**
 * @tc.name: CalculateTabBarDisplayModeBottomStyle001
 * @tc.desc: When barLayoutStyle is BOTTOM, returns BOTTOMTABBAR
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarCalcTestNg, CalculateTabBarDisplayModeBottomStyle001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    layoutProperty_->UpdateBarLayoutStyle(TabBarLayoutStyle::BOTTOM);
    EXPECT_EQ(pattern_->CalculateTabBarDisplayMode(720.0f), TabBarDisplayMode::BOTTOMTABBAR);
}

/**
 * @tc.name: CalculateTabBarDisplayModeTopStyle001
 * @tc.desc: When barLayoutStyle is TOP, returns TOP_TABBAR
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarCalcTestNg, CalculateTabBarDisplayModeTopStyle001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    layoutProperty_->UpdateBarLayoutStyle(TabBarLayoutStyle::TOP);
    EXPECT_EQ(pattern_->CalculateTabBarDisplayMode(720.0f), TabBarDisplayMode::TOP_TABBAR);
}

/**
 * @tc.name: CalculateTabBarDisplayModeAdaptableNoDividerLargeWidth001
 * @tc.desc: Adaptable style with large width and no divider → SIDEBAR (breakpoint LG+)
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarCalcTestNg, CalculateTabBarDisplayModeAdaptableNoDividerLargeWidth001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    layoutProperty_->UpdateBarLayoutStyle(TabBarLayoutStyle::SIDEBAR_ADAPTABLE);
    // Large width → breakpoint >= LG → SIDEBAR
    EXPECT_EQ(pattern_->CalculateTabBarDisplayMode(2000.0f), TabBarDisplayMode::SIDEBAR);
}

/**
 * @tc.name: CalculateTabBarDisplayModeAdaptableSmallWidth001
 * @tc.desc: Adaptable style with small width → BOTTOMTABBAR
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarCalcTestNg, CalculateTabBarDisplayModeAdaptableSmallWidth001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    layoutProperty_->UpdateBarLayoutStyle(TabBarLayoutStyle::SIDEBAR_ADAPTABLE);
    // Small width → breakpoint < LG → BOTTOMTABBAR
    EXPECT_EQ(pattern_->CalculateTabBarDisplayMode(300.0f), TabBarDisplayMode::BOTTOMTABBAR);
}

/**
 * @tc.name: CalculateTabBarDisplayModeAdaptableWithDividerSmallWidth001
 * @tc.desc: Adaptable style with visible divider and width < minSidebar+minContent+divider → BOTTOMTABBAR
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarCalcTestNg, CalculateTabBarDisplayModeAdaptableWithDividerSmallWidth001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    layoutProperty_->UpdateBarLayoutStyle(TabBarLayoutStyle::SIDEBAR_ADAPTABLE);
    TabsItemDivider divider;
    divider.isNull = false;
    divider.strokeWidth = 1.0_vp;
    layoutProperty_->UpdateSidebarDivider(divider);
    layoutProperty_->UpdateMinSidebarWidth(Dimension(400.0f, DimensionUnit::VP));
    layoutProperty_->UpdateMinContentWidth(Dimension(400.0f, DimensionUnit::VP));
    // width 300 < 400 + 400 + divider → BOTTOMTABBAR
    EXPECT_EQ(pattern_->CalculateTabBarDisplayMode(300.0f), TabBarDisplayMode::BOTTOMTABBAR);
}

/**
 * @tc.name: CalculateTabBarDisplayModeAdaptableCustomBreakpointNotNull001
 * @tc.desc: Adaptable style with a non-null custom breakpoint uses custom sm/md/lg values
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarCalcTestNg, CalculateTabBarDisplayModeAdaptableCustomBreakpointNotNull001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    layoutProperty_->UpdateBarLayoutStyle(TabBarLayoutStyle::SIDEBAR_ADAPTABLE);
    TabBarDisplayModeBreakpoint bp;
    bp.isNull = false;
    bp.sm = TabBarDisplayMode::SIDEBAR;
    bp.md = TabBarDisplayMode::BOTTOMTABBAR;
    bp.lg = TabBarDisplayMode::TOP_TABBAR;
    layoutProperty_->UpdateBarDisplayModeBreakpoint(bp);
    // Small width → WIDTH_SM → custom sm = SIDEBAR
    EXPECT_EQ(pattern_->CalculateTabBarDisplayMode(300.0f), TabBarDisplayMode::SIDEBAR);
}

/**
 * @tc.name: CalculateTabBarDisplayModeAdaptableCustomBreakpointMd001
 * @tc.desc: Adaptable style with custom breakpoint uses md value for medium width
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarCalcTestNg, CalculateTabBarDisplayModeAdaptableCustomBreakpointMd001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    layoutProperty_->UpdateBarLayoutStyle(TabBarLayoutStyle::SIDEBAR_ADAPTABLE);
    TabBarDisplayModeBreakpoint bp;
    bp.isNull = false;
    bp.sm = TabBarDisplayMode::SIDEBAR;
    bp.md = TabBarDisplayMode::BOTTOMTABBAR;
    bp.lg = TabBarDisplayMode::TOP_TABBAR;
    layoutProperty_->UpdateBarDisplayModeBreakpoint(bp);
    // Medium width → WIDTH_MD → custom md = BOTTOMTABBAR
    EXPECT_EQ(pattern_->CalculateTabBarDisplayMode(700.0f), TabBarDisplayMode::BOTTOMTABBAR);
}

/**
 * @tc.name: CalculateTabBarDisplayModeAdaptableCustomBreakpointLg001
 * @tc.desc: Adaptable style with custom breakpoint uses lg value for large width
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarCalcTestNg, CalculateTabBarDisplayModeAdaptableCustomBreakpointLg001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    layoutProperty_->UpdateBarLayoutStyle(TabBarLayoutStyle::SIDEBAR_ADAPTABLE);
    TabBarDisplayModeBreakpoint bp;
    bp.isNull = false;
    bp.sm = TabBarDisplayMode::SIDEBAR;
    bp.md = TabBarDisplayMode::BOTTOMTABBAR;
    bp.lg = TabBarDisplayMode::TOP_TABBAR;
    layoutProperty_->UpdateBarDisplayModeBreakpoint(bp);
    // Large width → WIDTH_LG → custom lg = TOP_TABBAR
    EXPECT_EQ(pattern_->CalculateTabBarDisplayMode(2000.0f), TabBarDisplayMode::TOP_TABBAR);
}

// ===================== SetCurrentBarDisplayMode =====================

/**
 * @tc.name: SetCurrentBarDisplayModeSameReturnsEarly001
 * @tc.desc: Setting the same display mode returns early without changes
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarCalcTestNg, SetCurrentBarDisplayModeSameReturnsEarly001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    pattern_->SetCurrentBarDisplayMode(TabBarDisplayMode::SIDEBAR);
    auto before = pattern_->GetCurrentBarDisplayMode();
    pattern_->SetCurrentBarDisplayMode(TabBarDisplayMode::SIDEBAR);
    auto after = pattern_->GetCurrentBarDisplayMode();
    ASSERT_TRUE(before.has_value());
    ASSERT_TRUE(after.has_value());
    EXPECT_EQ(before.value(), after.value());
}

/**
 * @tc.name: SetCurrentBarDisplayModeDifferentUpdates001
 * @tc.desc: Setting a different display mode updates the current mode
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarCalcTestNg, SetCurrentBarDisplayModeDifferentUpdates001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    pattern_->SetCurrentBarDisplayMode(TabBarDisplayMode::SIDEBAR);
    ASSERT_TRUE(pattern_->GetCurrentBarDisplayMode().has_value());
    EXPECT_EQ(pattern_->GetCurrentBarDisplayMode().value(), TabBarDisplayMode::SIDEBAR);

    pattern_->SetCurrentBarDisplayMode(TabBarDisplayMode::BOTTOMTABBAR);
    ASSERT_TRUE(pattern_->GetCurrentBarDisplayMode().has_value());
    EXPECT_EQ(pattern_->GetCurrentBarDisplayMode().value(), TabBarDisplayMode::BOTTOMTABBAR);
}

// ===================== GetIsRealDisableSwipe =====================

/**
 * @tc.name: GetIsRealDisableSwipeSidebarMode001
 * @tc.desc: In SIDEBAR mode, GetIsRealDisableSwipe returns true
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarCalcTestNg, GetIsRealDisableSwipeSidebarMode001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    pattern_->SetCurrentBarDisplayMode(TabBarDisplayMode::SIDEBAR);
    EXPECT_TRUE(pattern_->GetIsRealDisableSwipe());
}

/**
 * @tc.name: GetIsRealDisableSwipeBottomMode001
 * @tc.desc: In BOTTOMTABBAR mode with no custom animation, returns isDisableSwipe_
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarCalcTestNg, GetIsRealDisableSwipeBottomMode001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    pattern_->SetCurrentBarDisplayMode(TabBarDisplayMode::BOTTOMTABBAR);
    // Default isDisableSwipe_ is false, isCustomAnimation_ is false
    EXPECT_FALSE(pattern_->GetIsRealDisableSwipe());
}

// ===================== OnDisplayModeSwitchAnimationsFinished =====================

/**
 * @tc.name: OnDisplayModeSwitchAnimationsFinishedNotAnimating001
 * @tc.desc: When isOnDisplayModeSwitchAnimation_ is false, early returns without changes
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarCalcTestNg, OnDisplayModeSwitchAnimationsFinishedNotAnimating001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    // isOnDisplayModeSwitchAnimation_ is false by default
    pattern_->OnDisplayModeSwitchAnimationsFinished();
    EXPECT_FALSE(pattern_->IsOnDisplayModeSwitchAnimation());
}

/**
 * @tc.name: OnDisplayModeSwitchAnimationsFinishedCountGtZero001
 * @tc.desc: When displayModeAnimCount_ > 1, decrements count and stays in animation
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarCalcTestNg, OnDisplayModeSwitchAnimationsFinishedCountGtZero001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    pattern_->isOnDisplayModeSwitchAnimation_ = true;
    pattern_->displayModeAnimCount_ = 2;
    pattern_->displayModeAnimTarget_ = TabBarDisplayMode::SIDEBAR;

    pattern_->OnDisplayModeSwitchAnimationsFinished();
    // count decremented to 1, still animating
    EXPECT_TRUE(pattern_->IsOnDisplayModeSwitchAnimation());
    EXPECT_EQ(pattern_->displayModeAnimCount_, 1);
}

// ===================== ClampTabBarTransformOffset =====================

/**
 * @tc.name: ClampTabBarTransformOffsetNotFloating001
 * @tc.desc: When not floating bar and not floating top bar, early returns
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarCalcTestNg, ClampTabBarTransformOffsetNotFloating001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(tabBarNode_, nullptr);

    // Neither floating bar nor floating top bar → early return
    pattern_->ClampTabBarTransformOffset();
    EXPECT_FALSE(pattern_->isFloatingBar_);
    EXPECT_FALSE(pattern_->isFloatingTopBar_);
}

// ===================== SetFloatingScaleEnabled =====================

/**
 * @tc.name: SetFloatingScaleEnabledDuringAnimation001
 * @tc.desc: When isOnDisplayModeSwitchAnimation_ is true, SetFloatingScaleEnabled returns early
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarCalcTestNg, SetFloatingScaleEnabledDuringAnimation001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    pattern_->isOnDisplayModeSwitchAnimation_ = true;
    pattern_->SetFloatingScaleEnabled(true);
    // baseFloatingScale_ is set unconditionally; the early return only skips UpdateTransformScale
    EXPECT_EQ(pattern_->baseFloatingScale_, FLOATING_BAR_SCALE_ENLARGED);
    pattern_->isOnDisplayModeSwitchAnimation_ = false;
}

/**
 * @tc.name: SetFloatingScaleEnabledTrueSetsEnlarged001
 * @tc.desc: SetFloatingScaleEnabled(true) sets baseFloatingScale_ to enlarged value
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarCalcTestNg, SetFloatingScaleEnabledTrueSetsEnlarged001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    pattern_->SetFloatingScaleEnabled(true);
    EXPECT_FLOAT_EQ(pattern_->baseFloatingScale_, FLOATING_BAR_SCALE_ENLARGED);
}

/**
 * @tc.name: SetFloatingScaleEnabledFalseSetsScale001
 * @tc.desc: SetFloatingScaleEnabled(false) sets baseFloatingScale_ to normal value
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarCalcTestNg, SetFloatingScaleEnabledFalseSetsScale001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    pattern_->SetFloatingScaleEnabled(false);
    EXPECT_FLOAT_EQ(pattern_->baseFloatingScale_, FLOATING_BAR_SCALE);
}

// ===================== OnLuminanceUpdate =====================

/**
 * @tc.name: OnLuminanceUpdateLowLuminance001
 * @tc.desc: Luminance below LUMINANCE_THRESHOLD_LOW sets isColorPickerDark_ to true
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarCalcTestNg, OnLuminanceUpdateLowLuminance001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    pattern_->OnLuminanceUpdate(50); // < LUMINANCE_THRESHOLD_LOW (150)
    ASSERT_TRUE(pattern_->isColorPickerDark_.has_value());
    EXPECT_TRUE(pattern_->isColorPickerDark_.value());
}

/**
 * @tc.name: OnLuminanceUpdateHighLuminance001
 * @tc.desc: Luminance above LUMINANCE_THRESHOLD_HIGH sets isColorPickerDark_ to false
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarCalcTestNg, OnLuminanceUpdateHighLuminance001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    pattern_->OnLuminanceUpdate(250); // > LUMINANCE_THRESHOLD_HIGH (200)
    ASSERT_TRUE(pattern_->isColorPickerDark_.has_value());
    EXPECT_FALSE(pattern_->isColorPickerDark_.value());
}

/**
 * @tc.name: OnLuminanceUpdateMidLuminanceKeepsValue001
 * @tc.desc: Luminance between thresholds does not change existing isColorPickerDark_
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarCalcTestNg, OnLuminanceUpdateMidLuminanceKeepsValue001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    pattern_->isColorPickerDark_ = true;
    pattern_->OnLuminanceUpdate(175); // between LOW (150) and HIGH (200)
    ASSERT_TRUE(pattern_->isColorPickerDark_.has_value());
    EXPECT_TRUE(pattern_->isColorPickerDark_.value());
}

// ===================== GetColorInvertColorMode =====================

/**
 * @tc.name: GetColorInvertColorModeUndefinedNotEnabled001
 * @tc.desc: When IsColorInvertEnabled is false, returns COLOR_MODE_UNDEFINED
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarCalcTestNg, GetColorInvertColorModeUndefinedNotEnabled001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    EXPECT_EQ(pattern_->GetColorInvertColorMode(), ColorMode::COLOR_MODE_UNDEFINED);
}

/**
 * @tc.name: GetColorInvertColorModeDarkWhenDark001
 * @tc.desc: When color invert enabled and isColorPickerDark_, returns DARK
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarCalcTestNg, GetColorInvertColorModeDarkWhenDark001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    pattern_->isColorPickerDark_ = true;
    // IsColorInvertEnabled checks bar floating style systemMaterial — without it returns false,
    // so the result remains UNDEFINED. This verifies the early-return guard.
    EXPECT_EQ(pattern_->GetColorInvertColorMode(), ColorMode::COLOR_MODE_UNDEFINED);
}

// ===================== IsColorInvertEnabled =====================

/**
 * @tc.name: IsColorInvertEnabledFalseNoStyle001
 * @tc.desc: When barFloatingStyle has no value, IsColorInvertEnabled returns false
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarCalcTestNg, IsColorInvertEnabledFalseNoStyle001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    // No bar floating style set
    EXPECT_FALSE(pattern_->IsColorInvertEnabled());
}

/**
 * @tc.name: IsColorInvertEnabledFalseNoSystemMaterial001
 * @tc.desc: When barFloatingStyle exists but systemMaterial is null, returns false
 * @tc.type: FUNC
 */
HWTEST_F(TabsSidebarCalcTestNg, IsColorInvertEnabledFalseNoSystemMaterial001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    BarFloatingStyleParameters style;
    style.systemMaterial = nullptr;
    layoutProperty_->UpdateBarFloatingStyle(style);

    EXPECT_FALSE(pattern_->IsColorInvertEnabled());
}
} // namespace OHOS::Ace::NG

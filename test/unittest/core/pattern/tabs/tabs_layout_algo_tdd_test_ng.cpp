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
#include "core/components_ng/layout/layout_wrapper_node.h"
#include "core/components_ng/pattern/tabs/tabs_layout_property.h"
#include "core/components_ng/pattern/tabs/tabs_model.h"
#include "core/components_ng/pattern/tabs/tabs_model_ng.h"
#include "core/components_ng/pattern/tabs/tabs_node.h"
#include "core/components_ng/pattern/tabs/tabs_pattern.h"
#include "core/components_ng/pattern/tabs/tabs_declaration.h"

namespace OHOS::Ace::NG {
namespace {
RefPtr<Theme> GetLayoutAlgoTddTheme(ThemeType type)
{
    if (type == TabTheme::TypeId()) {
        auto themeConstants = TestNG::CreateThemeConstants(THEME_PATTERN_TAB);
        return TabTheme::Builder().Build(themeConstants);
    }
    return AceType::MakeRefPtr<DialogTheme>();
}

// Build a LayoutWrapperNode backed by a TabsNode with the given frame size and
// a TabsLayoutProperty, so layout-algorithm private helpers that take a
// LayoutWrapper* can be exercised directly.
struct LayoutAlgoTestCtx {
    RefPtr<TabsLayoutAlgorithm> algo;
    RefPtr<TabsNode> tabsNode;
    RefPtr<GeometryNode> geometryNode;
    RefPtr<TabsLayoutProperty> layoutProperty;
    std::unique_ptr<LayoutWrapperNode> layoutWrapper;
    RefPtr<FrameNode> swiperNode;
    RefPtr<FrameNode> tabBarNode;
    RefPtr<FrameNode> dividerNode;

    void Build(float width, float height)
    {
        Build(0, width, height, false);
    }

    void Build(int32_t barItemSize, float width, float height, bool isFloatingBar = false)
    {
        algo = AceType::MakeRefPtr<TabsLayoutAlgorithm>();
        algo->SetBarItemSize(barItemSize);
        algo->SetIsFloatingBar(isFloatingBar);
        tabsNode = AceType::MakeRefPtr<TabsNode>(
            V2::TABS_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
            AceType::MakeRefPtr<TabsPattern>(), true);
        ASSERT_NE(tabsNode, nullptr);
        geometryNode = AceType::MakeRefPtr<GeometryNode>();
        ASSERT_NE(geometryNode, nullptr);
        geometryNode->SetFrameSize(SizeF(width, height));
        layoutProperty = AceType::MakeRefPtr<TabsLayoutProperty>();
        ASSERT_NE(layoutProperty, nullptr);
        LayoutConstraintF constraint;
        constraint.selfIdealSize.SetWidth(width);
        constraint.selfIdealSize.SetHeight(height);
        constraint.maxSize.SetWidth(width);
        constraint.maxSize.SetHeight(height);
        layoutProperty->UpdateLayoutConstraint(constraint);
        layoutProperty->UpdateContentConstraint();
        layoutWrapper = std::make_unique<LayoutWrapperNode>(tabsNode, geometryNode, layoutProperty);
        layoutWrapper->SetLayoutAlgorithm(AceType::MakeRefPtr<LayoutAlgorithmWrapper>(algo));
    }
};
} // namespace

class TabsLayoutAlgoTddTestNg : public TabsTestNg {
public:
    static void SetUpTestSuite()
    {
        TestNG::SetUpTestSuite();
        MockPipelineContext::GetCurrent()->SetUseFlushUITasks(true);
        auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
        EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly([](ThemeType type) -> RefPtr<Theme> {
            return GetLayoutAlgoTddTheme(type);
        });
        EXPECT_CALL(*themeManager, GetTheme(_, _))
            .WillRepeatedly([](ThemeType type, int32_t themeScopeId) -> RefPtr<Theme> {
                return GetLayoutAlgoTddTheme(type);
            });
        MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    }
    static void TearDownTestSuite()
    {
        TestNG::TearDownTestSuite();
        g_isConfigChangePerform = false;
    }
};

// ===================== GetBarPosition =====================

/**
 * @tc.name: GetBarPositionDefault001
 * @tc.desc: GetBarPosition returns START by default
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, GetBarPositionDefault001, TestSize.Level1)
{
    LayoutAlgoTestCtx ctx;
    ctx.Build(720.0f, 400.0f);
    EXPECT_EQ(ctx.algo->GetBarPosition(ctx.layoutWrapper.get()), BarPosition::START);
}

/**
 * @tc.name: GetBarPositionEnd001
 * @tc.desc: GetBarPosition returns END when set
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, GetBarPositionEnd001, TestSize.Level1)
{
    LayoutAlgoTestCtx ctx;
    ctx.Build(720.0f, 400.0f);
    ctx.layoutProperty->UpdateTabBarPosition(BarPosition::END);
    EXPECT_EQ(ctx.algo->GetBarPosition(ctx.layoutWrapper.get()), BarPosition::END);
}

// ===================== GetAxis =====================

/**
 * @tc.name: GetAxisDefault001
 * @tc.desc: GetAxis returns HORIZONTAL by default
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, GetAxisDefault001, TestSize.Level1)
{
    LayoutAlgoTestCtx ctx;
    ctx.Build(720.0f, 400.0f);
    EXPECT_EQ(ctx.algo->GetAxis(ctx.layoutWrapper.get()), Axis::HORIZONTAL);
}

/**
 * @tc.name: GetAxisVertical001
 * @tc.desc: GetAxis returns VERTICAL when set with BarPosition=END
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, GetAxisVertical001, TestSize.Level1)
{
    LayoutAlgoTestCtx ctx;
    ctx.Build(720.0f, 400.0f);
    ctx.layoutProperty->UpdateAxis(Axis::VERTICAL);
    EXPECT_EQ(ctx.algo->GetAxis(ctx.layoutWrapper.get()), Axis::VERTICAL);
}

// ===================== GetDivider =====================

/**
 * @tc.name: GetDividerDefault001
 * @tc.desc: GetDivider returns default (isNull=true) when no divider set
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, GetDividerDefault001, TestSize.Level1)
{
    LayoutAlgoTestCtx ctx;
    ctx.Build(720.0f, 400.0f);
    auto divider = ctx.algo->GetDivider(ctx.layoutWrapper.get());
    // TabsItemDivider default-constructs with isNull=false
    EXPECT_FALSE(divider.isNull);
}

/**
 * @tc.name: GetDividerSet001
 * @tc.desc: GetDivider returns the set divider
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, GetDividerSet001, TestSize.Level1)
{
    LayoutAlgoTestCtx ctx;
    ctx.Build(720.0f, 400.0f);
    TabsItemDivider divider;
    divider.isNull = false;
    divider.strokeWidth = 5.0_vp;
    ctx.layoutProperty->UpdateDivider(divider);
    auto result = ctx.algo->GetDivider(ctx.layoutWrapper.get());
    EXPECT_FALSE(result.isNull);
    EXPECT_EQ(result.strokeWidth, 5.0_vp);
}

// ===================== CheckIfNeedModeChangeAnimation =====================

/**
 * @tc.name: CheckIfNeedModeChangeAnimationFirstLayout001
 * @tc.desc: On first layout, returns false regardless of other conditions
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, CheckIfNeedModeChangeAnimationFirstLayout001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    auto algo = AceType::MakeRefPtr<TabsLayoutAlgorithm>();
    EXPECT_FALSE(algo->CheckIfNeedModeChangeAnimation(layoutProperty_, pattern_, true, true));
    EXPECT_FALSE(algo->CheckIfNeedModeChangeAnimation(layoutProperty_, pattern_, false, true));
}

/**
 * @tc.name: CheckIfNeedModeChangeAnimationNoChange001
 * @tc.desc: When displayModeChanged is false, returns false
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, CheckIfNeedModeChangeAnimationNoChange001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    auto algo = AceType::MakeRefPtr<TabsLayoutAlgorithm>();
    EXPECT_FALSE(algo->CheckIfNeedModeChangeAnimation(layoutProperty_, pattern_, false, false));
}

/**
 * @tc.name: CheckIfNeedModeChangeAnimationNullPattern001
 * @tc.desc: With null tabsPattern, returns false
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, CheckIfNeedModeChangeAnimationNullPattern001, TestSize.Level1)
{
    LayoutAlgoTestCtx ctx;
    ctx.Build(720.0f, 400.0f);
    EXPECT_FALSE(ctx.algo->CheckIfNeedModeChangeAnimation(ctx.layoutProperty, nullptr, true, false));
}

/**
 * @tc.name: CheckIfNeedModeChangeAnimationNullProperty001
 * @tc.desc: With null layoutProperty, returns false
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, CheckIfNeedModeChangeAnimationNullProperty001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    auto algo = AceType::MakeRefPtr<TabsLayoutAlgorithm>();
    EXPECT_FALSE(algo->CheckIfNeedModeChangeAnimation(nullptr, pattern_, true, false));
}

/**
 * @tc.name: CheckIfNeedModeChangeAnimationNotFloatingNotEnd001
 * @tc.desc: With displayModeChanged=true, firstLayout=false, but not floating and barPosition=START, returns false
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, CheckIfNeedModeChangeAnimationNotFloatingNotEnd001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    auto algo = AceType::MakeRefPtr<TabsLayoutAlgorithm>();
    // Default BarPosition::START, not floating
    EXPECT_FALSE(algo->CheckIfNeedModeChangeAnimation(layoutProperty_, pattern_, true, false));
}

/**
 * @tc.name: CheckIfNeedModeChangeAnimationBarEnd001
 * @tc.desc: With barPosition=END (horizontal+end), isAnimatableTabBar is true, returns true
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, CheckIfNeedModeChangeAnimationBarEnd001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs(BarPosition::END);
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    auto algo = AceType::MakeRefPtr<TabsLayoutAlgorithm>();
    // BarPosition::END → isHorizontal && isBarPositionEnd → isAnimatableTabBar=true
    EXPECT_TRUE(algo->CheckIfNeedModeChangeAnimation(layoutProperty_, pattern_, true, false));
}

// ===================== MeasureDivider =====================

/**
 * @tc.name: MeasureDividerHorizontalNull001
 * @tc.desc: MeasureDivider with null divider returns 0
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, MeasureDividerHorizontalNull001, TestSize.Level1)
{
    LayoutAlgoTestCtx ctx;
    ctx.Build(720.0f, 400.0f);
    auto dividerNode = FrameNode::CreateFrameNode(
        V2::DIVIDER_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    auto dividerGeometry = AceType::MakeRefPtr<GeometryNode>();
    auto dividerLayoutProp = AceType::MakeRefPtr<LayoutProperty>();
    auto dividerWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(dividerNode, dividerGeometry, dividerLayoutProp);
    dividerWrapper->SetLayoutAlgorithm(AceType::MakeRefPtr<LayoutAlgorithmWrapper>(nullptr));
    // No divider set → divider.isNull defaults to false, strokeWidth=0 → returns 0
    float result = ctx.algo->MeasureDivider(ctx.layoutProperty, dividerWrapper, SizeF(720.0f, 400.0f));
    EXPECT_FLOAT_EQ(result, 0.0f);
}

/**
 * @tc.name: MeasureDividerHorizontalSet001
 * @tc.desc: MeasureDivider with valid divider returns strokeWidth in px
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, MeasureDividerHorizontalSet001, TestSize.Level1)
{
    LayoutAlgoTestCtx ctx;
    ctx.Build(720.0f, 400.0f);
    TabsItemDivider divider;
    divider.isNull = false;
    divider.strokeWidth = 10.0_vp;
    ctx.layoutProperty->UpdateDivider(divider);
    auto dividerNode = FrameNode::CreateFrameNode(
        V2::DIVIDER_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    auto dividerGeometry = AceType::MakeRefPtr<GeometryNode>();
    auto dividerLayoutProp = AceType::MakeRefPtr<LayoutProperty>();
    auto dividerWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(dividerNode, dividerGeometry, dividerLayoutProp);
    dividerWrapper->SetLayoutAlgorithm(AceType::MakeRefPtr<LayoutAlgorithmWrapper>(nullptr));
    float result = ctx.algo->MeasureDivider(ctx.layoutProperty, dividerWrapper, SizeF(720.0f, 400.0f));
    // dipScale=1.0 → 10vp = 10px
    EXPECT_FLOAT_EQ(result, 10.0f);
}

// ===================== MeasureEffectNode =====================

/**
 * @tc.name: MeasureEffectNodeHorizontal001
 * @tc.desc: MeasureEffectNode with horizontal axis sets width=parentWidth, height=strokeWidth
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, MeasureEffectNodeHorizontal001, TestSize.Level1)
{
    LayoutAlgoTestCtx ctx;
    ctx.Build(720.0f, 400.0f);
    TabsEffectNodeOption effectOption;
    effectOption.strokeWidth = 8.0_vp;
    ctx.layoutProperty->UpdateEffectNodeOption(effectOption);
    auto effectNode = FrameNode::CreateFrameNode(
        "EffectNode", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    auto effectGeometry = AceType::MakeRefPtr<GeometryNode>();
    auto effectLayoutProp = AceType::MakeRefPtr<LayoutProperty>();
    auto effectWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(effectNode, effectGeometry, effectLayoutProp);
    effectWrapper->SetLayoutAlgorithm(AceType::MakeRefPtr<LayoutAlgorithmWrapper>(nullptr));
    // Should not crash; horizontal axis → width=720, height=8
    ctx.algo->MeasureEffectNode(ctx.layoutProperty, effectWrapper, SizeF(720.0f, 400.0f));
    EXPECT_NE(effectWrapper->GetGeometryNode(), nullptr);
}

// ===================== UpdateBgMaskNodeVisibility =====================

/**
 * @tc.name: UpdateBgMaskNodeVisibilityNoMask001
 * @tc.desc: UpdateBgMaskNodeVisibility with no bg mask node is a no-op
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, UpdateBgMaskNodeVisibilityNoMask001, TestSize.Level1)
{
    LayoutAlgoTestCtx ctx;
    ctx.Build(720.0f, 400.0f);
    // No bg mask node → early return
    ctx.algo->UpdateBgMaskNodeVisibility(ctx.layoutWrapper.get(), true);
    EXPECT_NE(ctx.algo, nullptr);
}

// ===================== UpdateTabBarAndDividerVisibility =====================

/**
 * @tc.name: UpdateTabBarAndDividerVisibilityBasic001
 * @tc.desc: UpdateTabBarAndDividerVisibility sets tabBar visibility
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, UpdateTabBarAndDividerVisibilityBasic001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(frameNode_, nullptr);
    ASSERT_NE(tabBarNode_, nullptr);

    auto algo = AceType::MakeRefPtr<TabsLayoutAlgorithm>();
    auto geo = AceType::MakeRefPtr<GeometryNode>();
    auto lp = frameNode_->GetLayoutProperty<TabsLayoutProperty>();
    auto wrapper = std::make_unique<LayoutWrapperNode>(frameNode_, geo, lp);
    wrapper->SetLayoutAlgorithm(AceType::MakeRefPtr<LayoutAlgorithmWrapper>(algo));
    // Set up children wrappers so GetOrCreateChildByIndex works
    auto tabBarGeo = AceType::MakeRefPtr<GeometryNode>();
    auto tabBarWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(
        tabBarNode_, tabBarGeo, tabBarNode_->GetLayoutProperty());
    wrapper->AppendChild(tabBarWrapper);

    tabBarNode_->GetLayoutProperty()->UpdateVisibility(VisibleType::VISIBLE);
    algo->UpdateTabBarAndDividerVisibility(wrapper.get(), false);
    EXPECT_EQ(tabBarNode_->GetLayoutProperty()->GetVisibilityValue(VisibleType::VISIBLE), VisibleType::GONE);
}

// ===================== LayoutBackgroundMask (measure-file) =====================

/**
 * @tc.name: MeasureLayoutBackgroundMaskNotFloating001
 * @tc.desc: LayoutBackgroundMask with not floating bar/topbar is a no-op
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, MeasureLayoutBackgroundMaskNotFloating001, TestSize.Level1)
{
    LayoutAlgoTestCtx ctx;
    ctx.Build(720.0f, 400.0f);
    ctx.algo->SetIsFloatingBar(false);
    // isFloatingTopBar_ defaults to false
    ctx.algo->LayoutBackgroundMask(ctx.layoutWrapper.get());
    EXPECT_NE(ctx.algo, nullptr);
}

// ===================== CalcTopBarGridMaxWidth (measure-file) =====================

/**
 * @tc.name: MeasureCalcTopBarGridMaxWidthBoundary600001
 * @tc.desc: At exactly 600vp width, uses 8-col grid (medium)
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, MeasureCalcTopBarGridMaxWidthBoundary600001, TestSize.Level1)
{
    auto algo = AceType::MakeRefPtr<TabsLayoutAlgorithm>();
    // 600vp → 8-col grid
    float result = algo->CalcTopBarGridMaxWidth(600.0f, 0.0f);
    // cols=8, gutter=12, margin=0 → usableWidth=600, columnWidth=(600-7*12)/8=64.5
    // maxWidth = 64.5*8 + 7*12 = 516 + 84 = 600
    EXPECT_FLOAT_EQ(result, 600.0f);
}

/**
 * @tc.name: MeasureCalcTopBarGridMaxWidthBoundary840001
 * @tc.desc: At exactly 840vp width, uses 12-col grid (large)
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, MeasureCalcTopBarGridMaxWidthBoundary840001, TestSize.Level1)
{
    auto algo = AceType::MakeRefPtr<TabsLayoutAlgorithm>();
    // 840vp → 12-col grid
    float result = algo->CalcTopBarGridMaxWidth(840.0f, 0.0f);
    // cols=12, gutter=16, margin=0 → usableWidth=840, columnWidth=(840-11*16)/12=55.333...
    // occupy=10, maxWidth = 55.333*10 + 9*16 = 553.333 + 144 = 697.333
    EXPECT_NEAR(result, 697.333f, 0.01f);
}

/**
 * @tc.name: MeasureCalcTopBarGridMaxWidthBelow600001
 * @tc.desc: Below 600vp uses 4-col grid (small)
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, MeasureCalcTopBarGridMaxWidthBelow600001, TestSize.Level1)
{
    auto algo = AceType::MakeRefPtr<TabsLayoutAlgorithm>();
    // 599vp → 4-col grid
    float result = algo->CalcTopBarGridMaxWidth(599.0f, 0.0f);
    // cols=4, gutter=8, margin=0 → usableWidth=599, columnWidth=(599-3*8)/4=143.75
    // occupy=4, maxWidth = 143.75*4 + 3*8 = 575 + 24 = 599
    EXPECT_FLOAT_EQ(result, 599.0f);
}

// ===================== MeasureBackgroundMask (layout-file) =====================

/**
 * @tc.name: MeasureBackgroundMaskNotFloating001
 * @tc.desc: MeasureBackgroundMask with not floating bar/topbar is a no-op
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, MeasureBackgroundMaskNotFloating001, TestSize.Level1)
{
    LayoutAlgoTestCtx ctx;
    ctx.Build(720.0f, 400.0f);
    ctx.algo->SetIsFloatingBar(false);
    ctx.algo->SetIsFloatingTopBar(false);
    ctx.algo->MeasureBackgroundMask(ctx.layoutWrapper.get(), SizeF(720.0f, 400.0f), SizeF(720.0f, 56.0f));
    EXPECT_NE(ctx.algo, nullptr);
}

/**
 * @tc.name: MeasureBackgroundMaskFloatingNoBgMask001
 * @tc.desc: MeasureBackgroundMask with floating bar but no bg mask index is a no-op
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, MeasureBackgroundMaskFloatingNoBgMask001, TestSize.Level1)
{
    LayoutAlgoTestCtx ctx;
    ctx.Build(720.0f, 400.0f);
    ctx.algo->SetIsFloatingBar(true);
    // bgMaskIndex defaults to -1 → early return
    ctx.algo->MeasureBackgroundMask(ctx.layoutWrapper.get(), SizeF(720.0f, 400.0f), SizeF(720.0f, 56.0f));
    EXPECT_NE(ctx.algo, nullptr);
}

// ===================== MeasureSwiper =====================

/**
 * @tc.name: MeasureSwiperHorizontalNoOverlap001
 * @tc.desc: MeasureSwiper with horizontal axis and no bar overlap sets selfIdealSize height = parent - tabBar
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, MeasureSwiperHorizontalNoOverlap001, TestSize.Level1)
{
    LayoutAlgoTestCtx ctx;
    ctx.Build(720.0f, 400.0f);
    auto swiperGeo = AceType::MakeRefPtr<GeometryNode>();
    auto swiperLayoutProp = AceType::MakeRefPtr<LayoutProperty>();
    auto swiperNode = FrameNode::CreateFrameNode(
        V2::SWIPER_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    auto swiperWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(swiperNode, swiperGeo, swiperLayoutProp);
    swiperWrapper->SetLayoutAlgorithm(AceType::MakeRefPtr<LayoutAlgorithmWrapper>(nullptr));
    ctx.layoutWrapper->AppendChild(swiperWrapper);
    // Default: horizontal axis, no bar overlap
    RefPtr<LayoutWrapper> swiperWrapperBase = swiperWrapper;
    ctx.algo->MeasureSwiper(ctx.layoutProperty, swiperWrapperBase,
        SizeF(720.0f, 400.0f), SizeF(720.0f, 56.0f), 0.0f);
    // idealHeight = 400 - 56 - 0 = 344; idealWidth = 720
    auto lc = swiperLayoutProp->GetLayoutConstraint();
    ASSERT_TRUE(lc.has_value());
    EXPECT_FLOAT_EQ(lc->selfIdealSize.Height().value_or(0.0f), 344.0f);
    EXPECT_FLOAT_EQ(lc->selfIdealSize.Width().value_or(0.0f), 720.0f);
}

/**
 * @tc.name: MeasureSwiperHorizontalWithOverlap001
 * @tc.desc: MeasureSwiper with horizontal axis and bar overlap=true sets selfIdealSize height = parent
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, MeasureSwiperHorizontalWithOverlap001, TestSize.Level1)
{
    LayoutAlgoTestCtx ctx;
    ctx.Build(720.0f, 400.0f);
    ctx.layoutProperty->UpdateBarOverlap(true);
    auto swiperGeo = AceType::MakeRefPtr<GeometryNode>();
    auto swiperLayoutProp = AceType::MakeRefPtr<LayoutProperty>();
    auto swiperNode = FrameNode::CreateFrameNode(
        V2::SWIPER_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    auto swiperWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(swiperNode, swiperGeo, swiperLayoutProp);
    swiperWrapper->SetLayoutAlgorithm(AceType::MakeRefPtr<LayoutAlgorithmWrapper>(nullptr));
    ctx.layoutWrapper->AppendChild(swiperWrapper);
    RefPtr<LayoutWrapper> swiperWrapperBase = swiperWrapper;
    ctx.algo->MeasureSwiper(ctx.layoutProperty, swiperWrapperBase,
        SizeF(720.0f, 400.0f), SizeF(720.0f, 56.0f), 0.0f);
    // overlap → idealHeight = 400 (full parent height)
    auto lc = swiperLayoutProp->GetLayoutConstraint();
    ASSERT_TRUE(lc.has_value());
    EXPECT_FLOAT_EQ(lc->selfIdealSize.Height().value_or(0.0f), 400.0f);
    EXPECT_FLOAT_EQ(lc->selfIdealSize.Width().value_or(0.0f), 720.0f);
}

/**
 * @tc.name: MeasureSwiperVerticalNoOverlap001
 * @tc.desc: MeasureSwiper with vertical axis and no bar overlap sets selfIdealSize width = parent - tabBar
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, MeasureSwiperVerticalNoOverlap001, TestSize.Level1)
{
    LayoutAlgoTestCtx ctx;
    ctx.Build(720.0f, 400.0f);
    ctx.layoutProperty->UpdateAxis(Axis::VERTICAL);
    auto swiperGeo = AceType::MakeRefPtr<GeometryNode>();
    auto swiperLayoutProp = AceType::MakeRefPtr<LayoutProperty>();
    auto swiperNode = FrameNode::CreateFrameNode(
        V2::SWIPER_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    auto swiperWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(swiperNode, swiperGeo, swiperLayoutProp);
    swiperWrapper->SetLayoutAlgorithm(AceType::MakeRefPtr<LayoutAlgorithmWrapper>(nullptr));
    ctx.layoutWrapper->AppendChild(swiperWrapper);
    RefPtr<LayoutWrapper> swiperWrapperBase = swiperWrapper;
    ctx.algo->MeasureSwiper(ctx.layoutProperty, swiperWrapperBase,
        SizeF(720.0f, 400.0f), SizeF(50.0f, 400.0f), 0.0f);
    // vertical, no overlap → idealWidth = 720 - 50 - 0 = 670
    auto lc = swiperLayoutProp->GetLayoutConstraint();
    ASSERT_TRUE(lc.has_value());
    EXPECT_FLOAT_EQ(lc->selfIdealSize.Width().value_or(0.0f), 670.0f);
    EXPECT_FLOAT_EQ(lc->selfIdealSize.Height().value_or(0.0f), 400.0f);
}

/**
 * @tc.name: MeasureSwiperVerticalWithOverlap001
 * @tc.desc: MeasureSwiper with vertical axis and bar overlap=true sets selfIdealSize width = parent
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, MeasureSwiperVerticalWithOverlap001, TestSize.Level1)
{
    LayoutAlgoTestCtx ctx;
    ctx.Build(720.0f, 400.0f);
    ctx.layoutProperty->UpdateAxis(Axis::VERTICAL);
    ctx.layoutProperty->UpdateBarOverlap(true);
    auto swiperGeo = AceType::MakeRefPtr<GeometryNode>();
    auto swiperLayoutProp = AceType::MakeRefPtr<LayoutProperty>();
    auto swiperNode = FrameNode::CreateFrameNode(
        V2::SWIPER_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    auto swiperWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(swiperNode, swiperGeo, swiperLayoutProp);
    swiperWrapper->SetLayoutAlgorithm(AceType::MakeRefPtr<LayoutAlgorithmWrapper>(nullptr));
    ctx.layoutWrapper->AppendChild(swiperWrapper);
    RefPtr<LayoutWrapper> swiperWrapperBase = swiperWrapper;
    ctx.algo->MeasureSwiper(ctx.layoutProperty, swiperWrapperBase,
        SizeF(720.0f, 400.0f), SizeF(50.0f, 400.0f), 0.0f);
    // vertical, overlap → idealWidth = 720 (full parent width)
    auto lc = swiperLayoutProp->GetLayoutConstraint();
    ASSERT_TRUE(lc.has_value());
    EXPECT_FLOAT_EQ(lc->selfIdealSize.Width().value_or(0.0f), 720.0f);
    EXPECT_FLOAT_EQ(lc->selfIdealSize.Height().value_or(0.0f), 400.0f);
}

// ===================== MeasureTabBar =====================

/**
 * @tc.name: MeasureTabBarNotFloating001
 * @tc.desc: MeasureTabBar with not floating bar measures with constraint
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, MeasureTabBarNotFloating001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(frameNode_, nullptr);
    ASSERT_NE(tabBarNode_, nullptr);

    auto algo = AceType::MakeRefPtr<TabsLayoutAlgorithm>();
    auto geo = AceType::MakeRefPtr<GeometryNode>();
    auto lp = frameNode_->GetLayoutProperty<TabsLayoutProperty>();
    auto wrapper = std::make_unique<LayoutWrapperNode>(frameNode_, geo, lp);
    wrapper->SetLayoutAlgorithm(AceType::MakeRefPtr<LayoutAlgorithmWrapper>(algo));
    auto tabBarGeo = AceType::MakeRefPtr<GeometryNode>();
    auto tabBarWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(
        tabBarNode_, tabBarGeo, tabBarNode_->GetLayoutProperty());
    wrapper->AppendChild(tabBarWrapper);

    LayoutConstraintF constraint;
    constraint.maxSize.SetWidth(720.0f);
    constraint.maxSize.SetHeight(56.0f);
    algo->SetIsFloatingBar(false);
    algo->SetIsFloatingTopBar(false);
    algo->MeasureTabBar(wrapper.get(), constraint);
    // Should have measured the tabBar
    EXPECT_NE(algo, nullptr);
}

// ===================== UpdateSideBarAndSideBarDividerVisibility =====================

/**
 * @tc.name: UpdateSideBarAndSideBarDividerVisibilityFalse001
 * @tc.desc: UpdateSideBarAndSideBarDividerVisibility with isVisible=false
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, UpdateSideBarAndSideBarDividerVisibilityFalse001, TestSize.Level1)
{
    LayoutAlgoTestCtx ctx;
    ctx.Build(720.0f, 400.0f);
    ctx.algo->UpdateSideBarAndSideBarDividerVisibility(ctx.layoutWrapper.get(), false);
    EXPECT_NE(ctx.algo, nullptr);
}

// ===================== LayoutBackgroundMask (layout-file) =====================

/**
 * @tc.name: LayoutBackgroundMaskNotFloating001
 * @tc.desc: LayoutBackgroundMask with not floating bar is a no-op
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, LayoutBackgroundMaskNotFloating001, TestSize.Level1)
{
    LayoutAlgoTestCtx ctx;
    ctx.Build(720.0f, 400.0f);
    ctx.algo->SetIsFloatingBar(false);
    ctx.algo->SetIsFloatingTopBar(false);
    ctx.algo->LayoutBackgroundMask(ctx.layoutWrapper.get());
    EXPECT_NE(ctx.algo, nullptr);
}

// ===================== LayoutOffsetList =====================

/**
 * @tc.name: LayoutOffsetListNoChildren001
 * @tc.desc: LayoutOffsetList with no children wrappers returns empty vector
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, LayoutOffsetListNoChildren001, TestSize.Level1)
{
    LayoutAlgoTestCtx ctx;
    ctx.Build(720.0f, 400.0f);
    auto tabBarGeo = AceType::MakeRefPtr<GeometryNode>();
    auto tabBarLayoutProp = AceType::MakeRefPtr<LayoutProperty>();
    auto tabBarNode = FrameNode::CreateFrameNode(
        V2::TAB_BAR_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    auto tabBarWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(tabBarNode, tabBarGeo, tabBarLayoutProp);
    auto effectGeo = AceType::MakeRefPtr<GeometryNode>();
    auto effectLayoutProp = AceType::MakeRefPtr<LayoutProperty>();
    auto effectNode = FrameNode::CreateFrameNode(
        "EffectNode", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    auto effectWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(effectNode, effectGeo, effectLayoutProp);
    auto offsets = ctx.algo->LayoutOffsetList(
        ctx.layoutWrapper.get(), tabBarWrapper, effectWrapper, SizeF(720.0f, 400.0f));
    // LayoutOffsetList always returns 4 offsets (swiper, divider, tabBar, effect)
    EXPECT_EQ(offsets.size(), 4u);
}

// ===================== MeasureSideBar (more from layout-file) =====================

/**
 * @tc.name: MeasureSideBarWithSideBar001
 * @tc.desc: MeasureSideBar with sideBarNode set returns non-zero width
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, MeasureSideBarWithSideBar001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(frameNode_, nullptr);

    auto algo = AceType::MakeRefPtr<TabsLayoutAlgorithm>();
    auto geo = AceType::MakeRefPtr<GeometryNode>();
    auto lp = frameNode_->GetLayoutProperty<TabsLayoutProperty>();
    auto wrapper = std::make_unique<LayoutWrapperNode>(frameNode_, geo, lp);
    wrapper->SetLayoutAlgorithm(AceType::MakeRefPtr<LayoutAlgorithmWrapper>(algo));

    // Create a sideBar node and add it as child
    auto sideBarNode = FrameNode::CreateFrameNode(
        "SideBar", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    auto sideBarGeo = AceType::MakeRefPtr<GeometryNode>();
    auto sideBarWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(
        sideBarNode, sideBarGeo, sideBarNode->GetLayoutProperty());
    wrapper->AppendChild(sideBarWrapper);
    // Set sideBarWidth to 200
    lp->UpdateSidebarWidth(Dimension(200.0f));
    algo->MeasureSideBar(wrapper.get(), lp, SizeF(720.0f, 400.0f));
    EXPECT_NE(algo, nullptr);
}

// ===================== CalcTopBarGridMaxWidth (calc-file) =====================

/**
 * @tc.name: LayoutAlgoCalcTopBarGridMaxWidthSmallWidth001
 * @tc.desc: tabWidth < 600vp → 4-col grid, gutter 8vp, occupy 4
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, LayoutAlgoCalcTopBarGridMaxWidthSmallWidth001, TestSize.Level1)
{
    auto algo = AceType::MakeRefPtr<TabsLayoutAlgorithm>();
    float tabWidth = 400.0f;
    float barSideMargin = 16.0f;
    float result = algo->CalcTopBarGridMaxWidth(tabWidth, barSideMargin);
    // usableWidth = 400 - 2*16 = 368; cols=4, gutter=8
    // columnWidth = (368 - 3*8)/4 = 86; maxWidth = 86*4 + 3*8 = 368
    EXPECT_FLOAT_EQ(result, 368.0f);
}

/**
 * @tc.name: LayoutAlgoCalcTopBarGridMaxWidthMediumWidth001
 * @tc.desc: 600 <= tabWidth < 840 → 8-col grid, gutter 12vp, occupy 8
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, LayoutAlgoCalcTopBarGridMaxWidthMediumWidth001, TestSize.Level1)
{
    auto algo = AceType::MakeRefPtr<TabsLayoutAlgorithm>();
    float tabWidth = 700.0f;
    float barSideMargin = 24.0f;
    float result = algo->CalcTopBarGridMaxWidth(tabWidth, barSideMargin);
    // usableWidth = 700 - 2*24 = 652; cols=8, gutter=12, occupy=8
    // columnWidth = (652 - 7*12)/8 = 71; maxWidth = 71*8 + 7*12 = 652
    EXPECT_FLOAT_EQ(result, 652.0f);
}

/**
 * @tc.name: LayoutAlgoCalcTopBarGridMaxWidthLargeWidth001
 * @tc.desc: tabWidth >= 840 → 12-col grid, gutter 16vp, occupy 10
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, LayoutAlgoCalcTopBarGridMaxWidthLargeWidth001, TestSize.Level1)
{
    auto algo = AceType::MakeRefPtr<TabsLayoutAlgorithm>();
    float tabWidth = 1000.0f;
    float barSideMargin = 32.0f;
    float result = algo->CalcTopBarGridMaxWidth(tabWidth, barSideMargin);
    // usableWidth = 1000 - 2*32 = 936; cols=12, gutter=16
    // columnWidth = (936 - 11*16)/12 = 63.333...; maxWidth = 63.333*10 + 9*16 = 777.333
    EXPECT_NEAR(result, 777.333f, 0.01f);
}

/**
 * @tc.name: LayoutAlgoCalcTopBarGridMaxWidthMarginShrinksResult001
 * @tc.desc: Larger barSideMargin yields smaller maxWidth
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, LayoutAlgoCalcTopBarGridMaxWidthMarginShrinksResult001, TestSize.Level1)
{
    auto algo = AceType::MakeRefPtr<TabsLayoutAlgorithm>();
    float tabWidth = 1000.0f;
    float resultSmall = algo->CalcTopBarGridMaxWidth(tabWidth, 8.0f);
    float resultLarge = algo->CalcTopBarGridMaxWidth(tabWidth, 64.0f);
    EXPECT_GT(resultSmall, resultLarge);
}

/**
 * @tc.name: LayoutAlgoCalcTopBarGridMaxWidthZeroMargin001
 * @tc.desc: With zero margin, maxWidth equals usableWidth for occupy==cols (small width)
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, LayoutAlgoCalcTopBarGridMaxWidthZeroMargin001, TestSize.Level1)
{
    auto algo = AceType::MakeRefPtr<TabsLayoutAlgorithm>();
    float tabWidth = 500.0f;
    float result = algo->CalcTopBarGridMaxWidth(tabWidth, 0.0f);
    // cols=4, occupy=4 → maxWidth = usableWidth = 500
    EXPECT_FLOAT_EQ(result, 500.0f);
}

// ===================== ParseBarWidthAndMargin (margin-only branch) =====================

/**
 * @tc.name: ParseBarWidthAndMarginSmallWidthMargin001
 * @tc.desc: tabsWidth < 600 → floatingBarMargin_ = 16vp; no barFloatingStyle → nullopt
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, ParseBarWidthAndMarginSmallWidthMargin001, TestSize.Level1)
{
    LayoutAlgoTestCtx ctx;
    ctx.Build(2, 400.0f, 1000.0f);
    auto ret = ctx.algo->ParseBarWidthAndMargin(ctx.layoutWrapper.get(), 400.0f, 1000.0f);
    EXPECT_FALSE(ret.has_value());
    ASSERT_TRUE(ctx.algo->floatingBarMargin_.has_value());
    EXPECT_FLOAT_EQ(ctx.algo->floatingBarMargin_.value(), 16.0f);
}

/**
 * @tc.name: ParseBarWidthAndMarginMediumWidthMargin001
 * @tc.desc: 600 <= tabsWidth < 840 → floatingBarMargin_ = 24vp
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, ParseBarWidthAndMarginMediumWidthMargin001, TestSize.Level1)
{
    LayoutAlgoTestCtx ctx;
    ctx.Build(2, 700.0f, 1000.0f);
    auto ret = ctx.algo->ParseBarWidthAndMargin(ctx.layoutWrapper.get(), 700.0f, 1000.0f);
    EXPECT_FALSE(ret.has_value());
    ASSERT_TRUE(ctx.algo->floatingBarMargin_.has_value());
    EXPECT_FLOAT_EQ(ctx.algo->floatingBarMargin_.value(), 24.0f);
}

/**
 * @tc.name: ParseBarWidthAndMarginLargeWidthMargin001
 * @tc.desc: tabsWidth >= 840 → floatingBarMargin_ = 32vp
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, ParseBarWidthAndMarginLargeWidthMargin001, TestSize.Level1)
{
    LayoutAlgoTestCtx ctx;
    ctx.Build(2, 1000.0f, 1000.0f);
    auto ret = ctx.algo->ParseBarWidthAndMargin(ctx.layoutWrapper.get(), 1000.0f, 1000.0f);
    EXPECT_FALSE(ret.has_value());
    ASSERT_TRUE(ctx.algo->floatingBarMargin_.has_value());
    EXPECT_FLOAT_EQ(ctx.algo->floatingBarMargin_.value(), 32.0f);
}

/**
 * @tc.name: ParseBarWidthAndMarginCustomSideMargin001
 * @tc.desc: barFloatingStyle.barSideMargin set and non-negative overrides default margin
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, ParseBarWidthAndMarginCustomSideMargin001, TestSize.Level1)
{
    LayoutAlgoTestCtx ctx;
    ctx.Build(2, 400.0f, 1000.0f);
    BarFloatingStyleParameters style;
    style.barSideMargin = Dimension(50.0f, DimensionUnit::VP);
    ctx.layoutProperty->UpdateBarFloatingStyle(style);
    auto ret = ctx.algo->ParseBarWidthAndMargin(ctx.layoutWrapper.get(), 400.0f, 1000.0f);
    EXPECT_FALSE(ret.has_value());
    ASSERT_TRUE(ctx.algo->floatingBarMargin_.has_value());
    EXPECT_FLOAT_EQ(ctx.algo->floatingBarMargin_.value(), 50.0f);
}

/**
 * @tc.name: ParseBarWidthAndMarginSmallBarWidth001
 * @tc.desc: tabsWidth < 440 and smallBarWidth set → returns smallBarWidth in px
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, ParseBarWidthAndMarginSmallBarWidth001, TestSize.Level1)
{
    LayoutAlgoTestCtx ctx;
    ctx.Build(2, 300.0f, 1000.0f);
    BarFloatingStyleParameters style;
    style.smallBarWidth = Dimension(120.0f, DimensionUnit::VP);
    ctx.layoutProperty->UpdateBarFloatingStyle(style);
    auto ret = ctx.algo->ParseBarWidthAndMargin(ctx.layoutWrapper.get(), 300.0f, 1000.0f);
    ASSERT_TRUE(ret.has_value());
    EXPECT_FLOAT_EQ(ret.value(), 120.0f);
}

/**
 * @tc.name: ParseBarWidthAndMarginMediumBarWidth001
 * @tc.desc: Medium width and mediumBarWidth set → returns mediumBarWidth in px
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, ParseBarWidthAndMarginMediumBarWidth001, TestSize.Level1)
{
    LayoutAlgoTestCtx ctx;
    ctx.Build(2, 500.0f, 1000.0f);
    BarFloatingStyleParameters style;
    style.mediumBarWidth = Dimension(200.0f, DimensionUnit::VP);
    ctx.layoutProperty->UpdateBarFloatingStyle(style);
    auto ret = ctx.algo->ParseBarWidthAndMargin(ctx.layoutWrapper.get(), 500.0f, 1000.0f);
    ASSERT_TRUE(ret.has_value());
    EXPECT_FLOAT_EQ(ret.value(), 200.0f);
}

/**
 * @tc.name: ParseBarWidthAndMarginLargeBarWidthCalc001
 * @tc.desc: Large width and largeBarWidth set → returns largeBarWidth in px
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, ParseBarWidthAndMarginLargeBarWidthCalc001, TestSize.Level1)
{
    LayoutAlgoTestCtx ctx;
    ctx.Build(2, 1000.0f, 1000.0f);
    BarFloatingStyleParameters style;
    style.largeBarWidth = Dimension(300.0f, DimensionUnit::VP);
    ctx.layoutProperty->UpdateBarFloatingStyle(style);
    auto ret = ctx.algo->ParseBarWidthAndMargin(ctx.layoutWrapper.get(), 1000.0f, 1000.0f);
    ASSERT_TRUE(ret.has_value());
    EXPECT_FLOAT_EQ(ret.value(), 300.0f);
}

// ===================== CalcFloatingBarWidth (calc-file) =====================

/**
 * @tc.name: CalcFloatingBarWidth2ItemsSmall001
 * @tc.desc: 2 items, width < 600 → COUNT_2_MINIMUM (168vp)
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, CalcFloatingBarWidth2ItemsSmall001, TestSize.Level1)
{
    LayoutAlgoTestCtx ctx;
    ctx.Build(2, 400.0f, 1000.0f);
    auto ret = ctx.algo->CalcFloatingBarWidth(ctx.layoutWrapper.get(), 56.0f);
    ASSERT_TRUE(ret.has_value());
    EXPECT_FLOAT_EQ(ret.value(), 168.0f);
}

/**
 * @tc.name: CalcFloatingBarWidth2ItemsLarge001
 * @tc.desc: 2 items, width >= 600 → COUNT_2_MAXIMUM (184vp)
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, CalcFloatingBarWidth2ItemsLarge001, TestSize.Level1)
{
    LayoutAlgoTestCtx ctx;
    ctx.Build(2, 700.0f, 1000.0f);
    auto ret = ctx.algo->CalcFloatingBarWidth(ctx.layoutWrapper.get(), 56.0f);
    ASSERT_TRUE(ret.has_value());
    EXPECT_FLOAT_EQ(ret.value(), 184.0f);
}

/**
 * @tc.name: CalcFloatingBarWidth3ItemsSmall001
 * @tc.desc: 3 items, width < 600 → COUNT_3_MINIMUM (248vp)
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, CalcFloatingBarWidth3ItemsSmall001, TestSize.Level1)
{
    LayoutAlgoTestCtx ctx;
    ctx.Build(3, 400.0f, 1000.0f);
    auto ret = ctx.algo->CalcFloatingBarWidth(ctx.layoutWrapper.get(), 56.0f);
    ASSERT_TRUE(ret.has_value());
    EXPECT_FLOAT_EQ(ret.value(), 248.0f);
}

/**
 * @tc.name: CalcFloatingBarWidth3ItemsLarge001
 * @tc.desc: 3 items, width >= 600 → COUNT_3_MAXIMUM (272vp)
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, CalcFloatingBarWidth3ItemsLarge001, TestSize.Level1)
{
    LayoutAlgoTestCtx ctx;
    ctx.Build(3, 700.0f, 1000.0f);
    auto ret = ctx.algo->CalcFloatingBarWidth(ctx.layoutWrapper.get(), 56.0f);
    ASSERT_TRUE(ret.has_value());
    EXPECT_FLOAT_EQ(ret.value(), 272.0f);
}

/**
 * @tc.name: CalcFloatingBarWidth4ItemsSmall001
 * @tc.desc: 4 items, width < 600 → width = tabsWidth - 2*sideMargin, clamped to 328
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, CalcFloatingBarWidth4ItemsSmall001, TestSize.Level1)
{
    LayoutAlgoTestCtx ctx;
    ctx.Build(4, 400.0f, 1000.0f);
    auto ret = ctx.algo->CalcFloatingBarWidth(ctx.layoutWrapper.get(), 56.0f);
    ASSERT_TRUE(ret.has_value());
    // sideMargin = 16, width = 400 - 2*16 = 368, clamped to 328
    EXPECT_FLOAT_EQ(ret.value(), 328.0f);
}

/**
 * @tc.name: CalcFloatingBarWidth5ItemsLarge001
 * @tc.desc: 5 items (>=4), width >= 840 → width = (tabsWidth/2 - 2*sideMargin)*2, clamped to 360
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, CalcFloatingBarWidth5ItemsLarge001, TestSize.Level1)
{
    LayoutAlgoTestCtx ctx;
    ctx.Build(5, 1000.0f, 1000.0f);
    auto ret = ctx.algo->CalcFloatingBarWidth(ctx.layoutWrapper.get(), 56.0f);
    ASSERT_TRUE(ret.has_value());
    // sideMargin = 32, width = (1000/2 - 2*32)*2 = (500-64)*2 = 872, clamped to 360
    EXPECT_FLOAT_EQ(ret.value(), 360.0f);
}

/**
 * @tc.name: CalcFloatingBarWidth4ItemsMedium001
 * @tc.desc: 4 items, 600 <= width < 840 → width = tabsWidth - 2*(sideMargin + 12 + barHeight)
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, CalcFloatingBarWidth4ItemsMedium001, TestSize.Level1)
{
    LayoutAlgoTestCtx ctx;
    ctx.Build(4, 700.0f, 1000.0f);
    auto ret = ctx.algo->CalcFloatingBarWidth(ctx.layoutWrapper.get(), 56.0f);
    ASSERT_TRUE(ret.has_value());
    // sideMargin = 24, barHeight = min(56, 1000) = 56
    // width = 700 - 2*(24 + 12 + 56) = 700 - 184 = 516, clamped to 328
    EXPECT_FLOAT_EQ(ret.value(), 328.0f);
}

/**
 * @tc.name: CalcFloatingBarWidthWithCustomBarWidth001
 * @tc.desc: When barFloatingStyle provides a custom barWidth, CalcFloatingBarWidth returns it
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, CalcFloatingBarWidthWithCustomBarWidth001, TestSize.Level1)
{
    LayoutAlgoTestCtx ctx;
    ctx.Build(2, 300.0f, 1000.0f);
    BarFloatingStyleParameters style;
    style.smallBarWidth = Dimension(150.0f, DimensionUnit::VP);
    ctx.layoutProperty->UpdateBarFloatingStyle(style);
    auto ret = ctx.algo->CalcFloatingBarWidth(ctx.layoutWrapper.get(), 56.0f);
    ASSERT_TRUE(ret.has_value());
    EXPECT_FLOAT_EQ(ret.value(), 150.0f);
}

/**
 * @tc.name: CalcFloatingBarWidthZeroBarItemSize001
 * @tc.desc: barItemSize 0 → barItemSize <= 2 → returns COUNT_2_MAXIMUM for width >= 600
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, CalcFloatingBarWidthZeroBarItemSize001, TestSize.Level1)
{
    LayoutAlgoTestCtx ctx;
    ctx.Build(0, 700.0f, 1000.0f);
    auto ret = ctx.algo->CalcFloatingBarWidth(ctx.layoutWrapper.get(), 56.0f);
    // barItemSize_=0 <= 2, tabsWidth=700 >= breakpoint600 → COUNT_2_MAXIMUM
    EXPECT_TRUE(ret.has_value());
    EXPECT_GT(ret.value(), 0.0f);
}

// ===================== CalcFloatingBarWidth (more from layout-file) =====================

/**
 * @tc.name: CalcFloatingBarWidth4ItemsLarge001
 * @tc.desc: CalcFloatingBarWidth with 4 items at large width (>840)
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, CalcFloatingBarWidth4ItemsLarge001, TestSize.Level1)
{
    LayoutAlgoTestCtx ctx;
    ctx.Build(1200.0f, 400.0f);
    ctx.algo->SetBarItemSize(4);
    ctx.algo->SetIsFloatingBar(true);
    auto result = ctx.algo->CalcFloatingBarWidth(ctx.layoutWrapper.get(), 56.0f);
    ASSERT_TRUE(result.has_value());
    EXPECT_GT(result.value(), 0.0f);
}

/**
 * @tc.name: CalcFloatingBarWidth5ItemsMedium001
 * @tc.desc: CalcFloatingBarWidth with 5 items at medium width (600-840)
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, CalcFloatingBarWidth5ItemsMedium001, TestSize.Level1)
{
    LayoutAlgoTestCtx ctx;
    ctx.Build(700.0f, 400.0f);
    ctx.algo->SetBarItemSize(5);
    ctx.algo->SetIsFloatingBar(true);
    auto result = ctx.algo->CalcFloatingBarWidth(ctx.layoutWrapper.get(), 56.0f);
    ASSERT_TRUE(result.has_value());
    EXPECT_GT(result.value(), 0.0f);
}

// ===================== ParseBarWidthAndMargin (more from layout-file) =====================

/**
 * @tc.name: ParseBarWidthAndMarginNoStyle001
 * @tc.desc: ParseBarWidthAndMargin with no bar floating style returns nullopt
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, ParseBarWidthAndMarginNoStyle001, TestSize.Level1)
{
    LayoutAlgoTestCtx ctx;
    ctx.Build(720.0f, 400.0f);
    auto result = ctx.algo->ParseBarWidthAndMargin(ctx.layoutWrapper.get(), 720.0f, 400.0f);
    EXPECT_FALSE(result.has_value());
}

/**
 * @tc.name: ParseBarWidthAndMarginLargeBarWidth001
 * @tc.desc: ParseBarWidthAndMargin with largeBarWidth set by user
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, ParseBarWidthAndMarginLargeBarWidth001, TestSize.Level1)
{
    LayoutAlgoTestCtx ctx;
    ctx.Build(1200.0f, 400.0f);
    BarFloatingStyleParameters style;
    style.largeBarWidth = Dimension(500.0f);
    ctx.layoutProperty->UpdateBarFloatingStyle(style);
    auto result = ctx.algo->ParseBarWidthAndMargin(ctx.layoutWrapper.get(), 1200.0f, 400.0f);
    ASSERT_TRUE(result.has_value());
    EXPECT_FLOAT_EQ(result.value(), 500.0f);
}

// ===================== SetWrapContentMaxHeight / SetWrapContentMaxWidth =====================

/**
 * @tc.name: SetWrapContentMaxHeightWrapContent001
 * @tc.desc: SetWrapContentMaxHeight with WRAP_CONTENT policy sets max height
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, SetWrapContentMaxHeightWrapContent001, TestSize.Level1)
{
    auto algo = AceType::MakeRefPtr<TabsLayoutAlgorithm>();
    algo->heightLayoutPolicy_ = LayoutCalPolicy::WRAP_CONTENT;
    LayoutConstraintF constraint;
    constraint.maxSize.SetHeight(500.0f);
    algo->SetWrapContentMaxHeight(constraint, 300.0f);
    EXPECT_FLOAT_EQ(constraint.maxSize.Height(), 300.0f);
}

/**
 * @tc.name: SetWrapContentMaxHeightNotWrap001
 * @tc.desc: SetWrapContentMaxHeight with non-WRAP_CONTENT policy is a no-op
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, SetWrapContentMaxHeightNotWrap001, TestSize.Level1)
{
    auto algo = AceType::MakeRefPtr<TabsLayoutAlgorithm>();
    algo->heightLayoutPolicy_ = LayoutCalPolicy::NO_MATCH;
    LayoutConstraintF constraint;
    constraint.maxSize.SetHeight(500.0f);
    algo->SetWrapContentMaxHeight(constraint, 300.0f);
    EXPECT_FLOAT_EQ(constraint.maxSize.Height(), 500.0f);
}

/**
 * @tc.name: SetWrapContentMaxWidthWrapContent001
 * @tc.desc: SetWrapContentMaxWidth with WRAP_CONTENT policy sets max width
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, SetWrapContentMaxWidthWrapContent001, TestSize.Level1)
{
    auto algo = AceType::MakeRefPtr<TabsLayoutAlgorithm>();
    algo->widthLayoutPolicy_ = LayoutCalPolicy::WRAP_CONTENT;
    LayoutConstraintF constraint;
    constraint.maxSize.SetWidth(720.0f);
    algo->SetWrapContentMaxWidth(constraint, 500.0f);
    EXPECT_FLOAT_EQ(constraint.maxSize.Width(), 500.0f);
}

/**
 * @tc.name: SetWrapContentMaxWidthNotWrap001
 * @tc.desc: SetWrapContentMaxWidth with non-WRAP_CONTENT policy is a no-op
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, SetWrapContentMaxWidthNotWrap001, TestSize.Level1)
{
    auto algo = AceType::MakeRefPtr<TabsLayoutAlgorithm>();
    algo->widthLayoutPolicy_ = LayoutCalPolicy::NO_MATCH;
    LayoutConstraintF constraint;
    constraint.maxSize.SetWidth(720.0f);
    algo->SetWrapContentMaxWidth(constraint, 500.0f);
    EXPECT_FLOAT_EQ(constraint.maxSize.Width(), 720.0f);
}

// ===================== SetFixAtIdealSizeMaxSize =====================

/**
 * @tc.name: SetFixAtIdealSizeMaxSizeWidth001
 * @tc.desc: SetFixAtIdealSizeMaxSize with width FIX_AT_IDEAL_SIZE sets width to infinity
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, SetFixAtIdealSizeMaxSizeWidth001, TestSize.Level1)
{
    auto algo = AceType::MakeRefPtr<TabsLayoutAlgorithm>();
    algo->widthLayoutPolicy_ = LayoutCalPolicy::FIX_AT_IDEAL_SIZE;
    algo->heightLayoutPolicy_ = LayoutCalPolicy::WRAP_CONTENT;
    LayoutConstraintF constraint;
    constraint.maxSize.SetWidth(720.0f);
    constraint.maxSize.SetHeight(400.0f);
    algo->SetFixAtIdealSizeMaxSize(constraint);
    EXPECT_EQ(constraint.maxSize.Width(), std::numeric_limits<float>::infinity());
    EXPECT_FLOAT_EQ(constraint.maxSize.Height(), 400.0f);
}

/**
 * @tc.name: SetFixAtIdealSizeMaxSizeHeight001
 * @tc.desc: SetFixAtIdealSizeMaxSize with height FIX_AT_IDEAL_SIZE sets height to infinity
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, SetFixAtIdealSizeMaxSizeHeight001, TestSize.Level1)
{
    auto algo = AceType::MakeRefPtr<TabsLayoutAlgorithm>();
    algo->widthLayoutPolicy_ = LayoutCalPolicy::WRAP_CONTENT;
    algo->heightLayoutPolicy_ = LayoutCalPolicy::FIX_AT_IDEAL_SIZE;
    LayoutConstraintF constraint;
    constraint.maxSize.SetWidth(720.0f);
    constraint.maxSize.SetHeight(400.0f);
    algo->SetFixAtIdealSizeMaxSize(constraint);
    EXPECT_FLOAT_EQ(constraint.maxSize.Width(), 720.0f);
    EXPECT_EQ(constraint.maxSize.Height(), std::numeric_limits<float>::infinity());
}

/**
 * @tc.name: SetFixAtIdealSizeMaxSizeBoth001
 * @tc.desc: SetFixAtIdealSizeMaxSize with both FIX_AT_IDEAL_SIZE sets both to infinity
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, SetFixAtIdealSizeMaxSizeBoth001, TestSize.Level1)
{
    auto algo = AceType::MakeRefPtr<TabsLayoutAlgorithm>();
    algo->widthLayoutPolicy_ = LayoutCalPolicy::FIX_AT_IDEAL_SIZE;
    algo->heightLayoutPolicy_ = LayoutCalPolicy::FIX_AT_IDEAL_SIZE;
    LayoutConstraintF constraint;
    constraint.maxSize.SetWidth(720.0f);
    constraint.maxSize.SetHeight(400.0f);
    algo->SetFixAtIdealSizeMaxSize(constraint);
    EXPECT_EQ(constraint.maxSize.Width(), std::numeric_limits<float>::infinity());
    EXPECT_EQ(constraint.maxSize.Height(), std::numeric_limits<float>::infinity());
}

// ===================== IsWidthFixOrWrap / IsHeightFixOrWrap =====================

/**
 * @tc.name: IsWidthFixOrWrapTrue001
 * @tc.desc: IsWidthFixOrWrap returns true for FIX_AT_IDEAL_SIZE
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, IsWidthFixOrWrapTrue001, TestSize.Level1)
{
    auto algo = AceType::MakeRefPtr<TabsLayoutAlgorithm>();
    algo->widthLayoutPolicy_ = LayoutCalPolicy::FIX_AT_IDEAL_SIZE;
    EXPECT_TRUE(algo->IsWidthFixOrWrap());
}

/**
 * @tc.name: IsWidthFixOrWrapTrue002
 * @tc.desc: IsWidthFixOrWrap returns true for WRAP_CONTENT
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, IsWidthFixOrWrapTrue002, TestSize.Level1)
{
    auto algo = AceType::MakeRefPtr<TabsLayoutAlgorithm>();
    algo->widthLayoutPolicy_ = LayoutCalPolicy::WRAP_CONTENT;
    EXPECT_TRUE(algo->IsWidthFixOrWrap());
}

/**
 * @tc.name: IsWidthFixOrWrapFalse001
 * @tc.desc: IsWidthFixOrWrap returns false for NO_CONSTRAINT
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, IsWidthFixOrWrapFalse001, TestSize.Level1)
{
    auto algo = AceType::MakeRefPtr<TabsLayoutAlgorithm>();
    algo->widthLayoutPolicy_ = LayoutCalPolicy::NO_MATCH;
    EXPECT_FALSE(algo->IsWidthFixOrWrap());
}

/**
 * @tc.name: IsHeightFixOrWrapTrue001
 * @tc.desc: IsHeightFixOrWrap returns true for FIX_AT_IDEAL_SIZE
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, IsHeightFixOrWrapTrue001, TestSize.Level1)
{
    auto algo = AceType::MakeRefPtr<TabsLayoutAlgorithm>();
    algo->heightLayoutPolicy_ = LayoutCalPolicy::FIX_AT_IDEAL_SIZE;
    EXPECT_TRUE(algo->IsHeightFixOrWrap());
}

/**
 * @tc.name: IsHeightFixOrWrapTrue002
 * @tc.desc: IsHeightFixOrWrap returns true for WRAP_CONTENT
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, IsHeightFixOrWrapTrue002, TestSize.Level1)
{
    auto algo = AceType::MakeRefPtr<TabsLayoutAlgorithm>();
    algo->heightLayoutPolicy_ = LayoutCalPolicy::WRAP_CONTENT;
    EXPECT_TRUE(algo->IsHeightFixOrWrap());
}

/**
 * @tc.name: IsHeightFixOrWrapFalse001
 * @tc.desc: IsHeightFixOrWrap returns false for NO_CONSTRAINT
 * @tc.type: FUNC
 */
HWTEST_F(TabsLayoutAlgoTddTestNg, IsHeightFixOrWrapFalse001, TestSize.Level1)
{
    auto algo = AceType::MakeRefPtr<TabsLayoutAlgorithm>();
    algo->heightLayoutPolicy_ = LayoutCalPolicy::NO_MATCH;
    EXPECT_FALSE(algo->IsHeightFixOrWrap());
}
} // namespace OHOS::Ace::NG

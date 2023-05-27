/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include <cstddef>
#include <utility>

#include "gtest/gtest.h"

#include "base/geometry/dimension.h"
#include "base/geometry/ng/size_t.h"
#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"

#define private public
#define protected public
#include "core/components/tab_bar/tab_theme.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/event/event_hub.h"
#include "core/components_ng/pattern/divider/divider_pattern.h"
#include "core/components_ng/pattern/divider/divider_render_property.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"
#include "core/components_ng/pattern/swiper/swiper_event_hub.h"
#include "core/components_ng/pattern/swiper/swiper_pattern.h"
#include "core/components_ng/pattern/tabs/tab_bar_pattern.h"
#include "core/components_ng/pattern/tabs/tab_content_model_ng.h"
#include "core/components_ng/pattern/tabs/tab_content_pattern.h"
#include "core/components_ng/pattern/tabs/tabs_model_ng.h"
#include "core/components_ng/pattern/tabs/tabs_pattern.h"
#include "core/components_ng/pattern/text/text_layout_property.h"
#include "core/components_ng/test/mock/rosen/mock_canvas.h"
#include "core/components_ng/test/mock/theme/mock_theme_manager.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/pipeline_ng/pipeline_context.h"
#include "core/pipeline_ng/test/mock/mock_pipeline_base.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {
constexpr float FIRST_ITEM_WIDTH = 800.0f;
constexpr float FIRST_ITEM_HEIGHT = 800.0f;
const SizeF FIRST_ITEM_SIZE(FIRST_ITEM_WIDTH, FIRST_ITEM_HEIGHT);
constexpr float INFINITY_NUM = 1000000.0f;
const SizeF INFINITY_SIZE(INFINITY_NUM, INFINITY_NUM);
constexpr float NEGTIVE_NUM = -100.0f;
const SizeF NEGTIVE_SIZE(NEGTIVE_NUM, NEGTIVE_NUM);
constexpr float TABBAR_WIDTH = 50.0f;
constexpr float SWIPER_WIDTH = 750.0f;
const double DEFAULT_OFFSET = -1.0f;
const int DEFAULT_ITEMCOUNT = 1;
const int DEFAULT_INDEX = -1;
const int BEGIN_INDEX = 0;
const int CURRENT_INDEX = 1;
const int END_INDEX = 0;
const OffsetF CURRENT_OFFSET(1.0f, 1.0f);
constexpr float TEST_MASK_A_RADIUS_RATIO = 0.0f;
constexpr float TEST_MASK_B_RADIUS_RATIO = 1.414f;
constexpr float TEST_MASK_MIDDLE_RADIUS_RATIO = (TEST_MASK_A_RADIUS_RATIO + TEST_MASK_B_RADIUS_RATIO) / 2.0f;
constexpr int32_t TEST_SWIPER_INDEX = 0;
constexpr int32_t TEST_DIVIDER_INDEX = 1;
constexpr int32_t TEST_TAB_BAR_INDEX = 2;
constexpr int32_t TEST_SELECTED_MASK_COUNT = 2;
constexpr int32_t TEST_UNSELECTED_MASK_COUNT = 1;
} // namespace

class TabsTestNg : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void MockPipelineContextGetTheme();
};

void TabsTestNg::SetUpTestSuite()
{
    MockPipelineBase::SetUp();
}

void TabsTestNg::TearDownTestSuite()
{
    MockPipelineBase::TearDown();
}

void TabsTestNg::MockPipelineContextGetTheme()
{
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    MockPipelineBase::GetCurrent()->SetThemeManager(themeManager);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(AceType::MakeRefPtr<TabTheme>()));
}

/**
 * @tc.name: TabsModelSetDivider001
 * @tc.desc: test SetDivider
 * @tc.type: FUNC
 */
HWTEST_F(TabsTestNg, TabsModelSetDivider001, TestSize.Level1)
{
    MockPipelineContextGetTheme();
    TabsModelNG tabsModel;
    TabsItemDivider divider;
    Dimension strokeWidth = 10.0_vp;
    Dimension startMargin = 3.0_vp;
    Dimension endMargin = 4.0_vp;
    Color color = Color::BLACK;

    divider.strokeWidth = strokeWidth;
    divider.startMargin = startMargin;
    divider.endMargin = endMargin;
    divider.color = color;
    divider.isNull = false;

    ViewStackProcessor::GetInstance()->StartGetAccessRecordingFor(100);
    tabsModel.Create(BarPosition::START, 1, nullptr, nullptr);
    tabsModel.SetDivider(divider);
    auto tabsFrameNode = AceType::DynamicCast<TabsNode>(ViewStackProcessor::GetInstance()->GetMainElementNode());
    ASSERT_NE(tabsFrameNode, nullptr);
    EXPECT_EQ(tabsFrameNode->GetTag(), V2::TABS_ETS_TAG);
    auto dividerNode = AceType::DynamicCast<FrameNode>(tabsFrameNode->GetChildAtIndex(1));
    ASSERT_NE(dividerNode, nullptr);
    EXPECT_EQ(dividerNode->GetTag(), V2::DIVIDER_ETS_TAG);
    auto layoutProperty = tabsFrameNode->GetLayoutProperty<TabsLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    EXPECT_EQ(layoutProperty->GetDivider()->color.ColorToString(), color.ColorToString());
    EXPECT_EQ(layoutProperty->GetDivider()->strokeWidth.ToString(), strokeWidth.ToString());
    EXPECT_EQ(layoutProperty->GetDivider()->startMargin.ToString(), startMargin.ToString());
    EXPECT_EQ(layoutProperty->GetDivider()->endMargin.ToString(), endMargin.ToString());
    EXPECT_EQ(layoutProperty->GetDivider()->isNull, false);

    auto clone = layoutProperty->Clone();
    clone.Reset();
    std::unique_ptr<JsonValue> json = std::make_unique<JsonValue>();
    layoutProperty->ToJsonValue(json);
    ASSERT_NE(json, nullptr);

    auto dividerJson = json->GetValue("divider");
    ASSERT_NE(dividerJson, nullptr);
    layoutProperty->Reset();
    layoutProperty->ToJsonValue(json);
    tabsModel.Pop();

    TabsItemDivider divider2;
    EXPECT_FALSE(divider == divider2);
}

/**
 * @tc.name: TabsModelSetDivider002
 * @tc.desc: test SetDivider
 * @tc.type: FUNC
 */
HWTEST_F(TabsTestNg, TabsModelSetDivider002, TestSize.Level1)
{
    MockPipelineContextGetTheme();
    TabsModelNG tabsModel;
    TabsItemDivider divider;
    divider.isNull = true;

    ViewStackProcessor::GetInstance()->StartGetAccessRecordingFor(100);
    tabsModel.Create(BarPosition::START, 1, nullptr, nullptr);
    tabsModel.SetDivider(divider);
    auto tabsFrameNode = AceType::DynamicCast<TabsNode>(ViewStackProcessor::GetInstance()->GetMainElementNode());
    ASSERT_NE(tabsFrameNode, nullptr);
    EXPECT_EQ(tabsFrameNode->GetTag(), V2::TABS_ETS_TAG);
    auto dividerNode = AceType::DynamicCast<FrameNode>(tabsFrameNode->GetChildAtIndex(1));
    ASSERT_NE(dividerNode, nullptr);
    EXPECT_EQ(dividerNode->GetTag(), V2::DIVIDER_ETS_TAG);
    auto layoutProperty = tabsFrameNode->GetLayoutProperty<TabsLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    EXPECT_EQ(layoutProperty->GetDivider()->isNull, true);

    auto clone = layoutProperty->Clone();
    clone.Reset();
    std::unique_ptr<JsonValue> json = std::make_unique<JsonValue>();
    layoutProperty->ToJsonValue(json);
    ASSERT_NE(json, nullptr);
}

/**
 * @tc.name: TabsModelSetFadingEdge001
 * @tc.desc: test SetFadingEdge
 * @tc.type: FUNC
 */
HWTEST_F(TabsTestNg, TabsModelSetFadingEdge001, TestSize.Level1)
{
    MockPipelineContextGetTheme();

    TabsModelNG tabsModel;
    bool fadingEdge = true;
    tabsModel.Create(BarPosition::START, 1, nullptr, nullptr);
    tabsModel.SetFadingEdge(fadingEdge);
    auto tabsFrameNode = AceType::DynamicCast<TabsNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(tabsFrameNode, nullptr);
    EXPECT_EQ(tabsFrameNode->GetTag(), V2::TABS_ETS_TAG);
    auto tabBarNode = AceType::DynamicCast<FrameNode>(tabsFrameNode->GetChildAtIndex(TEST_TAB_BAR_INDEX));
    ASSERT_NE(tabBarNode, nullptr);
    EXPECT_EQ(tabBarNode->GetTag(), V2::TAB_BAR_ETS_TAG);
    auto tabBarPaintProperty = tabBarNode->GetPaintProperty<TabBarPaintProperty>();
    ASSERT_NE(tabBarPaintProperty, nullptr);
    EXPECT_TRUE(tabBarPaintProperty->GetFadingEdgeValue());

    std::unique_ptr<JsonValue> json = std::make_unique<JsonValue>();
    tabsFrameNode->ToJsonValue(json);

    auto fadingEdgeJson = json->GetValue("fadingEdge");
    EXPECT_NE(fadingEdgeJson, nullptr);
}

/**
 * @tc.name: TabsModelMeasure001
 * @tc.desc: Test Divider Measure and Layout.
 * @tc.type: FUNC
 */
HWTEST_F(TabsTestNg, TabsModelMeasure001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize all properties of tabs.
     */
    MockPipelineContextGetTheme();

    TabsModelNG instance;
    instance.Create(BarPosition::START, 1, nullptr, nullptr);
    TabsItemDivider divider;
    instance.SetDivider(divider);

    /**
     * @tc.steps: step2. Get tabs pattern to create layoutAlgorithm, and call measure and layout functions.
     * @tc.expected: step2. related function is called.
     */
    auto tabsFrameNode = AceType::DynamicCast<TabsNode>(ViewStackProcessor::GetInstance()->GetMainElementNode());
    ASSERT_NE(tabsFrameNode, nullptr);
    auto pattern = tabsFrameNode->GetPattern<TabsPattern>();
    ASSERT_NE(pattern, nullptr);
    auto tabsLayoutAlgorithm = pattern->CreateLayoutAlgorithm();
    ASSERT_NE(tabsLayoutAlgorithm, nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    LayoutWrapper layoutWrapper = LayoutWrapper(tabsFrameNode, geometryNode, tabsFrameNode->GetLayoutProperty());
    LayoutConstraintF layoutConstraintVaild;
    float layoutSize = 10000.0f;
    layoutConstraintVaild.selfIdealSize.SetSize(SizeF(layoutSize, layoutSize));
    AceType::DynamicCast<TabsLayoutProperty>(layoutWrapper.GetLayoutProperty())
        ->UpdateLayoutConstraint(layoutConstraintVaild);
    layoutWrapper.SetLayoutAlgorithm(AceType::MakeRefPtr<LayoutAlgorithmWrapper>(tabsLayoutAlgorithm));
    layoutWrapper.GetLayoutProperty()->UpdateContentConstraint();

    auto childLayoutConstraint = layoutWrapper.GetLayoutProperty()->CreateChildConstraint();
    childLayoutConstraint.selfIdealSize = OptionalSizeF(FIRST_ITEM_SIZE);

    auto swiperNode = AceType::DynamicCast<FrameNode>(tabsFrameNode->GetChildAtIndex(TEST_SWIPER_INDEX));
    ASSERT_NE(swiperNode, nullptr);
    RefPtr<GeometryNode> geometryNode1 = AceType::MakeRefPtr<GeometryNode>();
    RefPtr<LayoutWrapper> swiperLayoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(swiperNode, geometryNode1, swiperNode->GetLayoutProperty());
    swiperLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
    swiperLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(FIRST_ITEM_WIDTH), CalcLength(FIRST_ITEM_HEIGHT)));
    layoutWrapper.AppendChild(swiperLayoutWrapper);

    auto dividerNode = AceType::DynamicCast<FrameNode>(tabsFrameNode->GetChildAtIndex(TEST_DIVIDER_INDEX));
    ASSERT_NE(dividerNode, nullptr);
    RefPtr<GeometryNode> geometryNode2 = AceType::MakeRefPtr<GeometryNode>();
    RefPtr<LayoutWrapper> dividerLayoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(dividerNode, geometryNode2, dividerNode->GetLayoutProperty());
    dividerLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
    dividerLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(FIRST_ITEM_WIDTH), CalcLength(FIRST_ITEM_HEIGHT)));
    layoutWrapper.AppendChild(dividerLayoutWrapper);

    auto tabBarNode = AceType::DynamicCast<FrameNode>(tabsFrameNode->GetChildAtIndex(TEST_TAB_BAR_INDEX));
    ASSERT_NE(tabBarNode, nullptr);
    RefPtr<GeometryNode> geometryNode3 = AceType::MakeRefPtr<GeometryNode>();
    RefPtr<LayoutWrapper> tabBarLayoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(tabBarNode, geometryNode3, tabBarNode->GetLayoutProperty());
    tabBarLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
    tabBarLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(FIRST_ITEM_WIDTH), CalcLength(FIRST_ITEM_HEIGHT)));
    layoutWrapper.AppendChild(tabBarLayoutWrapper);

    tabsLayoutAlgorithm->Measure(&layoutWrapper);
    tabsLayoutAlgorithm->Layout(&layoutWrapper);
    auto dividerOffset = dividerLayoutWrapper->GetGeometryNode()->GetMarginFrameOffset();
    EXPECT_EQ(dividerOffset.GetX(), 0);
    EXPECT_EQ(dividerOffset.GetY(), 0);

    tabsFrameNode->GetLayoutProperty<TabsLayoutProperty>()->UpdateTabBarPosition(BarPosition::END);
    layoutWrapper.GetGeometryNode()->SetFrameSize(SizeF(10000.0f, 10000.0f));
    tabsLayoutAlgorithm->Layout(&layoutWrapper);
    dividerOffset = dividerLayoutWrapper->GetGeometryNode()->GetMarginFrameOffset();
    EXPECT_EQ(dividerOffset.GetX(), 0);
    EXPECT_EQ(dividerOffset.GetY(), layoutSize);

    instance.SetIsVertical(true);
    tabsLayoutAlgorithm->Measure(&layoutWrapper);
    tabsLayoutAlgorithm->Layout(&layoutWrapper);
    dividerOffset = dividerLayoutWrapper->GetGeometryNode()->GetMarginFrameOffset();
    EXPECT_EQ(dividerOffset.GetX(), layoutSize);
    EXPECT_EQ(dividerOffset.GetY(), 0);

    tabsFrameNode->GetLayoutProperty<TabsLayoutProperty>()->UpdateTabBarPosition(BarPosition::START);
    layoutWrapper.GetGeometryNode()->SetFrameSize(SizeF(10000.0f, 10000.0f));
    tabsLayoutAlgorithm->Layout(&layoutWrapper);
    dividerOffset = dividerLayoutWrapper->GetGeometryNode()->GetMarginFrameOffset();
    EXPECT_EQ(dividerOffset.GetX(), 0);
    EXPECT_EQ(dividerOffset.GetY(), 0);
    tabsFrameNode->GetLayoutProperty<TabsLayoutProperty>()->UpdateBarOverlap(true);
    tabsLayoutAlgorithm->Measure(&layoutWrapper);
    EXPECT_TRUE(tabsFrameNode->GetLayoutProperty<TabsLayoutProperty>()->GetBarOverlap().value());

    tabsFrameNode->GetLayoutProperty<TabsLayoutProperty>()->UpdateBarOverlap(false);
    tabsLayoutAlgorithm->Measure(&layoutWrapper);
    EXPECT_FALSE(tabsFrameNode->GetLayoutProperty<TabsLayoutProperty>()->GetBarOverlap().value());
}

/**
 * @tc.name: TabsModelMeasure002
 * @tc.desc: Test Tabs Measure and Layout.
 * @tc.type: FUNC
 */
HWTEST_F(TabsTestNg, TabsModelMeasure002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize all properties of tabs.
     */
    MockPipelineContextGetTheme();

    TabsModelNG instance;
    instance.Create(BarPosition::START, 1, nullptr, nullptr);

    /**
     * @tc.steps: step2. Get tabs pattern to create layoutAlgorithm, and call measure and layout functions.
     * @tc.expected: related function is called.
     */
    auto tabsFrameNode = AceType::DynamicCast<TabsNode>(ViewStackProcessor::GetInstance()->GetMainElementNode());
    ASSERT_NE(tabsFrameNode, nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    auto pattern = tabsFrameNode->GetPattern<TabsPattern>();
    ASSERT_NE(pattern, nullptr);
    auto tabsLayoutAlgorithm = pattern->CreateLayoutAlgorithm();
    ASSERT_NE(tabsLayoutAlgorithm, nullptr);
    LayoutWrapper layoutWrapper = LayoutWrapper(tabsFrameNode, geometryNode, tabsFrameNode->GetLayoutProperty());
    LayoutConstraintF layoutConstrain;
    layoutConstrain.maxSize = FIRST_ITEM_SIZE;
    layoutConstrain.percentReference = FIRST_ITEM_SIZE;
    AceType::DynamicCast<TabsLayoutProperty>(layoutWrapper.GetLayoutProperty())
        ->UpdateLayoutConstraint(layoutConstrain);
    layoutWrapper.SetLayoutAlgorithm(AceType::MakeRefPtr<LayoutAlgorithmWrapper>(tabsLayoutAlgorithm));
    layoutWrapper.GetLayoutProperty()->UpdateContentConstraint();
    auto childLayoutConstraint = layoutWrapper.GetLayoutProperty()->CreateChildConstraint();
    childLayoutConstraint.maxSize = SizeF(FIRST_ITEM_SIZE);
    childLayoutConstraint.minSize = SizeF(0.0f, 0.0f);

    auto swiperNode = AceType::DynamicCast<FrameNode>(tabsFrameNode->GetChildAtIndex(TEST_SWIPER_INDEX));
    ASSERT_NE(swiperNode, nullptr);
    RefPtr<GeometryNode> swiperGeometryNode = AceType::MakeRefPtr<GeometryNode>();
    RefPtr<LayoutWrapper> swiperLayoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(swiperNode, swiperGeometryNode, swiperNode->GetLayoutProperty());
    
    auto swipeLayoutAlgorithm = swiperNode->GetPattern<Pattern>()->CreateLayoutAlgorithm();
    ASSERT_NE(swipeLayoutAlgorithm, nullptr);
    swiperLayoutWrapper->SetLayoutAlgorithm(
        AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(swipeLayoutAlgorithm));

    swiperLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
    swiperLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(FIRST_ITEM_WIDTH), CalcLength(SWIPER_WIDTH)));
    layoutWrapper.AppendChild(swiperLayoutWrapper);
    
    auto dividerNode = AceType::DynamicCast<FrameNode>(tabsFrameNode->GetChildAtIndex(TEST_DIVIDER_INDEX));
    ASSERT_NE(dividerNode, nullptr);
    RefPtr<GeometryNode> dividerGeometryNode = AceType::MakeRefPtr<GeometryNode>();
    RefPtr<LayoutWrapper> dividerLayoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(dividerNode, dividerGeometryNode, dividerNode->GetLayoutProperty());
    
    auto dividerLayoutAlgorithm = dividerNode->GetPattern<Pattern>()->CreateLayoutAlgorithm();
    ASSERT_NE(dividerLayoutAlgorithm, nullptr);
    dividerLayoutWrapper->SetLayoutAlgorithm(
        AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(dividerLayoutAlgorithm));

    dividerLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
    dividerLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(FIRST_ITEM_WIDTH), CalcLength(0.0f)));
    layoutWrapper.AppendChild(dividerLayoutWrapper);
    
    auto tabBarNode = AceType::DynamicCast<FrameNode>(tabsFrameNode->GetChildAtIndex(TEST_TAB_BAR_INDEX));
    ASSERT_NE(tabBarNode, nullptr);
    RefPtr<GeometryNode> tabbarGeometryNode = AceType::MakeRefPtr<GeometryNode>();
    RefPtr<LayoutWrapper> tabBarLayoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(tabBarNode, tabbarGeometryNode, tabBarNode->GetLayoutProperty());

    auto tabbarLayoutAlgorithm = tabBarNode->GetPattern<Pattern>()->CreateLayoutAlgorithm();
    ASSERT_NE(tabbarLayoutAlgorithm, nullptr);
    tabBarLayoutWrapper->SetLayoutAlgorithm(
        AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(tabbarLayoutAlgorithm));

    tabBarLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
    tabBarLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(FIRST_ITEM_WIDTH), CalcLength(TABBAR_WIDTH)));
    layoutWrapper.AppendChild(tabBarLayoutWrapper);

    tabsLayoutAlgorithm->Measure(&layoutWrapper);
    tabsLayoutAlgorithm->Layout(&layoutWrapper);

    auto tabbarOffset = tabBarLayoutWrapper->GetGeometryNode()->GetMarginFrameOffset();
    EXPECT_EQ(tabbarOffset.GetX(), 0.0f);
    EXPECT_EQ(tabbarOffset.GetY(), 0.0f);

    auto tabBarSize = tabBarLayoutWrapper->GetGeometryNode()->GetFrameSize();
    EXPECT_EQ(tabBarSize.Width(), FIRST_ITEM_WIDTH);
    EXPECT_EQ(tabBarSize.Height(), TABBAR_WIDTH);

    auto swiperOffset = swiperLayoutWrapper->GetGeometryNode()->GetMarginFrameOffset();
    EXPECT_EQ(swiperOffset.GetX(), 0.0f);
    EXPECT_EQ(swiperOffset.GetY(), TABBAR_WIDTH);

    auto swiperSize = swiperLayoutWrapper->GetGeometryNode()->GetFrameSize();
    EXPECT_EQ(swiperSize.Width(), FIRST_ITEM_WIDTH);
    EXPECT_EQ(swiperSize.Height(), SWIPER_WIDTH);

    tabsFrameNode->GetLayoutProperty<TabsLayoutProperty>()->UpdateTabBarPosition(BarPosition::END);
    tabsLayoutAlgorithm->Layout(&layoutWrapper);

    tabbarOffset = tabBarLayoutWrapper->GetGeometryNode()->GetMarginFrameOffset();
    EXPECT_EQ(tabbarOffset.GetX(), 0.0f);
    EXPECT_EQ(tabbarOffset.GetY(), SWIPER_WIDTH);

    swiperOffset = swiperLayoutWrapper->GetGeometryNode()->GetMarginFrameOffset();
    EXPECT_EQ(swiperOffset.GetX(), 0.0f);
    EXPECT_EQ(swiperOffset.GetY(), 0.0f);
}

/**
 * @tc.name: TabsModelMeasure003
 * @tc.desc: Test Tabs Measure and Layout when is vertical.
 * @tc.type: FUNC
 */
HWTEST_F(TabsTestNg, TabsModelMeasure003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize all properties of tabs.
     */
    MockPipelineContextGetTheme();

    TabsModelNG instance;
    instance.Create(BarPosition::START, 1, nullptr, nullptr);
    instance.SetIsVertical(true);

    /**
     * @tc.steps: step2. Get tabs pattern to create layoutAlgorithm, and call measure and layout functions.
     * @tc.expected: related function is called.
     */
    auto tabsFrameNode = AceType::DynamicCast<TabsNode>(ViewStackProcessor::GetInstance()->GetMainElementNode());
    ASSERT_NE(tabsFrameNode, nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    auto pattern = tabsFrameNode->GetPattern<TabsPattern>();
    ASSERT_NE(pattern, nullptr);
    auto tabsLayoutAlgorithm = pattern->CreateLayoutAlgorithm();
    ASSERT_NE(tabsLayoutAlgorithm, nullptr);
    LayoutWrapper layoutWrapper = LayoutWrapper(tabsFrameNode, geometryNode, tabsFrameNode->GetLayoutProperty());
    LayoutConstraintF layoutConstrain;
    layoutConstrain.maxSize = FIRST_ITEM_SIZE;
    layoutConstrain.percentReference = FIRST_ITEM_SIZE;
    AceType::DynamicCast<TabsLayoutProperty>(layoutWrapper.GetLayoutProperty())
        ->UpdateLayoutConstraint(layoutConstrain);
    layoutWrapper.SetLayoutAlgorithm(AceType::MakeRefPtr<LayoutAlgorithmWrapper>(tabsLayoutAlgorithm));
    layoutWrapper.GetLayoutProperty()->UpdateContentConstraint();
    auto childLayoutConstraint = layoutWrapper.GetLayoutProperty()->CreateChildConstraint();
    childLayoutConstraint.maxSize = SizeF(FIRST_ITEM_SIZE);
    childLayoutConstraint.minSize = SizeF(0.0f, 0.0f);

    auto swiperNode = AceType::DynamicCast<FrameNode>(tabsFrameNode->GetChildAtIndex(TEST_SWIPER_INDEX));
    ASSERT_NE(swiperNode, nullptr);
    RefPtr<GeometryNode> swiperGeometryNode = AceType::MakeRefPtr<GeometryNode>();
    RefPtr<LayoutWrapper> swiperLayoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(swiperNode, swiperGeometryNode, swiperNode->GetLayoutProperty());
    
    auto swipeLayoutAlgorithm = swiperNode->GetPattern<Pattern>()->CreateLayoutAlgorithm();
    ASSERT_NE(swipeLayoutAlgorithm, nullptr);
    swiperLayoutWrapper->SetLayoutAlgorithm(
        AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(swipeLayoutAlgorithm));

    swiperLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
    swiperLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(SWIPER_WIDTH), CalcLength(FIRST_ITEM_WIDTH)));
    layoutWrapper.AppendChild(swiperLayoutWrapper);
    auto dividerNode = AceType::DynamicCast<FrameNode>(tabsFrameNode->GetChildAtIndex(TEST_DIVIDER_INDEX));
    ASSERT_NE(dividerNode, nullptr);
    RefPtr<GeometryNode> dividerGeometryNode = AceType::MakeRefPtr<GeometryNode>();
    RefPtr<LayoutWrapper> dividerLayoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(dividerNode, dividerGeometryNode, dividerNode->GetLayoutProperty());
    
    auto dividerLayoutAlgorithm = dividerNode->GetPattern<Pattern>()->CreateLayoutAlgorithm();
    ASSERT_NE(dividerLayoutAlgorithm, nullptr);
    dividerLayoutWrapper->SetLayoutAlgorithm(
        AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(dividerLayoutAlgorithm));

    dividerLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
    dividerLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(0.0f), CalcLength(FIRST_ITEM_WIDTH)));
    layoutWrapper.AppendChild(dividerLayoutWrapper);
    auto tabBarNode = AceType::DynamicCast<FrameNode>(tabsFrameNode->GetChildAtIndex(TEST_TAB_BAR_INDEX));
    ASSERT_NE(tabBarNode, nullptr);
    RefPtr<GeometryNode> tabbarGeometryNode = AceType::MakeRefPtr<GeometryNode>();
    RefPtr<LayoutWrapper> tabBarLayoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(tabBarNode, tabbarGeometryNode, tabBarNode->GetLayoutProperty());

    auto tabbarLayoutAlgorithm = tabBarNode->GetPattern<Pattern>()->CreateLayoutAlgorithm();
    ASSERT_NE(tabbarLayoutAlgorithm, nullptr);
    tabBarLayoutWrapper->SetLayoutAlgorithm(
        AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(tabbarLayoutAlgorithm));

    tabBarLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
    tabBarLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(TABBAR_WIDTH), CalcLength(FIRST_ITEM_WIDTH)));
    layoutWrapper.AppendChild(tabBarLayoutWrapper);
    
    tabsLayoutAlgorithm->Measure(&layoutWrapper);
    tabsLayoutAlgorithm->Layout(&layoutWrapper);

    auto tabbarOffset = tabBarLayoutWrapper->GetGeometryNode()->GetMarginFrameOffset();
    EXPECT_EQ(tabbarOffset.GetX(), 0.0f);
    EXPECT_EQ(tabbarOffset.GetY(), 0.0f);

    auto tabBarSize = tabBarLayoutWrapper->GetGeometryNode()->GetFrameSize();
    EXPECT_EQ(tabBarSize.Width(), TABBAR_WIDTH);
    EXPECT_EQ(tabBarSize.Height(), FIRST_ITEM_WIDTH);

    auto swiperOffset = swiperLayoutWrapper->GetGeometryNode()->GetMarginFrameOffset();
    EXPECT_EQ(swiperOffset.GetX(), TABBAR_WIDTH);
    EXPECT_EQ(swiperOffset.GetY(), 0.0f);

    auto swiperSize = swiperLayoutWrapper->GetGeometryNode()->GetFrameSize();
    EXPECT_EQ(swiperSize.Width(), SWIPER_WIDTH);
    EXPECT_EQ(swiperSize.Height(), FIRST_ITEM_WIDTH);

    tabsFrameNode->GetLayoutProperty<TabsLayoutProperty>()->UpdateTabBarPosition(BarPosition::END);
    tabsLayoutAlgorithm->Layout(&layoutWrapper);

    tabbarOffset = tabBarLayoutWrapper->GetGeometryNode()->GetMarginFrameOffset();
    EXPECT_EQ(tabbarOffset.GetX(), SWIPER_WIDTH);
    EXPECT_EQ(tabbarOffset.GetY(), 0.0f);

    swiperOffset = swiperLayoutWrapper->GetGeometryNode()->GetMarginFrameOffset();
    EXPECT_EQ(swiperOffset.GetX(), 0.0f);
    EXPECT_EQ(swiperOffset.GetY(), 0.0f);
}

/**
 * @tc.name: TabsModelMeasure004
 * @tc.desc: Test Tabs Measure and Layout when the ideaSize is infinity.
 * @tc.type: FUNC
 */
HWTEST_F(TabsTestNg, TabsModelMeasure004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize all properties of tabs.
     */
    MockPipelineContextGetTheme();

    TabsModelNG instance;
    instance.Create(BarPosition::START, 1, nullptr, nullptr);
    instance.SetIsVertical(true);

    /**
     * @tc.steps: step2. Get tabs pattern to create layoutAlgorithm, and call measure and layout functions.
     * @tc.expected: related function is called.
     */
    auto tabsFrameNode = AceType::DynamicCast<TabsNode>(ViewStackProcessor::GetInstance()->GetMainElementNode());
    ASSERT_NE(tabsFrameNode, nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    auto pattern = tabsFrameNode->GetPattern<TabsPattern>();
    ASSERT_NE(pattern, nullptr);
    auto tabsLayoutAlgorithm = pattern->CreateLayoutAlgorithm();
    ASSERT_NE(tabsLayoutAlgorithm, nullptr);
    LayoutWrapper layoutWrapper = LayoutWrapper(tabsFrameNode, geometryNode, tabsFrameNode->GetLayoutProperty());
    LayoutConstraintF layoutConstrain;
    layoutConstrain.selfIdealSize.SetSize(SizeF(FIRST_ITEM_WIDTH, INFINITY_NUM));
    AceType::DynamicCast<TabsLayoutProperty>(layoutWrapper.GetLayoutProperty())
        ->UpdateLayoutConstraint(layoutConstrain);
    layoutWrapper.SetLayoutAlgorithm(AceType::MakeRefPtr<LayoutAlgorithmWrapper>(tabsLayoutAlgorithm));
    layoutWrapper.GetLayoutProperty()->UpdateContentConstraint();
    tabsLayoutAlgorithm->Measure(&layoutWrapper);
    tabsLayoutAlgorithm->Layout(&layoutWrapper);

    auto tabsSize = layoutWrapper.GetGeometryNode()->GetFrameSize();
    EXPECT_EQ(tabsSize.Width(), 0.0f);
    EXPECT_EQ(tabsSize.Height(), 0.0f);

    layoutConstrain.selfIdealSize.SetSize(SizeF(INFINITY_NUM, FIRST_ITEM_HEIGHT));
    AceType::DynamicCast<TabsLayoutProperty>(layoutWrapper.GetLayoutProperty())
        ->UpdateLayoutConstraint(layoutConstrain);
    layoutWrapper.SetLayoutAlgorithm(AceType::MakeRefPtr<LayoutAlgorithmWrapper>(tabsLayoutAlgorithm));

    tabsLayoutAlgorithm->Measure(&layoutWrapper);
    tabsLayoutAlgorithm->Layout(&layoutWrapper);

    tabsSize = layoutWrapper.GetGeometryNode()->GetFrameSize();
    EXPECT_EQ(tabsSize.Width(), 0.0f);
    EXPECT_EQ(tabsSize.Height(), 0.0f);

    layoutConstrain.selfIdealSize.SetSize(NEGTIVE_SIZE);
    AceType::DynamicCast<TabsLayoutProperty>(layoutWrapper.GetLayoutProperty())
        ->UpdateLayoutConstraint(layoutConstrain);
    layoutWrapper.SetLayoutAlgorithm(AceType::MakeRefPtr<LayoutAlgorithmWrapper>(tabsLayoutAlgorithm));

    tabsLayoutAlgorithm->Measure(&layoutWrapper);
    tabsLayoutAlgorithm->Layout(&layoutWrapper);

    auto tabsOffset = layoutWrapper.GetGeometryNode()->GetMarginFrameOffset();
    EXPECT_EQ(tabsOffset.GetX(), 0.0f);
    EXPECT_EQ(tabsOffset.GetY(), 0.0f);

    layoutConstrain.selfIdealSize.SetSize(FIRST_ITEM_SIZE);
    AceType::DynamicCast<TabsLayoutProperty>(layoutWrapper.GetLayoutProperty())
        ->UpdateLayoutConstraint(layoutConstrain);
    layoutWrapper.SetLayoutAlgorithm(AceType::MakeRefPtr<LayoutAlgorithmWrapper>(tabsLayoutAlgorithm));

    auto childLayoutConstraint = layoutWrapper.GetLayoutProperty()->CreateChildConstraint();
    childLayoutConstraint.maxSize = SizeF(FIRST_ITEM_SIZE);
    childLayoutConstraint.minSize = SizeF(0.0f, 0.0f);

    auto tabBarNode = AceType::DynamicCast<FrameNode>(tabsFrameNode->GetChildAtIndex(0));
    ASSERT_NE(tabBarNode, nullptr);
    RefPtr<GeometryNode> tabbarGeometryNode = AceType::MakeRefPtr<GeometryNode>();
    RefPtr<LayoutWrapper> tabBarLayoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(tabBarNode, tabbarGeometryNode, tabBarNode->GetLayoutProperty());

    auto tabbarLayoutAlgorithm = tabBarNode->GetPattern<Pattern>()->CreateLayoutAlgorithm();
    ASSERT_NE(tabbarLayoutAlgorithm, nullptr);
    tabBarLayoutWrapper->SetLayoutAlgorithm(
        AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(tabbarLayoutAlgorithm));

    tabBarLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
    tabBarLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(TABBAR_WIDTH), CalcLength(FIRST_ITEM_HEIGHT)));
    layoutWrapper.AppendChild(tabBarLayoutWrapper);

    tabsLayoutAlgorithm->Measure(&layoutWrapper);
    tabsLayoutAlgorithm->Layout(&layoutWrapper);

    tabsOffset = layoutWrapper.GetGeometryNode()->GetMarginFrameOffset();
    EXPECT_EQ(tabsOffset.GetX(), 0.0f);
    EXPECT_EQ(tabsOffset.GetY(), 0.0f);

    auto dividerNode = AceType::DynamicCast<FrameNode>(tabsFrameNode->GetChildAtIndex(1));
    ASSERT_NE(dividerNode, nullptr);
    RefPtr<GeometryNode> dividerGeometryNode = AceType::MakeRefPtr<GeometryNode>();
    RefPtr<LayoutWrapper> dividerLayoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(dividerNode, dividerGeometryNode, dividerNode->GetLayoutProperty());
    
    auto dividerLayoutAlgorithm = dividerNode->GetPattern<Pattern>()->CreateLayoutAlgorithm();
    ASSERT_NE(dividerLayoutAlgorithm, nullptr);
    dividerLayoutWrapper->SetLayoutAlgorithm(
        AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(dividerLayoutAlgorithm));

    dividerLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
    dividerLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(0.0f), CalcLength(FIRST_ITEM_HEIGHT)));
    layoutWrapper.AppendChild(dividerLayoutWrapper);

    tabsLayoutAlgorithm->Measure(&layoutWrapper);
    tabsLayoutAlgorithm->Layout(&layoutWrapper);

    tabsOffset = layoutWrapper.GetGeometryNode()->GetMarginFrameOffset();
    EXPECT_EQ(tabsOffset.GetX(), 0.0f);
    EXPECT_EQ(tabsOffset.GetY(), 0.0f);
}

/**
 * @tc.name: TabsModelOnUpdateShowDivider001
 * @tc.desc: Test tabs OnUpdateShowDivider.
 * @tc.type: FUNC
 */
HWTEST_F(TabsTestNg, TabsModelOnUpdateShowDivider001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize all properties of tabs.
     */
    MockPipelineContextGetTheme();

    TabsModelNG instance;
    instance.Create(BarPosition::START, 1, nullptr, nullptr);
    Color color = Color::RED;
    TabsItemDivider divider;
    divider.color = color;
    instance.SetDivider(divider);

    /**
     * @tc.steps: step2. Get tabs pattern to create layoutAlgorithm, and call measure and layout functions.
     * @tc.expected: step2. related function is called.
     */
    auto tabsFrameNode = AceType::DynamicCast<TabsNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(tabsFrameNode, nullptr);

    auto pattern = tabsFrameNode->GetPattern<TabsPattern>();
    pattern->OnUpdateShowDivider();

    auto dividerNode = tabsFrameNode->GetChildAtIndex(1);
    ASSERT_NE(dividerNode, nullptr);
    auto dividerFrameNode = AceType::DynamicCast<FrameNode>(dividerNode);

    auto dividerRenderProperty = dividerFrameNode->GetPaintProperty<DividerRenderProperty>();
    ASSERT_NE(dividerRenderProperty, nullptr);
    auto dividerColor = dividerRenderProperty->GetDividerColor();
    EXPECT_EQ(dividerColor.value().ColorToString(), color.ColorToString());
}

/**
 * @tc.name: TabsModelOnUpdateShowDivider002
 * @tc.desc: Test tabs OnUpdateShowDivider when pattern without host
 * @tc.type: FUNC
 */
HWTEST_F(TabsTestNg, TabsModelOnUpdateShowDivider002, TestSize.Level2)
{
    auto pattern = AceType::MakeRefPtr<TabsPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->OnUpdateShowDivider();
    EXPECT_EQ(pattern->GetHostFrameSize(), std::nullopt);
}

/**
 * @tc.name: TabsModelOnUpdateShowDivider003
 * @tc.desc: Test tabs OnUpdateShowDivider when pattern without host
 * @tc.type: FUNC
 */
HWTEST_F(TabsTestNg, TabsModelOnUpdateShowDivider003, TestSize.Level2)
{
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto tabsNode =
        TabsModelNG::GetOrCreateTabsNode(V2::TABS_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<TabsPattern>(); });
    auto pattern = tabsNode->GetPattern<TabsPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->OnUpdateShowDivider();
    EXPECT_LT(tabsNode->GetChildren().size(), static_cast<std::size_t>(2));
}

/**
 * @tc.name: TabsModelOnUpdateShowDivider004
 * @tc.desc: Test tabs OnUpdateShowDivider when pattern without host
 * @tc.type: FUNC
 */
HWTEST_F(TabsTestNg, TabsModelOnUpdateShowDivider004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize all properties of tabs.
     */
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto tabsNode =
        TabsModelNG::GetOrCreateTabsNode(V2::TABS_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<TabsPattern>(); });
    ASSERT_NE(tabsNode, nullptr);

    bool hasTabBarNode = tabsNode->HasTabBarNode();
    auto controller = AceType::MakeRefPtr<SwiperController>();
    auto tabBarId = tabsNode->GetTabBarId();
    auto tabBarNode = FrameNode::GetOrCreateFrameNode(
        V2::TAB_BAR_ETS_TAG, tabBarId, [controller]() { return AceType::MakeRefPtr<TabBarPattern>(controller); });
    ASSERT_NE(tabBarNode, nullptr);

    if (!hasTabBarNode) {
        tabBarNode->MountToParent(tabsNode);
    }

    bool hasSwiperNode = tabsNode->HasSwiperNode();
    auto swiperId = tabsNode->GetSwiperId();
    auto swiperNode = FrameNode::GetOrCreateFrameNode(
        V2::SWIPER_ETS_TAG, swiperId, []() { return AceType::MakeRefPtr<SwiperPattern>(); });
    ASSERT_NE(swiperNode, nullptr);

    if (!hasSwiperNode) {
        swiperNode->MountToParent(tabsNode);
    }

    auto pattern = tabsNode->GetPattern<TabsPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->OnUpdateShowDivider();
    auto childNode = tabsNode->GetChildAtIndex(1);
    ASSERT_NE(childNode, nullptr);
    EXPECT_NE(childNode->GetTag(), V2::DIVIDER_ETS_TAG);
}

/**
 * @tc.name: TabsModelToJsonValue001
 * @tc.desc: Test tabs ToJsonValue when host has not child.
 * @tc.type: FUNC
 */
HWTEST_F(TabsTestNg, TabsModelToJsonValue001, TestSize.Level2)
{
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto tabsNode =
        TabsModelNG::GetOrCreateTabsNode(V2::TABS_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<TabsPattern>(); });
    ASSERT_NE(tabsNode, nullptr);
    std::unique_ptr<JsonValue> json = std::make_unique<JsonValue>();
    tabsNode->ToJsonValue(json);
    ASSERT_NE(json, nullptr);
    auto dividerJson = json->GetValue("divider");
    ASSERT_NE(dividerJson, nullptr);
    EXPECT_TRUE(dividerJson->IsNull());
}

/**
 * @tc.name: TabContentModelSetIndicator001
 * @tc.desc: test SetIndicator
 * @tc.type: FUNC
 */
HWTEST_F(TabsTestNg, TabContentModelSetIndicator001, TestSize.Level1)
{
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    ASSERT_NE(themeManager, nullptr);

    MockPipelineBase::GetCurrent()->SetThemeManager(themeManager);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(AceType::MakeRefPtr<TabTheme>()));

    TabContentModelNG tabContentModel;
    IndicatorStyle indicator;
    Color color = Color::BLACK;
    Dimension height = 10.0_vp;
    Dimension width = 20.0_vp;
    Dimension borderRadius = 2.0_vp;
    Dimension marginTop = 3.0_vp;

    indicator.color = color;
    indicator.height = height;
    indicator.width = width;
    indicator.borderRadius = borderRadius;
    indicator.marginTop = marginTop;

    tabContentModel.Create();
    tabContentModel.SetIndicator(indicator);
    auto tabContentFrameNode = AceType::DynamicCast<TabContentNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(tabContentFrameNode, nullptr);
    EXPECT_EQ(tabContentFrameNode->GetTag(), V2::TAB_CONTENT_ITEM_ETS_TAG);
    auto tabContentPattern = tabContentFrameNode->GetPattern<TabContentPattern>();
    ASSERT_NE(tabContentPattern, nullptr);

    EXPECT_EQ(tabContentPattern->GetIndicatorStyle().color.ColorToString(), color.ColorToString());
    EXPECT_EQ(tabContentPattern->GetIndicatorStyle().height.ToString(), height.ToString());
    EXPECT_EQ(tabContentPattern->GetIndicatorStyle().width.ToString(), width.ToString());
    EXPECT_EQ(tabContentPattern->GetIndicatorStyle().borderRadius.ToString(), borderRadius.ToString());
    EXPECT_EQ(tabContentPattern->GetIndicatorStyle().marginTop.ToString(), marginTop.ToString());
}

/**
 * @tc.name: TabContentModelSetIndicator002
 * @tc.desc: test SetIndicator with getTheme once
 * @tc.type: FUNC
 */
HWTEST_F(TabsTestNg, TabContentModelSetIndicator002, TestSize.Level1)
{
    MockPipelineContextGetTheme();

    TabContentModelNG tabContentModel;
    IndicatorStyle indicator;
    Dimension width = 0.0_vp;
    Dimension borderRadius = 0.0_vp;

    tabContentModel.Create();
    tabContentModel.SetIndicator(indicator);
    auto tabContentFrameNode = AceType::DynamicCast<TabContentNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(tabContentFrameNode, nullptr);
    EXPECT_EQ(tabContentFrameNode->GetTag(), V2::TAB_CONTENT_ITEM_ETS_TAG);
    auto tabContentPattern = tabContentFrameNode->GetPattern<TabContentPattern>();
    ASSERT_NE(tabContentPattern, nullptr);

    EXPECT_EQ(tabContentPattern->GetIndicatorStyle().width.ToString(), width.ToString());
    EXPECT_EQ(tabContentPattern->GetIndicatorStyle().borderRadius.ToString(), borderRadius.ToString());
}

/**
 * @tc.name: TabContentModelSetSubTabBorderRadius001
 * @tc.desc: test SetSubTabBorderRadius
 * @tc.type: FUNC
 */
HWTEST_F(TabsTestNg, TabContentModelSetSubTabBorderRadius001, TestSize.Level1)
{
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    ASSERT_NE(themeManager, nullptr);

    MockPipelineBase::GetCurrent()->SetThemeManager(themeManager);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(AceType::MakeRefPtr<TabTheme>()));
    TabContentModelNG tabContentModel;
    BoardStyle boardStyle;
    boardStyle.borderRadius = 10.0_vp;
    tabContentModel.Create();
    tabContentModel.SetBoard(boardStyle);
    auto tabContentFrameNode = AceType::DynamicCast<TabContentNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(tabContentFrameNode, nullptr);
    EXPECT_EQ(tabContentFrameNode->GetTag(), V2::TAB_CONTENT_ITEM_ETS_TAG);
    auto tabContentPattern = tabContentFrameNode->GetPattern<TabContentPattern>();
    ASSERT_NE(tabContentPattern, nullptr);
    auto board = tabContentPattern->GetBoardStyle();
    EXPECT_EQ(board.borderRadius.ToString(), boardStyle.borderRadius.ToString());
}

/**
 * @tc.name: TabContentModelSetSubTabBorderRadius002
 * @tc.desc: test SetSubTabBorderRadius with getTheme once
 * @tc.type: FUNC
 */
HWTEST_F(TabsTestNg, TabContentModelSetSubTabBorderRadius002, TestSize.Level1)
{
    MockPipelineContextGetTheme();
    TabContentModelNG tabContentModel;
    BoardStyle boardStyle;
    boardStyle.borderRadius = 10.0_vp;
    tabContentModel.Create();
    tabContentModel.SetBoard(boardStyle);
    auto tabContentFrameNode = AceType::DynamicCast<TabContentNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(tabContentFrameNode, nullptr);
    EXPECT_EQ(tabContentFrameNode->GetTag(), V2::TAB_CONTENT_ITEM_ETS_TAG);
    auto tabContentPattern = tabContentFrameNode->GetPattern<TabContentPattern>();
    ASSERT_NE(tabContentPattern, nullptr);
    auto board = tabContentPattern->GetBoardStyle();
    EXPECT_EQ(board.borderRadius.ToString(), boardStyle.borderRadius.ToString());
}

/**
 * @tc.name: TabContentModelSetSelectedMode001
 * @tc.desc: test SetSelectedMode
 * @tc.type: FUNC
 */
HWTEST_F(TabsTestNg, TabContentModelSetSelectedMode001, TestSize.Level1)
{
    MockPipelineContextGetTheme();
    TabContentModelNG tabContentModel;
    SelectedMode selectedMode = SelectedMode::INDICATOR;
    tabContentModel.Create();
    tabContentModel.SetSelectedMode(selectedMode);
    auto tabContentFrameNode = AceType::DynamicCast<TabContentNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(tabContentFrameNode, nullptr);
    EXPECT_EQ(tabContentFrameNode->GetTag(), V2::TAB_CONTENT_ITEM_ETS_TAG);
    auto tabContentPattern = tabContentFrameNode->GetPattern<TabContentPattern>();
    ASSERT_NE(tabContentPattern, nullptr);
    EXPECT_EQ(tabContentPattern->GetSelectedMode(), selectedMode);
}

/**
 * @tc.name: TabContentModelSetLabelStyle001
 * @tc.desc: test SetLabelStyle
 * @tc.type: FUNC
 */
HWTEST_F(TabsTestNg, TabContentModelSetLabelStyle001, TestSize.Level1)
{
    MockPipelineContextGetTheme();

    TabContentModelNG tabContentModel;
    LabelStyle labelStyle;
    tabContentModel.Create();
    tabContentModel.SetLabelStyle(labelStyle);
    auto tabContentFrameNode = AceType::DynamicCast<TabContentNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(tabContentFrameNode, nullptr);
    EXPECT_EQ(tabContentFrameNode->GetTag(), V2::TAB_CONTENT_ITEM_ETS_TAG);
    auto tabContentPattern = tabContentFrameNode->GetPattern<TabContentPattern>();
    ASSERT_NE(tabContentPattern, nullptr);
    EXPECT_EQ(tabContentPattern->GetLabelStyle().textOverflow, std::nullopt);
    EXPECT_EQ(tabContentPattern->GetLabelStyle().maxLines, std::nullopt);
    EXPECT_EQ(tabContentPattern->GetLabelStyle().minFontSize, std::nullopt);
    EXPECT_EQ(tabContentPattern->GetLabelStyle().maxFontSize, std::nullopt);
    EXPECT_EQ(tabContentPattern->GetLabelStyle().heightAdaptivePolicy, std::nullopt);
    EXPECT_EQ(tabContentPattern->GetLabelStyle().fontSize, std::nullopt);
    EXPECT_EQ(tabContentPattern->GetLabelStyle().fontWeight, std::nullopt);
    EXPECT_EQ(tabContentPattern->GetLabelStyle().fontFamily, std::nullopt);
    EXPECT_EQ(tabContentPattern->GetLabelStyle().fontStyle, std::nullopt);
}

/**
 * @tc.name: TabContentModelSetLabelStyle002
 * @tc.desc: test SetLabelStyle
 * @tc.type: FUNC
 */
HWTEST_F(TabsTestNg, TabContentModelSetLabelStyle002, TestSize.Level1)
{
    MockPipelineContextGetTheme();

    TabContentModelNG tabContentModel;
    LabelStyle labelStyle;
    labelStyle.textOverflow = TextOverflow::CLIP;
    labelStyle.maxLines = 0;
    labelStyle.minFontSize = 0.0_vp;
    labelStyle.maxFontSize = 0.0_vp;
    labelStyle.heightAdaptivePolicy = TextHeightAdaptivePolicy::MAX_LINES_FIRST;
    labelStyle.fontSize = 0.0_vp;
    labelStyle.fontWeight = FontWeight::NORMAL;
    labelStyle.fontFamily = { "unknown" };
    std::vector<std::string> fontVect = { "" };
    labelStyle.fontStyle = Ace::FontStyle::NORMAL;
    tabContentModel.Create();
    tabContentModel.SetLabelStyle(labelStyle);
    auto tabContentFrameNode = AceType::DynamicCast<TabContentNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(tabContentFrameNode, nullptr);
    EXPECT_EQ(tabContentFrameNode->GetTag(), V2::TAB_CONTENT_ITEM_ETS_TAG);
    auto tabContentPattern = tabContentFrameNode->GetPattern<TabContentPattern>();
    ASSERT_NE(tabContentPattern, nullptr);
    EXPECT_EQ(tabContentPattern->GetLabelStyle().textOverflow, labelStyle.textOverflow);
    EXPECT_EQ(tabContentPattern->GetLabelStyle().maxLines, labelStyle.maxLines);
    EXPECT_EQ(tabContentPattern->GetLabelStyle().minFontSize->ToString(), labelStyle.minFontSize->ToString());
    EXPECT_EQ(tabContentPattern->GetLabelStyle().maxFontSize->ToString(), labelStyle.maxFontSize->ToString());
    EXPECT_EQ(tabContentPattern->GetLabelStyle().heightAdaptivePolicy, labelStyle.heightAdaptivePolicy);
    EXPECT_EQ(tabContentPattern->GetLabelStyle().fontSize->ToString(), labelStyle.fontSize->ToString());
    EXPECT_EQ(tabContentPattern->GetLabelStyle().fontWeight, labelStyle.fontWeight);
    EXPECT_EQ(tabContentPattern->GetLabelStyle().fontFamily, labelStyle.fontFamily);
    EXPECT_EQ(tabContentPattern->GetLabelStyle().fontStyle, labelStyle.fontStyle);
}

/**
 * @tc.name: TabContentModelUpdateLabelStyle001
 * @tc.desc: test UpdateLabelStyle
 * @tc.type: FUNC
 */
HWTEST_F(TabsTestNg, TabContentModelUpdateLabelStyle001, TestSize.Level1)
{
    MockPipelineContextGetTheme();
    auto layoutProperty = AceType::MakeRefPtr<TextLayoutProperty>();

    TabContentModelNG tabContentModel;
    LabelStyle labelStyle;
    labelStyle.textOverflow = TextOverflow::CLIP;
    labelStyle.maxLines = 0;
    labelStyle.minFontSize = 0.0_vp;
    labelStyle.maxFontSize = 0.0_vp;
    labelStyle.heightAdaptivePolicy = TextHeightAdaptivePolicy::MAX_LINES_FIRST;
    labelStyle.fontSize = 0.0_vp;
    labelStyle.fontWeight = FontWeight::NORMAL;
    labelStyle.fontFamily = { "unknown" };
    std::vector<std::string> fontVect = { "" };
    labelStyle.fontStyle = Ace::FontStyle::NORMAL;
    tabContentModel.Create();
    tabContentModel.UpdateLabelStyle(labelStyle, layoutProperty);
    EXPECT_EQ(layoutProperty->GetTextOverflow(), labelStyle.textOverflow);
    EXPECT_EQ(layoutProperty->GetMaxLines(), labelStyle.maxLines);
    EXPECT_EQ(layoutProperty->GetAdaptMinFontSize()->ToString(), labelStyle.minFontSize->ToString());
    EXPECT_EQ(layoutProperty->GetAdaptMaxFontSize()->ToString(), labelStyle.maxFontSize->ToString());
    EXPECT_EQ(layoutProperty->GetHeightAdaptivePolicy(), labelStyle.heightAdaptivePolicy);
    EXPECT_EQ(layoutProperty->GetFontSize()->ToString(), labelStyle.fontSize->ToString());
    EXPECT_EQ(layoutProperty->GetFontWeight(), labelStyle.fontWeight);
    EXPECT_EQ(layoutProperty->GetFontFamily(), labelStyle.fontFamily);
    EXPECT_EQ(layoutProperty->GetItalicFontStyle(), labelStyle.fontStyle);
}

/**
 * @tc.name: TabContentModelToJsonValue001
 * @tc.desc: test SetIndicator
 * @tc.type: FUNC
 */
HWTEST_F(TabsTestNg, TabContentModelToJsonValue001, TestSize.Level1)
{
    MockPipelineContextGetTheme();

    LabelStyle labelStyle;
    labelStyle.textOverflow = TextOverflow::CLIP;
    labelStyle.maxLines = 0;
    labelStyle.minFontSize = 0.0_vp;
    labelStyle.maxFontSize = 0.0_vp;
    labelStyle.heightAdaptivePolicy = TextHeightAdaptivePolicy::MAX_LINES_FIRST;
    labelStyle.fontSize = 0.0_vp;
    labelStyle.fontWeight = FontWeight::NORMAL;
    labelStyle.fontFamily = { "unknown", "unknow2" };

    TabContentModelNG tabContentModel;
    tabContentModel.Create();
    tabContentModel.SetLabelStyle(labelStyle);
    auto tabContentFrameNode = AceType::DynamicCast<TabContentNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(tabContentFrameNode, nullptr);
    EXPECT_EQ(tabContentFrameNode->GetTag(), V2::TAB_CONTENT_ITEM_ETS_TAG);
    std::unique_ptr<JsonValue> json = std::make_unique<JsonValue>();
    tabContentFrameNode->ToJsonValue(json);
    EXPECT_NE(json, nullptr);
}

/**
 * @tc.name: TabContentModelAddTabBarItem001
 * @tc.desc: test AddTabBarItem
 * @tc.type: FUNC
 */
HWTEST_F(TabsTestNg, TabContentModelAddTabBarItem001, TestSize.Level1)
{
    MockPipelineContextGetTheme();
    const std::string text_test = "text_test";
    TabContentModelNG tabContentModel;
    SelectedMode selectedMode = SelectedMode::INDICATOR;
    tabContentModel.Create();
    tabContentModel.SetSelectedMode(selectedMode);
    auto tabContentFrameNode = AceType::DynamicCast<TabContentNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(tabContentFrameNode, nullptr);
    EXPECT_EQ(tabContentFrameNode->GetTag(), V2::TAB_CONTENT_ITEM_ETS_TAG);
    TabsModelNG tabsModel;
    tabsModel.Create(BarPosition::START, 1, nullptr, nullptr);
    auto tabsNode = AceType::DynamicCast<TabsNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(tabsNode, nullptr);
    auto tabContentPattern = tabContentFrameNode->GetPattern<TabContentPattern>();
    ASSERT_NE(tabContentPattern, nullptr);
    tabContentFrameNode->GetTabBarItemId();
    auto swiperNode = AceType::DynamicCast<FrameNode>(tabsNode->GetChildAtIndex(TEST_SWIPER_INDEX));
    tabContentFrameNode->MountToParent(swiperNode);
    tabContentPattern->SetTabBar(text_test, "", nullptr);
    EXPECT_EQ(tabContentPattern->GetTabBarParam().GetText(), text_test);
    tabContentModel.AddTabBarItem(tabContentFrameNode, DEFAULT_NODE_SLOT, true);
    EXPECT_EQ(tabContentPattern->GetSelectedMode(), selectedMode);

    TabContentModelNG tabContentModel2;
    tabContentModel2.Create();
    tabContentModel2.SetSelectedMode(selectedMode);
    auto tabContentFrameNode2 = AceType::DynamicCast<TabContentNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(tabContentFrameNode2, nullptr);
    EXPECT_EQ(tabContentFrameNode2->GetTag(), V2::TAB_CONTENT_ITEM_ETS_TAG);;
    auto tabContentPattern2 = tabContentFrameNode2->GetPattern<TabContentPattern>();
    ASSERT_NE(tabContentPattern2, nullptr);
    tabContentFrameNode2->MountToParent(swiperNode);
    tabContentPattern2->SetTabBar(text_test, "", nullptr);
    EXPECT_EQ(tabContentPattern2->GetTabBarParam().GetText(), text_test);
    tabContentModel.AddTabBarItem(tabContentFrameNode2, DEFAULT_NODE_SLOT, true);
}

/**
 * @tc.name: TabBarPatternOnDirtyLayoutWrapperSwap001
 * @tc.desc: test OnDirtyLayoutWrapperSwap
 * @tc.type: FUNC
 */
HWTEST_F(TabsTestNg, TabBarPatternOnDirtyLayoutWrapperSwap001, TestSize.Level1)
{
    MockPipelineContextGetTheme();
    TabsModelNG tabsModel;
    bool fadingEdge = true;
    tabsModel.Create(BarPosition::START, 1, nullptr, nullptr);
    tabsModel.SetFadingEdge(fadingEdge);
    auto tabsFrameNode = AceType::DynamicCast<TabsNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(tabsFrameNode, nullptr);
    EXPECT_EQ(tabsFrameNode->GetTag(), V2::TABS_ETS_TAG);
    auto tabBarNode = AceType::DynamicCast<FrameNode>(tabsFrameNode->GetChildAtIndex(TEST_TAB_BAR_INDEX));
    ASSERT_NE(tabBarNode, nullptr);
    EXPECT_EQ(tabBarNode->GetTag(), V2::TAB_BAR_ETS_TAG);

    auto tabBarPattern = tabBarNode->GetPattern<TabBarPattern>();
    ASSERT_NE(tabBarPattern, nullptr);
    auto tabBarLayoutProperty = tabBarNode->GetLayoutProperty<TabBarLayoutProperty>();
    ASSERT_NE(tabBarLayoutProperty, nullptr);
    auto layoutAlgorithm = tabBarPattern->CreateLayoutAlgorithm();
    ASSERT_NE(layoutAlgorithm, nullptr);
    DirtySwapConfig config;
    auto layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(tabBarNode, tabBarNode->GetGeometryNode(), tabBarLayoutProperty);
    ASSERT_NE(layoutWrapper, nullptr);
    auto algorithmWrapper = AceType::MakeRefPtr<LayoutAlgorithmWrapper>(layoutAlgorithm, false, false);
    ASSERT_NE(algorithmWrapper, nullptr);

    layoutWrapper->SetLayoutAlgorithm(algorithmWrapper);
    EXPECT_EQ(layoutWrapper->layoutAlgorithm_, algorithmWrapper);
    ASSERT_NE(tabBarPattern, nullptr);
    tabBarPattern->SetTabBarStyle(TabBarStyle::SUBTABBATSTYLE, 0);
    EXPECT_EQ(tabBarPattern->tabBarStyles_[0], TabBarStyle::SUBTABBATSTYLE);
    tabBarPattern->OnDirtyLayoutWrapperSwap(layoutWrapper, config);
    EXPECT_EQ(tabBarPattern->indicator_, 0);
}

/**
 * @tc.name: TabBarPatternUpdateSubTabBoard001
 * @tc.desc: test UpdateSubTabBoard
 * @tc.type: FUNC
 */
HWTEST_F(TabsTestNg, TabBarPatternUpdateSubTabBoard001, TestSize.Level1)
{
    MockPipelineContextGetTheme();
    const std::string text_test = "text_test";

    TabContentModelNG tabContentModel;

    SelectedMode selectedMode = SelectedMode::INDICATOR;
    tabContentModel.Create();
    tabContentModel.SetSelectedMode(selectedMode);
    auto tabContentFrameNode = AceType::DynamicCast<TabContentNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(tabContentFrameNode, nullptr);
    EXPECT_EQ(tabContentFrameNode->GetTag(), V2::TAB_CONTENT_ITEM_ETS_TAG);

    TabsModelNG tabsModel;
    tabsModel.Create(BarPosition::START, 1, nullptr, nullptr);
    auto tabsNode = AceType::DynamicCast<TabsNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(tabsNode, nullptr);

    auto tabBarNode = AceType::DynamicCast<FrameNode>(tabsNode->GetChildAtIndex(TEST_TAB_BAR_INDEX));
    ASSERT_NE(tabBarNode, nullptr);
    EXPECT_EQ(tabBarNode->GetTag(), V2::TAB_BAR_ETS_TAG);

    auto tabBarPattern = tabBarNode->GetPattern<TabBarPattern>();
    ASSERT_NE(tabBarPattern, nullptr);
    tabBarPattern->UpdateSubTabBoard();

    auto tabContentPattern = tabContentFrameNode->GetPattern<TabContentPattern>();
    ASSERT_NE(tabContentPattern, nullptr);
    EXPECT_EQ(tabContentPattern->GetSelectedMode(), selectedMode);
    tabContentFrameNode->GetTabBarItemId();

    auto swiperNode = AceType::DynamicCast<FrameNode>(tabsNode->GetChildAtIndex(TEST_SWIPER_INDEX));
    ASSERT_NE(swiperNode, nullptr);
    EXPECT_EQ(swiperNode->GetTag(), V2::SWIPER_ETS_TAG);
    tabContentFrameNode->MountToParent(swiperNode);
    tabContentPattern->SetTabBar(text_test, "", nullptr);
    EXPECT_EQ(tabContentPattern->GetTabBarParam().GetText(), text_test);

    tabContentModel.AddTabBarItem(tabContentFrameNode, DEFAULT_NODE_SLOT, true);
    tabBarPattern->UpdateSubTabBoard();

    tabBarPattern->SetSelectedMode(SelectedMode::BOARD, 0);
    EXPECT_EQ(tabBarPattern->selectedModes_[0], SelectedMode::BOARD);
    tabBarPattern->UpdateSubTabBoard();

    tabBarPattern->SetSelectedMode(SelectedMode::INDICATOR, 0);
    EXPECT_EQ(tabBarPattern->selectedModes_[0], SelectedMode::INDICATOR);
    TabContentModelNG tabContentModel2;
    tabContentModel2.Create();
    tabContentModel2.SetSelectedMode(selectedMode);
    tabContentFrameNode = AceType::DynamicCast<TabContentNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(tabContentFrameNode, nullptr);
    EXPECT_EQ(tabContentFrameNode->GetTag(), V2::TAB_CONTENT_ITEM_ETS_TAG);
    tabContentFrameNode->MountToParent(swiperNode);
    tabContentPattern->SetTabBar(text_test, "", nullptr);
    EXPECT_EQ(tabContentPattern->GetTabBarParam().GetText(), text_test);
    tabContentModel.AddTabBarItem(tabContentFrameNode, DEFAULT_NODE_SLOT, true);
    tabBarPattern->UpdateSubTabBoard();
    EXPECT_EQ(tabBarPattern->indicator_, 0);

    tabBarPattern->indicator_ = 1;
    tabBarPattern->UpdateSubTabBoard();
    EXPECT_EQ(tabBarPattern->indicator_, 1);

    IndicatorStyle style;
    tabBarPattern->SetIndicatorStyle(style, 1);
    tabBarPattern->UpdateSubTabBoard();
    EXPECT_EQ(tabBarPattern->indicator_, 1);

    auto tabBarLayoutProperty = tabBarNode->GetLayoutProperty<TabBarLayoutProperty>();
    ASSERT_NE(tabBarLayoutProperty, nullptr);
    tabBarLayoutProperty->UpdateAxis(Axis::VERTICAL);
    tabBarPattern->UpdateSubTabBoard();
    EXPECT_EQ(tabBarPattern->indicator_, 1);

    tabBarLayoutProperty->UpdateAxis(Axis::HORIZONTAL);
    tabBarPattern->UpdateSubTabBoard();
    EXPECT_EQ(tabBarPattern->indicator_, 1);
}

/**
 * @tc.name: TabBarPatternUpdateGradientRegions001
 * @tc.desc: test UpdateGradientRegions
 * @tc.type: FUNC
 */
HWTEST_F(TabsTestNg, TabBarPatternUpdateGradientRegions001, TestSize.Level1)
{
    MockPipelineContextGetTheme();
    const std::string text_test = "text_test";

    TabContentModelNG tabContentModel;

    SelectedMode selectedMode = SelectedMode::INDICATOR;
    tabContentModel.Create();
    tabContentModel.SetSelectedMode(selectedMode);
    auto tabContentFrameNode = AceType::DynamicCast<TabContentNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(tabContentFrameNode, nullptr);
    EXPECT_EQ(tabContentFrameNode->GetTag(), V2::TAB_CONTENT_ITEM_ETS_TAG);

    TabsModelNG tabsModel;
    tabsModel.Create(BarPosition::START, 1, nullptr, nullptr);
    tabsModel.SetTabBarMode(TabBarMode::SCROLLABLE);
    auto tabsNode = AceType::DynamicCast<TabsNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(tabsNode, nullptr);
    EXPECT_EQ(tabsNode->GetTag(), V2::TABS_ETS_TAG);
    auto tabContentPattern = tabContentFrameNode->GetPattern<TabContentPattern>();
    ASSERT_NE(tabContentPattern, nullptr);
    EXPECT_EQ(tabContentPattern->GetSelectedMode(), selectedMode);
    tabContentFrameNode->GetTabBarItemId();

    auto swiperNode = AceType::DynamicCast<FrameNode>(tabsNode->GetChildAtIndex(TEST_SWIPER_INDEX));
    ASSERT_NE(swiperNode, nullptr);
    EXPECT_EQ(swiperNode->GetTag(), V2::SWIPER_ETS_TAG);
    tabContentFrameNode->MountToParent(swiperNode);
    tabContentPattern->SetTabBar(text_test, "", nullptr);
    EXPECT_EQ(tabContentPattern->GetTabBarParam().GetText(), text_test);
    tabContentModel.AddTabBarItem(tabContentFrameNode, DEFAULT_NODE_SLOT, true);

    auto tabBarNode = AceType::DynamicCast<FrameNode>(tabsNode->GetChildAtIndex(TEST_TAB_BAR_INDEX));
    ASSERT_NE(tabBarNode, nullptr);
    EXPECT_EQ(tabBarNode->GetTag(), V2::TAB_BAR_ETS_TAG);

    auto tabBarLayoutProperty = tabBarNode->GetLayoutProperty<TabBarLayoutProperty>();
    ASSERT_NE(tabBarLayoutProperty, nullptr);
    tabBarLayoutProperty->UpdateAxis(Axis::VERTICAL);
    EXPECT_EQ(tabBarLayoutProperty->GetAxisValue(), Axis::VERTICAL);

    auto tabBarPattern = tabBarNode->GetPattern<TabBarPattern>();
    ASSERT_NE(tabBarPattern, nullptr);
    tabBarPattern->tabItemOffsets_ = { { -1.0f, -1.0f } };
    tabBarPattern->childrenMainSize_ = 10.0f;
    tabBarPattern->UpdateGradientRegions();
    EXPECT_TRUE(tabBarPattern->gradientRegions_[2]);

    tabBarPattern->tabItemOffsets_ = { { 1.0f, 1.0f } };
    tabBarPattern->childrenMainSize_ = 10.0f;
    tabBarPattern->UpdateGradientRegions();
    EXPECT_FALSE(tabBarPattern->gradientRegions_[2]);

    tabBarPattern->tabItemOffsets_ = { { 10.0f, 10.0f } };
    tabBarPattern->childrenMainSize_ = 10.0f;
    tabBarPattern->UpdateGradientRegions();
    EXPECT_TRUE(tabBarPattern->gradientRegions_[3]);

    tabBarPattern->tabItemOffsets_ = { { -10.0f, -10.0f } };
    tabBarPattern->childrenMainSize_ = 5.0f;
    tabBarPattern->UpdateGradientRegions();
    EXPECT_FALSE(tabBarPattern->gradientRegions_[3]);

    tabBarLayoutProperty->UpdateAxis(Axis::HORIZONTAL);
    EXPECT_EQ(tabBarLayoutProperty->GetAxisValue(), Axis::HORIZONTAL);

    tabBarPattern->tabItemOffsets_ = { { -1.0f, -1.0f } };
    tabBarPattern->childrenMainSize_ = 10.0f;
    tabBarPattern->UpdateGradientRegions();
    EXPECT_TRUE(tabBarPattern->gradientRegions_[0]);

    tabBarPattern->tabItemOffsets_ = { { 1.0f, 1.0f } };
    tabBarPattern->childrenMainSize_ = 10.0f;
    tabBarPattern->UpdateGradientRegions();
    EXPECT_FALSE(tabBarPattern->gradientRegions_[0]);

    tabBarPattern->tabItemOffsets_ = { { 10.0f, 10.0f } };
    tabBarPattern->childrenMainSize_ = 10.0f;
    tabBarPattern->UpdateGradientRegions();
    EXPECT_TRUE(tabBarPattern->gradientRegions_[1]);

    tabBarPattern->tabItemOffsets_ = { { -10.0f, -10.0f } };
    tabBarPattern->childrenMainSize_ = 5.0f;
    tabBarPattern->UpdateGradientRegions();
    EXPECT_FALSE(tabBarPattern->gradientRegions_[1]);
}

/**
 * @tc.name: TabBarPatternSetSelectedMode001
 * @tc.desc: test SetSelectedMode
 * @tc.type: FUNC
 */
HWTEST_F(TabsTestNg, TabBarPatternSetSelectedMode001, TestSize.Level1)
{
    MockPipelineContextGetTheme();
    TabsModelNG tabsModel;
    bool fadingEdge = true;
    tabsModel.Create(BarPosition::START, 1, nullptr, nullptr);
    tabsModel.SetFadingEdge(fadingEdge);

    auto tabsFrameNode = AceType::DynamicCast<TabsNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(tabsFrameNode, nullptr);
    EXPECT_EQ(tabsFrameNode->GetTag(), V2::TABS_ETS_TAG);
    auto tabBarNode = AceType::DynamicCast<FrameNode>(tabsFrameNode->GetChildAtIndex(TEST_TAB_BAR_INDEX));
    ASSERT_NE(tabBarNode, nullptr);
    EXPECT_EQ(tabBarNode->GetTag(), V2::TAB_BAR_ETS_TAG);
    auto tabBarPaintProperty = tabBarNode->GetPaintProperty<TabBarPaintProperty>();
    ASSERT_NE(tabBarPaintProperty, nullptr);
    EXPECT_EQ(tabBarPaintProperty->GetFadingEdgeValue(), fadingEdge);

    auto tabBarPattern = tabBarNode->GetPattern<TabBarPattern>();
    ASSERT_NE(tabBarPattern, nullptr);
    tabBarPattern->SetSelectedMode(SelectedMode::INDICATOR, 0);
    EXPECT_EQ(tabBarPattern->selectedModes_[0], SelectedMode::INDICATOR);
    tabBarPattern->SetSelectedMode(SelectedMode::INDICATOR, 1);
    EXPECT_EQ(tabBarPattern->selectedModes_[1], SelectedMode::INDICATOR);
    tabBarPattern->SetSelectedMode(SelectedMode::INDICATOR, 0);
    EXPECT_EQ(tabBarPattern->selectedModes_[0], SelectedMode::INDICATOR);

    IndicatorStyle indicator;
    tabBarPattern->SetIndicatorStyle(indicator, 0);
    EXPECT_EQ(tabBarPattern->indicatorStyles_[0], indicator);
    tabBarPattern->SetIndicatorStyle(indicator, 1);
    EXPECT_EQ(tabBarPattern->indicatorStyles_[1], indicator);
    tabBarPattern->SetIndicatorStyle(indicator, 0);
    EXPECT_EQ(tabBarPattern->indicatorStyles_[0], indicator);

    tabBarPattern->SetTabBarStyle(TabBarStyle::SUBTABBATSTYLE, 0);
    EXPECT_EQ(tabBarPattern->tabBarStyles_[0], TabBarStyle::SUBTABBATSTYLE);
    tabBarPattern->SetTabBarStyle(TabBarStyle::SUBTABBATSTYLE, 1);
    EXPECT_EQ(tabBarPattern->tabBarStyles_[1], TabBarStyle::SUBTABBATSTYLE);
    tabBarPattern->SetTabBarStyle(TabBarStyle::SUBTABBATSTYLE, 0);
    EXPECT_EQ(tabBarPattern->tabBarStyles_[0], TabBarStyle::SUBTABBATSTYLE);

    auto selectedMode = tabBarPattern->GetSelectedMode();
    EXPECT_EQ(selectedMode, SelectedMode::INDICATOR);
}

/**
 * @tc.name: TabBarPatternUpdateIndicator001
 * @tc.desc: test UpdateIndicator
 * @tc.type: FUNC
 */
HWTEST_F(TabsTestNg, TabBarPatternUpdateIndicator001, TestSize.Level1)
{
    MockPipelineContextGetTheme();
    TabsModelNG tabsModel;
    bool fadingEdge = true;
    tabsModel.Create(BarPosition::START, 1, nullptr, nullptr);
    tabsModel.SetFadingEdge(fadingEdge);
    auto tabsFrameNode = AceType::DynamicCast<TabsNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(tabsFrameNode, nullptr);
    EXPECT_EQ(tabsFrameNode->GetTag(), V2::TABS_ETS_TAG);
    auto tabBarNode = AceType::DynamicCast<FrameNode>(tabsFrameNode->GetChildAtIndex(TEST_TAB_BAR_INDEX));
    ASSERT_NE(tabBarNode, nullptr);
    EXPECT_EQ(tabBarNode->GetTag(), V2::TAB_BAR_ETS_TAG);

    auto tabBarLayoutProperty = tabBarNode->GetLayoutProperty<TabBarLayoutProperty>();
    ASSERT_NE(tabBarLayoutProperty, nullptr);

    auto tabBarPaintProperty = tabBarNode->GetPaintProperty<TabBarPaintProperty>();
    ASSERT_NE(tabBarPaintProperty, nullptr);
    EXPECT_EQ(tabBarPaintProperty->GetFadingEdgeValue(), fadingEdge);

    auto tabBarPattern = tabBarNode->GetPattern<TabBarPattern>();
    tabBarPattern->SetTabBarStyle(TabBarStyle::BOTTOMTABBATSTYLE, 0);
    EXPECT_EQ(tabBarPattern->tabBarStyles_[0], TabBarStyle::BOTTOMTABBATSTYLE);
    tabBarPattern->UpdateIndicator(0);
    tabBarPattern->SetTabBarStyle(TabBarStyle::SUBTABBATSTYLE, 0);
    EXPECT_EQ(tabBarPattern->tabBarStyles_[0], TabBarStyle::SUBTABBATSTYLE);
    tabBarPattern->UpdateIndicator(0);
    tabBarPattern->SetTabBarStyle(TabBarStyle::NOSTYLE, 0);
    EXPECT_EQ(tabBarPattern->tabBarStyles_[0], TabBarStyle::NOSTYLE);
    tabBarPattern->UpdateIndicator(0);

    tabBarPattern->SetTabBarStyle(TabBarStyle::BOTTOMTABBATSTYLE, 0);
    EXPECT_EQ(tabBarPattern->tabBarStyles_[0], TabBarStyle::BOTTOMTABBATSTYLE);

    tabBarLayoutProperty->UpdateAxis(Axis::VERTICAL);
    EXPECT_EQ(tabBarLayoutProperty->GetAxisValue(), Axis::VERTICAL);
    tabBarPattern->tabBarType_[0] = true;
    tabBarPattern->UpdateIndicator(0);

    tabBarPattern->SetTabBarStyle(TabBarStyle::SUBTABBATSTYLE, 0);
    EXPECT_EQ(tabBarPattern->tabBarStyles_[0], TabBarStyle::SUBTABBATSTYLE);

    tabBarLayoutProperty->UpdateAxis(Axis::HORIZONTAL);
    EXPECT_EQ(tabBarLayoutProperty->GetAxisValue(), Axis::HORIZONTAL);
    tabBarPattern->tabBarType_[0] = false;
    tabBarPattern->UpdateIndicator(0);
    EXPECT_EQ(tabBarPattern->indicator_, 0);

    tabBarPattern->indicator_ = 1;
    tabBarPattern->UpdateIndicator(0);
    EXPECT_EQ(tabBarPattern->indicator_, 1);
}

/**
 * @tc.name: TabBarPatternPlayPressAnimation001
 * @tc.desc: test PlayPressAnimation
 * @tc.type: FUNC
 */
HWTEST_F(TabsTestNg, TabBarPatternPlayPressAnimation001, TestSize.Level1)
{
    MockPipelineContextGetTheme();
    TabsModelNG tabsModel;
    bool fadingEdge = true;
    tabsModel.Create(BarPosition::START, 1, nullptr, nullptr);
    tabsModel.SetFadingEdge(fadingEdge);
    auto tabsFrameNode = AceType::DynamicCast<TabsNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(tabsFrameNode, nullptr);
    EXPECT_EQ(tabsFrameNode->GetTag(), V2::TABS_ETS_TAG);
    auto tabBarNode = AceType::DynamicCast<FrameNode>(tabsFrameNode->GetChildAtIndex(TEST_TAB_BAR_INDEX));
    ASSERT_NE(tabBarNode, nullptr);
    EXPECT_EQ(tabBarNode->GetTag(), V2::TAB_BAR_ETS_TAG);

    auto tabBarPaintProperty = tabBarNode->GetPaintProperty<TabBarPaintProperty>();
    ASSERT_NE(tabBarPaintProperty, nullptr);
    EXPECT_EQ(tabBarPaintProperty->GetFadingEdgeValue(), fadingEdge);

    auto tabBarPattern = tabBarNode->GetPattern<TabBarPattern>();
    tabBarPattern->SetTabBarStyle(TabBarStyle::SUBTABBATSTYLE, 0);
    IndicatorStyle indicatorStyle;
    tabBarPattern->SetIndicatorStyle(indicatorStyle, 0);
    EXPECT_EQ(tabBarPattern->tabBarStyles_[0], TabBarStyle::SUBTABBATSTYLE);
    tabBarPattern->SetSelectedMode(SelectedMode::BOARD, 0);
    EXPECT_EQ(tabBarPattern->selectedModes_[0], SelectedMode::BOARD);
    tabBarPattern->PlayPressAnimation(0, Color::BLACK, AnimationType::PRESS);

    tabBarPattern->SetSelectedMode(SelectedMode::INDICATOR, 0);
    EXPECT_EQ(tabBarPattern->selectedModes_[0], SelectedMode::INDICATOR);
    tabBarPattern->PlayPressAnimation(0, Color::BLACK, AnimationType::PRESS);
    EXPECT_EQ(tabBarPattern->indicator_, 0);

    auto tabBarLayoutProperty = tabBarNode->GetLayoutProperty<TabBarLayoutProperty>();
    ASSERT_NE(tabBarLayoutProperty, nullptr);

    tabBarLayoutProperty->UpdateAxis(Axis::HORIZONTAL);
    tabBarPattern->SetSelectedMode(SelectedMode::BOARD, 0);
    EXPECT_EQ(tabBarPattern->selectedModes_[0], SelectedMode::BOARD);
    tabBarPattern->PlayPressAnimation(0, Color::TRANSPARENT, AnimationType::PRESS);
    EXPECT_EQ(tabBarPattern->indicator_, 0);

    tabBarLayoutProperty->UpdateAxis(Axis::HORIZONTAL);
    tabBarPattern->SetSelectedMode(SelectedMode::BOARD, 0);
    EXPECT_EQ(tabBarPattern->selectedModes_[0], SelectedMode::BOARD);
    tabBarPattern->PlayPressAnimation(0, Color::BLACK, AnimationType::PRESS);
    EXPECT_EQ(tabBarPattern->indicator_, 0);

    tabBarLayoutProperty->UpdateAxis(Axis::HORIZONTAL);
    tabBarPattern->SetSelectedMode(SelectedMode::INDICATOR, 0);
    EXPECT_EQ(tabBarPattern->selectedModes_[0], SelectedMode::INDICATOR);
    tabBarPattern->PlayPressAnimation(0, Color::TRANSPARENT, AnimationType::PRESS);
    EXPECT_EQ(tabBarPattern->indicator_, 0);

    tabBarLayoutProperty->UpdateAxis(Axis::VERTICAL);
    tabBarPattern->SetSelectedMode(SelectedMode::BOARD, 0);
    EXPECT_EQ(tabBarPattern->selectedModes_[0], SelectedMode::BOARD);
    tabBarPattern->PlayPressAnimation(0, Color::TRANSPARENT, AnimationType::PRESS);
    EXPECT_EQ(tabBarPattern->indicator_, 0);
}

/**
 * @tc.name: TabBarPatternHandleClick001
 * @tc.desc: test HandleClick
 * @tc.type: FUNC
 */
HWTEST_F(TabsTestNg, TabBarPatternHandleClick001, TestSize.Level1)
{
    MockPipelineContextGetTheme();
    EXPECT_CALL(*MockPipelineBase::GetCurrent(), AddScheduleTask(_)).WillRepeatedly(Return(0));
    EXPECT_CALL(*MockPipelineBase::GetCurrent(), RemoveScheduleTask(_)).Times(AnyNumber());

    const std::string text_test = "text_test";

    TabContentModelNG tabContentModel;

    SelectedMode selectedMode = SelectedMode::INDICATOR;
    tabContentModel.Create();
    tabContentModel.SetSelectedMode(selectedMode);
    auto tabContentFrameNode = AceType::DynamicCast<TabContentNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(tabContentFrameNode, nullptr);
    EXPECT_EQ(tabContentFrameNode->GetTag(), V2::TAB_CONTENT_ITEM_ETS_TAG);

    TabsModelNG tabsModel;
    tabsModel.Create(BarPosition::START, 1, nullptr, nullptr);
    auto tabsNode = AceType::DynamicCast<TabsNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(tabsNode, nullptr);
    EXPECT_EQ(tabsNode->GetTag(), V2::TABS_ETS_TAG);
    auto tabContentPattern = tabContentFrameNode->GetPattern<TabContentPattern>();
    ASSERT_NE(tabContentPattern, nullptr);
    EXPECT_EQ(tabContentPattern->GetSelectedMode(), SelectedMode::INDICATOR);
    tabContentFrameNode->GetTabBarItemId();

    auto swiperNode = AceType::DynamicCast<FrameNode>(tabsNode->GetChildAtIndex(TEST_SWIPER_INDEX));
    EXPECT_EQ(swiperNode->GetTag(), V2::SWIPER_ETS_TAG);
    tabContentFrameNode->MountToParent(swiperNode);
    tabContentPattern->SetTabBar(text_test, "", nullptr);
    EXPECT_EQ(tabContentPattern->GetTabBarParam().GetText(), text_test);
    tabContentModel.AddTabBarItem(tabContentFrameNode, DEFAULT_NODE_SLOT, true);

    auto tabBarNode = AceType::DynamicCast<FrameNode>(tabsNode->GetChildAtIndex(TEST_TAB_BAR_INDEX));
    ASSERT_NE(tabBarNode, nullptr);
    EXPECT_EQ(tabBarNode->GetTag(), V2::TAB_BAR_ETS_TAG);

    auto tabBarPattern = tabBarNode->GetPattern<TabBarPattern>();
    ASSERT_NE(tabBarPattern, nullptr);

    auto tabBarLayoutProperty = tabBarNode->GetLayoutProperty<TabBarLayoutProperty>();
    ASSERT_NE(tabBarLayoutProperty, nullptr);
    tabBarLayoutProperty->UpdateAxis(Axis::VERTICAL);
    EXPECT_EQ(tabBarLayoutProperty->GetAxisValue(), Axis::VERTICAL);
    tabBarPattern->tabBarStyle_ = TabBarStyle::SUBTABBATSTYLE;
    tabBarPattern->tabItemOffsets_ = { { 0.0f, 0.0f }, { 10.0f, 10.0f } };

    GestureEvent info;
    Offset offset(1, 1);
    info.SetLocalLocation(offset);
    tabBarLayoutProperty->UpdateAxis(Axis::HORIZONTAL);
    tabBarPattern->HandleClick(info);
    EXPECT_EQ(tabBarLayoutProperty->GetAxisValue(), Axis::HORIZONTAL);

    tabBarPattern->indicator_ = 1;
    tabBarPattern->HandleClick(info);
    EXPECT_EQ(tabBarPattern->indicator_, 1);
}

/**
 * @tc.name: TabBarPatternHandleSubTabBarClick001
 * @tc.desc: test HandleSubTabBarClick
 * @tc.type: FUNC
 */
HWTEST_F(TabsTestNg, TabBarPatternHandleSubTabBarClick001, TestSize.Level1)
{
    MockPipelineContextGetTheme();
    EXPECT_CALL(*MockPipelineBase::GetCurrent(), AddScheduleTask(_)).WillRepeatedly(Return(0));
    EXPECT_CALL(*MockPipelineBase::GetCurrent(), RemoveScheduleTask(_)).Times(AnyNumber());

    const std::string text_test = "text_test";

    TabContentModelNG tabContentModel;

    SelectedMode selectedMode = SelectedMode::INDICATOR;
    tabContentModel.Create();
    tabContentModel.SetSelectedMode(selectedMode);
    auto tabContentFrameNode = AceType::DynamicCast<TabContentNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(tabContentFrameNode, nullptr);
    EXPECT_EQ(tabContentFrameNode->GetTag(), V2::TAB_CONTENT_ITEM_ETS_TAG);

    TabsModelNG tabsModel;
    tabsModel.Create(BarPosition::START, 1, nullptr, nullptr);
    auto tabsNode = AceType::DynamicCast<TabsNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(tabsNode, nullptr);
    auto tabContentPattern = tabContentFrameNode->GetPattern<TabContentPattern>();
    ASSERT_NE(tabContentPattern, nullptr);
    tabContentFrameNode->GetTabBarItemId();

    auto swiperNode = AceType::DynamicCast<FrameNode>(tabsNode->GetChildAtIndex(TEST_SWIPER_INDEX));
    EXPECT_EQ(swiperNode->GetTag(), V2::SWIPER_ETS_TAG);
    tabContentFrameNode->MountToParent(swiperNode);
    tabContentPattern->SetTabBar(text_test, "", nullptr);
    EXPECT_EQ(tabContentPattern->GetTabBarParam().GetText(), text_test);

    tabContentPattern->SetTabBarStyle(TabBarStyle::SUBTABBATSTYLE);
    EXPECT_EQ(tabContentPattern->GetTabBarStyle(), TabBarStyle::SUBTABBATSTYLE);
    tabContentModel.AddTabBarItem(tabContentFrameNode, DEFAULT_NODE_SLOT, true);
    tabContentPattern->SetTabBar(text_test, "", nullptr);
    EXPECT_EQ(tabContentPattern->GetTabBarParam().GetText(), text_test);

    tabContentPattern->SetTabBarStyle(TabBarStyle::SUBTABBATSTYLE);
    EXPECT_EQ(tabContentPattern->GetTabBarStyle(), TabBarStyle::SUBTABBATSTYLE);
    tabContentModel.AddTabBarItem(tabContentFrameNode, DEFAULT_NODE_SLOT, true);

    auto tabBarNode = AceType::DynamicCast<FrameNode>(tabsNode->GetChildAtIndex(TEST_TAB_BAR_INDEX));
    ASSERT_NE(tabBarNode, nullptr);
    EXPECT_EQ(tabBarNode->GetTag(), V2::TAB_BAR_ETS_TAG);

    auto tabBarPattern = tabBarNode->GetPattern<TabBarPattern>();
    ASSERT_NE(tabBarPattern, nullptr);

    auto tabBarLayoutProperty = tabBarNode->GetLayoutProperty<TabBarLayoutProperty>();
    ASSERT_NE(tabBarLayoutProperty, nullptr);
    tabBarLayoutProperty->UpdateAxis(Axis::VERTICAL);
    EXPECT_EQ(tabBarLayoutProperty->GetAxisValue(), Axis::VERTICAL);
    tabBarPattern->tabItemOffsets_ = { { 0.0f, 0.0f }, { 10.0f, 10.0f } };

    GestureEvent info;
    Offset offset(1, 1);
    info.SetLocalLocation(offset);
    tabBarLayoutProperty->UpdateAxis(Axis::HORIZONTAL);
    EXPECT_EQ(tabBarLayoutProperty->GetAxisValue(), Axis::HORIZONTAL);
    tabBarPattern->SetTabBarStyle(TabBarStyle::SUBTABBATSTYLE, 1);
    EXPECT_EQ(tabBarPattern->tabBarStyles_[1], TabBarStyle::SUBTABBATSTYLE);
    tabBarPattern->indicator_ = 1;
    tabBarPattern->SetTabBarStyle(TabBarStyle::SUBTABBATSTYLE, 0);
    EXPECT_EQ(tabBarPattern->tabBarStyles_[0], TabBarStyle::SUBTABBATSTYLE);
    tabBarPattern->HandleClick(info);

    tabBarLayoutProperty->UpdateAxis(Axis::VERTICAL);
    EXPECT_EQ(tabBarLayoutProperty->GetAxisValue(), Axis::VERTICAL);
    tabBarPattern->SetTabBarStyle(TabBarStyle::SUBTABBATSTYLE, 1);
    EXPECT_EQ(tabBarPattern->tabBarStyles_[1], TabBarStyle::SUBTABBATSTYLE);
    tabBarPattern->indicator_ = 1;
    tabBarPattern->SetTabBarStyle(TabBarStyle::SUBTABBATSTYLE, 0);
    EXPECT_EQ(tabBarPattern->tabBarStyles_[0], TabBarStyle::SUBTABBATSTYLE);
    tabBarPattern->HandleClick(info);

    tabBarLayoutProperty->UpdateAxis(Axis::HORIZONTAL);
    EXPECT_EQ(tabBarLayoutProperty->GetAxisValue(), Axis::HORIZONTAL);
    tabBarPattern->SetTabBarStyle(TabBarStyle::SUBTABBATSTYLE, 1);
    EXPECT_EQ(tabBarPattern->tabBarStyles_[1], TabBarStyle::SUBTABBATSTYLE);
    tabBarPattern->indicator_ = 1;
    tabBarPattern->SetTabBarStyle(TabBarStyle::BOTTOMTABBATSTYLE, 0);
    EXPECT_EQ(tabBarPattern->tabBarStyles_[0], TabBarStyle::BOTTOMTABBATSTYLE);
    tabBarPattern->HandleClick(info);

    tabBarLayoutProperty->UpdateAxis(Axis::HORIZONTAL);
    tabBarPattern->SetTabBarStyle(TabBarStyle::BOTTOMTABBATSTYLE, 1);
    EXPECT_EQ(tabBarPattern->tabBarStyles_[1], TabBarStyle::BOTTOMTABBATSTYLE);
    tabBarPattern->indicator_ = 1;
    tabBarPattern->SetTabBarStyle(TabBarStyle::SUBTABBATSTYLE, 0);
    EXPECT_EQ(tabBarPattern->tabBarStyles_[0], TabBarStyle::SUBTABBATSTYLE);
    tabBarPattern->HandleClick(info);

    tabBarPattern->indicator_ = 1;
    tabBarPattern->HandleSubTabBarClick(tabBarLayoutProperty, 1);
    EXPECT_EQ(tabBarPattern->indicator_, 1);
    PipelineContext::GetCurrentContext()->SetThemeManager(nullptr);
}

/**
 * @tc.name: TabBarPatternGetIndicatorRect001
 * @tc.desc: test GetIndicatorRect
 * @tc.type: FUNC
 */
HWTEST_F(TabsTestNg, TabBarPatternGetIndicatorRect001, TestSize.Level1)
{
    MockPipelineContextGetTheme();
    EXPECT_CALL(*MockPipelineBase::GetCurrent(), AddScheduleTask(_)).WillRepeatedly(Return(0));
    EXPECT_CALL(*MockPipelineBase::GetCurrent(), RemoveScheduleTask(_)).Times(AnyNumber());

    const std::string text_test = "text_test";

    TabContentModelNG tabContentModel;

    SelectedMode selectedMode = SelectedMode::INDICATOR;
    tabContentModel.Create();
    tabContentModel.SetSelectedMode(selectedMode);
    auto tabContentFrameNode = AceType::DynamicCast<TabContentNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(tabContentFrameNode, nullptr);
    EXPECT_EQ(tabContentFrameNode->GetTag(), V2::TAB_CONTENT_ITEM_ETS_TAG);

    TabsModelNG tabsModel;
    tabsModel.Create(BarPosition::START, 1, nullptr, nullptr);
    auto tabsNode = AceType::DynamicCast<TabsNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(tabsNode, nullptr);
    auto tabContentPattern = tabContentFrameNode->GetPattern<TabContentPattern>();
    ASSERT_NE(tabContentPattern, nullptr);
    EXPECT_EQ(tabContentPattern->GetSelectedMode(), selectedMode);
    tabContentFrameNode->GetTabBarItemId();

    auto swiperNode = AceType::DynamicCast<FrameNode>(tabsNode->GetChildAtIndex(TEST_SWIPER_INDEX));
    ASSERT_NE(swiperNode, nullptr);
    EXPECT_EQ(swiperNode->GetTag(), V2::SWIPER_ETS_TAG);
    tabContentFrameNode->MountToParent(swiperNode);
    tabContentPattern->SetTabBar(text_test, "", nullptr);
    EXPECT_EQ(tabContentPattern->GetTabBarParam().GetText(), text_test);
    tabContentModel.AddTabBarItem(tabContentFrameNode, DEFAULT_NODE_SLOT, true);

    auto tabBarNode = AceType::DynamicCast<FrameNode>(tabsNode->GetChildAtIndex(TEST_TAB_BAR_INDEX));
    ASSERT_NE(tabBarNode, nullptr);
    EXPECT_EQ(tabBarNode->GetTag(), V2::TAB_BAR_ETS_TAG);

    auto tabBarPattern = tabBarNode->GetPattern<TabBarPattern>();
    ASSERT_NE(tabBarPattern, nullptr);

    auto tabBarLayoutProperty = tabBarNode->GetLayoutProperty<TabBarLayoutProperty>();
    ASSERT_NE(tabBarLayoutProperty, nullptr);
    auto rect = tabBarLayoutProperty->GetIndicatorRect(0);
    EXPECT_EQ(rect.GetX(), 0);
}

/**
 * @tc.name: TabBarPatternGetSelectedMode001
 * @tc.desc: test GetSelectedMode
 * @tc.type: FUNC
 */
HWTEST_F(TabsTestNg, TabBarPatternGetSelectedMode001, TestSize.Level1)
{
    MockPipelineContextGetTheme();
    EXPECT_CALL(*MockPipelineBase::GetCurrent(), AddScheduleTask(_)).WillRepeatedly(Return(0));
    EXPECT_CALL(*MockPipelineBase::GetCurrent(), RemoveScheduleTask(_)).Times(AnyNumber());

    const std::string text_test = "text_test";

    TabContentModelNG tabContentModel;

    SelectedMode selectedMode = SelectedMode::INDICATOR;
    tabContentModel.Create();
    tabContentModel.SetSelectedMode(selectedMode);
    auto tabContentFrameNode = AceType::DynamicCast<TabContentNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(tabContentFrameNode, nullptr);
    EXPECT_EQ(tabContentFrameNode->GetTag(), V2::TAB_CONTENT_ITEM_ETS_TAG);

    TabsModelNG tabsModel;
    tabsModel.Create(BarPosition::START, 1, nullptr, nullptr);
    auto tabsNode = AceType::DynamicCast<TabsNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(tabsNode, nullptr);
    auto tabContentPattern = tabContentFrameNode->GetPattern<TabContentPattern>();
    ASSERT_NE(tabContentPattern, nullptr);
    EXPECT_EQ(tabContentPattern->GetSelectedMode(), selectedMode);
    tabContentFrameNode->GetTabBarItemId();

    auto swiperNode = AceType::DynamicCast<FrameNode>(tabsNode->GetChildAtIndex(TEST_SWIPER_INDEX));
    ASSERT_NE(swiperNode, nullptr);
    EXPECT_EQ(swiperNode->GetTag(), V2::SWIPER_ETS_TAG);
    tabContentFrameNode->MountToParent(swiperNode);
    tabContentPattern->SetTabBar(text_test, "", nullptr);
    EXPECT_EQ(tabContentPattern->GetTabBarParam().GetText(), text_test);
    tabContentModel.AddTabBarItem(tabContentFrameNode, DEFAULT_NODE_SLOT, true);

    auto tabBarNode = AceType::DynamicCast<FrameNode>(tabsNode->GetChildAtIndex(TEST_TAB_BAR_INDEX));
    ASSERT_NE(tabBarNode, nullptr);
    EXPECT_EQ(tabBarNode->GetTag(), V2::TAB_BAR_ETS_TAG);

    auto tabBarPattern = tabBarNode->GetPattern<TabBarPattern>();
    ASSERT_NE(tabBarPattern, nullptr);

    tabBarPattern->indicator_ = 1;
    auto mode = tabBarPattern->GetSelectedMode();
    EXPECT_EQ(mode, selectedMode);
}


/**
 * @tc.name: TabBarModifierSetIndicator001
 * @tc.desc: test SetIndicator
 * @tc.type: FUNC
 */
HWTEST_F(TabsTestNg, TabBarModifierSetIndicator001, TestSize.Level1)
{
    RectF rect(0.0f, 0.0f, 1.0f, 1.0f);
    auto tabBarModifier = AceType::MakeRefPtr<TabBarModifier>();
    tabBarModifier->SetIndicator(rect);
    EXPECT_EQ(tabBarModifier->indicatorLeft_->Get(), 0.0f);
    EXPECT_EQ(tabBarModifier->indicatorTop_->Get(), 0.0f);
    tabBarModifier->indicatorLeft_ = nullptr;
    tabBarModifier->indicatorTop_ = nullptr;
    tabBarModifier->SetIndicator(rect);
    EXPECT_EQ(tabBarModifier->indicatorLeft_, nullptr);
    EXPECT_EQ(tabBarModifier->indicatorTop_, nullptr);
}

/**
 * @tc.name: TabBarModifierSetIndicatorColor001
 * @tc.desc: test SetIndicatorColor
 * @tc.type: FUNC
 */
HWTEST_F(TabsTestNg, TabBarModifierSetIndicatorColor001, TestSize.Level1)
{
    LinearColor indicatorColor(Color::BLACK);
    auto tabBarModifier = AceType::MakeRefPtr<TabBarModifier>();
    tabBarModifier->SetIndicatorColor(indicatorColor);
    EXPECT_TRUE(tabBarModifier->indicatorColor_->Get() == indicatorColor);
    tabBarModifier->indicatorColor_ = nullptr;
    tabBarModifier->SetIndicatorColor(indicatorColor);
    EXPECT_EQ(tabBarModifier->indicatorColor_, nullptr);
}

/**
 * @tc.name: TabBarModifierSetIndicatorWidth001
 * @tc.desc: test SetIndicatorWidth
 * @tc.type: FUNC
 */
HWTEST_F(TabsTestNg, TabBarModifierSetIndicatorWidth001, TestSize.Level1)
{
    float indicatorWidth = 1.0f;
    auto tabBarModifier = AceType::MakeRefPtr<TabBarModifier>();
    tabBarModifier->SetIndicatorWidth(indicatorWidth);
    EXPECT_EQ(tabBarModifier->indicatorWidth_->Get(), indicatorWidth);
    tabBarModifier->indicatorWidth_ = nullptr;
    tabBarModifier->SetIndicatorWidth(indicatorWidth);
    EXPECT_EQ(tabBarModifier->indicatorWidth_, nullptr);
}

/**
 * @tc.name: TabBarModifierSetIndicatorHeight001
 * @tc.desc: test SetIndicatorHeight
 * @tc.type: FUNC
 */
HWTEST_F(TabsTestNg, TabBarModifierSetIndicatorHeight001, TestSize.Level1)
{
    auto tabBarModifier = AceType::MakeRefPtr<TabBarModifier>();
    float indicatorHeight = 1.0f;
    tabBarModifier->SetIndicatorHeight(indicatorHeight);
    EXPECT_EQ(tabBarModifier->indicatorHeight_->Get(), indicatorHeight);
    tabBarModifier->indicatorHeight_ = nullptr;
    tabBarModifier->SetIndicatorHeight(indicatorHeight);
    EXPECT_EQ(tabBarModifier->indicatorHeight_, nullptr);
}

/**
 * @tc.name: TabBarModifierSetIndicatorBorderRadius001
 * @tc.desc: test SetIndicatorBorderRadius
 * @tc.type: FUNC
 */
HWTEST_F(TabsTestNg, TabBarModifierSetIndicatorBorderRadius001, TestSize.Level1)
{
    float indicatorBorderRadius = 1.0f;
    auto tabBarModifier = AceType::MakeRefPtr<TabBarModifier>();
    tabBarModifier->SetIndicatorBorderRadius(indicatorBorderRadius);
    EXPECT_EQ(tabBarModifier->indicatorBorderRadius_->Get(), indicatorBorderRadius);
    tabBarModifier->indicatorBorderRadius_ = nullptr;
    tabBarModifier->SetIndicatorBorderRadius(indicatorBorderRadius);
    EXPECT_EQ(tabBarModifier->indicatorBorderRadius_, nullptr);
}

/**
 * @tc.name: TabBarModifierSetIndicatorMarginTop001
 * @tc.desc: test SetIndicatorMarginTop
 * @tc.type: FUNC
 */
HWTEST_F(TabsTestNg, TabBarModifierSetIndicatorMarginTop001, TestSize.Level1)
{
    float indicatorMarginTop = 1.0f;
    auto tabBarModifier = AceType::MakeRefPtr<TabBarModifier>();
    tabBarModifier->SetIndicatorMarginTop(indicatorMarginTop);
    EXPECT_EQ(tabBarModifier->indicatorMarginTop_->Get(), indicatorMarginTop);
    tabBarModifier->indicatorMarginTop_ = nullptr;
    tabBarModifier->SetIndicatorMarginTop(indicatorMarginTop);
    EXPECT_EQ(tabBarModifier->indicatorMarginTop_, nullptr);
}

/**
 * @tc.name: TabBarModifierSetSelectedMode001
 * @tc.desc: test SetSelectedMode
 * @tc.type: FUNC
 */
HWTEST_F(TabsTestNg, TabBarModifierSetSelectedMode001, TestSize.Level1)
{
    SelectedMode selectedMode = SelectedMode::INDICATOR;
    auto tabBarModifier = AceType::MakeRefPtr<TabBarModifier>();
    tabBarModifier->SetSelectedMode(selectedMode);
    ASSERT_NE(tabBarModifier->hasIndicator_, nullptr);
    EXPECT_TRUE(tabBarModifier->hasIndicator_->Get());
    tabBarModifier->hasIndicator_ = nullptr;
    tabBarModifier->SetSelectedMode(selectedMode);
    EXPECT_EQ(tabBarModifier->hasIndicator_, nullptr);
}

/**
 * @tc.name: TabBarModifierOnDraw001
 * @tc.desc: test onDraw
 * @tc.type: FUNC
 */
HWTEST_F(TabsTestNg, TabBarModifierOnDraw001, TestSize.Level1)
{
    MockPipelineContextGetTheme();
    auto tabBarModifier = AceType::MakeRefPtr<TabBarModifier>();
    EXPECT_FALSE(tabBarModifier == nullptr);

    Testing::MockCanvas rsCanvas;
    EXPECT_CALL(rsCanvas, AttachBrush(_)).WillRepeatedly(ReturnRef(rsCanvas));
    EXPECT_CALL(rsCanvas, DetachBrush()).WillRepeatedly(ReturnRef(rsCanvas));
    EXPECT_CALL(rsCanvas, DrawRect(_)).Times(AnyNumber());
    EXPECT_CALL(rsCanvas, DrawRoundRect(_)).Times(AnyNumber());
    EXPECT_CALL(rsCanvas, Restore()).Times(AnyNumber());

    DrawingContext context { rsCanvas, 10.0f, 10.0f };
    float indicatorHeight = 1.0f;
    float indicatorWidth = 1.0f;
    float indicatorMarginTop = 1.0f;
    float indicatorBorderRadius = 1.0f;

    tabBarModifier->SetSelectedMode(SelectedMode::INDICATOR);
    tabBarModifier->SetIndicatorHeight(indicatorHeight);
    tabBarModifier->SetIndicatorWidth(indicatorWidth);
    tabBarModifier->SetIndicatorMarginTop(indicatorMarginTop);
    tabBarModifier->SetIndicatorBorderRadius(indicatorBorderRadius);
    tabBarModifier->onDraw(context);

    EXPECT_TRUE(tabBarModifier->hasIndicator_->Get());
    EXPECT_EQ(tabBarModifier->indicatorHeight_->Get(), indicatorHeight);
    EXPECT_EQ(tabBarModifier->indicatorWidth_->Get(), indicatorWidth);
    EXPECT_EQ(tabBarModifier->indicatorMarginTop_->Get(), indicatorMarginTop);
    EXPECT_EQ(tabBarModifier->indicatorBorderRadius_->Get(), indicatorBorderRadius);

    indicatorHeight = -1.0f;
    indicatorWidth = -1.0f;
    indicatorMarginTop = -1.0f;
    indicatorBorderRadius = -1.0f;

    tabBarModifier->SetSelectedMode(SelectedMode::INDICATOR);
    tabBarModifier->SetIndicatorHeight(indicatorHeight);
    tabBarModifier->SetIndicatorWidth(indicatorWidth);
    tabBarModifier->SetIndicatorMarginTop(indicatorMarginTop);
    tabBarModifier->SetIndicatorBorderRadius(indicatorBorderRadius);
    tabBarModifier->onDraw(context);

    EXPECT_TRUE(tabBarModifier->hasIndicator_->Get());
    EXPECT_EQ(tabBarModifier->indicatorHeight_->Get(), indicatorHeight);
    EXPECT_EQ(tabBarModifier->indicatorWidth_->Get(), indicatorWidth);
    EXPECT_EQ(tabBarModifier->indicatorMarginTop_->Get(), indicatorMarginTop);
    EXPECT_EQ(tabBarModifier->indicatorBorderRadius_->Get(), indicatorBorderRadius);

    tabBarModifier->SetSelectedMode(SelectedMode::BOARD);
    tabBarModifier->onDraw(context);
    EXPECT_FALSE(tabBarModifier->hasIndicator_->Get());

    tabBarModifier->hasIndicator_ = nullptr;
    tabBarModifier->onDraw(context);
    EXPECT_EQ(tabBarModifier->hasIndicator_, nullptr);
}

/**
 * @tc.name: TabBarPaintMethodGetForegroundDrawFunction001
 * @tc.desc: test GetForegroundDrawFunction
 * @tc.type: FUNC
 */
HWTEST_F(TabsTestNg, TabBarPaintMethodGetForegroundDrawFunction001, TestSize.Level1)
{
    MockPipelineContextGetTheme();
    TabsModelNG tabsModel;
    tabsModel.Create(BarPosition::START, 1, nullptr, nullptr);

    auto tabsFrameNode = AceType::DynamicCast<TabsNode>(ViewStackProcessor::GetInstance()->Finish());

    ASSERT_NE(tabsFrameNode, nullptr);
    EXPECT_EQ(tabsFrameNode->GetTag(), V2::TABS_ETS_TAG);
    auto tabBarFrameNode = AceType::DynamicCast<FrameNode>(tabsFrameNode->GetChildAtIndex(TEST_TAB_BAR_INDEX));
    ASSERT_NE(tabBarFrameNode, nullptr);
    EXPECT_EQ(tabBarFrameNode->GetTag(), V2::TAB_BAR_ETS_TAG);
    auto tabBarPattern = tabBarFrameNode->GetPattern<TabBarPattern>();
    ASSERT_NE(tabBarPattern, nullptr);
    auto paintProperty = tabBarPattern->CreatePaintProperty();
    ASSERT_NE(paintProperty, nullptr);
    IndicatorStyle indicator;
    tabBarPattern->SetIndicatorStyle(indicator, 0);
    EXPECT_EQ(tabBarPattern->indicatorStyles_[0], indicator);
    SelectedMode selectedMode = SelectedMode::INDICATOR;
    tabBarPattern->SetSelectedMode(selectedMode, 0);
    EXPECT_EQ(tabBarPattern->selectedModes_[0], selectedMode);
    auto paintMethod = tabBarPattern->CreateNodePaintMethod();
    ASSERT_NE(paintMethod, nullptr);

    auto paintMethod2 = tabBarPattern->CreateNodePaintMethod();
    ASSERT_NE(paintMethod2, nullptr);

    PaintWrapper paintWrapper(tabBarFrameNode->GetRenderContext(), tabBarFrameNode->GetGeometryNode(), paintProperty);
    auto drawFunction = paintMethod->GetForegroundDrawFunction(&paintWrapper);
    ASSERT_NE(drawFunction, nullptr);

    paintProperty = tabBarPattern->CreatePaintProperty();
    auto tabBarPaintProperty = AceType::DynamicCast<TabBarPaintProperty>(paintProperty);
    tabBarPaintProperty->UpdateFadingEdge(false);
    EXPECT_FALSE(tabBarPaintProperty->GetFadingEdgeValue());
    PaintWrapper paintWrapper2(tabBarFrameNode->GetRenderContext(), tabBarFrameNode->GetGeometryNode(), paintProperty);
    drawFunction = paintMethod->GetForegroundDrawFunction(&paintWrapper2);
    ASSERT_EQ(drawFunction, nullptr);
    auto clone = paintProperty->Clone();
    EXPECT_NE(clone, nullptr);
}

/**
 * @tc.name: TabBarPaintMethodGetContentModifier001
 * @tc.desc: test GetContentModifier
 * @tc.type: FUNC
 */
HWTEST_F(TabsTestNg, TabBarPaintMethodGetContentModifier001, TestSize.Level1)
{
    MockPipelineContextGetTheme();
    TabsModelNG tabsModel;
    tabsModel.Create(BarPosition::START, 1, nullptr, nullptr);

    auto tabsFrameNode = AceType::DynamicCast<TabsNode>(ViewStackProcessor::GetInstance()->Finish());

    ASSERT_NE(tabsFrameNode, nullptr);
    EXPECT_EQ(tabsFrameNode->GetTag(), V2::TABS_ETS_TAG);
    auto tabBarFrameNode = AceType::DynamicCast<FrameNode>(tabsFrameNode->GetChildAtIndex(TEST_TAB_BAR_INDEX));
    ASSERT_NE(tabBarFrameNode, nullptr);
    EXPECT_EQ(tabBarFrameNode->GetTag(), V2::TAB_BAR_ETS_TAG);
    auto tabBarPattern = tabBarFrameNode->GetPattern<TabBarPattern>();
    ASSERT_NE(tabBarPattern, nullptr);
    auto paintProperty = tabBarPattern->CreatePaintProperty();
    ASSERT_NE(paintProperty, nullptr);
    IndicatorStyle indicator;
    tabBarPattern->SetIndicatorStyle(indicator, 0);
    EXPECT_EQ(tabBarPattern->indicatorStyles_[0], indicator);
    SelectedMode selectedMode = SelectedMode::INDICATOR;
    tabBarPattern->SetSelectedMode(selectedMode, 0);
    EXPECT_EQ(tabBarPattern->selectedModes_[0], selectedMode);
    auto paintMethod = tabBarPattern->CreateNodePaintMethod();
    ASSERT_NE(paintMethod, nullptr);

    tabBarPattern->indicator_ = 1;
    auto paintMethod2 = tabBarPattern->CreateNodePaintMethod();
    ASSERT_EQ(paintMethod2, nullptr);

    IndicatorStyle style;
    tabBarPattern->SetIndicatorStyle(style, 1);
    auto paintMethod3 = tabBarPattern->CreateNodePaintMethod();
    ASSERT_EQ(paintMethod3, nullptr);

    PaintWrapper paintWrapper(tabBarFrameNode->GetRenderContext(), tabBarFrameNode->GetGeometryNode(), paintProperty);
    auto contentModifier = paintMethod->GetContentModifier(&paintWrapper);
    EXPECT_NE(contentModifier, nullptr);
}

/**
 * @tc.name: TabBarPaintMethodUpdateContentModifier001
 * @tc.desc: test UpdateContentModifier
 * @tc.type: FUNC
 */
HWTEST_F(TabsTestNg, TabBarPaintMethodUpdateContentModifier001, TestSize.Level1)
{
    MockPipelineContextGetTheme();
    TabsModelNG tabsModel;
    tabsModel.Create(BarPosition::START, 1, nullptr, nullptr);

    auto tabsFrameNode = AceType::DynamicCast<TabsNode>(ViewStackProcessor::GetInstance()->Finish());

    ASSERT_NE(tabsFrameNode, nullptr);
    EXPECT_EQ(tabsFrameNode->GetTag(), V2::TABS_ETS_TAG);
    auto tabBarFrameNode = AceType::DynamicCast<FrameNode>(tabsFrameNode->GetChildAtIndex(TEST_TAB_BAR_INDEX));
    ASSERT_NE(tabBarFrameNode, nullptr);
    EXPECT_EQ(tabBarFrameNode->GetTag(), V2::TAB_BAR_ETS_TAG);
    auto tabBarPattern = tabBarFrameNode->GetPattern<TabBarPattern>();
    ASSERT_NE(tabBarPattern, nullptr);
    auto paintProperty = tabBarPattern->CreatePaintProperty();
    ASSERT_NE(paintProperty, nullptr);
    IndicatorStyle indicator;
    tabBarPattern->SetIndicatorStyle(indicator, 0);
    EXPECT_EQ(tabBarPattern->indicatorStyles_[0], indicator);
    SelectedMode selectedMode = SelectedMode::INDICATOR;
    tabBarPattern->SetSelectedMode(selectedMode, 0);
    EXPECT_EQ(tabBarPattern->selectedModes_[0], selectedMode);
    auto paintMethod = tabBarPattern->CreateNodePaintMethod();
    ASSERT_NE(paintMethod, nullptr);

    PaintWrapper paintWrapper(tabBarFrameNode->GetRenderContext(), tabBarFrameNode->GetGeometryNode(), paintProperty);
    paintMethod->UpdateContentModifier(&paintWrapper);
    auto tabBarPaintMethod = Ace::AceType::DynamicCast<TabBarPaintMethod>(paintMethod);
    ASSERT_NE(tabBarPaintMethod, nullptr);
    EXPECT_NE(tabBarPaintMethod->tabBarModifier_, nullptr);

    PaintWrapper paintWrapper2(tabBarFrameNode->GetRenderContext(), tabBarFrameNode->GetGeometryNode(), nullptr);
    paintMethod->UpdateContentModifier(&paintWrapper2);
    EXPECT_NE(tabBarPaintMethod->tabBarModifier_, nullptr);
}

/**
 * @tc.name: TabBarPaintMethodPaintGradient001
 * @tc.desc: test PaintGradient
 * @tc.type: FUNC
 */
HWTEST_F(TabsTestNg, TabBarPaintMethodPaintGradient001, TestSize.Level1)
{
    MockPipelineContextGetTheme();
    TabsModelNG tabsModel;
    tabsModel.Create(BarPosition::START, 1, nullptr, nullptr);

    auto tabsFrameNode = AceType::DynamicCast<TabsNode>(ViewStackProcessor::GetInstance()->Finish());

    ASSERT_NE(tabsFrameNode, nullptr);
    EXPECT_EQ(tabsFrameNode->GetTag(), V2::TABS_ETS_TAG);
    auto tabBarFrameNode = AceType::DynamicCast<FrameNode>(tabsFrameNode->GetChildAtIndex(TEST_TAB_BAR_INDEX));
    ASSERT_NE(tabBarFrameNode, nullptr);
    EXPECT_EQ(tabBarFrameNode->GetTag(), V2::TAB_BAR_ETS_TAG);
    auto tabBarPattern = tabBarFrameNode->GetPattern<TabBarPattern>();
    ASSERT_NE(tabBarPattern, nullptr);
    auto paintProperty = tabBarPattern->CreatePaintProperty();
    ASSERT_NE(paintProperty, nullptr);
    IndicatorStyle indicator;
    tabBarPattern->SetIndicatorStyle(indicator, 0);
    SelectedMode selectedMode = SelectedMode::INDICATOR;
    EXPECT_EQ(tabBarPattern->indicatorStyles_[0], indicator);
    tabBarPattern->SetSelectedMode(selectedMode, 0);
    EXPECT_EQ(tabBarPattern->selectedModes_[0], selectedMode);
    std::vector<bool> gradientRegions = { true, true, true, true, true };
    auto tabBarModifier = AceType::MakeRefPtr<TabBarModifier>();
    auto paintMethod = AceType::MakeRefPtr<TabBarPaintMethod>(
        tabBarModifier, gradientRegions, Color::BLACK, indicator, 0.0, selectedMode);
    ASSERT_NE(paintMethod, nullptr);

    PaintWrapper paintWrapper(tabBarFrameNode->GetRenderContext(), tabBarFrameNode->GetGeometryNode(), paintProperty);
    auto drawFunction = paintMethod->GetForegroundDrawFunction(&paintWrapper);
    ASSERT_NE(drawFunction, nullptr);
    Testing::MockCanvas rsCanvas;
    EXPECT_CALL(rsCanvas, DetachPen()).WillRepeatedly(ReturnRef(rsCanvas));
    EXPECT_CALL(rsCanvas, AttachBrush(_)).WillRepeatedly(ReturnRef(rsCanvas));
    EXPECT_CALL(rsCanvas, DrawRect(_)).WillRepeatedly(Return());
    drawFunction(rsCanvas);
    EXPECT_TRUE(paintMethod->gradientRegions_[0]);

    gradientRegions[0] = false;
    paintMethod = AceType::MakeRefPtr<TabBarPaintMethod>(
        tabBarModifier, gradientRegions, Color::BLACK, indicator, 0.0, selectedMode);
    ASSERT_NE(paintMethod, nullptr);
    PaintWrapper paintWrapper2(tabBarFrameNode->GetRenderContext(), tabBarFrameNode->GetGeometryNode(), paintProperty);
    drawFunction = paintMethod->GetForegroundDrawFunction(&paintWrapper2);
    ASSERT_NE(drawFunction, nullptr);
    drawFunction(rsCanvas);
    EXPECT_FALSE(paintMethod->gradientRegions_[0]);
}

/**
 * @tc.name: TabBarAccessibilityPropertyTestNg001
 * @tc.desc: Test the ItemCount property of TabBar.
 * @tc.type: FUNC
 */
HWTEST_F(TabsTestNg, TabBarAccessibilityPropertyTestNg001, TestSize.Level1)
{
    TabContentModelNG tabContentModel;
    tabContentModel.Create();
    auto tabContentFrameNode = AceType::DynamicCast<TabContentNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(tabContentFrameNode, nullptr);

    TabsModelNG tabsModel;
    tabsModel.Create(BarPosition::START, 1, nullptr, nullptr);
    auto tabsNode = AceType::DynamicCast<TabsNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(tabsNode, nullptr);

    auto swiperNode = AceType::DynamicCast<FrameNode>(tabsNode->GetChildAtIndex(TEST_SWIPER_INDEX));
    ASSERT_NE(swiperNode, nullptr);
    tabContentFrameNode->MountToParent(swiperNode);

    auto tabBarNode = AceType::DynamicCast<FrameNode>(tabsNode->GetChildAtIndex(TEST_TAB_BAR_INDEX));
    ASSERT_NE(tabBarNode, nullptr);

    auto accessibilityProperty = tabBarNode->GetAccessibilityProperty<TabBarAccessibilityProperty>();
    ASSERT_NE(accessibilityProperty, nullptr);
    EXPECT_EQ(accessibilityProperty->GetCollectionItemCounts(), DEFAULT_ITEMCOUNT);
}

/**
 * @tc.name: TabBarAccessibilityPropertyTestNg002
 * @tc.desc: Test the IsScrollable property of TabBar.
 * @tc.type: FUNC
 */
HWTEST_F(TabsTestNg, TabBarAccessibilityPropertyTestNg002, TestSize.Level1)
{
    TabContentModelNG tabContentModel1;
    tabContentModel1.Create();
    auto tabContentFrameNode1 =
        AceType::DynamicCast<TabContentNode>(ViewStackProcessor::GetInstance()->GetMainFrameNode());
    ASSERT_NE(tabContentFrameNode1, nullptr);

    TabContentModelNG tabContentModel2;
    tabContentModel2.Create();
    auto tabContentFrameNode2 =
        AceType::DynamicCast<TabContentNode>(ViewStackProcessor::GetInstance()->GetMainFrameNode());
    ASSERT_NE(tabContentFrameNode2, nullptr);

    TabsModelNG tabsModel;
    tabsModel.Create(BarPosition::START, 1, nullptr, nullptr);
    auto tabsNode = AceType::DynamicCast<TabsNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(tabsNode, nullptr);

    auto tabBarNode = AceType::DynamicCast<FrameNode>(tabsNode->GetChildAtIndex(TEST_TAB_BAR_INDEX));
    ASSERT_NE(tabBarNode, nullptr);
    auto tabBarLayoutProperty = tabBarNode->GetLayoutProperty<TabBarLayoutProperty>();
    ASSERT_NE(tabBarLayoutProperty, nullptr);
    tabBarLayoutProperty->UpdateTabBarMode(TabBarMode::SCROLLABLE);
    auto accessibilityProperty = tabBarNode->GetAccessibilityProperty<TabBarAccessibilityProperty>();
    ASSERT_NE(accessibilityProperty, nullptr);
    EXPECT_FALSE(accessibilityProperty->IsScrollable());

    auto swiperNode = AceType::DynamicCast<FrameNode>(tabsNode->GetChildAtIndex(TEST_SWIPER_INDEX));
    ASSERT_NE(swiperNode, nullptr);
    tabContentFrameNode1->MountToParent(swiperNode);
    tabContentFrameNode2->MountToParent(swiperNode);
    EXPECT_TRUE(accessibilityProperty->IsScrollable());

    tabBarLayoutProperty->UpdateTabBarMode(TabBarMode::FIXED);
    EXPECT_FALSE(accessibilityProperty->IsScrollable());

    tabBarLayoutProperty->UpdateTabBarMode(TabBarMode::FIXED_START);
    EXPECT_FALSE(accessibilityProperty->IsScrollable());
}

/**
 * @tc.name: TabBarAccessibilityPropertyTestNg003
 * @tc.desc: Test the index properties of tabbar.
 * @tc.type: FUNC
 */
HWTEST_F(TabsTestNg, TabBarAccessibilityPropertyTestNg003, TestSize.Level1)
{
    TabsModelNG tabsModel;
    tabsModel.Create(BarPosition::START, 1, nullptr, nullptr);
    auto tabsNode = AceType::DynamicCast<TabsNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(tabsNode, nullptr);

    auto tabBarNode = AceType::DynamicCast<FrameNode>(tabsNode->GetChildAtIndex(TEST_TAB_BAR_INDEX));
    ASSERT_NE(tabBarNode, nullptr);
    auto tabBarAccessibilityProperty = tabBarNode->GetAccessibilityProperty<TabBarAccessibilityProperty>();
    ASSERT_NE(tabBarAccessibilityProperty, nullptr);
    EXPECT_EQ(tabBarAccessibilityProperty->GetBeginIndex(), DEFAULT_INDEX);
    EXPECT_EQ(tabBarAccessibilityProperty->GetCurrentIndex(), DEFAULT_INDEX);
    EXPECT_EQ(tabBarAccessibilityProperty->GetEndIndex(), DEFAULT_INDEX);

    TabContentModelNG tabContentModel;
    tabContentModel.Create();
    auto tabContentFrameNode = AceType::DynamicCast<TabContentNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(tabContentFrameNode, nullptr);

    auto swiperNode = AceType::DynamicCast<FrameNode>(tabsNode->GetChildAtIndex(TEST_SWIPER_INDEX));
    ASSERT_NE(swiperNode, nullptr);
    tabContentFrameNode->MountToParent(swiperNode);

    auto pattern = tabBarNode->GetPattern<TabBarPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->SetIndicator(CURRENT_INDEX);

    EXPECT_EQ(tabBarAccessibilityProperty->GetBeginIndex(), BEGIN_INDEX);
    EXPECT_EQ(tabBarAccessibilityProperty->GetCurrentIndex(), CURRENT_INDEX);
    EXPECT_EQ(tabBarAccessibilityProperty->GetEndIndex(), END_INDEX);
}

/**
 * @tc.name: TabBarAccessibilityPropertyTestNg004
 * @tc.desc: Test the SupportActions property of tabbar.
 * @tc.type: FUNC
 */
HWTEST_F(TabsTestNg, TabBarAccessibilityPropertyTestNg004, TestSize.Level1)
{
    TabContentModelNG tabContentModel1;
    tabContentModel1.Create();
    auto tabContentFrameNode1 =
        AceType::DynamicCast<TabContentNode>(ViewStackProcessor::GetInstance()->GetMainFrameNode());
    ASSERT_NE(tabContentFrameNode1, nullptr);

    TabContentModelNG tabContentModel2;
    tabContentModel2.Create();
    auto tabContentFrameNode2 =
        AceType::DynamicCast<TabContentNode>(ViewStackProcessor::GetInstance()->GetMainFrameNode());
    ASSERT_NE(tabContentFrameNode2, nullptr);

    TabsModelNG tabsModel;
    tabsModel.Create(BarPosition::START, 1, nullptr, nullptr);
    auto tabsNode = AceType::DynamicCast<TabsNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(tabsNode, nullptr);

    auto swiperNode = AceType::DynamicCast<FrameNode>(tabsNode->GetChildAtIndex(TEST_SWIPER_INDEX));
    ASSERT_NE(swiperNode, nullptr);
    tabContentFrameNode1->MountToParent(swiperNode);
    tabContentFrameNode2->MountToParent(swiperNode);

    auto tabBarNode = AceType::DynamicCast<FrameNode>(tabsNode->GetChildAtIndex(TEST_TAB_BAR_INDEX));
    ASSERT_NE(tabBarNode, nullptr);
    RefPtr<GeometryNode> geometryNode_ = Ace::Referenced::MakeRefPtr<GeometryNode>();
    tabBarNode->SetGeometryNode(geometryNode_);
    auto tabBarLayoutProperty = tabBarNode->GetLayoutProperty<TabBarLayoutProperty>();
    ASSERT_NE(tabBarLayoutProperty, nullptr);
    tabBarLayoutProperty->UpdateTabBarMode(TabBarMode::SCROLLABLE);

    auto tabBarPattern = tabBarNode->GetPattern<TabBarPattern>();
    ASSERT_NE(tabBarPattern, nullptr);
    tabBarPattern->currentOffset_ = DEFAULT_OFFSET;
    tabBarPattern->tabItemOffsets_.emplace_back(CURRENT_OFFSET);

    auto accessibilityProperty = tabBarNode->GetAccessibilityProperty<TabBarAccessibilityProperty>();
    ASSERT_NE(accessibilityProperty, nullptr);
    accessibilityProperty->ResetSupportAction();
    std::unordered_set<AceAction> supportAceActions = accessibilityProperty->GetSupportAction();
    uint64_t actions = 0, exptectActions = 0;
    exptectActions |= 1UL << static_cast<uint32_t>(AceAction::ACTION_SCROLL_FORWARD);
    exptectActions |= 1UL << static_cast<uint32_t>(AceAction::ACTION_SCROLL_BACKWARD);
    for (auto action : supportAceActions) {
        actions |= 1UL << static_cast<uint32_t>(action);
    }
    EXPECT_EQ(actions, exptectActions);
}
/**
 * @tc.name: TabsModelSetBarOverlap001
 * @tc.desc: test SetBarOverlap
 * @tc.type: FUNC
 */
HWTEST_F(TabsTestNg, TabsModelSetBarOverlap001, TestSize.Level1)
{
    MockPipelineContextGetTheme();
    EXPECT_CALL(*MockPipelineBase::GetCurrent(), AddScheduleTask(_)).WillRepeatedly(Return(0));
    EXPECT_CALL(*MockPipelineBase::GetCurrent(), RemoveScheduleTask(_)).WillRepeatedly(Return());

    /**
     * @tc.steps: steps1. Create tabsModel
     */
    TabsModelNG tabsModel;
    tabsModel.Create(BarPosition::START, 1, nullptr, nullptr);
    auto tabsNode = AceType::DynamicCast<TabsNode>(ViewStackProcessor::GetInstance()->GetMainElementNode());
    ASSERT_NE(tabsNode, nullptr);

    auto layoutProperty = tabsNode->GetLayoutProperty<TabsLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
   
    /**
     * @tc.steps: steps2. SetBarOverlap true
     * @tc.expected: steps2. Check the BarOverlap property value
     */
    tabsModel.SetBarOverlap(true);
    EXPECT_TRUE(layoutProperty->GetBarOverlap().value());

    tabsModel.SetBarOverlap(false);
    EXPECT_FALSE(layoutProperty->GetBarOverlap().value());

    auto tabsRenderContext = tabsNode->GetRenderContext();
    ASSERT_NE(tabsRenderContext, nullptr);
    tabsRenderContext->UpdateBackgroundColor(Color::RED);

    tabsModel.SetBarOverlap(true);
    EXPECT_TRUE(layoutProperty->GetBarOverlap().value());

    tabsModel.SetBarOverlap(false);
    EXPECT_FALSE(layoutProperty->GetBarOverlap().value());
}

/**
 * @tc.name: TabsEventHubChangeEvent001
 * @tc.desc: Tabs OnChangeEvent
 * @tc.type: FUNC
 */
HWTEST_F(TabsTestNg, TabsEventHubChangeEvent001, TestSize.Level1)
{
    MockPipelineContextGetTheme();
    TabsModelNG tabsModel;
    TabsItemDivider divider;
    Dimension strokeWidth = 10.0_vp;
    Dimension startMargin = 3.0_vp;
    Dimension endMargin = 4.0_vp;
    Color color = Color::BLACK;

    divider.strokeWidth = strokeWidth;
    divider.startMargin = startMargin;
    divider.endMargin = endMargin;
    divider.color = color;
    divider.isNull = false;

    ViewStackProcessor::GetInstance()->StartGetAccessRecordingFor(100);
    tabsModel.Create(BarPosition::START, 1, nullptr, nullptr);
    tabsModel.SetOnChangeEvent([](const BaseEventInfo* info) {});
    tabsModel.SetDivider(divider);
    auto frameNode = AceType::DynamicCast<TabsNode>(ViewStackProcessor::GetInstance()->GetMainElementNode());
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TabsPattern>();
    ASSERT_NE(pattern, nullptr);
    ASSERT_NE(pattern->onIndexChangeEvent_, nullptr);
}

void BuildTabBar(const RefPtr<TabsNode>& tabsNode, TabBarStyle tabBarStyle1, TabBarStyle tabBarStyle2)
{
    const std::string text_test = "text_test";
    TabContentModelNG tabContentModel;
    SelectedMode selectedMode = SelectedMode::INDICATOR;
    tabContentModel.Create();
    tabContentModel.SetSelectedMode(selectedMode);
    auto tabContentFrameNode =
        AceType::DynamicCast<TabContentNode>(ViewStackProcessor::GetInstance()->GetMainFrameNode());
    ASSERT_NE(tabContentFrameNode, nullptr);
    auto tabContentPattern = tabContentFrameNode->GetPattern<TabContentPattern>();
    ASSERT_NE(tabContentPattern, nullptr);

    auto swiperNode = AceType::DynamicCast<FrameNode>(tabsNode->GetChildAtIndex(TEST_SWIPER_INDEX));
    ASSERT_NE(swiperNode, nullptr);
    EXPECT_EQ(swiperNode->GetTag(), V2::SWIPER_ETS_TAG);
    tabContentFrameNode->MountToParent(swiperNode);
    tabContentPattern->SetTabBar(text_test, "", nullptr);
    tabContentPattern->SetTabBarStyle(tabBarStyle1);
    EXPECT_EQ(tabContentPattern->GetTabBarParam().GetText(), text_test);
    tabContentModel.AddTabBarItem(tabContentFrameNode, 0, false);

    TabContentModelNG tabContentModel2;
    tabContentModel2.Create();
    tabContentModel2.SetSelectedMode(selectedMode);
    auto tabContentFrameNode2 =
        AceType::DynamicCast<TabContentNode>(ViewStackProcessor::GetInstance()->GetMainFrameNode());
    ASSERT_NE(tabContentFrameNode2, nullptr);
    auto tabContentPattern2 = tabContentFrameNode2->GetPattern<TabContentPattern>();
    ASSERT_NE(tabContentPattern2, nullptr);
    tabContentFrameNode2->MountToParent(swiperNode);
    tabContentPattern2->SetTabBar(text_test, "", nullptr);
    tabContentPattern2->SetTabBarStyle(tabBarStyle2);
    EXPECT_EQ(tabContentPattern2->GetTabBarParam().GetText(), text_test);
    tabContentModel2.AddTabBarItem(tabContentFrameNode2, 1, false);
}

/**
 * @tc.name: TabBarPatternHandleBottomTabBarChange001
 * @tc.desc: test HandleBottomTabBarChange
 * @tc.type: FUNC
 */
HWTEST_F(TabsTestNg, TabBarPatternHandleBottomTabBarChange001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. build two bottom style tabbar.
     */
    MockPipelineContextGetTheme();
    EXPECT_CALL(*MockPipelineBase::GetCurrent(), AddScheduleTask(_)).WillRepeatedly(Return(0));
    EXPECT_CALL(*MockPipelineBase::GetCurrent(), RemoveScheduleTask(_)).Times(AnyNumber());

    TabsModelNG tabsModel;
    tabsModel.Create(BarPosition::START, 1, nullptr, nullptr);
    auto tabsNode = AceType::DynamicCast<TabsNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(tabsNode, nullptr);
    auto tabBarNode = AceType::DynamicCast<FrameNode>(tabsNode->GetChildAtIndex(TEST_TAB_BAR_INDEX));
    ASSERT_NE(tabBarNode, nullptr);
    auto tabBarPattern = tabBarNode->GetPattern<TabBarPattern>();
    ASSERT_NE(tabBarPattern, nullptr);

    BuildTabBar(tabsNode, TabBarStyle::BOTTOMTABBATSTYLE, TabBarStyle::BOTTOMTABBATSTYLE);

    /**
     * @tc.steps: step2. call HandleBottomTabBarChange function.
     * @tc.expected: step2. expect The function is run ok.
     */
    tabBarPattern->SetTabBarStyle(TabBarStyle::BOTTOMTABBATSTYLE, 0);
    tabBarPattern->SetTabBarStyle(TabBarStyle::BOTTOMTABBATSTYLE, 1);
    tabBarPattern->indicator_ = 0;
    tabBarPattern->HandleBottomTabBarChange(1);
    EXPECT_EQ(tabBarPattern->indicator_, 0);
}

/**
 * @tc.name: TabBarPatternHandleBottomTabBarChange002
 * @tc.desc: test HandleBottomTabBarChange
 * @tc.type: FUNC
 */
HWTEST_F(TabsTestNg, TabBarPatternHandleBottomTabBarChange002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. build a bottom style tabbar and a sub style tabbar.
     */
    MockPipelineContextGetTheme();
    EXPECT_CALL(*MockPipelineBase::GetCurrent(), AddScheduleTask(_)).WillRepeatedly(Return(0));
    EXPECT_CALL(*MockPipelineBase::GetCurrent(), RemoveScheduleTask(_)).Times(AnyNumber());

    TabsModelNG tabsModel;
    tabsModel.Create(BarPosition::START, 1, nullptr, nullptr);
    auto tabsNode = AceType::DynamicCast<TabsNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(tabsNode, nullptr);
    auto tabBarNode = AceType::DynamicCast<FrameNode>(tabsNode->GetChildAtIndex(TEST_TAB_BAR_INDEX));
    ASSERT_NE(tabBarNode, nullptr);
    auto tabBarPattern = tabBarNode->GetPattern<TabBarPattern>();
    ASSERT_NE(tabBarPattern, nullptr);

    BuildTabBar(tabsNode, TabBarStyle::BOTTOMTABBATSTYLE, TabBarStyle::SUBTABBATSTYLE);

    /**
     * @tc.steps: step2. call HandleBottomTabBarChange function.
     * @tc.expected: step2. expect The function is run ok.
     */
    tabBarPattern->SetTabBarStyle(TabBarStyle::BOTTOMTABBATSTYLE, 0);
    tabBarPattern->SetTabBarStyle(TabBarStyle::SUBTABBATSTYLE, 1);
    tabBarPattern->indicator_ = 0;
    tabBarPattern->HandleBottomTabBarChange(1);
    EXPECT_EQ(tabBarPattern->indicator_, 0);
}

/**
 * @tc.name: TabBarPatternHandleBottomTabBarChange003
 * @tc.desc: test HandleBottomTabBarChange
 * @tc.type: FUNC
 */
HWTEST_F(TabsTestNg, TabBarPatternHandleBottomTabBarChange003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. build a bottom style tabbar and a sub style tabbar.
     */
    MockPipelineContextGetTheme();
    EXPECT_CALL(*MockPipelineBase::GetCurrent(), AddScheduleTask(_)).WillRepeatedly(Return(0));
    EXPECT_CALL(*MockPipelineBase::GetCurrent(), RemoveScheduleTask(_)).Times(AnyNumber());

    TabsModelNG tabsModel;
    tabsModel.Create(BarPosition::START, 0, nullptr, nullptr);
    auto tabsNode = AceType::DynamicCast<TabsNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(tabsNode, nullptr);
    auto tabBarNode = AceType::DynamicCast<FrameNode>(tabsNode->GetChildAtIndex(TEST_TAB_BAR_INDEX));
    ASSERT_NE(tabBarNode, nullptr);
    auto tabBarPattern = tabBarNode->GetPattern<TabBarPattern>();
    ASSERT_NE(tabBarPattern, nullptr);

    BuildTabBar(tabsNode, TabBarStyle::BOTTOMTABBATSTYLE, TabBarStyle::SUBTABBATSTYLE);

    /**
     * @tc.steps: step2. call HandleBottomTabBarChange function.
     * @tc.expected: step2. expect The function is run ok.
     */
    tabBarPattern->SetTabBarStyle(TabBarStyle::BOTTOMTABBATSTYLE, 0);
    tabBarPattern->SetTabBarStyle(TabBarStyle::SUBTABBATSTYLE, 1);
    tabBarPattern->indicator_ = 1;
    tabBarPattern->HandleBottomTabBarChange(0);
    EXPECT_EQ(tabBarPattern->indicator_, 1);
}

/**
 * @tc.name: TabBarPatternHandleBottomTabBarChange004
 * @tc.desc: test HandleBottomTabBarChange
 * @tc.type: FUNC
 */
HWTEST_F(TabsTestNg, TabBarPatternHandleBottomTabBarChange004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. build a sub style tabbar and a bottom style tabbar.
     */
    MockPipelineContextGetTheme();
    EXPECT_CALL(*MockPipelineBase::GetCurrent(), AddScheduleTask(_)).WillRepeatedly(Return(0));
    EXPECT_CALL(*MockPipelineBase::GetCurrent(), RemoveScheduleTask(_)).Times(AnyNumber());

    TabsModelNG tabsModel;
    tabsModel.Create(BarPosition::START, 1, nullptr, nullptr);
    auto tabsNode = AceType::DynamicCast<TabsNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(tabsNode, nullptr);
    auto tabBarNode = AceType::DynamicCast<FrameNode>(tabsNode->GetChildAtIndex(TEST_TAB_BAR_INDEX));
    ASSERT_NE(tabBarNode, nullptr);
    auto tabBarPattern = tabBarNode->GetPattern<TabBarPattern>();
    ASSERT_NE(tabBarPattern, nullptr);

    BuildTabBar(tabsNode, TabBarStyle::SUBTABBATSTYLE, TabBarStyle::BOTTOMTABBATSTYLE);

    /**
     * @tc.steps: step2. call HandleBottomTabBarChange function.
     * @tc.expected: step2. expect The function is run ok.
     */
    tabBarPattern->SetTabBarStyle(TabBarStyle::SUBTABBATSTYLE, 0);
    tabBarPattern->SetTabBarStyle(TabBarStyle::BOTTOMTABBATSTYLE, 1);
    tabBarPattern->indicator_ = 0;
    tabBarPattern->HandleBottomTabBarChange(0);
    EXPECT_EQ(tabBarPattern->indicator_, 0);
}

/**
 * @tc.name: TabBarPatternHandleBottomTabBarChange005
 * @tc.desc: test HandleBottomTabBarChange
 * @tc.type: FUNC
 */
HWTEST_F(TabsTestNg, TabBarPatternHandleBottomTabBarChange005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. build two sub style tabbar.
     */
    MockPipelineContextGetTheme();
    EXPECT_CALL(*MockPipelineBase::GetCurrent(), AddScheduleTask(_)).WillRepeatedly(Return(0));
    EXPECT_CALL(*MockPipelineBase::GetCurrent(), RemoveScheduleTask(_)).Times(AnyNumber());

    TabsModelNG tabsModel;
    tabsModel.Create(BarPosition::START, 1, nullptr, nullptr);
    auto tabsNode = AceType::DynamicCast<TabsNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(tabsNode, nullptr);
    auto tabBarNode = AceType::DynamicCast<FrameNode>(tabsNode->GetChildAtIndex(TEST_TAB_BAR_INDEX));
    ASSERT_NE(tabBarNode, nullptr);
    auto tabBarPattern = tabBarNode->GetPattern<TabBarPattern>();
    ASSERT_NE(tabBarPattern, nullptr);

    BuildTabBar(tabsNode, TabBarStyle::SUBTABBATSTYLE, TabBarStyle::SUBTABBATSTYLE);

    /**
     * @tc.steps: step2. call HandleBottomTabBarChange function.
     * @tc.expected: step2. expect The function is run ok.
     */
    tabBarPattern->SetTabBarStyle(TabBarStyle::SUBTABBATSTYLE, 0);
    tabBarPattern->SetTabBarStyle(TabBarStyle::SUBTABBATSTYLE, 1);
    tabBarPattern->indicator_ = 0;
    tabBarPattern->HandleBottomTabBarChange(1);
    EXPECT_EQ(tabBarPattern->indicator_, 0);
}

/**
 * @tc.name: TabBarPatternMaskAnimationFinish001
 * @tc.desc: test MaskAnimationFinish
 * @tc.type: FUNC
 */
HWTEST_F(TabsTestNg, TabBarPatternMaskAnimationFinish001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. build two bottom style tabbar.
     */
    MockPipelineContextGetTheme();
    EXPECT_CALL(*MockPipelineBase::GetCurrent(), AddScheduleTask(_)).WillRepeatedly(Return(0));
    EXPECT_CALL(*MockPipelineBase::GetCurrent(), RemoveScheduleTask(_)).Times(AnyNumber());

    TabsModelNG tabsModel;
    tabsModel.Create(BarPosition::START, 1, nullptr, nullptr);
    auto tabsNode = AceType::DynamicCast<TabsNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(tabsNode, nullptr);
    auto tabBarNode = AceType::DynamicCast<FrameNode>(tabsNode->GetChildAtIndex(TEST_TAB_BAR_INDEX));
    ASSERT_NE(tabBarNode, nullptr);
    auto tabBarPattern = tabBarNode->GetPattern<TabBarPattern>();
    ASSERT_NE(tabBarPattern, nullptr);

    BuildTabBar(tabsNode, TabBarStyle::BOTTOMTABBATSTYLE, TabBarStyle::BOTTOMTABBATSTYLE);

    /**
     * @tc.steps: step2. call MaskAnimationFinish function.
     * @tc.expected: step2. expect The function is run ok.
     */
    tabBarPattern->MaskAnimationFinish(tabBarNode, 0, true);
    EXPECT_NE(tabBarNode->GetChildAtIndex(0), nullptr);
    tabBarPattern->MaskAnimationFinish(tabBarNode, 1, false);
    EXPECT_NE(tabBarNode->GetChildAtIndex(1), nullptr);
    EXPECT_EQ(tabBarPattern->indicator_, 0);
}

/**
 * @tc.name: TabBarPatternMaskAnimationFinish002
 * @tc.desc: test MaskAnimationFinish
 * @tc.type: FUNC
 */
HWTEST_F(TabsTestNg, TabBarPatternMaskAnimationFinish002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. build two bottom style tabbar.
     */
    MockPipelineContextGetTheme();
    EXPECT_CALL(*MockPipelineBase::GetCurrent(), AddScheduleTask(_)).WillRepeatedly(Return(0));
    EXPECT_CALL(*MockPipelineBase::GetCurrent(), RemoveScheduleTask(_)).Times(AnyNumber());

    TabsModelNG tabsModel;
    tabsModel.Create(BarPosition::START, 1, nullptr, nullptr);
    auto tabsNode = AceType::DynamicCast<TabsNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(tabsNode, nullptr);
    auto tabBarNode = AceType::DynamicCast<FrameNode>(tabsNode->GetChildAtIndex(TEST_TAB_BAR_INDEX));
    ASSERT_NE(tabBarNode, nullptr);
    auto tabBarPattern = tabBarNode->GetPattern<TabBarPattern>();
    ASSERT_NE(tabBarPattern, nullptr);

    BuildTabBar(tabsNode, TabBarStyle::BOTTOMTABBATSTYLE, TabBarStyle::BOTTOMTABBATSTYLE);

    /**
     * @tc.steps: step2. call MaskAnimationFinish function.
     * @tc.expected: step2. expect The function is run ok.
     */
    tabBarPattern->MaskAnimationFinish(tabBarNode, 0, false);
    EXPECT_NE(tabBarNode->GetChildAtIndex(0), nullptr);
    tabBarPattern->MaskAnimationFinish(tabBarNode, 1, true);
    EXPECT_NE(tabBarNode->GetChildAtIndex(1), nullptr);
    EXPECT_EQ(tabBarPattern->indicator_, 0);
}

/**
 * @tc.name: TabBarPatternChangeMask001
 * @tc.desc: test ChangeMask
 * @tc.type: FUNC
 */
HWTEST_F(TabsTestNg, TabBarPatternChangeMask001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. build two bottom style tabbar.
     */
    MockPipelineContextGetTheme();
    EXPECT_CALL(*MockPipelineBase::GetCurrent(), AddScheduleTask(_)).WillRepeatedly(Return(0));
    EXPECT_CALL(*MockPipelineBase::GetCurrent(), RemoveScheduleTask(_)).Times(AnyNumber());

    TabsModelNG tabsModel;
    tabsModel.Create(BarPosition::START, 1, nullptr, nullptr);
    auto tabsNode = AceType::DynamicCast<TabsNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(tabsNode, nullptr);
    auto tabBarNode = AceType::DynamicCast<FrameNode>(tabsNode->GetChildAtIndex(TEST_TAB_BAR_INDEX));
    ASSERT_NE(tabBarNode, nullptr);
    auto tabBarPattern = tabBarNode->GetPattern<TabBarPattern>();
    ASSERT_NE(tabBarPattern, nullptr);

    BuildTabBar(tabsNode, TabBarStyle::BOTTOMTABBATSTYLE, TabBarStyle::BOTTOMTABBATSTYLE);

    /**
     * @tc.steps: step2. call ChangeMask function.
     * @tc.expected: step2. expect The function is run ok.
     */
    auto tabBarGeometryNode = tabBarNode->GetGeometryNode();
    ASSERT_NE(tabBarGeometryNode, nullptr);
    auto tabBarOffset = tabBarGeometryNode->GetMarginFrameOffset();
    tabBarPattern->ChangeMask(tabBarNode, 1.0f, tabBarOffset, 1.0f, TEST_MASK_MIDDLE_RADIUS_RATIO, true);
    tabBarPattern->ChangeMask(tabBarNode, 1.0f, tabBarOffset, 0.99f, TEST_MASK_MIDDLE_RADIUS_RATIO, false);
    EXPECT_EQ(tabBarPattern->indicator_, 0);

    auto selectedmaskPosition = tabBarNode->GetChildren().size() - TEST_SELECTED_MASK_COUNT;
    auto selectedMaskNode = AceType::DynamicCast<FrameNode>(tabBarNode->GetChildAtIndex(selectedmaskPosition));
    ASSERT_NE(selectedMaskNode, nullptr);
    auto selectedImageNode = AceType::DynamicCast<FrameNode>(selectedMaskNode->GetChildren().front());
    ASSERT_NE(selectedImageNode, nullptr);
    auto selectedImageRenderContext = selectedImageNode->GetRenderContext();
    ASSERT_NE(selectedImageRenderContext, nullptr);
    EXPECT_DOUBLE_EQ(selectedImageRenderContext->GetOpacity().value(), 1.0f);
    auto unselectedmaskPosition = tabBarNode->GetChildren().size() - TEST_UNSELECTED_MASK_COUNT;
    auto unselectedMaskNode = AceType::DynamicCast<FrameNode>(tabBarNode->GetChildAtIndex(unselectedmaskPosition));
    ASSERT_NE(unselectedMaskNode, nullptr);
    auto unSelectedImageNode = AceType::DynamicCast<FrameNode>(unselectedMaskNode->GetChildren().front());
    ASSERT_NE(unSelectedImageNode, nullptr);
    auto unSelectedImageRenderContext = unSelectedImageNode->GetRenderContext();
    ASSERT_NE(unSelectedImageRenderContext, nullptr);
    EXPECT_DOUBLE_EQ(unSelectedImageRenderContext->GetOpacity().value(), 0.99f);
}

/**
 * @tc.name: TabBarPatternUpdateImageColor001
 * @tc.desc: test UpdateImageColor
 * @tc.type: FUNC
 */
HWTEST_F(TabsTestNg, TabBarPatternUpdateImageColor001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. build two bottom style tabbar.
     */
    MockPipelineContextGetTheme();
    EXPECT_CALL(*MockPipelineBase::GetCurrent(), AddScheduleTask(_)).WillRepeatedly(Return(0));
    EXPECT_CALL(*MockPipelineBase::GetCurrent(), RemoveScheduleTask(_)).Times(AnyNumber());

    TabsModelNG tabsModel;
    tabsModel.Create(BarPosition::START, 1, nullptr, nullptr);
    auto tabsNode = AceType::DynamicCast<TabsNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(tabsNode, nullptr);
    auto tabBarNode = AceType::DynamicCast<FrameNode>(tabsNode->GetChildAtIndex(TEST_TAB_BAR_INDEX));
    ASSERT_NE(tabBarNode, nullptr);
    auto tabBarPattern = tabBarNode->GetPattern<TabBarPattern>();
    ASSERT_NE(tabBarPattern, nullptr);

    BuildTabBar(tabsNode, TabBarStyle::BOTTOMTABBATSTYLE, TabBarStyle::BOTTOMTABBATSTYLE);

    /**
     * @tc.steps: step2. call UpdateImageColor function.
     * @tc.expected: step2. expect The function is run ok.
     */
    tabBarPattern->UpdateImageColor(0);
    EXPECT_NE(tabBarNode->GetChildAtIndex(0), nullptr);
    tabBarPattern->UpdateImageColor(1);
    EXPECT_NE(tabBarNode->GetChildAtIndex(1), nullptr);
    EXPECT_EQ(tabBarPattern->indicator_, 0);
    tabBarPattern->SetMaskAnimationByCreate(false);
    EXPECT_EQ(tabBarPattern->IsMaskAnimationByCreate(), false);
}

/**
 * @tc.name: TabBarLayoutAlgorithmLayoutMask001
 * @tc.desc: test LayoutMask
 * @tc.type: FUNC
 */
HWTEST_F(TabsTestNg, TabBarLayoutAlgorithmLayoutMask001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. call UpdateSelectedMask and UpdateUnselectedMask.
     */
    MockPipelineContextGetTheme();
    EXPECT_CALL(*MockPipelineBase::GetCurrent(), AddScheduleTask(_)).WillRepeatedly(Return(0));
    EXPECT_CALL(*MockPipelineBase::GetCurrent(), RemoveScheduleTask(_)).Times(AnyNumber());

    TabsModelNG tabsModel;
    tabsModel.Create(BarPosition::START, 1, nullptr, nullptr);
    TabsItemDivider divider;
    tabsModel.SetDivider(divider);
    auto tabsNode = AceType::DynamicCast<TabsNode>(ViewStackProcessor::GetInstance()->GetMainElementNode());
    ASSERT_NE(tabsNode, nullptr);
    auto tabBarNode = AceType::DynamicCast<FrameNode>(tabsNode->GetChildAtIndex(TEST_TAB_BAR_INDEX));
    ASSERT_NE(tabBarNode, nullptr);
    auto tabBarPattern = tabBarNode->GetPattern<TabBarPattern>();
    ASSERT_NE(tabBarPattern, nullptr);

    auto pattern = tabsNode->GetPattern<TabsPattern>();
    ASSERT_NE(pattern, nullptr);
    auto tabBarLayoutAlgorithm = AceType::DynamicCast<TabBarLayoutAlgorithm>(tabBarPattern->CreateLayoutAlgorithm());
    ASSERT_NE(tabBarLayoutAlgorithm, nullptr);
    auto tabBarLayoutProperty = tabBarNode->GetLayoutProperty<TabBarLayoutProperty>();
    ASSERT_NE(tabBarLayoutProperty, nullptr);
    tabBarLayoutProperty->UpdateSelectedMask(0);
    tabBarLayoutProperty->UpdateUnselectedMask(1);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    LayoutWrapper layoutWrapper = LayoutWrapper(tabBarNode, geometryNode, tabBarLayoutProperty);
    layoutWrapper.SetLayoutAlgorithm(AceType::MakeRefPtr<LayoutAlgorithmWrapper>(tabBarLayoutAlgorithm));

    /**
     * @tc.steps: step2. build selectedMaskNode and unselectedMaskNode.
     */
    auto selectedmaskPosition = tabBarNode->GetChildren().size() - TEST_SELECTED_MASK_COUNT;
    auto selectedMaskNode = AceType::DynamicCast<FrameNode>(tabBarNode->GetChildAtIndex(selectedmaskPosition));
    ASSERT_NE(selectedMaskNode, nullptr);
    RefPtr<GeometryNode> geometryNode1 = AceType::MakeRefPtr<GeometryNode>();
    RefPtr<LayoutWrapper> selectedMaskLayoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(selectedMaskNode, geometryNode1, selectedMaskNode->GetLayoutProperty());
    layoutWrapper.AppendChild(selectedMaskLayoutWrapper);

    auto unselectedmaskPosition = tabBarNode->GetChildren().size() - TEST_UNSELECTED_MASK_COUNT;
    auto unselectedMaskNode = AceType::DynamicCast<FrameNode>(tabBarNode->GetChildAtIndex(unselectedmaskPosition));
    ASSERT_NE(unselectedMaskNode, nullptr);
    RefPtr<GeometryNode> geometryNode2 = AceType::MakeRefPtr<GeometryNode>();
    RefPtr<LayoutWrapper> unselectedMaskLayoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(unselectedMaskNode, geometryNode2, unselectedMaskNode->GetLayoutProperty());
    layoutWrapper.AppendChild(unselectedMaskLayoutWrapper);

    /**
     * @tc.steps: step3. call LayoutMask function.
     * @tc.expected: step3. expect The function is run ok.
     */
    tabBarLayoutAlgorithm->LayoutMask(&layoutWrapper);
    EXPECT_EQ(tabBarLayoutProperty->GetSelectedMask().value_or(-1), 0);
    EXPECT_EQ(tabBarLayoutProperty->GetUnselectedMask().value_or(-1), 1);
}

/**
 * @tc.name: TabBarLayoutAlgorithmLayoutMask002
 * @tc.desc: test LayoutMask
 * @tc.type: FUNC
 */
HWTEST_F(TabsTestNg, TabBarLayoutAlgorithmLayoutMask002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. build selectedMaskNode and unselectedMaskNode.
     */
    MockPipelineContextGetTheme();
    EXPECT_CALL(*MockPipelineBase::GetCurrent(), AddScheduleTask(_)).WillRepeatedly(Return(0));
    EXPECT_CALL(*MockPipelineBase::GetCurrent(), RemoveScheduleTask(_)).Times(AnyNumber());

    TabsModelNG tabsModel;
    tabsModel.Create(BarPosition::START, 1, nullptr, nullptr);
    TabsItemDivider divider;
    tabsModel.SetDivider(divider);
    auto tabsNode = AceType::DynamicCast<TabsNode>(ViewStackProcessor::GetInstance()->GetMainElementNode());
    ASSERT_NE(tabsNode, nullptr);
    auto tabBarNode = AceType::DynamicCast<FrameNode>(tabsNode->GetChildAtIndex(TEST_TAB_BAR_INDEX));
    ASSERT_NE(tabBarNode, nullptr);
    auto tabBarPattern = tabBarNode->GetPattern<TabBarPattern>();
    ASSERT_NE(tabBarPattern, nullptr);

    auto pattern = tabsNode->GetPattern<TabsPattern>();
    ASSERT_NE(pattern, nullptr);
    auto tabBarLayoutAlgorithm = AceType::DynamicCast<TabBarLayoutAlgorithm>(tabBarPattern->CreateLayoutAlgorithm());
    ASSERT_NE(tabBarLayoutAlgorithm, nullptr);
    auto tabBarLayoutProperty = tabBarNode->GetLayoutProperty<TabBarLayoutProperty>();
    ASSERT_NE(tabBarLayoutProperty, nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    LayoutWrapper layoutWrapper = LayoutWrapper(tabBarNode, geometryNode, tabBarNode->GetLayoutProperty());
    layoutWrapper.SetLayoutAlgorithm(AceType::MakeRefPtr<LayoutAlgorithmWrapper>(tabBarLayoutAlgorithm));

    auto selectedmaskPosition = tabBarNode->GetChildren().size() - TEST_SELECTED_MASK_COUNT;
    auto selectedMaskNode = AceType::DynamicCast<FrameNode>(tabBarNode->GetChildAtIndex(selectedmaskPosition));
    ASSERT_NE(selectedMaskNode, nullptr);
    RefPtr<GeometryNode> geometryNode1 = AceType::MakeRefPtr<GeometryNode>();
    RefPtr<LayoutWrapper> selectedMaskLayoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(selectedMaskNode, geometryNode1, selectedMaskNode->GetLayoutProperty());
    layoutWrapper.AppendChild(selectedMaskLayoutWrapper);

    auto unselectedmaskPosition = tabBarNode->GetChildren().size() - TEST_UNSELECTED_MASK_COUNT;
    auto unselectedMaskNode = AceType::DynamicCast<FrameNode>(tabBarNode->GetChildAtIndex(unselectedmaskPosition));
    ASSERT_NE(unselectedMaskNode, nullptr);
    RefPtr<GeometryNode> geometryNode2 = AceType::MakeRefPtr<GeometryNode>();
    RefPtr<LayoutWrapper> unselectedMaskLayoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(unselectedMaskNode, geometryNode2, unselectedMaskNode->GetLayoutProperty());
    layoutWrapper.AppendChild(unselectedMaskLayoutWrapper);

    /**
     * @tc.steps: step2. call LayoutMask function.
     * @tc.expected: step2. expect The function is run ok.
     */
    tabBarLayoutAlgorithm->LayoutMask(&layoutWrapper);
    EXPECT_EQ(tabBarLayoutProperty->GetSelectedMask().value_or(-1), -1);
    EXPECT_EQ(tabBarLayoutProperty->GetUnselectedMask().value_or(-1), -1);
}
} // namespace OHOS::Ace::NG

/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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
#include "mock_navigation_route.h"
#include "mock_navigation_stack.h"

#include "core/components_ng/layout/layout_wrapper_node.h"
#include "core/components_ng/pattern/navigation/navigation_model_ng.h"
#include "core/components_ng/pattern/navigation/navigation_pattern.h"
#include "core/components_ng/pattern/navigation/title_bar_pattern.h"
#include "core/components_ng/pattern/navigation/tool_bar_pattern.h"
#include "core/components_ng/pattern/navigation/nav_bar_pattern.h"
#include "test/mock/adapter/ohos/osal/mock_system_properties.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"
#include "test/mock/frameworks/core/common/mock_theme_manager.h"
#include "test/mock/frameworks/core/common/mock_container.h"
#include "test/mock/frameworks/base/thread/mock_task_executor.h"

#include "core/components_ng/manager/navigation/navigation_manager.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {
constexpr float DEFAULT_ROOT_HEIGHT = 800.f;
constexpr float DEFAULT_ROOT_WIDTH = 480.f;

RefPtr<NavigationGroupNode> CreateNavigationWithTitle(const RefPtr<MockNavigationStack>& stack,
    NavigationTitlebarOptions options, NavigationTitleMode titleMode = NavigationTitleMode::FREE,
    std::optional<std::string> mainTitle = std::nullopt, std::optional<std::string> subTitle = std::nullopt)
{
    NavigationModelNG navigationModel;
    navigationModel.Create();
    navigationModel.SetNavigationStack(stack);
    navigationModel.SetTitleMode(titleMode);
    navigationModel.ParseCommonTitle(
        subTitle.has_value(), mainTitle.has_value(), subTitle.value_or(""), mainTitle.value_or(""));
    navigationModel.SetTitlebarOptions(std::move(options));
    return AceType::DynamicCast<NavigationGroupNode>(ViewStackProcessor::GetInstance()->Finish());
}

RefPtr<NavigationGroupNode> CreateNavigationWithToolBar(const RefPtr<MockNavigationStack>& stack,
    NavigationToolbarOptions&& options, std::vector<NG::BarItem>&& toolBarItems)
{
    NavigationModelNG navigationModel;
    navigationModel.Create();
    navigationModel.SetNavigationStack(stack);
    navigationModel.SetToolbarConfiguration(std::move(toolBarItems));
    navigationModel.SetToolbarOptions(std::move(options));
    return AceType::DynamicCast<NavigationGroupNode>(ViewStackProcessor::GetInstance()->Finish());
}

RefPtr<NavDestinationGroupNode> CreateNavDestinationWithTitle(
    NavigationTitlebarOptions options, std::optional<std::string> mainTitle, std::optional<std::string> subTitle)
{
    NavDestinationModelNG navdestinationModel;
    navdestinationModel.Create();
    navdestinationModel.ParseCommonTitle(
        subTitle.has_value(), mainTitle.has_value(), subTitle.value_or(""), mainTitle.value_or(""));
    navdestinationModel.SetTitlebarOptions(std::move(options));
    return AceType::DynamicCast<NavDestinationGroupNode>(ViewStackProcessor::GetInstance()->Finish());
}

RefPtr<NavDestinationGroupNode> CreateNavDestinationWithToolBar(
    NavigationToolbarOptions&& options, std::vector<NG::BarItem>&& toolBarItems)
{
    NavDestinationModelNG navdestinationModel;
    navdestinationModel.Create();
    navdestinationModel.SetToolbarConfiguration(std::move(toolBarItems));
    navdestinationModel.SetToolBarOptions(std::move(options));
    return AceType::DynamicCast<NavDestinationGroupNode>(ViewStackProcessor::GetInstance()->Finish());
}

void ApplyNavStrategies(const std::unordered_set<ImmersiveStrategy>& strategies)
{
    auto safeAreaManager = MockPipelineContext::GetCurrent()->GetSafeAreaManager();
    ASSERT_NE(safeAreaManager, nullptr);
    safeAreaManager->ApplyDefaultImmersiveStrategy(strategies);
}

std::optional<BarStyle> GetNavDestinationTitleBarStyle(const RefPtr<NavDestinationGroupNode>& node)
{
    auto pattern = node->GetPattern<NavDestinationPatternBase>();
    if (!pattern) {
        return std::nullopt;
    }
    return pattern->GetTitleBarStyle();
}
} // namespace

class BarStyleTestNg : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    static void RunMeasureAndLayout(RefPtr<LayoutWrapperNode>& layoutWrapper, float width = DEFAULT_ROOT_WIDTH);
};

void BarStyleTestNg::RunMeasureAndLayout(RefPtr<LayoutWrapperNode>& layoutWrapper, float width)
{
    layoutWrapper->SetActive();
    layoutWrapper->SetRootMeasureNode();
    LayoutConstraintF LayoutConstraint;
    LayoutConstraint.parentIdealSize = { width, DEFAULT_ROOT_HEIGHT };
    LayoutConstraint.percentReference = { width, DEFAULT_ROOT_HEIGHT };
    LayoutConstraint.selfIdealSize = { width, DEFAULT_ROOT_HEIGHT };
    LayoutConstraint.maxSize = { width, DEFAULT_ROOT_HEIGHT };
    layoutWrapper->Measure(LayoutConstraint);
    layoutWrapper->Layout();
    layoutWrapper->MountToHostOnMainThread();
}

void BarStyleTestNg::SetUpTestSuite()
{
    MockPipelineContext::SetUp();
    MockContainer::SetUp();
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(AceType::MakeRefPtr<NavigationBarTheme>()));
    EXPECT_CALL(*themeManager, GetTheme(_, _)).WillRepeatedly(Return(AceType::MakeRefPtr<NavigationBarTheme>()));
    MockContainer::Current()->SetNavigationRoute(AceType::MakeRefPtr<MockNavigationRoute>(""));
}

void BarStyleTestNg::TearDownTestSuite()
{
    MockPipelineContext::TearDown();
    MockContainer::TearDown();
}

/**
 * @tc.name: BarStyleTest001
 * @tc.desc: Test whether can pass 'barStyle' when creating navigation and navDestination
 * @tc.type: FUNC
 */
HWTEST_F(BarStyleTestNg, BarStyleTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. init model, create navigation with title and barStyle.
     */
    auto candidateBarStyle = BarStyle::SAFE_AREA_PADDING;
    auto mockNavPathStack = AceType::MakeRefPtr<MockNavigationStack>();
    NavigationTitlebarOptions options;
    options.brOptions.barStyle = candidateBarStyle;
    auto navigationNode = CreateNavigationWithTitle(
        mockNavPathStack, options, NavigationTitleMode::FREE, "navigation", std::nullopt);
    ASSERT_NE(navigationNode, nullptr);
    auto navBarNode = AceType::DynamicCast<NavDestinationNodeBase>(navigationNode->GetNavBarNode());
    ASSERT_NE(navBarNode, nullptr);
    auto titleBarOfNavBar = AceType::DynamicCast<TitleBarNode>(navBarNode->GetTitleBarNode());
    ASSERT_NE(titleBarOfNavBar, nullptr);
    auto titleOfNavBar = titleBarOfNavBar->GetTitle();
    ASSERT_NE(titleOfNavBar, nullptr);
    /**
    * @tc.steps: step3. check whether the barStyle is set correctlly.
    */
    // check the barStyle in navBar's title bar
    auto patternOfNavBarTitleBar = titleBarOfNavBar->GetPattern<TitleBarPattern>();
    ASSERT_NE(patternOfNavBarTitleBar, nullptr);
    auto navBarTitleBarStyle =
    patternOfNavBarTitleBar->GetTitleBarOptions().brOptions.barStyle.value_or(BarStyle::STANDARD);
    ASSERT_EQ(navBarTitleBarStyle, candidateBarStyle);
    // check the barStyle in navBar's pattern
    auto navBarPattern = navBarNode->GetPattern<NavDestinationPatternBase>();
    ASSERT_NE(navBarPattern, nullptr);
    ASSERT_EQ(navBarPattern->GetTitleBarStyle().value_or(BarStyle::STANDARD), candidateBarStyle);
    /**
    * @tc.steps: step3. create destination with barStyle.
    */
    NavigationTitlebarOptions optionsOfNavDest;
    optionsOfNavDest.brOptions.barStyle = candidateBarStyle;
    auto navDestination = CreateNavDestinationWithTitle(optionsOfNavDest, "navDestination", std::nullopt);
    ASSERT_NE(navDestination, nullptr);
    auto titleBarOfNavDest = AceType::DynamicCast<TitleBarNode>(navDestination->GetTitleBarNode());
    ASSERT_NE(titleBarOfNavDest, nullptr);
    auto titleOfNavDest = titleBarOfNavDest->GetTitle();
    ASSERT_NE(titleOfNavDest, nullptr);
    /**
    * @tc.steps: step4. check whether the barStyle of dest is set correctlly.
    */
    // check the barStyle in navDestination's title bar
    auto patternOfNavDestTitleBar = titleBarOfNavDest->GetPattern<TitleBarPattern>();
    ASSERT_NE(patternOfNavDestTitleBar, nullptr);
    auto destTitleBarStyle =
        patternOfNavDestTitleBar->GetTitleBarOptions().brOptions.barStyle.value_or(BarStyle::STANDARD);
    ASSERT_EQ(destTitleBarStyle, candidateBarStyle);
    // check the barStyle in navDestination's pattern
    auto navDestinationPattern = navDestination->GetPattern<NavDestinationPatternBase>();
    ASSERT_NE(navDestinationPattern, nullptr);
    ASSERT_EQ(navDestinationPattern->GetTitleBarStyle().value_or(BarStyle::STANDARD), candidateBarStyle);
}

/**
 * @tc.name: SafeAreaPaddingTest002
 * @tc.desc: Test whether navBar's safe area padding set without subtitle
 * @tc.type: FUNC
 */
HWTEST_F(BarStyleTestNg, SafeAreaPaddingTest002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. init model, create navigation with barStyle SAFE_AREA_PADDING and only mainTitle.
     */
    auto candidateBarStyle = BarStyle::SAFE_AREA_PADDING;
    auto mockNavPathStack = AceType::MakeRefPtr<MockNavigationStack>();
    NavigationTitlebarOptions options;
    options.brOptions.barStyle = candidateBarStyle;
    auto navigationNode = CreateNavigationWithTitle(
        mockNavPathStack, options, NavigationTitleMode::MINI, "navigationMainTitle", std::nullopt);
    ASSERT_NE(navigationNode, nullptr);
    /**
     * @tc.steps: step2. run navigation's onModifyDone and test the layout property 'safeAreaPadding'.
     */
    auto navigationPattern = navigationNode->GetPattern<NavigationPattern>();
    ASSERT_NE(navigationPattern, nullptr);
    navigationPattern->OnModifyDone();
    auto layoutWrapper = navigationNode->CreateLayoutWrapper();
    ASSERT_NE(layoutWrapper, nullptr);
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    context->taskExecutor_ = AceType::MakeRefPtr<MockTaskExecutor>();
    auto taskExecutor = context->GetTaskExecutor();
    ASSERT_NE(taskExecutor, nullptr);
    BarStyleTestNg::RunMeasureAndLayout(layoutWrapper);
    /**
     * @tc.steps: step3. check property 'safeAreaPadding'.
     */
    auto navBar = AceType::DynamicCast<NavBarNode>(navigationNode->GetNavBarNode());
    ASSERT_NE(navBar, nullptr);
    auto navBarContent = AceType::DynamicCast<FrameNode>(navBar->GetContentNode());
    ASSERT_NE(navBarContent, nullptr);
    auto contentLayoutProperty = navBarContent->GetLayoutProperty();
    ASSERT_NE(contentLayoutProperty, nullptr);
    const auto& safeAreaPadding = contentLayoutProperty->GetSafeAreaPaddingProperty();
    ASSERT_NE(safeAreaPadding, nullptr);
    ASSERT_EQ(safeAreaPadding->top.value_or(CalcLength(0.0f)),
        CalcLength(SINGLE_LINE_TITLEBAR_HEIGHT.ConvertToPx()));
}

/**
 * @tc.name: SafeAreaPaddingTest003
 * @tc.desc: Test whether navBar's safe area padding set without subtitle
 * @tc.type: FUNC
 */
HWTEST_F(BarStyleTestNg, SafeAreaPaddingTest003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. init model, create navigation with barStyle SAFE_AREA_PADDING and only mainTitle.
     */
    auto candidateBarStyle = BarStyle::SAFE_AREA_PADDING;
    auto mockNavPathStack = AceType::MakeRefPtr<MockNavigationStack>();
    NavigationTitlebarOptions options;
    options.brOptions.barStyle = candidateBarStyle;
    auto navigationNode = CreateNavigationWithTitle(
        mockNavPathStack, options, NavigationTitleMode::FULL, "navigationMainTitle", std::nullopt);
    ASSERT_NE(navigationNode, nullptr);
    /**
     * @tc.steps: step2. run navigation's onModifyDone and test the layout property 'safeAreaPadding'.
     */
    auto navigationPattern = navigationNode->GetPattern<NavigationPattern>();
    ASSERT_NE(navigationPattern, nullptr);
    navigationPattern->OnModifyDone();
    auto layoutWrapper = navigationNode->CreateLayoutWrapper();
    ASSERT_NE(layoutWrapper, nullptr);
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    context->taskExecutor_ = AceType::MakeRefPtr<MockTaskExecutor>();
    auto taskExecutor = context->GetTaskExecutor();
    ASSERT_NE(taskExecutor, nullptr);
    BarStyleTestNg::RunMeasureAndLayout(layoutWrapper);
    /**
     * @tc.steps: step3. check property 'safeAreaPadding'.
     */
    auto navBar = AceType::DynamicCast<NavBarNode>(navigationNode->GetNavBarNode());
    ASSERT_NE(navBar, nullptr);
    auto navBarContent = AceType::DynamicCast<FrameNode>(navBar->GetContentNode());
    ASSERT_NE(navBarContent, nullptr);
    auto contentLayoutProperty = navBarContent->GetLayoutProperty();
    ASSERT_NE(contentLayoutProperty, nullptr);
    const auto& safeAreaPadding = contentLayoutProperty->GetSafeAreaPaddingProperty();
    ASSERT_NE(safeAreaPadding, nullptr);
    ASSERT_EQ(safeAreaPadding->top.value_or(CalcLength(0.0f)),
        CalcLength(FULL_SINGLE_LINE_TITLEBAR_HEIGHT.ConvertToPx()));
}

/**
 * @tc.name: SafeAreaPaddingTest004
 * @tc.desc: Test whether navBar's safe area padding set without subtitle
 * @tc.type: FUNC
 */
HWTEST_F(BarStyleTestNg, SafeAreaPaddingTest004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. init model, create navigation with barStyle SAFE_AREA_PADDING and only mainTitle.
     */
    auto candidateBarStyle = BarStyle::SAFE_AREA_PADDING;
    auto mockNavPathStack = AceType::MakeRefPtr<MockNavigationStack>();
    NavigationTitlebarOptions options;
    options.brOptions.barStyle = candidateBarStyle;
    auto navigationNode = CreateNavigationWithTitle(
        mockNavPathStack, options, NavigationTitleMode::FREE, "navigationMainTitle", "navigationSubTitle");
    ASSERT_NE(navigationNode, nullptr);
    /**
     * @tc.steps: step2. run navigation's onModifyDone and test the layout property 'safeAreaPadding'.
     */
    auto navigationPattern = navigationNode->GetPattern<NavigationPattern>();
    ASSERT_NE(navigationPattern, nullptr);
    navigationPattern->OnModifyDone();
    auto layoutWrapper = navigationNode->CreateLayoutWrapper();
    ASSERT_NE(layoutWrapper, nullptr);
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    context->taskExecutor_ = AceType::MakeRefPtr<MockTaskExecutor>();
    auto taskExecutor = context->GetTaskExecutor();
    ASSERT_NE(taskExecutor, nullptr);
    BarStyleTestNg::RunMeasureAndLayout(layoutWrapper);
    /**
     * @tc.steps: step3. check property 'safeAreaPadding'.
     */
    auto navBar = AceType::DynamicCast<NavBarNode>(navigationNode->GetNavBarNode());
    ASSERT_NE(navBar, nullptr);
    auto navBarContent = AceType::DynamicCast<FrameNode>(navBar->GetContentNode());
    ASSERT_NE(navBarContent, nullptr);
    auto contentLayoutProperty = navBarContent->GetLayoutProperty();
    ASSERT_NE(contentLayoutProperty, nullptr);
    const auto& safeAreaPadding = contentLayoutProperty->GetSafeAreaPaddingProperty();
    ASSERT_NE(safeAreaPadding, nullptr);
    ASSERT_EQ(safeAreaPadding->top.value_or(CalcLength(0.0f)),
        CalcLength(FULL_DOUBLE_LINE_TITLEBAR_HEIGHT.ConvertToPx()));
}

/**
 * @tc.name: SafeAreaPaddingTest005
 * @tc.desc: Test whether navBar's safe area padding set with subtitle
 * @tc.type: FUNC
 */
HWTEST_F(BarStyleTestNg, SafeAreaPaddingTest005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. init model, create navigation with barStyle SAFE_AREA_PADDING and only mainTitle.
     */
    auto candidateBarStyle = BarStyle::SAFE_AREA_PADDING;
    auto mockNavPathStack = AceType::MakeRefPtr<MockNavigationStack>();
    NavigationTitlebarOptions options;
    options.brOptions.barStyle = candidateBarStyle;
    auto navigationNode = CreateNavigationWithTitle(
        mockNavPathStack, options, NavigationTitleMode::MINI, "navigationMainTitle", "navigationSubTitle");
    ASSERT_NE(navigationNode, nullptr);
    /**
     * @tc.steps: step2. run navigation's onModifyDone and test the layout property 'safeAreaPadding'.
     */
    auto navigationPattern = navigationNode->GetPattern<NavigationPattern>();
    ASSERT_NE(navigationPattern, nullptr);
    navigationPattern->OnModifyDone();
    auto layoutWrapper = navigationNode->CreateLayoutWrapper();
    ASSERT_NE(layoutWrapper, nullptr);
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    context->taskExecutor_ = AceType::MakeRefPtr<MockTaskExecutor>();
    auto taskExecutor = context->GetTaskExecutor();
    ASSERT_NE(taskExecutor, nullptr);
    BarStyleTestNg::RunMeasureAndLayout(layoutWrapper);
    /**
     * @tc.steps: step3. check property 'safeAreaPadding'.
     */
    auto navBar = AceType::DynamicCast<NavBarNode>(navigationNode->GetNavBarNode());
    ASSERT_NE(navBar, nullptr);
    auto navBarContent = AceType::DynamicCast<FrameNode>(navBar->GetContentNode());
    ASSERT_NE(navBarContent, nullptr);
    auto contentLayoutProperty = navBarContent->GetLayoutProperty();
    ASSERT_NE(contentLayoutProperty, nullptr);
    const auto& safeAreaPadding = contentLayoutProperty->GetSafeAreaPaddingProperty();
    ASSERT_NE(safeAreaPadding, nullptr);
    ASSERT_EQ(safeAreaPadding->top.value_or(CalcLength(0.0f)),
        CalcLength(DOUBLE_LINE_TITLEBAR_HEIGHT.ConvertToPx()));
}

/**
 * @tc.name: SafeAreaPaddingTest006
 * @tc.desc: Test whether navBar's safe area padding set with subtitle
 * @tc.type: FUNC
 */
HWTEST_F(BarStyleTestNg, SafeAreaPaddingTest006, TestSize.Level1)
{
    /**
     * @tc.steps: step1. init model, create navigation with barStyle SAFE_AREA_PADDING and only mainTitle.
     */
    auto candidateBarStyle = BarStyle::SAFE_AREA_PADDING;
    auto mockNavPathStack = AceType::MakeRefPtr<MockNavigationStack>();
    NavigationTitlebarOptions options;
    options.brOptions.barStyle = candidateBarStyle;
    auto navigationNode = CreateNavigationWithTitle(
        mockNavPathStack, options, NavigationTitleMode::FULL, "navigationMainTitle", "navigationSubTitle");
    ASSERT_NE(navigationNode, nullptr);
    /**
     * @tc.steps: step2. run navigation's onModifyDone and test the layout property 'safeAreaPadding'.
     */
    auto navigationPattern = navigationNode->GetPattern<NavigationPattern>();
    ASSERT_NE(navigationPattern, nullptr);
    navigationPattern->OnModifyDone();
    auto layoutWrapper = navigationNode->CreateLayoutWrapper();
    ASSERT_NE(layoutWrapper, nullptr);
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    context->taskExecutor_ = AceType::MakeRefPtr<MockTaskExecutor>();
    auto taskExecutor = context->GetTaskExecutor();
    ASSERT_NE(taskExecutor, nullptr);
    BarStyleTestNg::RunMeasureAndLayout(layoutWrapper);
    /**
     * @tc.steps: step3. check property 'safeAreaPadding'.
     */
    auto navBar = AceType::DynamicCast<NavBarNode>(navigationNode->GetNavBarNode());
    ASSERT_NE(navBar, nullptr);
    auto navBarContent = AceType::DynamicCast<FrameNode>(navBar->GetContentNode());
    ASSERT_NE(navBarContent, nullptr);
    auto contentLayoutProperty = navBarContent->GetLayoutProperty();
    ASSERT_NE(contentLayoutProperty, nullptr);
    const auto& safeAreaPadding = contentLayoutProperty->GetSafeAreaPaddingProperty();
    ASSERT_NE(safeAreaPadding, nullptr);
    ASSERT_EQ(safeAreaPadding->top.value_or(CalcLength(0.0f)),
        CalcLength(FULL_DOUBLE_LINE_TITLEBAR_HEIGHT.ConvertToPx()));
}

/**
 * @tc.name: SafeAreaPaddingTest007
 * @tc.desc: Test whether navDest's safe area padding set with subtitle
 * @tc.type: FUNC
 */
HWTEST_F(BarStyleTestNg, SafeAreaPaddingTest007, TestSize.Level1)
{
    /**
     * @tc.steps: step1. init model, create navdestination with barStyle SAFE_AREA_PADDING and only mainTitle.
     */
    NavigationTitlebarOptions optionsOfNavDest;
    optionsOfNavDest.brOptions.barStyle = BarStyle::SAFE_AREA_PADDING;
    auto navDestination = CreateNavDestinationWithTitle(optionsOfNavDest, "navDestination", std::nullopt);
    ASSERT_NE(navDestination, nullptr);
    auto navDestinationPattern = navDestination->GetPattern<NavDestinationPattern>();
    ASSERT_NE(navDestinationPattern, nullptr);
    navDestinationPattern->OnModifyDone();
    auto layoutWrapper = navDestination->CreateLayoutWrapper();
    ASSERT_NE(layoutWrapper, nullptr);
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    context->taskExecutor_ = AceType::MakeRefPtr<MockTaskExecutor>();
    auto taskExecutor = context->GetTaskExecutor();
    ASSERT_NE(taskExecutor, nullptr);
    BarStyleTestNg::RunMeasureAndLayout(layoutWrapper);
    auto navDestinationContent = AceType::DynamicCast<FrameNode>(navDestination->GetContentNode());
    ASSERT_NE(navDestinationContent, nullptr);
    auto contentLayoutProperty = navDestinationContent->GetLayoutProperty();
    ASSERT_NE(contentLayoutProperty, nullptr);
    const auto& safeAreaPadding = contentLayoutProperty->GetSafeAreaPaddingProperty();
    ASSERT_NE(safeAreaPadding, nullptr);
    ASSERT_EQ(safeAreaPadding->top.value_or(CalcLength(0.0f)),
        CalcLength(SINGLE_LINE_TITLEBAR_HEIGHT.ConvertToPx()));
}

/**
 * @tc.name: titleBarStyleTest001
 * @tc.desc: Branch: if (navBar.titleBarStyle == BarStyle::STANDARD && !navBar.hasSubtitle)
 *           Condition: navBar.titleBarStyle == BarStyle::STANDARD && !navBar.hasSubtitle
 * @tc.type: FUNC
 */
HWTEST_F(BarStyleTestNg, titleBarStyleTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. init model, create navigation with barStyle STANDARD and only mainTitle.
     */
    auto candidateBarStyle = BarStyle::STANDARD;
    auto mockNavPathStack = AceType::MakeRefPtr<MockNavigationStack>();
    NavigationTitlebarOptions options;
    options.brOptions.barStyle = candidateBarStyle;
    auto navigationNode = CreateNavigationWithTitle(
        mockNavPathStack, options, NavigationTitleMode::FULL, "navigationMainTitle");
    ASSERT_NE(navigationNode, nullptr);
    /**
     * @tc.steps: step2. run navigation's onModifyDone and test the layout property 'safeAreaPadding'.
     */
    auto navigationPattern = navigationNode->GetPattern<NavigationPattern>();
    ASSERT_NE(navigationPattern, nullptr);
    navigationPattern->OnModifyDone();
    auto layoutWrapper = navigationNode->CreateLayoutWrapper();
    ASSERT_NE(layoutWrapper, nullptr);
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    context->taskExecutor_ = AceType::MakeRefPtr<MockTaskExecutor>();
    auto taskExecutor = context->GetTaskExecutor();
    ASSERT_NE(taskExecutor, nullptr);
    BarStyleTestNg::RunMeasureAndLayout(layoutWrapper);
    /**
     * @tc.steps: step3. check property 'safeAreaPadding'.
     */
    auto navBar = AceType::DynamicCast<NavBarNode>(navigationNode->GetNavBarNode());
    ASSERT_NE(navBar, nullptr);
    auto navBarContent = AceType::DynamicCast<FrameNode>(navBar->GetContentNode());
    ASSERT_NE(navBarContent, nullptr);
    auto contentLayoutProperty = navBarContent->GetLayoutProperty();
    ASSERT_NE(contentLayoutProperty, nullptr);
    const auto& safeAreaPadding = contentLayoutProperty->GetSafeAreaPaddingProperty();
    /**
     * @tc.steps: step4. property 'safeAreaPadding' should be nullptr cause no SAFE_AREA_PADDING barStyle set.
     */
    ASSERT_EQ(safeAreaPadding, nullptr);
}

/**
 * @tc.name: titleBarStyleTest002
 * @tc.desc: Branch: if (navBar.titleBarStyle == BarStyle::STANDARD && navBar.hasSubtitle)
 *           Condition: navBar.titleBarStyle == BarStyle::STANDARD && navBar.hasSubtitle
 * @tc.type: FUNC
 */
HWTEST_F(BarStyleTestNg, titleBarStyleTest002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. init model, create navigation with barStyle STANDARD, main title and subtitle.
     */
    auto candidateBarStyle = BarStyle::STANDARD;
    auto mockNavPathStack = AceType::MakeRefPtr<MockNavigationStack>();
    NavigationTitlebarOptions options;
    options.brOptions.barStyle = candidateBarStyle;
    auto navigationNode = CreateNavigationWithTitle(
        mockNavPathStack, options, NavigationTitleMode::FULL, "navigationMainTitle", "navigationSubTitle");
    ASSERT_NE(navigationNode, nullptr);
    /**
     * @tc.steps: step2. run navigation's onModifyDone and test the layout property 'safeAreaPadding'.
     */
    auto navigationPattern = navigationNode->GetPattern<NavigationPattern>();
    ASSERT_NE(navigationPattern, nullptr);
    navigationPattern->OnModifyDone();
    auto layoutWrapper = navigationNode->CreateLayoutWrapper();
    ASSERT_NE(layoutWrapper, nullptr);
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    context->taskExecutor_ = AceType::MakeRefPtr<MockTaskExecutor>();
    auto taskExecutor = context->GetTaskExecutor();
    ASSERT_NE(taskExecutor, nullptr);
    BarStyleTestNg::RunMeasureAndLayout(layoutWrapper);
    /**
     * @tc.steps: step3. check property 'safeAreaPadding'.
     */
    auto navBar = AceType::DynamicCast<NavBarNode>(navigationNode->GetNavBarNode());
    ASSERT_NE(navBar, nullptr);
    auto navBarContent = AceType::DynamicCast<FrameNode>(navBar->GetContentNode());
    ASSERT_NE(navBarContent, nullptr);
    auto contentLayoutProperty = navBarContent->GetLayoutProperty();
    ASSERT_NE(contentLayoutProperty, nullptr);
    const auto& safeAreaPadding = contentLayoutProperty->GetSafeAreaPaddingProperty();
    /**
     * @tc.steps: step4. property 'safeAreaPadding' should be nullptr cause no SAFE_AREA_PADDING barStyle set.
     */
    ASSERT_EQ(safeAreaPadding, nullptr);
}

/**
 * @tc.name: titleBarStyleTest003
 * @tc.desc: Branch: if (navDestination.titleBarStyle == BarStyle::STANDARD && !navDestination.hasSubtitle)
 *           Condition: navDestination.titleBarStyle == BarStyle::STANDARD && !navDestination.hasSubtitle
 * @tc.type: FUNC
 */
HWTEST_F(BarStyleTestNg, titleBarStyleTest003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. init model, create navdestination with barStyle STANDARD and only mainTitle.
     */
    NavigationTitlebarOptions optionsOfNavDest;
    optionsOfNavDest.brOptions.barStyle = BarStyle::STANDARD;
    auto navDestination = CreateNavDestinationWithTitle(optionsOfNavDest, "navDestination", std::nullopt);
    ASSERT_NE(navDestination, nullptr);
    auto navDestinationPattern = navDestination->GetPattern<NavDestinationPattern>();
    ASSERT_NE(navDestinationPattern, nullptr);
    /**
     * @tc.steps: step2. run navDestinationPattern's onModifyDone and test the layout property 'safeAreaPadding'.
     */
    navDestinationPattern->OnModifyDone();
    auto layoutWrapper = navDestination->CreateLayoutWrapper();
    ASSERT_NE(layoutWrapper, nullptr);
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    context->taskExecutor_ = AceType::MakeRefPtr<MockTaskExecutor>();
    auto taskExecutor = context->GetTaskExecutor();
    ASSERT_NE(taskExecutor, nullptr);
    BarStyleTestNg::RunMeasureAndLayout(layoutWrapper);
    /**
     * @tc.steps: step3. check property 'safeAreaPadding'.
     */
    auto navDestinationContent = AceType::DynamicCast<FrameNode>(navDestination->GetContentNode());
    ASSERT_NE(navDestinationContent, nullptr);
    auto contentLayoutProperty = navDestinationContent->GetLayoutProperty();
    ASSERT_NE(contentLayoutProperty, nullptr);
    const auto& safeAreaPadding = contentLayoutProperty->GetSafeAreaPaddingProperty();
    /**
     * @tc.steps: step4. property 'safeAreaPadding' should be nullptr cause no SAFE_AREA_PADDING barStyle set.
     */
    ASSERT_EQ(safeAreaPadding, nullptr);
}

/**
 * @tc.name: titleBarStyleTest004
 * @tc.desc: Branch: if (navDestination.titleBarStyle == BarStyle::STANDARD && navDestination.hasSubtitle)
 *           Condition: navDestination.titleBarStyle == BarStyle::STANDARD && navDestination.hasSubtitle
 * @tc.type: FUNC
 */
HWTEST_F(BarStyleTestNg, titleBarStyleTest004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. init model, create navdestination with barStyle STANDARD and only mainTitle.
     */
    NavigationTitlebarOptions optionsOfNavDest;
    optionsOfNavDest.brOptions.barStyle = BarStyle::STANDARD;
    auto navDestination = CreateNavDestinationWithTitle(optionsOfNavDest, "navDestination", "navDestinationSub");
    ASSERT_NE(navDestination, nullptr);
    auto navDestinationPattern = navDestination->GetPattern<NavDestinationPattern>();
    ASSERT_NE(navDestinationPattern, nullptr);
    /**
     * @tc.steps: step2. run navDestinationPattern's onModifyDone and test the layout property 'safeAreaPadding'.
     */
    navDestinationPattern->OnModifyDone();
    auto layoutWrapper = navDestination->CreateLayoutWrapper();
    ASSERT_NE(layoutWrapper, nullptr);
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    context->taskExecutor_ = AceType::MakeRefPtr<MockTaskExecutor>();
    auto taskExecutor = context->GetTaskExecutor();
    ASSERT_NE(taskExecutor, nullptr);
    BarStyleTestNg::RunMeasureAndLayout(layoutWrapper);
    /**
     * @tc.steps: step3. check property 'safeAreaPadding'.
     */
    auto navDestinationContent = AceType::DynamicCast<FrameNode>(navDestination->GetContentNode());
    ASSERT_NE(navDestinationContent, nullptr);
    auto contentLayoutProperty = navDestinationContent->GetLayoutProperty();
    ASSERT_NE(contentLayoutProperty, nullptr);
    const auto& safeAreaPadding = contentLayoutProperty->GetSafeAreaPaddingProperty();
    /**
     * @tc.steps: step4. property 'safeAreaPadding' should be nullptr cause no SAFE_AREA_PADDING barStyle set.
     */
    ASSERT_EQ(safeAreaPadding, nullptr);
}


/**
 * @tc.name: titleBarStyleTest010
 * @tc.desc: Branch: if (navBar.titleBarStyle == BarStyle::STANDARD)
 *           Condition: navBar.titleBarStyle == BarStyle::STANDARD
 * @tc.type: FUNC
 */
HWTEST_F(BarStyleTestNg, titleBarStyleTest010, TestSize.Level1)
{
    /**
     * @tc.steps: step1. init model, create navigation with barStyle STANDARD, main title and subtitle.
     */
    auto candidateBarStyle = BarStyle::STANDARD;
    auto mockNavPathStack = AceType::MakeRefPtr<MockNavigationStack>();
    NavigationTitlebarOptions options;
    options.brOptions.barStyle = candidateBarStyle;
    auto navigationNode = CreateNavigationWithTitle(
        mockNavPathStack, options, NavigationTitleMode::FULL, "navigationMainTitle", "navigationSubTitle");
    ASSERT_NE(navigationNode, nullptr);
    /**
     * @tc.steps: step2. run navigation's onModifyDone and test the layout property 'safeAreaPadding'.
     */
    auto navigationPattern = navigationNode->GetPattern<NavigationPattern>();
    ASSERT_NE(navigationPattern, nullptr);
    navigationPattern->OnModifyDone();
    auto layoutWrapper = navigationNode->CreateLayoutWrapper();
    ASSERT_NE(layoutWrapper, nullptr);
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    context->taskExecutor_ = AceType::MakeRefPtr<MockTaskExecutor>();
    auto taskExecutor = context->GetTaskExecutor();
    ASSERT_NE(taskExecutor, nullptr);
    BarStyleTestNg::RunMeasureAndLayout(layoutWrapper);
    /**
     * @tc.steps: step3. check property 'safeAreaPadding'.
     */
    auto navBar = AceType::DynamicCast<NavBarNode>(navigationNode->GetNavBarNode());
    ASSERT_NE(navBar, nullptr);
    auto navBarContent = AceType::DynamicCast<FrameNode>(navBar->GetContentNode());
    ASSERT_NE(navBarContent, nullptr);
    auto geometryNode = navBarContent->GetGeometryNode();
    ASSERT_NE(geometryNode, nullptr);
    auto marginFrameOffset = geometryNode->GetMarginFrameOffset();
    /**
     * @tc.steps: step4. property 'marginFrameOffset' should be ...
     */
    ASSERT_NE(marginFrameOffset.GetY(), 0.0f);
}

/**
 * @tc.name: titleBarStyleTest011
 * @tc.desc: Branch: if (navBar.titleBarStyle == BarStyle::STACK)
 *           Condition: navBar.titleBarStyle == BarStyle::STACK
 * @tc.type: FUNC
 */
HWTEST_F(BarStyleTestNg, titleBarStyleTest011, TestSize.Level1)
{
    /**
     * @tc.steps: step1. init model, create navigation with barStyle STACK, main title and subtitle.
     */
    auto candidateBarStyle = BarStyle::STACK;
    auto mockNavPathStack = AceType::MakeRefPtr<MockNavigationStack>();
    NavigationTitlebarOptions options;
    options.brOptions.barStyle = candidateBarStyle;
    auto navigationNode = CreateNavigationWithTitle(
        mockNavPathStack, options, NavigationTitleMode::FULL, "navigationMainTitle", "navigationSubTitle");
    ASSERT_NE(navigationNode, nullptr);
    /**
     * @tc.steps: step2. run navigation's onModifyDone and test the layout property 'safeAreaPadding'.
     */
    auto navigationPattern = navigationNode->GetPattern<NavigationPattern>();
    ASSERT_NE(navigationPattern, nullptr);
    navigationPattern->OnModifyDone();
    auto layoutWrapper = navigationNode->CreateLayoutWrapper();
    ASSERT_NE(layoutWrapper, nullptr);
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    context->taskExecutor_ = AceType::MakeRefPtr<MockTaskExecutor>();
    auto taskExecutor = context->GetTaskExecutor();
    ASSERT_NE(taskExecutor, nullptr);
    BarStyleTestNg::RunMeasureAndLayout(layoutWrapper);
    /**
     * @tc.steps: step3. check property 'safeAreaPadding'.
     */
    auto navBar = AceType::DynamicCast<NavBarNode>(navigationNode->GetNavBarNode());
    ASSERT_NE(navBar, nullptr);
    auto navBarContent = AceType::DynamicCast<FrameNode>(navBar->GetContentNode());
    ASSERT_NE(navBarContent, nullptr);
    auto geometryNode = navBarContent->GetGeometryNode();
    ASSERT_NE(geometryNode, nullptr);
    auto marginFrameOffset = geometryNode->GetMarginFrameOffset();
    /**
     * @tc.steps: step4. property 'marginFrameOffset' should be ...
     */
    ASSERT_EQ(marginFrameOffset.GetY(), 0.0f);
}

/**
 * @tc.name: titleBarStyleTest012
 * @tc.desc: Branch: if (navBar.titleBarStyle == BarStyle::SAFE_AREA_PADDING)
 *           Condition: navBar.titleBarStyle == BarStyle::SAFE_AREA_PADDING
 * @tc.type: FUNC
 */
HWTEST_F(BarStyleTestNg, titleBarStyleTest012, TestSize.Level1)
{
    /**
     * @tc.steps: step1. init model, create navigation with barStyle SAFE_AREA_PADDING, main title and subtitle.
     */
    auto candidateBarStyle = BarStyle::SAFE_AREA_PADDING;
    auto mockNavPathStack = AceType::MakeRefPtr<MockNavigationStack>();
    NavigationTitlebarOptions options;
    options.brOptions.barStyle = candidateBarStyle;
    auto navigationNode = CreateNavigationWithTitle(
        mockNavPathStack, options, NavigationTitleMode::FULL, "navigationMainTitle", "navigationSubTitle");
    ASSERT_NE(navigationNode, nullptr);
    /**
     * @tc.steps: step2. run navigation's onModifyDone and test the layout property 'safeAreaPadding'.
     */
    auto navigationPattern = navigationNode->GetPattern<NavigationPattern>();
    ASSERT_NE(navigationPattern, nullptr);
    navigationPattern->OnModifyDone();
    auto layoutWrapper = navigationNode->CreateLayoutWrapper();
    ASSERT_NE(layoutWrapper, nullptr);
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    context->taskExecutor_ = AceType::MakeRefPtr<MockTaskExecutor>();
    auto taskExecutor = context->GetTaskExecutor();
    ASSERT_NE(taskExecutor, nullptr);
    BarStyleTestNg::RunMeasureAndLayout(layoutWrapper);
    /**
     * @tc.steps: step3. check property 'safeAreaPadding'.
     */
    auto navBar = AceType::DynamicCast<NavBarNode>(navigationNode->GetNavBarNode());
    ASSERT_NE(navBar, nullptr);
    auto navBarContent = AceType::DynamicCast<FrameNode>(navBar->GetContentNode());
    ASSERT_NE(navBarContent, nullptr);
    auto geometryNode = navBarContent->GetGeometryNode();
    ASSERT_NE(geometryNode, nullptr);
    auto marginFrameOffset = geometryNode->GetMarginFrameOffset();
    /**
     * @tc.steps: step4. property 'marginFrameOffset' should be ...
     */
    ASSERT_EQ(marginFrameOffset.GetY(), 0.0f);
}

/**
 * @tc.name: titleBarStyleTest013
 * @tc.desc: Branch: if (navDestination.titleBarStyle == BarStyle::STANDARD)
 *           Condition: navDestination.titleBarStyle == BarStyle::STANDARD
 * @tc.type: FUNC
 */
HWTEST_F(BarStyleTestNg, titleBarStyleTest013, TestSize.Level1)
{
    /**
     * @tc.steps: step1. init model, create navdestination with barStyle STANDARD and only mainTitle.
     */
    NavigationTitlebarOptions optionsOfNavDest;
    optionsOfNavDest.brOptions.barStyle = BarStyle::STANDARD;
    auto navDestination = CreateNavDestinationWithTitle(optionsOfNavDest, "navDestination", "navDestinationSub");
    ASSERT_NE(navDestination, nullptr);
    auto navDestinationPattern = navDestination->GetPattern<NavDestinationPattern>();
    ASSERT_NE(navDestinationPattern, nullptr);
    /**
     * @tc.steps: step2. run navDestinationPattern's onModifyDone and test the layout property 'safeAreaPadding'.
     */
    navDestinationPattern->OnModifyDone();
    auto layoutWrapper = navDestination->CreateLayoutWrapper();
    ASSERT_NE(layoutWrapper, nullptr);
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    context->taskExecutor_ = AceType::MakeRefPtr<MockTaskExecutor>();
    auto taskExecutor = context->GetTaskExecutor();
    ASSERT_NE(taskExecutor, nullptr);
    BarStyleTestNg::RunMeasureAndLayout(layoutWrapper);
    /**
     * @tc.steps: step3. check property 'safeAreaPadding'.
     */
    auto navDestinationContent = AceType::DynamicCast<FrameNode>(navDestination->GetContentNode());
    ASSERT_NE(navDestinationContent, nullptr);
    auto geometryNode = navDestinationContent->GetGeometryNode();
    ASSERT_NE(geometryNode, nullptr);
    auto marginFrameOffset = geometryNode->GetMarginFrameOffset();
    /**
     * @tc.steps: step4. property 'marginFrameOffset' should be ...
     */
    ASSERT_NE(marginFrameOffset.GetY(), 0.0f);
}

/**
 * @tc.name: titleBarStyleTest014
 * @tc.desc: Branch: if (navDestination.titleBarStyle == BarStyle::STACK)
 *           Condition: navDestination.titleBarStyle == BarStyle::STACK
 * @tc.type: FUNC
 */
HWTEST_F(BarStyleTestNg, titleBarStyleTest014, TestSize.Level1)
{
    /**
     * @tc.steps: step1. init model, create navdestination with barStyle STACK and only mainTitle.
     */
    NavigationTitlebarOptions optionsOfNavDest;
    optionsOfNavDest.brOptions.barStyle = BarStyle::STACK;
    auto navDestination = CreateNavDestinationWithTitle(optionsOfNavDest, "navDestination", "navDestinationSub");
    ASSERT_NE(navDestination, nullptr);
    auto navDestinationPattern = navDestination->GetPattern<NavDestinationPattern>();
    ASSERT_NE(navDestinationPattern, nullptr);
    /**
     * @tc.steps: step2. run navDestinationPattern's onModifyDone and test the layout property 'safeAreaPadding'.
     */
    navDestinationPattern->OnModifyDone();
    auto layoutWrapper = navDestination->CreateLayoutWrapper();
    ASSERT_NE(layoutWrapper, nullptr);
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    context->taskExecutor_ = AceType::MakeRefPtr<MockTaskExecutor>();
    auto taskExecutor = context->GetTaskExecutor();
    ASSERT_NE(taskExecutor, nullptr);
    BarStyleTestNg::RunMeasureAndLayout(layoutWrapper);
    /**
     * @tc.steps: step3. check property 'safeAreaPadding'.
     */
    auto navDestinationContent = AceType::DynamicCast<FrameNode>(navDestination->GetContentNode());
    ASSERT_NE(navDestinationContent, nullptr);
    auto geometryNode = navDestinationContent->GetGeometryNode();
    ASSERT_NE(geometryNode, nullptr);
    auto marginFrameOffset = geometryNode->GetMarginFrameOffset();
    /**
     * @tc.steps: step4. property 'marginFrameOffset' should be ...
     */
    ASSERT_EQ(marginFrameOffset.GetY(), 0.0f);
}

/**
 * @tc.name: titleBarStyleTest015
 * @tc.desc: Branch: if (navBar.titleBarStyle == BarStyle::STANDARD && navBar.titleMode == MINI && navBar.hasSubTitle)
 *           Condition: navBar.titleBarStyle == BarStyle::STANDARD && navBar.titleMode == MINI && navBar.hasSubTitle
 * @tc.type: FUNC
 */
HWTEST_F(BarStyleTestNg, titleBarStyleTest015, TestSize.Level1)
{
    /**
     * @tc.steps: step1. init model, create navigation with barStyle STANDARD, main title and subtitle.
     */
    auto candidateBarStyle = BarStyle::STANDARD;
    auto mockNavPathStack = AceType::MakeRefPtr<MockNavigationStack>();
    NavigationTitlebarOptions options;
    options.brOptions.barStyle = candidateBarStyle;
    auto navigationNode = CreateNavigationWithTitle(
        mockNavPathStack, options, NavigationTitleMode::MINI, "navigationMainTitle", "navigationSubTitle");
    ASSERT_NE(navigationNode, nullptr);
    /**
     * @tc.steps: step2. run navigation's onModifyDone and test the layout property 'safeAreaPadding'.
     */
    auto navigationPattern = navigationNode->GetPattern<NavigationPattern>();
    ASSERT_NE(navigationPattern, nullptr);
    navigationPattern->OnModifyDone();
    auto layoutWrapper = navigationNode->CreateLayoutWrapper();
    ASSERT_NE(layoutWrapper, nullptr);
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    context->taskExecutor_ = AceType::MakeRefPtr<MockTaskExecutor>();
    auto taskExecutor = context->GetTaskExecutor();
    ASSERT_NE(taskExecutor, nullptr);
    BarStyleTestNg::RunMeasureAndLayout(layoutWrapper);
    /**
     * @tc.steps: step3. check property 'safeAreaPadding'.
     */
    auto navBar = AceType::DynamicCast<NavBarNode>(navigationNode->GetNavBarNode());
    ASSERT_NE(navBar, nullptr);
    auto navBarContent = AceType::DynamicCast<FrameNode>(navBar->GetContentNode());
    ASSERT_NE(navBarContent, nullptr);
    auto geometryNode = navBarContent->GetGeometryNode();
    ASSERT_NE(geometryNode, nullptr);
    auto marginFrameOffset = geometryNode->GetMarginFrameOffset();
    /**
     * @tc.steps: step4. property 'marginFrameOffset' should be ...
     */
    ASSERT_EQ(marginFrameOffset.GetY(), DOUBLE_LINE_TITLEBAR_HEIGHT.ConvertToPx());
}

/**
 * @tc.name: titleBarStyleTest016
 * @tc.desc: Branch: if (navBar.titleBarStyle == BarStyle::STANDARD && navBar.titleMode == MINI && !navBar.hasSubTitle)
 *           Condition: navBar.titleBarStyle == BarStyle::STANDARD && navBar.titleMode == MINI && !navBar.hasSubTitle
 * @tc.type: FUNC
 */
HWTEST_F(BarStyleTestNg, titleBarStyleTest016, TestSize.Level1)
{
    /**
     * @tc.steps: step1. init model, create navigation with barStyle STANDARD, main title and subtitle.
     */
    auto candidateBarStyle = BarStyle::STANDARD;
    auto mockNavPathStack = AceType::MakeRefPtr<MockNavigationStack>();
    NavigationTitlebarOptions options;
    options.brOptions.barStyle = candidateBarStyle;
    auto navigationNode = CreateNavigationWithTitle(
        mockNavPathStack, options, NavigationTitleMode::MINI, "navigationMainTitle", std::nullopt);
    ASSERT_NE(navigationNode, nullptr);
    /**
     * @tc.steps: step2. run navigation's onModifyDone and test the layout property 'safeAreaPadding'.
     */
    auto navigationPattern = navigationNode->GetPattern<NavigationPattern>();
    ASSERT_NE(navigationPattern, nullptr);
    navigationPattern->OnModifyDone();
    auto layoutWrapper = navigationNode->CreateLayoutWrapper();
    ASSERT_NE(layoutWrapper, nullptr);
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    context->taskExecutor_ = AceType::MakeRefPtr<MockTaskExecutor>();
    auto taskExecutor = context->GetTaskExecutor();
    ASSERT_NE(taskExecutor, nullptr);
    BarStyleTestNg::RunMeasureAndLayout(layoutWrapper);
    /**
     * @tc.steps: step3. check property 'safeAreaPadding'.
     */
    auto navBar = AceType::DynamicCast<NavBarNode>(navigationNode->GetNavBarNode());
    ASSERT_NE(navBar, nullptr);
    auto navBarContent = AceType::DynamicCast<FrameNode>(navBar->GetContentNode());
    ASSERT_NE(navBarContent, nullptr);
    auto geometryNode = navBarContent->GetGeometryNode();
    ASSERT_NE(geometryNode, nullptr);
    auto marginFrameOffset = geometryNode->GetMarginFrameOffset();
    /**
     * @tc.steps: step4. property 'marginFrameOffset' should be ...
     */
    ASSERT_EQ(marginFrameOffset.GetY(), SINGLE_LINE_TITLEBAR_HEIGHT.ConvertToPx());
}

/**
 * @tc.name: titleBarStyleTest017
 * @tc.desc: Branch: if (navDestination.titleBarStyle == BarStyle::STANDARD)
 *           Condition: navDestination.titleBarStyle == BarStyle::STANDARD
 * @tc.type: FUNC
 */
HWTEST_F(BarStyleTestNg, titleBarStyleTest017, TestSize.Level1)
{
    /**
     * @tc.steps: step1. init model, create navdestination with barStyle STANDARD and only mainTitle.
     */
    NavigationTitlebarOptions optionsOfNavDest;
    optionsOfNavDest.brOptions.barStyle = BarStyle::STANDARD;
    auto navDestination = CreateNavDestinationWithTitle(optionsOfNavDest, "navDestination", "navDestinationSub");
    ASSERT_NE(navDestination, nullptr);
    auto navDestinationPattern = navDestination->GetPattern<NavDestinationPattern>();
    ASSERT_NE(navDestinationPattern, nullptr);
    /**
     * @tc.steps: step2. run navDestinationPattern's onModifyDone and test the layout property 'safeAreaPadding'.
     */
    navDestinationPattern->OnModifyDone();
    auto layoutWrapper = navDestination->CreateLayoutWrapper();
    ASSERT_NE(layoutWrapper, nullptr);
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    context->taskExecutor_ = AceType::MakeRefPtr<MockTaskExecutor>();
    auto taskExecutor = context->GetTaskExecutor();
    ASSERT_NE(taskExecutor, nullptr);
    BarStyleTestNg::RunMeasureAndLayout(layoutWrapper);
    /**
     * @tc.steps: step3. check property 'safeAreaPadding'.
     */
    auto navDestinationContent = AceType::DynamicCast<FrameNode>(navDestination->GetContentNode());
    ASSERT_NE(navDestinationContent, nullptr);
    auto geometryNode = navDestinationContent->GetGeometryNode();
    ASSERT_NE(geometryNode, nullptr);
    auto marginFrameOffset = geometryNode->GetMarginFrameOffset();
    /**
     * @tc.steps: step4. property 'marginFrameOffset' should be ...
     */
    ASSERT_EQ(marginFrameOffset.GetY(), DOUBLE_LINE_TITLEBAR_HEIGHT.ConvertToPx());
}

/**
 * @tc.name: toolBarStyleTest001
 * @tc.desc: Branch: if (navBar.toolBarStyle == BarStyle::STANDARD)
 *           Condition: navBar.toolBarStyle == BarStyle::STANDARD
 * @tc.type: FUNC
 */
HWTEST_F(BarStyleTestNg, toolBarStyleTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. init model, create navigation with barStyle STANDARD, main title and subtitle.
     */
    NavigationToolbarOptions options;
    options.brOptions.barStyle = BarStyle::STANDARD;
    std::vector<BarItem> toolbarItems = { BarItem() };
    auto navigationNode = CreateNavigationWithToolBar(
        AceType::MakeRefPtr<MockNavigationStack>(), std::move(options), std::move(toolbarItems));
    ASSERT_NE(navigationNode, nullptr);
    /**
     * @tc.steps: step2. run navigation's onModifyDone and test the layout property 'safeAreaPadding'.
     */
    auto navigationPattern = navigationNode->GetPattern<NavigationPattern>();
    ASSERT_NE(navigationPattern, nullptr);
    navigationPattern->OnModifyDone();
    auto layoutWrapper = navigationNode->CreateLayoutWrapper();
    ASSERT_NE(layoutWrapper, nullptr);
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    context->taskExecutor_ = AceType::MakeRefPtr<MockTaskExecutor>();
    auto taskExecutor = context->GetTaskExecutor();
    ASSERT_NE(taskExecutor, nullptr);
    BarStyleTestNg::RunMeasureAndLayout(layoutWrapper);
    /**
     * @tc.steps: step3. check property 'safeAreaPadding'.
     */
    auto navBar = AceType::DynamicCast<NavBarNode>(navigationNode->GetNavBarNode());
    ASSERT_NE(navBar, nullptr);
    auto navBarContent = AceType::DynamicCast<FrameNode>(navBar->GetContentNode());
    ASSERT_NE(navBarContent, nullptr);
    auto contentLayoutProperty = navBarContent->GetLayoutProperty();
    ASSERT_NE(contentLayoutProperty, nullptr);
    const auto& safeAreaPadding = contentLayoutProperty->GetSafeAreaPaddingProperty();
    /**
     * @tc.steps: step4. property 'safeAreaPadding' should be nullptr cause no SAFE_AREA_PADDING barStyle set.
     */
    ASSERT_EQ(safeAreaPadding, nullptr);
}

/**
 * @tc.name: toolBarStyleTest002
 * @tc.desc: Branch: if (navBar.toolBarStyle == BarStyle::STACK)
 *           Condition: navBar.toolBarStyle == BarStyle::STACK
 * @tc.type: FUNC
 */
HWTEST_F(BarStyleTestNg, toolBarStyleTest002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. init model, create navigation with barStyle STACK, main title and subtitle.
     */
    NavigationToolbarOptions options;
    options.brOptions.barStyle = BarStyle::STACK;
    std::vector<BarItem> toolbarItems = { BarItem() };
    auto navigationNode = CreateNavigationWithToolBar(
        AceType::MakeRefPtr<MockNavigationStack>(), std::move(options), std::move(toolbarItems));
    ASSERT_NE(navigationNode, nullptr);
    /**
     * @tc.steps: step2. run navigation's onModifyDone and test the layout property 'safeAreaPadding'.
     */
    auto navigationPattern = navigationNode->GetPattern<NavigationPattern>();
    ASSERT_NE(navigationPattern, nullptr);
    navigationPattern->OnModifyDone();
    auto layoutWrapper = navigationNode->CreateLayoutWrapper();
    ASSERT_NE(layoutWrapper, nullptr);
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    context->taskExecutor_ = AceType::MakeRefPtr<MockTaskExecutor>();
    auto taskExecutor = context->GetTaskExecutor();
    ASSERT_NE(taskExecutor, nullptr);
    BarStyleTestNg::RunMeasureAndLayout(layoutWrapper);
    /**
     * @tc.steps: step3. check property 'safeAreaPadding'.
     */
    auto navBar = AceType::DynamicCast<NavBarNode>(navigationNode->GetNavBarNode());
    ASSERT_NE(navBar, nullptr);
    auto navBarContent = AceType::DynamicCast<FrameNode>(navBar->GetContentNode());
    ASSERT_NE(navBarContent, nullptr);
    auto contentLayoutProperty = navBarContent->GetLayoutProperty();
    ASSERT_NE(contentLayoutProperty, nullptr);
    const auto& safeAreaPadding = contentLayoutProperty->GetSafeAreaPaddingProperty();
    /**
     * @tc.steps: step4. property 'safeAreaPadding' should be nullptr cause no SAFE_AREA_PADDING barStyle set.
     */
    ASSERT_EQ(safeAreaPadding, nullptr);
}

/**
 * @tc.name: toolBarStyleTest003
 * @tc.desc: Branch: if (navBar.toolBarStyle == BarStyle::SAFE_AREA_PADDING)
 *           Condition: navBar.toolBarStyle == BarStyle::SAFE_AREA_PADDING
 * @tc.type: FUNC
 */
HWTEST_F(BarStyleTestNg, toolBarStyleTest003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. init model, create navigation with barStyle SAFE_AREA_PADDING, main title and subtitle.
     */
    NavigationToolbarOptions options;
    options.brOptions.barStyle = BarStyle::SAFE_AREA_PADDING;
    std::vector<BarItem> toolbarItems = { BarItem() };
    auto navigationNode = CreateNavigationWithToolBar(
        AceType::MakeRefPtr<MockNavigationStack>(), std::move(options), std::move(toolbarItems));
    ASSERT_NE(navigationNode, nullptr);
    /**
     * @tc.steps: step2. run navigation's onModifyDone and test the layout property 'safeAreaPadding'.
     */
    auto navigationPattern = navigationNode->GetPattern<NavigationPattern>();
    ASSERT_NE(navigationPattern, nullptr);
    navigationPattern->OnModifyDone();
    auto layoutWrapper = navigationNode->CreateLayoutWrapper();
    ASSERT_NE(layoutWrapper, nullptr);
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    context->taskExecutor_ = AceType::MakeRefPtr<MockTaskExecutor>();
    auto taskExecutor = context->GetTaskExecutor();
    ASSERT_NE(taskExecutor, nullptr);
    BarStyleTestNg::RunMeasureAndLayout(layoutWrapper);
    /**
     * @tc.steps: step3. check property 'safeAreaPadding'.
     */
    auto navBar = AceType::DynamicCast<NavBarNode>(navigationNode->GetNavBarNode());
    ASSERT_NE(navBar, nullptr);
    auto navBarContent = AceType::DynamicCast<FrameNode>(navBar->GetContentNode());
    ASSERT_NE(navBarContent, nullptr);
    auto contentLayoutProperty = navBarContent->GetLayoutProperty();
    ASSERT_NE(contentLayoutProperty, nullptr);
    const auto& safeAreaPadding = contentLayoutProperty->GetSafeAreaPaddingProperty();
    /**
     * @tc.steps: step4. property 'safeAreaPadding' should be nullptr cause no SAFE_AREA_PADDING barStyle set.
     */
    ASSERT_NE(safeAreaPadding, nullptr);
}

/**
 * @tc.name: SetToolbarOptions001
 * @tc.desc: Test SetToolbarOptions function.
 * @tc.type: FUNC
 */
HWTEST_F(BarStyleTestNg, SetToolbarOptions001, TestSize.Level1)
{
    NavigationToolbarOptions options;
    options.bgOptions.color = std::make_optional(Color(0xff00ff00));
    BlurStyleOption blurStyleOption;
    blurStyleOption.blurStyle = BlurStyle::REGULAR;
    options.bgOptions.blurStyleOption = blurStyleOption;
    options.brOptions.barStyle = std::make_optional(BarStyle::STANDARD);
    EffectOption effectOption;
    effectOption.adaptiveColor = AdaptiveColor::DEFAULT;
    options.bgOptions.effectOption = effectOption;
    std::vector<BarItem> toolbarItems = { BarItem() };
    g_isConfigChangePerform = true;
    auto navigationNode = CreateNavigationWithToolBar(
        AceType::MakeRefPtr<MockNavigationStack>(), std::move(options), std::move(toolbarItems));
    ASSERT_NE(navigationNode, nullptr);
    auto navBarNode = AceType::DynamicCast<NavBarNode>(navigationNode->GetNavBarNode());
    ASSERT_NE(navBarNode, nullptr);
    auto navBarPattern = navBarNode->GetPattern<NavBarPattern>();
    ASSERT_NE(navBarPattern, nullptr);
    navBarPattern->OnColorModeChange(1);
    EXPECT_NE(navBarNode->GetToolBarNode(), nullptr);
    auto toolbarNode = AceType::DynamicCast<NavToolbarNode>(navBarNode->GetToolBarNode());
    ASSERT_NE(toolbarNode, nullptr);
    auto toolBarPattern = toolbarNode->GetPattern<NavToolbarPattern>();
    ASSERT_NE(toolBarPattern, nullptr);
    EXPECT_TRUE(toolBarPattern->options_.bgOptions.color.has_value());
    EXPECT_EQ(toolBarPattern->options_.bgOptions.color.value(), Color(0xff00ff00));

    EXPECT_TRUE(toolBarPattern->options_.bgOptions.blurStyleOption.has_value());
    EXPECT_EQ(toolBarPattern->options_.bgOptions.blurStyleOption->blurStyle, BlurStyle::REGULAR);

    EXPECT_TRUE(toolBarPattern->options_.brOptions.barStyle.has_value());
    EXPECT_EQ(toolBarPattern->options_.brOptions.barStyle.value(), BarStyle::STANDARD);
    EXPECT_TRUE(options.bgOptions.effectOption.has_value());
    EXPECT_EQ(options.bgOptions.effectOption->adaptiveColor, AdaptiveColor::DEFAULT);
    g_isConfigChangePerform = false;
}

/**
 * @tc.name: toolBarStyleTest
 * @tc.desc: Branch: if (navDestination.titleBarStyle == BarStyle::STANDARD)
 *           Condition: navDestination.titleBarStyle == BarStyle::STANDARD
 * @tc.type: FUNC
 */
HWTEST_F(BarStyleTestNg, toolBarStyleTest010, TestSize.Level1)
{
    /**
     * @tc.steps: step1. init model, create navdestination with barStyle STANDARD and only mainTitle.
     */
    NavigationToolbarOptions options;
    options.brOptions.barStyle = BarStyle::STANDARD;
    std::vector<BarItem> toolbarItems = { BarItem() };
    auto navDestination = CreateNavDestinationWithToolBar(std::move(options), std::move(toolbarItems));
    ASSERT_NE(navDestination, nullptr);
    auto navDestinationPattern = navDestination->GetPattern<NavDestinationPattern>();
    ASSERT_NE(navDestinationPattern, nullptr);
    /**
     * @tc.steps: step2. run navDestinationPattern's onModifyDone and test the layout property 'safeAreaPadding'.
     */
    navDestinationPattern->OnModifyDone();
    auto layoutWrapper = navDestination->CreateLayoutWrapper();
    ASSERT_NE(layoutWrapper, nullptr);
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    context->taskExecutor_ = AceType::MakeRefPtr<MockTaskExecutor>();
    auto taskExecutor = context->GetTaskExecutor();
    ASSERT_NE(taskExecutor, nullptr);
    BarStyleTestNg::RunMeasureAndLayout(layoutWrapper);
    /**
     * @tc.steps: step3. check property 'safeAreaPadding'.
     */
    auto navDestinationContent = AceType::DynamicCast<FrameNode>(navDestination->GetContentNode());
    ASSERT_NE(navDestinationContent, nullptr);
    auto contentLayoutProperty = navDestinationContent->GetLayoutProperty();
    ASSERT_NE(contentLayoutProperty, nullptr);
    const auto& safeAreaPadding = contentLayoutProperty->GetSafeAreaPaddingProperty();
    /**
     * @tc.steps: step4. property 'safeAreaPadding' should be nullptr cause no SAFE_AREA_PADDING barStyle set.
     */
    ASSERT_EQ(safeAreaPadding, nullptr);
}

/**
 * @tc.name: toolBarStyleTest
 * @tc.desc: Branch: if (navDestination.titleBarStyle == BarStyle::STACK)
 *           Condition: navDestination.titleBarStyle == BarStyle::STACK
 * @tc.type: FUNC
 */
HWTEST_F(BarStyleTestNg, toolBarStyleTest011, TestSize.Level1)
{
    /**
     * @tc.steps: step1. init model, create navdestination with barStyle STACK and only mainTitle.
     */
    NavigationToolbarOptions options;
    options.brOptions.barStyle = BarStyle::STACK;
    std::vector<BarItem> toolbarItems = { BarItem() };
    auto navDestination = CreateNavDestinationWithToolBar(std::move(options), std::move(toolbarItems));
    ASSERT_NE(navDestination, nullptr);
    auto navDestinationPattern = navDestination->GetPattern<NavDestinationPattern>();
    ASSERT_NE(navDestinationPattern, nullptr);
    /**
     * @tc.steps: step2. run navDestinationPattern's onModifyDone and test the layout property 'safeAreaPadding'.
     */
    navDestinationPattern->OnModifyDone();
    auto layoutWrapper = navDestination->CreateLayoutWrapper();
    ASSERT_NE(layoutWrapper, nullptr);
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    context->taskExecutor_ = AceType::MakeRefPtr<MockTaskExecutor>();
    auto taskExecutor = context->GetTaskExecutor();
    ASSERT_NE(taskExecutor, nullptr);
    BarStyleTestNg::RunMeasureAndLayout(layoutWrapper);
    /**
     * @tc.steps: step3. check property 'safeAreaPadding'.
     */
    auto navDestinationContent = AceType::DynamicCast<FrameNode>(navDestination->GetContentNode());
    ASSERT_NE(navDestinationContent, nullptr);
    auto contentLayoutProperty = navDestinationContent->GetLayoutProperty();
    ASSERT_NE(contentLayoutProperty, nullptr);
    const auto& safeAreaPadding = contentLayoutProperty->GetSafeAreaPaddingProperty();
    /**
     * @tc.steps: step4. property 'safeAreaPadding' should be nullptr cause no SAFE_AREA_PADDING barStyle set.
     */
    ASSERT_EQ(safeAreaPadding, nullptr);
}

/**
 * @tc.name: toolBarStyleTest
 * @tc.desc: Branch: if (navDestination.titleBarStyle == BarStyle::SAFE_AREA_PADDING)
 *           Condition: navDestination.titleBarStyle == BarStyle::SAFE_AREA_PADDING
 * @tc.type: FUNC
 */
HWTEST_F(BarStyleTestNg, toolBarStyleTest012, TestSize.Level1)
{
    /**
     * @tc.steps: step1. init model, create navdestination with barStyle SAFE_AREA_PADDING and only mainTitle.
     */
    NavigationToolbarOptions options;
    options.brOptions.barStyle = BarStyle::SAFE_AREA_PADDING;
    std::vector<BarItem> toolbarItems = { BarItem() };
    auto navDestination = CreateNavDestinationWithToolBar(std::move(options), std::move(toolbarItems));
    ASSERT_NE(navDestination, nullptr);
    auto navDestinationPattern = navDestination->GetPattern<NavDestinationPattern>();
    ASSERT_NE(navDestinationPattern, nullptr);
    /**
     * @tc.steps: step2. run navDestinationPattern's onModifyDone and test the layout property 'safeAreaPadding'.
     */
    navDestinationPattern->OnModifyDone();
    auto layoutWrapper = navDestination->CreateLayoutWrapper();
    ASSERT_NE(layoutWrapper, nullptr);
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    context->taskExecutor_ = AceType::MakeRefPtr<MockTaskExecutor>();
    auto taskExecutor = context->GetTaskExecutor();
    ASSERT_NE(taskExecutor, nullptr);
    BarStyleTestNg::RunMeasureAndLayout(layoutWrapper);
    /**
     * @tc.steps: step3. check property 'safeAreaPadding'.
     */
    auto navDestinationContent = AceType::DynamicCast<FrameNode>(navDestination->GetContentNode());
    ASSERT_NE(navDestinationContent, nullptr);
    auto contentLayoutProperty = navDestinationContent->GetLayoutProperty();
    ASSERT_NE(contentLayoutProperty, nullptr);
    const auto& safeAreaPadding = contentLayoutProperty->GetSafeAreaPaddingProperty();
    /**
     * @tc.steps: step4. property 'safeAreaPadding' should be nullptr cause no SAFE_AREA_PADDING barStyle set.
     */
    ASSERT_NE(safeAreaPadding, nullptr);
}

/**
 * @tc.name: ImmersiveStrategy_NavSafeAreaPadding_DefaultSafeAreaPadding001
 * @tc.desc: strategy enabled + no explicit barStyle -> Navigation and NavDestination
 *           default to SAFE_AREA_PADDING on title bar.
 * @tc.type: FUNC
 */
HWTEST_F(BarStyleTestNg, ImmersiveStrategy_NavSafeAreaPadding_DefaultSafeAreaPadding001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. enable NAVIGATION_EXPAND_SAFE_AREA strategy.
     */
    ApplyNavStrategies({ ImmersiveStrategy::NAVIGATION_EXPAND_SAFE_AREA });
    /**
     * @tc.steps: step2. create Navigation with title options but no explicit barStyle.
     */
    auto mockNavPathStack = AceType::MakeRefPtr<MockNavigationStack>();
    NavigationTitlebarOptions options;
    auto navigationNode = CreateNavigationWithTitle(
        mockNavPathStack, options, NavigationTitleMode::MINI, "navigation", std::nullopt);
    ASSERT_NE(navigationNode, nullptr);
    /**
     * @tc.steps: step3. check the pattern-level barStyle is injected as SAFE_AREA_PADDING.
     */
    auto navBarNode = AceType::DynamicCast<NavDestinationNodeBase>(navigationNode->GetNavBarNode());
    ASSERT_NE(navBarNode, nullptr);
    auto navBarPattern = navBarNode->GetPattern<NavDestinationPatternBase>();
    ASSERT_NE(navBarPattern, nullptr);
    EXPECT_EQ(navBarPattern->GetTitleBarStyle(), BarStyle::SAFE_AREA_PADDING);
    auto titleBarOfNavBar = AceType::DynamicCast<TitleBarNode>(navBarNode->GetTitleBarNode());
    ASSERT_NE(titleBarOfNavBar, nullptr);
    auto patternOfNavBarTitleBar = titleBarOfNavBar->GetPattern<TitleBarPattern>();
    ASSERT_NE(patternOfNavBarTitleBar, nullptr);
    EXPECT_EQ(patternOfNavBarTitleBar->GetTitleBarOptions().brOptions.barStyle, BarStyle::SAFE_AREA_PADDING);
    /**
     * @tc.steps: step4. check NavDestination title bar also defaults to SAFE_AREA_PADDING.
     */
    NavigationTitlebarOptions destOptions;
    auto navDestination = CreateNavDestinationWithTitle(destOptions, "navDestination", std::nullopt);
    ASSERT_NE(navDestination, nullptr);
    EXPECT_EQ(GetNavDestinationTitleBarStyle(navDestination), BarStyle::SAFE_AREA_PADDING);
    /**
     * @tc.steps: step5. run layout and check content safe area padding is applied so that
     *            content can extend into the bar area (same as explicit SAFE_AREA_PADDING).
     */
    auto navigationPattern = navigationNode->GetPattern<NavigationPattern>();
    ASSERT_NE(navigationPattern, nullptr);
    navigationPattern->OnModifyDone();
    auto layoutWrapper = navigationNode->CreateLayoutWrapper();
    ASSERT_NE(layoutWrapper, nullptr);
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    context->taskExecutor_ = AceType::MakeRefPtr<MockTaskExecutor>();
    BarStyleTestNg::RunMeasureAndLayout(layoutWrapper);
    auto navBar = AceType::DynamicCast<NavBarNode>(navigationNode->GetNavBarNode());
    ASSERT_NE(navBar, nullptr);
    auto navBarContent = AceType::DynamicCast<FrameNode>(navBar->GetContentNode());
    ASSERT_NE(navBarContent, nullptr);
    auto contentLayoutProperty = navBarContent->GetLayoutProperty();
    ASSERT_NE(contentLayoutProperty, nullptr);
    const auto& safeAreaPadding = contentLayoutProperty->GetSafeAreaPaddingProperty();
    ASSERT_NE(safeAreaPadding, nullptr);
    EXPECT_EQ(safeAreaPadding->top.value_or(CalcLength(0.0f)),
        CalcLength(SINGLE_LINE_TITLEBAR_HEIGHT.ConvertToPx()));
    ApplyNavStrategies({});
}

/**
 * @tc.name: ImmersiveStrategy_Toolbar_DefaultSafeAreaPadding002
 * @tc.desc: strategy enabled + no explicit barStyle -> toolbar defaults to
 *           SAFE_AREA_PADDING symmetrically.
 * @tc.type: FUNC
 */
HWTEST_F(BarStyleTestNg, ImmersiveStrategy_Toolbar_DefaultSafeAreaPadding002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. enable NAVIGATION_EXPAND_SAFE_AREA strategy.
     */
    ApplyNavStrategies({ ImmersiveStrategy::NAVIGATION_EXPAND_SAFE_AREA });
    /**
     * @tc.steps: step2. create Navigation with toolbar but no explicit barStyle.
     */
    NavigationToolbarOptions options;
    std::vector<BarItem> toolbarItems = { BarItem() };
    auto navigationNode = CreateNavigationWithToolBar(
        AceType::MakeRefPtr<MockNavigationStack>(), std::move(options), std::move(toolbarItems));
    ASSERT_NE(navigationNode, nullptr);
    auto navBarNode = AceType::DynamicCast<NavDestinationNodeBase>(navigationNode->GetNavBarNode());
    ASSERT_NE(navBarNode, nullptr);
    auto navBarPattern = navBarNode->GetPattern<NavDestinationPatternBase>();
    ASSERT_NE(navBarPattern, nullptr);
    EXPECT_EQ(navBarPattern->GetToolBarStyle(), BarStyle::SAFE_AREA_PADDING);
    /**
     * @tc.steps: step3. check NavDestination toolbar also defaults to SAFE_AREA_PADDING.
     */
    NavigationToolbarOptions destOptions;
    std::vector<BarItem> destToolbarItems = { BarItem() };
    auto navDestination = CreateNavDestinationWithToolBar(
        std::move(destOptions), std::move(destToolbarItems));
    ASSERT_NE(navDestination, nullptr);
    auto destPattern = navDestination->GetPattern<NavDestinationPatternBase>();
    ASSERT_NE(destPattern, nullptr);
    EXPECT_EQ(destPattern->GetToolBarStyle(), BarStyle::SAFE_AREA_PADDING);
    ApplyNavStrategies({});
}

/**
 * @tc.name: ImmersiveStrategy_NotEnabled_DefaultStandard003
 * @tc.desc: strategy not enabled -> barStyle stays unset (nullopt), behavior
 *           unchanged (STANDARD fallback).
 * @tc.type: FUNC
 */
HWTEST_F(BarStyleTestNg, ImmersiveStrategy_NotEnabled_DefaultStandard003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. make sure no strategy is applied.
     */
    ApplyNavStrategies({});
    /**
     * @tc.steps: step2. create Navigation with title options but no explicit barStyle.
     */
    auto mockNavPathStack = AceType::MakeRefPtr<MockNavigationStack>();
    NavigationTitlebarOptions options;
    auto navigationNode = CreateNavigationWithTitle(
        mockNavPathStack, options, NavigationTitleMode::FREE, "navigation", std::nullopt);
    ASSERT_NE(navigationNode, nullptr);
    auto navBarNode = AceType::DynamicCast<NavDestinationNodeBase>(navigationNode->GetNavBarNode());
    ASSERT_NE(navBarNode, nullptr);
    auto navBarPattern = navBarNode->GetPattern<NavDestinationPatternBase>();
    ASSERT_NE(navBarPattern, nullptr);
    EXPECT_FALSE(navBarPattern->GetTitleBarStyle().has_value());
    EXPECT_EQ(navBarPattern->GetTitleBarStyle().value_or(BarStyle::STANDARD), BarStyle::STANDARD);
    /**
     * @tc.steps: step3. layout keeps original SAFE_AREA behavior: no safe area padding injected.
     */
    auto navigationPattern = navigationNode->GetPattern<NavigationPattern>();
    ASSERT_NE(navigationPattern, nullptr);
    navigationPattern->OnModifyDone();
    auto layoutWrapper = navigationNode->CreateLayoutWrapper();
    ASSERT_NE(layoutWrapper, nullptr);
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    context->taskExecutor_ = AceType::MakeRefPtr<MockTaskExecutor>();
    BarStyleTestNg::RunMeasureAndLayout(layoutWrapper);
    auto navBar = AceType::DynamicCast<NavBarNode>(navigationNode->GetNavBarNode());
    ASSERT_NE(navBar, nullptr);
    auto navBarContent = AceType::DynamicCast<FrameNode>(navBar->GetContentNode());
    ASSERT_NE(navBarContent, nullptr);
    auto contentLayoutProperty = navBarContent->GetLayoutProperty();
    ASSERT_NE(contentLayoutProperty, nullptr);
    EXPECT_EQ(contentLayoutProperty->GetSafeAreaPaddingProperty(), nullptr);
}

/**
 * @tc.name: ImmersiveStrategy_ExplicitStack_OverridesStrategy004
 * @tc.desc: strategy enabled + explicit barStyle STACK -> developer setting wins,
 *           strategy does not inject.
 * @tc.type: FUNC
 */
HWTEST_F(BarStyleTestNg, ImmersiveStrategy_ExplicitStack_OverridesStrategy004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. enable NAVIGATION_EXPAND_SAFE_AREA strategy.
     */
    ApplyNavStrategies({ ImmersiveStrategy::NAVIGATION_EXPAND_SAFE_AREA });
    /**
     * @tc.steps: step2. create Navigation with explicit STACK barStyle.
     */
    auto mockNavPathStack = AceType::MakeRefPtr<MockNavigationStack>();
    NavigationTitlebarOptions options;
    options.brOptions.barStyle = BarStyle::STACK;
    auto navigationNode = CreateNavigationWithTitle(
        mockNavPathStack, options, NavigationTitleMode::FREE, "navigation", std::nullopt);
    ASSERT_NE(navigationNode, nullptr);
    auto navBarNode = AceType::DynamicCast<NavDestinationNodeBase>(navigationNode->GetNavBarNode());
    ASSERT_NE(navBarNode, nullptr);
    auto navBarPattern = navBarNode->GetPattern<NavDestinationPatternBase>();
    ASSERT_NE(navBarPattern, nullptr);
    EXPECT_EQ(navBarPattern->GetTitleBarStyle(), BarStyle::STACK);
    auto titleBarOfNavBar = AceType::DynamicCast<TitleBarNode>(navBarNode->GetTitleBarNode());
    ASSERT_NE(titleBarOfNavBar, nullptr);
    auto patternOfNavBarTitleBar = titleBarOfNavBar->GetPattern<TitleBarPattern>();
    ASSERT_NE(patternOfNavBarTitleBar, nullptr);
    EXPECT_EQ(patternOfNavBarTitleBar->GetTitleBarOptions().brOptions.barStyle, BarStyle::STACK);
    /**
     * @tc.steps: step3. explicit SAFE_AREA_PADDING also wins (kept as developer value).
     */
    NavigationTitlebarOptions paddingOptions;
    paddingOptions.brOptions.barStyle = BarStyle::SAFE_AREA_PADDING;
    auto paddingNode = CreateNavigationWithTitle(
        AceType::MakeRefPtr<MockNavigationStack>(), paddingOptions, NavigationTitleMode::FREE,
        "navigationPadding", std::nullopt);
    ASSERT_NE(paddingNode, nullptr);
    auto paddingNavBarNode = AceType::DynamicCast<NavDestinationNodeBase>(paddingNode->GetNavBarNode());
    ASSERT_NE(paddingNavBarNode, nullptr);
    auto paddingNavBarPattern = paddingNavBarNode->GetPattern<NavDestinationPatternBase>();
    ASSERT_NE(paddingNavBarPattern, nullptr);
    EXPECT_EQ(paddingNavBarPattern->GetTitleBarStyle(), BarStyle::SAFE_AREA_PADDING);
    ApplyNavStrategies({});
}

/**
 * @tc.name: ImmersiveStrategy_BarStyle_DynamicChange_UsesLatest005
 * @tc.desc: strategy injected SAFE_AREA_PADDING first, then developer sets STANDARD
 *           -> latest setting wins.
 * @tc.type: FUNC
 */
HWTEST_F(BarStyleTestNg, ImmersiveStrategy_BarStyle_DynamicChange_UsesLatest005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. enable strategy and create Navigation without barStyle (injected).
     */
    ApplyNavStrategies({ ImmersiveStrategy::NAVIGATION_EXPAND_SAFE_AREA });
    auto mockNavPathStack = AceType::MakeRefPtr<MockNavigationStack>();
    NavigationTitlebarOptions options;
    auto navigationNode = CreateNavigationWithTitle(
        mockNavPathStack, options, NavigationTitleMode::FREE, "navigation", std::nullopt);
    ASSERT_NE(navigationNode, nullptr);
    auto navBarNode = AceType::DynamicCast<NavDestinationNodeBase>(navigationNode->GetNavBarNode());
    ASSERT_NE(navBarNode, nullptr);
    auto navBarPattern = navBarNode->GetPattern<NavDestinationPatternBase>();
    ASSERT_NE(navBarPattern, nullptr);
    EXPECT_EQ(navBarPattern->GetTitleBarStyle(), BarStyle::SAFE_AREA_PADDING);
    /**
     * @tc.steps: step2. developer dynamically sets barStyle to STANDARD via update path.
     */
    NavigationTitlebarOptions updateOptions;
    updateOptions.brOptions.barStyle = BarStyle::STANDARD;
    NavigationModelNG::SetTitlebarOptions(AceType::RawPtr(navigationNode), std::move(updateOptions));
    /**
     * @tc.steps: step3. latest developer setting wins.
     */
    EXPECT_EQ(navBarPattern->GetTitleBarStyle(), BarStyle::STANDARD);
    ApplyNavStrategies({});
}

/**
 * @tc.name: ImmersiveStrategy_BarStyleUndefined_StrategyReinject006
 * @tc.desc: developer sets barStyle to undefined -> falls back to unset state,
 *           strategy re-injects default.
 * @tc.type: FUNC
 */
HWTEST_F(BarStyleTestNg, ImmersiveStrategy_BarStyleUndefined_StrategyReinject006, TestSize.Level1)
{
    /**
     * @tc.steps: step1. enable strategy and create Navigation with explicit STACK.
     */
    ApplyNavStrategies({ ImmersiveStrategy::NAVIGATION_EXPAND_SAFE_AREA });
    auto mockNavPathStack = AceType::MakeRefPtr<MockNavigationStack>();
    NavigationTitlebarOptions options;
    options.brOptions.barStyle = BarStyle::STACK;
    auto navigationNode = CreateNavigationWithTitle(
        mockNavPathStack, options, NavigationTitleMode::FREE, "navigation", std::nullopt);
    ASSERT_NE(navigationNode, nullptr);
    auto navBarNode = AceType::DynamicCast<NavDestinationNodeBase>(navigationNode->GetNavBarNode());
    ASSERT_NE(navBarNode, nullptr);
    auto navBarPattern = navBarNode->GetPattern<NavDestinationPatternBase>();
    ASSERT_NE(navBarPattern, nullptr);
    EXPECT_EQ(navBarPattern->GetTitleBarStyle(), BarStyle::STACK);
    /**
     * @tc.steps: step2. developer sets barStyle to undefined (nullopt, same semantics as unset).
     */
    NavigationTitlebarOptions undefinedOptions;
    NavigationModelNG::SetTitlebarOptions(AceType::RawPtr(navigationNode), std::move(undefinedOptions));
    /**
     * @tc.steps: step3. strategy re-injects SAFE_AREA_PADDING.
     */
    EXPECT_EQ(navBarPattern->GetTitleBarStyle(), BarStyle::SAFE_AREA_PADDING);
    ApplyNavStrategies({});
}

/**
 * @tc.name: ImmersiveStrategy_DynamicModify_AllInstancesUpdated007
 * @tc.desc: strategy dynamically enabled/disabled -> all Navigation instances
 *           without explicit barStyle follow the latest strategy.
 * @tc.type: FUNC
 */
HWTEST_F(BarStyleTestNg, ImmersiveStrategy_DynamicModify_AllInstancesUpdated007, TestSize.Level1)
{
    /**
     * @tc.steps: step1. no strategy: Navigation A stays unset.
     */
    ApplyNavStrategies({});
    NavigationTitlebarOptions optionsA;
    auto navigationNodeA = CreateNavigationWithTitle(
        AceType::MakeRefPtr<MockNavigationStack>(), optionsA, NavigationTitleMode::FREE,
        "navigationA", std::nullopt);
    ASSERT_NE(navigationNodeA, nullptr);
    auto navBarNodeA = AceType::DynamicCast<NavDestinationNodeBase>(navigationNodeA->GetNavBarNode());
    ASSERT_NE(navBarNodeA, nullptr);
    auto navBarPatternA = navBarNodeA->GetPattern<NavDestinationPatternBase>();
    ASSERT_NE(navBarPatternA, nullptr);
    EXPECT_FALSE(navBarPatternA->GetTitleBarStyle().has_value());
    /**
     * @tc.steps: step2. enable strategy: newly created Navigation B gets the default.
     */
    ApplyNavStrategies({ ImmersiveStrategy::NAVIGATION_EXPAND_SAFE_AREA });
    NavigationTitlebarOptions optionsB;
    auto navigationNodeB = CreateNavigationWithTitle(
        AceType::MakeRefPtr<MockNavigationStack>(), optionsB, NavigationTitleMode::FREE,
        "navigationB", std::nullopt);
    ASSERT_NE(navigationNodeB, nullptr);
    auto navBarNodeB = AceType::DynamicCast<NavDestinationNodeBase>(navigationNodeB->GetNavBarNode());
    ASSERT_NE(navBarNodeB, nullptr);
    auto navBarPatternB = navBarNodeB->GetPattern<NavDestinationPatternBase>();
    ASSERT_NE(navBarPatternB, nullptr);
    EXPECT_EQ(navBarPatternB->GetTitleBarStyle(), BarStyle::SAFE_AREA_PADDING);
    /**
     * @tc.steps: step3. re-apply title options on A (re-render path): A follows the latest strategy.
     */
    NavigationTitlebarOptions refreshOptionsA;
    NavigationModelNG::SetTitlebarOptions(AceType::RawPtr(navigationNodeA), std::move(refreshOptionsA));
    EXPECT_EQ(navBarPatternA->GetTitleBarStyle(), BarStyle::SAFE_AREA_PADDING);
    /**
     * @tc.steps: step4. disable strategy: newly created Navigation C stays unset and B reverts
     *            through the re-render path.
     */
    ApplyNavStrategies({});
    NavigationTitlebarOptions optionsC;
    auto navigationNodeC = CreateNavigationWithTitle(
        AceType::MakeRefPtr<MockNavigationStack>(), optionsC, NavigationTitleMode::FREE,
        "navigationC", std::nullopt);
    ASSERT_NE(navigationNodeC, nullptr);
    auto navBarNodeC = AceType::DynamicCast<NavDestinationNodeBase>(navigationNodeC->GetNavBarNode());
    ASSERT_NE(navBarNodeC, nullptr);
    auto navBarPatternC = navBarNodeC->GetPattern<NavDestinationPatternBase>();
    ASSERT_NE(navBarPatternC, nullptr);
    EXPECT_FALSE(navBarPatternC->GetTitleBarStyle().has_value());
    NavigationTitlebarOptions refreshOptionsB;
    NavigationModelNG::SetTitlebarOptions(AceType::RawPtr(navigationNodeB), std::move(refreshOptionsB));
    EXPECT_FALSE(navBarPatternB->GetTitleBarStyle().has_value());
}

/**
 * @tc.name: ImmersiveStrategy_WithLegacyStrategies_NoConflict008
 * @tc.desc: NAVIGATION_EXPAND_SAFE_AREA coexists with AVOID_CUTOUT/AVOID_FLOAT_NAV
 *           without conflict.
 * @tc.type: FUNC
 */
HWTEST_F(BarStyleTestNg, ImmersiveStrategy_WithLegacyStrategies_NoConflict008, TestSize.Level1)
{
    /**
     * @tc.steps: step1. apply all three strategies together.
     */
    ApplyNavStrategies({ ImmersiveStrategy::AVOID_CUTOUT, ImmersiveStrategy::AVOID_FLOAT_NAV,
        ImmersiveStrategy::NAVIGATION_EXPAND_SAFE_AREA });
    auto safeAreaManager = MockPipelineContext::GetCurrent()->GetSafeAreaManager();
    ASSERT_NE(safeAreaManager, nullptr);
    EXPECT_TRUE(safeAreaManager->IsImmersiveStrategySet(ImmersiveStrategy::AVOID_CUTOUT));
    EXPECT_TRUE(safeAreaManager->IsImmersiveStrategySet(ImmersiveStrategy::AVOID_FLOAT_NAV));
    EXPECT_TRUE(safeAreaManager->IsImmersiveStrategySet(ImmersiveStrategy::NAVIGATION_EXPAND_SAFE_AREA));
    /**
     * @tc.steps: step2. Navigation default barStyle follows NAVIGATION_EXPAND_SAFE_AREA.
     */
    NavigationTitlebarOptions options;
    auto navigationNode = CreateNavigationWithTitle(
        AceType::MakeRefPtr<MockNavigationStack>(), options, NavigationTitleMode::FREE,
        "navigation", std::nullopt);
    ASSERT_NE(navigationNode, nullptr);
    auto navBarNode = AceType::DynamicCast<NavDestinationNodeBase>(navigationNode->GetNavBarNode());
    ASSERT_NE(navBarNode, nullptr);
    auto navBarPattern = navBarNode->GetPattern<NavDestinationPatternBase>();
    ASSERT_NE(navBarPattern, nullptr);
    EXPECT_EQ(navBarPattern->GetTitleBarStyle(), BarStyle::SAFE_AREA_PADDING);
    ApplyNavStrategies({});
}
} // namespace OHOS::Ace::NG

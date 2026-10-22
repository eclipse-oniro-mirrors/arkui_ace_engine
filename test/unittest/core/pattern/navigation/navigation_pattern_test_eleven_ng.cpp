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
#include <iostream>
#include <iterator>
#include <optional>

#include "gtest/gtest.h"
#include "mock_navigation_route.h"
#include "mock_navigation_stack.h"
#define private public
#define protected public
#include "test/mock/frameworks/base/thread/mock_task_executor.h"
#include "test/mock/frameworks/core/common/mock_container.h"
#include "test/mock/frameworks/core/common/mock_theme_manager.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"

#include "base/mousestyle/mouse_style.h"
#include "core/common/agingadapation/aging_adapation_dialog_util.h"
#include "core/components/dialog/dialog_properties.h"
#include "core/components_ng/layout/layout_wrapper_node.h"
#include "core/components_ng/manager/content_change_manager/content_change_manager.h"
#include "core/components_ng/manager/force_split/force_split_manager.h"
#include "core/components_ng/pattern/custom_frame_node/custom_pattern.h"
#include "core/components_ng/pattern/dialog/dialog_pattern.h"
#include "core/components_ng/pattern/dialog/dialog_view.h"
#include "core/components_ng/pattern/divider/divider_pattern.h"
#include "core/components_ng/pattern/navigation/nav_bar_node.h"
#include "core/components_ng/pattern/navigation/nav_bar_pattern.h"
#include "core/components_ng/pattern/navigation/navigation_drag_bar_pattern.h"
#include "core/components_ng/pattern/navigation/navigation_model_ng.h"
#include "core/components_ng/pattern/navigation/navigation_pattern.h"
#include "core/components_ng/pattern/navigation/title_bar_pattern.h"
#include "core/components_ng/pattern/navigation/tool_bar_pattern.h"
#include "core/components_ng/pattern/scroll/scroll_pattern.h"
#include "core/components_ng/pattern/text/text_pattern.h"
#include "core/components_ng/manager/navigation/navigation_manager.h"
#include "core/components_ng/manager/toolbar/toolbar_manager.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS::Ace::NG {
namespace {
RefPtr<NavigationBarTheme> CreateAndBindNavigationBarTheme()
{
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    auto navigationTheme = AceType::MakeRefPtr<NavigationBarTheme>();
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(navigationTheme));
    EXPECT_CALL(*themeManager, GetTheme(_, _)).WillRepeatedly(Return(navigationTheme));
    return navigationTheme;
}

class PageLevelSplitNavigationStack : public NavigationStack {
    DECLARE_ACE_TYPE(PageLevelSplitNavigationStack, NavigationStack);
public:
    int32_t GetReplaceValue() const override
    {
        return replaceValue_;
    }
    int32_t replaceValue_ = 0;
};

class PageLevelSplitNavBarNode : public NavBarNode {
    DECLARE_ACE_TYPE(PageLevelSplitNavBarNode, NavBarNode);
public:
    using NavBarNode::NavBarNode;
    void SystemTransitionPushEnd(bool transitionIn) override
    {
        ++pushEndCount_;
        NavBarNode::SystemTransitionPushEnd(transitionIn);
    }
    void SystemTransitionPushFinish(bool transitionIn, int32_t animationId) override
    {
        ++pushFinishCount_;
        NavBarNode::SystemTransitionPushFinish(transitionIn, animationId);
    }
    int32_t pushEndCount_ = 0;
    int32_t pushFinishCount_ = 0;
};
} // namespace

class NavigationPatternTestElevenNg : public testing::Test {
public:
    static RefPtr<DialogTheme> dialogTheme_;
    static RefPtr<NavigationBarTheme> navigationBarTheme_;
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void MockPipelineContextGetTheme();

protected:
    void InitPageLevelSplitTest();
    void TearDown() override;
    RefPtr<NavDestinationGroupNode> CreateDestination(const std::string& name);
    void SetStack(const NavPathList& paths);
    void SyncPortraitColumns(const NavPathList& before, const NavPathList& after);

    RefPtr<NavigationGroupNode> navigation_;
    RefPtr<NavigationPattern> pattern_;
    RefPtr<PageLevelSplitNavigationStack> stack_;
    RefPtr<ForceSplitManager> manager_;
    bool pageLevelSplitTestInitialized_ = false;
};

void NavigationPatternTestElevenNg::MockPipelineContextGetTheme()
{
    CreateAndBindNavigationBarTheme();
}

namespace {
RefPtr<Theme> GetTheme(ThemeType type)
{
    if (type == DialogTheme::TypeId()) {
        return NavigationPatternTestElevenNg::dialogTheme_;
    } else if (type == NavigationBarTheme::TypeId()) {
        return NavigationPatternTestElevenNg::navigationBarTheme_;
    } else {
        return nullptr;
    }
}
} // namespace

RefPtr<DialogTheme> NavigationPatternTestElevenNg::dialogTheme_ = nullptr;
RefPtr<NavigationBarTheme> NavigationPatternTestElevenNg::navigationBarTheme_ = nullptr;

void NavigationPatternTestElevenNg::SetUpTestSuite()
{
    MockPipelineContext::SetUp();
    MockContainer::SetUp();

    dialogTheme_ = AceType::MakeRefPtr<DialogTheme>();
    navigationBarTheme_ = AceType::MakeRefPtr<NavigationBarTheme>();
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly([](ThemeType type) -> RefPtr<Theme> {
        return GetTheme(type);
    });
    EXPECT_CALL(*themeManager, GetTheme(_, _))
        .WillRepeatedly([](ThemeType type, int32_t themeScopeId) -> RefPtr<Theme> { return GetTheme(type); });
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
}

void NavigationPatternTestElevenNg::TearDownTestSuite()
{
    MockPipelineContext::TearDown();
    MockContainer::TearDown();
}

/**
 * @tc.name: RecognizeHomePageIfNeeded008
 * @tc.desc: Branch: if (forceSplitMgr && forceSplitMgr->IsPageLevelSplitEnabled()) { => true
 *           page-level split control skips legacy home recognition
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestElevenNg, RecognizeHomePageIfNeeded008, TestSize.Level1)
{
    NavigationPatternTestElevenNg::SetUpTestSuite();
    MockPipelineContextGetTheme();
    NavigationModelNG navigationModel;
    navigationModel.Create();
    navigationModel.SetNavigationStack();
    navigationModel.SetUsrNavigationMode(NavigationMode::SPLIT);
    auto navNode = AceType::DynamicCast<NavigationGroupNode>(ViewStackProcessor::GetInstance()->GetMainFrameNode());
    ASSERT_NE(navNode, nullptr);
    auto pattern = navNode->GetPattern<NavigationPattern>();
    ASSERT_NE(pattern, nullptr);
    auto& stack = pattern->navigationStack_;
    ASSERT_NE(stack, nullptr);
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto manager = context->GetForceSplitManager();
    ASSERT_NE(manager, nullptr);
    manager->isRouter_ = false;
    manager->isForceSplitSupported_ = true;
    manager->enablePageLevelSplit_ = true;
    manager->SetHomePageName("");

    auto dest = NavDestinationGroupNode::GetOrCreateGroupNode(
        V2::NAVDESTINATION_VIEW_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        []() { return AceType::MakeRefPtr<NavDestinationPattern>(); });
    ASSERT_NE(dest, nullptr);
    auto destPattern = dest->GetPattern<NavDestinationPattern>();
    ASSERT_NE(destPattern, nullptr);
    destPattern->SetName("two");
    stack->navPathList_.clear();
    stack->navPathList_.push_back({ "two", dest });
    pattern->forceSplitHomeDest_ = nullptr;

    pattern->RecognizeHomePageIfNeeded();
    EXPECT_EQ(pattern->forceSplitHomeDest_.Upgrade(), nullptr);
    EXPECT_TRUE(manager->GetHomePageName().empty());
}

/**
 * @tc.name: TryEstablishPageLevelSplitHome001
 * @tc.desc: Branch: if (!forceSplitMgr->IsPageLevelSplitEnabled()) { => true
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestElevenNg, TryEstablishPageLevelSplitHome001, TestSize.Level1)
{
    NavigationPatternTestElevenNg::SetUpTestSuite();
    MockPipelineContextGetTheme();
    NavigationModelNG navigationModel;
    navigationModel.Create();
    navigationModel.SetNavigationStack();
    navigationModel.SetUsrNavigationMode(NavigationMode::SPLIT);
    auto navNode = AceType::DynamicCast<NavigationGroupNode>(ViewStackProcessor::GetInstance()->GetMainFrameNode());
    ASSERT_NE(navNode, nullptr);
    auto pattern = navNode->GetPattern<NavigationPattern>();
    ASSERT_NE(pattern, nullptr);
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto manager = context->GetForceSplitManager();
    ASSERT_NE(manager, nullptr);
    manager->enablePageLevelSplit_ = false;
    manager->SetHomePageName("");
    pattern->forceSplitHomeDest_ = nullptr;

    pattern->TryEstablishPageLevelSplitHome();
    EXPECT_EQ(pattern->forceSplitHomeDest_.Upgrade(), nullptr);
    EXPECT_TRUE(manager->GetHomePageName().empty());
}

/**
 * @tc.name: TryEstablishPageLevelSplitHome002
 * @tc.desc: Branch: if (!topNavPath.has_value()) { => true
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestElevenNg, TryEstablishPageLevelSplitHome002, TestSize.Level1)
{
    NavigationPatternTestElevenNg::SetUpTestSuite();
    MockPipelineContextGetTheme();
    NavigationModelNG navigationModel;
    navigationModel.Create();
    navigationModel.SetNavigationStack();
    navigationModel.SetUsrNavigationMode(NavigationMode::SPLIT);
    auto navNode = AceType::DynamicCast<NavigationGroupNode>(ViewStackProcessor::GetInstance()->GetMainFrameNode());
    ASSERT_NE(navNode, nullptr);
    auto pattern = navNode->GetPattern<NavigationPattern>();
    ASSERT_NE(pattern, nullptr);
    auto& stack = pattern->navigationStack_;
    ASSERT_NE(stack, nullptr);
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto manager = context->GetForceSplitManager();
    ASSERT_NE(manager, nullptr);
    manager->enablePageLevelSplit_ = true;
    manager->SetHomePageName("");
    stack->navPathList_.clear();
    pattern->forceSplitHomeDest_ = nullptr;

    pattern->TryEstablishPageLevelSplitHome();
    EXPECT_EQ(pattern->forceSplitHomeDest_.Upgrade(), nullptr);
    EXPECT_TRUE(manager->GetHomePageName().empty());
}

/**
 * @tc.name: TryEstablishPageLevelSplitHome003
 * @tc.desc: Branch: top path node is not a NavDestination => topPattern null, early return
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestElevenNg, TryEstablishPageLevelSplitHome003, TestSize.Level1)
{
    NavigationPatternTestElevenNg::SetUpTestSuite();
    MockPipelineContextGetTheme();
    NavigationModelNG navigationModel;
    navigationModel.Create();
    navigationModel.SetNavigationStack();
    navigationModel.SetUsrNavigationMode(NavigationMode::SPLIT);
    auto navNode = AceType::DynamicCast<NavigationGroupNode>(ViewStackProcessor::GetInstance()->GetMainFrameNode());
    ASSERT_NE(navNode, nullptr);
    auto pattern = navNode->GetPattern<NavigationPattern>();
    ASSERT_NE(pattern, nullptr);
    auto& stack = pattern->navigationStack_;
    ASSERT_NE(stack, nullptr);
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto manager = context->GetForceSplitManager();
    ASSERT_NE(manager, nullptr);
    manager->enablePageLevelSplit_ = true;
    manager->SetHomePageName("");
    stack->navPathList_.clear();
    stack->navPathList_.push_back({ "PageA", nullptr });
    pattern->forceSplitHomeDest_ = nullptr;

    pattern->TryEstablishPageLevelSplitHome();
    EXPECT_EQ(pattern->forceSplitHomeDest_.Upgrade(), nullptr);
    EXPECT_TRUE(manager->GetHomePageName().empty());
}

/**
 * @tc.name: TryEstablishPageLevelSplitHome004
 * @tc.desc: Branch: if (forceSplitMgr->IsPageSplitDisabled(topPattern->GetName())) { => true
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestElevenNg, TryEstablishPageLevelSplitHome004, TestSize.Level1)
{
    NavigationPatternTestElevenNg::SetUpTestSuite();
    MockPipelineContextGetTheme();
    NavigationModelNG navigationModel;
    navigationModel.Create();
    navigationModel.SetNavigationStack();
    navigationModel.SetUsrNavigationMode(NavigationMode::SPLIT);
    auto navNode = AceType::DynamicCast<NavigationGroupNode>(ViewStackProcessor::GetInstance()->GetMainFrameNode());
    ASSERT_NE(navNode, nullptr);
    auto pattern = navNode->GetPattern<NavigationPattern>();
    ASSERT_NE(pattern, nullptr);
    auto& stack = pattern->navigationStack_;
    ASSERT_NE(stack, nullptr);
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto manager = context->GetForceSplitManager();
    ASSERT_NE(manager, nullptr);
    manager->enablePageLevelSplit_ = true;
    manager->pageSplitOptions_ = { { "PageA", false } };
    manager->SetHomePageName("");

    auto dest = NavDestinationGroupNode::GetOrCreateGroupNode(
        V2::NAVDESTINATION_VIEW_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        []() { return AceType::MakeRefPtr<NavDestinationPattern>(); });
    ASSERT_NE(dest, nullptr);
    dest->GetPattern<NavDestinationPattern>()->SetName("PageA");
    stack->navPathList_.clear();
    stack->navPathList_.push_back({ "PageA", dest });
    pattern->forceSplitHomeDest_ = nullptr;

    pattern->TryEstablishPageLevelSplitHome();
    EXPECT_EQ(pattern->forceSplitHomeDest_.Upgrade(), nullptr);
    EXPECT_TRUE(manager->GetHomePageName().empty());
}

/**
 * @tc.name: TryEstablishPageLevelSplitHome005
 * @tc.desc: Branch: no home established, NAVIGATION mode => current top split page becomes home
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestElevenNg, TryEstablishPageLevelSplitHome005, TestSize.Level1)
{
    NavigationPatternTestElevenNg::SetUpTestSuite();
    MockPipelineContextGetTheme();
    NavigationModelNG navigationModel;
    navigationModel.Create();
    navigationModel.SetNavigationStack();
    navigationModel.SetUsrNavigationMode(NavigationMode::SPLIT);
    auto navNode = AceType::DynamicCast<NavigationGroupNode>(ViewStackProcessor::GetInstance()->GetMainFrameNode());
    ASSERT_NE(navNode, nullptr);
    auto pattern = navNode->GetPattern<NavigationPattern>();
    ASSERT_NE(pattern, nullptr);
    auto& stack = pattern->navigationStack_;
    ASSERT_NE(stack, nullptr);
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto manager = context->GetForceSplitManager();
    ASSERT_NE(manager, nullptr);
    manager->enablePageLevelSplit_ = true;
    manager->pageSplitOptions_ = { { "PageA", true } };
    manager->SetHomePageName("");

    auto dest = NavDestinationGroupNode::GetOrCreateGroupNode(
        V2::NAVDESTINATION_VIEW_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        []() { return AceType::MakeRefPtr<NavDestinationPattern>(); });
    ASSERT_NE(dest, nullptr);
    dest->GetPattern<NavDestinationPattern>()->SetName("PageA");
    stack->navPathList_.clear();
    stack->navPathList_.push_back({ "PageA", dest });
    pattern->forceSplitHomeDest_ = nullptr;
    pattern->navBarIsHome_ = false;

    // This case exercises destination anchoring with no usable NavBar.
    navNode->GetLayoutProperty<NavigationLayoutProperty>()->UpdateHideNavBar(true);
    pattern->TryEstablishPageLevelSplitHome();
    EXPECT_EQ(pattern->forceSplitHomeDest_.Upgrade(), dest);
    EXPECT_EQ(manager->GetHomePageName(), "PageA");
    EXPECT_EQ(dest->GetNavDestinationType(), NavDestinationType::HOME);
    EXPECT_FALSE(pattern->navBarIsHome_);
}

/**
 * @tc.name: TryEstablishPageLevelSplitHome006
 * @tc.desc: Branch: if (!forceSplitMgr->GetHomePageName().empty()) { => true
 *                   if (homeInStack && belowTopIsSplit) { => true, keep established home
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestElevenNg, TryEstablishPageLevelSplitHome006, TestSize.Level1)
{
    NavigationPatternTestElevenNg::SetUpTestSuite();
    MockPipelineContextGetTheme();
    NavigationModelNG navigationModel;
    navigationModel.Create();
    navigationModel.SetNavigationStack();
    navigationModel.SetUsrNavigationMode(NavigationMode::SPLIT);
    auto navNode = AceType::DynamicCast<NavigationGroupNode>(ViewStackProcessor::GetInstance()->GetMainFrameNode());
    ASSERT_NE(navNode, nullptr);
    auto pattern = navNode->GetPattern<NavigationPattern>();
    ASSERT_NE(pattern, nullptr);
    auto& stack = pattern->navigationStack_;
    ASSERT_NE(stack, nullptr);
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto manager = context->GetForceSplitManager();
    ASSERT_NE(manager, nullptr);
    manager->enablePageLevelSplit_ = true;
    manager->pageSplitOptions_ = { { "PageA", true }, { "PageB", true } };

    auto destA = NavDestinationGroupNode::GetOrCreateGroupNode(
        V2::NAVDESTINATION_VIEW_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        []() { return AceType::MakeRefPtr<NavDestinationPattern>(); });
    ASSERT_NE(destA, nullptr);
    destA->GetPattern<NavDestinationPattern>()->SetName("PageA");
    auto destB = NavDestinationGroupNode::GetOrCreateGroupNode(
        V2::NAVDESTINATION_VIEW_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        []() { return AceType::MakeRefPtr<NavDestinationPattern>(); });
    ASSERT_NE(destB, nullptr);
    destB->GetPattern<NavDestinationPattern>()->SetName("PageB");
    stack->navPathList_.clear();
    stack->navPathList_.push_back({ "PageA", destA });
    stack->navPathList_.push_back({ "PageB", destB });
    manager->SetHomePageName("PageA");
    destA->SetNavDestinationType(NavDestinationType::HOME);
    pattern->forceSplitHomeDest_ = WeakPtr(destA);

    // Exercise continuity/reanchoring after a stack change in an active split.
    pattern->forceSplitSuccess_ = true;
    pattern->pageLevelSplitStackChanged_ = true;
    pattern->pageLevelDisplaceSessionActive_ = false;
    pattern->navBarIsHome_ = false;
    pattern->TryEstablishPageLevelSplitHome();
    EXPECT_EQ(pattern->forceSplitHomeDest_.Upgrade(), destA);
    EXPECT_EQ(manager->GetHomePageName(), "PageA");
    EXPECT_NE(destB->GetNavDestinationType(), NavDestinationType::HOME);
}

/**
 * @tc.name: TryEstablishPageLevelSplitHome007
 * @tc.desc: Branch: home in stack but below-top is non-split => re-anchor home to top
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestElevenNg, TryEstablishPageLevelSplitHome007, TestSize.Level1)
{
    NavigationPatternTestElevenNg::SetUpTestSuite();
    MockPipelineContextGetTheme();
    NavigationModelNG navigationModel;
    navigationModel.Create();
    navigationModel.SetNavigationStack();
    navigationModel.SetUsrNavigationMode(NavigationMode::SPLIT);
    auto navNode = AceType::DynamicCast<NavigationGroupNode>(ViewStackProcessor::GetInstance()->GetMainFrameNode());
    ASSERT_NE(navNode, nullptr);
    auto pattern = navNode->GetPattern<NavigationPattern>();
    ASSERT_NE(pattern, nullptr);
    auto& stack = pattern->navigationStack_;
    ASSERT_NE(stack, nullptr);
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto manager = context->GetForceSplitManager();
    ASSERT_NE(manager, nullptr);
    manager->enablePageLevelSplit_ = true;
    manager->pageSplitOptions_ = { { "PageA", true }, { "PageB", false }, { "PageC", true } };

    auto destA = NavDestinationGroupNode::GetOrCreateGroupNode(
        V2::NAVDESTINATION_VIEW_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        []() { return AceType::MakeRefPtr<NavDestinationPattern>(); });
    ASSERT_NE(destA, nullptr);
    destA->GetPattern<NavDestinationPattern>()->SetName("PageA");
    auto destB = NavDestinationGroupNode::GetOrCreateGroupNode(
        V2::NAVDESTINATION_VIEW_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        []() { return AceType::MakeRefPtr<NavDestinationPattern>(); });
    ASSERT_NE(destB, nullptr);
    destB->GetPattern<NavDestinationPattern>()->SetName("PageB");
    auto destC = NavDestinationGroupNode::GetOrCreateGroupNode(
        V2::NAVDESTINATION_VIEW_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        []() { return AceType::MakeRefPtr<NavDestinationPattern>(); });
    ASSERT_NE(destC, nullptr);
    destC->GetPattern<NavDestinationPattern>()->SetName("PageC");
    stack->navPathList_.clear();
    stack->navPathList_.push_back({ "PageA", destA });
    stack->navPathList_.push_back({ "PageB", destB });
    stack->navPathList_.push_back({ "PageC", destC });
    manager->SetHomePageName("PageA");
    destA->SetNavDestinationType(NavDestinationType::HOME);
    pattern->forceSplitHomeDest_ = WeakPtr(destA);

    // Exercise continuity/reanchoring after a stack change in an active split.
    pattern->forceSplitSuccess_ = true;
    pattern->pageLevelSplitStackChanged_ = true;
    pattern->pageLevelDisplaceSessionActive_ = false;
    pattern->navBarIsHome_ = false;
    pattern->TryEstablishPageLevelSplitHome();
    EXPECT_EQ(pattern->forceSplitHomeDest_.Upgrade(), destC);
    EXPECT_EQ(manager->GetHomePageName(), "PageC");
    EXPECT_EQ(destC->GetNavDestinationType(), NavDestinationType::HOME);
}

/**
 * @tc.name: TryEstablishPageLevelSplitHome008
 * @tc.desc: After a split home above a full-screen page is popped, restore the first
 *           split page of the remaining stack as home, including duplicate page names.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestElevenNg, TryEstablishPageLevelSplitHome008, TestSize.Level1)
{
    NavigationPatternTestElevenNg::SetUpTestSuite();
    MockPipelineContextGetTheme();
    NavigationModelNG navigationModel;
    navigationModel.Create();
    navigationModel.SetNavigationStack();
    navigationModel.SetUsrNavigationMode(NavigationMode::SPLIT);
    auto navNode = AceType::DynamicCast<NavigationGroupNode>(ViewStackProcessor::GetInstance()->GetMainFrameNode());
    ASSERT_NE(navNode, nullptr);
    auto pattern = navNode->GetPattern<NavigationPattern>();
    ASSERT_NE(pattern, nullptr);
    auto& stack = pattern->navigationStack_;
    ASSERT_NE(stack, nullptr);
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto manager = context->GetForceSplitManager();
    ASSERT_NE(manager, nullptr);
    manager->enablePageLevelSplit_ = true;
    manager->pageSplitOptions_ = { { "detail1", true } };

    auto destA = NavDestinationGroupNode::GetOrCreateGroupNode(
        V2::NAVDESTINATION_VIEW_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        []() { return AceType::MakeRefPtr<NavDestinationPattern>(); });
    ASSERT_NE(destA, nullptr);
    destA->GetPattern<NavDestinationPattern>()->SetName("detail1");
    auto destB = NavDestinationGroupNode::GetOrCreateGroupNode(
        V2::NAVDESTINATION_VIEW_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        []() { return AceType::MakeRefPtr<NavDestinationPattern>(); });
    ASSERT_NE(destB, nullptr);
    destB->GetPattern<NavDestinationPattern>()->SetName("detail1");
    auto poppedHome = NavDestinationGroupNode::GetOrCreateGroupNode(
        V2::NAVDESTINATION_VIEW_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        []() { return AceType::MakeRefPtr<NavDestinationPattern>(); });
    ASSERT_NE(poppedHome, nullptr);
    poppedHome->GetPattern<NavDestinationPattern>()->SetName("detail1");
    stack->navPathList_.clear();
    stack->navPathList_.push_back({ "detail1", destA });
    stack->navPathList_.push_back({ "detail1", destB });
    manager->SetHomePageName("detail1");
    pattern->forceSplitHomeDest_ = WeakPtr(poppedHome);

    // Exercise continuity/reanchoring after a stack change in an active split.
    pattern->forceSplitSuccess_ = true;
    pattern->pageLevelSplitStackChanged_ = true;
    pattern->pageLevelDisplaceSessionActive_ = false;
    pattern->navBarIsHome_ = false;
    pattern->TryEstablishPageLevelSplitHome();
    EXPECT_EQ(pattern->forceSplitHomeDest_.Upgrade(), destA);
    EXPECT_EQ(manager->GetHomePageName(), "detail1");
    EXPECT_EQ(destA->GetNavDestinationType(), NavDestinationType::HOME);
    EXPECT_NE(destB->GetNavDestinationType(), NavDestinationType::HOME);
}

/**
 * @tc.name: TryEstablishPageLevelSplitHome009
 * @tc.desc: Branch: if (allDestNodes.size() >= SECOND_ZINDEX_VALUE) { => false
 *           single destination stack has no below-top page, re-anchor home to the top itself
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestElevenNg, TryEstablishPageLevelSplitHome009, TestSize.Level1)
{
    NavigationPatternTestElevenNg::SetUpTestSuite();
    MockPipelineContextGetTheme();
    NavigationModelNG navigationModel;
    navigationModel.Create();
    navigationModel.SetNavigationStack();
    navigationModel.SetUsrNavigationMode(NavigationMode::SPLIT);
    auto navNode = AceType::DynamicCast<NavigationGroupNode>(ViewStackProcessor::GetInstance()->GetMainFrameNode());
    ASSERT_NE(navNode, nullptr);
    auto pattern = navNode->GetPattern<NavigationPattern>();
    ASSERT_NE(pattern, nullptr);
    auto& stack = pattern->navigationStack_;
    ASSERT_NE(stack, nullptr);
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto manager = context->GetForceSplitManager();
    ASSERT_NE(manager, nullptr);
    manager->enablePageLevelSplit_ = true;
    manager->pageSplitOptions_ = { { "PageA", true } };

    auto dest = NavDestinationGroupNode::GetOrCreateGroupNode(
        V2::NAVDESTINATION_VIEW_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        []() { return AceType::MakeRefPtr<NavDestinationPattern>(); });
    ASSERT_NE(dest, nullptr);
    dest->GetPattern<NavDestinationPattern>()->SetName("PageA");
    stack->navPathList_.clear();
    stack->navPathList_.push_back({ "PageA", dest });
    // stale home from previous session, not in current stack
    manager->SetHomePageName("PageX");
    pattern->forceSplitHomeDest_ = nullptr;

    // This case exercises destination anchoring with no usable NavBar.
    navNode->GetLayoutProperty<NavigationLayoutProperty>()->UpdateHideNavBar(true);
    // Exercise continuity/reanchoring after a stack change in an active split.
    pattern->forceSplitSuccess_ = true;
    pattern->pageLevelSplitStackChanged_ = true;
    pattern->pageLevelDisplaceSessionActive_ = false;
    pattern->navBarIsHome_ = false;
    pattern->TryEstablishPageLevelSplitHome();
    EXPECT_EQ(pattern->forceSplitHomeDest_.Upgrade(), dest);
    EXPECT_EQ(manager->GetHomePageName(), "PageA");
}

/**
 * @tc.name: TryEstablishPageLevelSplitHome010
 * @tc.desc: Branch: if (GetBehaviorMode() == DISPLACE && allDestNodes.size() >= 2) { => true
 *           displace mode anchors the page below top as home
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestElevenNg, TryEstablishPageLevelSplitHome010, TestSize.Level1)
{
    NavigationPatternTestElevenNg::SetUpTestSuite();
    MockPipelineContextGetTheme();
    NavigationModelNG navigationModel;
    navigationModel.Create();
    navigationModel.SetNavigationStack();
    navigationModel.SetUsrNavigationMode(NavigationMode::SPLIT);
    auto navNode = AceType::DynamicCast<NavigationGroupNode>(ViewStackProcessor::GetInstance()->GetMainFrameNode());
    ASSERT_NE(navNode, nullptr);
    auto pattern = navNode->GetPattern<NavigationPattern>();
    ASSERT_NE(pattern, nullptr);
    auto& stack = pattern->navigationStack_;
    ASSERT_NE(stack, nullptr);
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto manager = context->GetForceSplitManager();
    ASSERT_NE(manager, nullptr);
    manager->enablePageLevelSplit_ = true;
    manager->SetBehaviorMode(ForceSplitBehaviorMode::DISPLACE);
    manager->pageSplitOptions_ = { { "PageA", true }, { "PageB", true } };
    manager->SetHomePageName("");

    auto destA = NavDestinationGroupNode::GetOrCreateGroupNode(
        V2::NAVDESTINATION_VIEW_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        []() { return AceType::MakeRefPtr<NavDestinationPattern>(); });
    ASSERT_NE(destA, nullptr);
    destA->GetPattern<NavDestinationPattern>()->SetName("PageA");
    auto destB = NavDestinationGroupNode::GetOrCreateGroupNode(
        V2::NAVDESTINATION_VIEW_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        []() { return AceType::MakeRefPtr<NavDestinationPattern>(); });
    ASSERT_NE(destB, nullptr);
    destB->GetPattern<NavDestinationPattern>()->SetName("PageB");
    stack->navPathList_.clear();
    stack->navPathList_.push_back({ "PageA", destA });
    stack->navPathList_.push_back({ "PageB", destB });
    pattern->forceSplitHomeDest_ = nullptr;

    pattern->TryEstablishPageLevelSplitHome();
    EXPECT_EQ(pattern->forceSplitHomeDest_.Upgrade(), destA);
    EXPECT_EQ(manager->GetHomePageName(), "PageA");
    EXPECT_EQ(destA->GetNavDestinationType(), NavDestinationType::HOME);
}

/**
 * @tc.name: TryEstablishPageLevelSplitHome011
 * @tc.desc: Branch: if (GetBehaviorMode() == DISPLACE && allDestNodes.size() >= 2) { => false
 *           displace mode with single destination anchors top itself as home
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestElevenNg, TryEstablishPageLevelSplitHome011, TestSize.Level1)
{
    NavigationPatternTestElevenNg::SetUpTestSuite();
    MockPipelineContextGetTheme();
    NavigationModelNG navigationModel;
    navigationModel.Create();
    navigationModel.SetNavigationStack();
    navigationModel.SetUsrNavigationMode(NavigationMode::SPLIT);
    auto navNode = AceType::DynamicCast<NavigationGroupNode>(ViewStackProcessor::GetInstance()->GetMainFrameNode());
    ASSERT_NE(navNode, nullptr);
    auto pattern = navNode->GetPattern<NavigationPattern>();
    ASSERT_NE(pattern, nullptr);
    auto& stack = pattern->navigationStack_;
    ASSERT_NE(stack, nullptr);
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto manager = context->GetForceSplitManager();
    ASSERT_NE(manager, nullptr);
    manager->enablePageLevelSplit_ = true;
    manager->SetBehaviorMode(ForceSplitBehaviorMode::DISPLACE);
    manager->pageSplitOptions_ = { { "PageA", true } };
    manager->SetHomePageName("");

    auto dest = NavDestinationGroupNode::GetOrCreateGroupNode(
        V2::NAVDESTINATION_VIEW_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        []() { return AceType::MakeRefPtr<NavDestinationPattern>(); });
    ASSERT_NE(dest, nullptr);
    dest->GetPattern<NavDestinationPattern>()->SetName("PageA");
    stack->navPathList_.clear();
    stack->navPathList_.push_back({ "PageA", dest });
    pattern->forceSplitHomeDest_ = nullptr;

    // This case exercises destination anchoring with no usable NavBar.
    navNode->GetLayoutProperty<NavigationLayoutProperty>()->UpdateHideNavBar(true);
    pattern->TryEstablishPageLevelSplitHome();
    EXPECT_EQ(pattern->forceSplitHomeDest_.Upgrade(), dest);
    EXPECT_EQ(manager->GetHomePageName(), "PageA");
}

/**
 * @tc.name: IsTopNavDestinationSplitDisabled001
 * @tc.desc: Branch: if (pathNames.empty()) { => true
 *                   return forceSplitMgr->IsPageLevelSplitEnabled()
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestElevenNg, IsTopNavDestinationSplitDisabled001, TestSize.Level1)
{
    NavigationPatternTestElevenNg::SetUpTestSuite();
    MockPipelineContextGetTheme();
    NavigationModelNG navigationModel;
    navigationModel.Create();
    navigationModel.SetNavigationStack();
    navigationModel.SetUsrNavigationMode(NavigationMode::SPLIT);
    auto navNode = AceType::DynamicCast<NavigationGroupNode>(ViewStackProcessor::GetInstance()->GetMainFrameNode());
    ASSERT_NE(navNode, nullptr);
    auto pattern = navNode->GetPattern<NavigationPattern>();
    ASSERT_NE(pattern, nullptr);
    auto& stack = pattern->navigationStack_;
    ASSERT_NE(stack, nullptr);
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto manager = context->GetForceSplitManager();
    ASSERT_NE(manager, nullptr);
    stack->navPathList_.clear();

    manager->enablePageLevelSplit_ = true;
    EXPECT_TRUE(pattern->IsTopNavDestinationSplitDisabled());
    manager->enablePageLevelSplit_ = false;
    EXPECT_FALSE(pattern->IsTopNavDestinationSplitDisabled());
}

/**
 * @tc.name: IsTopNavDestinationSplitDisabled002
 * @tc.desc: Branch: if (pathNames.empty()) { => false
 *                   page stack synced and top disabled by explicit option
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestElevenNg, IsTopNavDestinationSplitDisabled002, TestSize.Level1)
{
    NavigationPatternTestElevenNg::SetUpTestSuite();
    MockPipelineContextGetTheme();
    NavigationModelNG navigationModel;
    navigationModel.Create();
    navigationModel.SetNavigationStack();
    navigationModel.SetUsrNavigationMode(NavigationMode::SPLIT);
    auto navNode = AceType::DynamicCast<NavigationGroupNode>(ViewStackProcessor::GetInstance()->GetMainFrameNode());
    ASSERT_NE(navNode, nullptr);
    auto pattern = navNode->GetPattern<NavigationPattern>();
    ASSERT_NE(pattern, nullptr);
    auto& stack = pattern->navigationStack_;
    ASSERT_NE(stack, nullptr);
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto manager = context->GetForceSplitManager();
    ASSERT_NE(manager, nullptr);
    manager->enablePageLevelSplit_ = true;
    manager->pageSplitOptions_ = { { "PageB", false } };
    manager->pageStack_.clear();

    auto destA = NavDestinationGroupNode::GetOrCreateGroupNode(
        V2::NAVDESTINATION_VIEW_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        []() { return AceType::MakeRefPtr<NavDestinationPattern>(); });
    ASSERT_NE(destA, nullptr);
    destA->GetPattern<NavDestinationPattern>()->SetName("PageA");
    auto destB = NavDestinationGroupNode::GetOrCreateGroupNode(
        V2::NAVDESTINATION_VIEW_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        []() { return AceType::MakeRefPtr<NavDestinationPattern>(); });
    ASSERT_NE(destB, nullptr);
    destB->GetPattern<NavDestinationPattern>()->SetName("PageB");
    stack->navPathList_.clear();
    stack->navPathList_.push_back({ "PageA", destA });
    stack->navPathList_.push_back({ "PageB", destB });

    EXPECT_TRUE(pattern->IsTopNavDestinationSplitDisabled());
    EXPECT_EQ(manager->pageStack_, (std::vector<std::string> { "PageA", "PageB" }));
}

/**
 * @tc.name: IsTopNavDestinationSplitDisabled003
 * @tc.desc: Branch: top destination split enabled by explicit option => false
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestElevenNg, IsTopNavDestinationSplitDisabled003, TestSize.Level1)
{
    NavigationPatternTestElevenNg::SetUpTestSuite();
    MockPipelineContextGetTheme();
    NavigationModelNG navigationModel;
    navigationModel.Create();
    navigationModel.SetNavigationStack();
    navigationModel.SetUsrNavigationMode(NavigationMode::SPLIT);
    auto navNode = AceType::DynamicCast<NavigationGroupNode>(ViewStackProcessor::GetInstance()->GetMainFrameNode());
    ASSERT_NE(navNode, nullptr);
    auto pattern = navNode->GetPattern<NavigationPattern>();
    ASSERT_NE(pattern, nullptr);
    auto& stack = pattern->navigationStack_;
    ASSERT_NE(stack, nullptr);
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto manager = context->GetForceSplitManager();
    ASSERT_NE(manager, nullptr);
    manager->enablePageLevelSplit_ = true;
    manager->pageSplitOptions_ = { { "PageB", true } };

    auto destA = NavDestinationGroupNode::GetOrCreateGroupNode(
        V2::NAVDESTINATION_VIEW_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        []() { return AceType::MakeRefPtr<NavDestinationPattern>(); });
    ASSERT_NE(destA, nullptr);
    destA->GetPattern<NavDestinationPattern>()->SetName("PageA");
    auto destB = NavDestinationGroupNode::GetOrCreateGroupNode(
        V2::NAVDESTINATION_VIEW_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        []() { return AceType::MakeRefPtr<NavDestinationPattern>(); });
    ASSERT_NE(destB, nullptr);
    destB->GetPattern<NavDestinationPattern>()->SetName("PageB");
    stack->navPathList_.clear();
    stack->navPathList_.push_back({ "PageA", destA });
    stack->navPathList_.push_back({ "PageB", destB });

    EXPECT_FALSE(pattern->IsTopNavDestinationSplitDisabled());
}

/**
 * @tc.name: NotifyForceFullScreenChangeIfNeededPageLevel001
 * @tc.desc: Branch: if (forceSplitMgr->IsPageLevelSplitEnabled()) { => true
 *                   fullScreenPages takes no effect, page stack synced, state unchanged
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestElevenNg, NotifyForceFullScreenChangeIfNeededPageLevel001, TestSize.Level1)
{
    NavigationPatternTestElevenNg::SetUpTestSuite();
    MockPipelineContextGetTheme();
    NavigationModelNG navigationModel;
    navigationModel.Create();
    navigationModel.SetNavigationStack();
    navigationModel.SetUsrNavigationMode(NavigationMode::SPLIT);
    auto navNode = AceType::DynamicCast<NavigationGroupNode>(ViewStackProcessor::GetInstance()->GetMainFrameNode());
    ASSERT_NE(navNode, nullptr);
    auto pattern = navNode->GetPattern<NavigationPattern>();
    ASSERT_NE(pattern, nullptr);
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto manager = context->GetForceSplitManager();
    ASSERT_NE(manager, nullptr);
    manager->isRouter_ = false;
    manager->isForceSplitSupported_ = true;
    manager->enablePageLevelSplit_ = true;
    manager->fullScreenPages_ = { "FullScreen" };
    manager->pageStack_.clear();
    pattern->isTopFullScreenPage_ = false;

    pattern->NotifyForceFullScreenChangeIfNeeded({ "FullScreen" });
    EXPECT_FALSE(pattern->isTopFullScreenPage_);
    EXPECT_FALSE(pattern->isTopFullScreenChanged_);
    EXPECT_EQ(manager->pageStack_, (std::vector<std::string> { "FullScreen" }));
}

/**
 * @tc.name: NotifyForceFullScreenChangeIfNeededPageLevel002
 * @tc.desc: Branch: if (forceSplitMgr->IsPageLevelSplitEnabled()) { => false
 *                   if (isTopFullScreenPage_ != isCurTopFullScreenPage) { => false
 *           page-level off: fullScreen top keeps fullscreen state
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestElevenNg, NotifyForceFullScreenChangeIfNeededPageLevel002, TestSize.Level1)
{
    NavigationPatternTestElevenNg::SetUpTestSuite();
    MockPipelineContextGetTheme();
    NavigationModelNG navigationModel;
    navigationModel.Create();
    navigationModel.SetNavigationStack();
    navigationModel.SetUsrNavigationMode(NavigationMode::SPLIT);
    auto navNode = AceType::DynamicCast<NavigationGroupNode>(ViewStackProcessor::GetInstance()->GetMainFrameNode());
    ASSERT_NE(navNode, nullptr);
    auto pattern = navNode->GetPattern<NavigationPattern>();
    ASSERT_NE(pattern, nullptr);
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto manager = context->GetForceSplitManager();
    ASSERT_NE(manager, nullptr);
    manager->isRouter_ = false;
    manager->isForceSplitSupported_ = true;
    manager->enablePageLevelSplit_ = false;
    manager->fullScreenPages_ = { "FullScreen" };
    pattern->isTopFullScreenPage_ = true;

    pattern->NotifyForceFullScreenChangeIfNeeded({ "FullScreen" });
    EXPECT_TRUE(pattern->isTopFullScreenPage_);
    EXPECT_FALSE(pattern->isTopFullScreenChanged_);
}

/**
 * @tc.name: NotifyForceFullScreenChangeIfNeededPageLevel003
 * @tc.desc: Branch: if (isTopFullScreenPage_ != isCurTopFullScreenPage) { => true
 *           page-level on clears a legacy fullscreen state
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestElevenNg, NotifyForceFullScreenChangeIfNeededPageLevel003, TestSize.Level1)
{
    NavigationPatternTestElevenNg::SetUpTestSuite();
    MockPipelineContextGetTheme();
    NavigationModelNG navigationModel;
    navigationModel.Create();
    navigationModel.SetNavigationStack();
    navigationModel.SetUsrNavigationMode(NavigationMode::SPLIT);
    auto navNode = AceType::DynamicCast<NavigationGroupNode>(ViewStackProcessor::GetInstance()->GetMainFrameNode());
    ASSERT_NE(navNode, nullptr);
    auto pattern = navNode->GetPattern<NavigationPattern>();
    ASSERT_NE(pattern, nullptr);
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto manager = context->GetForceSplitManager();
    ASSERT_NE(manager, nullptr);
    manager->isRouter_ = false;
    manager->isForceSplitSupported_ = true;
    manager->enablePageLevelSplit_ = true;
    manager->fullScreenPages_ = { "FullScreen" };
    pattern->isTopFullScreenPage_ = true;

    pattern->NotifyForceFullScreenChangeIfNeeded({ "FullScreen" });
    EXPECT_FALSE(pattern->isTopFullScreenPage_);
    EXPECT_TRUE(pattern->isTopFullScreenChanged_);
}

/**
 * @tc.name: NotifyForceFullScreenChangeIfNeededPageLevel004
 * @tc.desc: Branch: if (!allNames.empty()) { => false
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestElevenNg, NotifyForceFullScreenChangeIfNeededPageLevel004, TestSize.Level1)
{
    NavigationPatternTestElevenNg::SetUpTestSuite();
    MockPipelineContextGetTheme();
    NavigationModelNG navigationModel;
    navigationModel.Create();
    navigationModel.SetNavigationStack();
    navigationModel.SetUsrNavigationMode(NavigationMode::SPLIT);
    auto navNode = AceType::DynamicCast<NavigationGroupNode>(ViewStackProcessor::GetInstance()->GetMainFrameNode());
    ASSERT_NE(navNode, nullptr);
    auto pattern = navNode->GetPattern<NavigationPattern>();
    ASSERT_NE(pattern, nullptr);
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(context, nullptr);
    auto manager = context->GetForceSplitManager();
    ASSERT_NE(manager, nullptr);
    manager->isRouter_ = false;
    manager->isForceSplitSupported_ = true;
    manager->enablePageLevelSplit_ = true;
    pattern->isTopFullScreenPage_ = false;

    pattern->NotifyForceFullScreenChangeIfNeeded({});
    EXPECT_FALSE(pattern->isTopFullScreenPage_);
    EXPECT_FALSE(pattern->isTopFullScreenChanged_);
}

/**
 * @tc.name: UpdateNavContentAndChildVisibility001
 * @tc.desc: Branch: if (!isVisible || !forceSplitSuccess_) { => true
 *           force split off: children below home are not hidden
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestElevenNg, UpdateNavContentAndChildVisibility001, TestSize.Level1)
{
    NavigationPatternTestElevenNg::SetUpTestSuite();
    MockPipelineContextGetTheme();
    NavigationModelNG navigationModel;
    navigationModel.Create();
    navigationModel.SetNavigationStack();
    navigationModel.SetUsrNavigationMode(NavigationMode::SPLIT);
    auto navNode = AceType::DynamicCast<NavigationGroupNode>(ViewStackProcessor::GetInstance()->GetMainFrameNode());
    ASSERT_NE(navNode, nullptr);
    auto pattern = navNode->GetPattern<NavigationPattern>();
    ASSERT_NE(pattern, nullptr);

    auto navContentNode = FrameNode::CreateFrameNode(V2::NAVIGATION_CONTENT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(navContentNode, nullptr);

    // isVisible=false breaks immediately
    pattern->forceSplitSuccess_ = true;
    pattern->UpdateNavContentAndChildVisibility(navContentNode, false);
    EXPECT_EQ(navContentNode->GetLayoutProperty()->GetVisibilityValue(VisibleType::VISIBLE), VisibleType::INVISIBLE);

    // force split failed: children below home are skipped even when visible
    pattern->forceSplitSuccess_ = false;
    pattern->UpdateNavContentAndChildVisibility(navContentNode, true);
    EXPECT_EQ(navContentNode->GetLayoutProperty()->GetVisibilityValue(VisibleType::VISIBLE), VisibleType::VISIBLE);
}

/**
 * @tc.name: UpdateNavContentAndChildVisibility002
 * @tc.desc: Branch: if (!isVisible || !forceSplitSuccess_) { => false
 *                   if (destNode->GetIndex() >= homeIndex) { => true
 *           force split on: destinations below home become invisible
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestElevenNg, UpdateNavContentAndChildVisibility002, TestSize.Level1)
{
    NavigationPatternTestElevenNg::SetUpTestSuite();
    MockPipelineContextGetTheme();
    NavigationModelNG navigationModel;
    navigationModel.Create();
    navigationModel.SetNavigationStack();
    navigationModel.SetUsrNavigationMode(NavigationMode::SPLIT);
    auto navNode = AceType::DynamicCast<NavigationGroupNode>(ViewStackProcessor::GetInstance()->GetMainFrameNode());
    ASSERT_NE(navNode, nullptr);
    auto pattern = navNode->GetPattern<NavigationPattern>();
    ASSERT_NE(pattern, nullptr);

    auto navContentNode = FrameNode::CreateFrameNode(V2::NAVIGATION_CONTENT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(navContentNode, nullptr);

    auto belowHomeDest = NavDestinationGroupNode::GetOrCreateGroupNode(
        V2::NAVDESTINATION_VIEW_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        []() { return AceType::MakeRefPtr<NavDestinationPattern>(); });
    ASSERT_NE(belowHomeDest, nullptr);
    belowHomeDest->SetIndex(0);
    auto homeDest = NavDestinationGroupNode::GetOrCreateGroupNode(
        V2::NAVDESTINATION_VIEW_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        []() { return AceType::MakeRefPtr<NavDestinationPattern>(); });
    ASSERT_NE(homeDest, nullptr);
    homeDest->SetIndex(1);
    belowHomeDest->MountToParent(navContentNode);
    homeDest->MountToParent(navContentNode);

    pattern->forceSplitSuccess_ = true;
    pattern->forceSplitHomeDest_ = WeakPtr(homeDest);
    pattern->UpdateNavContentAndChildVisibility(navContentNode, true);
    EXPECT_EQ(belowHomeDest->GetLayoutProperty()->GetVisibilityValue(VisibleType::VISIBLE), VisibleType::INVISIBLE);
}

/**
 * @tc.name: UpdateNavContentAndChildVisibility003
 * @tc.desc: Branch: auto homeNode = forceSplitHomeDest_.Upgrade(); CHECK_NULL_BREAK(homeNode)
 *           force split on without home: loop breaks, no child is hidden
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestElevenNg, UpdateNavContentAndChildVisibility003, TestSize.Level1)
{
    NavigationPatternTestElevenNg::SetUpTestSuite();
    MockPipelineContextGetTheme();
    NavigationModelNG navigationModel;
    navigationModel.Create();
    navigationModel.SetNavigationStack();
    navigationModel.SetUsrNavigationMode(NavigationMode::SPLIT);
    auto navNode = AceType::DynamicCast<NavigationGroupNode>(ViewStackProcessor::GetInstance()->GetMainFrameNode());
    ASSERT_NE(navNode, nullptr);
    auto pattern = navNode->GetPattern<NavigationPattern>();
    ASSERT_NE(pattern, nullptr);

    auto navContentNode = FrameNode::CreateFrameNode(V2::NAVIGATION_CONTENT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(navContentNode, nullptr);
    auto belowHomeDest = NavDestinationGroupNode::GetOrCreateGroupNode(
        V2::NAVDESTINATION_VIEW_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        []() { return AceType::MakeRefPtr<NavDestinationPattern>(); });
    ASSERT_NE(belowHomeDest, nullptr);
    belowHomeDest->SetIndex(0);
    belowHomeDest->MountToParent(navContentNode);

    pattern->forceSplitSuccess_ = true;
    pattern->forceSplitHomeDest_ = nullptr;
    pattern->UpdateNavContentAndChildVisibility(navContentNode, true);
    EXPECT_EQ(belowHomeDest->GetLayoutProperty()->GetVisibilityValue(VisibleType::VISIBLE), VisibleType::VISIBLE);
}

void NavigationPatternTestElevenNg::InitPageLevelSplitTest()
{
    SetUpTestSuite();
    pageLevelSplitTestInitialized_ = true;
    CreateAndBindNavigationBarTheme();
    NavigationModelNG model;
    model.Create();
    stack_ = AceType::MakeRefPtr<PageLevelSplitNavigationStack>();
    model.SetNavigationStack(stack_);
    navigation_ = AceType::DynamicCast<NavigationGroupNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(navigation_, nullptr);
    pattern_ = navigation_->GetPattern<NavigationPattern>();
    ASSERT_NE(pattern_, nullptr);
    manager_ = MockPipelineContext::GetCurrent()->GetForceSplitManager();
    ASSERT_NE(manager_, nullptr);
    manager_->SetForceSplitSupported(true);
    manager_->SetEnablePageLevelSplit(true);
    manager_->SetBehaviorMode(ForceSplitBehaviorMode::NAVIGATION);
    manager_->SetEnableInheritSplitStrategy(false);
    manager_->SetPageSplitOptions({ { "detail1", true }, { "detail2", false } });
    pattern_->SetIsTargetForceSplitNav(true);
}

void NavigationPatternTestElevenNg::TearDown()
{
    if (!pageLevelSplitTestInitialized_) {
        return;
    }
    pattern_ = nullptr;
    navigation_ = nullptr;
    stack_ = nullptr;
    manager_ = nullptr;
    TearDownTestSuite();
    pageLevelSplitTestInitialized_ = false;
}

RefPtr<NavDestinationGroupNode> NavigationPatternTestElevenNg::CreateDestination(const std::string& name)
{
    auto destination = NavDestinationGroupNode::GetOrCreateGroupNode(
        V2::NAVDESTINATION_VIEW_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        []() { return AceType::MakeRefPtr<NavDestinationPattern>(); });
    destination->GetPattern<NavDestinationPattern>()->SetName(name);
    destination->SetNavDestinationType(NavDestinationType::DETAIL);
    destination->SetColumnType(ForceSplitPageColumnType::SECONDARY);
    return destination;
}

void NavigationPatternTestElevenNg::SetStack(const NavPathList& paths)
{
    stack_->SetNavPathList(paths);
    std::vector<std::string> names;
    int32_t index = 0;
    for (const auto& path : paths) {
        names.push_back(path.first);
        auto destination = AceType::DynamicCast<NavDestinationGroupNode>(
            NavigationGroupNode::GetNavDestinationNode(path.second));
        if (destination) {
            destination->SetIndex(index);
        }
        ++index;
    }
    manager_->SetPageStack(std::move(names));
}

void NavigationPatternTestElevenNg::SyncPortraitColumns(const NavPathList& before, const NavPathList& after)
{
    SetStack(before);
    stack_->SavePreNavList();
    SetStack(after);
    pattern_->UpdatePageLevelDisplaceColumns(before.back(), after.back());
}

/**
 * @tc.name: CollectPageLevelSplitDestinations001
 * @tc.desc: An empty stack, invalid top node and disabled top policy do not establish a home.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestElevenNg, CollectPageLevelSplitDestinations001, TestSize.Level1)
{
    ASSERT_NO_FATAL_FAILURE(InitPageLevelSplitTest());
    EXPECT_TRUE(pattern_->CollectPageLevelSplitDestinations(manager_, navigation_->GetId()).empty());
    auto invalid = FrameNode::CreateFrameNode("invalid", ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<Pattern>());
    SetStack({ { "invalid", invalid } });
    EXPECT_TRUE(pattern_->CollectPageLevelSplitDestinations(manager_, navigation_->GetId()).empty());
    auto disabled = CreateDestination("detail2");
    SetStack({ { "detail2", disabled } });
    pattern_->TryEstablishPageLevelSplitHome();
    EXPECT_EQ(pattern_->forceSplitHomeDest_.Upgrade(), nullptr);
    EXPECT_FALSE(pattern_->navBarIsHome_);
}

/**
 * @tc.name: CollectPageLevelSplitDestinations002
 * @tc.desc: Invalid entries are skipped while distinct destination instances retain stack order.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestElevenNg, CollectPageLevelSplitDestinations002, TestSize.Level1)
{
    ASSERT_NO_FATAL_FAILURE(InitPageLevelSplitTest());
    auto first = CreateDestination("detail1");
    auto second = CreateDestination("detail1");
    SetStack({ { "missing", nullptr }, { "detail1", first }, { "detail1", second } });
    auto destinations = pattern_->CollectPageLevelSplitDestinations(manager_, navigation_->GetId());
    ASSERT_EQ(destinations.size(), 2);
    EXPECT_EQ(destinations.front(), first);
    EXPECT_EQ(destinations.back(), second);
}

/**
 * @tc.name: TryEstablishPageLevelSplitHome012
 * @tc.desc: The first split destination uses a valid NavBar and demotes an old destination home.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestElevenNg, TryEstablishPageLevelSplitHome012, TestSize.Level1)
{
    ASSERT_NO_FATAL_FAILURE(InitPageLevelSplitTest());
    auto oldHome = CreateDestination("oldHome");
    oldHome->SetNavDestinationType(NavDestinationType::HOME);
    pattern_->forceSplitHomeDest_ = WeakPtr(oldHome);
    auto first = CreateDestination("detail1");
    SetStack({ { "detail1", first } });
    ASSERT_TRUE(pattern_->IsNavBarValid());
    pattern_->TryEstablishPageLevelSplitHome();
    EXPECT_TRUE(pattern_->navBarIsHome_);
    EXPECT_EQ(pattern_->forceSplitHomeDest_.Upgrade(), nullptr);
    EXPECT_EQ(oldHome->GetNavDestinationType(), NavDestinationType::DETAIL);
    EXPECT_EQ(first->GetNavDestinationType(), NavDestinationType::DETAIL);
}

/**
 * @tc.name: TryEstablishPageLevelSplitHome013
 * @tc.desc: DISPLACE reentry after a non-split page anchors that page, including duplicate split names.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestElevenNg, TryEstablishPageLevelSplitHome013, TestSize.Level1)
{
    ASSERT_NO_FATAL_FAILURE(InitPageLevelSplitTest());
    manager_->SetBehaviorMode(ForceSplitBehaviorMode::DISPLACE);
    auto first = CreateDestination("detail1");
    auto fullScreen = CreateDestination("detail2");
    auto second = CreateDestination("detail1");
    SetStack({ { "detail1", first } });
    pattern_->TryEstablishPageLevelSplitHome();
    ASSERT_TRUE(pattern_->navBarIsHome_);
    pattern_->pageLevelSplitStackChanged_ = true;
    pattern_->pageLevelDisplaceSessionActive_ = false;
    pattern_->forceSplitSuccess_ = false;
    SetStack({ { "detail1", first }, { "detail2", fullScreen }, { "detail1", second } });
    pattern_->TryEstablishPageLevelSplitHome();
    EXPECT_FALSE(pattern_->navBarIsHome_);
    EXPECT_EQ(pattern_->forceSplitHomeDest_.Upgrade(), fullScreen);
    EXPECT_EQ(fullScreen->GetNavDestinationType(), NavDestinationType::HOME);
    EXPECT_EQ(second->GetNavDestinationType(), NavDestinationType::DETAIL);
    EXPECT_EQ(manager_->GetHomePageName(), "detail2");
}

/**
 * @tc.name: TryEstablishPageLevelSplitHome014
 * @tc.desc: NAVIGATION reentry uses the new split instance as home rather than the earlier same-name page.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestElevenNg, TryEstablishPageLevelSplitHome014, TestSize.Level1)
{
    ASSERT_NO_FATAL_FAILURE(InitPageLevelSplitTest());
    auto first = CreateDestination("detail1");
    auto fullScreen = CreateDestination("detail2");
    auto second = CreateDestination("detail1");
    first->SetNavDestinationType(NavDestinationType::HOME);
    pattern_->forceSplitHomeDest_ = WeakPtr(first);
    pattern_->pageLevelSplitStackChanged_ = true;
    manager_->SetHomePageName("detail1");
    SetStack({ { "detail1", first }, { "detail2", fullScreen }, { "detail1", second } });
    pattern_->TryEstablishPageLevelSplitHome();
    EXPECT_EQ(pattern_->forceSplitHomeDest_.Upgrade(), second);
    EXPECT_EQ(first->GetNavDestinationType(), NavDestinationType::DETAIL);
    EXPECT_EQ(fullScreen->GetNavDestinationType(), NavDestinationType::DETAIL);
    EXPECT_EQ(second->GetNavDestinationType(), NavDestinationType::HOME);
    EXPECT_FALSE(pattern_->navBarIsHome_);
}

/**
 * @tc.name: TryEstablishPageLevelSplitHome015
 * @tc.desc: Rotation without a stack operation preserves destination and NavBar homes in both modes.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestElevenNg, TryEstablishPageLevelSplitHome015, TestSize.Level1)
{
    ASSERT_NO_FATAL_FAILURE(InitPageLevelSplitTest());
    auto home = CreateDestination("detail1");
    auto top = CreateDestination("detail1");
    SetStack({ { "detail1", home }, { "detail1", top } });
    for (auto mode : { ForceSplitBehaviorMode::NAVIGATION, ForceSplitBehaviorMode::DISPLACE }) {
        manager_->SetBehaviorMode(mode);
        pattern_->forceSplitSuccess_ = false;
        pattern_->pageLevelSplitStackChanged_ = false;
        pattern_->navBarIsHome_ = false;
        pattern_->forceSplitHomeDest_ = WeakPtr(home);
        home->SetNavDestinationType(NavDestinationType::HOME);
        pattern_->TryEstablishPageLevelSplitHome();
        EXPECT_EQ(pattern_->forceSplitHomeDest_.Upgrade(), home);
        EXPECT_EQ(top->GetNavDestinationType(), NavDestinationType::DETAIL);
        pattern_->navBarIsHome_ = true;
        pattern_->forceSplitHomeDest_ = nullptr;
        pattern_->TryEstablishPageLevelSplitHome();
        EXPECT_TRUE(pattern_->navBarIsHome_);
        EXPECT_EQ(pattern_->forceSplitHomeDest_.Upgrade(), nullptr);
    }
}

/**
 * @tc.name: ShouldKeepPageLevelSplitSession001
 * @tc.desc: Session preservation covers display, stack-change, mode and logical-session combinations.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestElevenNg, ShouldKeepPageLevelSplitSession001, TestSize.Level1)
{
    ASSERT_NO_FATAL_FAILURE(InitPageLevelSplitTest());
    auto home = CreateDestination("detail1");
    auto other = CreateDestination("detail1");
    struct SessionCase {
        bool navBarHome;
        bool destinationHome;
        bool hideNavBar;
        bool split;
        bool stackChanged;
        bool displace;
        bool active;
        bool expected;
    };
    const SessionCase cases[] = {
        { true, false, false, true, true, false, false, true },
        { true, false, false, false, false, false, false, true },
        { true, false, false, false, true, false, true, false },
        { true, false, false, false, true, true, true, true },
        { true, false, false, false, true, true, false, false },
        { true, false, true, true, false, true, true, false },
        { false, true, false, false, false, false, false, true },
        { false, true, false, false, true, true, true, true },
        { false, true, false, true, true, false, false, false },
        { false, true, false, false, true, false, true, false },
        { false, false, false, false, false, true, true, false },
    };
    for (size_t index = 0; index < std::size(cases); ++index) {
        SCOPED_TRACE(index);
        const auto& value = cases[index];
        pattern_->navBarIsHome_ = value.navBarHome;
        pattern_->forceSplitHomeDest_ = value.destinationHome ? WeakPtr(home) : WeakPtr<NavDestinationGroupNode>();
        pattern_->forceSplitSuccess_ = value.split;
        pattern_->pageLevelSplitStackChanged_ = value.stackChanged;
        pattern_->pageLevelDisplaceSessionActive_ = value.active;
        navigation_->GetLayoutProperty<NavigationLayoutProperty>()->UpdateHideNavBar(value.hideNavBar);
        EXPECT_EQ(pattern_->ShouldKeepPageLevelSplitSession({ home }, value.displace, navigation_->GetId()),
            value.expected);
    }
    pattern_->navBarIsHome_ = false;
    pattern_->forceSplitHomeDest_ = WeakPtr(other);
    pattern_->pageLevelSplitStackChanged_ = false;
    EXPECT_FALSE(pattern_->ShouldKeepPageLevelSplitSession({ home }, true, navigation_->GetId()));
}

/**
 * @tc.name: InitializePageLevelSplitHome001
 * @tc.desc: A filtered single destination with extra raw entries must not select the NavBar.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestElevenNg, InitializePageLevelSplitHome001, TestSize.Level1)
{
    ASSERT_NO_FATAL_FAILURE(InitPageLevelSplitTest());
    auto top = CreateDestination("detail1");
    SetStack({ { "missing", nullptr }, { "detail1", top } });
    pattern_->TryEstablishPageLevelSplitHome();
    EXPECT_FALSE(pattern_->navBarIsHome_);
    EXPECT_EQ(pattern_->forceSplitHomeDest_.Upgrade(), top);
    EXPECT_EQ(top->GetNavDestinationType(), NavDestinationType::HOME);
}

/**
 * @tc.name: ShouldKeepPageLevelSplitHome001
 * @tc.desc: Continuity requires the home instance and a split-active predecessor, or a valid DISPLACE NavBar.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestElevenNg, ShouldKeepPageLevelSplitHome001, TestSize.Level1)
{
    ASSERT_NO_FATAL_FAILURE(InitPageLevelSplitTest());
    auto home = CreateDestination("detail1");
    auto top = CreateDestination("detail1");
    auto fullScreen = CreateDestination("detail2");
    manager_->SetHomePageName("detail1");
    pattern_->forceSplitHomeDest_ = WeakPtr(home);
    EXPECT_TRUE(pattern_->ShouldKeepPageLevelSplitHome({ home, top }, manager_, false, navigation_->GetId()));
    EXPECT_FALSE(pattern_->ShouldKeepPageLevelSplitHome(
        { home, fullScreen, top }, manager_, false, navigation_->GetId()));
    EXPECT_FALSE(pattern_->ShouldKeepPageLevelSplitHome({ top }, manager_, false, navigation_->GetId()));
    pattern_->forceSplitHomeDest_ = nullptr;
    EXPECT_FALSE(pattern_->ShouldKeepPageLevelSplitHome({ home, top }, manager_, false, navigation_->GetId()));
    pattern_->navBarIsHome_ = true;
    EXPECT_TRUE(pattern_->ShouldKeepPageLevelSplitHome({ top }, manager_, true, navigation_->GetId()));
    navigation_->GetLayoutProperty<NavigationLayoutProperty>()->UpdateHideNavBar(true);
    EXPECT_FALSE(pattern_->ShouldKeepPageLevelSplitHome({ top }, manager_, true, navigation_->GetId()));
    manager_->SetHomePageName("");
    EXPECT_FALSE(pattern_->ShouldKeepPageLevelSplitHome({ top }, manager_, false, navigation_->GetId()));
}

/**
 * @tc.name: ReanchorPageLevelSplitHome001
 * @tc.desc: NAVIGATION restores the earliest consecutive split destination and demotes a popped same-name home.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestElevenNg, ReanchorPageLevelSplitHome001, TestSize.Level1)
{
    ASSERT_NO_FATAL_FAILURE(InitPageLevelSplitTest());
    auto fullScreen = CreateDestination("detail2");
    auto first = CreateDestination("detail1");
    auto second = CreateDestination("detail1");
    auto popped = CreateDestination("detail1");
    popped->SetNavDestinationType(NavDestinationType::HOME);
    pattern_->forceSplitHomeDest_ = WeakPtr(popped);
    pattern_->ReanchorPageLevelSplitHome({ fullScreen, first, second }, manager_, false);
    EXPECT_EQ(pattern_->forceSplitHomeDest_.Upgrade(), first);
    EXPECT_EQ(first->GetNavDestinationType(), NavDestinationType::HOME);
    EXPECT_EQ(second->GetNavDestinationType(), NavDestinationType::DETAIL);
    EXPECT_EQ(popped->GetNavDestinationType(), NavDestinationType::DETAIL);
}

/**
 * @tc.name: ReanchorPageLevelSplitHome002
 * @tc.desc: DISPLACE uses a valid NavBar for a sole destination and falls back to the destination when hidden.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestElevenNg, ReanchorPageLevelSplitHome002, TestSize.Level1)
{
    ASSERT_NO_FATAL_FAILURE(InitPageLevelSplitTest());
    manager_->SetBehaviorMode(ForceSplitBehaviorMode::DISPLACE);
    auto top = CreateDestination("detail1");
    top->SetNavDestinationType(NavDestinationType::HOME);
    pattern_->forceSplitHomeDest_ = WeakPtr(top);
    pattern_->ReanchorPageLevelSplitHome({ top }, manager_, true);
    EXPECT_TRUE(pattern_->navBarIsHome_);
    EXPECT_EQ(pattern_->forceSplitHomeDest_.Upgrade(), nullptr);
    EXPECT_EQ(top->GetNavDestinationType(), NavDestinationType::DETAIL);
    pattern_->ReanchorPageLevelSplitHome({ top }, manager_, true);
    EXPECT_TRUE(pattern_->navBarIsHome_);
    navigation_->GetLayoutProperty<NavigationLayoutProperty>()->UpdateHideNavBar(true);
    pattern_->ReanchorPageLevelSplitHome({ top }, manager_, true);
    EXPECT_FALSE(pattern_->navBarIsHome_);
    EXPECT_EQ(pattern_->forceSplitHomeDest_.Upgrade(), top);
}

/**
 * @tc.name: PageLevelNavigationSelector001
 * @tc.desc: Excluded Navigation must neither establish a home nor overwrite the target policy stack.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestElevenNg, PageLevelNavigationSelector001, TestSize.Level1)
{
    ASSERT_NO_FATAL_FAILURE(InitPageLevelSplitTest());
    auto navManager = MockPipelineContext::GetCurrent()->GetNavigationManager();
    ASSERT_NE(navManager, nullptr);
    navManager->SetForceSplitNavigationId("target");
    auto top = CreateDestination("detail1");
    SetStack({ { "detail1", top } });
    manager_->SetPageStack({ "targetHome", "targetDetail" });
    pattern_->SetIsTargetForceSplitNav(false);
    pattern_->TryEstablishPageLevelSplitHome();
    EXPECT_TRUE(pattern_->IsTopNavDestinationSplitDisabled());
    pattern_->NotifyForceFullScreenChangeIfNeeded({ "detail1" });
    EXPECT_EQ(manager_->pageStack_, (std::vector<std::string> { "targetHome", "targetDetail" }));
    EXPECT_EQ(pattern_->forceSplitHomeDest_.Upgrade(), nullptr);
    EXPECT_FALSE(pattern_->navBarIsHome_);
}

/**
 * @tc.name: PageLevelNavigationSelector002
 * @tc.desc: Target Navigation refreshes its own stack before resolving inherited policies.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestElevenNg, PageLevelNavigationSelector002, TestSize.Level1)
{
    ASSERT_NO_FATAL_FAILURE(InitPageLevelSplitTest());
    auto navManager = MockPipelineContext::GetCurrent()->GetNavigationManager();
    ASSERT_NE(navManager, nullptr);
    navManager->SetForceSplitNavigationDepth(1);
    manager_->SetEnableInheritSplitStrategy(true);
    auto home = CreateDestination("detail1");
    auto top = CreateDestination("inherited");
    SetStack({ { "detail1", home }, { "inherited", top } });
    manager_->SetPageStack({ "otherNavigation" });
    pattern_->TryEstablishPageLevelSplitHome();
    EXPECT_EQ(manager_->pageStack_, (std::vector<std::string> { "detail1", "inherited" }));
    EXPECT_EQ(pattern_->forceSplitHomeDest_.Upgrade(), top);
    EXPECT_FALSE(pattern_->IsTopNavDestinationSplitDisabled());
}

/**
 * @tc.name: UpdatePageLevelDisplaceColumns001
 * @tc.desc: Inactive sessions and incomplete stack operations preserve column ownership.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestElevenNg, UpdatePageLevelDisplaceColumns001, TestSize.Level1)
{
    ASSERT_NO_FATAL_FAILURE(InitPageLevelSplitTest());
    auto first = CreateDestination("detail1");
    auto second = CreateDestination("detail1");
    SetStack({ { "detail1", first }, { "detail1", second } });
    pattern_->UpdatePageLevelDisplaceColumns({ { "detail1", first } }, { { "detail1", second } });
    EXPECT_EQ(first->GetColumnType(), ForceSplitPageColumnType::SECONDARY);
    pattern_->pageLevelDisplaceSessionActive_ = true;
    pattern_->UpdatePageLevelDisplaceColumns(std::nullopt, { { "detail1", second } });
    pattern_->UpdatePageLevelDisplaceColumns({ { "detail1", first } }, std::nullopt);
    pattern_->UpdatePageLevelDisplaceColumns({ { "detail1", first } }, { { "detail1", first } });
    EXPECT_EQ(first->GetColumnType(), ForceSplitPageColumnType::SECONDARY);
    EXPECT_EQ(second->GetColumnType(), ForceSplitPageColumnType::SECONDARY);
}

/**
 * @tc.name: UpdatePageLevelDisplaceColumns002
 * @tc.desc: Removing the destination home ends the logical DISPLACE session.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestElevenNg, UpdatePageLevelDisplaceColumns002, TestSize.Level1)
{
    ASSERT_NO_FATAL_FAILURE(InitPageLevelSplitTest());
    auto home = CreateDestination("detail1");
    auto first = CreateDestination("detail1");
    auto second = CreateDestination("detail1");
    pattern_->pageLevelDisplaceSessionActive_ = true;
    pattern_->forceSplitHomeDest_ = WeakPtr(home);
    SetStack({ { "detail1", first }, { "detail1", second } });
    pattern_->UpdatePageLevelDisplaceColumns({ { "detail1", first } }, { { "detail1", second } });
    EXPECT_FALSE(pattern_->pageLevelDisplaceSessionActive_);
    EXPECT_EQ(first->GetColumnType(), ForceSplitPageColumnType::SECONDARY);
    pattern_->forceSplitHomeDest_ = nullptr;
    pattern_->pageLevelDisplaceSessionActive_ = true;
    pattern_->UpdatePageLevelDisplaceColumns({ { "detail1", first } }, { { "detail1", second } });
    EXPECT_FALSE(pattern_->pageLevelDisplaceSessionActive_);
}

/**
 * @tc.name: UpdatePageLevelDisplaceColumns003
 * @tc.desc: A new same-name instance is a push and displaces the previous secondary page.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestElevenNg, UpdatePageLevelDisplaceColumns003, TestSize.Level1)
{
    ASSERT_NO_FATAL_FAILURE(InitPageLevelSplitTest());
    manager_->SetBehaviorMode(ForceSplitBehaviorMode::DISPLACE);
    pattern_->pageLevelDisplaceSessionActive_ = true;
    pattern_->navBarIsHome_ = true;
    auto first = CreateDestination("detail1");
    auto second = CreateDestination("detail1");
    SyncPortraitColumns({ { "detail1", first } }, { { "detail1", first }, { "detail1", second } });
    EXPECT_EQ(first->GetColumnType(), ForceSplitPageColumnType::PRIMARY);
    EXPECT_EQ(second->GetColumnType(), ForceSplitPageColumnType::SECONDARY);
    SyncPortraitColumns({ { "detail1", first }, { "detail1", second } }, { { "detail1", first } });
    EXPECT_EQ(first->GetColumnType(), ForceSplitPageColumnType::SECONDARY);
}

/**
 * @tc.name: UpdatePageLevelDisplaceColumns004
 * @tc.desc: A replace operation does not move the replaced secondary page to primary.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestElevenNg, UpdatePageLevelDisplaceColumns004, TestSize.Level1)
{
    ASSERT_NO_FATAL_FAILURE(InitPageLevelSplitTest());
    manager_->SetBehaviorMode(ForceSplitBehaviorMode::DISPLACE);
    pattern_->pageLevelDisplaceSessionActive_ = true;
    pattern_->navBarIsHome_ = true;
    stack_->replaceValue_ = 1;
    auto first = CreateDestination("detail1");
    auto second = CreateDestination("detail1");
    SyncPortraitColumns({ { "detail1", first } }, { { "detail1", first }, { "detail1", second } });
    EXPECT_EQ(first->GetColumnType(), ForceSplitPageColumnType::SECONDARY);
    EXPECT_EQ(second->GetColumnType(), ForceSplitPageColumnType::SECONDARY);
}

/**
 * @tc.name: UpdatePageLevelDisplaceColumns005
 * @tc.desc: Transparent and dialog pages keep the previous page in the secondary column.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestElevenNg, UpdatePageLevelDisplaceColumns005, TestSize.Level1)
{
    ASSERT_NO_FATAL_FAILURE(InitPageLevelSplitTest());
    manager_->SetBehaviorMode(ForceSplitBehaviorMode::DISPLACE);
    manager_->SetTransPages({ "transparent" });
    pattern_->pageLevelDisplaceSessionActive_ = true;
    pattern_->navBarIsHome_ = true;
    auto first = CreateDestination("detail1");
    auto transparent = CreateDestination("transparent");
    auto second = CreateDestination("detail1");
    SyncPortraitColumns({ { "detail1", first } }, { { "detail1", first }, { "transparent", transparent } });
    EXPECT_EQ(first->GetColumnType(), ForceSplitPageColumnType::SECONDARY);
    SyncPortraitColumns({ { "detail1", first }, { "transparent", transparent } },
        { { "detail1", first }, { "transparent", transparent }, { "detail1", second } });
    EXPECT_EQ(transparent->GetColumnType(), ForceSplitPageColumnType::SECONDARY);
    auto dialog = CreateDestination("dialog");
    dialog->SetNavDestinationMode(NavDestinationMode::DIALOG);
    SyncPortraitColumns({ { "detail1", second } }, { { "detail1", second }, { "dialog", dialog } });
    EXPECT_EQ(second->GetColumnType(), ForceSplitPageColumnType::SECONDARY);
}

/**
 * @tc.name: UpdatePageLevelDisplaceColumns006
 * @tc.desc: A destination home is preserved while later secondary destinations are displaced.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestElevenNg, UpdatePageLevelDisplaceColumns006, TestSize.Level1)
{
    ASSERT_NO_FATAL_FAILURE(InitPageLevelSplitTest());
    manager_->SetBehaviorMode(ForceSplitBehaviorMode::DISPLACE);
    pattern_->pageLevelDisplaceSessionActive_ = true;
    auto home = CreateDestination("detail1");
    auto first = CreateDestination("detail1");
    auto second = CreateDestination("detail1");
    home->SetNavDestinationType(NavDestinationType::HOME);
    home->SetColumnType(ForceSplitPageColumnType::PRIMARY);
    pattern_->forceSplitHomeDest_ = WeakPtr(home);
    SyncPortraitColumns({ { "detail1", home }, { "detail1", first } },
        { { "detail1", home }, { "detail1", first }, { "detail1", second } });
    EXPECT_EQ(first->GetColumnType(), ForceSplitPageColumnType::PRIMARY);
    EXPECT_EQ(home->GetColumnType(), ForceSplitPageColumnType::PRIMARY);
    SyncPortraitColumns({ { "detail1", home }, { "detail1", first }, { "detail1", second } },
        { { "detail1", home } });
    EXPECT_EQ(home->GetColumnType(), ForceSplitPageColumnType::PRIMARY);
    EXPECT_EQ(home->GetNavDestinationType(), NavDestinationType::HOME);
}

/**
 * @tc.name: AdjustNodeForStackSyncPageLevel001
 * @tc.desc: A non-split top page ends DISPLACE continuity, while feature-off preserves the legacy session state.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestElevenNg, AdjustNodeForStackSyncPageLevel001, TestSize.Level1)
{
    ASSERT_NO_FATAL_FAILURE(InitPageLevelSplitTest());
    manager_->SetBehaviorMode(ForceSplitBehaviorMode::DISPLACE);
    auto first = CreateDestination("detail1");
    auto disabled = CreateDestination("detail2");
    SetStack({ { "detail1", first }, { "detail2", disabled } });
    pattern_->pageLevelDisplaceSessionActive_ = true;
    pattern_->AdjustNodeForStackSync({ { "detail1", first } }, { { "detail2", disabled } });
    EXPECT_FALSE(pattern_->pageLevelDisplaceSessionActive_);
    manager_->SetEnablePageLevelSplit(false);
    pattern_->pageLevelDisplaceSessionActive_ = true;
    pattern_->AdjustNodeForStackSync({ { "detail1", first } }, { { "detail2", disabled } });
    EXPECT_TRUE(pattern_->pageLevelDisplaceSessionActive_);
}

/**
 * @tc.name: TryForceSplitIfNeededPageLevel001
 * @tc.desc: An unchanged false result must clear a stale pipeline split flag (f2bffa regression).
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestElevenNg, TryForceSplitIfNeededPageLevel001, TestSize.Level1)
{
    ASSERT_NO_FATAL_FAILURE(InitPageLevelSplitTest());
    manager_->isRouter_ = false;
    manager_->SetForceSplitSupported(true);
    manager_->isForceSplitEnable_ = false;
    auto context = MockPipelineContext::GetCurrent();
    context->SetIsCurrentInForceSplitMode(true);
    pattern_->TryForceSplitIfNeeded();
    EXPECT_FALSE(context->IsCurrentInForceSplitMode());
    EXPECT_FALSE(pattern_->forceSplitSuccess_);
}

/**
 * @tc.name: InitializePageLevelSplitHome002
 * @tc.desc: Keeping the same destination home does not demote it during initialization.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestElevenNg, InitializePageLevelSplitHome002, TestSize.Level1)
{
    ASSERT_NO_FATAL_FAILURE(InitPageLevelSplitTest());
    auto top = CreateDestination("detail1");
    top->SetNavDestinationType(NavDestinationType::HOME);
    pattern_->forceSplitHomeDest_ = WeakPtr(top);
    navigation_->GetLayoutProperty<NavigationLayoutProperty>()->UpdateHideNavBar(true);
    SetStack({ { "detail1", top } });
    pattern_->InitializePageLevelSplitHome({ top }, manager_, false, navigation_->GetId());
    EXPECT_EQ(pattern_->forceSplitHomeDest_.Upgrade(), top);
    EXPECT_EQ(top->GetNavDestinationType(), NavDestinationType::HOME);
}

/**
 * @tc.name: ReanchorPageLevelSplitHome003
 * @tc.desc: NAVIGATION without a policy boundary selects the bottom split page and tolerates missing patterns.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestElevenNg, ReanchorPageLevelSplitHome003, TestSize.Level1)
{
    ASSERT_NO_FATAL_FAILURE(InitPageLevelSplitTest());
    auto first = CreateDestination("detail1");
    auto top = CreateDestination("detail1");
    pattern_->ReanchorPageLevelSplitHome({ first, top }, manager_, false);
    EXPECT_EQ(pattern_->forceSplitHomeDest_.Upgrade(), first);
    pattern_->ReanchorPageLevelSplitHome({ nullptr, top }, manager_, false);
    EXPECT_EQ(pattern_->forceSplitHomeDest_.Upgrade(), top);
    pattern_->ReanchorPageLevelSplitHome({ nullptr }, manager_, false);
    EXPECT_EQ(pattern_->forceSplitHomeDest_.Upgrade(), top);
}

/**
 * @tc.name: UpdatePageLevelDisplaceColumns007
 * @tc.desc: Invalid nodes and a missing stack cannot mutate a logical session.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestElevenNg, UpdatePageLevelDisplaceColumns007, TestSize.Level1)
{
    ASSERT_NO_FATAL_FAILURE(InitPageLevelSplitTest());
    pattern_->pageLevelDisplaceSessionActive_ = true;
    pattern_->navBarIsHome_ = true;
    auto first = CreateDestination("detail1");
    auto second = CreateDestination("detail1");
    SetStack({ { "detail1", first }, { "detail1", second } });
    pattern_->UpdatePageLevelDisplaceColumns({ { "invalid", nullptr } }, { { "detail1", second } });
    pattern_->UpdatePageLevelDisplaceColumns({ { "detail1", first } }, { { "invalid", nullptr } });
    pattern_->navigationStack_ = nullptr;
    pattern_->UpdatePageLevelDisplaceColumns({ { "detail1", first } }, { { "detail1", second } });
    pattern_->navigationStack_ = stack_;
    EXPECT_TRUE(pattern_->pageLevelDisplaceSessionActive_);
    EXPECT_EQ(first->GetColumnType(), ForceSplitPageColumnType::SECONDARY);
}

/**
 * @tc.name: UpdatePageLevelDisplaceColumns008
 * @tc.desc: A previous primary or home destination must not be displaced again.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestElevenNg, UpdatePageLevelDisplaceColumns008, TestSize.Level1)
{
    ASSERT_NO_FATAL_FAILURE(InitPageLevelSplitTest());
    manager_->SetBehaviorMode(ForceSplitBehaviorMode::DISPLACE);
    pattern_->pageLevelDisplaceSessionActive_ = true;
    pattern_->navBarIsHome_ = true;
    auto first = CreateDestination("detail1");
    auto second = CreateDestination("detail1");
    first->SetColumnType(ForceSplitPageColumnType::PRIMARY);
    SyncPortraitColumns({ { "detail1", first } }, { { "detail1", first }, { "detail1", second } });
    EXPECT_EQ(first->GetColumnType(), ForceSplitPageColumnType::PRIMARY);
    first->SetColumnType(ForceSplitPageColumnType::SECONDARY);
    first->SetNavDestinationType(NavDestinationType::HOME);
    SyncPortraitColumns({ { "detail1", first } }, { { "detail1", first }, { "detail1", second } });
    EXPECT_EQ(first->GetColumnType(), ForceSplitPageColumnType::SECONDARY);
    EXPECT_EQ(first->GetNavDestinationType(), NavDestinationType::HOME);
}

/**
 * @tc.name: UpdatePageLevelDisplaceColumns009
 * @tc.desc: An already present destination is not treated as a new push.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestElevenNg, UpdatePageLevelDisplaceColumns009, TestSize.Level1)
{
    ASSERT_NO_FATAL_FAILURE(InitPageLevelSplitTest());
    manager_->SetBehaviorMode(ForceSplitBehaviorMode::DISPLACE);
    pattern_->pageLevelDisplaceSessionActive_ = true;
    pattern_->navBarIsHome_ = true;
    auto first = CreateDestination("detail1");
    auto second = CreateDestination("detail1");
    SyncPortraitColumns({ { "detail1", second }, { "detail1", first } },
        { { "detail1", first }, { "detail1", second } });
    EXPECT_EQ(first->GetColumnType(), ForceSplitPageColumnType::SECONDARY);
    EXPECT_EQ(second->GetColumnType(), ForceSplitPageColumnType::SECONDARY);
}

/**
 * @tc.name: TryForceSplitIfNeededPageLevel002
 * @tc.desc: A changed home instance rebuilds columns even when both cached split booleans are unchanged.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestElevenNg, TryForceSplitIfNeededPageLevel002, TestSize.Level1)
{
    ASSERT_NO_FATAL_FAILURE(InitPageLevelSplitTest());
    auto container = AceType::DynamicCast<MockContainer>(Container::Current());
    ASSERT_NE(container, nullptr);
    EXPECT_CALL(*container, IsMainWindow).WillRepeatedly(Return(true));
    manager_->SetBehaviorMode(ForceSplitBehaviorMode::DISPLACE);
    manager_->isForceSplitEnable_ = true;
    auto page = FrameNode::CreateFrameNode(V2::PAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<Pattern>());
    pattern_->pageNode_ = WeakPtr(page);
    NavigationModelNG model;
    ASSERT_TRUE(model.CreatePrimaryContentIfNeeded(navigation_));
    auto content = AceType::DynamicCast<FrameNode>(navigation_->GetContentNode());
    ASSERT_NE(content, nullptr);
    auto poppedHome = CreateDestination("detail1");
    auto newHome = CreateDestination("detail1");
    auto detail = CreateDestination("detail1");
    SetStack({ { "detail1", newHome }, { "detail1", detail } });
    newHome->MountToParent(content);
    detail->MountToParent(content);
    pattern_->forceSplitHomeDest_ = WeakPtr(poppedHome);
    pattern_->primaryNodes_ = { poppedHome };
    pattern_->forceSplitSuccess_ = true;
    pattern_->forceSplitUseNavBar_ = false;
    pattern_->pageLevelSplitStackChanged_ = true;
    manager_->SetHomePageName("detail1");
    pattern_->TryForceSplitIfNeeded();
    EXPECT_TRUE(pattern_->forceSplitSuccess_);
    EXPECT_FALSE(pattern_->forceSplitUseNavBar_);
    EXPECT_EQ(pattern_->forceSplitHomeDest_.Upgrade(), newHome);
    ASSERT_EQ(pattern_->primaryNodes_.size(), 1);
    EXPECT_EQ(pattern_->primaryNodes_.back().Upgrade(), newHome);
    EXPECT_EQ(newHome->GetParent(), navigation_->GetPrimaryContentNode());
    EXPECT_EQ(detail->GetParent(), content);
    EXPECT_TRUE(pattern_->pageLevelDisplaceSessionActive_);
    EXPECT_FALSE(pattern_->pageLevelSplitStackChanged_);
}

/**
 * @tc.name: TryEstablishPageLevelSplitHome016
 * @tc.desc: A disabled feature and missing navigation stack leave the previous home unchanged.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestElevenNg, TryEstablishPageLevelSplitHome016, TestSize.Level1)
{
    ASSERT_NO_FATAL_FAILURE(InitPageLevelSplitTest());
    auto home = CreateDestination("detail1");
    pattern_->forceSplitHomeDest_ = WeakPtr(home);
    manager_->SetEnablePageLevelSplit(false);
    pattern_->TryEstablishPageLevelSplitHome();
    EXPECT_EQ(pattern_->forceSplitHomeDest_.Upgrade(), home);
    manager_->SetEnablePageLevelSplit(true);
    pattern_->navigationStack_ = nullptr;
    pattern_->TryEstablishPageLevelSplitHome();
    EXPECT_EQ(pattern_->forceSplitHomeDest_.Upgrade(), home);
    pattern_->navigationStack_ = stack_;
}

/**
 * @tc.name: CreateAnimationWithPushPageLevel001
 * @tc.desc: Split-home animations skip the outgoing NavBar and tolerate a missing pre-node.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestElevenNg, CreateAnimationWithPushPageLevel001, TestSize.Level1)
{
    ASSERT_NO_FATAL_FAILURE(InitPageLevelSplitTest());
    auto navBar = AceType::MakeRefPtr<PageLevelSplitNavBarNode>(
        V2::NAVBAR_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<NavBarPattern>());
    navBar->InitializePatternAndContext();
    auto top = CreateDestination("detail1");
    pattern_->forceSplitSuccess_ = true;
    int32_t finished = 0;
    navigation_->CreateAnimationWithPush(
        TransitionUnitInfo(navBar, false, 1), TransitionUnitInfo(top, false, 1), [&finished] { ++finished; }, true);
    EXPECT_EQ(navBar->pushEndCount_, 0);
    EXPECT_EQ(finished, 1);
    navigation_->CreateAnimationWithPush(
        TransitionUnitInfo(nullptr, false, 2), TransitionUnitInfo(top, false, 2), [&finished] { ++finished; }, true);
    EXPECT_EQ(finished, 2);
    pattern_->forceSplitSuccess_ = false;
    navigation_->CreateAnimationWithPush(
        TransitionUnitInfo(navBar, false, 3), TransitionUnitInfo(top, false, 3), [] {}, true);
    EXPECT_EQ(navBar->pushEndCount_, 1);
}

/**
 * @tc.name: TransitionWithPushPageLevel001
 * @tc.desc: Finishing a split NavBar push finalizes the NavBar and preserves its visibility.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestElevenNg, TransitionWithPushPageLevel001, TestSize.Level1)
{
    ASSERT_NO_FATAL_FAILURE(InitPageLevelSplitTest());
    auto navBar = AceType::MakeRefPtr<PageLevelSplitNavBarNode>(
        V2::NAVBAR_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<NavBarPattern>());
    navBar->InitializePatternAndContext();
    auto top = CreateDestination("detail1");
    pattern_->forceSplitSuccess_ = true;
    navBar->GetLayoutProperty()->UpdateVisibility(VisibleType::VISIBLE);
    navigation_->TransitionWithPush(navBar, top, true);
    EXPECT_EQ(navBar->pushFinishCount_, 1);
    EXPECT_EQ(navBar->GetLayoutProperty()->GetVisibilityValue(VisibleType::VISIBLE), VisibleType::VISIBLE);
    EXPECT_TRUE(navigation_->hideNodes_.empty());
}


/**
 * @tc.name: CreateAnimationWithPushPageLevel002
 * @tc.desc: Outgoing normal pages animate, while custom-transition and overlay pages skip that animation.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestElevenNg, CreateAnimationWithPushPageLevel002, TestSize.Level1)
{
    ASSERT_NO_FATAL_FAILURE(InitPageLevelSplitTest());
    auto navBar = AceType::MakeRefPtr<PageLevelSplitNavBarNode>(
        V2::NAVBAR_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<NavBarPattern>());
    navBar->InitializePatternAndContext();
    auto top = CreateDestination("detail1");
    pattern_->forceSplitSuccess_ = true;
    navigation_->CreateAnimationWithPush(
        TransitionUnitInfo(navBar, false, 1), TransitionUnitInfo(top, false, 1), [] {}, false);
    EXPECT_EQ(navBar->pushEndCount_, 1);
    navigation_->CreateAnimationWithPush(
        TransitionUnitInfo(navBar, true, 2), TransitionUnitInfo(top, false, 2), [] {}, false);
    EXPECT_EQ(navBar->pushEndCount_, 1);
    top->SetIsFullScreenOverlay(true);
    navigation_->CreateAnimationWithPush(
        TransitionUnitInfo(navBar, false, 3), TransitionUnitInfo(top, false, 3), [] {}, false);
    EXPECT_EQ(navBar->pushEndCount_, 1);
}

/**
 * @tc.name: TransitionWithPushPageLevel002
 * @tc.desc: Overlay pushes leave the covered split NavBar transition untouched.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestElevenNg, TransitionWithPushPageLevel002, TestSize.Level1)
{
    ASSERT_NO_FATAL_FAILURE(InitPageLevelSplitTest());
    auto navBar = AceType::MakeRefPtr<PageLevelSplitNavBarNode>(
        V2::NAVBAR_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<NavBarPattern>());
    navBar->InitializePatternAndContext();
    auto top = CreateDestination("detail1");
    top->SetIsFullScreenOverlay(true);
    pattern_->forceSplitSuccess_ = true;
    navigation_->TransitionWithPush(navBar, top, true);
    EXPECT_EQ(navBar->pushEndCount_, 0);
    EXPECT_EQ(navBar->pushFinishCount_, 0);
    EXPECT_TRUE(navigation_->hideNodes_.empty());
}

} // namespace OHOS::Ace::NG

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

#include "navigation_test_min.h"
#include "mock_navigation_route.h"

#define private public
#define protected public

#include "test/mock/frameworks/core/common/mock_container.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"
#include "test/mock/interfaces/inner_api/ui_session/mock_ui_session_manager.h"

#include "core/components_ng/manager/content_change_manager/content_change_manager.h"
#include "core/components_ng/pattern/navigation/navigation_pattern.h"
#include "core/components_ng/pattern/navrouter/navdestination_group_node.h"
#include "core/components_ng/pattern/navrouter/navdestination_pattern.h"

#undef private
#undef protected

using namespace testing;
using namespace testing::ext;


namespace OHOS::Ace::NG {

// Test classes for TDD development (quick compile mode)
// These classes use different names from full test suite to avoid conflicts
// When ace_enable_full_test_suite=true, full test files will be compiled instead

class NavigationModelNgTestCore : public NavigationTestBase {};
class NavigationPatternTestCore : public NavigationTestBase {
public:
    static void SetUpTestCase()
    {
        MockPipelineContext::SetUp();
        MockContainer::SetUp();
    }

    static void TearDownTestCase()
    {
        MockPipelineContext::TearDown();
        MockContainer::TearDown();
    }
};

// Placeholder test to ensure compilation
HWTEST_F(NavigationModelNgTestCore, NavigationTestPlaceholder, TestSize.Level1)
{
    EXPECT_TRUE(true);
}

/**
 * @tc.name: ContentChangeNavigationStartTest001
 * @tc.desc: Test Navigation reports PAGE START and normal END with the same top destination identity.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestCore, ContentChangeNavigationStartTest001, TestSize.Level1)
{
    auto pipeline = MockPipelineContext::GetCurrent();
    ASSERT_NE(pipeline, nullptr);
    auto manager = pipeline->GetContentChangeManager();
    ASSERT_NE(manager, nullptr);
    ContentChangeConfig config;
    config.reportStartEvent = true;
    manager->StartContentChangeReport(config);

    auto navigationPattern = AceType::MakeRefPtr<NavigationPattern>();
    auto navigationNode =
        FrameNode::CreateFrameNode(V2::NAVIGATION_VIEW_ETS_TAG, 411, navigationPattern, true);
    auto topDestination = FrameNode::CreateFrameNode(
        V2::NAVDESTINATION_VIEW_ETS_TAG, 412, AceType::MakeRefPtr<Pattern>(), true);
    ASSERT_NE(navigationNode, nullptr);
    ASSERT_NE(topDestination, nullptr);
    auto mockUiSessionManager = reinterpret_cast<MockUiSessionManager*>(UiSessionManager::GetInstance());
    ASSERT_NE(mockUiSessionManager, nullptr);

    EXPECT_CALL(*mockUiSessionManager,
        ReportContentChangeEvent(ChangeType::PAGE_START, HasSubstr("\"$ID\":412"))).Times(1);
    navigationPattern->ContentChangeOnTransitionStart(topDestination);
    Mock::VerifyAndClearExpectations(mockUiSessionManager);

    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(ChangeType::PAGE, _)).Times(1);
    navigationPattern->ContentChangeReport(topDestination);
    EXPECT_TRUE(manager->activeContentChanges_.empty());
    Mock::VerifyAndClearExpectations(mockUiSessionManager);
    manager->StopContentChangeReport();
}

/**
 * @tc.name: ContentChangeNavigationPopIdentityTest001
 * @tc.desc: Pop uses Navigation identity, retains the destination dump, and permits another START after END.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestCore, ContentChangeNavigationPopIdentityTest001, TestSize.Level1)
{
    auto manager = MockPipelineContext::GetCurrent()->GetContentChangeManager();
    ContentChangeConfig config;
    config.reportStartEvent = true;
    manager->StartContentChangeReport(config);
    auto navigation = NavigationGroupNode::GetOrCreateGroupNode(
        V2::NAVIGATION_VIEW_ETS_TAG, 421, []() { return AceType::MakeRefPtr<NavigationPattern>(); });
    auto pattern = navigation->GetPattern<NavigationPattern>();
    pattern->SetNavigationStack(AceType::MakeRefPtr<NavigationStack>());
    auto target = FrameNode::CreateFrameNode(
        V2::NAVDESTINATION_VIEW_ETS_TAG, 422, AceType::MakeRefPtr<Pattern>(), true);
    auto mock = reinterpret_cast<MockUiSessionManager*>(UiSessionManager::GetInstance());
    for (int32_t i = 0; i < 2; ++i) {
        EXPECT_CALL(*mock, ReportContentChangeEvent(ChangeType::PAGE_START, HasSubstr("\"$ID\":421"))).Times(1);
        EXPECT_CALL(*mock, ReportContentChangeEvent(ChangeType::PAGE,
            AllOf(HasSubstr("\"$ID\":422"), Not(HasSubstr("\"$ID\":421"))))).Times(1);
        pattern->ContentChangeOnTransitionStart(target, true);
        EXPECT_EQ(manager->transitioningNodes_.count(navigation->GetId()), 1);
        EXPECT_EQ(manager->transitioningNodes_.count(target->GetId()), 0);
        EXPECT_EQ(manager->activeContentChanges_.count(navigation->GetId()), 1);
        EXPECT_EQ(manager->activeContentChanges_.count(target->GetId()), 0);
        navigation->ContentChangeReport(target, true);
        EXPECT_FALSE(manager->IsTransitioning());
        EXPECT_TRUE(manager->activeContentChanges_.empty());
        Mock::VerifyAndClearExpectations(mock);
    }
    manager->StopContentChangeReport();
}

/**
 * @tc.name: ContentChangeNavigationPopFallbackTest001
 * @tc.desc: Empty-target pop uses Navigation identity while END follows the configured dump fallback.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestCore, ContentChangeNavigationPopFallbackTest001, TestSize.Level1)
{
    auto manager = MockPipelineContext::GetCurrent()->GetContentChangeManager();
    ContentChangeConfig config;
    config.reportStartEvent = true;
    manager->StartContentChangeReport(config);
    auto navigation = NavigationGroupNode::GetOrCreateGroupNode(
        V2::NAVIGATION_VIEW_ETS_TAG, 423, []() { return AceType::MakeRefPtr<NavigationPattern>(); });
    auto pattern = navigation->GetPattern<NavigationPattern>();
    pattern->SetNavigationStack(AceType::MakeRefPtr<NavigationStack>());
    auto navBar = FrameNode::CreateFrameNode(V2::NAVBAR_ETS_TAG, 424, AceType::MakeRefPtr<Pattern>(), true);
    auto home = FrameNode::CreateFrameNode(
        V2::NAVDESTINATION_VIEW_ETS_TAG, 425, AceType::MakeRefPtr<Pattern>(), true);
    navigation->SetNavBarNode(navBar);
    navigation->customHomeDestination_ = home;
    auto mock = reinterpret_cast<MockUiSessionManager*>(UiSessionManager::GetInstance());
    for (int32_t i = 0; i < 3; ++i) {
        navigation->SetUseHomeDestination(i != 0);
        if (i == 2) {
            // With Home selected, do not switch to the unused NavBar when Home is absent.
            navigation->customHomeDestination_.Reset();
        }
        RefPtr<FrameNode> dumpNode = i == 0 ? navBar : (i == 1 ? home : AceType::DynamicCast<FrameNode>(navigation));
        auto tree = JsonUtil::CreateSharedPtrJson(true);
        dumpNode->DumpSimplifyTreeWithParamConfig(0, tree, false, { false, false, false });
        EXPECT_CALL(*mock, ReportContentChangeEvent(ChangeType::PAGE_START,
            HasSubstr("\"$ID\":" + std::to_string(navigation->GetId())))).Times(1);
        EXPECT_CALL(*mock, ReportContentChangeEvent(ChangeType::PAGE, tree->ToString())).Times(1);
        pattern->ContentChangeOnTransitionStart(nullptr, true);
        EXPECT_EQ(manager->activeContentChanges_.count(navigation->GetId()), 1);
        pattern->ContentChangeReport(nullptr, true);
        EXPECT_FALSE(manager->IsTransitioning());
        EXPECT_TRUE(manager->activeContentChanges_.empty());
        Mock::VerifyAndClearExpectations(mock);
    }
    manager->StopContentChangeReport();
}

/**
 * @tc.name: ContentChangeNavigationPopFallbackTest002
 * @tc.desc: When no fallback node exists at START, the non-animated fallback clears Navigation identity.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestCore, ContentChangeNavigationPopFallbackTest002, TestSize.Level1)
{
    auto manager = MockPipelineContext::GetCurrent()->GetContentChangeManager();
    ContentChangeConfig config;
    config.reportStartEvent = true;
    manager->StartContentChangeReport(config);
    auto navigation = NavigationGroupNode::GetOrCreateGroupNode(
        V2::NAVIGATION_VIEW_ETS_TAG, 426, []() { return AceType::MakeRefPtr<NavigationPattern>(); });
    auto pattern = navigation->GetPattern<NavigationPattern>();
    pattern->SetNavigationStack(AceType::MakeRefPtr<NavigationStack>());
    auto navBar = FrameNode::CreateFrameNode(V2::NAVBAR_ETS_TAG, 427, AceType::MakeRefPtr<Pattern>(), true);
    navigation->SetNavBarNode(navBar);
    navigation->SetUseHomeDestination(false);
    auto mock = reinterpret_cast<MockUiSessionManager*>(UiSessionManager::GetInstance());
    EXPECT_CALL(*mock, ReportContentChangeEvent(ChangeType::PAGE_START, HasSubstr("\"$ID\":426"))).Times(1);
    EXPECT_CALL(*mock, ReportContentChangeEvent(ChangeType::PAGE, HasSubstr("\"$ID\":426"))).Times(1);
    navigation->SetNavBarNode(nullptr);
    pattern->ContentChangeOnTransitionStart(nullptr, true);
    pattern->StartDefaultAnimation(nullptr, nullptr, true, false);
    EXPECT_FALSE(manager->IsTransitioning());
    EXPECT_TRUE(manager->activeContentChanges_.empty());
    Mock::VerifyAndClearExpectations(mock);
    manager->StopContentChangeReport();
}

/**
 * @tc.name: ContentChangeNavigationPopDestinationFinishTest001
 * @tc.desc: Single-node pop clears Navigation identity; a subsequent push still uses destination identity.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestCore, ContentChangeNavigationPopDestinationFinishTest001, TestSize.Level1)
{
    auto manager = MockPipelineContext::GetCurrent()->GetContentChangeManager();
    ContentChangeConfig config;
    config.reportStartEvent = true;
    manager->StartContentChangeReport(config);
    auto navigation = NavigationGroupNode::GetOrCreateGroupNode(
        V2::NAVIGATION_VIEW_ETS_TAG, 428, []() { return AceType::MakeRefPtr<NavigationPattern>(); });
    auto pattern = navigation->GetPattern<NavigationPattern>();
    pattern->SetNavigationStack(AceType::MakeRefPtr<NavigationStack>());
    auto destination = NavDestinationGroupNode::GetOrCreateGroupNode(
        V2::NAVDESTINATION_VIEW_ETS_TAG, 429, []() { return AceType::MakeRefPtr<NavDestinationPattern>(); });
    destination->GetPattern<NavDestinationPattern>()->SetNavigationNode(navigation);
    destination->SetInCurrentStack(true);
    auto mock = reinterpret_cast<MockUiSessionManager*>(UiSessionManager::GetInstance());
    destination->SetSystemTransitionType(NavigationSystemTransitionType::FADE);
    for (auto operation : { NavigationOperation::POP, NavigationOperation::PUSH }) {
        const bool isPopPage = operation == NavigationOperation::POP;
        EXPECT_CALL(*mock, ReportContentChangeEvent(ChangeType::PAGE_START,
            HasSubstr(isPopPage ? "\"$ID\":428" : "\"$ID\":429"))).Times(1);
        EXPECT_CALL(*mock, ReportContentChangeEvent(ChangeType::PAGE,
            AllOf(HasSubstr("\"$ID\":429"), Not(HasSubstr("\"$ID\":428"))))).Times(1);
        pattern->ContentChangeOnTransitionStart(destination, isPopPage);
        ASSERT_NE(destination->DoTransition(operation, true), -1);
        // Mock RenderContext does not run opacity completion; use the production callback factory.
        auto finish = destination->BuildTransitionFinishCallback(true, false);
        finish();
        EXPECT_FALSE(manager->IsTransitioning());
        EXPECT_TRUE(manager->activeContentChanges_.empty());
        Mock::VerifyAndClearExpectations(mock);
    }
    manager->StopContentChangeReport();
}

/**
 * @tc.name: ContentChangeNavigationHomeFadePopTest001
 * @tc.desc: Two single-node animations hand pop completion to Home only when START is enabled.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestCore, ContentChangeNavigationHomeFadePopTest001, TestSize.Level1)
{
    auto manager = MockPipelineContext::GetCurrent()->GetContentChangeManager();
    auto navigation = NavigationGroupNode::GetOrCreateGroupNode(
        V2::NAVIGATION_VIEW_ETS_TAG, 450, []() { return AceType::MakeRefPtr<NavigationPattern>(); });
    auto pattern = navigation->GetPattern<NavigationPattern>();
    pattern->SetNavigationStack(AceType::MakeRefPtr<NavigationStack>());
    auto home = NavDestinationGroupNode::GetOrCreateGroupNode(
        V2::NAVDESTINATION_VIEW_ETS_TAG, 451, []() { return AceType::MakeRefPtr<NavDestinationPattern>(); });
    auto detail = NavDestinationGroupNode::GetOrCreateGroupNode(
        V2::NAVDESTINATION_VIEW_ETS_TAG, 452, []() { return AceType::MakeRefPtr<NavDestinationPattern>(); });
    for (auto node : { home, detail }) {
        node->GetPattern<NavDestinationPattern>()->SetNavigationNode(navigation);
        node->SetSystemTransitionType(NavigationSystemTransitionType::FADE);
    }
    home->SetIsHomeDestination(true);
    navigation->SetUseHomeDestination(true);
    navigation->customHomeDestination_ = home;
    pattern->SetNavigationMode(NavigationMode::STACK);
    auto mock = reinterpret_cast<MockUiSessionManager*>(UiSessionManager::GetInstance());
    for (bool enabled : { false, true }) {
        ContentChangeConfig config;
        config.reportStartEvent = enabled;
        manager->StartContentChangeReport(config);
        EXPECT_CALL(*mock, ReportContentChangeEvent(ChangeType::PAGE_START, HasSubstr("\"$ID\":450")))
            .Times(enabled ? 1 : 0);
        EXPECT_CALL(*mock, ReportContentChangeEvent(ChangeType::PAGE, HasSubstr("\"$ID\":451")))
            .Times(enabled ? 1 : 0);
        pattern->ContentChangeOnTransitionStart(nullptr, true);
        navigation->TransitionWithPop(detail, home, true);
        EXPECT_EQ(home->needReportHomeTransition_, enabled);
        // Mock RenderContext does not run opacity completion; invoke the same callback factory.
        auto finish = home->BuildTransitionFinishCallback(true, false);
        finish();
        detail->BuildTransitionFinishCallback(false, false)();
        EXPECT_EQ(pattern->runningTransitionCount_, 0);
        EXPECT_FALSE(home->needReportHomeTransition_);
        EXPECT_FALSE(manager->IsTransitioning());
        EXPECT_TRUE(manager->activeContentChanges_.empty());
        Mock::VerifyAndClearExpectations(mock);
        manager->StopContentChangeReport();
    }
}

/**
 * @tc.name: ContentChangeNavigationHomeGroupFinishTest001
 * @tc.desc: Without explicit handoff, Home completion must leave END to the group callback.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestCore, ContentChangeNavigationHomeGroupFinishTest001, TestSize.Level1)
{
    auto manager = MockPipelineContext::GetCurrent()->GetContentChangeManager();
    ContentChangeConfig config;
    config.reportStartEvent = true;
    manager->StartContentChangeReport(config);
    auto navigation = NavigationGroupNode::GetOrCreateGroupNode(
        V2::NAVIGATION_VIEW_ETS_TAG, 453, []() { return AceType::MakeRefPtr<NavigationPattern>(); });
    auto pattern = navigation->GetPattern<NavigationPattern>();
    pattern->SetNavigationStack(AceType::MakeRefPtr<NavigationStack>());
    auto home = NavDestinationGroupNode::GetOrCreateGroupNode(
        V2::NAVDESTINATION_VIEW_ETS_TAG, 454, []() { return AceType::MakeRefPtr<NavDestinationPattern>(); });
    home->GetPattern<NavDestinationPattern>()->SetNavigationNode(navigation);
    home->SetIsHomeDestination(true);
    auto mock = reinterpret_cast<MockUiSessionManager*>(UiSessionManager::GetInstance());
    EXPECT_CALL(*mock, ReportContentChangeEvent(ChangeType::PAGE_START, _)).Times(1);
    EXPECT_CALL(*mock, ReportContentChangeEvent(ChangeType::PAGE, _)).Times(0);
    pattern->ContentChangeOnTransitionStart(home, true);
    pattern->OnStartOneTransitionAnimation();
    home->BuildTransitionFinishCallback(true, false)();
    EXPECT_EQ(manager->activeContentChanges_.count(navigation->GetId()), 1);
    Mock::VerifyAndClearExpectations(mock);
    EXPECT_CALL(*mock, ReportContentChangeEvent(ChangeType::PAGE, HasSubstr("\"$ID\":454"))).Times(1);
    navigation->ContentChangeReport(home, true);
    EXPECT_TRUE(manager->activeContentChanges_.empty());
    EXPECT_FALSE(manager->IsTransitioning());
    Mock::VerifyAndClearExpectations(mock);
    manager->StopContentChangeReport();
}

/**
 * @tc.name: ContentChangeNavigationCustomPopFinishTest001
 * @tc.desc: Custom pop with a null destination clears Navigation identity and dumps the configured NavBar.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestCore, ContentChangeNavigationCustomPopFinishTest001, TestSize.Level1)
{
    auto manager = MockPipelineContext::GetCurrent()->GetContentChangeManager();
    ContentChangeConfig config;
    config.reportStartEvent = true;
    manager->StartContentChangeReport(config);
    auto navigation = NavigationGroupNode::GetOrCreateGroupNode(
        V2::NAVIGATION_VIEW_ETS_TAG, 430, []() { return AceType::MakeRefPtr<NavigationPattern>(); });
    auto pattern = navigation->GetPattern<NavigationPattern>();
    pattern->SetNavigationStack(AceType::MakeRefPtr<NavigationStack>());
    auto navBar = FrameNode::CreateFrameNode(V2::NAVBAR_ETS_TAG, 431, AceType::MakeRefPtr<Pattern>(), true);
    navigation->SetNavBarNode(navBar);
    navigation->SetUseHomeDestination(false);
    auto previous = NavDestinationGroupNode::GetOrCreateGroupNode(
        V2::NAVDESTINATION_VIEW_ETS_TAG, 432, []() { return AceType::MakeRefPtr<NavDestinationPattern>(); });
    auto mock = reinterpret_cast<MockUiSessionManager*>(UiSessionManager::GetInstance());
    EXPECT_CALL(*mock, ReportContentChangeEvent(ChangeType::PAGE_START, HasSubstr("\"$ID\":430"))).Times(1);
    EXPECT_CALL(*mock, ReportContentChangeEvent(ChangeType::PAGE,
        AllOf(HasSubstr("\"$ID\":431"), Not(HasSubstr("\"$ID\":430"))))).Times(1);
    pattern->ContentChangeOnTransitionStart(nullptr, true);
    auto proxy = AceType::MakeRefPtr<NavigationTransitionProxy>();
    pattern->proxyList_.emplace_back(proxy);
    NavigationTransition transition;
    transition.isValid = true;
    transition.interactive = false;
    transition.timeout = -1;
    pattern->OnStartOneTransitionAnimation();
    ASSERT_TRUE(pattern->ExecuteAddAnimation(previous, nullptr, true, proxy, transition));
    proxy->FireFinishCallback();
    proxy->FireFinishCallback();
    EXPECT_FALSE(manager->IsTransitioning());
    EXPECT_TRUE(manager->activeContentChanges_.empty());
    Mock::VerifyAndClearExpectations(mock);
    manager->StopContentChangeReport();
}

/**
 * @tc.name: ContentChangeNavigationPopDispatchTest001
 * @tc.desc: Non-animated empty-target pop uses Navigation identity and retains the NavBar dump after layout.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestCore, ContentChangeNavigationPopDispatchTest001, TestSize.Level1)
{
    auto manager = MockPipelineContext::GetCurrent()->GetContentChangeManager();
    ContentChangeConfig config;
    config.reportStartEvent = true;
    manager->StartContentChangeReport(config);
    auto navigation = NavigationGroupNode::GetOrCreateGroupNode(
        V2::NAVIGATION_VIEW_ETS_TAG, 433, []() { return AceType::MakeRefPtr<NavigationPattern>(); });
    auto pattern = navigation->GetPattern<NavigationPattern>();
    pattern->SetNavigationStack(AceType::MakeRefPtr<NavigationStack>());
    auto navBar = FrameNode::CreateFrameNode(V2::NAVBAR_ETS_TAG, 434, AceType::MakeRefPtr<Pattern>(), true);
    navigation->SetNavBarNode(navBar);
    navigation->SetUseHomeDestination(false);
    auto mock = reinterpret_cast<MockUiSessionManager*>(UiSessionManager::GetInstance());
    {
        InSequence sequence;
        EXPECT_CALL(*mock, ReportContentChangeEvent(ChangeType::PAGE_START, HasSubstr("\"$ID\":433"))).Times(1);
        EXPECT_CALL(*mock, ReportContentChangeEvent(ChangeType::PAGE,
            AllOf(HasSubstr("\"$ID\":434"), Not(HasSubstr("\"$ID\":433"))))).Times(1);
    }
    // The pipeline mock executes after-layout tasks immediately by default.
    pattern->StartTransition(nullptr, nullptr, false, true, false);
    EXPECT_FALSE(manager->IsTransitioning());
    EXPECT_TRUE(manager->activeContentChanges_.empty());
    Mock::VerifyAndClearExpectations(mock);
    manager->StopContentChangeReport();
}

/**
 * @tc.name: ContentChangeNavigationReplaceDispatchTest001
 * @tc.desc: Replace retains destination identity even when the removed previous page sets isPopPage.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestCore, ContentChangeNavigationReplaceDispatchTest001, TestSize.Level1)
{
    auto manager = MockPipelineContext::GetCurrent()->GetContentChangeManager();
    ContentChangeConfig config;
    config.reportStartEvent = true;
    manager->StartContentChangeReport(config);
    auto navigation = NavigationGroupNode::GetOrCreateGroupNode(
        V2::NAVIGATION_VIEW_ETS_TAG, 435, []() { return AceType::MakeRefPtr<NavigationPattern>(); });
    auto pattern = navigation->GetPattern<NavigationPattern>();
    pattern->SetNavigationStack(AceType::MakeRefPtr<NavigationStack>());
    pattern->navigationStack_->UpdateReplaceValue(1);
    pattern->isReplace_ = true;
    auto destination = NavDestinationGroupNode::GetOrCreateGroupNode(
        V2::NAVDESTINATION_VIEW_ETS_TAG, 436, []() { return AceType::MakeRefPtr<NavDestinationPattern>(); });
    destination->GetPattern<NavDestinationPattern>()->SetNavigationNode(navigation);
    auto mock = reinterpret_cast<MockUiSessionManager*>(UiSessionManager::GetInstance());
    EXPECT_CALL(*mock, ReportContentChangeEvent(ChangeType::PAGE_START, HasSubstr("\"$ID\":436"))).Times(1);
    EXPECT_CALL(*mock, ReportContentChangeEvent(ChangeType::PAGE, HasSubstr("\"$ID\":436"))).Times(1);
    pattern->StartTransition(nullptr, destination, false, true, false);
    EXPECT_FALSE(manager->IsTransitioning());
    EXPECT_TRUE(manager->activeContentChanges_.empty());
    Mock::VerifyAndClearExpectations(mock);
    manager->StopContentChangeReport();
}

/**
 * @tc.name: ContentChangeNavigationCustomPopExpiredTargetTest001
 * @tc.desc: Removing the dump fallback before custom finish does not prevent Navigation identity cleanup.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestCore, ContentChangeNavigationCustomPopExpiredTargetTest001, TestSize.Level1)
{
    auto manager = MockPipelineContext::GetCurrent()->GetContentChangeManager();
    ContentChangeConfig config;
    config.reportStartEvent = true;
    manager->StartContentChangeReport(config);
    auto navigation = NavigationGroupNode::GetOrCreateGroupNode(
        V2::NAVIGATION_VIEW_ETS_TAG, 440, []() { return AceType::MakeRefPtr<NavigationPattern>(); });
    auto pattern = navigation->GetPattern<NavigationPattern>();
    pattern->SetNavigationStack(AceType::MakeRefPtr<NavigationStack>());
    auto navBar = FrameNode::CreateFrameNode(V2::NAVBAR_ETS_TAG, 441, AceType::MakeRefPtr<Pattern>(), true);
    navigation->SetNavBarNode(navBar);
    navigation->SetUseHomeDestination(false);
    auto previous = NavDestinationGroupNode::GetOrCreateGroupNode(
        V2::NAVDESTINATION_VIEW_ETS_TAG, 442, []() { return AceType::MakeRefPtr<NavDestinationPattern>(); });
    auto mock = reinterpret_cast<MockUiSessionManager*>(UiSessionManager::GetInstance());
    EXPECT_CALL(*mock, ReportContentChangeEvent(ChangeType::PAGE_START, HasSubstr("\"$ID\":440"))).Times(1);
    EXPECT_CALL(*mock, ReportContentChangeEvent(ChangeType::PAGE, HasSubstr("\"$ID\":440"))).Times(1);
    pattern->ContentChangeOnTransitionStart(nullptr, true);
    auto proxy = AceType::MakeRefPtr<NavigationTransitionProxy>();
    pattern->proxyList_.emplace_back(proxy);
    NavigationTransition transition;
    transition.isValid = true;
    transition.interactive = false;
    transition.timeout = -1;
    pattern->OnStartOneTransitionAnimation();
    ASSERT_TRUE(pattern->ExecuteAddAnimation(previous, nullptr, true, proxy, transition));
    // This node was never attached to the main tree; no detach callback performs the cleanup.
    navigation->SetNavBarNode(nullptr);
    navBar.Reset();
    EXPECT_EQ(manager->activeContentChanges_.count(navigation->GetId()), 1);
    proxy->FireFinishCallback();
    EXPECT_FALSE(manager->IsTransitioning());
    EXPECT_TRUE(manager->activeContentChanges_.empty());
    Mock::VerifyAndClearExpectations(mock);
    manager->StopContentChangeReport();
}

/**
 * @tc.name: ContentChangeNavigationCustomPopInteractiveTest001
 * @tc.desc: Interactive pop finish and interruption both clear Navigation identity without PAGE CANCEL.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestCore, ContentChangeNavigationCustomPopInteractiveTest001, TestSize.Level1)
{
    auto manager = MockPipelineContext::GetCurrent()->GetContentChangeManager();
    ContentChangeConfig config;
    config.reportStartEvent = true;
    auto mock = reinterpret_cast<MockUiSessionManager*>(UiSessionManager::GetInstance());
    for (bool success : { true, false }) {
        manager->StartContentChangeReport(config);
        auto navigation = NavigationGroupNode::GetOrCreateGroupNode(
            V2::NAVIGATION_VIEW_ETS_TAG, 443, []() { return AceType::MakeRefPtr<NavigationPattern>(); });
        auto pattern = navigation->GetPattern<NavigationPattern>();
        pattern->SetNavigationStack(AceType::MakeRefPtr<NavigationStack>());
        auto navBar = FrameNode::CreateFrameNode(V2::NAVBAR_ETS_TAG, 444, AceType::MakeRefPtr<Pattern>(), true);
        navigation->SetNavBarNode(navBar);
        navigation->SetUseHomeDestination(false);
        auto previous = NavDestinationGroupNode::GetOrCreateGroupNode(
            V2::NAVDESTINATION_VIEW_ETS_TAG, 445, []() { return AceType::MakeRefPtr<NavDestinationPattern>(); });
        pattern->SetNavigationTransition([](const RefPtr<NavDestinationContext>&,
            const RefPtr<NavDestinationContext>&, NavigationOperation) {
            NavigationTransition transition;
            transition.isValid = true;
            transition.interactive = true;
            transition.timeout = -1;
            transition.transition = [](const RefPtr<NavigationTransitionProxy>&) {};
            return transition;
        });
        EXPECT_CALL(*mock, ReportContentChangeEvent(ChangeType::PAGE_START, HasSubstr("\"$ID\":443"))).Times(1);
        EXPECT_CALL(*mock, ReportContentChangeEvent(ChangeType::PAGE,
            HasSubstr(success ? "\"$ID\":444" : "\"$ID\":443"))).Times(1);
        pattern->ContentChangeOnTransitionStart(nullptr, true);
        ASSERT_TRUE(pattern->TriggerCustomAnimation(previous, nullptr, true));
        auto proxy = pattern->GetTopNavigationProxy();
        ASSERT_NE(proxy, nullptr);
        if (success) {
            proxy->FinishInteractiveAnimation();
        } else {
            proxy->CancelInteractiveAnimation();
        }
        EXPECT_FALSE(manager->IsTransitioning());
        EXPECT_TRUE(manager->activeContentChanges_.empty());
        Mock::VerifyAndClearExpectations(mock);
        manager->StopContentChangeReport();
    }
}

/**
 * @tc.name: ContentChangeNavigationPopStartDisabledTest001
 * @tc.desc: START disabled preserves the existing no-animation END without creating fallback state.
 * @tc.type: FUNC
 */
HWTEST_F(NavigationPatternTestCore, ContentChangeNavigationPopStartDisabledTest001, TestSize.Level1)
{
    auto manager = MockPipelineContext::GetCurrent()->GetContentChangeManager();
    ContentChangeConfig config;
    config.reportStartEvent = false;
    manager->StartContentChangeReport(config);
    auto navigation = NavigationGroupNode::GetOrCreateGroupNode(
        V2::NAVIGATION_VIEW_ETS_TAG, 446, []() { return AceType::MakeRefPtr<NavigationPattern>(); });
    auto pattern = navigation->GetPattern<NavigationPattern>();
    pattern->SetNavigationStack(AceType::MakeRefPtr<NavigationStack>());
    auto navBar = FrameNode::CreateFrameNode(V2::NAVBAR_ETS_TAG, 447, AceType::MakeRefPtr<Pattern>(), true);
    navigation->SetNavBarNode(navBar);
    navigation->SetUseHomeDestination(false);
    auto mock = reinterpret_cast<MockUiSessionManager*>(UiSessionManager::GetInstance());
    EXPECT_CALL(*mock, ReportContentChangeEvent(ChangeType::PAGE_START, _)).Times(0);
    EXPECT_CALL(*mock, ReportContentChangeEvent(ChangeType::PAGE, HasSubstr("\"$ID\":447"))).Times(1);
    pattern->ContentChangeOnTransitionStart(nullptr, true);
    EXPECT_FALSE(manager->IsTransitioning());
    pattern->StartTransition(nullptr, nullptr, false, true, false);
    EXPECT_FALSE(manager->IsTransitioning());
    EXPECT_TRUE(manager->activeContentChanges_.empty());
    Mock::VerifyAndClearExpectations(mock);
    // The historical custom transition with a null target did not emit an END here.
    EXPECT_CALL(*mock, ReportContentChangeEvent(ChangeType::PAGE, _)).Times(0);
    pattern->ContentChangeReport(nullptr, true);
    Mock::VerifyAndClearExpectations(mock);
    manager->StopContentChangeReport();
}

} // namespace OHOS::Ace::NG

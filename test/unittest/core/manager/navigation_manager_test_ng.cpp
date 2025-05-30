/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <optional>

#include "gtest/gtest.h"
#include "test/unittest/core/pattern/navigation/mock_navigation_route.h"
#include "test/unittest/core/pattern/navigation/mock_navigation_stack.h"

#include "core/components_ng/pattern/navigation/navigation_model_ng.h"
#include "core/components_ng/pattern/navigation/navigation_pattern.h"
#include "test/mock/core/pipeline/mock_pipeline_context.h"
#include "test/mock/core/common/mock_container.h"
#include "test/mock/core/common/mock_theme_manager.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {
constexpr char DEST_NAME_BASE[] = "dest";
RefPtr<NavigationManager> GetNavigationManager()
{
    auto pipeline = MockPipelineContext::GetCurrent();
    return pipeline ? pipeline->GetNavigationManager() : nullptr;
}

void RunNavigationStackSync(const RefPtr<NavigationPattern>& navigationPattern)
{
    navigationPattern->OnModifyDone();
    navigationPattern->MarkNeedSyncWithJsStack();
    navigationPattern->SyncWithJsStackIfNeeded();
}

RefPtr<NavigationGroupNode> InitAndCreateNavigation(const RefPtr<MockNavigationStack>& stack)
{
    NavigationModelNG navigationModel;
    navigationModel.Create();
    navigationModel.SetNavigationStack(stack);
    return AceType::DynamicCast<NavigationGroupNode>(ViewStackProcessor::GetInstance()->Finish());
}
} // namespace

class NavigationManagerTestNg : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
};

void NavigationManagerTestNg::SetUpTestSuite()
{
    MockPipelineContext::SetUp();
    MockContainer::SetUp();
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(AceType::MakeRefPtr<NavigationBarTheme>()));
    EXPECT_CALL(*themeManager, GetTheme(_, _)).WillRepeatedly(Return(AceType::MakeRefPtr<NavigationBarTheme>()));
    MockContainer::Current()->SetNavigationRoute(AceType::MakeRefPtr<MockNavigationRoute>(""));
}

void NavigationManagerTestNg::TearDownTestSuite()
{
    MockPipelineContext::TearDown();
    MockContainer::TearDown();
}

/**
 * @tc.name: NavigationManagerTest001
 * @tc.desc: Test basic ability of get navigationJsonInfo
 * @tc.type: FUNC
 * @tc.author:
 */
HWTEST_F(NavigationManagerTestNg, NavigationManagerTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. test whether can get navigation manager
     */
    auto navigationManager = GetNavigationManager();
    ASSERT_NE(navigationManager, nullptr);
    /**
     * @tc.steps: step2. test whether recoverable can be set
     */
    auto mockNavPathStack = AceType::MakeRefPtr<MockNavigationStack>();
    auto navigationNode = InitAndCreateNavigation(mockNavPathStack);
    ASSERT_NE(navigationNode, nullptr);
    ASSERT_EQ(navigationNode->CanRecovery(), false);
    navigationNode->OnInspectorIdUpdate("testTag");
    ASSERT_EQ(navigationNode->CanRecovery(), false);
    navigationNode->SetRecoverable(true);
    ASSERT_EQ(navigationNode->CanRecovery(), true);
    /**
     * @tc.steps: step3. test whether can get navigation recovery info
     */
    auto navigationPattern = AceType::DynamicCast<NavigationPattern>(navigationNode->GetPattern());
    ASSERT_NE(navigationPattern, nullptr);
    navigationPattern->OnModifyDone();
    auto json = navigationManager->GetNavigationJsonInfo();
    ASSERT_NE(json, nullptr);
}

/**
 * @tc.name: NavigationManagerTest002
 * @tc.desc: Test getting complex navigationJsonInfo
 * @tc.type: FUNC
 * @tc.author:
 */
HWTEST_F(NavigationManagerTestNg, NavigationManagerTest002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. get navigation manager
     */
    auto navigationManager = GetNavigationManager();
    ASSERT_NE(navigationManager, nullptr);
    /**
     * @tc.steps: step2. build navigation
     */
    auto mockNavPathStack = AceType::MakeRefPtr<MockNavigationStack>();
    auto navigationNode = InitAndCreateNavigation(mockNavPathStack);
    ASSERT_NE(navigationNode, nullptr);
    const std::string navigationId = "navigationId";
    navigationNode->OnInspectorIdUpdate(navigationId);
    navigationNode->SetRecoverable(true);
    /**
     * @tc.steps: step3. mock stack operation and sync stack
     */
    const int32_t testNumber = 10;
    for (int32_t index = 0; index < testNumber; ++ index) {
        mockNavPathStack->MockPushPath(AceType::MakeRefPtr<MockNavPathInfo>(DEST_NAME_BASE + std::to_string(index)));
    }
    auto navigationPattern = AceType::DynamicCast<NavigationPattern>(navigationNode->GetPattern());
    ASSERT_NE(navigationNode, navigationPattern);
    RunNavigationStackSync(navigationPattern);
    auto navigationContent = navigationNode->GetContentNode();
    ASSERT_NE(navigationContent, nullptr);
    ASSERT_EQ(static_cast<int32_t>(navigationContent->GetChildren().size()), testNumber);
    /**
     * @tc.steps: step4. mock set recovery property and check
     */
    const int32_t recoveryDestNum = 5;
    for (int32_t index = recoveryDestNum; index < testNumber; ++ index) {
        auto navDestination = AceType::DynamicCast<NavDestinationGroupNode>(navigationContent->GetChildAtIndex(index));
        ASSERT_NE(navDestination, nullptr);
        navDestination->SetRecoverable(false);
    }
    for (int32_t index = 0; index < recoveryDestNum; ++ index) {
        auto navDestination = AceType::DynamicCast<NavDestinationGroupNode>(navigationContent->GetChildAtIndex(index));
        ASSERT_NE(navDestination, nullptr);
        // the default value of navdestination->CanRecovery() should be true
        ASSERT_EQ(navDestination->CanRecovery(), index < recoveryDestNum ? true : false);
    }
    /**
     * @tc.steps: step5. get json info and do verify
     */
    auto json = navigationManager->GetNavigationJsonInfo();
    std::cout << "Got Json: " << json->ToString() << std::endl;
    ASSERT_NE(json, nullptr);
    ASSERT_EQ(json->IsArray(), true);
    auto navigationInfo = json->GetArrayItem(0);
    ASSERT_NE(navigationInfo, nullptr);
    auto stackInfo = navigationInfo->GetValue("stack");
    ASSERT_NE(stackInfo, nullptr);
    ASSERT_EQ(stackInfo->IsArray(), true);
    ASSERT_EQ(stackInfo->GetArraySize(), recoveryDestNum);
    for (int32_t index = 0; index < recoveryDestNum; ++ index) {
        auto destInfo = stackInfo->GetArrayItem(index);
        ASSERT_NE(destInfo, nullptr);
        auto nameInfo = destInfo->GetValue("name");
        ASSERT_NE(nameInfo, nullptr);
        ASSERT_EQ(nameInfo->IsString(), true);
        ASSERT_EQ(nameInfo->GetString(), DEST_NAME_BASE + std::to_string(index));
    }
}

/**
 * @tc.name: NavigationManagerTest003
 * @tc.desc: Test storage navigationJsonInfo
 * @tc.type: FUNC
 * @tc.author:
 */
HWTEST_F(NavigationManagerTestNg, NavigationManagerTest003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. get navigation manager
     */
    auto navigationManager = GetNavigationManager();
    ASSERT_NE(navigationManager, nullptr);
    /**
     * @tc.steps: step2. build navigation
     */
    auto mockNavPathStack = AceType::MakeRefPtr<MockNavigationStack>();
    auto navigationNode = InitAndCreateNavigation(mockNavPathStack);
    ASSERT_NE(navigationNode, nullptr);
    const std::string navigationId = "navigationId";
    navigationNode->OnInspectorIdUpdate(navigationId);
    navigationNode->SetRecoverable(true);
    /**
     * @tc.steps: step3. mock stack operation and sync stack
     */
    const int32_t testNumber = 10;
    for (int32_t index = 0; index < testNumber; ++ index) {
        mockNavPathStack->MockPushPath(AceType::MakeRefPtr<MockNavPathInfo>(DEST_NAME_BASE + std::to_string(index)));
    }
    auto navigationPattern = AceType::DynamicCast<NavigationPattern>(navigationNode->GetPattern());
    ASSERT_NE(navigationNode, navigationPattern);
    RunNavigationStackSync(navigationPattern);
    auto navigationContent = navigationNode->GetContentNode();
    ASSERT_NE(navigationContent, nullptr);
    ASSERT_EQ(static_cast<int32_t>(navigationContent->GetChildren().size()), testNumber);
    /**
     * @tc.steps: step4. mock set recovery property and check
     */
    const int32_t recoveryDestNum = 5;
    for (int32_t index = recoveryDestNum; index < testNumber; ++ index) {
        auto navDestination = AceType::DynamicCast<NavDestinationGroupNode>(navigationContent->GetChildAtIndex(index));
        ASSERT_NE(navDestination, nullptr);
        navDestination->SetRecoverable(false);
    }
    /**
     * @tc.steps: step5. get json info and do storage
     */
    auto json = navigationManager->GetNavigationJsonInfo();
    navigationManager->StorageNavigationRecoveryInfo(std::move(json));
    auto recoveryDests = navigationManager->GetNavigationRecoveryInfo(navigationId);
    ASSERT_EQ(static_cast<int32_t>(recoveryDests.size()), recoveryDestNum);
    for (int32_t index = 0; index < recoveryDestNum; ++ index) {
        auto name = recoveryDests[index].name;
        ASSERT_EQ(name, DEST_NAME_BASE + std::to_string(index));
    }
}

/**
 * @tc.name: NavigationManagerTest004
 * @tc.desc: Test cache navigation node animation
 * @tc.type: FUNC
 * @tc.author:
 */
HWTEST_F(NavigationManagerTestNg, NavigationManagerTest004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. get navigation manager
     */
    auto navigationManager = GetNavigationManager();
    ASSERT_NE(navigationManager, nullptr);
    /**
     * @tc.steps: step2. build navigation
     */
    auto preNode = NavDestinationGroupNode::GetOrCreateGroupNode(
        "navDestinationNode", 3, []() { return AceType::MakeRefPtr<NavDestinationPattern>(); });
    auto contentNode = NavDestinationGroupNode::GetOrCreateGroupNode(
        V2::UI_EXTENSION_COMPONENT_ETS_TAG, 22, []() { return AceType::MakeRefPtr<NavDestinationPattern>(); });
    preNode->contentNode_ = contentNode;
    auto preFrameNode = AceType::DynamicCast<FrameNode>(preNode);
    navigationManager->SetNavNodeInTransition(nullptr, preFrameNode);
    navigationManager->SetPreNodeNeverSet(false);
    /**
     * @tc.steps: step3. Update previous navigation render group property
     */
    navigationManager->UpdatePreNavNodeRenderGroupProperty();
    bool preNodeNeverSet = navigationManager->preNodeNeverSet_;
    ASSERT_EQ(preNodeNeverSet, false);
}

/**
 * @tc.name: NavigationManagerTest005
 * @tc.desc: Test cache navigation node animation
 * @tc.type: FUNC
 * @tc.author:
 */
HWTEST_F(NavigationManagerTestNg, NavigationManagerTest005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. get navigation manager
     */
    auto navigationManager = GetNavigationManager();
    ASSERT_NE(navigationManager, nullptr);
    /**
     * @tc.steps: step2. build navigation
     */
    auto curNode = NavDestinationGroupNode::GetOrCreateGroupNode(
        "navDestinationNode", 3, []() { return AceType::MakeRefPtr<NavDestinationPattern>(); });
    auto contentNode = NavDestinationGroupNode::GetOrCreateGroupNode(
        V2::UI_EXTENSION_COMPONENT_ETS_TAG, 22, []() { return AceType::MakeRefPtr<NavDestinationPattern>(); });
    curNode->contentNode_ = contentNode;
    auto curFrameNode = AceType::DynamicCast<FrameNode>(curNode);
    navigationManager->SetNavNodeInTransition(curFrameNode, nullptr);
    /**
     * @tc.steps: step3. Update current navigation render group property
     */
    navigationManager->UpdateCurNavNodeRenderGroupProperty();
    bool currentNodeNeverSet = navigationManager->currentNodeNeverSet_;
    bool curNodeAnimationCached = navigationManager->curNodeAnimationCached_;
    ASSERT_EQ(currentNodeNeverSet, false);
    ASSERT_EQ(curNodeAnimationCached, false);
}

/**
 * @tc.name: NavigationManagerTest006
 * @tc.desc: Test cache navigation node animation
 * @tc.type: FUNC
 * @tc.author:
 */
HWTEST_F(NavigationManagerTestNg, NavigationManagerTest006, TestSize.Level1)
{
    /**
     * @tc.steps: step1. get navigation manager
     */
    auto navigationManager = GetNavigationManager();
    ASSERT_NE(navigationManager, nullptr);
    /**
     * @tc.steps: step2. build navigation
     */
    auto curNode = NavDestinationGroupNode::GetOrCreateGroupNode(
        "navDestinationNode", 3, []() { return AceType::MakeRefPtr<NavDestinationPattern>(); });
    auto contentNode = NavDestinationGroupNode::GetOrCreateGroupNode(
        V2::UI_EXTENSION_COMPONENT_ETS_TAG, 22, []() { return AceType::MakeRefPtr<NavDestinationPattern>(); });
    curNode->contentNode_ = contentNode;
    auto curFrameNode = AceType::DynamicCast<FrameNode>(curNode);
    navigationManager->SetNavNodeInTransition(curFrameNode, nullptr);
    /**
     * @tc.steps: step3. Reset current navigation render group property
     */
    navigationManager->ResetCurNavNodeRenderGroupProperty();
    bool curNodeAnimationCached = navigationManager->curNodeAnimationCached_;
    bool currentNodeNeverSet = navigationManager->currentNodeNeverSet_;
    ASSERT_EQ(curNodeAnimationCached, false);
    ASSERT_EQ(currentNodeNeverSet, false);
}

/**
 * @tc.name: NavigationManagerTest007
 * @tc.desc: Test unconfig navigation and disable animation
 * @tc.type: FUNC
 * @tc.author:
 */
HWTEST_F(NavigationManagerTestNg, NavigationManagerTest007, TestSize.Level1)
{
    /**
     * @tc.steps: step1. get navigation manager
     */
    auto navigationManager = GetNavigationManager();
    ASSERT_NE(navigationManager, nullptr);
    /**
     * @tc.steps: step2. build navigation
     */
    navigationManager->hasCacheNavigationNodeEnable_ = true;
    auto mockNavPathStack = AceType::MakeRefPtr<MockNavigationStack>();
    auto navigationNode = InitAndCreateNavigation(mockNavPathStack);
    ASSERT_NE(navigationNode, nullptr);
    const std::string navigationId = "navigationId";
    navigationNode->OnInspectorIdUpdate(navigationId);
    navigationNode->SetRecoverable(true);
    /**
     * @tc.steps: step3. Unconfig navigation and disable animation
     */
    auto preNode = AceType::DynamicCast<FrameNode>(navigationNode->GetNavBarNode());
    auto curNode = AceType::DynamicCast<FrameNode>(navigationNode->GetTopDestination());
    navigationManager->hasCacheNavigationNodeEnable_ = true;
    navigationNode->ResetTransitionAnimationNodeState(preNode, curNode);

    auto managerCurNode = navigationManager->curNavNode_.Upgrade();
    auto managerPreNode = navigationManager->preNavNode_.Upgrade();
    bool isInAnimation = navigationManager->IsNavigationInAnimation();
    ASSERT_EQ(managerCurNode, nullptr);
    ASSERT_EQ(managerPreNode, nullptr);
    ASSERT_EQ(isInAnimation, false);
}

/**
 * @tc.name: IsOuterMostNavigation001
 * @tc.desc: Branch: if (dumpMap_.empty()) { => true;
 * @tc.type: FUNC
 * @tc.author:
 */
HWTEST_F(NavigationManagerTestNg, IsOuterMostNavigation001, TestSize.Level1)
{
    auto manager = GetNavigationManager();
    ASSERT_NE(manager, nullptr);
    manager->dumpMap_.clear();

    auto dest1 = NavDestinationGroupNode::GetOrCreateGroupNode(
        V2::NAVDESTINATION_VIEW_ETS_TAG, 3, []() { return AceType::MakeRefPtr<NavDestinationPattern>(); });
    ASSERT_NE(dest1, nullptr);
    EXPECT_FALSE(manager->IsOuterMostNavigation(dest1->GetId(), dest1->GetDepth()));
}

/**
 * @tc.name: IsOuterMostNavigation002
 * @tc.desc: Branch: outerMostKey == DumpMapKey(nodeId, depth); => true;
 * @tc.type: FUNC
 * @tc.author:
 */
HWTEST_F(NavigationManagerTestNg, IsOuterMostNavigation002, TestSize.Level1)
{
    auto manager = GetNavigationManager();
    ASSERT_NE(manager, nullptr);
    manager->dumpMap_.clear();

    auto dest1 = NavDestinationGroupNode::GetOrCreateGroupNode(
        V2::NAVDESTINATION_VIEW_ETS_TAG, 1, []() { return AceType::MakeRefPtr<NavDestinationPattern>(); });
    ASSERT_NE(dest1, nullptr);
    dest1->SetDepth(1);
    auto dest2 = NavDestinationGroupNode::GetOrCreateGroupNode(
        V2::NAVDESTINATION_VIEW_ETS_TAG, 2, []() { return AceType::MakeRefPtr<NavDestinationPattern>(); });
    ASSERT_NE(dest2, nullptr);
    dest2->SetDepth(2);
    auto dest3 = NavDestinationGroupNode::GetOrCreateGroupNode(
        V2::NAVDESTINATION_VIEW_ETS_TAG, 3, []() { return AceType::MakeRefPtr<NavDestinationPattern>(); });
    ASSERT_NE(dest3, nullptr);
    dest3->SetDepth(2);
    manager->AddNavigationDumpCallback(dest1->GetId(), dest1->GetDepth(), [](int) {});
    manager->AddNavigationDumpCallback(dest2->GetId(), dest2->GetDepth(), [](int) {});
    manager->AddNavigationDumpCallback(dest3->GetId(), dest3->GetDepth(), [](int) {});

    EXPECT_TRUE(manager->IsOuterMostNavigation(dest1->GetId(), dest1->GetDepth()));
}

/**
 * @tc.name: IsOuterMostNavigation003
 * @tc.desc: Branch: outerMostKey == DumpMapKey(nodeId, depth); => false;
 * @tc.type: FUNC
 * @tc.author:
 */
HWTEST_F(NavigationManagerTestNg, IsOuterMostNavigation003, TestSize.Level1)
{
    auto manager = GetNavigationManager();
    ASSERT_NE(manager, nullptr);
    manager->dumpMap_.clear();

    auto dest1 = NavDestinationGroupNode::GetOrCreateGroupNode(
        V2::NAVDESTINATION_VIEW_ETS_TAG, 1, []() { return AceType::MakeRefPtr<NavDestinationPattern>(); });
    ASSERT_NE(dest1, nullptr);
    dest1->SetDepth(1);
    auto dest2 = NavDestinationGroupNode::GetOrCreateGroupNode(
        V2::NAVDESTINATION_VIEW_ETS_TAG, 2, []() { return AceType::MakeRefPtr<NavDestinationPattern>(); });
    ASSERT_NE(dest2, nullptr);
    dest2->SetDepth(2);
    auto dest3 = NavDestinationGroupNode::GetOrCreateGroupNode(
        V2::NAVDESTINATION_VIEW_ETS_TAG, 3, []() { return AceType::MakeRefPtr<NavDestinationPattern>(); });
    ASSERT_NE(dest3, nullptr);
    dest3->SetDepth(2);
    manager->AddNavigationDumpCallback(dest1->GetId(), dest1->GetDepth(), [](int) {});
    manager->AddNavigationDumpCallback(dest2->GetId(), dest2->GetDepth(), [](int) {});
    manager->AddNavigationDumpCallback(dest3->GetId(), dest3->GetDepth(), [](int) {});

    EXPECT_FALSE(manager->IsOuterMostNavigation(dest2->GetId(), dest2->GetDepth()));
}
} // namespace OHOS::Ace::NG

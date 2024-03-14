/*
 * Copyright (c) 2023 iSoftStone Information Technology (Group) Co.,Ltd.
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

#define protected public
#define private public

#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/observer_handler.h"
#include "core/components_ng/base/ui_node.h"
#include "core/components_ng/pattern/scroll/scroll_pattern.h"
#include "core/components_ng/pattern/navigation/navigation_pattern.h"
#include "core/components_ng/pattern/navrouter/navdestination_layout_property.h"
#include "core/components_ng/pattern/navrouter/navdestination_model_ng.h"
#include "core/components_ng/pattern/navrouter/navdestination_pattern.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "test/mock/core/common/mock_container.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {

class ObserverTestNg : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
};

void ObserverTestNg::SetUpTestCase()
{
    MockContainer::SetUp();
}

void ObserverTestNg::TearDownTestCase()
{
    MockContainer::TearDown();
}

/**
 * @tc.name: ObserverTestNg001
 * @tc.desc: Test the operation of Observer
 * @tc.type: FUNC
 */
HWTEST_F(ObserverTestNg, ObserverTestNg001, TestSize.Level1)
{
    auto navigation = NavigationGroupNode::GetOrCreateGroupNode(
        "navigation", 11, []() { return AceType::MakeRefPtr<NavigationPattern>(); });
    auto contentNode = NavDestinationGroupNode::GetOrCreateGroupNode(
        V2::NAVDESTINATION_VIEW_ETS_TAG, 22, []() { return AceType::MakeRefPtr<NavDestinationPattern>(); });
    auto pattern = contentNode->GetPattern<NavDestinationPattern>();
    UIObserverHandler::GetInstance().NotifyNavigationStateChange(AceType::WeakClaim(Referenced::RawPtr(pattern)),
                                                                 NavDestinationState::ON_SHOWN);
    ASSERT_EQ(UIObserverHandler::GetInstance().navigationHandleFunc_, nullptr);
    auto pattern1 = navigation->GetPattern<NavigationPattern>();
    pattern1->navigationStack_ = AceType::MakeRefPtr<NavigationStack>();
    UIObserverHandler::GetInstance().NotifyNavigationStateChange(AceType::WeakClaim(Referenced::RawPtr(pattern1)),
                                                                 NavDestinationState::ON_SHOWN);
    ASSERT_EQ(UIObserverHandler::GetInstance().navigationHandleFunc_, nullptr);
}

/**
 * @tc.name: ObserverTestNg002
 * @tc.desc: Test the operation of Observer
 * @tc.type: FUNC
 */
HWTEST_F(ObserverTestNg, ObserverTestNg002, TestSize.Level1)
{
    auto navigation = NavigationGroupNode::GetOrCreateGroupNode(
        "navigation", 11, []() { return AceType::MakeRefPtr<NavigationPattern>(); });
    navigation->GetPattern<NavigationPattern>()->navigationStack_ = AceType::MakeRefPtr<NavigationStack>();
    auto contentNode = NavDestinationGroupNode::GetOrCreateGroupNode(
        V2::NAVDESTINATION_VIEW_ETS_TAG, 22, []() { return AceType::MakeRefPtr<NavDestinationPattern>(); });
    auto pattern = contentNode->GetPattern<NavDestinationPattern>();
    pattern->name_ = "test_name";
    pattern->isOnShow_ = true;
    pattern->navigationNode_ = AceType::WeakClaim(Referenced::RawPtr(navigation));

    auto info = UIObserverHandler::GetInstance().GetNavigationState(nullptr);
    ASSERT_EQ(info, nullptr);

    info = UIObserverHandler::GetInstance().GetNavigationState(navigation);
    ASSERT_EQ(info, nullptr);

    ASSERT_EQ(pattern->GetNavigationNode(), navigation);

    info = UIObserverHandler::GetInstance().GetNavigationState(contentNode);
    ASSERT_EQ(info->name, "test_name");
    ASSERT_EQ(info->navigationId, "");
    ASSERT_EQ(info->state, NavDestinationState::ON_SHOWN);
}

/**
 * @tc.name: ObserverTestNg003
 * @tc.desc: Test the operation of Observer
 * @tc.type: FUNC
 */
HWTEST_F(ObserverTestNg, ObserverTestNg003, TestSize.Level1)
{
    double testDensity = 0.;
    UIObserverHandler::DensityHandleFunc densityHandleFunc = [&testDensity](AbilityContextInfo& context, double density) -> void {
        testDensity = density;
    };
    UIObserverHandler::GetInstance().densityHandleFunc_ = densityHandleFunc;
    double targetDensity = 3.5;
    UIObserverHandler::GetInstance().NotifyDensityChange(targetDensity);
    EXPECT_EQ(testDensity, targetDensity);
}
}

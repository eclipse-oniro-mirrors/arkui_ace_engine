/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, nullptr);
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "core/interfaces/native/implementation/nav_destination_context_peer.h"
#include "accessor_test_base.h"
#include "node_api.h"
#include "core/interfaces/native/utility/converter.h"
#include "core/interfaces/native/utility/reverse_converter.h"
#include "test/mock/core/render/mock_animation_utils.h"

#include "gmock/gmock.h"

namespace OHOS::Ace::NG {

using namespace testing;
using namespace testing::ext;
using namespace Converter;

class NavDestinationContextAccessorTest : public AccessorTestBase<
    GENERATED_ArkUINavDestinationContextAccessor,
    &GENERATED_ArkUIAccessors::getNavDestinationContextAccessor,
    NavDestinationContextPeer> {
};

/**
 * @tc.name: initialStateTest
 * @tc.desc: Check the functionality of NavDestinationContextAccessor
 * @tc.type: FUNC
 */
HWTEST_F(NavDestinationContextAccessorTest, initialStateTest, TestSize.Level1)
{
    ASSERT_NE(accessor_->getConfigInRouteMap, nullptr);
    EXPECT_EQ(accessor_->getConfigInRouteMap(peer_), nullptr);
}
} // namespace OHOS::Ace::NG

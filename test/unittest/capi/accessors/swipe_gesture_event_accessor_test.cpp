/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "accessor_test_base.h"
#include "core/interfaces/native/implementation/swipe_gesture_event_peer.h"
#include "core/interfaces/native/utility/converter.h"
#include "core/interfaces/native/utility/reverse_converter.h"
#include "test/unittest/capi/accessors/accessor_test_fixtures.h"

namespace OHOS::Ace::NG {

using namespace testing;
using namespace testing::ext;

class SwipeGestureEventAccessorTest
    : public AccessorTestBase<GENERATED_ArkUISwipeGestureEventAccessor,
        &GENERATED_ArkUIAccessors::getSwipeGestureEventAccessor, SwipeGestureEventPeer> {
public:
    void SetUp(void) override
    {
        AccessorTestBase::SetUp();
        const double testDensity = 2.0;
        MockPipelineContext::GetCurrent()->SetDensity(testDensity);
        eventInfo_ = std::make_shared<SwipeGestureEvent>();
        peer_->SetEventInfo(eventInfo_);
    }
    std::shared_ptr<SwipeGestureEvent> eventInfo_;
};

/**
 * @tc.name: GetAngleTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(SwipeGestureEventAccessorTest, GetAngleTest, TestSize.Level1)
{
    for (auto& [input, value, expected] : AccessorTestFixtures::testFixtureDoubleArkInt32Values) {
        eventInfo_->SetAngle(value);
        auto arkRes = accessor_->getAngle(peer_);
        EXPECT_EQ(arkRes, expected) <<
            "Input value is: " << input << ", method: GetAngle";
    }
}

/**
 * @tc.name: SetAngleTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(SwipeGestureEventAccessorTest, SetAngleTest, TestSize.Level1)
{
    for (auto& [input, value, expected] : AccessorTestFixtures::testFixtureNumberValues) {
        accessor_->setAngle(peer_, &value);
        EXPECT_NEAR(eventInfo_->GetAngle(), expected, FLT_EPSILON) <<
            "Input value is: " << input << ", method: SetAngle";
    }
}

/**
 * @tc.name: GetSpeedTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(SwipeGestureEventAccessorTest, GetSpeedTest, TestSize.Level1)
{
    for (auto& [input, value, expected] : AccessorTestFixtures::testFixtureDoubleArkInt32Values) {
        eventInfo_->SetSpeed(value);
        auto arkRes = accessor_->getSpeed(peer_);
        EXPECT_EQ(arkRes, expected) <<
            "Input value is: " << input << ", method: GetSpeed";
    }
}

/**
 * @tc.name: SetSpeedTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(SwipeGestureEventAccessorTest, SetSpeedTest, TestSize.Level1)
{
    for (auto& [input, value, expected] : AccessorTestFixtures::testFixtureNumberValues) {
        accessor_->setSpeed(peer_, &value);
        EXPECT_NEAR(eventInfo_->GetSpeed(), expected, FLT_EPSILON) <<
            "Input value is: " << input << ", method: SetSpeed";
    }
}
}
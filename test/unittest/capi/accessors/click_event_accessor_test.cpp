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

#include "gmock/gmock.h"

#include "accessor_test_base.h"
#include "test/unittest/capi/accessors/accessor_test_fixtures.h"
#include "core/interfaces/native/utility/converter.h"
#include "core/interfaces/native/utility/reverse_converter.h"
#include "core/interfaces/native/implementation/click_event_peer.h"

namespace OHOS::Ace::NG {

using namespace testing;
using namespace testing::ext;
using namespace AccessorTestFixtures;

namespace {
    const double EPSILON = 0.00001;
} // namespace

class ClickEventAccessorTest : public AccessorTestBase<GENERATED_ArkUIClickEventAccessor,
    &GENERATED_ArkUIAccessors::getClickEventAccessor, ClickEventPeer> {
public:
    void SetUp(void) override
    {
        AccessorTestBase::SetUp();
        ASSERT_NE(peer_, nullptr);
        eventInfo_ = std::make_unique<GestureEvent>();
        peer_->SetEventInfo(eventInfo_.get());
    }

    std::unique_ptr<GestureEvent> eventInfo_;
};

/**
 * @tc.name: setDisplayXTestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ClickEventAccessorTest, setDisplayXTestValidValues, TestSize.Level1)
{
    ASSERT_NE(accessor_->setDisplayX, nullptr);

    for (auto& [input, value, expecte] : testFixtureNumberDoubleValues) {
        accessor_->setDisplayX(peer_, &value);
        const auto& offset = eventInfo_->GetScreenLocation();
        const auto result = PipelineBase::Px2VpWithCurrentDensity(offset.GetX());
        EXPECT_NEAR(result, expecte, EPSILON) << "Input value is: " << input;
    }
}

/**
 * @tc.name: getDisplayXTestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ClickEventAccessorTest, getDisplayXTestValidValues, TestSize.Level1)
{
    ASSERT_NE(accessor_->getDisplayX, nullptr);

    for (auto& [input, value, expecte] : testFixtureNumberDoubleRoundValues) {
        Offset offset = eventInfo_->GetScreenLocation();
        const auto convValue = Converter::Convert<double>(value);
        const auto newValue = PipelineBase::Vp2PxWithCurrentDensity(convValue);
        offset.SetX(newValue);
        eventInfo_->SetScreenLocation(offset);
        auto result = Converter::Convert<int>(accessor_->getDisplayX(peer_));
        result = static_cast<double>(result);
        EXPECT_NEAR(result, expecte, EPSILON) << "Input value is: " << input;
    }
}

/**
 * @tc.name: setDisplayYTestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ClickEventAccessorTest, setDisplayYTestValidValues, TestSize.Level1)
{
    ASSERT_NE(accessor_->setDisplayY, nullptr);

    for (auto& [input, value, expecte] : testFixtureNumberDoubleValues) {
        accessor_->setDisplayY(peer_, &value);
        const auto& offset = eventInfo_->GetScreenLocation();
        const auto result = PipelineBase::Px2VpWithCurrentDensity(offset.GetY());
        EXPECT_NEAR(result, expecte, EPSILON) << "Input value is: " << input;
    }
}

/**
 * @tc.name: getDisplayYTestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ClickEventAccessorTest, getDisplayYTestValidValues, TestSize.Level1)
{
    ASSERT_NE(accessor_->getDisplayY, nullptr);

    for (auto& [input, value, expecte] : testFixtureNumberDoubleRoundValues) {
        Offset offset = eventInfo_->GetScreenLocation();
        const auto convValue = Converter::Convert<double>(value);
        const auto newValue = PipelineBase::Vp2PxWithCurrentDensity(convValue);
        offset.SetY(newValue);
        eventInfo_->SetScreenLocation(offset);
        auto result = Converter::Convert<int>(accessor_->getDisplayY(peer_));
        result = static_cast<double>(result);
        EXPECT_NEAR(result, expecte, EPSILON) << "Input value is: " << input;
    }
}

/**
 * @tc.name: setWindowXTestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ClickEventAccessorTest, setWindowXTestValidValues, TestSize.Level1)
{
    ASSERT_NE(accessor_->setWindowX, nullptr);

    for (auto& [input, value, expecte] : testFixtureNumberDoubleValues) {
        accessor_->setWindowX(peer_, &value);
        const auto& offset = eventInfo_->GetGlobalLocation();
        const auto result = PipelineBase::Px2VpWithCurrentDensity(offset.GetX());
        EXPECT_NEAR(result, expecte, EPSILON) << "Input value is: " << input;
    }
}

/**
 * @tc.name: getWindowXTestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ClickEventAccessorTest, getWindowXTestValidValues, TestSize.Level1)
{
    ASSERT_NE(accessor_->getWindowX, nullptr);

    for (auto& [input, value, expecte] : testFixtureNumberDoubleRoundValues) {
        Offset offset = eventInfo_->GetGlobalLocation();
        const auto convValue = Converter::Convert<double>(value);
        const auto newValue = PipelineBase::Vp2PxWithCurrentDensity(convValue);
        offset.SetX(newValue);
        eventInfo_->SetGlobalLocation(offset);
        auto result = Converter::Convert<int>(accessor_->getWindowX(peer_));
        result = static_cast<double>(result);
        EXPECT_NEAR(result, expecte, EPSILON) << "Input value is: " << input;
    }
}

/**
 * @tc.name: setWindowYTestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ClickEventAccessorTest, setWindowYTestValidValues, TestSize.Level1)
{
    ASSERT_NE(accessor_->setWindowY, nullptr);

    for (auto& [input, value, expecte] : testFixtureNumberDoubleValues) {
        accessor_->setWindowY(peer_, &value);
        const auto& offset = eventInfo_->GetGlobalLocation();
        const auto result = PipelineBase::Px2VpWithCurrentDensity(offset.GetY());
        EXPECT_NEAR(result, expecte, EPSILON) << "Input value is: " << input;
    }
}

/**
 * @tc.name: getWindowYTestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ClickEventAccessorTest, getWindowYTestValidValues, TestSize.Level1)
{
    ASSERT_NE(accessor_->getWindowY, nullptr);

    for (auto& [input, value, expecte] : testFixtureNumberDoubleRoundValues) {
        Offset offset = eventInfo_->GetGlobalLocation();
        const auto convValue = Converter::Convert<double>(value);
        const auto newValue = PipelineBase::Vp2PxWithCurrentDensity(convValue);
        offset.SetY(newValue);
        eventInfo_->SetGlobalLocation(offset);
        auto result = Converter::Convert<int>(accessor_->getWindowY(peer_));
        result = static_cast<double>(result);
        EXPECT_NEAR(result, expecte, EPSILON) << "Input value is: " << input;
    }
}

/**
 * @tc.name: setXTestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ClickEventAccessorTest, setXTestValidValues, TestSize.Level1)
{
    ASSERT_NE(accessor_->setX, nullptr);

    for (auto& [input, value, expecte] : testFixtureNumberDoubleValues) {
        accessor_->setX(peer_, &value);
        const auto& offset = eventInfo_->GetLocalLocation();
        const auto result = PipelineBase::Px2VpWithCurrentDensity(offset.GetX());
        EXPECT_NEAR(result, expecte, EPSILON) << "Input value is: " << input;
    }
}

/**
 * @tc.name: getXTestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ClickEventAccessorTest, getXTestValidValues, TestSize.Level1)
{
    ASSERT_NE(accessor_->getX, nullptr);

    for (auto& [input, value, expecte] : testFixtureNumberDoubleRoundValues) {
        Offset offset = eventInfo_->GetLocalLocation();
        const auto convValue = Converter::Convert<double>(value);
        const auto newValue = PipelineBase::Vp2PxWithCurrentDensity(convValue);
        offset.SetX(newValue);
        eventInfo_->SetLocalLocation(offset);
        auto result = Converter::Convert<int>(accessor_->getX(peer_));
        result = static_cast<double>(result);
        EXPECT_NEAR(result, expecte, EPSILON) << "Input value is: " << input;
    }
}

/**
 * @tc.name: setYTestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ClickEventAccessorTest, setYTestValidValues, TestSize.Level1)
{
    ASSERT_NE(accessor_->setY, nullptr);

    for (auto& [input, value, expecte] : testFixtureNumberDoubleValues) {
        accessor_->setY(peer_, &value);
        const auto& offset = eventInfo_->GetLocalLocation();
        const auto result = PipelineBase::Px2VpWithCurrentDensity(offset.GetY());
        EXPECT_NEAR(result, expecte, EPSILON) << "Input value is: " << input;
    }
}

/**
 * @tc.name: getXTestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ClickEventAccessorTest, getYTestValidValues, TestSize.Level1)
{
    ASSERT_NE(accessor_->getY, nullptr);

    for (auto& [input, value, expecte] : testFixtureNumberDoubleRoundValues) {
        Offset offset = eventInfo_->GetLocalLocation();
        const auto convValue = Converter::Convert<double>(value);
        const auto newValue = PipelineBase::Vp2PxWithCurrentDensity(convValue);
        offset.SetY(newValue);
        eventInfo_->SetLocalLocation(offset);
        auto result = Converter::Convert<int>(accessor_->getY(peer_));
        result = static_cast<double>(result);
        EXPECT_NEAR(result, expecte, EPSILON) << "Input value is: " << input;
    }
}
}
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

#include <cmath>
#include <vector>
#include <tuple>

#include "accessor_test_base.h"
#include "accessor_test_fixtures.h"
#include "node_api.h"
#include "base/memory/ace_type.h"
#include "core/components_ng/gestures/recognizers/gesture_recognizer.h"
#include "core/interfaces/native/implementation/drag_event_peer.h"
#include "core/interfaces/native/implementation/unified_data_peer.h"
#include "core/interfaces/native/utility/reverse_converter.h"
#include "core/interfaces/native/utility/converter.h"
#include "frameworks/base/geometry/offset.h"
#include "frameworks/base/geometry/rect.h"
#include "frameworks/core/gestures/velocity.h"

namespace OHOS::Ace::NG {

using namespace testing;
using namespace testing::ext;
using namespace AccessorTestFixtures;

namespace GeneratedModifier {
    const GENERATED_ArkUIUnifiedDataAccessor* GetUnifiedDataAccessor();
}

namespace Converter {
    void AssignArkValue(Ark_UnifiedData& arkData, const RefPtr<UnifiedData>& data)
    {
        const auto peer = GeneratedModifier::GetUnifiedDataAccessor()->ctor();
        peer->unifiedData = data;
        arkData = peer;
    }
}

namespace {
    using namespace Converter;

    constexpr int32_t COUNTER_NUMBER_TEN_HANDLE = 10;

    class UnifiedDataMock : public Ace::UnifiedData {
    public:
        int64_t GetSize() override
        {
            return COUNTER_NUMBER_TEN_HANDLE;
        }
    };

    const std::vector<std::tuple<std::string, Ark_DragResult, DragRet>> testFixtureEnumDragResultValues {
        { "ARK_DRAG_RESULT_DRAG_SUCCESSFUL", ARK_DRAG_RESULT_DRAG_SUCCESSFUL, DragRet::DRAG_SUCCESS },
        { "ARK_DRAG_RESULT_DRAG_FAILED", ARK_DRAG_RESULT_DRAG_FAILED,  DragRet::DRAG_FAIL },
        { "ARK_DRAG_RESULT_DRAG_CANCELED", ARK_DRAG_RESULT_DRAG_CANCELED, DragRet::DRAG_CANCEL },
        { "ARK_DRAG_RESULT_DROP_ENABLED", ARK_DRAG_RESULT_DROP_ENABLED, DragRet::ENABLE_DROP },
        { "ARK_DRAG_RESULT_DROP_DISABLED", ARK_DRAG_RESULT_DROP_DISABLED, DragRet::DISABLE_DROP },
        { "ARK_DRAG_RESULT_INVALID", static_cast<Ark_DragResult>(-1), DragRet::DISABLE_DROP },
    };

    const std::vector<std::tuple<std::string, DragRet, Ark_DragResult>> testFixtureEnumArkDragResultValues {
        {  "ARK_DRAG_RESULT_DRAG_SUCCESSFUL", DragRet::DRAG_SUCCESS, ARK_DRAG_RESULT_DRAG_SUCCESSFUL },
        {  "ARK_DRAG_RESULT_DRAG_FAILED", DragRet::DRAG_FAIL, ARK_DRAG_RESULT_DRAG_FAILED },
        {  "ARK_DRAG_RESULT_DRAG_CANCELED", DragRet::DRAG_CANCEL, ARK_DRAG_RESULT_DRAG_CANCELED },
        {  "ARK_DRAG_RESULT_DROP_ENABLED", DragRet::ENABLE_DROP, ARK_DRAG_RESULT_DROP_ENABLED },
        {  "ARK_DRAG_RESULT_DROP_DISABLED", DragRet::DISABLE_DROP, ARK_DRAG_RESULT_DROP_DISABLED },
        {  "ARK_DRAG_RESULT_INVALID", static_cast<DragRet>(-1), static_cast<Ark_DragResult>(-1) },
    };
} // namespace

class DragEventAccessorTest
    : public AccessorTestBase<GENERATED_ArkUIDragEventAccessor,
        &GENERATED_ArkUIAccessors::getDragEventAccessor, DragEventPeer> {
public:
    void SetUp(void) override
    {
        AccessorTestBase::SetUp();
        dragEvent_ = AceType::MakeRefPtr<OHOS::Ace::DragEvent>();
        ASSERT_NE(dragEvent_, nullptr);
        peer_->dragInfo = dragEvent_;
    }
    RefPtr<OHOS::Ace::DragEvent> dragEvent_ = nullptr;
};

/**
 * @tc.name: GetWindowXTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(DragEventAccessorTest, GetWindowXTest, TestSize.Level1)
{
    for (auto& [input, value, expected] : testFixtureInt32WithNegativeValues) {
        dragEvent_->SetX(Convert<int32_t>(value));
        auto windowX = accessor_->getWindowX(peer_);
        EXPECT_EQ(Convert<int32_t>(windowX), expected) <<
            "Input value is: " << input << ", method: GetWindowXTest";
    }
}

/**
 * @tc.name: GetWindowYTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(DragEventAccessorTest, GetWindowYTest, TestSize.Level1)
{
    for (auto& [input, value, expected] : testFixtureInt32WithNegativeValues) {
        dragEvent_->SetY(Convert<int32_t>(value));
        auto windowY = accessor_->getWindowY(peer_);
        EXPECT_EQ(Convert<int32_t>(windowY), expected) <<
            "Input value is: " << input << ", method: GetWindowYTest";
    }
}

/**
 * @tc.name: GetXTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(DragEventAccessorTest, GetXTest, TestSize.Level1)
{
    for (auto& [input, value, expected] : testFixtureInt32WithNegativeValues) {
        dragEvent_->SetX(Convert<int32_t>(value));
        auto x = accessor_->getX(peer_);
        EXPECT_EQ(Convert<int32_t>(x), expected) <<
            "Input value is: " << input << ", method: GetXTest";
    }
}

/**
 * @tc.name: GetYTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(DragEventAccessorTest, GetYTest, TestSize.Level1)
{
    for (auto& [input, value, expected] : testFixtureInt32WithNegativeValues) {
        dragEvent_->SetY(Convert<int32_t>(value));
        auto y = accessor_->getY(peer_);
        EXPECT_EQ(Convert<int32_t>(y), expected) <<
            "Input value is: " << input << ", method: GetYTest";
    }
}

/**
 * @tc.name: SetResultTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(DragEventAccessorTest, SetResultTest, TestSize.Level1)
{
    for (auto& [input, value, expected] : testFixtureEnumDragResultValues) {
        accessor_->setResult(peer_, value);
        EXPECT_EQ(dragEvent_->GetResult(), expected) <<
            "Input value is: " << input << ", method: SetResult";
    }
}

/**
 * @tc.name: SetResultTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(DragEventAccessorTest, GetResultTest, TestSize.Level1)
{
    for (auto& [input, value, expected] : testFixtureEnumArkDragResultValues) {
        dragEvent_->SetResult(value);
        EXPECT_EQ(accessor_->getResult(peer_), expected) <<
            "Input value is: " << input << ", method: SetResult";
    }
}

/**
 * @tc.name: SetResultTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(DragEventAccessorTest, GetPreviewRectTest, TestSize.Level1)
{
    for (auto& [input, value, expected] : testFixtureLengthAnyValidValues) {
        Rect rect(value.ConvertToPx(), value.ConvertToPx(), value.ConvertToPx(), value.ConvertToPx());
        dragEvent_->SetPreviewRect(rect);
        auto arkRect = accessor_->getPreviewRect(peer_);
        CompareOptLength(arkRect.x, ArkValue<Opt_Length>(expected));
        CompareOptLength(arkRect.y, ArkValue<Opt_Length>(expected));
        CompareOptLength(arkRect.width, ArkValue<Opt_Length>(expected));
        CompareOptLength(arkRect.height, ArkValue<Opt_Length>(expected));
    }
}

/**
 * @tc.name: SetDataTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(DragEventAccessorTest, SetDataTest, TestSize.Level1)
{
    auto unifiedData = AceType::MakeRefPtr<UnifiedDataMock>();
    auto arkUnifiedData = ArkValue<Ark_UnifiedData>(unifiedData);
    accessor_->setData(peer_, arkUnifiedData);
    ASSERT_NE(dragEvent_->GetData(), nullptr);
    EXPECT_EQ(dragEvent_->GetData()->GetSize(), COUNTER_NUMBER_TEN_HANDLE) <<
        "Input value is: " << COUNTER_NUMBER_TEN_HANDLE << ", method: setData";
    auto unifiedDataPeer = arkUnifiedData;
    GeneratedModifier::GetUnifiedDataAccessor()->destroyPeer(unifiedDataPeer);
}

/**
 * @tc.name: GetDataTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(DragEventAccessorTest, GetDataTest, TestSize.Level1)
{
    auto unifiedData = AceType::MakeRefPtr<UnifiedDataMock>();
    auto arkUnifiedData = ArkValue<Ark_UnifiedData>(unifiedData);
    accessor_->setData(peer_, arkUnifiedData);
    auto getData = accessor_->getData(peer_);
    ASSERT_NE(getData, nullptr);
    auto dataPeer = getData;
    ASSERT_NE(dataPeer->unifiedData, nullptr);
    EXPECT_EQ(dataPeer->unifiedData->GetSize(), COUNTER_NUMBER_TEN_HANDLE) <<
        "Input value is: " << COUNTER_NUMBER_TEN_HANDLE << ", method: getData";
    auto unifiedDataPeer = arkUnifiedData;
    GeneratedModifier::GetUnifiedDataAccessor()->destroyPeer(unifiedDataPeer);
    GeneratedModifier::GetUnifiedDataAccessor()->destroyPeer(dataPeer);
}

/**
 * @tc.name: GetUseCustomDropAnimationTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(DragEventAccessorTest, GetUseCustomDropAnimationTest, TestSize.Level1)
{
    for (auto& [input, value, expected] : testFixtureBooleanValues) {
        dragEvent_->UseCustomAnimation(expected);
        auto useCustomDropAnimation = accessor_->getUseCustomDropAnimation(peer_);
        EXPECT_EQ(Convert<bool>(useCustomDropAnimation), expected) <<
            "Input value is: " << input << ", method: GetYTest";
    }
}

/**
 * @tc.name: SetUseCustomDropAnimationTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(DragEventAccessorTest, SetUseCustomDropAnimationTest, TestSize.Level1)
{
    for (auto& [input, value, expected] : testFixtureBooleanValues) {
        accessor_->setUseCustomDropAnimation(peer_, value);
        EXPECT_EQ(dragEvent_->IsUseCustomAnimation(), expected) <<
            "Input value is: " << input << ", method: GetYTest";
    }
}

/**
 * @tc.name: GetDisplayXTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(DragEventAccessorTest, GetDisplayXTest, TestSize.Level1)
{
    for (auto& [input, value, expected] : AccessorTestFixtures::testFixtureNumberFloatAnythingValidValues) {
        dragEvent_->SetScreenX(value);
        auto x = accessor_->getDisplayX(peer_);
        EXPECT_EQ(x, static_cast<int32_t>(Convert<int>(expected))) <<
            "Input value is: " << input << ", method: getDisplayX";
    }
}

/**
 * @tc.name: GetDisplayYTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(DragEventAccessorTest, GetDisplayYTest, TestSize.Level1)
{
    for (auto& [input, value, expected] : AccessorTestFixtures::testFixtureNumberFloatAnythingValidValues) {
        dragEvent_->SetScreenY(value);
        auto y = accessor_->getDisplayY(peer_);
        EXPECT_EQ(y, static_cast<int32_t>(Convert<int>(expected))) <<
            "Input value is: " << input << ", method: getDisplayY";
    }
}

/**
 * @tc.name: GetVelocityXTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(DragEventAccessorTest, GetVelocityXTest, TestSize.Level1)
{
    for (auto& [input, value, expected] : AccessorTestFixtures::testFixtureNumberFloatAnythingValidValues) {
        auto offset = Offset(value, 0.0);
        auto velocity = Velocity(offset);
        dragEvent_->SetVelocity(velocity);
        auto velocityX = accessor_->getVelocityX(peer_);
        EXPECT_EQ(velocityX, static_cast<int32_t>(Convert<int>(expected))) <<
            "Input value is: " << input << ", method: getVelocityX";
        auto arkVelocity = accessor_->getVelocity(peer_);
        EXPECT_EQ(arkVelocity, abs(static_cast<int32_t>(Convert<int>(expected)))) <<
            "Input value is: " << input << ", method: getVelocity";
    }
}

/**
 * @tc.name: GetVelocityYTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(DragEventAccessorTest, GetVelocityYTest, TestSize.Level1)
{
    for (auto& [input, value, expected] : AccessorTestFixtures::testFixtureNumberFloatAnythingValidValues) {
        auto offset = Offset(0.0, value);
        auto velocity = Velocity(offset);
        dragEvent_->SetVelocity(velocity);
        auto velocityY = accessor_->getVelocityY(peer_);
        EXPECT_EQ(velocityY, static_cast<int32_t>(Convert<int>(expected))) <<
            "Input value is: " << input << ", method: getVelocityY";
        auto arkVelocity = accessor_->getVelocity(peer_);
        EXPECT_EQ(arkVelocity, abs(static_cast<int32_t>(Convert<int>(expected)))) <<
            "Input value is: " << input << ", method: getVelocity";
    }
}

/**
 * @tc.name: GetVelocityTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(DragEventAccessorTest, GetVelocityTest, TestSize.Level1)
{
    for (auto& [input, value, expected] : AccessorTestFixtures::testFixtureVelocityValues) {
        auto offset = Offset(value, value);
        auto velocity = Velocity(offset);
        dragEvent_->SetVelocity(velocity);
        auto arkVelocity = accessor_->getVelocity(peer_);
        EXPECT_EQ(arkVelocity, abs(static_cast<int32_t>(Convert<int>(expected)))) <<
            "Input value is: " << input << ", method: getVelocity";
    }
}

/**
 * @tc.name: SetDragBehaviorTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(DragEventAccessorTest, SetDragBehaviorTest, TestSize.Level1)
{
    for (auto& [input, value, expected] : AccessorTestFixtures::testFixtureEnumDragBehaviorValues) {
        EXPECT_EQ(OHOS::Ace::DragBehavior::UNKNOWN, dragEvent_->GetDragBehavior());
        dragEvent_->GetDragBehavior();
        accessor_->setDragBehavior(peer_, value);
        EXPECT_EQ(expected, dragEvent_->GetDragBehavior()) <<
            "Input value is: " << input << ", method: setDragBehavior";
        dragEvent_->SetDragBehavior(OHOS::Ace::DragBehavior::UNKNOWN);
    }
}

/**
 * @tc.name: GetModifierKeyStateTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(DragEventAccessorTest, GetModifierKeyStateTest, TestSize.Level1)
{
    const std::vector<std::tuple<std::vector<std::string>, std::vector<KeyCode>, bool>> TEST_PLAN = {
        { {"ctrl"}, {KeyCode::KEY_CTRL_LEFT}, true },
        { {"ctrl"}, {KeyCode::KEY_CTRL_RIGHT}, true },
        { {"shift"}, {KeyCode::KEY_CTRL_LEFT}, false },
        { {"ctrl", "shift"}, {KeyCode::KEY_CTRL_LEFT, KeyCode::KEY_SHIFT_RIGHT}, true },
        { {"shift", "alt"}, {KeyCode::KEY_ALT_LEFT, KeyCode::KEY_SHIFT_RIGHT}, true },
        { {"shift", "ctrl", "alt"}, {KeyCode::KEY_CTRL_LEFT}, false },
        { {"fn"}, {KeyCode::KEY_CTRL_LEFT}, false }
    };

    for (auto& [param, value, expected] : TEST_PLAN) {
        Converter::ArkArrayHolder<Array_String> stringHolder(param);
        Array_String stringArrayValues = stringHolder.ArkValue();
        dragEvent_->SetPressedKeyCodes(value);
        auto result = accessor_->getModifierKeyState(peer_, &stringArrayValues);
        EXPECT_EQ(Converter::Convert<bool>(result), expected);
    }
}

} // namespace OHOS::Ace::NG
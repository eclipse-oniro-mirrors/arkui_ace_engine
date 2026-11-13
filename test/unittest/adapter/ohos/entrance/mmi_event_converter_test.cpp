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

#include "gtest/gtest.h"
#define private public
#define protected public
#include "adapter/ohos/entrance/mmi_event_convertor.h"
#include "base/input_manager/input_manager.h"
#include "base/utils/system_properties.h"
#include "core/event/key_event.h"
#include "pointer_event.h"
#include "test/mock/frameworks/base/thread/mock_task_executor.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace {
bool InputManager::IsAgentEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    return pointerEvent && pointerEvent->HasFlag(MMI::InputEvent::EVENT_FLAG_CONTROLLER);
}
} // namespace OHOS::Ace

namespace OHOS::Ace::Platform {
namespace {
constexpr int32_t AGENT_CALLING_UID = 20010041;
constexpr uint64_t SAF_SESSION_ID = 0xFEDCBA9800000042ULL;
} // namespace

class MMIEventConvertorTest : public testing::Test {
public:
    static void SetUpTestSuite() {};
    static void TearDownTestSuite() {};
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.name: MMIConvertorTest_001
 * @tc.desc: Test MMIEventConvertorTest
 * @tc.type: FUNC
 */
HWTEST_F(MMIEventConvertorTest, MMIConvertorTest_001, TestSize.Level1)
{
    auto pointerEvent = MMI::PointerEvent::Create();
    MMI::PointerEvent::PointerItem pointerItem;
    pointerItem.SetWindowXPos(0.0001);
    pointerItem.SetWindowYPos(100.2);
    pointerItem.SetWindowX(0);
    pointerItem.SetWindowY(100);
    pointerItem.SetPointerId(0);
    pointerEvent->SetPointerId(0);
    pointerEvent->SetSourceType(MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN);
    pointerEvent->UpdatePointerItem(0, pointerItem);
    auto res = ConvertTouchEvent(pointerEvent);
    ASSERT_EQ(res.x, pointerItem.GetWindowX());
    pointerEvent->SetSourceType(MMI::PointerEvent::SOURCE_TYPE_JOYSTICK);
    res = ConvertTouchEvent(pointerEvent);
    pointerEvent->SetSourceType(MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN);
    res = ConvertTouchEvent(pointerEvent);
    pointerItem.SetExtension(OHOS::MMI::PointerEvent::PointerItemExtension::PREDICT_WINDOW_X, 100.123);
    pointerEvent->UpdatePointerItem(0, pointerItem);
    res = ConvertTouchEvent(pointerEvent);
    pointerItem.SetExtension(OHOS::MMI::PointerEvent::PointerItemExtension::PREDICT_WINDOW_Y, 200.456);
    pointerEvent->UpdatePointerItem(0, pointerItem);
    res = ConvertTouchEvent(pointerEvent);
}
/**
 * @tc.name: AgentPointerIdentity001
 * @tc.desc: Controller input carries UID and the full session; clearing its flag restores manual input.
 * @tc.type: FUNC
 */
HWTEST_F(MMIEventConvertorTest, AgentPointerIdentity001, TestSize.Level1)
{
    auto pointer = MMI::PointerEvent::Create();
    ASSERT_NE(pointer, nullptr);
    MMI::PointerEvent::PointerItem item;
    item.SetPointerId(0);
    pointer->SetPointerId(0);
    pointer->UpdatePointerItem(0, item);
    pointer->SetSourceType(MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN);
    pointer->AddFlag(MMI::InputEvent::EVENT_FLAG_CONTROLLER);
    pointer->SetInjectorInfo(AGENT_CALLING_UID, 0);
    pointer->SetInjectorOptionalData(SAF_SESSION_ID);
    auto touch = ConvertTouchEvent(pointer);
    EXPECT_TRUE(touch.isAgent);
    EXPECT_EQ(touch.callingUid, AGENT_CALLING_UID);
    EXPECT_TRUE(touch.hasSafSessionId);
    EXPECT_EQ(touch.safSessionId, SAF_SESSION_ID);
    pointer->ClearFlag(MMI::InputEvent::EVENT_FLAG_CONTROLLER);
    touch = ConvertTouchEvent(pointer);
    EXPECT_FALSE(touch.isAgent);
    EXPECT_EQ(touch.callingUid, -1);
    EXPECT_FALSE(touch.hasSafSessionId);
}

/**
 * @tc.name: AgentKeyCancelConversion001
 * @tc.desc: Agent Enter cancellation reaches cleanup while other inputs retain their action mapping.
 * @tc.type: FUNC
 */
HWTEST_F(MMIEventConvertorTest, AgentKeyCancelConversion001, TestSize.Level1)
{
    auto key = MMI::KeyEvent::Create();
    ASSERT_NE(key, nullptr);
    key->SetInjectorInfo(AGENT_CALLING_UID, 0);
    key->SetInjectorOptionalData(SAF_SESSION_ID);
    KeyEvent event;
    for (auto code : { MMI::KeyEvent::KEYCODE_ENTER, MMI::KeyEvent::KEYCODE_NUMPAD_ENTER,
             MMI::KeyEvent::KEYCODE_A }) {
        key->SetKeyCode(code);
        key->SetKeyAction(MMI::KeyEvent::KEY_ACTION_CANCEL);
        key->ClearFlag(MMI::InputEvent::EVENT_FLAG_CONTROLLER);
        ConvertKeyEvent(key, event);
        EXPECT_EQ(event.action, KeyAction::UNKNOWN);
        key->AddFlag(MMI::InputEvent::EVENT_FLAG_CONTROLLER);
        ConvertKeyEvent(key, event);
        EXPECT_EQ(event.action, code == MMI::KeyEvent::KEYCODE_A ? KeyAction::UNKNOWN : KeyAction::CANCEL);
        key->SetKeyAction(MMI::KeyEvent::KEY_ACTION_DOWN);
        ConvertKeyEvent(key, event);
        EXPECT_EQ(event.action, KeyAction::DOWN);
        key->SetKeyAction(MMI::KeyEvent::KEY_ACTION_UP);
        ConvertKeyEvent(key, event);
        EXPECT_EQ(event.action, KeyAction::UP);
    }
}
} // namespace OHOS::Ace::Platform

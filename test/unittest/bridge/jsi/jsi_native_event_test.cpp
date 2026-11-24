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

#include "gtest/gtest.h"

#include "bridge/declarative_frontend/engine/jsi/jsi_native_event.h"
#include "core/components_ng/gestures/base_gesture_event.h"
#include "core/event/key_event.h"
#include "core/event/touch_event.h"

using namespace testing::ext;

namespace OHOS::Ace::Framework {
class JsiNativeEventTest : public testing::Test {
public:
    void SetUp() override
    {
        panda::RuntimeOption option;
        option.SetLogLevel(panda::RuntimeOption::LOG_LEVEL::ERROR);
        vm_ = panda::JSNApi::CreateJSVM(option);
        ASSERT_NE(vm_, nullptr);
    }

    void TearDown() override
    {
        if (vm_) {
            panda::JSNApi::DestroyJSVM(vm_);
            vm_ = nullptr;
        }
    }

protected:
    void CollectGarbage()
    {
        panda::JSNApi::TriggerGC(vm_, panda::JSNApi::TRIGGER_GC_TYPE::FULL_GC);
    }

    panda::ecmascript::EcmaVM* vm_ = nullptr;
};

/**
 * @tc.name: Bind_001
 * @tc.desc: A saved gesture event retains its data and derived type after native dispatch ends.
 * @tc.type: FUNC
 */
HWTEST_F(JsiNativeEventTest, Bind_001, TestSize.Level1)
{
    std::weak_ptr<BaseGestureEvent> lifetime;
    {
        panda::LocalScope scope(vm_);
        std::shared_ptr<BaseGestureEvent> event = std::make_shared<TapGestureEvent>();
        event->SetDeviceId(42);
        event->SetPressedKeyCodes({ KeyCode::KEY_CTRL_LEFT });
        lifetime = event;
        auto object = panda::ObjectRef::New(vm_);
        JsiNativeEvent::Bind(vm_, object, event);
        event.reset();
        CollectGarbage();

        ASSERT_FALSE(lifetime.expired());
        auto* saved = static_cast<BaseGestureEvent*>(object->GetNativePointerField(vm_, 0));
        ASSERT_NE(saved, nullptr);
        EXPECT_NE(TypeInfoHelper::DynamicCast<TapGestureEvent>(saved), nullptr);
        EXPECT_EQ(saved->GetDeviceId(), 42);
        ASSERT_EQ(saved->GetPressedKeyCodes().size(), 1);
        EXPECT_EQ(saved->GetPressedKeyCodes().front(), KeyCode::KEY_CTRL_LEFT);
    }
    CollectGarbage();
    EXPECT_TRUE(lifetime.expired());
}

/**
 * @tc.name: Bind_002
 * @tc.desc: Keeping only tapLocation retains the event after the parent JS object is collected.
 * @tc.type: FUNC
 */
HWTEST_F(JsiNativeEventTest, Bind_002, TestSize.Level1)
{
    std::weak_ptr<BaseGestureEvent> lifetime;
    panda::CopyableGlobal<panda::ObjectRef> savedLocation;
    {
        panda::LocalScope scope(vm_);
        std::shared_ptr<BaseGestureEvent> event = std::make_shared<TapGestureEvent>();
        FingerInfo finger;
        finger.localLocation_ = Offset(12.0, 34.0);
        event->SetFingerList({ finger });
        lifetime = event;
        auto parent = panda::ObjectRef::New(vm_);
        auto location = panda::ObjectRef::New(vm_);
        JsiNativeEvent::Bind(vm_, parent, event);
        JsiNativeEvent::Bind(vm_, location, event);
        parent->Set(vm_, panda::StringRef::NewFromUtf8(vm_, "tapLocation"), location);
        savedLocation = panda::CopyableGlobal<panda::ObjectRef>(vm_, location);
    }
    CollectGarbage();
    ASSERT_FALSE(lifetime.expired());
    {
        panda::LocalScope scope(vm_);
        auto* saved = static_cast<BaseGestureEvent*>(savedLocation.ToLocal()->GetNativePointerField(vm_, 0));
        ASSERT_NE(saved, nullptr);
        ASSERT_EQ(saved->GetFingerList().size(), 1);
        EXPECT_EQ(saved->GetFingerList().front().localLocation_, Offset(12.0, 34.0));
    }
    savedLocation.Reset();
    CollectGarbage();
    EXPECT_TRUE(lifetime.expired());
}

/**
 * @tc.name: Bind_003
 * @tc.desc: Collecting the location first does not invalidate the independently retained event.
 * @tc.type: FUNC
 */
HWTEST_F(JsiNativeEventTest, Bind_003, TestSize.Level1)
{
    std::weak_ptr<BaseGestureEvent> lifetime;
    panda::CopyableGlobal<panda::ObjectRef> savedEvent;
    {
        panda::LocalScope scope(vm_);
        auto event = std::make_shared<BaseGestureEvent>();
        event->SetDeviceId(7);
        lifetime = event;
        auto object = panda::ObjectRef::New(vm_);
        auto location = panda::ObjectRef::New(vm_);
        JsiNativeEvent::Bind(vm_, object, event);
        JsiNativeEvent::Bind(vm_, location, event);
        savedEvent = panda::CopyableGlobal<panda::ObjectRef>(vm_, object);
    }
    CollectGarbage();
    ASSERT_FALSE(lifetime.expired());
    {
        panda::LocalScope scope(vm_);
        auto* saved = static_cast<BaseGestureEvent*>(savedEvent.ToLocal()->GetNativePointerField(vm_, 0));
        ASSERT_NE(saved, nullptr);
        EXPECT_EQ(saved->GetDeviceId(), 7);
    }
    savedEvent.Reset();
    CollectGarbage();
    EXPECT_TRUE(lifetime.expired());
}

/**
 * @tc.name: Bind_004
 * @tc.desc: A JS-owned touch snapshot survives its stack source and supports propagation flags.
 * @tc.type: FUNC
 */
HWTEST_F(JsiNativeEventTest, Bind_004, TestSize.Level1)
{
    std::weak_ptr<TouchEventInfo> lifetime;
    {
        panda::LocalScope scope(vm_);
        auto object = panda::ObjectRef::New(vm_);
        {
            TouchEventInfo original("touchEvent");
            original.SetDeviceId(9);
            auto snapshot = std::make_shared<TouchEventInfo>(original);
            lifetime = snapshot;
            JsiNativeEvent::Bind(vm_, object, snapshot);
            snapshot->SetStopPropagation(true);
            snapshot->SetPreventDefault(true);
            EXPECT_FALSE(original.IsStopPropagation());
            EXPECT_FALSE(original.IsPreventDefault());
            original.SetDeviceId(10);
        }
        CollectGarbage();
        ASSERT_FALSE(lifetime.expired());
        auto* saved = static_cast<TouchEventInfo*>(object->GetNativePointerField(vm_, 0));
        ASSERT_NE(saved, nullptr);
        EXPECT_EQ(saved->GetDeviceId(), 9);
        EXPECT_TRUE(saved->IsStopPropagation());
        EXPECT_TRUE(saved->IsPreventDefault());
    }
    CollectGarbage();
    EXPECT_TRUE(lifetime.expired());
}
} // namespace OHOS::Ace::Framework

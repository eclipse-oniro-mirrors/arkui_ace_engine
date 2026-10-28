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

#include "event_test_min.h"

#include "core/components/panel/sliding_events.h"
#include "core/components_ng/pattern/swiper/swiper_change_event.h"
#include "core/components_v2/grid/grid_event.h"
#include "core/event/ace_events.h"

using namespace testing::ext;

namespace OHOS::Ace {
namespace {
template<typename T>
void VerifyEventType(T& event, const std::string& eventName)
{
    BaseEventInfo* base = &event;
    TypeInfoBase* typeInfo = base;
    auto* jsonEvent = TypeInfoHelper::DynamicCast<JSONStringifiableEventInfo>(base);
    ASSERT_NE(jsonEvent, nullptr);
    EXPECT_EQ(TypeInfoHelper::DynamicCast<T>(base), &event);
    EXPECT_EQ(TypeInfoHelper::DynamicCast<T>(typeInfo), &event);
    EXPECT_EQ(TypeInfoHelper::DynamicCast<BaseEventInfo>(jsonEvent), base);
    EXPECT_EQ(TypeInfoHelper::DynamicCast<JSONStringifiableEventInfo>(typeInfo), jsonEvent);
    EXPECT_EQ(base->GetType(), eventName);
    EXPECT_EQ(TypeInfoHelper::TypeId(base), T::TypeId());

    const BaseEventInfo* constBase = base;
    EXPECT_EQ(TypeInfoHelper::DynamicCast<T>(constBase), &event);
    EXPECT_EQ(TypeInfoHelper::DynamicCast<JSONStringifiableEventInfo>(constBase), jsonEvent);
}
} // namespace

class EventTest : public EventTestBase {};

/**
 * @tc.name: DynamicCast_001
 * @tc.desc: Grid events retain their identity and serialization through the event base classes.
 * @tc.type: FUNC
 */
HWTEST_F(EventTest, DynamicCast_001, TestSize.Level1)
{
    V2::GridEventInfo event(7);
    VerifyEventType(event, "grid");
    BaseEventInfo* base = &event;
    auto* jsonEvent = TypeInfoHelper::DynamicCast<JSONStringifiableEventInfo>(base);
    ASSERT_NE(jsonEvent, nullptr);
    EXPECT_EQ(jsonEvent->ToJSONString(), R"("grid",{"first":7},null)");
    EXPECT_EQ(TypeInfoHelper::DynamicCast<SwiperChangeEvent>(base), nullptr);
}

/**
 * @tc.name: DynamicCast_002
 * @tc.desc: Swiper events retain their identity and serialization through the event base classes.
 * @tc.type: FUNC
 */
HWTEST_F(EventTest, DynamicCast_002, TestSize.Level1)
{
    SwiperChangeEvent event(3);
    VerifyEventType(event, "SwiperChangeEvent");
    const BaseEventInfo* base = &event;
    const auto* jsonEvent = TypeInfoHelper::DynamicCast<JSONStringifiableEventInfo>(base);
    ASSERT_NE(jsonEvent, nullptr);
    EXPECT_EQ(jsonEvent->ToJSONString(), R"("change",{"index":3},null)");
}

/**
 * @tc.name: DynamicCast_003
 * @tc.desc: Panel events retain their identity and size-change payload after conversion.
 * @tc.type: FUNC
 */
HWTEST_F(EventTest, DynamicCast_003, TestSize.Level1)
{
    SlidingPanelSizeChangeEvent event(PanelMode::FULL, 100.0, 200.0);
    VerifyEventType(event, "SlidingPanelSizeChangeEvent");
    BaseEventInfo* base = &event;
    auto* jsonEvent = TypeInfoHelper::DynamicCast<JSONStringifiableEventInfo>(base);
    ASSERT_NE(jsonEvent, nullptr);
    EXPECT_EQ(jsonEvent->ToJSONString(), R"("sizechange",{"size":{"height":200,"width":100},"mode":"full"})");
}

/**
 * @tc.name: DynamicCast_004
 * @tc.desc: Ordinary events and null pointers do not advertise JSON serialization support.
 * @tc.type: FUNC
 */
HWTEST_F(EventTest, DynamicCast_004, TestSize.Level1)
{
    BaseEventInfo event("ordinary");
    EXPECT_EQ(TypeInfoHelper::DynamicCast<JSONStringifiableEventInfo>(&event), nullptr);
    const BaseEventInfo* constEvent = &event;
    EXPECT_EQ(TypeInfoHelper::DynamicCast<JSONStringifiableEventInfo>(constEvent), nullptr);
    BaseEventInfo* nullEvent = nullptr;
    EXPECT_EQ(TypeInfoHelper::DynamicCast<JSONStringifiableEventInfo>(nullEvent), nullptr);
    const BaseEventInfo* constNullEvent = nullptr;
    EXPECT_EQ(TypeInfoHelper::DynamicCast<JSONStringifiableEventInfo>(constNullEvent), nullptr);
}

/**
 * @tc.name: SetStopPropagation_001
 * @tc.desc: JSON-capable events share propagation state with their BaseEventInfo view.
 * @tc.type: FUNC
 */
HWTEST_F(EventTest, SetStopPropagation_001, TestSize.Level1)
{
    V2::GridEventInfo event(0);
    BaseEventInfo* base = &event;
    auto* jsonEvent = TypeInfoHelper::DynamicCast<JSONStringifiableEventInfo>(base);
    ASSERT_NE(jsonEvent, nullptr);
    EXPECT_FALSE(base->IsStopPropagation());
    jsonEvent->SetStopPropagation(true);
    EXPECT_TRUE(base->IsStopPropagation());
    base->SetStopPropagation(false);
    EXPECT_FALSE(jsonEvent->IsStopPropagation());
}
} // namespace OHOS::Ace

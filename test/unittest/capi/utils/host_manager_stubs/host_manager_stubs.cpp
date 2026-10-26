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

// Implementations backing the host-test framework stand-ins used by
// interaction_observer_manager_test.cpp: a registry-backed Container and a
// test-controllable GetSysTimestamp clock (the real time_util.h declaration
// is kept; only the definition is supplied, plus a test-only
// SetSysTimestamp control point, so the drag-start duration field can be
// driven deterministically).
#include <map>

#include "base/utils/time_util.h"
#include "core/common/container.h"
#include "core/event/touch_event.h"
#include "host_manager_stubs.h"

namespace OHOS::Ace {
namespace {
std::map<int32_t, RefPtr<Container>>& StubRegistry()
{
    static std::map<int32_t, RefPtr<Container>> registry;
    return registry;
}
} // namespace

RefPtr<Container> Container::GetContainer(int32_t instanceId)
{
    auto& registry = StubRegistry();
    auto it = registry.find(instanceId);
    return it == registry.end() ? RefPtr<Container>() : it->second;
}

void Container::PutStubContainer(int32_t instanceId, const RefPtr<Container>& container)
{
    StubRegistry()[instanceId] = container;
}

void Container::ClearStubContainers()
{
    StubRegistry().clear();
}

// Test-controllable clock, in nanoseconds like the real one. Tests drive it
// only through SetSysTimestamp; see interaction_observer_manager_test.cpp.
static int64_t g_stubSysTimestampNanos = 1000000000;

int64_t GetSysTimestamp()
{
    return g_stubSysTimestampNanos;
}

void SetSysTimestamp(int64_t nanoseconds)
{
    g_stubSysTimestampNanos = nanoseconds;
}

// Link-time mirror of TouchEvent::GetScreenOffset (the real one-liner in
// core/event/touch_event.cpp) so the manager's TOUCH JSON path links without
// the heavy real TU. The public screenX/screenY members carry the value.
Offset TouchEvent::GetScreenOffset() const
{
    return Offset(screenX, screenY);
}

// Link-time mirror of TouchEvent::SetId (core/event/touch_event.cpp).
TouchEvent& TouchEvent::SetId(int32_t id)
{
    this->id = id;
    return *this;
}

// Host-test stand-in for the ref-count death-detection debug hook (the real
// implementation lives outside the harness TU set); a no-op keeps Claim<T>
// linkable.
void Referenced::OnDetectedClaimDeathObj(bool) {}

// Host-test stand-in for the lifecycle-check debug hook; same rationale.
void LifeCycleCheckable::OnDetectedObjDestroyInUse() {}
} // namespace OHOS::Ace

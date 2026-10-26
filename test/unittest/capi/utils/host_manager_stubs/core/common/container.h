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

// Host-test stand-in for core/common/container.h. It shadows the real header
// through include-path order (see interaction_observer_manager_test.cpp) and
// provides exactly the surface UIInfoCollectionManager consumes: instance
// lookup for registration validation and the bundle name for the drag-start
// hostName field. The registry is test-controlled.
#ifndef FOUNDATION_ACE_TEST_CAPI_HOST_MANAGER_STUBS_CONTAINER_H
#define FOUNDATION_ACE_TEST_CAPI_HOST_MANAGER_STUBS_CONTAINER_H

#include <cstdint>
#include <string>

#include "ui/base/ace_type.h"

namespace OHOS::Ace {
class Container : public AceType {
public:
    // Registry-backed instance lookup used by RegisterObserver validation
    // and GetHostName. An unknown instance yields a null RefPtr.
    static RefPtr<Container> GetContainer(int32_t instanceId);
    static void PutStubContainer(int32_t instanceId, const RefPtr<Container>& container);
    static void ClearStubContainers();

    const std::string& GetBundleName() const { return bundleName_; }
    void SetBundleName(std::string bundleName) { bundleName_ = std::move(bundleName); }

private:
    std::string bundleName_;
};
} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_TEST_CAPI_HOST_MANAGER_STUBS_CONTAINER_H

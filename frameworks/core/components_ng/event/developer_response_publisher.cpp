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

#include "core/components_ng/event/developer_response_publisher.h"

#include "base/log/log_wrapper.h"

namespace OHOS::Ace::NG {

DeveloperResponsePublisher* DeveloperResponsePublisher::GetInstance()
{
    static DeveloperResponsePublisher instance;
    return &instance;
}

void DeveloperResponsePublisher::SetObserver(DeveloperResponseObserver* observer)
{
    observer_.store(observer, std::memory_order_release);
}

void DeveloperResponsePublisher::PublishClick(const DeveloperResponseCandidate& candidate)
{
    auto observer = observer_.load(std::memory_order_acquire);
    if (!observer) {
        return;
    }
    observer->OnDeveloperResponse(candidate);
}

void DeveloperResponsePublisher::PublishTouch(const DeveloperResponseCandidate& candidate)
{
    auto observer = observer_.load(std::memory_order_acquire);
    if (!observer) {
        return;
    }
    observer->OnDeveloperResponse(candidate);
}

} // namespace OHOS::Ace::NG

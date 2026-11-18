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

#ifndef FOUNDATION_ACE_ACE_ENGINE_ADAPTER_OHOS_CAPABILITY_INTERACTION_STOP_DRAG_LISTENER_IMPL_H
#define FOUNDATION_ACE_ACE_ENGINE_ADAPTER_OHOS_CAPABILITY_INTERACTION_STOP_DRAG_LISTENER_IMPL_H

#include <functional>
#include <memory>
#include <mutex>

#include "i_stop_drag_listener.h"

#include "core/common/ace_engine.h"
#include "core/common/container.h"
#include "core/common/container_scope.h"

using StopDragEndingCallback = std::function<void()>;

namespace OHOS::Ace {
class StopDragListenerImpl : public OHOS::Msdp::DeviceStatus::IStopDragListener {
public:
    StopDragListenerImpl() = default;
    explicit StopDragListenerImpl(StopDragEndingCallback callback, int32_t instanceId = -1)
        : callback_(std::move(callback)),
          instanceId_(instanceId >= 0 ? instanceId : ContainerScope::CurrentId())
    {}

    ~StopDragListenerImpl() override
    {
        if (callback_ == nullptr) {
            return;
        }
        auto container = AceEngine::Get().GetContainer(instanceId_);
        auto taskExecutor = container != nullptr ? container->GetTaskExecutor() : nullptr;
        if (!taskExecutor || taskExecutor->WillRunOnCurrentThread(TaskExecutor::TaskType::JS)) {
            return;
        }
        auto mutex = std::make_shared<std::mutex>();
        std::lock_guard lock(*mutex);
        taskExecutor->PostTask(
            [callback = callback_, mutex]() mutable {
                std::lock_guard lock(*mutex);
            },
            TaskExecutor::TaskType::JS, "ArkUIStopDragCallbackRelease", PriorityType::VIP);
        callback_ = nullptr;
    }

    void OnDragEndMessage() override
    {
        if (callback_ != nullptr) {
            callback_();
        }
    }

private:
    StopDragEndingCallback callback_;
    int32_t instanceId_ = -1;
};
} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_ACE_ENGINE_ADAPTER_OHOS_CAPABILITY_INTERACTION_STOP_DRAG_LISTENER_IMPL_H

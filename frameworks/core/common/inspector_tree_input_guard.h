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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMMON_INSPECTOR_TREE_INPUT_GUARD_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMMON_INSPECTOR_TREE_INPUT_GUARD_H

#include <cstdint>
#include <functional>
#include <utility>

#include "base/utils/macros.h"

namespace OHOS::Ace {
// Scoped to one synchronous UISession traversal. Other dump callers are unaffected.
class ACE_FORCE_EXPORT InspectorTreeInputGuard final {
public:
    InspectorTreeInputGuard(uint32_t interval, std::function<bool()> hasPendingInput)
        : enabled_(interval != DISABLED_INTERVAL), interval_(interval),
          hasPendingInput_(std::move(hasPendingInput)), previous_(current_)
    {
        current_ = this;
        if (enabled_) {
            CheckNow();
        }
    }

    ~InspectorTreeInputGuard()
    {
        current_ = previous_;
    }

    InspectorTreeInputGuard(const InspectorTreeInputGuard&) = delete;
    InspectorTreeInputGuard& operator=(const InspectorTreeInputGuard&) = delete;

    static bool Checkpoint()
    {
        if (!current_) {
            return false;
        }
        if (current_->interrupted_) {
            return true;
        }
        ++current_->totalVisited_;
        if (!current_->enabled_) {
            return false;
        }
        if (++current_->visited_ >= current_->interval_) {
            current_->visited_ = 0;
            current_->CheckNow();
        }
        return current_->interrupted_;
    }

    static bool IsInterrupted()
    {
        return current_ && current_->interrupted_;
    }

    uint64_t GetVisitedNodes() const
    {
        return totalVisited_;
    }

    bool CheckNow()
    {
        if (!enabled_) {
            return false;
        }
        interrupted_ = interrupted_ || (hasPendingInput_ && hasPendingInput_());
        return interrupted_;
    }

private:
    static constexpr uint32_t DISABLED_INTERVAL = UINT32_MAX;
    inline static thread_local InspectorTreeInputGuard* current_ = nullptr;
    bool enabled_ = false;
    uint32_t interval_ = 0;
    uint32_t visited_ = 0;
    uint64_t totalVisited_ = 0;
    bool interrupted_ = false;
    std::function<bool()> hasPendingInput_;
    InspectorTreeInputGuard* previous_;
};
} // namespace OHOS::Ace
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMMON_INSPECTOR_TREE_INPUT_GUARD_H

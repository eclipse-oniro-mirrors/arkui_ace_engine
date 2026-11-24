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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMMON_INSPECTOR_TREE_UI_STATE_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMMON_INSPECTOR_TREE_UI_STATE_H

#include <atomic>
#include <cstdint>
#include <functional>
#include <set>
#include <utility>

#include "base/utils/macros.h"
#include "core/common/inspector_tree_activity.h"

namespace OHOS::Ace {
// One tracker per pipeline. Activity mutations belong to its UI thread; IPC threads
// only read the process-wide per-activity atomic snapshot. Reporting subscriptions do not own it.
class ACE_FORCE_EXPORT InspectorTreeUiState final {
public:
    InspectorTreeUiState() = default;
    ~InspectorTreeUiState()
    {
        Reset();
    }

    InspectorTreeUiState(const InspectorTreeUiState&) = delete;
    InspectorTreeUiState& operator=(const InspectorTreeUiState&) = delete;

    void Start(InspectorTreeActivity activity, int32_t nodeId)
    {
        bool wasActive = HasActivity(activity);
        activities_.emplace(activity, nodeId);
        if (!wasActive) {
            activityCounts_[static_cast<size_t>(activity)].fetch_add(1);
        }
    }

    void Stop(InspectorTreeActivity activity, int32_t nodeId)
    {
        if (activities_.erase({ activity, nodeId }) == 0) {
            return;
        }
        if (!HasActivity(activity)) {
            activityCounts_[static_cast<size_t>(activity)].fetch_sub(1);
        }
    }

    void RemoveNode(int32_t nodeId)
    {
        Stop(InspectorTreeActivity::SCROLL, nodeId);
        Stop(InspectorTreeActivity::SWIPER_SCROLL, nodeId);
        Stop(InspectorTreeActivity::SWIPER_TRANSITION, nodeId);
        Stop(InspectorTreeActivity::PAGE_TRANSITION, nodeId);
    }

    void Reset()
    {
        for (size_t i = 0; i < ACTIVITY_COUNT; i++) {
            if (HasActivity(static_cast<InspectorTreeActivity>(i))) {
                activityCounts_[i].fetch_sub(1);
            }
        }
        activities_.clear();
    }

    // Returns the mutually exclusive UI-activity error for an incoming collection request,
    // or nullptr when idle. Priority: page transition > swiper animation > scrolling.
    static const char* GetActiveError()
    {
        if (activityCounts_[static_cast<size_t>(InspectorTreeActivity::PAGE_TRANSITION)].load() != 0) {
            return "PAGE_TRANSITIONING";
        }
        if (activityCounts_[static_cast<size_t>(InspectorTreeActivity::SWIPER_TRANSITION)].load() != 0 ||
            activityCounts_[static_cast<size_t>(InspectorTreeActivity::SWIPER_SCROLL)].load() != 0) {
            return "SWIPER_ANIMATING";
        }
        if (activityCounts_[static_cast<size_t>(InspectorTreeActivity::SCROLL)].load() != 0) {
            return "SCROLLING";
        }
        return nullptr;
    }

    // Called on the IPC thread, before invoking anything that posts collection to UI.
    static void DispatchCollection(
        const std::function<void()>& postCollection, const std::function<void(const char*)>& report)
    {
        const char* error = GetActiveError();
        if (error != nullptr) {
            report(error);
            return;
        }
        postCollection();
    }

private:
    static constexpr size_t ACTIVITY_COUNT = 4;
    inline static std::atomic<uint32_t> activityCounts_[ACTIVITY_COUNT] {};
    std::set<std::pair<InspectorTreeActivity, int32_t>> activities_;

    bool HasActivity(InspectorTreeActivity activity) const
    {
        auto it = activities_.lower_bound({ activity, INT32_MIN });
        return it != activities_.end() && it->first == activity;
    }
};
} // namespace OHOS::Ace
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMMON_INSPECTOR_TREE_UI_STATE_H

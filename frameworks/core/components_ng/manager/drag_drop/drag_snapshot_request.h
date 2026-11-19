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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_DRAG_DROP_DRAG_SNAPSHOT_REQUEST_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_DRAG_DROP_DRAG_SNAPSHOT_REQUEST_H

#include <functional>
#include <utility>

#include "base/image/pixel_map.h"
#include "base/utils/noncopyable.h"

namespace OHOS::Ace::NG {
// Owned and consumed on the UI thread. External snapshot callbacks must only keep weak references.
class DragSnapshotRequest {
public:
    using Callback = std::function<void(const RefPtr<PixelMap>&)>;

    explicit DragSnapshotRequest(Callback callback) : callback_(std::move(callback)) {}
    ~DragSnapshotRequest()
    {
        Cancel();
    }

    void SetCleanup(std::function<void()> cleanup)
    {
        if (!active_) {
            if (cleanup) {
                cleanup();
            }
            return;
        }
        cleanup_ = std::move(cleanup);
    }

    void Complete(const RefPtr<PixelMap>& pixelMap)
    {
        if (!active_) {
            return;
        }
        active_ = false;
        auto callback = std::move(callback_);
        callback_ = nullptr;
        Cleanup();
        if (callback) {
            callback(pixelMap);
        }
    }

    void Cancel()
    {
        active_ = false;
        callback_ = nullptr;
        Cleanup();
    }

private:
    void Cleanup()
    {
        auto cleanup = std::move(cleanup_);
        cleanup_ = nullptr;
        if (cleanup) {
            cleanup();
        }
    }

    bool active_ = true;
    Callback callback_;
    std::function<void()> cleanup_;

    ACE_DISALLOW_COPY_AND_MOVE(DragSnapshotRequest);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_DRAG_DROP_DRAG_SNAPSHOT_REQUEST_H

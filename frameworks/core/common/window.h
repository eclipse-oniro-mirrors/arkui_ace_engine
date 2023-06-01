/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMMON_WINDOW_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMMON_WINDOW_H

#include <memory>

#include "base/geometry/ng/rect_t.h"
#include "base/thread/task_executor.h"
#include "base/utils/macros.h"
#include "base/utils/noncopyable.h"
#include "core/common/ace_page.h"
#include "core/common/platform_window.h"

namespace OHOS::Rosen {
class RSUIDirector;
}

namespace OHOS::Ace {

namespace NG {
class FrameNode;
} // namespace NG
class ACE_EXPORT Window {
public:
    Window() = default;
    explicit Window(std::unique_ptr<PlatformWindow> platformWindow);
    virtual ~Window() = default;

    uint32_t GetWindowId() const
    {
        return windowId_;
    }

    const std::string& GetWindowName() const
    {
        return windowName_;
    }

    void SetWindowRect(Rect rect)
    {
        windowRect_ = rect;
    }

    Rect GetWindowRect() const
    {
        return windowRect_;
    }

    bool IsDecorEnable() const
    {
        return false;
    }

    bool IsFocused() const
    {
        return true;
    }

    virtual void RequestFrame();

    virtual void SetTaskExecutor(const RefPtr<TaskExecutor>& taskExecutor) {}

    virtual void SetInstanceId(int32_t instanceId) {}

    virtual void Init() {}

    virtual void Destroy()
    {
        platformWindow_->Destroy();
        platformWindow_.reset();
    }

    virtual void SetRootRenderNode(const RefPtr<RenderNode>& root);

    virtual void SetRootFrameNode(const RefPtr<NG::FrameNode>& root) {}

    virtual void RecordFrameTime(uint64_t timeStamp, const std::string& name) {}

    virtual void FlushTasks() {}

    virtual bool FlushCustomAnimation(uint64_t timeStamp)
    {
        return false;
    }

    virtual std::shared_ptr<Rosen::RSUIDirector> GetRSUIDirector() const
    {
        return nullptr;
    }

    void OnVsync(uint64_t nanoTimestamp, uint32_t frameCount);

    virtual void SetVsyncCallback(AceVsyncCallback&& callback);

    virtual void OnShow()
    {
        onShow_ = true;
    }

    virtual void OnHide()
    {
        onShow_ = false;
    }

    bool IsHide() const
    {
        return !onShow_;
    }

    void SetDensity(double density)
    {
        density_ = density;
    }

    void SetGetWindowRectImpl(std::function<Rect()>&& callback)
    {
        windowRectImpl_ = std::move(callback);
    }

    virtual Rect GetCurrentWindowRect() const
    {
        Rect rect;
        if (windowRectImpl_) {
            rect = windowRectImpl_();
        }
        return rect;
    }

    void SetGetViewSafeAreaImpl(std::function<SafeAreaEdgeInserts()>&& callback)
    {
        viewSafeAreaImpl_ = std::move(callback);
    }

    SafeAreaEdgeInserts GetCurrentViewSafeArea() const
    {
        SafeAreaEdgeInserts viewSafeArea;
        if (viewSafeAreaImpl_) {
            viewSafeArea = viewSafeAreaImpl_();
        }
        return viewSafeArea;
    }

    virtual void SetDrawTextAsBitmap(bool useBitmap) {}

    virtual float GetRefreshRate() const
    {
        return 0.0f;
    }

    uint64_t GetLastRequestVsyncTime() const
    {
        return lastRequestVsyncTime_;
    }

    SafeAreaEdgeInserts GetSafeArea() const
    {
        return viewSafeArea_;
    }

    void SetSafeArea(const SafeAreaEdgeInserts& viewSafeArea)
    {
        viewSafeArea_ = viewSafeArea;
    }

    SafeAreaEdgeInserts viewSafeArea_;

    void SetSystemSafeArea(const SafeAreaEdgeInserts& safeArea)
    {
        if (systemSafeArea_ == safeArea) {
            return;
        }
        systemSafeArea_ = safeArea;
    }

    SafeAreaEdgeInserts GetSystemSafeArea() const
    {
        return systemSafeArea_;
    }

    void SetCutoutSafeArea(const SafeAreaEdgeInserts& safeArea)
    {
        if (cutoutSafeArea_ == safeArea) {
            return;
        }
        cutoutSafeArea_ = safeArea;
    }

    SafeAreaEdgeInserts GetCutoutSafeArea() const
    {
        return cutoutSafeArea_;
    }

    virtual void SetKeepScreenOn(bool keepScreenOn) {};

protected:
    bool isRequestVsync_ = false;
    bool onShow_ = true;
    double density_ = 1.0;

    struct VsyncCallback {
        AceVsyncCallback callback_ = nullptr;
        int32_t containerId_ = -1;
    };
    std::list<struct VsyncCallback> callbacks_;

    uint64_t lastRequestVsyncTime_ = 0;

    // window properties
    std::string windowName_;
    uint32_t windowId_ = 0;
    Rect windowRect_;

    SafeAreaEdgeInserts systemSafeArea_;
    SafeAreaEdgeInserts cutoutSafeArea_;

private:
    std::function<Rect()> windowRectImpl_;
    std::function<SafeAreaEdgeInserts()> viewSafeAreaImpl_;
    std::unique_ptr<PlatformWindow> platformWindow_;

    ACE_DISALLOW_COPY_AND_MOVE(Window);
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMMON_WINDOW_H

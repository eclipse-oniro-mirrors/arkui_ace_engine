/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMMON_PIPELINE_NG_CONTEXT_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMMON_PIPELINE_NG_CONTEXT_H

#include <utility>

#include "base/memory/referenced.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/custom/custom_node.h"
#include "core/components_ng/pattern/stage/stage_manager.h"
#include "core/event/touch_event.h"
#include "core/pipeline/pipeline_base.h"

namespace OHOS::Ace::NG {

class ACE_EXPORT PipelineContext final : public PipelineBase {
    DECLARE_ACE_TYPE(NG::PipelineContext, PipelineBase);

public:
    PipelineContext(std::unique_ptr<Window> window, RefPtr<TaskExecutor> taskExecutor,
        RefPtr<AssetManager> assetManager, RefPtr<PlatformResRegister> platformResRegister,
        const RefPtr<Frontend>& frontend, int32_t instanceId);
    PipelineContext(std::unique_ptr<Window> window, RefPtr<TaskExecutor> taskExecutor,
        RefPtr<AssetManager> assetManager, const RefPtr<Frontend>& frontend, int32_t instanceId);

    ~PipelineContext() override = default;

    static RefPtr<PipelineContext> GetCurrentContext();

    void SetupRootElement() override;

    bool Animate(const AnimationOption& option, const RefPtr<Curve>& curve,
        const std::function<void()>& propertyCallback, const std::function<void()>& finishCallBack = nullptr) override
    {
        return false;
    }

    void AddKeyFrame(float fraction, const RefPtr<Curve>& curve, const std::function<void()>& propertyCallback) override
    {}

    void AddKeyFrame(float fraction, const std::function<void()>& propertyCallback) override {}

    void PrepareOpenImplicitAnimation() override {}

    void OpenImplicitAnimation(const AnimationOption& option, const RefPtr<Curve>& curve,
        const std::function<void()>& finishCallBack = nullptr) override
    {}

    void PrepareCloseImplicitAnimation() override {}

    bool CloseImplicitAnimation() override
    {
        return false;
    }

    // add schedule task and return the unique mark id.
    uint32_t AddScheduleTask(const RefPtr<ScheduleTask>& task) override;

    // remove schedule task by id.
    void RemoveScheduleTask(uint32_t id) override {}

    // Called by view when touch event received.
    void OnTouchEvent(const TouchEvent& point, bool isSubPipe = false) override;

    // Called by container when key event received.
    // if return false, then this event needs platform to handle it.
    bool OnKeyEvent(const KeyEvent& event) override
    {
        return false;
    }

    // Called by view when mouse event received.
    void OnMouseEvent(const MouseEvent& event) override {}

    // Called by view when axis event received.
    void OnAxisEvent(const AxisEvent& event) override {}

    // Called by container when rotation event received.
    // if return false, then this event needs platform to handle it.
    bool OnRotationEvent(const RotationEvent& event) const override
    {
        return false;
    }

    // Called by window when received vsync signal.
    void OnVsyncEvent(uint64_t nanoTimestamp, uint32_t frameCount) override
    {
        FlushVsync(nanoTimestamp, frameCount);
    }

    void OnDragEvent(int32_t x, int32_t y, DragEventAction action) override {}

    // Called by view when idle event.
    void OnIdle(int64_t deadline) override {}

    void OnActionEvent(const std::string& action) override {}

    void SetBuildAfterCallback(const std::function<void()>& callback) override {}

    void SendEventToAccessibility(const AccessibilityEvent& accessibilityEvent) override {}

    void SaveExplicitAnimationOption(const AnimationOption& option) override {}

    void CreateExplicitAnimator(const std::function<void()>& onFinishEvent) override {}

    void ClearExplicitAnimationOption() override {}

    AnimationOption GetExplicitAnimationOption() const override
    {
        return {};
    }

    void Destroy() override {}

    void OnShow() override {}

    void OnHide() override {}

    void OnSurfaceChanged(
        int32_t width, int32_t height, WindowSizeChangeReason type = WindowSizeChangeReason::UNDEFINED) override
    {
        SetRootSize(density_, width, height);
    }

    void OnSurfacePositionChanged(int32_t posX, int32_t posY) override {}

    void OnSurfaceDensityChanged(double density) override;

    void OnSystemBarHeightChanged(double statusBar, double navigationBar) override {}

    void OnSurfaceDestroyed() override {}

    void NotifyOnPreDraw() override {}

    bool CallRouterBackToPopPage() override
    {
        return false;
    }

    void AddDirtyCustomNode(const RefPtr<CustomNode>& dirtyNode);

    void AddDirtyLayoutNode(const RefPtr<FrameNode>& dirty);

    void AddDirtyRenderNode(const RefPtr<FrameNode>& dirty);

    void FlushDirtyNodeUpdate();

    void SetRootRect(double width, double height, double offset) override;

    RefPtr<StageManager> GetStageManager();

protected:
    void FlushVsync(uint64_t nanoTimestamp, uint32_t frameCount) override;
    void FlushPipelineWithoutAnimation() override;
    void FlushMessages() override;
    void FlushAnimation(uint64_t nanoTimestamp) override;

private:
    void FlushTouchEvents();

    template<typename T>
    struct NodeCompare {
        bool operator()(const T& nodeLeft, const T& nodeRight)
        {
            if (nodeLeft->GetDepth() < nodeRight->GetDepth()) {
                return true;
            }
            if (nodeLeft->GetDepth() == nodeRight->GetDepth()) {
                return nodeLeft < nodeRight;
            }
            return false;
        }
    };

    template<typename T>
    struct NodeCompareWeak {
        bool operator()(const T& nodeLeftWeak, const T& nodeRightWeak)
        {
            auto nodeLeft = nodeLeftWeak.Upgrade();
            auto nodeRight = nodeRightWeak.Upgrade();
            if (!nodeLeft || !nodeRight) {
                return false;
            }
            auto compare = NodeCompare<decltype(nodeLeft)>();
            return compare(nodeLeft, nodeRight);
        }
    };

    std::unordered_map<uint32_t, WeakPtr<ScheduleTask>> scheduleTasks_;
    std::set<WeakPtr<CustomNode>, NodeCompareWeak<WeakPtr<CustomNode>>> dirtyNodes_;
    std::list<TouchEvent> touchEvents_;

    RefPtr<FrameNode> rootNode_ = nullptr;
    RefPtr<StageManager> stageManager_ = nullptr;
    uint32_t nextScheduleTaskId_ = 0;
    bool hasIdleTasks_ = false;
    ACE_DISALLOW_COPY_AND_MOVE(PipelineContext);
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMMON_PIPELINE_NG_CONTEXT_H

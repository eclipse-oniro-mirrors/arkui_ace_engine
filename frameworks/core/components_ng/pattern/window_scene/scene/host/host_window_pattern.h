/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_HOST_WINDOW_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_HOST_WINDOW_PATTERN_H

#include "session/host/include/session.h"

#include "core/components_ng/pattern/pattern.h"

namespace OHOS::Ace::NG {
class HostWindowPattern : public Pattern {
    DECLARE_ACE_TYPE(HostWindowPattern, Pattern);

public:
    HostWindowPattern();
    ~HostWindowPattern() override = default;

    bool OnDirtyLayoutWrapperSwap(FrameNode* frameNode, const DirtySwapConfig& config) override;

    void DispatchPointerEvent(const std::shared_ptr<OHOS::MMI::PointerEvent>& pointerEvent);
    void DispatchKeyEvent(const std::shared_ptr<OHOS::MMI::KeyEvent>& keyEvent);

protected:
    void OnAttachToFrameNode() override;

    void InitContent();

    virtual bool HasStartingPage() = 0;

    void RegisterLifecycleListener();
    void UnregisterLifecycleListener();

    virtual void OnConnect();
    virtual void OnForeground() {}
    virtual void OnBackground() {}

    int32_t instanceId_ = -1;

    RefPtr<FrameNode> startingNode_;
    RefPtr<FrameNode> contentNode_;
    RefPtr<FrameNode> snapshotNode_;

    sptr<Rosen::Session> session_;

private:
    void CreateStartingNode();
    void CreateSnapshotNode();

    void BufferAvailableCallback();

    std::shared_ptr<Rosen::ILifecycleListener> lifecycleListener_;

    friend class LifecycleListener;

    ACE_DISALLOW_COPY_AND_MOVE(HostWindowPattern);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_HOST_WINDOW_PATTERN_H

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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_WINDOW_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_WINDOW_PATTERN_H

#include "common/rs_vector4.h"
#include "key_event.h"
#include "pointer_event.h"
#include "session/host/include/session.h"

#include "core/common/container.h"
#include "core/components_ng/pattern/stack/stack_pattern.h"
#include "core/image/image_source_info.h"

namespace OHOS::Ace::NG {
class WindowPattern : public StackPattern {
    DECLARE_ACE_TYPE(WindowPattern, StackPattern);

public:
    WindowPattern() = default;
    ~WindowPattern() override = default;

    std::vector<Rosen::Rect> GetHotAreas();
    sptr<Rosen::Session> GetSession();

    static int32_t CalculateTranslateDegree(int32_t hostId);

protected:
    void OnAttachToFrameNode() override;

    void DispatchPointerEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent);
    void DispatchKeyEvent(const std::shared_ptr<MMI::KeyEvent>& keyEvent);
    void DispatchKeyEventForConsumed(const std::shared_ptr<MMI::KeyEvent>& keyEvent, bool& isConsumed);
    void DisPatchFocusActiveEvent(bool isFocusActive);
    void TransferFocusState(bool focusState);

    virtual bool HasStartingPage() = 0;
    bool IsMainWindow() const;

    void RegisterLifecycleListener();
    void UnregisterLifecycleListener();

    void CreateStartingNode();
    void CreateContentNode();
    void CreateSnapshotNode(std::optional<std::shared_ptr<Media::PixelMap>> snapshot = std::nullopt);
    void ClearImageCache(const ImageSourceInfo& sourceInfo);
    void CreateBlankNode();
    void AddChild(const RefPtr<FrameNode>& host, const RefPtr<FrameNode>& child,
        const std::string& nodeType, int32_t index = DEFAULT_NODE_SLOT);
    void RemoveChild(const RefPtr<FrameNode>& host, const RefPtr<FrameNode>& child,
        const std::string& nodeType);

    virtual void OnActivation() {}
    virtual void OnConnect() {}
    virtual void OnForeground() {}
    virtual void OnBackground() {}
    virtual void OnDisconnect() {}

    RefPtr<FrameNode> startingNode_;
    RefPtr<FrameNode> contentNode_;
    RefPtr<FrameNode> snapshotNode_;
    RefPtr<FrameNode> blankNode_;
    std::string startingNodeName_ = "startingNode";
    std::string contentNodeName_ = "contentNode";
    std::string snapshotNodeName_ = "snapshotNode";
    std::string blankNodeName_ = "blankNode";
    bool attachToFrameNodeFlag_ = false;

    sptr<Rosen::Session> session_;
    int32_t instanceId_ = Container::CurrentId();
    std::function<void()> callback_;
    std::function<void(const Rosen::Vector4f&)> boundsChangedCallback_;

private:
    void InitMouseEvent(const RefPtr<InputEventHub>& inputHub);
    void InitTouchEvent(const RefPtr<GestureEventHub>& gestureHub);
    void HandleMouseEvent(const MouseInfo& info);
    void HandleTouchEvent(const TouchEventInfo& info);
    bool IsFilterTouchEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent);
    bool IsFilterMouseEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent);
    void SetWindowSceneConsumed(int32_t action);
    void FilterInvalidPointerItem(const std::shared_ptr<MMI::PointerEvent>& pointerEvent);

    std::shared_ptr<Rosen::ILifecycleListener> lifecycleListener_;
    RefPtr<TouchEventImpl> touchEvent_;
    RefPtr<InputEvent> mouseEvent_;

    friend class LifecycleListener;
    friend class WindowEventProcess;

    ACE_DISALLOW_COPY_AND_MOVE(WindowPattern);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_WINDOW_PATTERN_H

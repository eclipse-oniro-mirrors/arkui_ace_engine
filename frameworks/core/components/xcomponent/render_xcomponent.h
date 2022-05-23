/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_XCOMPONENT_RENDER_XCOMPONENT_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_XCOMPONENT_RENDER_XCOMPONENT_H

#include "core/components/common/layout/constants.h"
#include "core/components/xcomponent/native_interface_xcomponent_impl.h"
#include "core/components/xcomponent/resource/xcomponent_delegate.h"
#include "core/components/xcomponent/xcomponent_component.h"
#include "core/pipeline/base/render_node.h"

namespace OHOS::Ace {
class RenderXComponent : public RenderNode {
    DECLARE_ACE_TYPE(RenderXComponent, RenderNode);

public:
    using XComponentSizeInitEvent = std::function<void(int64_t, int32_t, int32_t)>;
    using XComponentSizeChangeEvent = std::function<void(int64_t, int32_t, int32_t)>;

    static RefPtr<RenderNode> Create();

    ~RenderXComponent() override = default;

    void Update(const RefPtr<Component>& component) override;
    void PerformLayout() override;
    void Paint(RenderContext& context, const Offset& offset) override;
    bool HandleMouseEvent(const MouseEvent& event) override;
    void HandleMouseHoverEvent(MouseState mouseState) override;
    void PushTask(const TaskFunction& func);

    void SetDelegate(const RefPtr<XComponentDelegate>& delegate)
    {
        delegate_ = delegate;
    }

    void NativeXComponentInit(
        OH_NativeXComponent* nativeXComponent,
        WeakPtr<NativeXComponentImpl> nativeXComponentImpl);
    void NativeXComponentChange();
    void NativeXComponentDestroy();

    void SetXComponentSizeInit(XComponentSizeInitEvent &&xcomponentSizeInitEvent)
    {
        xcomponentSizeInitEvent_ = std::move(xcomponentSizeInitEvent);
    }

    void SetXComponentSizeChange(XComponentSizeChangeEvent &&xcomponentSizeChangeEvent)
    {
        xcomponentSizeChangeEvent_ = std::move(xcomponentSizeChangeEvent);
    }

    void SetXComponentId(const std::string& id)
    {
        id_ = id;
    }

    const std::string& GetXComponentId() const
    {
        return id_;
    }

    void NativeXComponentDispatchTouchEvent(const OH_NativeXComponent_TouchEvent& touchEvent);
    bool NativeXComponentDispatchMouseEvent(const OH_NativeXComponent_MouseEvent& mouseEvent);

protected:
    RefPtr<XComponentDelegate> delegate_;
    RefPtr<XComponentTaskPool> pool_;
    std::list<TaskFunction> tasks_;
    OH_NativeXComponent* nativeXComponent_ = nullptr;
    WeakPtr<NativeXComponentImpl> nativeXComponentImpl_;
    XComponentSizeInitEvent xcomponentSizeInitEvent_;
    XComponentSizeChangeEvent xcomponentSizeChangeEvent_;

    Offset position_;
    Size drawSize_;
    Offset prePosition_;
    Size preDrawSize_;
    int64_t textureId_ = -1;
    bool isXComponentInit = false;
    bool positionChange_ = false;
    bool sizeChange_ = false;
    std::string id_;

    void NativeXComponentOffset(const double& x, const double& y);
};
} // namespace OHOS::Ace
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_XCOMPONENT_RENDER_XCOMPONENT_H

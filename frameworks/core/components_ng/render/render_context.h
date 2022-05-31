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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PAINTS_RENDER_CONTEXT_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PAINTS_RENDER_CONTEXT_H

#include <memory>

#include "base/memory/ace_type.h"
#include "core/components/common/properties/color.h"

namespace OHOS::Ace::NG {

class GeometryNode;
class RenderPropertyNode;
class FrameNode;

// RenderContext is used for render node to paint.
class RenderContext : public virtual AceType {
    DECLARE_ACE_TYPE(V2::RenderContext, AceType)

public:
    ~RenderContext() override = default;

    static RefPtr<RenderContext> Create();

    void SetRequestFrame(const std::function<void()>& requestFrame)
    {
        requestFrame_ = requestFrame;
    }

    virtual void RebuildFrame(FrameNode* self) {};

    virtual void SyncGeometryProperties(GeometryNode* geometryNode) {}

    virtual void InitContext(bool isRoot = false) {}

    virtual void StartRecording(float x, float y, float width, float height) {}
    virtual void StopRecordingIfNeeded() {}

    virtual void SetDrawContentAtLast(bool usedrawContentLastOrder) {}

    virtual void UpdateBgColor(const Color& value) {}

    // cast to SkCanvas.
    virtual void* GetCanvas() = 0;
    virtual void Restore() = 0;

    void RequestNextFrame() const
    {
        if (requestFrame_) {
            requestFrame_();
        }
    }

protected:
    RenderContext() = default;

private:
    std::function<void()> requestFrame_;
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PAINTS_RENDER_CONTEXT_H

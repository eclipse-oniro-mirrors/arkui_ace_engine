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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PAINTS_RENDER_WRAPPER_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PAINTS_RENDER_WRAPPER_H

#include <functional>
#include <memory>

#include "base/geometry/ng/offset_t.h"
#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"
#include "core/components_ng/property/render_property.h"
#include "core/components_ng/render/render_context.h"
#include "core/components_ng/render/render_property.h"

namespace OHOS::Ace::NG {

// RenderWrapper are used to flush dirty render task.
class RenderWrapper : public virtual AceType {
    DECLARE_ACE_TYPE(RenderWrapper, AceType)

public:
    using ContentPaintImpl = std::function<void(RenderContext*, OffsetF)>;

    RenderWrapper(
        WeakPtr<RenderContext> renderContext, RefPtr<GeometryNode> geometryNode, RefPtr<RenderProperty> layoutProperty)
        : renderContext_(std::move(renderContext)), geometryNode_(std::move(geometryNode)),
          renderProperty_(std::move(layoutProperty))
    {}
    ~RenderWrapper() override = default;

    void SetContentPaintImpl(ContentPaintImpl&& contentPaintImpl)
    {
        contentPaintImpl_ = std::move(contentPaintImpl);
    }

    void FlushRender();

private:
    WeakPtr<RenderContext> renderContext_;
    RefPtr<GeometryNode> geometryNode_;
    RefPtr<RenderProperty> renderProperty_;
    ContentPaintImpl contentPaintImpl_;
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PAINTS_RENDER_WRAPPER_H

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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_PATH_PAINT_METHOD_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_PATH_PAINT_METHOD_H

#include "base/geometry/ng/size_t.h"
#include "core/components_ng/base/geometry_node.h"
#include "core/components_ng/pattern/shape/path_paint_property.h"
#include "core/components_ng/pattern/shape/shape_paint_property.h"
#include "core/components_ng/render/node_paint_method.h"
#include "core/components_ng/render/path_painter.h"

namespace OHOS::Ace::NG {

class ACE_EXPORT PathPaintMethod : public NodePaintMethod {
    DECLARE_ACE_TYPE(PathPaintMethod, NodePaintMethod)
public:
    PathPaintMethod() = default;
    explicit PathPaintMethod(const RefPtr<ShapePaintProperty>& shapePaintProperty)
        : propertiesFromAncestor_(shapePaintProperty)
    {}
    ~PathPaintMethod() override = default;

    CanvasDrawFunction GetContentDrawFunction(PaintWrapper* paintWrapper) override
    {
        auto shapePaintProperty = DynamicCast<PathPaintProperty>(paintWrapper->GetPaintProperty()->Clone());
        if (!shapePaintProperty) {
            return nullptr;
        }
        if (propertiesFromAncestor_) {
            shapePaintProperty->UpdateShapeProperty(propertiesFromAncestor_);
        }
        return [shapePaintProperty](RSCanvas& canvas) { PathPainter::DrawPath(canvas, *shapePaintProperty); };
    }

private:
    RefPtr<ShapePaintProperty> propertiesFromAncestor_;
    ACE_DISALLOW_COPY_AND_MOVE(PathPaintMethod);
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_PATH_PAINT_METHOD_H
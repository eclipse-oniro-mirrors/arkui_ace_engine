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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_ELLIPSE_PAINT_METHOD_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_ELLIPSE_PAINT_METHOD_H

#include "base/geometry/ng/rect_t.h"
#include "base/geometry/ng/size_t.h"
#include "core/components_ng/base/geometry_node.h"
#include "core/components_ng/pattern/shape/shape_paint_property.h"
#include "core/components_ng/render/ellipse_painter.h"
#include "core/components_ng/render/node_paint_method.h"

namespace OHOS::Ace::NG {

class ACE_EXPORT EllipsePaintMethod : public NodePaintMethod {
    DECLARE_ACE_TYPE(EllipsePaintMethod, NodePaintMethod)
public:
    EllipsePaintMethod() = default;
    explicit EllipsePaintMethod(const RefPtr<ShapePaintProperty>& shapePaintProperty)
        : propertiesFromAncestor_(shapePaintProperty)
    {}
    ~EllipsePaintMethod() override = default;

    CanvasDrawFunction GetContentDrawFunction(PaintWrapper* paintWrapper) override
    {
        auto shapePaintProperty = DynamicCast<ShapePaintProperty>(paintWrapper->GetPaintProperty()->Clone());
        if (!shapePaintProperty) {
            return nullptr;
        }
        if (propertiesFromAncestor_) {
            shapePaintProperty->UpdateShapeProperty(propertiesFromAncestor_);
        }
        float height = paintWrapper->GetContentSize().Height();
        float width = paintWrapper->GetContentSize().Width();
        float dx = paintWrapper->GetContentOffset().GetX();
        float dy = paintWrapper->GetContentOffset().GetY();
        RectF rect(dx, dy, width, height);
        return [rect, shapePaintProperty](
                   RSCanvas& canvas) { EllipsePainter::DrawEllipse(canvas, rect, *shapePaintProperty); };
    }

private:
    RefPtr<ShapePaintProperty> propertiesFromAncestor_;
    ACE_DISALLOW_COPY_AND_MOVE(EllipsePaintMethod);
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_ELLIPSE_PAINT_METHOD_H
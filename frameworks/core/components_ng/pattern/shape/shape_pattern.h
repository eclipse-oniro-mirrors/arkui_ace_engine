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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SHAPE_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SHAPE_PATTERN_H

#include <cstddef>
#include <optional>

#include "base/geometry/ng/rect_t.h"
#include "base/log/log_wrapper.h"
#include "base/memory/referenced.h"
#include "base/utils/noncopyable.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/pattern/shape/shape_container_pattern.h"
#include "core/components_ng/pattern/shape/shape_layout_algorithm.h"
#include "core/components_ng/pattern/shape/shape_overlay_modifier.h"
#include "core/components_ng/pattern/shape/shape_paint_property.h"
#include "core/components_ng/pattern/shape/shape_view_box.h"

namespace OHOS::Ace::NG {
class ShapePattern : public Pattern {
    DECLARE_ACE_TYPE(ShapePattern, Pattern);

public:
    ShapePattern() = default;
    ~ShapePattern() override = default;

    RefPtr<LayoutAlgorithm> CreateLayoutAlgorithm() override
    {
        return MakeRefPtr<ShapeLayoutAlgorithm>();
    }

    RefPtr<PaintProperty> CreatePaintProperty() override
    {
        return MakeRefPtr<ShapePaintProperty>();
    }

protected:
    RefPtr<ShapePaintProperty> GetAncestorPaintProperty()
    {
        auto curFrameNode = GetHost();
        CHECK_NULL_RETURN(curFrameNode, nullptr);
        auto childNode = curFrameNode;
        ShapePaintProperty propertiesFromAncestor;
        auto parentFrameNode = AceType::DynamicCast<FrameNode>(curFrameNode->GetAncestorNodeOfFrame());
        while (parentFrameNode) {
            auto parentPaintProperty = parentFrameNode->GetPaintProperty<ShapePaintProperty>();
            if (parentPaintProperty) {
                propertiesFromAncestor.UpdateShapeProperty(parentPaintProperty);
                UpdateForeground(parentFrameNode, childNode);
                auto pattern = AceType::DynamicCast<ShapeContainerPattern>(parentFrameNode->GetPattern());
                if (pattern) {
                    pattern->AddChildShapeNode(WeakPtr<FrameNode>(childNode));
                }
            }
            curFrameNode = parentFrameNode;
            parentFrameNode = AceType::DynamicCast<FrameNode>(curFrameNode->GetAncestorNodeOfFrame());
        }
        return DynamicCast<ShapePaintProperty>(propertiesFromAncestor.Clone());
    }

    void UpdateForeground(RefPtr<FrameNode> parentFrameNode, RefPtr<FrameNode> childFrameNode)
    {
        auto renderContext = parentFrameNode->GetRenderContext();
        auto childRenderContext = childFrameNode->GetRenderContext();
        if (childRenderContext) {
            if (!childRenderContext->HasForegroundColor() && !childRenderContext->HasForegroundColorStrategy()) {
                if (renderContext->HasForegroundColor()) {
                    childRenderContext->UpdateForegroundColor(renderContext->GetForegroundColorValue());
                    childRenderContext->ResetForegroundColorStrategy();
                } else if (renderContext->HasForegroundColorStrategy()) {
                    childRenderContext->UpdateForegroundColorStrategy(renderContext->GetForegroundColorStrategyValue());
                    childRenderContext->ResetForegroundColor();
                }
            }
        }
    }

    RefPtr<ShapeOverlayModifier> shapeOverlayModifier_;

private:
    bool OnDirtyLayoutWrapperSwap(FrameNode* frameNode, bool skipMeasure, bool skipLayout) override
    {
        return !(skipMeasure || frameNode->SkipMeasureContent());
    }
    ACE_DISALLOW_COPY_AND_MOVE(ShapePattern);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SHAPE_PATTERN_H

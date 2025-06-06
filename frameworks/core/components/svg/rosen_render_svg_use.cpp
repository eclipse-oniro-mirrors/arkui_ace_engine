/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "frameworks/core/components/svg/rosen_render_svg_use.h"

#include "frameworks/core/components/common/painter/rosen_svg_painter.h"
#include "frameworks/core/components/transform/rosen_render_transform.h"

namespace OHOS::Ace {

void RosenRenderSvgUse::Paint(RenderContext& context, const Offset& offset)
{
    auto rsNode = static_cast<RosenRenderContext*>(&context)->GetRSNode();
    if (rsNode == nullptr) {
        return;
    }

    if (GreatNotEqual(x_.Value(), 0.0) || GreatNotEqual(y_.Value(), 0.0)) {
        rsNode->SetTranslate(
            { ConvertDimensionToPx(x_, LengthType::HORIZONTAL), ConvertDimensionToPx(y_, LengthType::VERTICAL) });
    }
    if (NeedTransform()) {
        auto [transform, pivotX, pivotY] = GetRawTransformInfo();
        rsNode->SetPivot(pivotX, pivotY);
        RosenRenderTransform::SyncTransformToRsNode(rsNode, transform);
    }

    RenderNode::Paint(context, offset);
}

void RosenRenderSvgUse::PaintDirectly(RenderContext& context, const Offset& offset)
{
    const auto renderContext = static_cast<RosenRenderContext*>(&context);
    auto canvas = renderContext->GetCanvas();
    if (!canvas) {
        LOGE("Paint canvas is null");
        return;
    }

    RSAutoCanvasRestore save(*canvas, true);
    bool translateXY = GreatNotEqual(x_.Value(), 0.0) || GreatNotEqual(y_.Value(), 0.0);
    if (translateXY) {
        canvas->Translate(
            ConvertDimensionToPx(x_, LengthType::HORIZONTAL), ConvertDimensionToPx(y_, LengthType::VERTICAL));
    }
    if (NeedTransform()) {
        canvas->ConcatMatrix(RosenSvgPainter::ToDrawingMatrix(GetTransformMatrix4Raw()));
    }
    PaintMaskLayer(context, offset, offset);

    RenderSvgBase::PaintDirectly(context, offset);
}

} // namespace OHOS::Ace

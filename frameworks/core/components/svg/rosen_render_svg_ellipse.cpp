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

#include "frameworks/core/components/svg/rosen_render_svg_ellipse.h"

#include "frameworks/core/components/common/painter/rosen_svg_painter.h"
#include "frameworks/core/components/transform/rosen_render_transform.h"

namespace OHOS::Ace {

void RosenRenderSvgEllipse::Paint(RenderContext& context, const Offset& offset)
{
    const auto renderContext = static_cast<RosenRenderContext*>(&context);
    auto rsNode = renderContext->GetRSNode();
    auto canvas = renderContext->GetCanvas();
    if (!canvas) {
        LOGE("Paint canvas is null");
        return;
    }

    if (rsNode && NeedTransform()) {
        auto [transform, pivotX, pivotY] = GetRawTransformInfo();
        rsNode->SetPivot(pivotX, pivotY);
        RosenRenderTransform::SyncTransformToRsNode(rsNode, transform);
    }

    RSAutoCanvasRestore save(*canvas, false);
    PaintMaskLayer(context, offset, offset);

    RSPath path;
    GetPath(path);
    UpdateGradient(fillState_);

    RenderInfo renderInfo = { AceType::Claim(this), offset, opacity_, true };
    RosenSvgPainter::SetFillStyle(canvas, path, fillState_, renderInfo);
    RosenSvgPainter::SetStrokeStyle(canvas, path, strokeState_, renderInfo);
    RenderNode::Paint(context, offset);
}

void RosenRenderSvgEllipse::PaintDirectly(RenderContext& context, const Offset& offset)
{
    auto canvas = static_cast<RosenRenderContext*>(&context)->GetCanvas();
    if (!canvas) {
        LOGE("Paint canvas is null");
        return;
    }
    RSAutoCanvasRestore save(*canvas, true);
    if (NeedTransform()) {
        canvas->ConcatMatrix(RosenSvgPainter::ToDrawingMatrix(GetTransformMatrix4Raw()));
    }
    PaintMaskLayer(context, offset, offset);

    RSPath path;
    GetPath(path);
    UpdateGradient(fillState_);
    RosenSvgPainter::SetFillStyle(canvas, path, fillState_, opacity_);
    RosenSvgPainter::SetStrokeStyle(canvas, path, strokeState_, opacity_);
}

void RosenRenderSvgEllipse::UpdateMotion(const std::string& path, const std::string& rotate, double percent)
{
    auto rsNode = GetRSNode();
    if (!rsNode) {
        LOGE("transformLayer is null");
        return;
    }
    RosenSvgPainter::UpdateMotionMatrix(rsNode, path, rotate, percent);
}

Rect RosenRenderSvgEllipse::GetPaintBounds(const Offset& offset)
{
    RSPath path;
    GetPath(path);
    auto bounds = path.GetBounds();
    return Rect(bounds.GetLeft(), bounds.GetTop(), bounds.GetWidth(), bounds.GetHeight());
}

void RosenRenderSvgEllipse::GetPath(RSPath& path)
{
    double rx = 0.0;
    if (GreatOrEqual(rx_.Value(), 0.0)) {
        rx = ConvertDimensionToPx(rx_, LengthType::HORIZONTAL);
    } else {
        if (GreatNotEqual(ry_.Value(), 0.0)) {
            rx = ConvertDimensionToPx(ry_, LengthType::VERTICAL);
        }
    }
    double ry = 0.0;
    if (GreatOrEqual(ry_.Value(), 0.0)) {
        ry = ConvertDimensionToPx(ry_, LengthType::VERTICAL);
    } else {
        if (GreatNotEqual(rx_.Value(), 0.0)) {
            ry = ConvertDimensionToPx(rx_, LengthType::HORIZONTAL);
        }
    }
    auto rect = RSRect(ConvertDimensionToPx(cx_, LengthType::HORIZONTAL) - rx,
        ConvertDimensionToPx(cy_, LengthType::VERTICAL) - ry,
        rx + ConvertDimensionToPx(cx_, LengthType::HORIZONTAL),
        ry + ConvertDimensionToPx(cy_, LengthType::VERTICAL));
    path.AddOval(rect);
}

} // namespace OHOS::Ace

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

#include "frameworks/core/components/svg/rosen_render_svg_polygon.h"

#include "frameworks/core/components/common/painter/rosen_svg_painter.h"
#include "frameworks/core/components/transform/rosen_render_transform.h"

namespace OHOS::Ace {

void RosenRenderSvgPolygon::Paint(RenderContext& context, const Offset& offset)
{
    if (points_.empty()) {
        return;
    }
    const auto renderContext = static_cast<RosenRenderContext*>(&context);
    auto rsNode = renderContext->GetRSNode();
    auto canvas = renderContext->GetCanvas();
    if (!canvas) {
        LOGE("Paint canvas is null");
        return;
    }
    RSPath out;
    if (!GetPath(&out)) {
        return;
    }
    if (rsNode && NeedTransform()) {
        auto [transform, pivotX, pivotY] = GetRawTransformInfo();
        rsNode->SetPivot(pivotX, pivotY);
        RosenRenderTransform::SyncTransformToRsNode(rsNode, transform);
    }

    RSAutoCanvasRestore save(*canvas, false);
    PaintMaskLayer(context, offset, offset);

    UpdateGradient(fillState_);

    RenderInfo renderInfo = { AceType::Claim(this), offset, opacity_, true };
    RosenSvgPainter::SetFillStyle(canvas, out, fillState_, renderInfo);
    RosenSvgPainter::SetStrokeStyle(canvas, out, strokeState_, renderInfo);
    RenderNode::Paint(context, offset);
}

void RosenRenderSvgPolygon::PaintDirectly(RenderContext& context, const Offset& offset)
{
    if (points_.empty()) {
        return;
    }
    auto canvas = static_cast<RosenRenderContext*>(&context)->GetCanvas();
    if (!canvas) {
        LOGE("Paint canvas is null");
        return;
    }
    RSPath out;
    if (!GetPath(&out)) {
        return;
    }
    RSAutoCanvasRestore save(*canvas, true);
    if (NeedTransform()) {
        canvas->ConcatMatrix(RosenSvgPainter::ToDrawingMatrix(GetTransformMatrix4Raw()));
    }
    PaintMaskLayer(context, offset, offset);

    UpdateGradient(fillState_);
    RosenSvgPainter::SetFillStyle(canvas, out, fillState_, opacity_);
    RosenSvgPainter::SetStrokeStyle(canvas, out, strokeState_, opacity_);
}

bool RosenRenderSvgPolygon::CreateRSPath(const std::string& pointsStr, std::vector<RSPoint>& rsPoints)
{
    if (pointsStr.empty()) {
        return false;
    }
    RosenSvgPainter::StringToPoints(pointsStr.c_str(), rsPoints);
    return !rsPoints.empty();
}

bool RosenRenderSvgPolygon::CreateRSPaths(
    const std::string& points1, const std::string& points2, double weight, RSPath* out)
{
    RSPath begin;
    RSPath end;
    if (points1.empty() || points2.empty() || out == nullptr) {
        return false;
    }
    std::vector<RSPoint> rsPoints1;
    std::vector<RSPoint> rsPoints2;
    if (!CreateRSPath(points1.c_str(), rsPoints1) || !CreateRSPath(points2.c_str(), rsPoints2)) {
        return false;
    }
    if (rsPoints1.size() != rsPoints2.size()) {
        return false;
    }
    if (isBy_) {
        auto rsPointIter1 = rsPoints1.begin();
        auto rsPointIter2 = rsPoints2.begin();
        while (rsPointIter1 != rsPoints1.end()) {
            *rsPointIter1 = *rsPointIter1 + *rsPointIter2;
            ++rsPointIter1;
            ++rsPointIter2;
        }
    }
    begin.AddPoly(rsPoints1, rsPoints1.size(), isClose_);
    end.AddPoly(rsPoints2, rsPoints2.size(), isClose_);
    out->BuildFromInterpolate(begin, end, weight);
    return true;
}

void RosenRenderSvgPolygon::UpdateMotion(const std::string& path, const std::string& rotate, double percent)
{
    auto rsNode = GetRSNode();
    if (!rsNode) {
        LOGE("transformLayer is null");
        return;
    }
    RosenSvgPainter::UpdateMotionMatrix(rsNode, path, rotate, percent);
}

Rect RosenRenderSvgPolygon::GetPaintBounds(const Offset& offset)
{
    RSPath path;
    if (!GetPath(&path)) {
        return GetPaintRect();
    }
    auto bounds = path.GetBounds();
    return Rect(bounds.GetLeft(), bounds.GetTop(), bounds.GetWidth(), bounds.GetHeight());
}

bool RosenRenderSvgPolygon::GetPath(RSPath* out)
{
    if (!out) {
        LOGE("SkPath is null");
        return false;
    }
    if (pointsVector_.empty()) {
        return GetPathWithoutAnimate(out);
    } else {
        int32_t firstPart = (int)weight_;
        int32_t pathsSize = static_cast<int32_t>(pointsVector_.size());
        if (firstPart < 0 || firstPart > (pathsSize - 1)) {
            return false;
        }
        float weight = 1.0f;
        int32_t currValue = 0;
        int32_t nextValue = 0;
        if (firstPart == (pathsSize - 1)) {
            currValue = firstPart;
            nextValue = firstPart - 1;
        } else {
            weight = weight_ - firstPart;
            currValue = firstPart + 1;
            nextValue = firstPart;
        }
        if (!CreateRSPaths(pointsVector_[currValue].c_str(), pointsVector_[nextValue].c_str(), weight, out)) {
            return GetPathWithoutAnimate(out);
        }
    }
    if (fillState_.IsEvenodd()) {
        out->SetFillStyle(RSPathFillType::EVENTODD);
    }
    return true;
}

bool RosenRenderSvgPolygon::GetPathWithoutAnimate(RSPath* out)
{
    if (!out) {
        LOGE("path is null");
        return false;
    }
    std::vector<RSPoint> rsPoints;
    if (!CreateRSPath(points_, rsPoints)) {
        return false;
    }
    out->AddPoly(rsPoints, rsPoints.size(), isClose_);
    if (fillState_.IsEvenodd()) {
        out->SetFillStyle(RSPathFillType::EVENTODD);
    }
    return true;
}

} // namespace OHOS::Ace

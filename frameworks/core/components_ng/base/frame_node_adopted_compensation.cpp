/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "core/components_ng/base/frame_node.h"

#include <algorithm>
#include <cmath>
#include <memory>
#include <vector>

#include "base/geometry/transform_util.h"
#include "base/utils/utils.h"

namespace OHOS::Ace::NG {
namespace {
RectF GetBoundingRect(const std::vector<Point>& points)
{
    CHECK_NULL_RETURN(!points.empty(), RectF());
    auto minX = points.front().GetX();
    auto maxX = minX;
    auto minY = points.front().GetY();
    auto maxY = minY;
    for (const auto& point : points) {
        minX = std::min(minX, point.GetX());
        maxX = std::max(maxX, point.GetX());
        minY = std::min(minY, point.GetY());
        maxY = std::max(maxY, point.GetY());
    }
    return RectF(minX, minY, maxX - minX, maxY - minY);
}

std::vector<Point> GetRectPoints(const RectF& rect)
{
    return {
        Point(rect.Left(), rect.Top()),
        Point(rect.Right(), rect.Top()),
        Point(rect.Right(), rect.Bottom()),
        Point(rect.Left(), rect.Bottom()),
    };
}
} // namespace

void FrameNode::SetUseAdoptedCompensation(bool useAdoptedCompensation)
{
    if (!useAdoptedCompensation && !UseAdoptedCompensation()) {
        return;
    }
    if (!useAdoptedCompensation) {
        adoptedCompensationInfo_.reset();
        return;
    }
    if (!adoptedCompensationInfo_) {
        adoptedCompensationInfo_ = std::make_unique<AdoptedCompensationInfo>();
    }
    adoptedCompensationInfo_->useAdoptedCompensation = true;
}

bool FrameNode::HasAdoptedCompensation() const
{
    return IsAdopted() && UseAdoptedCompensation();
}

void FrameNode::SetAdoptedCompensation(const OffsetF& offset, const Matrix4& transform)
{
    if (!adoptedCompensationInfo_) {
        adoptedCompensationInfo_ = std::make_unique<AdoptedCompensationInfo>();
    }
    adoptedCompensationInfo_->offset = offset;
    adoptedCompensationInfo_->transform = transform;
}

OffsetF FrameNode::GetAdoptedCompensationOffset() const
{
    return adoptedCompensationInfo_ ? adoptedCompensationInfo_->offset : OffsetF();
}

Matrix4 FrameNode::GetAdoptedCompensationTransform() const
{
    return adoptedCompensationInfo_ ? adoptedCompensationInfo_->transform : Matrix4();
}

void FrameNode::ApplyAdoptedCompensationToPoints(std::vector<Point>& points) const
{
    if (!HasAdoptedCompensation()) {
        return;
    }
    auto offset = GetAdoptedCompensationOffset();
    auto transform = GetAdoptedCompensationTransform();
    for (auto& point : points) {
        point = transform * (point + Offset(offset.GetX(), offset.GetY()));
    }
}

void FrameNode::ApplyAdoptedCompensationToPoints(std::vector<PointF>& points) const
{
    if (!HasAdoptedCompensation()) {
        return;
    }
    auto offset = GetAdoptedCompensationOffset();
    auto transform = GetAdoptedCompensationTransform();
    for (auto& point : points) {
        auto result = transform * (Point(point.GetX(), point.GetY()) + Offset(offset.GetX(), offset.GetY()));
        point.SetX(result.GetX());
        point.SetY(result.GetY());
    }
}

void FrameNode::ApplyAdoptedCompensationToRect(RectF& rect) const
{
    auto points = GetRectPoints(rect);
    ApplyAdoptedCompensationToPoints(points);
    rect = GetBoundingRect(points);
}

void FrameNode::ApplyAdoptedCompensationToPoint(Point& point) const
{
    if (!HasAdoptedCompensation()) {
        return;
    }
    auto offset = GetAdoptedCompensationOffset();
    point = GetAdoptedCompensationTransform() * (point + Offset(offset.GetX(), offset.GetY()));
}

void FrameNode::ApplyAdoptedCompensationToPoint(PointF& point) const
{
    Point transformedPoint(point.GetX(), point.GetY());
    ApplyAdoptedCompensationToPoint(transformedPoint);
    point.SetX(transformedPoint.GetX());
    point.SetY(transformedPoint.GetY());
}

void FrameNode::RevertAdoptedCompensationFromPoint(PointF& point) const
{
    if (!HasAdoptedCompensation()) {
        return;
    }
    auto transformedPoint =
        Matrix4::Invert(GetAdoptedCompensationTransform()) * Point(point.GetX(), point.GetY());
    auto offset = GetAdoptedCompensationOffset();
    point.SetX(transformedPoint.GetX() - offset.GetX());
    point.SetY(transformedPoint.GetY() - offset.GetY());
}

void FrameNode::ApplyAdoptedCompensationToCenterAndSize(PointF& center, float& width, float& height) const
{
    if (!HasAdoptedCompensation()) {
        return;
    }
    ApplyAdoptedCompensationToPoint(center);
    auto offset = GetAdoptedCompensationOffset();
    auto transform = GetAdoptedCompensationTransform();
    auto origin = transform * Point(offset.GetX(), offset.GetY());
    auto right = transform * Point(offset.GetX() + width, offset.GetY());
    auto bottom = transform * Point(offset.GetX(), offset.GetY() + height);
    width = std::hypot(right.GetX() - origin.GetX(), right.GetY() - origin.GetY());
    height = std::hypot(bottom.GetX() - origin.GetX(), bottom.GetY() - origin.GetY());
}

OffsetF FrameNode::GetTransformedAdoptedCompensationOffset() const
{
    CHECK_NULL_RETURN(HasAdoptedCompensation(), OffsetF());
    auto offset = GetAdoptedCompensationOffset();
    auto point = GetAdoptedCompensationTransform() * Point(offset.GetX(), offset.GetY());
    return OffsetF(point.GetX(), point.GetY());
}

VectorF FrameNode::GetAdoptedCompensationScale() const
{
    DecomposedTransform transform;
    if (!TransformUtil::DecomposeTransform(transform, GetAdoptedCompensationTransform())) {
        return VectorF(1.0f, 1.0f);
    }
    return VectorF(static_cast<float>(transform.scale[0]), static_cast<float>(transform.scale[1]));
}
} // namespace OHOS::Ace::NG

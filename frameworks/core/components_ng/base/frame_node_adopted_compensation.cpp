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

#include "core/components_ng/base/frame_node_adopted_compensation.h"

#include <algorithm>
#include <cmath>
#include <memory>
#include <vector>

#include "base/geometry/transform_util.h"
#include "base/utils/utils.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/frame_node_adopted_render_adapter.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
namespace {
constexpr int32_t TRANSLATION_X_ROW = 0;
constexpr int32_t TRANSLATION_Y_ROW = 1;
constexpr int32_t TRANSLATION_COLUMN = 3;

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

bool IsUnsupportedTransform(const Matrix4& matrix, bool checkScale)
{
    DecomposedTransform transform;
    if (!TransformUtil::DecomposeTransform(transform, matrix)) {
        return true;
    }
    constexpr int32_t zIndex = 2;
    Quaternion identity(0.0f, 0.0f, 0.0f, 1.0f);
    if (transform.quaternion != identity || !NearZero(transform.translate[zIndex])) {
        return true;
    }
    if (!checkScale) {
        return false;
    }
    for (const auto& scale : transform.scale) {
        if (!NearEqual(scale, 1.0)) {
            return true;
        }
    }
    return false;
}

bool HasNonTranslationChange(const Matrix4& oldTransform, const Matrix4& newTransform)
{
    auto oldNonTranslation = oldTransform;
    auto newNonTranslation = newTransform;
    oldNonTranslation.Set(TRANSLATION_X_ROW, TRANSLATION_COLUMN, 0.0);
    oldNonTranslation.Set(TRANSLATION_Y_ROW, TRANSLATION_COLUMN, 0.0);
    newNonTranslation.Set(TRANSLATION_X_ROW, TRANSLATION_COLUMN, 0.0);
    newNonTranslation.Set(TRANSLATION_Y_ROW, TRANSLATION_COLUMN, 0.0);
    return !(oldNonTranslation == newNonTranslation);
}
} // namespace

bool FrameNodeAdoptedCompensation::UseAdoptedCompensation(const FrameNode* frameNode)
{
    CHECK_NULL_RETURN(frameNode, false);
    const auto& compensationInfo = frameNode->GetAdoptedCompensationInfo();
    return compensationInfo && compensationInfo->useAdoptedCompensation;
}

void FrameNodeAdoptedCompensation::SetUseAdoptedCompensation(
    FrameNode* frameNode, bool useAdoptedCompensation)
{
    CHECK_NULL_VOID(frameNode);
    if (!useAdoptedCompensation && !UseAdoptedCompensation(frameNode)) {
        return;
    }
    auto& compensationInfo = frameNode->GetAdoptedCompensationInfo();
    if (!useAdoptedCompensation) {
        compensationInfo.reset();
        return;
    }
    if (!compensationInfo) {
        compensationInfo = std::make_unique<FrameNode::AdoptedCompensationInfo>();
    }
    compensationInfo->useAdoptedCompensation = true;
}

bool FrameNodeAdoptedCompensation::HasAdoptedCompensation(const FrameNode* frameNode)
{
    return frameNode && frameNode->IsAdopted() && UseAdoptedCompensation(frameNode);
}

void FrameNodeAdoptedCompensation::SetAdoptedCompensation(
    FrameNode* frameNode, const OffsetF& offset, const Matrix4& transform)
{
    CHECK_NULL_VOID(frameNode);
    auto& compensationInfo = frameNode->GetAdoptedCompensationInfo();
    if (!compensationInfo) {
        compensationInfo = std::make_unique<FrameNode::AdoptedCompensationInfo>();
    }
    const auto offsetChanged = !(compensationInfo->offset == offset);
    const auto transformChanged = !(compensationInfo->transform == transform);
    if (!offsetChanged && !transformChanged) {
        return;
    }
    const auto nonTranslationChanged =
        transformChanged && HasNonTranslationChange(compensationInfo->transform, transform);
    compensationInfo->offset = offset;
    compensationInfo->transform = transform;
    if (!HasAdoptedCompensation(frameNode)) {
        return;
    }

    frameNode->AddFrameNodeChangeInfoFlag(FRAME_NODE_CHANGE_GEOMETRY_CHANGE);
    if (nonTranslationChanged) {
        frameNode->AddFrameNodeChangeInfoFlag(FRAME_NODE_CHANGE_TRANSFORM_CHANGE);
    }
    auto pipeline = frameNode->GetContext();
    CHECK_NULL_VOID(pipeline);
    pipeline->RequestFrame();
}

OffsetF FrameNodeAdoptedCompensation::GetAdoptedCompensationOffset(const FrameNode* frameNode)
{
    CHECK_NULL_RETURN(frameNode, OffsetF());
    const auto& compensationInfo = frameNode->GetAdoptedCompensationInfo();
    return compensationInfo ? compensationInfo->offset : OffsetF();
}

Matrix4 FrameNodeAdoptedCompensation::GetAdoptedCompensationTransform(const FrameNode* frameNode)
{
    CHECK_NULL_RETURN(frameNode, Matrix4());
    const auto& compensationInfo = frameNode->GetAdoptedCompensationInfo();
    return compensationInfo ? compensationInfo->transform : Matrix4();
}

bool FrameNodeAdoptedCompensation::HasAdoptedCompensationNonTranslationTransform(const FrameNode* frameNode)
{
    std::vector<FrameNodeAdoptedRenderAdapter::RSNodePtr> layers;
    if (FrameNodeAdoptedRenderAdapter::CollectLayers(frameNode, layers) && !layers.empty()) {
        return std::any_of(layers.begin(), layers.end(), [](const auto& layer) {
            auto transform = FrameNodeAdoptedRenderAdapter::GetTransform(layer);
            transform.Set(TRANSLATION_X_ROW, TRANSLATION_COLUMN, 0.0);
            transform.Set(TRANSLATION_Y_ROW, TRANSLATION_COLUMN, 0.0);
            return !transform.IsIdentityMatrix();
        });
    }
    auto transform = GetAdoptedCompensationTransform(frameNode);
    transform.Set(TRANSLATION_X_ROW, TRANSLATION_COLUMN, 0.0);
    transform.Set(TRANSLATION_Y_ROW, TRANSLATION_COLUMN, 0.0);
    return !transform.IsIdentityMatrix();
}

bool FrameNodeAdoptedCompensation::HasAdoptedCompensationUnsupportedTransform(
    const FrameNode* frameNode, bool checkScale)
{
    std::vector<FrameNodeAdoptedRenderAdapter::RSNodePtr> layers;
    if (FrameNodeAdoptedRenderAdapter::CollectLayers(frameNode, layers) && !layers.empty()) {
        return std::any_of(layers.begin(), layers.end(), [checkScale](const auto& layer) {
            return IsUnsupportedTransform(FrameNodeAdoptedRenderAdapter::GetTransform(layer), checkScale);
        });
    }
    return HasAdoptedCompensation(frameNode) &&
        IsUnsupportedTransform(GetAdoptedCompensationTransform(frameNode), checkScale);
}

bool FrameNodeAdoptedCompensation::HasAdoptedCompensationPaintRectTransform(const FrameNode* frameNode)
{
    std::vector<FrameNodeAdoptedRenderAdapter::RSNodePtr> layers;
    if (FrameNodeAdoptedRenderAdapter::CollectLayers(frameNode, layers) && !layers.empty()) {
        return std::any_of(layers.begin(), layers.end(), [](const auto& layer) {
            auto bounds = FrameNodeAdoptedRenderAdapter::GetBounds(layer);
            return FrameNodeAdoptedRenderAdapter::GetPaintRect(layer) != bounds;
        });
    }
    if (!HasAdoptedCompensation(frameNode)) {
        return false;
    }
    auto transformedOrigin = GetAdoptedCompensationTransform(frameNode) * Point();
    auto offset = GetAdoptedCompensationOffset(frameNode);
    return !NearEqual(transformedOrigin.GetX(), offset.GetX()) ||
        !NearEqual(transformedOrigin.GetY(), offset.GetY()) ||
        HasAdoptedCompensationNonTranslationTransform(frameNode);
}

void FrameNodeAdoptedCompensation::ApplyAdoptedCompensationToPoints(
    const FrameNode* frameNode, std::vector<Point>& points)
{
    if (!HasAdoptedCompensation(frameNode)) {
        return;
    }
    auto transform = GetAdoptedCompensationTransform(frameNode);
    for (auto& point : points) {
        point = transform * point;
    }
}

void FrameNodeAdoptedCompensation::ApplyAdoptedCompensationToPoints(
    const FrameNode* frameNode, std::vector<PointF>& points)
{
    if (!HasAdoptedCompensation(frameNode)) {
        return;
    }
    auto transform = GetAdoptedCompensationTransform(frameNode);
    for (auto& point : points) {
        auto result = transform * Point(point.GetX(), point.GetY());
        point.SetX(result.GetX());
        point.SetY(result.GetY());
    }
}

void FrameNodeAdoptedCompensation::ApplyAdoptedCompensationToRect(const FrameNode* frameNode, RectF& rect)
{
    std::vector<FrameNodeAdoptedRenderAdapter::RSNodePtr> layers;
    if (!FrameNodeAdoptedRenderAdapter::CollectLayers(frameNode, layers) || layers.empty()) {
        auto points = GetRectPoints(rect);
        ApplyAdoptedCompensationToPoints(frameNode, points);
        rect = GetBoundingRect(points);
        return;
    }
    for (const auto& layer : layers) {
        auto scale = FrameNodeAdoptedRenderAdapter::GetScale(layer);
        auto offset = rect.GetOffset();
        rect.SetWidth(rect.Width() * scale.x);
        rect.SetHeight(rect.Height() * scale.y);
        offset = OffsetF(offset.GetX() * scale.x, offset.GetY() * scale.y);
        offset += FrameNodeAdoptedRenderAdapter::GetPaintRect(layer).GetOffset();
        rect.SetOffset(offset);
    }
}

void FrameNodeAdoptedCompensation::ApplyAdoptedCompensationToPoint(const FrameNode* frameNode, Point& point)
{
    if (!HasAdoptedCompensation(frameNode)) {
        return;
    }
    point = GetAdoptedCompensationTransform(frameNode) * point;
}

void FrameNodeAdoptedCompensation::ApplyAdoptedCompensationToPoint(const FrameNode* frameNode, PointF& point)
{
    Point transformedPoint(point.GetX(), point.GetY());
    ApplyAdoptedCompensationToPoint(frameNode, transformedPoint);
    point.SetX(transformedPoint.GetX());
    point.SetY(transformedPoint.GetY());
}

void FrameNodeAdoptedCompensation::RevertAdoptedCompensationFromPoint(const FrameNode* frameNode, Point& point)
{
    if (!HasAdoptedCompensation(frameNode)) {
        return;
    }
    point = Matrix4::Invert(GetAdoptedCompensationTransform(frameNode)) * point;
}

void FrameNodeAdoptedCompensation::ApplyAdoptedCompensationToPointFromWindow(
    const FrameNode* frameNode, Point& point)
{
    std::vector<FrameNodeAdoptedRenderAdapter::RSNodePtr> layers;
    if (!FrameNodeAdoptedRenderAdapter::CollectLayers(frameNode, layers) || layers.empty()) {
        RevertAdoptedCompensationFromPoint(frameNode, point);
        return;
    }
    for (const auto& layer : layers) {
        auto offset = FrameNodeAdoptedRenderAdapter::GetBounds(layer).GetOffset();
        point = point - Offset(offset.GetX(), offset.GetY());
        point = FrameNodeAdoptedRenderAdapter::GetTransform(layer) * point;
    }
}

void FrameNodeAdoptedCompensation::RevertAdoptedCompensationFromPoint(const FrameNode* frameNode, PointF& point)
{
    Point transformedPoint(point.GetX(), point.GetY());
    RevertAdoptedCompensationFromPoint(frameNode, transformedPoint);
    point.SetX(transformedPoint.GetX());
    point.SetY(transformedPoint.GetY());
}

void FrameNodeAdoptedCompensation::ApplyAdoptedCompensationToCenterAndSize(
    const FrameNode* frameNode, PointF& center, float& width, float& height)
{
    if (!HasAdoptedCompensation(frameNode)) {
        return;
    }
    ApplyAdoptedCompensationToPoint(frameNode, center);
    std::vector<FrameNodeAdoptedRenderAdapter::RSNodePtr> layers;
    if (FrameNodeAdoptedRenderAdapter::CollectLayers(frameNode, layers) && !layers.empty()) {
        for (const auto& layer : layers) {
            auto scale = FrameNodeAdoptedRenderAdapter::GetScale(layer);
            width *= scale.x;
            height *= scale.y;
        }
        return;
    }
    auto transform = GetAdoptedCompensationTransform(frameNode);
    auto origin = transform * Point();
    auto right = transform * Point(width, 0.0);
    auto bottom = transform * Point(0.0, height);
    width = std::hypot(right.GetX() - origin.GetX(), right.GetY() - origin.GetY());
    height = std::hypot(bottom.GetX() - origin.GetX(), bottom.GetY() - origin.GetY());
}

OffsetF FrameNodeAdoptedCompensation::GetTransformedAdoptedCompensationOffset(const FrameNode* frameNode)
{
    std::vector<FrameNodeAdoptedRenderAdapter::RSNodePtr> layers;
    if (FrameNodeAdoptedRenderAdapter::CollectLayers(frameNode, layers) && !layers.empty()) {
        OffsetF result;
        for (const auto& layer : layers) {
            result += FrameNodeAdoptedRenderAdapter::GetPaintRect(layer).GetOffset();
        }
        return result;
    }
    auto point = GetAdoptedCompensationTransform(frameNode) * Point();
    return OffsetF(point.GetX(), point.GetY());
}

std::pair<OffsetF, bool> FrameNodeAdoptedCompensation::GetAdoptedCompensationOffsetWithTranslate(
    const FrameNode* frameNode)
{
    std::vector<FrameNodeAdoptedRenderAdapter::RSNodePtr> layers;
    if (!FrameNodeAdoptedRenderAdapter::CollectLayers(frameNode, layers) || layers.empty()) {
        return { GetAdoptedCompensationOffset(frameNode),
            HasAdoptedCompensationNonTranslationTransform(frameNode) };
    }
    OffsetF result;
    bool hasNonTranslationTransform = false;
    for (const auto& layer : layers) {
        auto bounds = FrameNodeAdoptedRenderAdapter::GetBounds(layer);
        auto translate = FrameNodeAdoptedRenderAdapter::GetTranslate(layer);
        result += bounds.GetOffset() + translate;
        auto matrix = FrameNodeAdoptedRenderAdapter::GetTransform(layer);
        matrix.Set(TRANSLATION_X_ROW, TRANSLATION_COLUMN, 0.0);
        matrix.Set(TRANSLATION_Y_ROW, TRANSLATION_COLUMN, 0.0);
        hasNonTranslationTransform = hasNonTranslationTransform || !matrix.IsIdentityMatrix();
    }
    return { result, hasNonTranslationTransform };
}

VectorF FrameNodeAdoptedCompensation::GetAdoptedCompensationScale(const FrameNode* frameNode)
{
    std::vector<FrameNodeAdoptedRenderAdapter::RSNodePtr> layers;
    if (FrameNodeAdoptedRenderAdapter::CollectLayers(frameNode, layers) && !layers.empty()) {
        VectorF result(1.0f, 1.0f);
        for (const auto& layer : layers) {
            auto scale = FrameNodeAdoptedRenderAdapter::GetScale(layer);
            result.x *= scale.x;
            result.y *= scale.y;
        }
        return result;
    }
    DecomposedTransform transform;
    if (!TransformUtil::DecomposeTransform(transform, GetAdoptedCompensationTransform(frameNode))) {
        return VectorF(1.0f, 1.0f);
    }
    return VectorF(static_cast<float>(transform.scale[0]), static_cast<float>(transform.scale[1]));
}
} // namespace OHOS::Ace::NG

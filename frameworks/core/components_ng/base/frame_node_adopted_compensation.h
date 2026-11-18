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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_BASE_FRAME_NODE_ADOPTED_COMPENSATION_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_BASE_FRAME_NODE_ADOPTED_COMPENSATION_H

#include <utility>
#include <vector>

#include "base/utils/macros.h"

namespace OHOS::Ace {
class Matrix4;
class Point;

namespace NG {
class FrameNode;
template<typename T>
class OffsetT;
template<typename T>
class PointT;
template<typename T>
class RectT;
struct VectorF;

using OffsetF = OffsetT<float>;
using PointF = PointT<float>;
using RectF = RectT<float>;

class ACE_FORCE_EXPORT FrameNodeAdoptedCompensation final {
public:
    // Checks only whether the overlay-position compensation policy is enabled. Use this when validating whether an
    // adopted child accepts compensation updates; a null node reports false.
    static bool UseAdoptedCompensation(const FrameNode* frameNode);

    // Enables compensation when an adopted relationship is established, or clears all compensation state when the
    // relationship or policy is removed. A null node is ignored.
    static void SetUseAdoptedCompensation(FrameNode* frameNode, bool useAdoptedCompensation);

    // Stores the caller-computed layout offset and aggregate transform. Calculations which these values can represent
    // continue to use them; only calculations requiring per-layer semantics inspect the actual RSNode parent chain.
    static void SetAdoptedCompensation(FrameNode* frameNode, const OffsetF& offset, const Matrix4& transform);

    // Checks both the adopted relationship and the policy flag. Coordinate consumers use this before applying d or M;
    // a null node reports false.
    static bool HasAdoptedCompensation(const FrameNode* frameNode);

    // Returns the caller-computed transform-free accumulated offset.
    static OffsetF GetAdoptedCompensationOffset(const FrameNode* frameNode);

    // Returns the caller-computed aggregate transform.
    static Matrix4 GetAdoptedCompensationTransform(const FrameNode* frameNode);

    // Detects rotation, scale, skew, or other non-translation entries in M. Use this to decide whether a
    // translation-only coordinate result is lossy; translation entries alone do not trigger the result.
    static bool HasAdoptedCompensationNonTranslationTransform(const FrameNode* frameNode);

    // Checks every skipped RSNode layer independently, so transforms which cancel in an aggregate matrix are retained.
    static bool HasAdoptedCompensationUnsupportedTransform(const FrameNode* frameNode, bool checkScale);

    // Checks whether any skipped RSNode layer changes its paint rect. Unlike the non-translation check, x/y transform
    // translation is significant because normal HasRenderTransform compares transformed and untransformed paint rects.
    static bool HasAdoptedCompensationPaintRectTransform(const FrameNode* frameNode);

    // Returns the sum of every skipped layer's transformed paint-rect offset because an aggregate transform cannot
    // reproduce that per-layer accumulation. Falls back to the caller transform when no skipped RSNode layer exists.
    static OffsetF GetTransformedAdoptedCompensationOffset(const FrameNode* frameNode);

    // Replays GetPaintRectWithTranslate for skipped RSNode layers: the offset contains layout position and x/y
    // transform translation, and the bool reports whether a non-translation transform makes the result lossy.
    static std::pair<OffsetF, bool> GetAdoptedCompensationOffsetWithTranslate(const FrameNode* frameNode);

    // Multiplies the scale of every skipped RSNode layer because aggregate decomposition loses layer semantics.
    static VectorF GetAdoptedCompensationScale(const FrameNode* frameNode);

    // Applies the caller-computed aggregate transform to double-precision point collections.
    static void ApplyAdoptedCompensationToPoints(const FrameNode* frameNode, std::vector<Point>& points);

    // Applies the caller-computed aggregate transform to float point collections.
    static void ApplyAdoptedCompensationToPoints(const FrameNode* frameNode, std::vector<PointF>& points);

    // Replays the normal FrameNode visible-rect calculation for every skipped RSNode layer; an aggregate matrix cannot
    // reproduce its repeated scale-and-transformed-paint-offset behavior.
    static void ApplyAdoptedCompensationToRect(const FrameNode* frameNode, RectF& rect);

    // Applies M to a double-precision point while walking from an adopted child toward window coordinates.
    static void ApplyAdoptedCompensationToPoint(const FrameNode* frameNode, Point& point);

    // Applies M to a float point used by selection, drag, magnifier, and other overlay coordinate consumers.
    static void ApplyAdoptedCompensationToPoint(const FrameNode* frameNode, PointF& point);

    // Applies inverse(M) to a double-precision point while converting window coordinates back toward a child.
    static void RevertAdoptedCompensationFromPoint(const FrameNode* frameNode, Point& point);

    // Replays ConvertPositionToWindow(fromWindow=true) for skipped layers. That legacy path subtracts each layout
    // offset and then applies the layer's forward visual matrix; it is not the mathematical inverse of M.
    static void ApplyAdoptedCompensationToPointFromWindow(const FrameNode* frameNode, Point& point);

    // Applies inverse(M) to a float point used by reverse hit-test and overlay coordinate conversion paths.
    static void RevertAdoptedCompensationFromPoint(const FrameNode* frameNode, PointF& point);

    // Applies the caller aggregate transform to the center, while deriving width and height from per-layer scale when
    // skipped RSNode layers exist.
    static void ApplyAdoptedCompensationToCenterAndSize(
        const FrameNode* frameNode, PointF& center, float& width, float& height);

private:
    FrameNodeAdoptedCompensation() = delete;
    ~FrameNodeAdoptedCompensation() = delete;
};
} // namespace NG
} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_BASE_FRAME_NODE_ADOPTED_COMPENSATION_H

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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_INTERACTION_OBSERVER_UI_INTERACTION_JSON_BUILDER_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_INTERACTION_OBSERVER_UI_INTERACTION_JSON_BUILDER_H

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace OHOS::Ace {
class JsonValue;

namespace NG {

// Plain screen-space point (physical pixels) used by the interaction event
// serialization, kept free of framework geometry types so the builder can be
// unit tested in isolation.
struct ArkUIInteractionPoint {
    double x = 0.0;
    double y = 0.0;
};

// Parameters for the Pinch event payload; "scale" is written only when set.
struct ArkUIInteractionPinchParam {
    int32_t nodeId = -1;
    std::vector<ArkUIInteractionPoint> points;
    int32_t fingers = 0;
    std::string action;
    std::optional<double> scale;
};

// Parameters for the Rotation event payload; "angle" is written only when set.
struct ArkUIInteractionRotationParam {
    int32_t nodeId = -1;
    std::vector<ArkUIInteractionPoint> points;
    int32_t fingers = 0;
    std::string action;
    std::optional<double> angle;
};

// Parameters for the Swipe event payload; speeds are physical px/s.
struct ArkUIInteractionSwipeParam {
    int32_t nodeId = -1;
    std::vector<ArkUIInteractionPoint> downPoints;
    std::vector<ArkUIInteractionPoint> upPoints;
    uint32_t direction = 0;
    double speed = 0.0;
    double actualSpeed = 0.0;
};

// Parameters for the Drag end payload; "id" (drop target) is written only for
// a successful drop with a known target.
struct ArkUIInteractionDragEndParam {
    ArkUIInteractionPoint point;
    std::string hostName;
    bool dropSuccess = false;
    std::optional<int32_t> targetId;
};

// Serializes interaction events delivered to NDK interaction observers.
// Field semantics follow the inspector JsonReport payloads
// (frameworks/core/components_ng/manager/event/json_child_report.cpp) with the
// schema changes required by the public contract: the top level always carries
// "schemaVersion": 1 and "type": "<event type>", and all coordinates are
// screen physical pixels. Differences kept on purpose:
// - Pan/Pinch/Rotation "cancel" payloads also carry id/point (and direction or
//   fingers) as required by the interaction observer specification, while the
//   inspector report omits them.
// - Swipe speed values stay in physical px/s (no vp conversion).
class ArkUIInteractionJsonBuilder {
public:
    // type is "Tap" or "Click"; carries id, point, count, fingers.
    static std::string BuildTapOrClickJson(bool isClick, int32_t nodeId, const ArkUIInteractionPoint& point,
        int32_t count, int32_t fingers);

    // Carries id, point, actualDuration (milliseconds), action ("end").
    static std::string BuildLongPressJson(int32_t nodeId, const ArkUIInteractionPoint& point,
        int64_t actualDurationMs);

    // Carries id, point, direction (PanDirection mask), action
    // ("start" | "end" | "cancel").
    static std::string BuildPanJson(int32_t nodeId, const ArkUIInteractionPoint& point, uint32_t direction,
        const std::string& action);

    // point is the per-finger screen position array; carries id, point,
    // fingers, action; scale is present only on "end".
    static std::string BuildPinchJson(const ArkUIInteractionPinchParam& param);

    // Carries id, point (per-finger array), fingers, action; angle in degrees
    // is present only on "end".
    static std::string BuildRotationJson(const ArkUIInteractionRotationParam& param);

    // Carries id, downPoint and upPoint (per-finger arrays), direction,
    // speed, actualSpeed (physical px/s).
    static std::string BuildSwipeJson(const ArkUIInteractionSwipeParam& param);

    // Carries action ("down" | "up"), fingerId, point; "down" also carries id
    // (hit node ID), "up" never does.
    static std::string BuildTouchJson(bool isDown, int32_t fingerId, const ArkUIInteractionPoint& point,
        bool hasNodeId, int32_t nodeId);

    // Carries id, point, hostName, actualDuration (milliseconds),
    // action ("start").
    static std::string BuildDragStartJson(int32_t nodeId, const ArkUIInteractionPoint& point,
        const std::string& hostName, int64_t actualDurationMs);

    // Carries point, dropResult ("success" | "fail"), hostName,
    // action ("end"); id (drop target) is present only on success.
    static std::string BuildDragEndJson(const ArkUIInteractionDragEndParam& param);

private:
    static void PutPointArray(const std::unique_ptr<JsonValue>& array,
        const std::vector<ArkUIInteractionPoint>& points);
};

} // namespace NG
} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_INTERACTION_OBSERVER_UI_INTERACTION_JSON_BUILDER_H

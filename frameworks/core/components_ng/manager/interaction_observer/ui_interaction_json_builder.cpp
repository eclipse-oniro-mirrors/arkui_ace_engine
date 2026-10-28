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

#include "ui_interaction_json_builder.h"

#include "base/json/json_util.h"

namespace OHOS::Ace::NG {
namespace {

constexpr int32_t INTERACTION_SCHEMA_VERSION = 1;

void PutHeader(const std::shared_ptr<JsonValue>& value, const char* type)
{
    value->Put("schemaVersion", INTERACTION_SCHEMA_VERSION);
    value->Put("type", type);
}

std::unique_ptr<JsonValue> CreatePointValue(const ArkUIInteractionPoint& point)
{
    auto pointValue = JsonUtil::CreateArray();
    pointValue->Put(point.x);
    pointValue->Put(point.y);
    return pointValue;
}

std::unique_ptr<JsonValue> CreatePointArrayValue(const std::vector<ArkUIInteractionPoint>& points)
{
    auto arrayValue = JsonUtil::CreateArray();
    for (const auto& point : points) {
        auto pointValue = CreatePointValue(point);
        arrayValue->Put(pointValue);
    }
    return arrayValue;
}

} // namespace

void ArkUIInteractionJsonBuilder::PutPointArray(
    const std::unique_ptr<JsonValue>& array, const std::vector<ArkUIInteractionPoint>& points)
{
    for (const auto& point : points) {
        auto pointValue = CreatePointValue(point);
        array->Put(pointValue);
    }
}

std::string ArkUIInteractionJsonBuilder::BuildTapOrClickJson(bool isClick, int32_t nodeId,
    const ArkUIInteractionPoint& point, int32_t count, int32_t fingers)
{
    auto value = JsonUtil::CreateSharedPtrJson();
    PutHeader(value, isClick ? "Click" : "Tap");
    value->Put("id", nodeId);
    value->Put("point", CreatePointValue(point));
    value->Put("count", count);
    value->Put("fingers", fingers);
    return value->ToString();
}

std::string ArkUIInteractionJsonBuilder::BuildLongPressJson(
    int32_t nodeId, const ArkUIInteractionPoint& point, int64_t actualDurationMs)
{
    auto value = JsonUtil::CreateSharedPtrJson();
    PutHeader(value, "LongPress");
    value->Put("id", nodeId);
    value->Put("action", "end");
    value->Put("point", CreatePointValue(point));
    value->Put("actualDuration", actualDurationMs);
    return value->ToString();
}

std::string ArkUIInteractionJsonBuilder::BuildPanJson(int32_t nodeId, const ArkUIInteractionPoint& point,
    uint32_t direction, const std::string& action)
{
    auto value = JsonUtil::CreateSharedPtrJson();
    PutHeader(value, "Pan");
    value->Put("id", nodeId);
    value->Put("action", action.c_str());
    value->Put("point", CreatePointValue(point));
    value->Put("direction", static_cast<int64_t>(direction));
    return value->ToString();
}

std::string ArkUIInteractionJsonBuilder::BuildPinchJson(const ArkUIInteractionPinchParam& param)
{
    auto value = JsonUtil::CreateSharedPtrJson();
    PutHeader(value, "Pinch");
    value->Put("id", param.nodeId);
    value->Put("action", param.action.c_str());
    auto pointValue = CreatePointArrayValue(param.points);
    value->Put("point", pointValue);
    value->Put("fingers", param.fingers);
    if (param.scale.has_value()) {
        value->Put("scale", param.scale.value());
    }
    return value->ToString();
}

std::string ArkUIInteractionJsonBuilder::BuildRotationJson(const ArkUIInteractionRotationParam& param)
{
    auto value = JsonUtil::CreateSharedPtrJson();
    PutHeader(value, "Rotation");
    value->Put("id", param.nodeId);
    value->Put("action", param.action.c_str());
    auto pointValue = CreatePointArrayValue(param.points);
    value->Put("point", pointValue);
    value->Put("fingers", param.fingers);
    if (param.angle.has_value()) {
        value->Put("angle", param.angle.value());
    }
    return value->ToString();
}

std::string ArkUIInteractionJsonBuilder::BuildSwipeJson(const ArkUIInteractionSwipeParam& param)
{
    auto value = JsonUtil::CreateSharedPtrJson();
    PutHeader(value, "Swipe");
    value->Put("id", param.nodeId);
    auto upValue = CreatePointArrayValue(param.upPoints);
    value->Put("upPoint", upValue);
    // Keep the JsonReport fallback: without recorded down events the up
    // positions are reported as the down positions.
    if (param.downPoints.empty()) {
        value->Put("downPoint", upValue);
    } else {
        auto downValue = CreatePointArrayValue(param.downPoints);
        value->Put("downPoint", downValue);
    }
    value->Put("direction", static_cast<int64_t>(param.direction));
    value->Put("speed", param.speed);
    value->Put("actualSpeed", param.actualSpeed);
    return value->ToString();
}

std::string ArkUIInteractionJsonBuilder::BuildTouchJson(
    bool isDown, int32_t fingerId, const ArkUIInteractionPoint& point, bool hasNodeId, int32_t nodeId)
{
    auto value = JsonUtil::CreateSharedPtrJson();
    PutHeader(value, "Touch");
    value->Put("action", isDown ? "down" : "up");
    if (isDown && hasNodeId) {
        value->Put("id", nodeId);
    }
    value->Put("fingerId", fingerId);
    value->Put("point", CreatePointValue(point));
    return value->ToString();
}

std::string ArkUIInteractionJsonBuilder::BuildDragStartJson(int32_t nodeId, const ArkUIInteractionPoint& point,
    const std::string& hostName, int64_t actualDurationMs)
{
    auto value = JsonUtil::CreateSharedPtrJson();
    PutHeader(value, "Drag");
    value->Put("action", "start");
    value->Put("id", nodeId);
    value->Put("point", CreatePointValue(point));
    value->Put("hostName", hostName.c_str());
    value->Put("actualDuration", actualDurationMs);
    return value->ToString();
}

std::string ArkUIInteractionJsonBuilder::BuildDragEndJson(const ArkUIInteractionDragEndParam& param)
{
    auto value = JsonUtil::CreateSharedPtrJson();
    PutHeader(value, "Drag");
    value->Put("action", "end");
    value->Put("point", CreatePointValue(param.point));
    value->Put("dropResult", param.dropSuccess ? "success" : "fail");
    if (param.dropSuccess && param.targetId.has_value()) {
        value->Put("id", param.targetId.value());
    }
    value->Put("hostName", param.hostName.c_str());
    return value->ToString();
}

} // namespace OHOS::Ace::NG

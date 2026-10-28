/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "core/components_ng/property/gradient_property.h"
#include "base/json/json_util.h"
#include "base/log/log.h"
#include "base/utils/string_utils.h"

namespace OHOS::Ace::NG {

void Gradient::AddColor(const GradientColor& color)
{
    colors_.push_back(color);
}

void Gradient::ClearColors()
{
    colors_.clear();
}

void Gradient::CreateGradientWithType(GradientType type)
{
    type_ = type;
    switch (type_) {
        case GradientType::LINEAR:
            linearGradient_ = std::make_shared<LinearGradient>();
            break;
        case GradientType::RADIAL:
            radialGradient_ = std::make_shared<RadialGradient>();
            break;
        case GradientType::SWEEP:
            sweepGradient_ = std::make_shared<SweepGradient>();
            break;
        default:
            LOGE("GradientType not supported");
    }
}

namespace GradientJsonUtils {

namespace {
void PutTreeDimension(JsonValue& json, const char* key, const std::optional<Dimension>& value, bool angle = false)
{
    if (!value) {
        return;
    }
    if (angle) {
        json.Put(key, value->Value());
    } else {
        json.Put(key, value->ToString().c_str());
    }
}

void PutTreeGradientCenter(JsonValue& json, const char* key, const std::optional<Dimension>& value)
{
    if (!value) {
        return;
    }
    // Gradient centers store percentages on a 0..100 scale; radius dimensions use 0..1.
    const auto text = value->Unit() == DimensionUnit::PERCENT
        ? StringUtils::DoubleToString(value->Value()) + "%" : value->ToString();
    json.Put(key, text.c_str());
}

void PutTreeGradientColors(JsonValue& json, const Gradient& data)
{
    auto colors = JsonUtil::CreateArray();
    for (const auto& stop : data.GetColors()) {
        auto entry = JsonUtil::Create();
        const auto& color = stop.GetColor();
        entry->Put("color", color.ColorToString().c_str());
        entry->Put("colorSpace", static_cast<int32_t>(color.GetColorSpace()));
        if (const auto hdr = color.GetHeadRoomColor()) {
            auto channels = JsonUtil::Create();
            channels->Put("red", hdr->red);
            channels->Put("green", hdr->green);
            channels->Put("blue", hdr->blue);
            channels->Put("alpha", hdr->alpha);
            channels->Put("headRoom", hdr->headRoom);
            entry->PutRef("hdr", std::move(channels));
        }
        if (stop.GetHasValue()) {
            const auto& offset = stop.GetDimension();
            // Gradient stops store percentages on a 0..100 scale, unlike normal Dimension values.
            const auto value = offset.Unit() == DimensionUnit::PERCENT
                ? StringUtils::DoubleToString(offset.Value()) + "%" : offset.ToString();
            entry->Put("offset", value.c_str());
        }
        colors->PutRef(std::move(entry));
    }
    json.Put("repeating", data.GetRepeat());
    json.PutRef("colors", std::move(colors));
}

void PutTreeLinearGradient(JsonValue& json, const Gradient& data)
{
    const auto& linear = data.GetLinearGradient();
    CHECK_NULL_VOID(linear);
    PutTreeDimension(json, "angle", linear->angle, true);
    std::string direction;
    if (linear->linearX == GradientDirection::LEFT) {
        direction = "Left";
    } else if (linear->linearX == GradientDirection::RIGHT) {
        direction = "Right";
    }
    if (linear->linearY == GradientDirection::TOP) {
        direction += "Top";
    } else if (linear->linearY == GradientDirection::BOTTOM) {
        direction += "Bottom";
    }
    if (!direction.empty()) {
        json.Put("direction", ("GradientDirection." + direction).c_str());
    }
}

void PutTreeSweepGradient(JsonValue& json, const Gradient& data)
{
    const auto& sweep = data.GetSweepGradient();
    CHECK_NULL_VOID(sweep);
    PutTreeGradientCenter(json, "centerX", sweep->centerX);
    PutTreeGradientCenter(json, "centerY", sweep->centerY);
    PutTreeDimension(json, "startAngle", sweep->startAngle, true);
    PutTreeDimension(json, "endAngle", sweep->endAngle, true);
    PutTreeDimension(json, "rotation", sweep->rotation, true);
}

void PutTreeRadialGradient(JsonValue& json, const Gradient& data)
{
    const auto& radial = data.GetRadialGradient();
    CHECK_NULL_VOID(radial);
    PutTreeGradientCenter(json, "centerX", radial->radialCenterX);
    PutTreeGradientCenter(json, "centerY", radial->radialCenterY);
    PutTreeDimension(json, "horizontalSize", radial->radialHorizontalSize);
    PutTreeDimension(json, "verticalSize", radial->radialVerticalSize);
    if (radial->radialShape) {
        json.Put("shape", static_cast<int32_t>(*radial->radialShape));
    }
    if (radial->radialSizeType) {
        json.Put("sizeType", static_cast<int32_t>(*radial->radialSizeType));
    }
}
} // namespace

std::unique_ptr<JsonValue> ToTreeJson(const Gradient& data)
{
    auto result = JsonUtil::Create();
    switch (data.GetType()) {
        case GradientType::LINEAR:
            result->Put("type", "linear");
            PutTreeLinearGradient(*result, data);
            break;
        case GradientType::SWEEP:
            result->Put("type", "sweep");
            PutTreeSweepGradient(*result, data);
            break;
        case GradientType::RADIAL:
            result->Put("type", "radial");
            PutTreeRadialGradient(*result, data);
            break;
        default:
            return nullptr;
    }
    PutTreeGradientColors(*result, data);
    return result;
}


static void GetColorsAndRepeating(std::unique_ptr<JsonValue>& resultJson, const Gradient& gradient)
{
    auto jsonColorArray = JsonUtil::CreateArray(true);
    auto colors = gradient.GetColors();
    for (size_t i = 0; i < colors.size(); ++i) {
        auto temp = JsonUtil::CreateArray(true);
        auto value = std::to_string(colors[i].GetDimension().Value() / 100.0);
        auto color = colors[i].GetColor().ColorToString();
        temp->Put("0", color.c_str());
        temp->Put("1", value.c_str());
        auto index = std::to_string(i);
        jsonColorArray->Put(index.c_str(), temp);
    }
    resultJson->Put("colors", jsonColorArray);
    resultJson->Put("repeating", gradient.GetRepeat() ? "true" : "false");
}

std::unique_ptr<JsonValue> LinearGradientToJson(const Gradient& data)
{
    auto resultJson = JsonUtil::Create(true);
    if (GradientType::LINEAR != data.GetType()) {
        return resultJson;
    }
    auto linearGradient = data.GetLinearGradient();
    CHECK_NULL_RETURN(linearGradient, resultJson);
    if (linearGradient->angle.has_value()) {
        resultJson->Put("angle", linearGradient->angle->ToString().c_str());
    }

    auto linearX = linearGradient->linearX;
    auto linearY = linearGradient->linearY;
    if (linearX == GradientDirection::LEFT) {
        if (linearY == GradientDirection::TOP) {
            resultJson->Put("direction", "GradientDirection.LeftTop");
        } else if (linearY == GradientDirection::BOTTOM) {
            resultJson->Put("direction", "GradientDirection.LeftBottom");
        } else {
            resultJson->Put("direction", "GradientDirection.Left");
        }
    } else if (linearX == GradientDirection::RIGHT) {
        if (linearY == GradientDirection::TOP) {
            resultJson->Put("direction", "GradientDirection.RightTop");
        } else if (linearY == GradientDirection::BOTTOM) {
            resultJson->Put("direction", "GradientDirection.RightBottom");
        } else {
            resultJson->Put("direction", "GradientDirection.Right");
        }
    } else {
        if (linearY == GradientDirection::TOP) {
            resultJson->Put("direction", "GradientDirection.Top");
        } else if (linearY == GradientDirection::BOTTOM) {
            resultJson->Put("direction", "GradientDirection.Bottom");
        } else {
            resultJson->Put("direction", "GradientDirection.None");
        }
    }
    GetColorsAndRepeating(resultJson, data);
    return resultJson;
}

std::unique_ptr<JsonValue> SweepGradientToJson(const Gradient& data)
{
    auto resultJson = JsonUtil::Create(true);

    if (GradientType::SWEEP != data.GetType()) {
        return resultJson;
    }
    auto sweepGradient = data.GetSweepGradient();
    CHECK_NULL_RETURN(sweepGradient, resultJson);
    auto radialCenterX = sweepGradient->centerX;
    auto radialCenterY = sweepGradient->centerY;
    if (radialCenterX && radialCenterY) {
        auto jsPoint = JsonUtil::CreateArray(true);
        jsPoint->Put("0", radialCenterX->ToString().c_str());
        jsPoint->Put("1", radialCenterY->ToString().c_str());
        resultJson->Put("center", jsPoint);
    }

    auto startAngle = sweepGradient->startAngle;
    auto endAngle = sweepGradient->endAngle;
    if (startAngle) {
        resultJson->Put("start", startAngle->ToString().c_str());
    }
    if (endAngle) {
        resultJson->Put("end", endAngle->ToString().c_str());
    }

    GetColorsAndRepeating(resultJson, data);

    return resultJson;
}

std::unique_ptr<JsonValue> RadialGradientToJson(const Gradient& data)
{
    auto resultJson = JsonUtil::Create(true);
    if (GradientType::RADIAL != data.GetType()) {
        return resultJson;
    }
    auto radialGradient = data.GetRadialGradient();
    CHECK_NULL_RETURN(radialGradient, resultJson);

    auto radialCenterX = radialGradient->radialCenterX;
    auto radialCenterY = radialGradient->radialCenterY;
    if (radialCenterX && radialCenterY) {
        auto jsPoint = JsonUtil::CreateArray(true);
        jsPoint->Put("0", radialCenterX->ToString().c_str());
        jsPoint->Put("1", radialCenterY->ToString().c_str());
        resultJson->Put("center", jsPoint);
    }

    auto radius = radialGradient->radialVerticalSize;
    if (radius) {
        resultJson->Put("radius", radius->ToString().c_str());
    }

    GetColorsAndRepeating(resultJson, data);

    return resultJson;
}
} // namespace GradientJsonUtils

} // namespace OHOS::Ace::NG

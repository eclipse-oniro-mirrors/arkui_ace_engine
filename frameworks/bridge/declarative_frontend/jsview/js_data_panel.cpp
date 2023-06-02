/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "bridge/declarative_frontend/jsview/js_data_panel.h"

#include <vector>

#include "bridge/declarative_frontend/jsview/js_interactable_view.h"
#include "bridge/declarative_frontend/jsview/js_linear_gradient.h"
#include "bridge/declarative_frontend/jsview/js_utils.h"
#include "bridge/declarative_frontend/jsview/models/data_panel_model_impl.h"
#include "core/components/data_panel/data_panel_theme.h"
#include "core/components_ng/pattern/data_panel/data_panel_model_ng.h"

namespace OHOS::Ace {

std::unique_ptr<DataPanelModel> DataPanelModel::instance_ = nullptr;
std::mutex DataPanelModel::mutex_;

DataPanelModel* DataPanelModel::GetInstance()
{
    if (!instance_) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!instance_) {
#ifdef NG_BUILD
            instance_.reset(new NG::DataPanelModelNG());
#else
            if (Container::IsCurrentUseNewPipeline()) {
                instance_.reset(new NG::DataPanelModelNG());
            } else {
                instance_.reset(new Framework::DataPanelModelImpl());
            }
#endif
        }
    }
    return instance_.get();
}

} // namespace OHOS::Ace
namespace OHOS::Ace::Framework {

constexpr size_t MAX_COUNT = 9;

void JSDataPanel::JSBind(BindingTarget globalObj)
{
    JSClass<JSDataPanel>::Declare("DataPanel");
    JSClass<JSDataPanel>::StaticMethod("create", &JSDataPanel::Create);
    JSClass<JSDataPanel>::StaticMethod("closeEffect", &JSDataPanel::CloseEffect);

    JSClass<JSDataPanel>::StaticMethod("valueColors", &JSDataPanel::ValueColors);
    JSClass<JSDataPanel>::StaticMethod("trackBackgroundColor", &JSDataPanel::TrackBackground);
    JSClass<JSDataPanel>::StaticMethod("strokeWidth", &JSDataPanel::StrokeWidth);
    JSClass<JSDataPanel>::StaticMethod("trackShadow", &JSDataPanel::ShadowOption);

    JSClass<JSDataPanel>::StaticMethod("onClick", &JSInteractableView::JsOnClick);
    JSClass<JSDataPanel>::StaticMethod("onTouch", &JSInteractableView::JsOnTouch);
    JSClass<JSDataPanel>::StaticMethod("onKeyEvent", &JSInteractableView::JsOnKey);
    JSClass<JSDataPanel>::StaticMethod("onDeleteEvent", &JSInteractableView::JsOnDelete);
    JSClass<JSDataPanel>::StaticMethod("onAppear", &JSInteractableView::JsOnAppear);
    JSClass<JSDataPanel>::StaticMethod("onDisAppear", &JSInteractableView::JsOnDisAppear);
    JSClass<JSDataPanel>::StaticMethod("remoteMessage", &JSInteractableView::JsCommonRemoteMessage);

    JSClass<JSDataPanel>::Inherit<JSViewAbstract>();
    JSClass<JSDataPanel>::Bind(globalObj);
}

void JSDataPanel::Create(const JSCallbackInfo& info)
{
    if (info.Length() < 1 || !info[0]->IsObject()) {
        LOGE("toggle create error, info is non-valid");
        return;
    }
    auto param = JsonUtil::ParseJsonString(info[0]->ToString());
    if (!param || param->IsNull()) {
        LOGE("JSDataPanel::Create param is null");
        return;
    }
    // max
    auto max = param->GetDouble("max", 100.0);
    // values
    auto values = param->GetValue("values");
    if (!values || !values->IsArray()) {
        LOGE("JSDataPanel::Create values is not array");
        return;
    }
    auto type = param->GetValue("type");
    size_t length = static_cast<size_t>(values->GetArraySize());
    std::vector<double> dateValues;
    double dataSum = 0.0;
    for (size_t i = 0; i < length && i < MAX_COUNT; i++) {
        auto item = values->GetArrayItem(i);
        if (!item || !item->IsNumber()) {
            LOGE("JSDataPanel::Create value is not number");
            continue;
        }
        auto value = item->GetDouble();
        if (LessOrEqual(value, 0.0)) {
            value = 0.0;
        }
        dataSum += value;
        if (GreatOrEqual(dataSum, max) && max > 0) {
            value = max - (dataSum - value);
            if (NearEqual(value, 0.0)) {
                break;
            }
        }
        dateValues.emplace_back(value);
    }
    if (LessOrEqual(max, 0.0)) {
        max = dataSum;
    }
    size_t dataPanelType = 0;
    if (type->IsNumber()) {
        if (type->GetInt() == static_cast<int32_t>(ChartType::LINE)) {
            dataPanelType = 1;
        } else if (type->GetInt() == static_cast<int32_t>(ChartType::RAINBOW)) {
            dataPanelType = 0;
        }
    }
    DataPanelModel::GetInstance()->Create(dateValues, max, dataPanelType);
}

void JSDataPanel::CloseEffect(const JSCallbackInfo& info)
{
    bool isCloseEffect = true;
    if (info[0]->IsBoolean()) {
        isCloseEffect = info[0]->ToBoolean();
    }
    DataPanelModel::GetInstance()->SetEffect(isCloseEffect);
}

void JSDataPanel::ValueColors(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        return;
    }

    std::vector<OHOS::Ace::NG::Gradient> valueColors;
    if (!info[0]->IsArray() || info[0]->IsEmpty()) {
        ConvertThemeColor(valueColors);
        DataPanelModel::GetInstance()->SetValueColors(valueColors);
        return;
    }

    auto paramArray = JSRef<JSArray>::Cast(info[0]);
    size_t length = paramArray->Length();
    for (size_t i = 0; i < length && i < MAX_COUNT; i++) {
        auto item = paramArray->GetValueAt(i);
        OHOS::Ace::NG::Gradient gradient;
        if (!ConvertGradientColor(item, gradient)) {
            valueColors.clear();
            ConvertThemeColor(valueColors);
            break;
        }
        valueColors.emplace_back(gradient);
    }
    DataPanelModel::GetInstance()->SetValueColors(valueColors);
}

void JSDataPanel::TrackBackground(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        return;
    }
    Color color;
    if (!ParseJsColor(info[0], color)) {
        RefPtr<DataPanelTheme> theme = GetTheme<DataPanelTheme>();
        color = theme->GetBackgroundColor();
    }

    DataPanelModel::GetInstance()->SetTrackBackground(color);
}

void JSDataPanel::StrokeWidth(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        return;
    }

    RefPtr<DataPanelTheme> theme = GetTheme<DataPanelTheme>();
    CalcDimension strokeWidthDimension;
    if (!ParseJsDimensionVp(info[0], strokeWidthDimension)) {
        strokeWidthDimension = theme->GetThickness();
    }

    // If the parameter value is string(''), parse result 0.
    // The value of 0 is allowed, but the value of string('') is not allowed, so use theme value.
    if (info[0]->IsString() && info[0]->ToString().empty()) {
        strokeWidthDimension = theme->GetThickness();
    }

    if (strokeWidthDimension.IsNegative() || strokeWidthDimension.Unit() == DimensionUnit::PERCENT) {
        strokeWidthDimension = theme->GetThickness();
    }
    DataPanelModel::GetInstance()->SetStrokeWidth(strokeWidthDimension);
}

void JSDataPanel::ShadowOption(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        return;
    }

    auto paramObject = JSRef<JSObject>::Cast(info[0]);
    JSRef<JSVal> jsRadius = paramObject->GetProperty("radius");
    JSRef<JSVal> jsOffsetX = paramObject->GetProperty("offsetX");
    JSRef<JSVal> jsOffsetY = paramObject->GetProperty("offsetY");

    RefPtr<DataPanelTheme> theme = GetTheme<DataPanelTheme>();
    double radius = 0.0;
    if (!ParseJsDouble(jsRadius, radius)) {
        radius = theme->GetTrackShadowRadius().ConvertToVp();
    }

    if (NonPositive(radius)) {
        radius = theme->GetTrackShadowRadius().ConvertToVp();
    }

    double offsetX = 0.0;
    if (!ParseJsDouble(jsOffsetX, offsetX)) {
        offsetX = theme->GetTrackShadowOffsetX().ConvertToVp();
    }

    double offsetY = 0.0;
    if (!ParseJsDouble(jsOffsetY, offsetY)) {
        offsetY = theme->GetTrackShadowOffsetY().ConvertToVp();
    }

    std::vector<OHOS::Ace::NG::Gradient> shadowColors;
    auto colors = paramObject->GetProperty("colors");
    if (!colors->IsEmpty() && colors->IsArray()) {
        auto colorsArray = JSRef<JSArray>::Cast(colors);
        for (size_t i = 0; i < colorsArray->Length(); i++) {
            auto item = colorsArray->GetValueAt(i);
            OHOS::Ace::NG::Gradient gradient;
            if (!ConvertGradientColor(item, gradient)) {
                shadowColors.clear();
                ConvertThemeColor(shadowColors);
                break;
            }
            shadowColors.emplace_back(gradient);
        }
    }

    OHOS::Ace::NG::DataPanelShadow shadow;
    shadow.radius = radius;
    shadow.offsetX = offsetX;
    shadow.offsetY = offsetY;
    shadow.colors = shadowColors;
    DataPanelModel::GetInstance()->SetShadowOption(shadow);
}

bool JSDataPanel::ConvertGradientColor(const JsiRef<JsiValue>& itemParam, OHOS::Ace::NG::Gradient& gradient)
{
    if (!itemParam->IsObject()) {
        return ConvertResourceColor(itemParam, gradient);
    }

    JSLinearGradient* jsLinearGradient = JSRef<JSObject>::Cast(itemParam)->Unwrap<JSLinearGradient>();
    if (!jsLinearGradient) {
        return ConvertResourceColor(itemParam, gradient);
    }

    size_t colorLength = jsLinearGradient->GetGradient().size();
    for (size_t colorIndex = 0; colorIndex < colorLength; colorIndex++) {
        OHOS::Ace::NG::GradientColor gradientColor;
        gradientColor.SetLinearColor(LinearColor(jsLinearGradient->GetGradient().at(colorIndex).first));
        gradientColor.SetDimension(jsLinearGradient->GetGradient().at(colorIndex).second);
        gradient.AddColor(gradientColor);
    }
    return true;
}

bool JSDataPanel::ConvertResourceColor(const JsiRef<JsiValue>& itemParam, OHOS::Ace::NG::Gradient& gradient)
{
    Color color;
    if (!ParseJsColor(itemParam, color)) {
        LOGE("ParseJsColor error.");
        return false;
    }
    OHOS::Ace::NG::GradientColor gradientColorStart;
    gradientColorStart.SetLinearColor(LinearColor(color));
    gradientColorStart.SetDimension(Dimension(0.0));
    gradient.AddColor(gradientColorStart);
    OHOS::Ace::NG::GradientColor gradientColorEnd;
    gradientColorEnd.SetLinearColor(LinearColor(color));
    gradientColorEnd.SetDimension(Dimension(1.0));
    gradient.AddColor(gradientColorEnd);
    return true;
}

void JSDataPanel::ConvertThemeColor(std::vector<OHOS::Ace::NG::Gradient>& colors)
{
    RefPtr<DataPanelTheme> theme = GetTheme<DataPanelTheme>();
    auto themeColors = theme->GetColorsArray();
    for (const auto& item : themeColors) {
        OHOS::Ace::NG::Gradient gradient;
        OHOS::Ace::NG::GradientColor gradientColorStart;
        gradientColorStart.SetLinearColor(LinearColor(item.first));
        gradientColorStart.SetDimension(Dimension(0.0));
        gradient.AddColor(gradientColorStart);
        OHOS::Ace::NG::GradientColor gradientColorEnd;
        gradientColorEnd.SetLinearColor(LinearColor(item.second));
        gradientColorEnd.SetDimension(Dimension(1.0));
        gradient.AddColor(gradientColorEnd);
        colors.emplace_back(gradient);
    }
}
} // namespace OHOS::Ace::Framework

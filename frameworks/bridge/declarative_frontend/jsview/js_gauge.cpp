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

#include "frameworks/bridge/declarative_frontend/jsview/js_gauge.h"

#include "bridge/declarative_frontend/view_stack_processor.h"
#include "core/components/chart/chart_component.h"
#include "core/components/progress/progress_component.h"

namespace OHOS::Ace::Framework {

void JSGauge::JSBind(BindingTarget globalObj)
{
    JSClass<JSGauge>::Declare("Gauge");
    JSClass<JSGauge>::StaticMethod("create", &JSGauge::Create);

    JSClass<JSGauge>::StaticMethod("value", &JSGauge::SetValue);
    JSClass<JSGauge>::StaticMethod("startAngle", &JSGauge::SetStartAngle);
    JSClass<JSGauge>::StaticMethod("endAngle", &JSGauge::SetEndAngle);
    JSClass<JSGauge>::StaticMethod("colors", &JSGauge::SetColors);
    JSClass<JSGauge>::StaticMethod("strokeWidth", &JSGauge::SetStrokeWidth);
    JSClass<JSGauge>::StaticMethod("labelTextConfig", &JSGauge::SetLableTextConfig);
    JSClass<JSGauge>::StaticMethod("labelColorConfig", &JSGauge::SetLableColorConfig);

    JSClass<JSGauge>::Inherit<JSViewAbstract>();
    JSClass<JSGauge>::Bind(globalObj);
}

void JSGauge::Create(const JSCallbackInfo& info)
{
    if (info.Length() < 1 && !info[0]->IsObject()) {
        LOGI("gauge create error, info is non-vaild");
        return;
    }

    auto paramObject = JSRef<JSObject>::Cast(info[0]);
    auto value = paramObject->GetProperty("value");
    auto min = paramObject->GetProperty("min");
    auto max = paramObject->GetProperty("max");

    double gaugeMin = min->IsNumber() ? min->ToNumber<double>() : 0;
    double gaugeMax = max->IsNumber() ? max->ToNumber<double>() : 100;
    double gaugeValue = value->IsNumber() ? value->ToNumber<double>() : 0;
    auto progressChild =
        AceType::MakeRefPtr<ProgressComponent>(gaugeMin, gaugeValue, 0.0, gaugeMax, ProgressType::GAUGE);
    progressChild->SetIndicatorFlag(true);
    ViewStackProcessor::GetInstance()->Push(progressChild);
    RefPtr<ProgressTheme> progressTheme = GetTheme<ProgressTheme>();
    progressChild->InitStyle(progressTheme);
}

void JSGauge::SetValue(const JSCallbackInfo& info)
{
    if (info.Length() < 1 && !info[0]->IsNumber()) {
        LOGE("JSGauge::SetValue::The info is wrong, it is supposed to have atleast 1 arguments");
        return;
    }
    auto component = ViewStackProcessor::GetInstance()->GetMainComponent();
    auto gaugeComponent = AceType::DynamicCast<ProgressComponent>(component);
    if (!gaugeComponent) {
        return;
    }
    gaugeComponent->SetValue(info[0]->ToNumber<double>());
}

void JSGauge::SetStartAngle(const JSCallbackInfo& info)
{
    if (info.Length() < 1 && !info[0]->IsNumber()) {
        LOGE("JSGauge::SetStartAngle::The info is wrong, it is supposed to have atleast 1 arguments");
        return;
    }
    auto component = ViewStackProcessor::GetInstance()->GetMainComponent();
    auto gaugeComponent = AceType::DynamicCast<ProgressComponent>(component);
    if (!gaugeComponent) {
        return;
    }
    gaugeComponent->GetTrack()->GetTrackInfo()->SetStartDegree(info[0]->ToNumber<double>());
}

void JSGauge::SetEndAngle(const JSCallbackInfo& info)
{
    if (info.Length() < 1 && !info[0]->IsNumber()) {
        LOGE("JSGauge::SetEndAngle::The info is wrong, it is supposed to have atleast 1 arguments");
        return;
    }
    auto component = ViewStackProcessor::GetInstance()->GetMainComponent();
    auto gaugeComponent = AceType::DynamicCast<ProgressComponent>(component);
    if (!gaugeComponent) {
        return;
    }
    gaugeComponent->GetTrack()->GetTrackInfo()->SetSweepDegree(info[0]->ToNumber<double>());
}

void JSGauge::SetColors(const JSCallbackInfo& info)
{
    if (info.Length() < 1 && !info[0]->IsNumber()) {
        LOGE("JSGauge::SetEndAngle::The info is wrong, it is supposed to have atleast 1 arguments");
        return;
    }
    std::vector<Color> colors;
    std::vector<double> values;
    auto jsColor = JSRef<JSArray>::Cast(info[0]);
    for (size_t i = 0; i < jsColor->Length(); ++i) {
        JSRef<JSArray> tempColors = jsColor->GetValueAt(i);
        double value = tempColors->GetValueAt(1)->ToNumber<double>();
        Color selectedColor;
        if (!ParseJsColor(tempColors->GetValueAt(0), selectedColor)) {
            return;
        }
        colors.push_back(selectedColor);
        values.push_back(value);
    }
    auto component = ViewStackProcessor::GetInstance()->GetMainComponent();
    auto gaugeComponent = AceType::DynamicCast<ProgressComponent>(component);
    if (!gaugeComponent) {
        return;
    }
    gaugeComponent->SetSectionsStyle(colors, values);
}

void JSGauge::SetStrokeWidth(const JSCallbackInfo& info)
{
    if (info.Length() < 1 && !info[0]->IsNumber()) {
        LOGE(" JSGauge::SetStrokeWidth::The info is wrong, it is supposed to have atleast 1 arguments");
        return;
    }
    Dimension strokeWidth;
    if (!ParseJsDimensionVp(info[0], strokeWidth)) {
        return;
    }
    auto component = ViewStackProcessor::GetInstance()->GetMainComponent();
    auto gaugeComponent = AceType::DynamicCast<ProgressComponent>(component);
    if (!gaugeComponent) {
        return;
    }
    gaugeComponent->SetTrackThickness(strokeWidth);
}

void JSGauge::SetLableTextConfig(const JSCallbackInfo& info)
{
    if (!info[0]->IsObject() || info.Length() < 1) {
        LOGI("JSGauge::SetLableTextConfig::The info is wrong, it is supposed to have atleast 1 arguments");
        return;
    }
    auto paramObject = JSRef<JSObject>::Cast(info[0]);
    auto markedValue = paramObject->GetProperty("markedLabelText");
    auto component = ViewStackProcessor::GetInstance()->GetMainComponent();
    auto gaugeComponent = AceType::DynamicCast<ProgressComponent>(component);

    if (markedValue->IsString()) {
        gaugeComponent->SetLableMarkedText(markedValue->ToString());
    }
}

void JSGauge::SetLableColorConfig(const JSCallbackInfo& info)
{
    if (!info[0]->IsObject() || info.Length() < 1) {
        LOGI("JSGauge::SetLableColorConfig::The info is wrong, it is supposed to have atleast 1 arguments");
        return;
    }
    auto paramObject = JSRef<JSObject>::Cast(info[0]);
    auto markedLabelColor = paramObject->GetProperty("markedLabelColor");
    auto component = ViewStackProcessor::GetInstance()->GetMainComponent();
    auto gaugeComponent = AceType::DynamicCast<ProgressComponent>(component);
    Color currentColor;

    if (ParseJsColor(markedLabelColor, currentColor)) {
        gaugeComponent->SetMarkedTextColor(currentColor);
    }
}

} // namespace OHOS::Ace::Framework
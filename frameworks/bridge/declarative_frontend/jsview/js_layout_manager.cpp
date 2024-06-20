/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <string>

#include "frameworks/bridge/common/utils/engine_helper.h"
#include "frameworks/bridge/declarative_frontend/engine/js_converter.h"
#include "frameworks/bridge/declarative_frontend/jsview/js_layout_manager.h"
#include "frameworks/bridge/declarative_frontend/jsview/js_text.h"
#include "native_engine/impl/ark/ark_native_engine.h"

namespace OHOS::Ace::Framework {
const int32_t ARG_NUMBER = 2;

void JSLayoutManager::GetLineCount(const JSCallbackInfo& args)
{
    auto layoutInfoInterface = layoutInfoInterface_.Upgrade();
    CHECK_NULL_VOID(layoutInfoInterface);
    auto lineCount = layoutInfoInterface->GetLineCount();
    args.SetReturnValue(JSRef<JSVal>::Make(ToJSValue(lineCount)));
}

void JSLayoutManager::GetLineMetrics(const JSCallbackInfo& args)
{
    auto layoutInfoInterface = layoutInfoInterface_.Upgrade();
    CHECK_NULL_VOID(layoutInfoInterface);
    int32_t lineNumber = 0;
    JSViewAbstract::ParseJsInteger<int32_t>(args[0], lineNumber);
    auto lineMetrics = layoutInfoInterface->GetLineMetrics(lineNumber);
    JSRef<JSObject> lineMetricsObj = JSRef<JSObject>::New();
    CreateJSLineMetrics(lineMetricsObj, lineMetrics);
    args.SetReturnValue(JSRef<JSVal>::Cast(lineMetricsObj));
}

void JSLayoutManager::CreateJSLineMetrics(JSRef<JSObject>& lineMetricsObj, const TextLineMetrics& lineMetrics)
{
    lineMetricsObj->SetProperty<int32_t>("startIndex", lineMetrics.startIndex);
    lineMetricsObj->SetProperty<int32_t>("endIndex",  lineMetrics.endIndex);
    lineMetricsObj->SetProperty<double>("ascent",  lineMetrics.ascender);
    lineMetricsObj->SetProperty<double>("descent", lineMetrics.descender);
    lineMetricsObj->SetProperty<double>("height",  lineMetrics.height);
    lineMetricsObj->SetProperty<double>("width",  lineMetrics.width);
    lineMetricsObj->SetProperty<double>("left",  lineMetrics.x);
    lineMetricsObj->SetProperty<double>("baseline",  lineMetrics.baseline);
    lineMetricsObj->SetProperty<int32_t>("lineNumber",  lineMetrics.lineNumber);
    lineMetricsObj->SetProperty<double>("topHeight",  lineMetrics.y);
    lineMetricsObj->SetPropertyObject("runMetrics", ConvertMapToJSMap(lineMetrics.runMetrics));
}

NapiMap JSLayoutManager::CreateNapiMap(napi_env env)
{
    NapiMap res = { nullptr, nullptr };
    napi_valuetype valueType;

    napi_value global = nullptr;
    CHECK_NULL_RETURN(!napi_get_global(env, &global) && global, res);

    napi_value constructor = nullptr;
    CHECK_NULL_RETURN(!napi_get_named_property(env, global, "Map", &constructor) && constructor, res);
    CHECK_NULL_RETURN(!napi_typeof(env, constructor, &valueType) && valueType == napi_valuetype::napi_function, res);

    napi_value mapInstance = nullptr;
    CHECK_NULL_RETURN(!napi_new_instance(env, constructor, 0, nullptr, &mapInstance) && mapInstance, res);

    napi_value mapSet = nullptr;
    CHECK_NULL_RETURN(!napi_get_named_property(env, mapInstance, "set", &mapSet) && mapSet, res);
    CHECK_NULL_RETURN(!napi_typeof(env, mapSet, &valueType) && valueType == napi_valuetype::napi_function, res);

    res.instance = mapInstance;
    res.setFunction = mapSet;

    return res;
}

bool JSLayoutManager::NapiMapSet(napi_env& env, NapiMap& map, uint32_t key, const RunMetrics& runMetrics)
{
    napi_value keyValue = nullptr;
    napi_create_int32(env, static_cast<int32_t>(key), &keyValue);
    napi_value runMetricsValue = nullptr;
    napi_create_object(env, &runMetricsValue);
    napi_set_named_property(env, runMetricsValue, "textStyle",
        JsConverter::ConvertJsValToNapiValue(CreateJSTextStyleResult(runMetrics.textStyle)));
    napi_set_named_property(env, runMetricsValue, "fontMetrics",
        JsConverter::ConvertJsValToNapiValue(CreateJSFontMetrics(runMetrics.fontMetrics)));
    napi_value args[ARG_NUMBER] = { keyValue, runMetricsValue };
    CHECK_NULL_RETURN(!napi_call_function(env, map.instance, map.setFunction, ARG_NUMBER, args, nullptr), false);
    return true;
}

JSRef<JSVal> JSLayoutManager::ConvertMapToJSMap(const std::map<size_t, RunMetrics>& map)
{
    JSRef<JSVal> mapVal;
    auto engine = EngineHelper::GetCurrentEngine();
    CHECK_NULL_RETURN(engine, mapVal);
    NativeEngine* nativeEngine = engine->GetNativeEngine();
    CHECK_NULL_RETURN(nativeEngine, mapVal);
    napi_env env = reinterpret_cast<napi_env>(nativeEngine);
    auto mapReturn = CreateNapiMap(env);
    for (const auto& [key, val] : map) {
        NapiMapSet(env, mapReturn, static_cast<uint32_t>(key), val);
    }
    mapVal = JsConverter::ConvertNapiValueToJsVal(mapReturn.instance);
    return mapVal;
}

JSRef<JSObject> JSLayoutManager::CreateJSRunMetrics(const RunMetrics& runMetrics)
{
    auto runMetricsObj = JSRef<JSObject>::New();
    runMetricsObj->SetPropertyObject("textStyle", CreateJSTextStyleResult(runMetrics.textStyle));
    runMetricsObj->SetPropertyObject("fontMetrics", CreateJSFontMetrics(runMetrics.fontMetrics));
    return runMetricsObj;
}

JSRef<JSObject> JSLayoutManager::CreateJSFontMetrics(const FontMetrics& fontMetrics)
{
    auto fontMetricsObj = JSRef<JSObject>::New();
    fontMetricsObj->SetProperty<uint32_t>("flags", fontMetrics.fFlags);
    fontMetricsObj->SetProperty<double>("top", fontMetrics.fTop);
    fontMetricsObj->SetProperty<double>("ascent", fontMetrics.fAscent);
    fontMetricsObj->SetProperty<double>("descent", fontMetrics.fDescent);
    fontMetricsObj->SetProperty<double>("bottom", fontMetrics.fBottom);
    fontMetricsObj->SetProperty<double>("leading", fontMetrics.fLeading);
    fontMetricsObj->SetProperty<double>("avgCharWidth", fontMetrics.fAvgCharWidth);
    fontMetricsObj->SetProperty<double>("maxCharWidth", fontMetrics.fMaxCharWidth);
    fontMetricsObj->SetProperty<double>("xMin", fontMetrics.fXMin);
    fontMetricsObj->SetProperty<double>("xMax", fontMetrics.fXMax);
    fontMetricsObj->SetProperty<double>("xHeight", fontMetrics.fXHeight);
    fontMetricsObj->SetProperty<double>("capHeight", fontMetrics.fCapHeight);
    fontMetricsObj->SetProperty<double>("underlineThickness", fontMetrics.fUnderlineThickness);
    fontMetricsObj->SetProperty<double>("underlinePosition", fontMetrics.fUnderlinePosition);
    fontMetricsObj->SetProperty<double>("strikethroughThickness", fontMetrics.fStrikeoutThickness);
    fontMetricsObj->SetProperty<double>("strikethroughPosition", fontMetrics.fStrikeoutPosition);
    return fontMetricsObj;
}

JSRef<JSObject> JSLayoutManager::CreateJSTextStyleResult(const TextStyle& textStyle)
{
    JSRef<JSObject> textStyleObj = JSRef<JSObject>::New();
    textStyleObj->SetProperty<int32_t>("decoration", static_cast<int32_t>(textStyle.GetTextDecoration()));
    textStyleObj->SetProperty<uint32_t>("color", textStyle.GetTextColor().GetValue());
    textStyleObj->SetProperty<int32_t>("fontWeight", static_cast<int32_t>(textStyle.GetFontWeight()));
    textStyleObj->SetProperty<int32_t>("fontStyle", static_cast<int32_t>(textStyle.GetFontStyle()));
    textStyleObj->SetProperty<int32_t>("baseline", static_cast<int32_t>(textStyle.GetTextBaseline()));
    JSRef<JSArray> fontFamiliesArray = JSRef<JSArray>::New();
    int32_t index = 0;
    for (const auto& item : textStyle.GetFontFamilies()) {
        fontFamiliesArray->SetValueAt(index, JSRef<JSVal>::Make(ToJSValue(item)));
        index++;
    }
    textStyleObj->SetPropertyObject("fontFamilies", fontFamiliesArray);
    textStyleObj->SetProperty<double>("fontSize", textStyle.GetFontSize().Value());

    textStyleObj->SetProperty<double>("letterSpacing", textStyle.GetLetterSpacing().Value());
    textStyleObj->SetProperty<double>("wordSpacing", textStyle.GetWordSpacing().Value());
    textStyleObj->SetProperty<double>("heightScale", textStyle.GetHeightScale());
    textStyleObj->SetProperty<int32_t>("halfLeading", textStyle.GetHalfLeading());
    textStyleObj->SetProperty<int32_t>("heightOnly", textStyle.GetHeightOnly());
    textStyleObj->SetProperty<std::string>("ellipsis", StringUtils::Str16ToStr8(textStyle.GetEllipsis()));
    textStyleObj->SetProperty<int32_t>("ellipsisMode", static_cast<int32_t>(textStyle.GetEllipsisMode()));
    textStyleObj->SetProperty<std::string>("locale", textStyle.GetLocale());
    return textStyleObj;
}

void JSLayoutManager::GetGlyphPositionAtCoordinate(const JSCallbackInfo& args)
{
    if (args.Length() < 2) { // 2:At least two parameters
        TAG_LOGE(AceLogTag::ACE_RICH_TEXT, "Info length error.");
        return;
    }
    auto layoutInfoInterface = layoutInfoInterface_.Upgrade();
    CHECK_NULL_VOID(layoutInfoInterface);
    int32_t coordinateX = 0;
    int32_t coordinateY = 0;
    JSContainerBase::ParseJsInt32(args[0], coordinateX);
    JSContainerBase::ParseJsInt32(args[1], coordinateY);
    auto value = layoutInfoInterface->GetGlyphPositionAtCoordinate(coordinateX, coordinateY);
    auto positionWithAffinityObj = JSRef<JSObject>::New();
    positionWithAffinityObj->SetPropertyObject("position", JSRef<JSVal>::Make(ToJSValue(value.position_)));
    auto affinity = JSRef<JSVal>::Make(ToJSValue(static_cast<int32_t>(value.affinity_)));
    positionWithAffinityObj->SetPropertyObject("affinity", affinity);
    args.SetReturnValue(JSRef<JSVal>::Cast(positionWithAffinityObj));
}

void JSLayoutManager::JSBind(BindingTarget globalObj)
{
    JSClass<JSLayoutManager>::Declare("LayoutManager");
    JSClass<JSLayoutManager>::CustomMethod("getLineCount", &JSLayoutManager::GetLineCount);
    JSClass<JSLayoutManager>::CustomMethod(
        "getGlyphPositionAtCoordinate", &JSLayoutManager::GetGlyphPositionAtCoordinate);
    JSClass<JSLayoutManager>::CustomMethod("getLineMetrics", &JSLayoutManager::GetLineMetrics);
    JSClass<JSLayoutManager>::Bind(globalObj, JSLayoutManager::Constructor, JSLayoutManager::Destructor);
}
} // namespace OHOS::Ace::Framework
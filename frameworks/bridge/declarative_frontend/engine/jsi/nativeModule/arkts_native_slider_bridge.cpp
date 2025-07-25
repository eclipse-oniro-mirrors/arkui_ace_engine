/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "bridge/declarative_frontend/engine/jsi/nativeModule/arkts_native_slider_bridge.h"
#include "bridge/declarative_frontend/engine/jsi/nativeModule/arkts_utils.h"
#include "bridge/declarative_frontend/jsview/js_shape_abstract.h"
#include "bridge/declarative_frontend/jsview/js_linear_gradient.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/slider/slider_model_ng.h"
#include "frameworks/bridge/common/utils/engine_helper.h"
#include "frameworks/bridge/declarative_frontend/engine/jsi/nativeModule/arkts_utils.h"

namespace OHOS::Ace::NG {
constexpr int NUM_0 = 0;
constexpr int NUM_1 = 1;
constexpr int NUM_2 = 2;
constexpr int32_t NUM_3 = 3;
constexpr int32_t NUM_4 = 4;
constexpr int32_t NUM_5 = 5;
constexpr int32_t NUM_6 = 6;
constexpr int NUM_8 = 8;
constexpr int SLIDER_MIN = 0;
constexpr int SLIDER_MAX = 100;
constexpr int PARAM_ARR_LENGTH_2 = 2;
const char* SLIDER_NODEPTR_OF_UINODE = "nodePtr_";
constexpr char STEPS_STRING[] = "stepsAccessibility";
constexpr char ENTRIES_STRING[] = "entries";
constexpr char NEXT_STRING[] = "next";
constexpr char VALUE_STRING[] = "value";
constexpr char TEXT_STRING[] = "text";
constexpr char DONE_STRING[] = "done";
namespace {
bool ConvertSliderGradientColor(const EcmaVM* vm, const Local<JSValueRef>& value, OHOS::Ace::NG::Gradient& gradient)
{
    if (!value->IsObject(vm)) {
        return false;
    }
    Framework::JSLinearGradient* jsLinearGradient =
        static_cast<Framework::JSLinearGradient*>(value->ToObject(vm)->GetNativePointerField(vm, 0));
    if (!jsLinearGradient) {
        return false;
    }

    size_t colorLength = jsLinearGradient->GetGradient().size();
    if (colorLength == 0) {
        return false;
    }
    for (size_t colorIndex = 0; colorIndex < colorLength; ++colorIndex) {
        OHOS::Ace::NG::GradientColor gradientColor;
        gradientColor.SetLinearColor(LinearColor(jsLinearGradient->GetGradient().at(colorIndex).first));
        gradientColor.SetDimension(jsLinearGradient->GetGradient().at(colorIndex).second);
        gradient.AddColor(gradientColor);
    }
    return true;
}
} // namespace

panda::Local<panda::JSValueRef> JsSliderChangeCallback(panda::JsiRuntimeCallInfo* runtimeCallInfo)
{
    auto vm = runtimeCallInfo->GetVM();
    int32_t argc = static_cast<int32_t>(runtimeCallInfo->GetArgsNumber());
    if (argc != NUM_2) {
        return panda::JSValueRef::Undefined(vm);
    }
    auto valueArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto modeArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    if (!valueArg->IsNumber() || !modeArg->IsNumber()) {
        return panda::JSValueRef::Undefined(vm);
    }
    double value = valueArg->ToNumber(vm)->Value();
    int32_t mode = modeArg->ToNumber(vm)->Value();
    auto ref = runtimeCallInfo->GetThisRef();
    auto obj = ref->ToObject(vm);
    if (obj->GetNativePointerFieldCount(vm) < NUM_1) {
        return panda::JSValueRef::Undefined(vm);
    }
    auto frameNode = static_cast<FrameNode*>(obj->GetNativePointerField(vm, 0));
    CHECK_NULL_RETURN(frameNode, panda::JSValueRef::Undefined(vm));
    SliderModelNG::SetChangeValue(frameNode, value, mode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue SliderBridge::SetShowTips(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> showTipArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    Local<JSValueRef> contentArg = runtimeCallInfo->GetCallArgRef(NUM_2);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());

    bool showTips = false;
    if (showTipArg->IsBoolean()) {
        showTips = showTipArg->ToBoolean(vm)->Value();
    }
    
    std::string content;
    RefPtr<ResourceObject> strResObj;
    if (ArkTSUtils::ParseJsString(vm, contentArg, content, strResObj)) {
        auto strRawPtr = AceType::RawPtr(strResObj);
        GetArkUINodeModifiers()->getSliderModifier()->setShowTipsPtr(nativeNode, showTips, content.c_str(), strRawPtr);
    } else {
        GetArkUINodeModifiers()->getSliderModifier()->setShowTipsPtr(nativeNode, showTips, nullptr, nullptr);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue SliderBridge::ResetShowTips(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getSliderModifier()->resetShowTips(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue SliderBridge::SetSliderStepSize(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    CalcDimension stepSize;
    if (!ArkTSUtils::ParseJsDimensionVpNG(vm, secondArg, stepSize, false)) {
        GetArkUINodeModifiers()->getSliderModifier()->resetSliderStepSize(nativeNode);
        return panda::JSValueRef::Undefined(vm);
    } else {
        if (LessNotEqual(stepSize.Value(), 0.0)) {
            GetArkUINodeModifiers()->getSliderModifier()->resetSliderStepSize(nativeNode);
            return panda::JSValueRef::Undefined(vm);
        }

        GetArkUINodeModifiers()->getSliderModifier()->setSliderStepSize(
            nativeNode, stepSize.Value(), static_cast<int>(stepSize.Unit()));
    }

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue SliderBridge::ResetSliderStepSize(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getSliderModifier()->resetSliderStepSize(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue SliderBridge::SetBlockSize(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    Local<JSValueRef> thirdArg = runtimeCallInfo->GetCallArgRef(NUM_2);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());

    CalcDimension blockWidth;
    CalcDimension blockHeight;
    bool hasBlockWidth = ArkTSUtils::ParseJsDimensionVp(vm, secondArg, blockWidth);
    bool hasBlockHeight = ArkTSUtils::ParseJsDimensionVp(vm, thirdArg, blockHeight);
    if (!hasBlockWidth && !hasBlockHeight) {
        GetArkUINodeModifiers()->getSliderModifier()->resetBlockSize(nativeNode);
        return panda::JSValueRef::Undefined(vm);
    }

    GetArkUINodeModifiers()->getSliderModifier()->setBlockSize(nativeNode,
        blockWidth.Value(), static_cast<int>(blockWidth.Unit()),
        blockHeight.Value(), static_cast<int>(blockHeight.Unit()));
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue SliderBridge::ResetBlockSize(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getSliderModifier()->resetBlockSize(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue SliderBridge::SetTrackBorderRadius(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());

    CalcDimension trackBorderRadius;
    if (!ArkTSUtils::ParseJsDimensionVpNG(vm, secondArg, trackBorderRadius, true)) {
        GetArkUINodeModifiers()->getSliderModifier()->resetTrackBorderRadius(nativeNode);
        return panda::JSValueRef::Undefined(vm);
    }

    if (LessNotEqual(trackBorderRadius.Value(), 0.0)) {
        GetArkUINodeModifiers()->getSliderModifier()->resetTrackBorderRadius(nativeNode);
        return panda::JSValueRef::Undefined(vm);
    }

    GetArkUINodeModifiers()->getSliderModifier()->setTrackBorderRadius(nativeNode,
        trackBorderRadius.Value(), static_cast<int>(trackBorderRadius.Unit()));
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue SliderBridge::ResetTrackBorderRadius(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getSliderModifier()->resetTrackBorderRadius(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue SliderBridge::SetStepColor(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    Color color;
    RefPtr<ResourceObject> colorResObj;
    auto nodeInfo = ArkTSUtils::MakeNativeNodeInfo(nativeNode);
    if (!ArkTSUtils::ParseJsColorAlpha(vm, secondArg, color, colorResObj, nodeInfo)) {
        GetArkUINodeModifiers()->getSliderModifier()->resetStepColor(nativeNode);
    } else {
        auto colorRawPtr = AceType::RawPtr(colorResObj);
        GetArkUINodeModifiers()->getSliderModifier()->setStepColorPtr(nativeNode, color.GetValue(), colorRawPtr);
    }

    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue SliderBridge::ResetStepColor(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getSliderModifier()->resetStepColor(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue SliderBridge::SetBlockBorderColor(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    Color color;
    RefPtr<ResourceObject> colorResObj;
    auto nodeInfo = ArkTSUtils::MakeNativeNodeInfo(nativeNode);
    if (!ArkTSUtils::ParseJsColorAlpha(vm, secondArg, color, colorResObj, nodeInfo)) {
        GetArkUINodeModifiers()->getSliderModifier()->resetBlockBorderColor(nativeNode);
    } else {
        auto colorRawPtr = AceType::RawPtr(colorResObj);
        GetArkUINodeModifiers()->getSliderModifier()->setBlockBorderColorPtr(nativeNode, color.GetValue(), colorRawPtr);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue SliderBridge::ResetBlockBorderColor(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getSliderModifier()->resetBlockBorderColor(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue SliderBridge::SetBlockBorderWidth(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());

    CalcDimension blockBorderWidth;
    if (!ArkTSUtils::ParseJsDimensionVp(vm, secondArg, blockBorderWidth)) {
        GetArkUINodeModifiers()->getSliderModifier()->resetBlockBorderWidth(nativeNode);
        return panda::JSValueRef::Undefined(vm);
    }

    if (LessNotEqual(blockBorderWidth.Value(), 0.0)) {
        GetArkUINodeModifiers()->getSliderModifier()->resetBlockBorderWidth(nativeNode);
        return panda::JSValueRef::Undefined(vm);
    }

    GetArkUINodeModifiers()->getSliderModifier()->setBlockBorderWidth(nativeNode,
        blockBorderWidth.Value(), static_cast<int>(blockBorderWidth.Unit()));
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue SliderBridge::ResetBlockBorderWidth(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getSliderModifier()->resetBlockBorderWidth(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue SliderBridge::SetBlockColor(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    Color color;
    RefPtr<ResourceObject> colorResObj;
    auto nodeInfo = ArkTSUtils::MakeNativeNodeInfo(nativeNode);
    if (!ArkTSUtils::ParseJsColorAlpha(vm, secondArg, color, colorResObj, nodeInfo)) {
        GetArkUINodeModifiers()->getSliderModifier()->resetBlockColor(nativeNode);
    } else {
        auto colorRawPtr = AceType::RawPtr(colorResObj);
        GetArkUINodeModifiers()->getSliderModifier()->setBlockColorPtr(nativeNode, color.GetValue(), colorRawPtr);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue SliderBridge::ResetBlockColor(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getSliderModifier()->resetBlockColor(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue SliderBridge::SetTrackBackgroundColor(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    Gradient gradient;
    Color color;
    RefPtr<ResourceObject> colorResObj;
    auto nodeInfo = ArkTSUtils::MakeNativeNodeInfo(nativeNode);
    if (ConvertSliderGradientColor(vm, secondArg, gradient)) {
        ArkUIGradientType gradientObj;
        auto colorLength = gradient.GetColors().size();
        std::vector<uint32_t> colorValues;
        std::vector<ArkUILengthType> offsetValues;
        GetArkUINodeModifiers()->getSliderModifier()->resetTrackBackgroundColor(nativeNode);
        if (colorLength <= 0) {
            return panda::JSValueRef::Undefined(vm);
        }
        for (int32_t i = 0; i < static_cast<int32_t>(colorLength); i++) {
            colorValues.push_back(gradient.GetColors()[i].GetLinearColor().GetValue());
            offsetValues.push_back(ArkUILengthType {
                .number = static_cast<ArkUI_Float32>(gradient.GetColors()[i].GetDimension().Value()),
                .unit = static_cast<int8_t>(gradient.GetColors()[i].GetDimension().Unit()) });
        }

        gradientObj.color = &(*colorValues.begin());
        gradientObj.offset = &(*offsetValues.begin());
        GetArkUINodeModifiers()->getSliderModifier()->setLinearTrackBackgroundColor(
            nativeNode, &gradientObj, colorLength);
    } else if (ArkTSUtils::ParseJsColorAlpha(vm, secondArg, color, colorResObj, nodeInfo)) {
        auto colorRawPtr = AceType::RawPtr(colorResObj);
        GetArkUINodeModifiers()->getSliderModifier()->setTrackBackgroundColorPtr(
            nativeNode, color.GetValue(), colorRawPtr);
    } else {
        GetArkUINodeModifiers()->getSliderModifier()->resetTrackBackgroundColor(nativeNode);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue SliderBridge::ResetTrackBackgroundColor(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getSliderModifier()->resetTrackBackgroundColor(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue SliderBridge::SetSelectColor(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    Gradient gradient;
    Color color;
    RefPtr<ResourceObject> colorResObj;
    auto nodeInfo = ArkTSUtils::MakeNativeNodeInfo(nativeNode);
    if (ConvertSliderGradientColor(vm, secondArg, gradient)) {
        ArkUIGradientType gradientObj;
        auto colorLength = gradient.GetColors().size();
        std::vector<uint32_t> colorValues;
        std::vector<ArkUILengthType> offsetValues;
        if (colorLength <= 0) {
            GetArkUINodeModifiers()->getSliderModifier()->resetSelectColor(nativeNode);
            return panda::JSValueRef::Undefined(vm);
        }

        for (int32_t i = 0; i < static_cast<int32_t>(colorLength); i++) {
            colorValues.push_back(gradient.GetColors()[i].GetLinearColor().GetValue());
            offsetValues.push_back(ArkUILengthType {
                .number = static_cast<ArkUI_Float32>(gradient.GetColors()[i].GetDimension().Value()),
                .unit = static_cast<int8_t>(gradient.GetColors()[i].GetDimension().Unit()) });
        }

        gradientObj.color = &(*colorValues.begin());
        gradientObj.offset = &(*offsetValues.begin());
        GetArkUINodeModifiers()->getSliderModifier()->setLinearSelectColor(nativeNode, &gradientObj, colorLength);
    } else if (ArkTSUtils::ParseJsColorAlpha(vm, secondArg, color, colorResObj, nodeInfo)) {
        auto colorRawPtr = AceType::RawPtr(colorResObj);
        GetArkUINodeModifiers()->getSliderModifier()->setSelectColorPtr(nativeNode, color.GetValue(), colorRawPtr);
    } else {
        GetArkUINodeModifiers()->getSliderModifier()->resetSelectColor(nativeNode);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue SliderBridge::ResetSelectColor(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getSliderModifier()->resetSelectColor(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue SliderBridge::SetShowSteps(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    bool flag = secondArg->ToBoolean(vm)->Value();
    Framework::JsiCallbackInfo info = Framework::JsiCallbackInfo(runtimeCallInfo);
    if ((info.Length() < NUM_3) || (!info[NUM_2]->IsObject())) {
        GetArkUINodeModifiers()->getSliderModifier()->setShowSteps(nativeNode, flag);
        return panda::JSValueRef::Undefined(vm);
    }
    StepOptions optionsMap{};
    auto optionsObject = Framework::JSRef<Framework::JSObject>::Cast(info[NUM_2]);
    Framework::JSRef<Framework::JSVal> jsOptionsMap = optionsObject->GetProperty(STEPS_STRING);
    if (jsOptionsMap->IsObject()) {
        SliderBridge::ParseStepOptionsMap(vm, jsOptionsMap, optionsMap);
    }
    if (optionsMap.size() > 0) {
        auto optionLength = NUM_0;
        auto options = new ArkUISliderShowStepOptions[optionsMap.size()];
        for (auto& option : optionsMap) {
            options[optionLength].step = option.first;
            options[optionLength].text = option.second.c_str();
            optionLength++;
        }
        GetArkUINodeModifiers()->getSliderModifier()->setShowStepsWithOptions(nativeNode, flag, options, optionLength);
        delete[] options;
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue SliderBridge::ResetShowSteps(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getSliderModifier()->resetShowSteps(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue SliderBridge::SetThickness(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());

    CalcDimension thickness;
    if (!ArkTSUtils::ParseJsDimensionVpNG(vm, secondArg, thickness, true)) {
        GetArkUINodeModifiers()->getSliderModifier()->resetThickness(nativeNode);
        return panda::JSValueRef::Undefined(vm);
    }

    GetArkUINodeModifiers()->getSliderModifier()->setThickness(nativeNode,
        thickness.Value(), static_cast<int>(thickness.Unit()));
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue SliderBridge::ResetThickness(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getSliderModifier()->resetThickness(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue SliderBridge::SetBlockStyle(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    auto* frameNode = reinterpret_cast<FrameNode*>(nativeNode);
    Framework::JsiCallbackInfo info = Framework::JsiCallbackInfo(runtimeCallInfo);
    if (!info[1]->IsObject()) {
        SliderBridge::ResetBlockStyle(runtimeCallInfo);
        return panda::JSValueRef::Undefined(vm);
    }
    auto jsObj = Framework::JSRef<Framework::JSObject>::Cast(info[1]);
    auto getType = jsObj->GetProperty("type");
    if (getType->IsNull() || !getType->IsNumber()) {
        SliderBridge::ResetBlockStyle(runtimeCallInfo);
        return panda::JSValueRef::Undefined(vm);
    }
    auto type = static_cast<SliderModel::BlockStyleType>(getType->ToNumber<int32_t>());
    if (type == SliderModel::BlockStyleType::IMAGE) {
        std::string src;
        RefPtr<ResourceObject> mediaResObj;
        auto image = jsObj->GetProperty("image");
        if (!Framework::JSShapeAbstract::ParseJsMedia(image, src, mediaResObj)) {
            SliderBridge::ResetBlockStyle(runtimeCallInfo);
            return panda::JSValueRef::Undefined(vm);
        }
        std::string bundleName;
        std::string moduleName;
        Framework::JSViewAbstract::GetJsMediaBundleInfo(image, bundleName, moduleName);
        SliderModelNG::SetBlockImage(frameNode, src, bundleName, moduleName);
        if (SystemProperties::ConfigChangePerform()) {
            SliderModelNG::CreateWithMediaResourceObj(frameNode, mediaResObj, bundleName, moduleName);
        }
    } else if (type == SliderModel::BlockStyleType::SHAPE) {
        auto shape = jsObj->GetProperty("shape");
        if (!shape->IsObject()) {
            SliderBridge::ResetBlockStyle(runtimeCallInfo);
            return panda::JSValueRef::Undefined(vm);
        }
        Framework::JSShapeAbstract* shapeAbstract =
            Framework::JSRef<Framework::JSObject>::Cast(shape)->Unwrap<Framework::JSShapeAbstract>();
        if (shapeAbstract == nullptr) {
            SliderBridge::ResetBlockStyle(runtimeCallInfo);
            return panda::JSValueRef::Undefined(vm);
        }
        SliderModelNG::SetBlockShape(frameNode, shapeAbstract->GetBasicShape());
    }
    SliderModelNG::SetBlockType(frameNode, type);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue SliderBridge::ResetBlockStyle(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    auto* frameNode = reinterpret_cast<FrameNode*>(nativeNode);
    SliderModelNG::ResetBlockType(frameNode);
    SliderModelNG::ResetBlockImage(frameNode);
    SliderModelNG::ResetBlockShape(frameNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue SliderBridge::SetValidSlideRange(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    Framework::JsiCallbackInfo info = Framework::JsiCallbackInfo(runtimeCallInfo);
    if (!info[1]->IsObject()) {
        GetArkUINodeModifiers()->getSliderModifier()->resetSliderValidSlideRange(nativeNode);
        return panda::JSValueRef::Undefined(vm);
    }
    float from = std::numeric_limits<float>::infinity();
    float to = std::numeric_limits<float>::infinity();
    auto jsObj = Framework::JSRef<Framework::JSObject>::Cast(info[1]);
    auto fromType = jsObj->GetProperty("from");
    if (!fromType->IsEmpty() && fromType->IsNumber()) {
        from = fromType->ToNumber<float>();
    }
    auto toType = jsObj->GetProperty("to");
    if (!toType->IsEmpty() && toType->IsNumber()) {
        to = toType->ToNumber<float>();
    }
    if ((std::isinf(from) && std::isinf(to)) || std::isnan(from) || std::isnan(to)) {
        GetArkUINodeModifiers()->getSliderModifier()->resetSliderValidSlideRange(nativeNode);
        return panda::JSValueRef::Undefined(vm);
    }
    auto* frameNode = reinterpret_cast<FrameNode*>(nativeNode);
    SliderModelNG::SetValidSlideRange(frameNode, from, to);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue SliderBridge::ResetValidSlideRange(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getSliderModifier()->resetSliderValidSlideRange(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue SliderBridge::SetSelectedBorderRadius(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nodeArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto nativeNode = nodePtr(nodeArg->ToNativePointer(vm)->Value());

    CalcDimension borderRadius;
    if (!ArkTSUtils::ParseJsDimensionVpNG(vm, secondArg, borderRadius, true)) {
        GetArkUINodeModifiers()->getSliderModifier()->resetSelectedBorderRadius(nativeNode);
        return panda::JSValueRef::Undefined(vm);
    }

    if (LessNotEqual(borderRadius.Value(), 0.0)) {
        GetArkUINodeModifiers()->getSliderModifier()->resetSelectedBorderRadius(nativeNode);
        return panda::JSValueRef::Undefined(vm);
    }

    GetArkUINodeModifiers()->getSliderModifier()->setSelectedBorderRadius(nativeNode,
        borderRadius.Value(), static_cast<int>(borderRadius.Unit()));
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue SliderBridge::ResetSelectedBorderRadius(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nodeArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(nodeArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getSliderModifier()->resetSelectedBorderRadius(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue SliderBridge::SetInteractionMode(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nodeArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto nativeNode = nodePtr(nodeArg->ToNativePointer(vm)->Value());
    if (!secondArg->IsNumber()) {
        GetArkUINodeModifiers()->getSliderModifier()->resetInteractionMode(nativeNode);
        return panda::JSValueRef::Undefined(vm);
    }
    int32_t interactionMode = secondArg->Int32Value(vm);
    GetArkUINodeModifiers()->getSliderModifier()->setInteractionMode(nativeNode, interactionMode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue SliderBridge::ResetInteractionMode(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nodeArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(nodeArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getSliderModifier()->resetInteractionMode(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue SliderBridge::SetMinResponsiveDistance(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nodeArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto nativeNode = nodePtr(nodeArg->ToNativePointer(vm)->Value());
    if (!secondArg->IsNumber()) {
        GetArkUINodeModifiers()->getSliderModifier()->resetMinResponsiveDistance(nativeNode);
        return panda::JSValueRef::Undefined(vm);
    }
    float distance = secondArg->ToNumber(vm)->Value();
    GetArkUINodeModifiers()->getSliderModifier()->setMinResponsiveDistance(nativeNode, distance);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue SliderBridge::ResetMinResponsiveDistance(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nodeArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(nodeArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getSliderModifier()->resetMinResponsiveDistance(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue SliderBridge::SetContentModifierBuilder(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto* frameNode = reinterpret_cast<FrameNode*>(firstArg->ToNativePointer(vm)->Value());
    if (!secondArg->IsObject(vm)) {
        SliderModelNG::SetBuilderFunc(frameNode, nullptr);
        return panda::JSValueRef::Undefined(vm);
    }
    panda::CopyableGlobal<panda::ObjectRef> obj(vm, secondArg);
    auto containerId = Container::CurrentId();
    SliderModelNG::SetBuilderFunc(frameNode,
        [vm, frameNode, obj = std::move(obj), containerId](
            SliderConfiguration config) -> RefPtr<FrameNode> {
            ContainerScope scope(containerId);
            auto context = ArkTSUtils::GetContext(vm);
            const char* keyOfSlider[] = { "value", "min", "max", "step", "enabled", "triggerChange" };
            Local<JSValueRef> valuesOfSlider[] = { panda::NumberRef::New(vm, config.value_),
                panda::NumberRef::New(vm, config.min_), panda::NumberRef::New(vm, config.max_),
                panda::NumberRef::New(vm, config.step_), panda::BooleanRef::New(vm, config.enabled_),
                panda::FunctionRef::New(vm, JsSliderChangeCallback) };
            auto slider = panda::ObjectRef::NewWithNamedProperties(vm, ArraySize(keyOfSlider),
                keyOfSlider, valuesOfSlider);
            slider->SetNativePointerFieldCount(vm, 1);
            slider->SetNativePointerField(vm, 0, static_cast<void*>(frameNode));
            panda::Local<panda::JSValueRef> params[NUM_2] = { context, slider };
            LocalScope pandaScope(vm);
            panda::TryCatch trycatch(vm);
            auto jsObject = obj.ToLocal();
            auto makeFunc = jsObject->Get(vm, panda::StringRef::NewFromUtf8(vm, "makeContentModifierNode"));
            CHECK_EQUAL_RETURN(makeFunc->IsFunction(vm), false, nullptr);
            panda::Local<panda::FunctionRef> func = makeFunc;
            auto result = func->Call(vm, jsObject, params, NUM_2);
            JSNApi::ExecutePendingJob(vm);
            CHECK_EQUAL_RETURN(result.IsEmpty() || trycatch.HasCaught() || !result->IsObject(vm), true, nullptr);
            auto resultObj = result->ToObject(vm);
            panda::Local<panda::JSValueRef> nodeptr =
                resultObj->Get(vm, panda::StringRef::NewFromUtf8(vm, SLIDER_NODEPTR_OF_UINODE));
            CHECK_EQUAL_RETURN(nodeptr.IsEmpty() || nodeptr->IsUndefined() || nodeptr->IsNull(), true, nullptr);
            auto* frameNode = reinterpret_cast<FrameNode*>(nodeptr->ToNativePointer(vm)->Value());
            CHECK_NULL_RETURN(frameNode, nullptr);
            return AceType::Claim(frameNode);
        });
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue SliderBridge::SetOnChange(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    uint32_t argsNumber = runtimeCallInfo->GetArgsNumber();
    if (argsNumber != NUM_2) {
        return panda::JSValueRef::Undefined(vm);
    }
    Local<JSValueRef> nativeNodeArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> callbackArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto nativeNode = nodePtr(nativeNodeArg->ToNativePointer(vm)->Value());
    auto frameNode = reinterpret_cast<FrameNode*>(nativeNode);
    CHECK_NULL_RETURN(frameNode, panda::NativePointerRef::New(vm, nullptr));
    if (callbackArg->IsUndefined() || callbackArg->IsNull() || !callbackArg->IsFunction(vm)) {
        GetArkUINodeModifiers()->getToggleModifier()->resetToggleOnChange(nativeNode);
        return panda::JSValueRef::Undefined(vm);
    }
    panda::Local<panda::FunctionRef> func = callbackArg->ToObject(vm);
    std::function<void(float, int32_t)> callback = [vm, frameNode, func = panda::CopyableGlobal(vm, func)](
                                                       float number, int32_t mode) {
        panda::LocalScope pandaScope(vm);
        panda::TryCatch trycatch(vm);
        PipelineContext::SetCallBackNode(AceType::WeakClaim(frameNode));

        panda::Local<panda::NumberRef> numberParam = panda::NumberRef::New(vm, number);
        panda::Local<panda::NumberRef> modeParam = panda::NumberRef::New(vm, mode);
        panda::Local<panda::JSValueRef> params[PARAM_ARR_LENGTH_2] = { numberParam, modeParam };
        func->Call(vm, func.ToLocal(), params, PARAM_ARR_LENGTH_2);
    };
    GetArkUINodeModifiers()->getSliderModifier()->setOnChange(nativeNode, reinterpret_cast<void*>(&callback));
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue SliderBridge::ResetOnChange(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nativeNodeArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(nativeNodeArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getSliderModifier()->resetOnChange(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

struct SliderOptions {
    double value = 0;
    double min = 0;
    double max = 100;
    double step = 1;
    bool reverse = false;
    int32_t style = 0;
    int32_t direction = 1;
};

static void GetStep(SliderOptions& options)
{
    if (LessOrEqual(options.step, 0.0) || options.step > options.max - options.min) {
        options.step = 1;
    }
}

static void GetValue(SliderOptions& options)
{
    if (options.value < options.min) {
        options.value = options.min;
    }

    if (options.value > options.max) {
        options.value = options.max;
    }
}

static void ParseStyleOptions(ArkUIRuntimeCallInfo* runtimeCallInfo, SliderOptions& options)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_VOID(vm);
    Local<JSValueRef> styleArg = runtimeCallInfo->GetCallArgRef(5);     // 5: parameter index
    Local<JSValueRef> directionArg = runtimeCallInfo->GetCallArgRef(6); // 6: parameter index
    Local<JSValueRef> reverseArg = runtimeCallInfo->GetCallArgRef(7);   // 7: parameter index

    if (!reverseArg.IsNull() && !reverseArg->IsUndefined() && reverseArg->IsBoolean()) {
        options.reverse = reverseArg->ToBoolean(vm)->Value();
    }
    if (!styleArg.IsNull() && !styleArg->IsUndefined() && styleArg->IsNumber()) {
        auto tempStyle = styleArg->Int32Value(vm);
        if (tempStyle >= static_cast<int32_t>(SliderMode::OUTSET) &&
                tempStyle <= static_cast<int32_t>(SliderMode::CAPSULE)) {
            options.style = tempStyle;
        }
    }
    if (!directionArg.IsNull() && !directionArg->IsUndefined() && directionArg->IsNumber()) {
        auto tempDirection = directionArg->Int32Value(vm);
        if (tempDirection >= static_cast<int32_t>(Axis::VERTICAL) &&
                tempDirection <= static_cast<int32_t>(Axis::HORIZONTAL)) {
            options.direction = tempDirection;
        }
    }
}

static void ParseOptions(ArkUIRuntimeCallInfo* runtimeCallInfo, SliderOptions& options)
{
    static const double valueMin = -1000000.0f;
    options.value = valueMin;
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_VOID(vm);
    Local<JSValueRef> valueArg = runtimeCallInfo->GetCallArgRef(1); // 1: parameter index
    Local<JSValueRef> minArg = runtimeCallInfo->GetCallArgRef(2);   // 2: parameter index
    Local<JSValueRef> maxArg = runtimeCallInfo->GetCallArgRef(3);   // 3: parameter index
    Local<JSValueRef> stepArg = runtimeCallInfo->GetCallArgRef(4);  // 4: parameter index
    if (!valueArg.IsNull() && !valueArg->IsUndefined() && valueArg->IsNumber()) {
        options.value = valueArg->ToNumber(vm)->Value();
    }
    if (!minArg.IsNull() && !minArg->IsUndefined() && minArg->IsNumber()) {
        options.min = minArg->ToNumber(vm)->Value();
    }
    if (!maxArg.IsNull() && !maxArg->IsUndefined() && maxArg->IsNumber()) {
        options.max = maxArg->ToNumber(vm)->Value();
    }
    if (!stepArg.IsNull() && !stepArg->IsUndefined() && stepArg->IsNumber()) {
        options.step = stepArg->ToNumber(vm)->Value();
    }

    if (GreatOrEqual(options.min, options.max)) {
        options.min = SLIDER_MIN;
        options.max = SLIDER_MAX;
    }

    GetStep(options);
    GetValue(options);
    ParseStyleOptions(runtimeCallInfo, options);
}

ArkUINativeModuleValue SliderBridge::SetSliderOptions(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    auto sliderModifier = GetArkUINodeModifiers()->getSliderModifier();
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_EQUAL_RETURN(runtimeCallInfo->GetArgsNumber() != NUM_8, true, panda::JSValueRef::Undefined(vm));
    SliderOptions options;
    ParseOptions(runtimeCallInfo, options);
    Local<JSValueRef> nodeArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    CHECK_NULL_RETURN(vm, panda::JSValueRef::Undefined(vm));
    auto pointer = nodeArg->ToNativePointer(vm);
    CHECK_EQUAL_RETURN(pointer.IsEmpty(), true, panda::JSValueRef::Undefined(vm));
    auto nativeNode = nodePtr(pointer->Value());
    CHECK_NULL_RETURN(nativeNode, panda::JSValueRef::Undefined(vm));
    sliderModifier->setStep(nativeNode, options.step);
    sliderModifier->setMinLabel(nativeNode, options.min);
    sliderModifier->setMaxLabel(nativeNode, options.max);
    sliderModifier->setSliderValue(nativeNode, options.value);
    sliderModifier->setSliderStyle(nativeNode, options.style);
    sliderModifier->setDirection(nativeNode, options.direction);
    sliderModifier->setReverse(nativeNode, options.reverse);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue SliderBridge::SetEnableHapticFeedback(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nodeArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> enableHapticFeedbackArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto nativeNode = nodePtr(nodeArg->ToNativePointer(vm)->Value());
    CHECK_NULL_RETURN(nativeNode, panda::NativePointerRef::New(vm, nullptr));
    auto nodeModifiers = GetArkUINodeModifiers();
    CHECK_NULL_RETURN(nodeModifiers, panda::NativePointerRef::New(vm, nullptr));
    auto SliderModifier = nodeModifiers->getSliderModifier();
    CHECK_NULL_RETURN(SliderModifier, panda::NativePointerRef::New(vm, nullptr));
    if (enableHapticFeedbackArg->IsBoolean()) {
        bool value = enableHapticFeedbackArg->ToBoolean(vm)->Value();
        SliderModifier->setEnableHapticFeedback(nativeNode, value);
    } else {
        SliderModifier->resetEnableHapticFeedback(nativeNode);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue SliderBridge::ResetEnableHapticFeedback(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> nodeArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(nodeArg->ToNativePointer(vm)->Value());
    CHECK_NULL_RETURN(nativeNode, panda::NativePointerRef::New(vm, nullptr));
    auto nodeModifiers = GetArkUINodeModifiers();
    CHECK_NULL_RETURN(nodeModifiers, panda::NativePointerRef::New(vm, nullptr));
    auto SliderModifier = nodeModifiers->getSliderModifier();
    CHECK_NULL_RETURN(SliderModifier, panda::NativePointerRef::New(vm, nullptr));
    SliderModifier->resetEnableHapticFeedback(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

static void ParsePrefixOrSuffixOptions(ArkUIRuntimeCallInfo* runtimeCallInfo, ArkUISliderCustomContentOptions& options)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_VOID(vm);
    if (runtimeCallInfo->GetArgsNumber() < NUM_6) {
        return;
    }
    Local<JSValueRef> textArg = runtimeCallInfo->GetCallArgRef(NUM_2);
    Local<JSValueRef> descriptionArg = runtimeCallInfo->GetCallArgRef(NUM_3);
    Local<JSValueRef> levelArg = runtimeCallInfo->GetCallArgRef(NUM_4);
    Local<JSValueRef> groupArg = runtimeCallInfo->GetCallArgRef(NUM_5);
    CalcDimension dimension;
    if (!textArg.IsNull() && !textArg->IsUndefined()) {
        if (textArg->IsString(vm)) {
            options.accessibilityText = textArg->ToString(vm)->ToString(vm).c_str();
        } else if (ArkTSUtils::ParseJsResource(vm, textArg, dimension)) {
            options.accessibilityText = dimension.ToString().c_str();
        }
    }

    if (!descriptionArg.IsNull() && !descriptionArg->IsUndefined()) {
        if (descriptionArg->IsString(vm)) {
            options.accessibilityDescription = descriptionArg->ToString(vm)->ToString(vm).c_str();
        } else if (ArkTSUtils::ParseJsResource(vm, descriptionArg, dimension)) {
            options.accessibilityDescription = dimension.ToString().c_str();
        }
    }

    if (!levelArg.IsNull() && !levelArg->IsUndefined()) {
        if (levelArg->IsString(vm)) {
            options.accessibilityLevel = levelArg->ToString(vm)->ToString(vm).c_str();
        } else if (ArkTSUtils::ParseJsResource(vm, levelArg, dimension)) {
            options.accessibilityLevel = dimension.ToString().c_str();
        }
    }
    if (!groupArg.IsNull() && !groupArg->IsUndefined() && groupArg->IsBoolean()) {
        options.accessibilityGroup = groupArg->ToBoolean(vm)->Value();
    }
}

ArkUINativeModuleValue SliderBridge::SetPrefix(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    if (runtimeCallInfo->GetArgsNumber() < NUM_3) {
        return panda::JSValueRef::Undefined(vm);
    }

    Local<JSValueRef> nodeArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(nodeArg->ToNativePointer(vm)->Value());
    CHECK_NULL_RETURN(nativeNode, panda::NativePointerRef::New(vm, nullptr));

    Local<JSValueRef> prefixArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto prefixNode = nodePtr(prefixArg->ToNativePointer(vm)->Value());
    CHECK_NULL_RETURN(prefixNode, panda::NativePointerRef::New(vm, nullptr));

    ArkUISliderCustomContentOptions options;
    ParsePrefixOrSuffixOptions(runtimeCallInfo, options);

    auto nodeModifiers = GetArkUINodeModifiers();
    CHECK_NULL_RETURN(nodeModifiers, panda::NativePointerRef::New(vm, nullptr));
    auto SliderModifier = nodeModifiers->getSliderModifier();
    CHECK_NULL_RETURN(SliderModifier, panda::NativePointerRef::New(vm, nullptr));
    SliderModifier->setPrefix(nativeNode, prefixNode, static_cast<ArkUISliderCustomContentOptions*>(&options));
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue SliderBridge::ResetPrefix(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));

    Local<JSValueRef> nodeArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(nodeArg->ToNativePointer(vm)->Value());
    CHECK_NULL_RETURN(nativeNode, panda::NativePointerRef::New(vm, nullptr));

    Local<JSValueRef> prefixArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto prefixNode = nodePtr(prefixArg->ToNativePointer(vm)->Value());
    CHECK_NULL_RETURN(prefixNode, panda::NativePointerRef::New(vm, nullptr));

    auto nodeModifiers = GetArkUINodeModifiers();
    CHECK_NULL_RETURN(nodeModifiers, panda::NativePointerRef::New(vm, nullptr));

    auto SliderModifier = nodeModifiers->getSliderModifier();
    CHECK_NULL_RETURN(SliderModifier, panda::NativePointerRef::New(vm, nullptr));
    SliderModifier->resetPrefix(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue SliderBridge::SetSuffix(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    if (runtimeCallInfo->GetArgsNumber() < NUM_3) {
        return panda::JSValueRef::Undefined(vm);
    }

    Local<JSValueRef> nodeArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(nodeArg->ToNativePointer(vm)->Value());
    CHECK_NULL_RETURN(nativeNode, panda::NativePointerRef::New(vm, nullptr));

    Local<JSValueRef> suffixtArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto suffixNode = nodePtr(suffixtArg->ToNativePointer(vm)->Value());
    CHECK_NULL_RETURN(suffixNode, panda::NativePointerRef::New(vm, nullptr));

    ArkUISliderCustomContentOptions options;
    ParsePrefixOrSuffixOptions(runtimeCallInfo, options);

    auto nodeModifiers = GetArkUINodeModifiers();
    CHECK_NULL_RETURN(nodeModifiers, panda::NativePointerRef::New(vm, nullptr));
    auto SliderModifier = nodeModifiers->getSliderModifier();
    CHECK_NULL_RETURN(SliderModifier, panda::NativePointerRef::New(vm, nullptr));
    SliderModifier->setSuffix(nativeNode, suffixNode, static_cast<ArkUISliderCustomContentOptions*>(&options));
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue SliderBridge::ResetSuffix(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));

    Local<JSValueRef> nodeArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    auto nativeNode = nodePtr(nodeArg->ToNativePointer(vm)->Value());
    CHECK_NULL_RETURN(nativeNode, panda::NativePointerRef::New(vm, nullptr));

    Local<JSValueRef> suffixtArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    auto suffixNode = nodePtr(suffixtArg->ToNativePointer(vm)->Value());
    CHECK_NULL_RETURN(suffixNode, panda::NativePointerRef::New(vm, nullptr));

    auto nodeModifiers = GetArkUINodeModifiers();
    CHECK_NULL_RETURN(nodeModifiers, panda::NativePointerRef::New(vm, nullptr));

    auto SliderModifier = nodeModifiers->getSliderModifier();
    CHECK_NULL_RETURN(SliderModifier, panda::NativePointerRef::New(vm, nullptr));
    SliderModifier->resetSuffix(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

napi_value SliderBridge::GetIteratorNext(const napi_env env, napi_value iterator, napi_value func, bool *done)
{
    napi_value next = nullptr;
    NAPI_CALL_BASE(env, napi_call_function(env, iterator, func, 0, nullptr, &next), nullptr);
    CHECK_NULL_RETURN(next, nullptr);
    napi_value doneValue = nullptr;
    NAPI_CALL_BASE(env, napi_get_named_property(env, next, DONE_STRING, &doneValue), nullptr);
    CHECK_NULL_RETURN(doneValue, nullptr);
    NAPI_CALL_BASE(env, napi_get_value_bool(env, doneValue, done), nullptr);
    return next;
}

int32_t SliderBridge::ParseStepOptionItemKey(const napi_env env, napi_value item)
{
    int32_t result = INT32_MIN;
    napi_value entry = nullptr;
    napi_value key = nullptr;
    napi_valuetype kType = napi_undefined;
    NAPI_CALL_BASE(env, napi_get_named_property(env, item, VALUE_STRING, &entry), result);
    CHECK_NULL_RETURN(entry, result);
    NAPI_CALL_BASE(env, napi_get_element(env, entry, NUM_0, &key), result);
    CHECK_NULL_RETURN(key, result);
    NAPI_CALL_BASE(env, napi_typeof(env, key, &kType), result);
    CHECK_NULL_RETURN(kType, result);
    if (napi_number == kType) {
        double step = NUM_0;
        NAPI_CALL_BASE(env, napi_get_value_double(env, key, &step), result);
        if ((step >= 0) && (NearZero(std::abs(step - std::floor(step)))) && (step <= INT32_MAX)) {
            result = static_cast<int32_t>(step);
        }
    }
    return result;
}

bool SliderBridge::ParseStepOptionItemValue(EcmaVM* vm, const napi_env env, napi_value item, std::string& stepText)
{
    bool result = false;
    CHECK_NULL_RETURN(vm, result);
    napi_value entry = nullptr;
    napi_value value = nullptr;
    napi_value textObject = nullptr;
    napi_valuetype vType = napi_undefined;
    NAPI_CALL_BASE(env, napi_get_named_property(env, item, VALUE_STRING, &entry), result);
    CHECK_NULL_RETURN(entry, result);
    NAPI_CALL_BASE(env, napi_get_element(env, entry, NUM_1, &value), result);
    CHECK_NULL_RETURN(value, result);
    NAPI_CALL_BASE(env, napi_typeof(env, value, &vType), result);
    CHECK_NULL_RETURN(vType, result);
    if (napi_object == vType) {
        NAPI_CALL_BASE(env, napi_get_named_property(env, value, TEXT_STRING, &textObject), result);
        CHECK_NULL_RETURN(textObject, result);
        panda::Local<panda::JSValueRef> localValue = NapiValueToLocalValue(textObject);
        result = ArkTSUtils::ParseJsString(vm, localValue, stepText);
    }
    return result;
}

napi_value SliderBridge::ParseStepOptionsMap(
    EcmaVM* vm, Framework::JSRef<Framework::JSVal> jsStepOptionsMap, StepOptions& stepOptionsMap)
{
    CHECK_NULL_RETURN(vm, nullptr);
    auto engine = EngineHelper::GetCurrentEngine();
    CHECK_NULL_RETURN(engine, nullptr);
    NativeEngine* nativeEngine = engine->GetNativeEngine();
    CHECK_NULL_RETURN(nativeEngine, nullptr);
    auto env = reinterpret_cast<napi_env>(nativeEngine);
    panda::Local<Framework::JsiValue> localValue = jsStepOptionsMap.Get().GetLocalHandle();
    JSValueWrapper valueWrapper = localValue;
    napi_value nativeValue = nativeEngine->ValueToNapiValue(valueWrapper);
    napi_value entriesFunc = nullptr;
    napi_value iterator = nullptr;
    napi_value nextFunc = nullptr;
    bool done = false;
    NAPI_CALL_BASE(env, napi_get_named_property(env, nativeValue, ENTRIES_STRING, &entriesFunc), nullptr);
    CHECK_NULL_RETURN(entriesFunc, nullptr);
    NAPI_CALL_BASE(env, napi_call_function(env, nativeValue, entriesFunc, NUM_0, nullptr, &iterator), nullptr);
    CHECK_NULL_RETURN(iterator, nullptr);
    NAPI_CALL_BASE(env, napi_get_named_property(env, iterator, NEXT_STRING, &nextFunc), nullptr);
    CHECK_NULL_RETURN(nextFunc, nullptr);
    napi_value next = SliderBridge::GetIteratorNext(env, iterator, nextFunc, &done);
    while ((nullptr != next) && !done) {
        auto optionKey = SliderBridge::ParseStepOptionItemKey(env, next);
        std::string optionStr = "";
        auto praseResult = SliderBridge::ParseStepOptionItemValue(vm, env, next, optionStr);
        next = SliderBridge::GetIteratorNext(env, iterator, nextFunc, &done);
        if ((optionKey < 0) || (!praseResult)) {
            continue;
        }
        stepOptionsMap[static_cast<uint32_t>(optionKey)] = optionStr;
    }
    return next;
}
} // namespace OHOS::Ace::NG

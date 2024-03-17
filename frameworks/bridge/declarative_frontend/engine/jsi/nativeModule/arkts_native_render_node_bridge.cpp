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
#include "bridge/declarative_frontend/engine/jsi/nativeModule/arkts_native_render_node_bridge.h"

#include <cstdint>

#include "base/geometry/dimension.h"
#include "base/geometry/ng/rect_t.h"
#include "base/geometry/shape.h"
#include "core/components/common/layout/constants.h"
#include "core/components_ng/pattern/render_node/render_node_properties.h"

namespace OHOS::Ace::NG {
namespace {
const uint32_t DEFAULT_COLOR = 0xFF000000;
}

ArkUINativeModuleValue RenderNodeBridge::AppendChild(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(1);
    auto childNativeNode = nodePtr(secondArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getRenderNodeModifier()->appendChild(nativeNode, childNativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue RenderNodeBridge::InsertChildAfter(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(1);
    auto child = nodePtr(secondArg->ToNativePointer(vm)->Value());
    Local<JSValueRef> thirdArg = runtimeCallInfo->GetCallArgRef(2);
    if (thirdArg.IsNull()) {
        GetArkUINodeModifiers()->getRenderNodeModifier()->insertChildAfter(nativeNode, child, nullptr);
        return panda::JSValueRef::Undefined(vm);
    }
    auto sibling = nodePtr(thirdArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getRenderNodeModifier()->insertChildAfter(nativeNode, child, sibling);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue RenderNodeBridge::RemoveChild(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(1);
    auto childNativeNode = nodePtr(secondArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getRenderNodeModifier()->removeChild(nativeNode, childNativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue RenderNodeBridge::ClearChildren(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getRenderNodeModifier()->clearChildren(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue RenderNodeBridge::SetClipToFrame(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(1);
    bool useClip = false;
    if (secondArg->IsBoolean()) {
        useClip = secondArg->ToBoolean(vm)->Value();
    }
    GetArkUINodeModifiers()->getRenderNodeModifier()->setClipToFrame(nativeNode, useClip);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue RenderNodeBridge::SetRotation(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(1);
    Local<JSValueRef> thirdArg = runtimeCallInfo->GetCallArgRef(2);
    Local<JSValueRef> fourthArg = runtimeCallInfo->GetCallArgRef(3);
    double rotationX = 0.0;
    double rotationY = 0.0;
    double rotationZ = 0.0;
    if (secondArg->IsNumber()) {
        rotationX = secondArg->ToNumber(vm)->Value();
    }
    if (thirdArg->IsNumber()) {
        rotationY = thirdArg->ToNumber(vm)->Value();
    }
    if (fourthArg->IsNumber()) {
        rotationZ = fourthArg->ToNumber(vm)->Value();
    }
    GetArkUINodeModifiers()->getRenderNodeModifier()->setRotation(nativeNode, rotationX, rotationY, rotationZ);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue RenderNodeBridge::SetShadowColor(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    Local<JSValueRef> color = runtimeCallInfo->GetCallArgRef(1);
    uint32_t colorValue = 0;
    if (color->IsNumber()) {
        colorValue = color->Uint32Value(vm);
    }
    GetArkUINodeModifiers()->getRenderNodeModifier()->setShadowColor(nativeNode, colorValue);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue RenderNodeBridge::SetShadowOffset(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(1);
    Local<JSValueRef> thirdArg = runtimeCallInfo->GetCallArgRef(2);
    double offsetX = 0.0;
    double offsetY = 0.0;
    if (secondArg->IsNumber()) {
        offsetX = secondArg->ToNumber(vm)->Value();
    }
    if (thirdArg->IsNumber()) {
        offsetY = thirdArg->ToNumber(vm)->Value();
    }
    GetArkUINodeModifiers()->getRenderNodeModifier()->setShadowOffset(nativeNode, offsetX, offsetY);
    return panda::JSValueRef::Undefined(vm);
}
ArkUINativeModuleValue RenderNodeBridge::SetShadowAlpha(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    Local<JSValueRef> alpha = runtimeCallInfo->GetCallArgRef(1);
    float alphaValue = 1.0;
    if (alpha->IsNumber()) {
        alphaValue = alpha->ToNumber(vm)->Value();
    }
    GetArkUINodeModifiers()->getRenderNodeModifier()->setShadowAlpha(nativeNode, alphaValue);
    return panda::JSValueRef::Undefined(vm);
}
ArkUINativeModuleValue RenderNodeBridge::SetShadowElevation(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    Local<JSValueRef> elevation = runtimeCallInfo->GetCallArgRef(1);
    float elevationValue = 0;
    if (elevation->IsNumber()) {
        elevationValue = elevation->ToNumber(vm)->Value();
    }
    GetArkUINodeModifiers()->getRenderNodeModifier()->setShadowElevation(nativeNode, elevationValue);
    return panda::JSValueRef::Undefined(vm);
}
ArkUINativeModuleValue RenderNodeBridge::SetShadowRadius(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    Local<JSValueRef> radius = runtimeCallInfo->GetCallArgRef(1);
    float radiusValue = 0;
    if (radius->IsNumber()) {
        radiusValue = radius->ToNumber(vm)->Value();
    }

    GetArkUINodeModifiers()->getRenderNodeModifier()->setShadowRadius(nativeNode, radiusValue);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue RenderNodeBridge::SetScale(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    
    Local<JSValueRef> scaleX = runtimeCallInfo->GetCallArgRef(1);
    float scaleXValue = 0;
    if (scaleX->IsNumber()) {
        scaleXValue = scaleX->ToNumber(vm)->Value();
    }

    Local<JSValueRef> scaleY = runtimeCallInfo->GetCallArgRef(2);
    float scaleYValue = 0;
    if (scaleY->IsNumber()) {
        scaleYValue = scaleY->ToNumber(vm)->Value();
    }

    GetArkUINodeModifiers()->getRenderNodeModifier()->setScale(nativeNode, scaleXValue, scaleYValue);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue RenderNodeBridge::SetBackgroundColor(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    Local<JSValueRef> color = runtimeCallInfo->GetCallArgRef(1);
    uint32_t colorValue = 0;
    if (color->IsNumber()) {
        colorValue = color->Uint32Value(vm);
    }

    GetArkUINodeModifiers()->getRenderNodeModifier()->setRenderNodeBackgroundColor(nativeNode, colorValue);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue RenderNodeBridge::SetPivot(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    
    Local<JSValueRef> pivotX = runtimeCallInfo->GetCallArgRef(1);
    float pivotXValue = 0;
    if (pivotX->IsNumber()) {
        pivotXValue = pivotX->ToNumber(vm)->Value();
    }

    Local<JSValueRef> pivotY = runtimeCallInfo->GetCallArgRef(2);
    float pivotYValue = 0;
    if (pivotY->IsNumber()) {
        pivotYValue = pivotY->ToNumber(vm)->Value();
    }

    GetArkUINodeModifiers()->getRenderNodeModifier()->setPivot(nativeNode, pivotXValue, pivotYValue);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue RenderNodeBridge::SetFrame(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    
    Local<JSValueRef> positionX = runtimeCallInfo->GetCallArgRef(1);
    float positionXValue = 0;
    if (positionX->IsNumber()) {
        positionXValue = positionX->ToNumber(vm)->Value();
    }

    Local<JSValueRef> positionY = runtimeCallInfo->GetCallArgRef(2);
    float positionYValue = 0;
    if (positionY->IsNumber()) {
        positionYValue = positionY->ToNumber(vm)->Value();
    }

    Local<JSValueRef> width = runtimeCallInfo->GetCallArgRef(3);
    float widthValue = 0;
    if (width->IsNumber()) {
        widthValue = width->ToNumber(vm)->Value();
    }

    Local<JSValueRef> height = runtimeCallInfo->GetCallArgRef(4);
    float heightValue = 0;
    if (height->IsNumber()) {
        heightValue = height->ToNumber(vm)->Value();
    }

    GetArkUINodeModifiers()->getRenderNodeModifier()->setFrame(
        nativeNode, positionXValue, positionYValue, widthValue, heightValue);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue RenderNodeBridge::SetSize(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());

    Local<JSValueRef> width = runtimeCallInfo->GetCallArgRef(1);
    float widthValue = 0;
    if (width->IsNumber()) {
        widthValue = width->ToNumber(vm)->Value();
    }

    Local<JSValueRef> height = runtimeCallInfo->GetCallArgRef(2);
    float heightValue = 0;
    if (height->IsNumber()) {
        heightValue = height->ToNumber(vm)->Value();
    }

    GetArkUINodeModifiers()->getRenderNodeModifier()->setSize(nativeNode, widthValue, heightValue);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue RenderNodeBridge::SetOpacity(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    Local<JSValueRef> opacity = runtimeCallInfo->GetCallArgRef(1);
    float opacityValue = 0;
    if (opacity->IsNumber()) {
        opacityValue = opacity->ToNumber(vm)->Value();
    }

    GetArkUINodeModifiers()->getRenderNodeModifier()->setOpacity(nativeNode, opacityValue);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue RenderNodeBridge::SetTranslate(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    
    Local<JSValueRef> translateX = runtimeCallInfo->GetCallArgRef(1);
    float translateXValue = 0;
    if (translateX->IsNumber()) {
        translateXValue = translateX->ToNumber(vm)->Value();
    }

    Local<JSValueRef> translateY = runtimeCallInfo->GetCallArgRef(2);
    float translateYValue = 0;
    if (translateY->IsNumber()) {
        translateYValue = translateY->ToNumber(vm)->Value();
    }

    GetArkUINodeModifiers()->getRenderNodeModifier()->setTranslate(
        nativeNode, translateXValue, translateYValue, 0);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue RenderNodeBridge::SetBorderStyle(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());

    auto leftStyle = RenderNodeBridge::GetNumber<uint32_t>(vm, runtimeCallInfo, 1, 0);
    auto leftStyleValue = static_cast<BorderStyle>(leftStyle);

    auto topStyle = RenderNodeBridge::GetNumber<uint32_t>(vm, runtimeCallInfo, 2, 0);
    auto topStyleValue = static_cast<BorderStyle>(topStyle);

    auto rightStyle = RenderNodeBridge::GetNumber<uint32_t>(vm, runtimeCallInfo, 3, 0);
    auto rightStyleValue = static_cast<BorderStyle>(rightStyle);

    auto bottomStyle = RenderNodeBridge::GetNumber<uint32_t>(vm, runtimeCallInfo, 4, 0);
    auto bottomStyleValue = static_cast<BorderStyle>(bottomStyle);

    GetArkUINodeModifiers()->getRenderNodeModifier()->setBorderStyle(nativeNode,
        static_cast<ArkUI_Int32>(leftStyleValue),
        static_cast<ArkUI_Int32>(topStyleValue),
        static_cast<ArkUI_Int32>(rightStyleValue),
        static_cast<ArkUI_Int32>(bottomStyleValue));
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue RenderNodeBridge::SetBorderWidth(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());

    auto leftWidthValue = RenderNodeBridge::GetNumber<float>(vm, runtimeCallInfo, 1, 0.0f);
    auto topWidthValue = RenderNodeBridge::GetNumber<float>(vm, runtimeCallInfo, 2, 0.0f);
    auto rightWidthValue = RenderNodeBridge::GetNumber<float>(vm, runtimeCallInfo, 3, 0.0f);
    auto bottomWidthValue = RenderNodeBridge::GetNumber<float>(vm, runtimeCallInfo, 4, 0.0f);

    GetArkUINodeModifiers()->getRenderNodeModifier()->setBorderWidth(
        nativeNode, leftWidthValue, topWidthValue, rightWidthValue, bottomWidthValue);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue RenderNodeBridge::SetBorderColor(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());

    auto leftColorValue = RenderNodeBridge::GetNumber<uint32_t>(vm, runtimeCallInfo, 1, DEFAULT_COLOR);
    auto topColorValue = RenderNodeBridge::GetNumber<uint32_t>(vm, runtimeCallInfo, 2, DEFAULT_COLOR);
    auto rightColorValue = RenderNodeBridge::GetNumber<uint32_t>(vm, runtimeCallInfo, 3, DEFAULT_COLOR);
    auto bottomColorValue = RenderNodeBridge::GetNumber<uint32_t>(vm, runtimeCallInfo, 4, DEFAULT_COLOR);

    GetArkUINodeModifiers()->getRenderNodeModifier()->setBorderColor(
        nativeNode, leftColorValue, topColorValue, rightColorValue, bottomColorValue);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue RenderNodeBridge::SetBorderRadius(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());

    auto topLeftRadiusValue = RenderNodeBridge::GetNumber<double>(vm, runtimeCallInfo, 1, 0.0);
    auto topRightRadiusValue = RenderNodeBridge::GetNumber<double>(vm, runtimeCallInfo, 2, 0.0);
    auto bottomLeftRadiusValue = RenderNodeBridge::GetNumber<double>(vm, runtimeCallInfo, 3, 0.0);
    auto bottomRightRadiusValue = RenderNodeBridge::GetNumber<double>(vm, runtimeCallInfo, 4, 0.0);

    GetArkUINodeModifiers()->getRenderNodeModifier()->setBorderRadius(
        nativeNode, topLeftRadiusValue, topRightRadiusValue, bottomLeftRadiusValue, bottomRightRadiusValue);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue RenderNodeBridge::SetRectMask(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());

    auto leftValue = RenderNodeBridge::GetNumber<float>(vm, runtimeCallInfo, 1, 0.0f);
    auto topValue = RenderNodeBridge::GetNumber<float>(vm, runtimeCallInfo, 2, 0.0f);
    auto rightValue = RenderNodeBridge::GetNumber<float>(vm, runtimeCallInfo, 3, 0.0f);
    auto bottomValue = RenderNodeBridge::GetNumber<float>(vm, runtimeCallInfo, 4, 0.0f);
    auto fillColorValue = RenderNodeBridge::GetNumber<uint32_t>(vm, runtimeCallInfo, 5, DEFAULT_COLOR);
    auto strokeColorValue = RenderNodeBridge::GetNumber<uint32_t>(vm, runtimeCallInfo, 6, DEFAULT_COLOR);
    auto strokeWidthValue = RenderNodeBridge::GetNumber<float>(vm, runtimeCallInfo, 7, 0.0f);

    GetArkUINodeModifiers()->getRenderNodeModifier()->setRectMask(nativeNode,
        leftValue, topValue, rightValue - leftValue, bottomValue - topValue,
        fillColorValue, strokeColorValue, strokeWidthValue);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue RenderNodeBridge::SetCircleMask(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());

    auto centerXValue = RenderNodeBridge::GetNumber<float>(vm, runtimeCallInfo, 1, 0.0f);
    auto centerYValue = RenderNodeBridge::GetNumber<float>(vm, runtimeCallInfo, 2, 0.0f);
    auto radiusValue = RenderNodeBridge::GetNumber<float>(vm, runtimeCallInfo, 3, 0.0f);
    auto fillColorValue = RenderNodeBridge::GetNumber<uint32_t>(vm, runtimeCallInfo, 4, DEFAULT_COLOR);
    auto strokeColorValue = RenderNodeBridge::GetNumber<uint32_t>(vm, runtimeCallInfo, 5, DEFAULT_COLOR);
    auto strokeWidthValue = RenderNodeBridge::GetNumber<float>(vm, runtimeCallInfo, 6, 0.0f);

    GetArkUINodeModifiers()->getRenderNodeModifier()->setCircleMask(nativeNode,
        centerXValue, centerYValue, radiusValue,
        fillColorValue, strokeColorValue, strokeWidthValue);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue RenderNodeBridge::SetRoundRectMask(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());

    auto topLeftXValue = RenderNodeBridge::GetNumber<float>(vm, runtimeCallInfo, 1, 0.0f);
    auto topLeftYValue = RenderNodeBridge::GetNumber<float>(vm, runtimeCallInfo, 2, 0.0f);

    auto topRightXValue = RenderNodeBridge::GetNumber<float>(vm, runtimeCallInfo, 3, 0.0f);
    auto topRightYValue = RenderNodeBridge::GetNumber<float>(vm, runtimeCallInfo, 4, 0.0f);

    auto bottomLeftXValue = RenderNodeBridge::GetNumber<float>(vm, runtimeCallInfo, 5, 0.0f);
    auto bottomLeftYValue = RenderNodeBridge::GetNumber<float>(vm, runtimeCallInfo, 6, 0.0f);

    auto bottomRightXValue = RenderNodeBridge::GetNumber<float>(vm, runtimeCallInfo, 7, 0.0f);
    auto bottomRightYValue = RenderNodeBridge::GetNumber<float>(vm, runtimeCallInfo, 8, 0.0f);

    auto leftValue = RenderNodeBridge::GetNumber<float>(vm, runtimeCallInfo, 9, 0.0f);
    auto topValue = RenderNodeBridge::GetNumber<float>(vm, runtimeCallInfo, 10, 0.0f);
    auto rightValue = RenderNodeBridge::GetNumber<float>(vm, runtimeCallInfo, 11, 0.0f);
    auto bottomValue = RenderNodeBridge::GetNumber<float>(vm, runtimeCallInfo, 12, 0.0f);

    auto fillColorValue = RenderNodeBridge::GetNumber<uint32_t>(vm, runtimeCallInfo, 13, DEFAULT_COLOR);
    auto strokeColorValue = RenderNodeBridge::GetNumber<uint32_t>(vm, runtimeCallInfo, 14, DEFAULT_COLOR);
    auto strokeWidthValue = RenderNodeBridge::GetNumber<float>(vm, runtimeCallInfo, 15, 0.0f);

    ArkUI_Float32 roundRect[] = {
        topLeftXValue, topLeftYValue, topRightXValue, topRightYValue,
        bottomLeftXValue, bottomLeftYValue, bottomRightXValue, bottomRightYValue,
        leftValue, topValue, rightValue - leftValue, bottomValue - topValue
    };

    GetArkUINodeModifiers()->getRenderNodeModifier()->setRoundRectMask(
        nativeNode, roundRect, sizeof(roundRect), fillColorValue, strokeColorValue, strokeWidthValue);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue RenderNodeBridge::SetOvalMask(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());

    auto leftValue = RenderNodeBridge::GetNumber<float>(vm, runtimeCallInfo, 1, 0.0f);
    auto topValue = RenderNodeBridge::GetNumber<float>(vm, runtimeCallInfo, 2, 0.0f);
    auto rightValue = RenderNodeBridge::GetNumber<float>(vm, runtimeCallInfo, 3, 0.0f);
    auto bottomValue = RenderNodeBridge::GetNumber<float>(vm, runtimeCallInfo, 4, 0.0f);

    auto fillColorValue = RenderNodeBridge::GetNumber<uint32_t>(vm, runtimeCallInfo, 5, DEFAULT_COLOR);
    auto strokeColorValue = RenderNodeBridge::GetNumber<uint32_t>(vm, runtimeCallInfo, 6, DEFAULT_COLOR);
    auto strokeWidthValue = RenderNodeBridge::GetNumber<float>(vm, runtimeCallInfo, 7, 0.0f);

    GetArkUINodeModifiers()->getRenderNodeModifier()->setOvalMask(nativeNode,
        leftValue, topValue, rightValue - leftValue, bottomValue - topValue,
        fillColorValue, strokeColorValue, strokeWidthValue);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue RenderNodeBridge::SetCommandPathMask(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());

    Local<JSValueRef> path = runtimeCallInfo->GetCallArgRef(1);
    std::string pathValue;
    if (path->IsString()) {
        pathValue = path->ToString(vm)->ToString();
    }

    auto fillColorValue = RenderNodeBridge::GetNumber<uint32_t>(vm, runtimeCallInfo, 2, DEFAULT_COLOR);
    auto strokeColorValue = RenderNodeBridge::GetNumber<uint32_t>(vm, runtimeCallInfo, 3, DEFAULT_COLOR);
    auto strokeWidthValue = RenderNodeBridge::GetNumber<float>(vm, runtimeCallInfo, 4, 0.0f);

    GetArkUINodeModifiers()->getRenderNodeModifier()->setCommandPathMask(
        nativeNode, pathValue.c_str(), fillColorValue, strokeColorValue, strokeWidthValue);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue RenderNodeBridge::Invalidate(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getRenderNodeModifier()->invalidate(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}
} // namespace OHOS::Ace::NG

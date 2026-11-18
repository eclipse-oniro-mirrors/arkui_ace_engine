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
#include "bridge/declarative_frontend/engine/jsi/nativeModule/arkts_native_container_span_bridge.h"

#include "frameworks/bridge/declarative_frontend/engine/jsi/nativeModule/arkts_utils.h"

namespace OHOS::Ace::NG {
namespace {
constexpr int BORDER_RADIUS_START_INDEX = 2; // Border Radius args start index
constexpr int BORDER_COLOR_ARG_INDEX = 6;
constexpr int BORDER_WIDTH_ARG_INDEX = 7;
} // namespace

ArkUINativeModuleValue ContainerSpanBridge::SetTextBackgroundStyle(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Color color;
    Color borderColor;
    CalcDimension borderWidth;
    std::vector<ArkUI_Float32> radiusArray;
    std::vector<ArkUI_Int32> valueUnits;
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(1);
    CHECK_NULL_RETURN(firstArg->IsNativePointer(vm), panda::JSValueRef::Undefined(vm));
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    std::shared_ptr<TextBackgroundStyle> style = std::make_shared<TextBackgroundStyle>();
    RefPtr<ResourceObject> colorResObj;
    RefPtr<ResourceObject> borderColorResObj;
    RefPtr<ResourceObject> borderWidthResObj;
    auto nodeInfo = ArkTSUtils::MakeNativeNodeInfo(nativeNode);
    if (!ArkTSUtils::ParseJsColorAlpha(vm, secondArg, color, colorResObj, nodeInfo)) {
        GetArkUINodeModifiers()->getContainerSpanModifier()->resetContainerSpanTextBackgroundStyle(nativeNode);
        return panda::JSValueRef::Undefined(vm);
    }
    Local<JSValueRef> borderColorArg = runtimeCallInfo->GetCallArgRef(BORDER_COLOR_ARG_INDEX);
    if (!ArkTSUtils::ParseJsColorAlpha(vm, borderColorArg, borderColor, borderColorResObj, nodeInfo)) {
        borderColor = Color::TRANSPARENT;
    }
    Local<JSValueRef> borderWidthArg = runtimeCallInfo->GetCallArgRef(BORDER_WIDTH_ARG_INDEX);
    ArkTSUtils::ParseJsDimensionVp(vm, borderWidthArg, borderWidth, borderWidthResObj);
    ArkTSUtils::ParseOuterBorderRadius(runtimeCallInfo, vm, radiusArray, valueUnits, BORDER_RADIUS_START_INDEX,
        style);
    ArkTSUtils::SetTextBackgroundStyle(style, color, colorResObj, radiusArray.data(), valueUnits.data(),
        borderColor, borderColorResObj, borderWidth, borderWidthResObj);
    GetArkUINodeModifiers()->getContainerSpanModifier()->setContainerSpanTextBackgroundStyle(nativeNode,
        color.GetValue(), radiusArray.data(), valueUnits.data(), static_cast<int32_t>(radiusArray.size()),
        style.get());
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ContainerSpanBridge::ResetTextBackgroundStyle(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(0);
    CHECK_NULL_RETURN(firstArg->IsNativePointer(vm), panda::JSValueRef::Undefined(vm));
    auto nativeNode = nodePtr(firstArg->ToNativePointer(vm)->Value());
    GetArkUINodeModifiers()->getContainerSpanModifier()->resetContainerSpanTextBackgroundStyle(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}
} // namespace OHOS::Ace::NG

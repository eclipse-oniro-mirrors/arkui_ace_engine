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

#include "bridge/declarative_frontend/engine/jsi/nativeModule/arkts_native_modal_ui_extension_bridge.h"

#include "jsnapi_expo.h"

#if !defined(PREVIEW)
#include "interfaces/inner_api/ace/ui_content.h"
#include "interfaces/inner_api/ace/modal_ui_extension_config.h"
#include "interfaces/inner_api/ace/modal_ui_extension_proxy.h"
#include "base/want/want_wrap.h"
#include "adapter/ohos/osal/want_wrap_ohos.h"
#include "bridge/common/utils/engine_helper.h"
#include "bridge/declarative_frontend/jsview/js_utils.h"
#include "core/common/container_scope.h"
#include "core/components_ng/property/transition_property.h"
#include "core/animation/animation_pub.h"
#include "core/animation/curve.h"
#include "ui/animation/animation_option.h"
#include "bridge/common/utils/utils.h"
#include "native_engine/impl/ark/ark_native_engine.h"
#endif

namespace OHOS::Ace::NG {
#if !defined(PREVIEW)
namespace {
constexpr int32_t MODAL_PROXY_FIELD_COUNT = 1;

void DestroyModalUIExtensionProxy(void* data, void* env, void* hint)
{
    auto proxyPtr = static_cast<std::shared_ptr<ModalUIExtensionProxy>*>(data);
    delete proxyPtr;
}

Local<JSValueRef> ModalUIExtensionProxySend(panda::JsiRuntimeCallInfo* info)
{
    EcmaVM* vm = info->GetVM();
    CHECK_NULL_RETURN(vm, panda::JSValueRef::Undefined(vm));
    Local<JSValueRef> thisObj = info->GetThisRef();
    if (!thisObj->IsObject(vm)) {
        return panda::JSValueRef::Undefined(vm);
    }
    auto proxyPtr = static_cast<std::shared_ptr<ModalUIExtensionProxy>*>(
        Local<ObjectRef>(thisObj)->GetNativePointerField(vm, 0));
    CHECK_NULL_RETURN(proxyPtr && *proxyPtr, panda::JSValueRef::Undefined(vm));
    Local<JSValueRef> dataArg = info->GetCallArgRef(0);
    if (!dataArg->IsObject(vm)) {
        return panda::JSValueRef::Undefined(vm);
    }
    auto engine = EngineHelper::GetCurrentEngine();
    CHECK_NULL_RETURN(engine, panda::JSValueRef::Undefined(vm));
    auto nativeEngine = engine->GetNativeEngine();
    CHECK_NULL_RETURN(nativeEngine, panda::JSValueRef::Undefined(vm));
    JSValueWrapper valueWrapper = dataArg;
    Framework::ScopeRAII scopeNapi(reinterpret_cast<napi_env>(nativeEngine));
    napi_value nativeValue = nativeEngine->ValueToNapiValue(valueWrapper);
    auto wantParamsWrap = WantParamsWrap::CreateWantWrap(reinterpret_cast<napi_env>(nativeEngine), nativeValue);
    CHECK_NULL_RETURN(wantParamsWrap, panda::JSValueRef::Undefined(vm));
    auto wantParamsWrapOhos = AceType::DynamicCast<WantParamsWrapOhos>(wantParamsWrap);
    CHECK_NULL_RETURN(wantParamsWrapOhos, panda::JSValueRef::Undefined(vm));
    (*proxyPtr)->SendData(wantParamsWrapOhos->GetWantParams());
    return panda::JSValueRef::Undefined(vm);
}

Local<JSValueRef> CreateModalUIExtensionProxyObject(EcmaVM* vm,
    const std::shared_ptr<ModalUIExtensionProxy>& proxy)
{
    auto proxyObj = panda::ObjectRef::New(vm);
    auto* proxyStorage = new std::shared_ptr<ModalUIExtensionProxy>(proxy);
    proxyObj->SetNativePointerFieldCount(vm, MODAL_PROXY_FIELD_COUNT);
    proxyObj->SetNativePointerField(vm, 0, proxyStorage, DestroyModalUIExtensionProxy);
    proxyObj->Set(vm, panda::StringRef::NewFromUtf8(vm, "send"),
        panda::FunctionRef::New(const_cast<panda::EcmaVM*>(vm), ModalUIExtensionProxySend));
    return proxyObj;
}

std::function<void(const std::shared_ptr<ModalUIExtensionProxy>&)> CreateOnRemoteReadyCallback(
    EcmaVM* vm, const Local<JSValueRef>& callbackVal, int32_t instanceId)
{
    if (!callbackVal->IsFunction(vm)) {
        return nullptr;
    }
    panda::Local<panda::FunctionRef> funcRef = callbackVal->ToObject(vm);
    auto func = panda::CopyableGlobal(vm, funcRef);
    return [vm, func = std::move(func), instanceId](
            const std::shared_ptr<ModalUIExtensionProxy>& proxy) {
        panda::LocalScope pandaScope(vm);
        panda::TryCatch trycatch(vm);
        ContainerScope scope(instanceId);
        auto proxyObj = CreateModalUIExtensionProxyObject(vm, proxy);
        panda::Local<panda::JSValueRef> params[1] = { proxyObj };
        func->Call(vm, func.ToLocal(), params, 1);
    };
}

std::function<void(const AAFwk::WantParams&)> CreateOnReceiveCallback(
    EcmaVM* vm, const Local<JSValueRef>& callbackVal, int32_t instanceId)
{
    if (!callbackVal->IsFunction(vm)) {
        return nullptr;
    }
    panda::Local<panda::FunctionRef> funcRef = callbackVal->ToObject(vm);
    auto func = panda::CopyableGlobal(vm, funcRef);
    return [vm, func = std::move(func), instanceId](const AAFwk::WantParams& wantParams) {
        panda::LocalScope pandaScope(vm);
        panda::TryCatch trycatch(vm);
        ContainerScope scope(instanceId);
        auto engine = EngineHelper::GetCurrentEngine();
        CHECK_NULL_VOID(engine);
        auto nativeEngine = engine->GetNativeEngine();
        CHECK_NULL_VOID(nativeEngine);
        auto env = reinterpret_cast<napi_env>(nativeEngine);
        Framework::ScopeRAII scopeNapi(env);
        auto nativeWantParams = WantWrap::ConvertParamsToNativeValue(wantParams, env);
        auto jsVal = NapiValueToLocalValue(nativeWantParams);
        panda::Local<panda::JSValueRef> params[1] = { jsVal };
        func->Call(vm, func.ToLocal(), params, 1);
    };
}

std::function<void(int32_t, const std::string&, const std::string&)> CreateOnErrorCallback(
    EcmaVM* vm, const Local<JSValueRef>& callbackVal, int32_t instanceId)
{
    if (!callbackVal->IsFunction(vm)) {
        return nullptr;
    }
    panda::Local<panda::FunctionRef> funcRef = callbackVal->ToObject(vm);
    auto func = panda::CopyableGlobal(vm, funcRef);
    return [vm, func = std::move(func), instanceId](
            int32_t code, const std::string& name, const std::string& message) {
        panda::LocalScope pandaScope(vm);
        panda::TryCatch trycatch(vm);
        ContainerScope scope(instanceId);
        auto errorObj = panda::ObjectRef::New(vm);
        errorObj->Set(vm, panda::StringRef::NewFromUtf8(vm, "code"), panda::NumberRef::New(vm, code));
        errorObj->Set(vm, panda::StringRef::NewFromUtf8(vm, "name"),
            panda::StringRef::NewFromUtf8(vm, name.c_str()));
        errorObj->Set(vm, panda::StringRef::NewFromUtf8(vm, "message"),
            panda::StringRef::NewFromUtf8(vm, message.c_str()));
        panda::Local<panda::JSValueRef> params[1] = { errorObj };
        func->Call(vm, func.ToLocal(), params, 1);
    };
}

std::function<void(int32_t)> CreateOnTerminatedCallback(
    EcmaVM* vm, const Local<JSValueRef>& callbackVal, int32_t instanceId)
{
    if (!callbackVal->IsFunction(vm)) {
        return nullptr;
    }
    panda::Local<panda::FunctionRef> funcRef = callbackVal->ToObject(vm);
    auto func = panda::CopyableGlobal(vm, funcRef);
    return [vm, func = std::move(func), instanceId](int32_t code) {
        panda::LocalScope pandaScope(vm);
        panda::TryCatch trycatch(vm);
        ContainerScope scope(instanceId);
        auto infoObj = panda::ObjectRef::New(vm);
        infoObj->Set(vm, panda::StringRef::NewFromUtf8(vm, "code"), panda::NumberRef::New(vm, code));
        panda::Local<panda::JSValueRef> params[1] = { infoObj };
        func->Call(vm, func.ToLocal(), params, 1);
    };
}

std::function<void()> CreateOnDrawReadyCallback(
    EcmaVM* vm, const Local<JSValueRef>& callbackVal, int32_t instanceId)
{
    if (!callbackVal->IsFunction(vm)) {
        return nullptr;
    }
    panda::Local<panda::FunctionRef> funcRef = callbackVal->ToObject(vm);
    auto func = panda::CopyableGlobal(vm, funcRef);
    return [vm, func = std::move(func), instanceId]() {
        panda::LocalScope pandaScope(vm);
        panda::TryCatch trycatch(vm);
        ContainerScope scope(instanceId);
        func->Call(vm, func.ToLocal(), nullptr, 0);
    };
}

RefPtr<WantWrap> ParseWantFromValue(EcmaVM* vm, const Local<JSValueRef>& wantVal)
{
    if (!wantVal->IsObject(vm)) {
        return nullptr;
    }
    auto engine = EngineHelper::GetCurrentEngine();
    CHECK_NULL_RETURN(engine, nullptr);
    auto nativeEngine = engine->GetNativeEngine();
    CHECK_NULL_RETURN(nativeEngine, nullptr);
    auto env = reinterpret_cast<napi_env>(nativeEngine);
    JSValueWrapper wantWrapper = wantVal;
    Framework::ScopeRAII wantScope(env);
    napi_value wantNapi = nativeEngine->ValueToNapiValue(wantWrapper);
    return WantWrap::CreateWantWrap(env, wantNapi);
}

RefPtr<NG::ChainedTransitionEffect> ParseChainedTransitionEffect(EcmaVM* vm, const Local<JSValueRef>& val)
{
    if (!val->IsObject(vm)) {
        return nullptr;
    }
    auto obj = val->ToObject(vm);
    auto typeVal = obj->Get(vm, panda::StringRef::NewFromUtf8(vm, "type_"));
    if (!typeVal->IsString(vm)) {
        return nullptr;
    }
    std::string type = typeVal->ToString(vm)->ToString(vm);
    auto effectVal = obj->Get(vm, panda::StringRef::NewFromUtf8(vm, "effect_"));
    RefPtr<NG::ChainedTransitionEffect> result;
    if (type == "identity") {
        result = AceType::MakeRefPtr<NG::ChainedIdentityEffect>();
    } else if (type == "opacity") {
        double opacity = 1.0;
        if (effectVal->IsNumber()) {
            opacity = effectVal->ToNumber(vm)->Value();
            if (opacity < 0.0 || opacity > 1.0) {
                opacity = 1.0;
            }
        }
        result = AceType::MakeRefPtr<NG::ChainedOpacityEffect>(opacity);
    } else if (type == "slideSwitch") {
        result = AceType::MakeRefPtr<NG::ChainedSlideSwitchEffect>();
    } else if (type == "move") {
        int32_t edge = 0;
        if (effectVal->IsNumber()) {
            edge = effectVal->ToNumber(vm)->Int32Value(vm);
        }
        if (edge < static_cast<int32_t>(NG::TransitionEdge::TOP) ||
            edge > static_cast<int32_t>(NG::TransitionEdge::END)) {
            edge = static_cast<int32_t>(NG::TransitionEdge::START);
        }
        result = AceType::MakeRefPtr<NG::ChainedMoveEffect>(static_cast<NG::TransitionEdge>(edge));
    } else if (type == "translate") {
        NG::TranslateOptions translate;
        if (effectVal->IsObject(vm)) {
            auto effectObj = effectVal->ToObject(vm);
            auto xVal = effectObj->Get(vm, panda::StringRef::NewFromUtf8(vm, "x"));
            auto yVal = effectObj->Get(vm, panda::StringRef::NewFromUtf8(vm, "y"));
            if (xVal->IsNumber()) {
                translate.x = CalcDimension(xVal->ToNumber(vm)->Value(), DimensionUnit::VP);
            }
            if (yVal->IsNumber()) {
                translate.y = CalcDimension(yVal->ToNumber(vm)->Value(), DimensionUnit::VP);
            }
        }
        result = AceType::MakeRefPtr<NG::ChainedTranslateEffect>(translate);
    } else if (type == "scale") {
        NG::ScaleOptions scale;
        if (effectVal->IsObject(vm)) {
            auto effectObj = effectVal->ToObject(vm);
            auto xVal = effectObj->Get(vm, panda::StringRef::NewFromUtf8(vm, "x"));
            auto yVal = effectObj->Get(vm, panda::StringRef::NewFromUtf8(vm, "y"));
            if (xVal->IsNumber()) {
                scale.xScale = xVal->ToNumber(vm)->Value();
            }
            if (yVal->IsNumber()) {
                scale.yScale = yVal->ToNumber(vm)->Value();
            }
        }
        result = AceType::MakeRefPtr<NG::ChainedScaleEffect>(scale);
    } else if (type == "rotate") {
        NG::RotateOptions rotate;
        if (effectVal->IsObject(vm)) {
            auto effectObj = effectVal->ToObject(vm);
            auto xVal = effectObj->Get(vm, panda::StringRef::NewFromUtf8(vm, "x"));
            auto yVal = effectObj->Get(vm, panda::StringRef::NewFromUtf8(vm, "y"));
            auto zVal = effectObj->Get(vm, panda::StringRef::NewFromUtf8(vm, "z"));
            auto angleVal = effectObj->Get(vm, panda::StringRef::NewFromUtf8(vm, "angle"));
            if (xVal->IsNumber()) {
                rotate.xDirection = xVal->ToNumber(vm)->Value();
            }
            if (yVal->IsNumber()) {
                rotate.yDirection = yVal->ToNumber(vm)->Value();
            }
            if (zVal->IsNumber()) {
                rotate.zDirection = zVal->ToNumber(vm)->Value();
            }
            if (angleVal->IsNumber()) {
                rotate.angle = angleVal->ToNumber(vm)->Value();
            }
        }
        result = AceType::MakeRefPtr<NG::ChainedRotateEffect>(rotate);
    } else if (type == "asymmetric") {
        RefPtr<NG::ChainedTransitionEffect> appearEffect;
        RefPtr<NG::ChainedTransitionEffect> disappearEffect;
        if (effectVal->IsObject(vm)) {
            auto effectObj = effectVal->ToObject(vm);
            auto appearVal = effectObj->Get(vm, panda::StringRef::NewFromUtf8(vm, "appear"));
            if (appearVal->IsObject(vm)) {
                appearEffect = ParseChainedTransitionEffect(vm, appearVal);
            }
            auto disappearVal = effectObj->Get(vm, panda::StringRef::NewFromUtf8(vm, "disappear"));
            if (disappearVal->IsObject(vm)) {
                disappearEffect = ParseChainedTransitionEffect(vm, disappearVal);
            }
        }
        result = AceType::MakeRefPtr<NG::ChainedAsymmetricEffect>(appearEffect, disappearEffect);
    } else {
        return nullptr;
    }
    auto animationVal = obj->Get(vm, panda::StringRef::NewFromUtf8(vm, "animation_"));
    if (animationVal->IsObject(vm)) {
        auto animObj = animationVal->ToObject(vm);
        auto option = std::make_shared<AnimationOption>();
        auto durationVal = animObj->Get(vm, panda::StringRef::NewFromUtf8(vm, "duration"));
        if (durationVal->IsNumber()) {
            option->SetDuration(durationVal->ToNumber(vm)->Int32Value(vm));
        }
        auto delayVal = animObj->Get(vm, panda::StringRef::NewFromUtf8(vm, "delay"));
        if (delayVal->IsNumber()) {
            option->SetDelay(delayVal->ToNumber(vm)->Int32Value(vm));
        }
        auto iterationsVal = animObj->Get(vm, panda::StringRef::NewFromUtf8(vm, "iterations"));
        if (iterationsVal->IsNumber()) {
            option->SetIteration(iterationsVal->ToNumber(vm)->Int32Value(vm));
        }
        auto curveVal = animObj->Get(vm, panda::StringRef::NewFromUtf8(vm, "curve"));
        if (curveVal->IsNumber()) {
            int32_t curveType = curveVal->ToNumber(vm)->Int32Value(vm);
            option->SetCurve(Framework::CreateCurve(curveType));
        } else if (curveVal->IsString(vm)) {
            std::string curveStr = curveVal->ToString(vm)->ToString(vm);
            option->SetCurve(Framework::CreateCurve(curveStr));
        }
        result->SetAnimationOption(option);
    }
    auto successorVal = obj->Get(vm, panda::StringRef::NewFromUtf8(vm, "successor_"));
    if (successorVal->IsObject(vm)) {
        result->SetNext(ParseChainedTransitionEffect(vm, successorVal));
    }
    return result;
}

void ParseModalUIExtensionCallbacks(
    EcmaVM* vm, const Local<ObjectRef>& callbackObj, ModalUIExtensionCallbacks& callbacks, int32_t instanceId)
{
    callbacks.onRemoteReady = CreateOnRemoteReadyCallback(
        vm, callbackObj->Get(vm, panda::StringRef::NewFromUtf8(vm, "onRemoteReady")), instanceId);
    callbacks.onReceive = CreateOnReceiveCallback(
        vm, callbackObj->Get(vm, panda::StringRef::NewFromUtf8(vm, "onReceive")), instanceId);
    callbacks.onError = CreateOnErrorCallback(
        vm, callbackObj->Get(vm, panda::StringRef::NewFromUtf8(vm, "onError")), instanceId);
    callbacks.onRelease = CreateOnTerminatedCallback(
        vm, callbackObj->Get(vm, panda::StringRef::NewFromUtf8(vm, "onTerminated")), instanceId);
    callbacks.onDrawReady = CreateOnDrawReadyCallback(
        vm, callbackObj->Get(vm, panda::StringRef::NewFromUtf8(vm, "onDrawReady")), instanceId);
}
} // namespace
#endif

ArkUINativeModuleValue ModalUIExtensionBridge::CreateModalUIExtension(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NumberRef::New(vm, 0));
#if !defined(PREVIEW)
    Local<JSValueRef> instanceIdArg = runtimeCallInfo->GetCallArgRef(0);
    if (!instanceIdArg->IsNumber()) {
        return panda::NumberRef::New(vm, 0);
    }
    int32_t instanceId = instanceIdArg->ToNumber(vm)->Int32Value(vm);

    Local<JSValueRef> wantArg = runtimeCallInfo->GetCallArgRef(1);
    if (!wantArg->IsObject(vm)) {
        return panda::NumberRef::New(vm, 0);
    }
    auto wantWrap = ParseWantFromValue(vm, wantArg);
    CHECK_NULL_RETURN(wantWrap, panda::NumberRef::New(vm, 0));
    const AAFwk::Want& want = wantWrap->GetWant();

    bool isDensityFollowHost = true;
    bool isWindowModeFollowHost = true;
    Local<JSValueRef> optionsArg = runtimeCallInfo->GetCallArgRef(2);
    if (optionsArg->IsObject(vm)) {
        auto optionsObj = optionsArg->ToObject(vm);
        Local<JSValueRef> dpiArg = optionsObj->Get(vm, panda::StringRef::NewFromUtf8(vm, "dpiFollowStrategy"));
        if (dpiArg->IsNumber()) {
            isDensityFollowHost = (dpiArg->ToNumber(vm)->Int32Value(vm) == 0);
        }
        Local<JSValueRef> windowModeArg =
            optionsObj->Get(vm, panda::StringRef::NewFromUtf8(vm, "windowModeFollowStrategy"));
        if (windowModeArg->IsNumber()) {
            isWindowModeFollowHost = (windowModeArg->ToNumber(vm)->Int32Value(vm) == 0);
        }
    }

    ModalUIExtensionCallbacks callbacks;
    Local<JSValueRef> callbackArg = runtimeCallInfo->GetCallArgRef(3);
    if (callbackArg->IsObject(vm)) {
        auto callbackObj = callbackArg->ToObject(vm);
        ParseModalUIExtensionCallbacks(vm, callbackObj, callbacks, instanceId);
    }

    ModalUIExtensionConfig config;
    config.isDensityFollowHost = isDensityFollowHost;
    config.isWindowModeFollowHost = isWindowModeFollowHost;
    if (optionsArg->IsObject(vm)) {
        auto optionsObj = optionsArg->ToObject(vm);
        Local<JSValueRef> transitionVal =
            optionsObj->Get(vm, panda::StringRef::NewFromUtf8(vm, "transition"));
        if (transitionVal->IsObject(vm)) {
            auto effect = ParseChainedTransitionEffect(vm, transitionVal);
            if (effect) {
                config.transition = new RefPtr<NG::ChainedTransitionEffect>(effect);
            }
        }
    }

    UIContent* uiContent = UIContent::GetUIContent(instanceId);
    CHECK_NULL_RETURN(uiContent, panda::NumberRef::New(vm, 0));
    int32_t sessionId = uiContent->CreateModalUIExtension(want, callbacks, config);
    return panda::NumberRef::New(vm, sessionId);
#else
    return panda::NumberRef::New(vm, 0);
#endif
}

ArkUINativeModuleValue ModalUIExtensionBridge::CloseModalUIExtension(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::JSValueRef::Undefined(vm));
#if !defined(PREVIEW)
    Local<JSValueRef> instanceIdArg = runtimeCallInfo->GetCallArgRef(0);
    if (!instanceIdArg->IsNumber()) {
        return panda::JSValueRef::Undefined(vm);
    }
    int32_t instanceId = instanceIdArg->ToNumber(vm)->Int32Value(vm);

    Local<JSValueRef> sessionIdArg = runtimeCallInfo->GetCallArgRef(1);
    if (!sessionIdArg->IsNumber()) {
        return panda::JSValueRef::Undefined(vm);
    }
    int32_t sessionId = sessionIdArg->ToNumber(vm)->Int32Value(vm);

    UIContent* uiContent = UIContent::GetUIContent(instanceId);
    CHECK_NULL_RETURN(uiContent, panda::JSValueRef::Undefined(vm));
    uiContent->CloseModalUIExtension(sessionId);
#endif
    return panda::JSValueRef::Undefined(vm);
}
} // namespace OHOS::Ace::NG

/**
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#include "ecmascript/napi/include/jsnapi.h"
#include "jsi_bindings.h"

#include "base/log/ace_performance_monitor.h"
#include "base/log/ace_trace.h"

namespace OHOS::Ace::Framework {

inline void JsiReleaseConstructorBinding(void* env, void* nativePtr, void* data)
{
    std::unique_ptr<JsiClassConstructorBindingRef> binding(static_cast<JsiClassConstructorBindingRef*>(data));
}

inline panda::Local<panda::FunctionRef> JsiNewClassFunction(EcmaVM* vm, const JsiClassConstructorBindingRef& binding)
{
    auto owner = std::make_unique<JsiClassConstructorBindingRef>(binding);
    auto function = panda::FunctionRef::NewClassFunction(
        vm, JsiJSConstructorInterceptor, JsiReleaseConstructorBinding, owner.get());
    if (function.IsEmpty()) {
        // No handle was produced; the VM has not taken ownership of data.
        // Keep RAII cleanup.
        return function;
    }
    // For any non-empty result (function or non-empty undefined), the VM already
    // holds data and releases it via JsiReleaseConstructorBinding during GC.
    owner.release();
    return function;
}

template<typename C>
thread_local std::unordered_map<std::string, panda::Global<panda::FunctionRef>> JsiClass<C>::staticFunctions_;

template<typename C>
thread_local std::unordered_map<std::string, panda::Global<panda::FunctionRef>> JsiClass<C>::customFunctions_;

template<typename C>
thread_local std::unordered_map<std::string, panda::Global<panda::FunctionRef>> JsiClass<C>::customGetFunctions_;

template<typename C>
thread_local std::unordered_map<std::string, panda::Global<panda::FunctionRef>> JsiClass<C>::customSetFunctions_;

template<typename C>
thread_local FunctionCallback JsiClass<C>::constructor_ = nullptr;

template<typename C>
thread_local std::string JsiClass<C>::className_;

template<typename C>
thread_local panda::Global<panda::FunctionRef> JsiClass<C>::classFunction_;

template<typename C>
void JsiClass<C>::Declare(const char* name)
{
    JsiClassBase::RegisterUnDeclare(&JsiClass<C>::UnDeclare);
    JsiClassBase::DeclareImpl(name, className_, staticFunctions_, customFunctions_, customGetFunctions_,
        customSetFunctions_, classFunction_);
    constructor_ = nullptr;
}
template<typename C>
void JsiClass<C>::UnDeclare()
{
    for (auto& [name, val] : staticFunctions_) {
        val.FreeGlobalHandleAddr();
    }
    staticFunctions_.clear();
    for (auto& [name, val] : customFunctions_) {
        val.FreeGlobalHandleAddr();
    }
    customFunctions_.clear();
    for (auto& [name, val] : customGetFunctions_) {
        val.FreeGlobalHandleAddr();
    }
    customGetFunctions_.clear();
    for (auto& [name, val] : customSetFunctions_) {
        val.FreeGlobalHandleAddr();
    }
    customSetFunctions_.clear();
    classFunction_.FreeGlobalHandleAddr();
    classFunction_.Empty();
    className_.clear();
    constructor_ = nullptr;
}

template<typename C>
template<typename Base, typename R, typename... Args>
void JsiClass<C>::Method(const char* name, FunctionBinding<Base, R, Args...>* binding)
{
    auto runtime = std::static_pointer_cast<ArkJSRuntime>(JsiDeclarativeEngineInstance::GetCurrentRuntime());
    auto vm = const_cast<EcmaVM*>(runtime->GetEcmaVm());
    customFunctions_.emplace(
        name, panda::Global<panda::FunctionRef>(
                  vm, panda::FunctionRef::New(vm, MethodCallback<Base, R, Args...>, nullptr, (void*)binding)));
}

template<typename C>
template<typename T>
void JsiClass<C>::CustomMethod(
    const char* name, FunctionBinding<T, panda::Local<panda::JSValueRef>, panda::JsiRuntimeCallInfo*>* binding)
{
    auto runtime = std::static_pointer_cast<ArkJSRuntime>(JsiDeclarativeEngineInstance::GetCurrentRuntime());
    auto vm = const_cast<EcmaVM*>(runtime->GetEcmaVm());
    customFunctions_.emplace(
        name, panda::Global<panda::FunctionRef>(
                  vm, panda::FunctionRef::New(
                          vm, InternalMemberFunctionCallback<T, panda::JsiRuntimeCallInfo*>, nullptr, (void*)binding)));
}

template<typename C>
void JsiClass<C>::CustomMethod(const char* name, FunctionCallback callback)
{
    JsiClassBase::AddCustomMethodImpl(name, callback, customFunctions_);
}

template<typename C>
template<typename T>
void JsiClass<C>::CustomMethod(const char* name, FunctionBinding<T, void, const JSCallbackInfo&>* binding)
{
    auto runtime = std::static_pointer_cast<ArkJSRuntime>(JsiDeclarativeEngineInstance::GetCurrentRuntime());
    auto vm = const_cast<EcmaVM*>(runtime->GetEcmaVm());
    customFunctions_.emplace(
        name, panda::Global<panda::FunctionRef>(
                  vm, panda::FunctionRef::New(vm, InternalJSMemberFunctionCallback<T>, nullptr, (void*)binding)));
}

template<typename C>
template<typename T>
void JsiClass<C>::CustomProperty(const char* name,
    FunctionBinding<T, panda::Local<panda::JSValueRef>, panda::JsiRuntimeCallInfo*>* getter,
    FunctionBinding<T, panda::Local<panda::JSValueRef>, panda::JsiRuntimeCallInfo*>* setter)
{
    auto runtime = std::static_pointer_cast<ArkJSRuntime>(JsiDeclarativeEngineInstance::GetCurrentRuntime());
    auto vm = const_cast<EcmaVM*>(runtime->GetEcmaVm());
    customGetFunctions_.emplace(
        name, panda::Global<panda::FunctionRef>(
                  vm, panda::FunctionRef::New(
                          vm, InternalMemberFunctionCallback<T, panda::JsiRuntimeCallInfo*>, nullptr, (void*)getter)));

    customSetFunctions_.emplace(
        name, panda::Global<panda::FunctionRef>(
                  vm, panda::FunctionRef::New(
                          vm, InternalMemberFunctionCallback<T, panda::JsiRuntimeCallInfo*>, nullptr, (void*)setter)));
}

template<typename C>
void JsiClass<C>::CustomProperty(const char* name, FunctionGetCallback getter, FunctionSetCallback setter)
{
    auto runtime = std::static_pointer_cast<ArkJSRuntime>(JsiDeclarativeEngineInstance::GetCurrentRuntime());
    auto vm = const_cast<EcmaVM*>(runtime->GetEcmaVm());
    customGetFunctions_.emplace(name, panda::Global<panda::FunctionRef>(vm, panda::FunctionRef::New(vm, getter)));
    customSetFunctions_.emplace(name, panda::Global<panda::FunctionRef>(vm, panda::FunctionRef::New(vm, setter)));
}

template<typename C>
template<typename T>
void JsiClass<C>::CustomProperty(const char* name, FunctionBinding<T, void, const JSCallbackInfo&>* getter,
    FunctionBinding<T, void, const JSCallbackInfo&>* setter)
{
    auto runtime = std::static_pointer_cast<ArkJSRuntime>(JsiDeclarativeEngineInstance::GetCurrentRuntime());
    auto vm = const_cast<EcmaVM*>(runtime->GetEcmaVm());
    customGetFunctions_.emplace(
        name, panda::Global<panda::FunctionRef>(
                  vm, panda::FunctionRef::New(vm, InternalJSMemberFunctionCallback<T>, nullptr, (void*)getter)));
    customSetFunctions_.emplace(
        name, panda::Global<panda::FunctionRef>(
                  vm, panda::FunctionRef::New(vm, InternalJSMemberFunctionCallback<T>, nullptr, (void*)setter)));
}

template<typename C>
template<typename R, typename... Args>
void JsiClass<C>::StaticMethod(const char* name, StaticFunctionBinding<R, Args...>* staticFunctionBinding)
{
    auto runtime = std::static_pointer_cast<ArkJSRuntime>(JsiDeclarativeEngineInstance::GetCurrentRuntime());
    auto vm = const_cast<EcmaVM*>(runtime->GetEcmaVm());
    staticFunctions_.emplace(name,
        panda::Global<panda::FunctionRef>(
            vm, panda::FunctionRef::New(vm, StaticMethodCallback<R, Args...>, nullptr, (void*)staticFunctionBinding)));
}

template<typename C>
void JsiClass<C>::StaticMethod(
    const char* name, StaticFunctionBinding<void, const JSCallbackInfo&>* staticFunctionBinding)
{
    JsiClassBase::AddStaticMethodJSImpl(
        name, staticFunctionBinding, staticFunctions_, JSStaticMethodCallback);
}

template<typename C>
void JsiClass<C>::CustomStaticMethod(const char* name, FunctionCallback callback)
{
    JsiClassBase::AddCustomStaticMethodImpl(name, callback, staticFunctions_);
}

template<typename C>
template<typename T>
void JsiClass<C>::StaticConstant(const char* name, T val)
{
    auto runtime = std::static_pointer_cast<ArkJSRuntime>(JsiDeclarativeEngineInstance::GetCurrentRuntime());
    auto vm = const_cast<EcmaVM*>(runtime->GetEcmaVm());
    panda::Local<panda::JSValueRef> key = panda::StringRef::NewFromUtf8(vm, name);
    classFunction_->Set(vm, key, JsiValueConvertor::toJsiValueWithVM<std::string>(vm, val));
}

template<typename C>
void JsiClass<C>::Bind(BindingTarget t, FunctionCallback ctor)
{
    constructor_ = ctor;
    JsiClassBase::BindImpl(t, ThisJSClass::JSName(), className_, ConstructorInterceptor, staticFunctions_,
        customFunctions_, customGetFunctions_, customSetFunctions_, classFunction_);
}

template<typename C>
void JsiClass<C>::Bind(
    BindingTarget t, JSFunctionCallback ctor, JSDestructorCallback<C> dtor, JSGCMarkCallback<C> gcMark)
{
    auto runtime = std::static_pointer_cast<ArkJSRuntime>(JsiDeclarativeEngineInstance::GetCurrentRuntime());
    Bind(const_cast<EcmaVM*>(runtime->GetEcmaVm()), t, ctor, dtor, gcMark);
}

template<typename C>
void JsiClass<C>::Bind(
    EcmaVM* vm, BindingTarget t, JSFunctionCallback ctor, JSDestructorCallback<C> dtor, JSGCMarkCallback<C> gcMark)
{
    auto binding = MakeJsiClassConstructorBinding<C>(ctor, dtor, gcMark);
    LocalScope scope(vm);
    classFunction_ = panda::Global<panda::FunctionRef>(vm, JsiNewClassFunction(vm, binding));
    if (classFunction_.IsEmpty() || !classFunction_->IsFunction(vm)) {
        return;
    }
    classFunction_->SetName(vm, StringRef::NewFromUtf8(vm, className_.c_str()));
    auto prototype = panda::Local<panda::ObjectRef>(classFunction_->GetFunctionPrototype(vm));
    prototype->Set(vm, panda::StringRef::NewFromUtf8(vm, "constructor"),
        panda::Local<panda::JSValueRef>(classFunction_.ToLocal()));
    for (const auto& [name, val] : staticFunctions_) {
        classFunction_->Set(vm, panda::StringRef::NewFromUtf8(vm, name.c_str()), val.ToLocal());
    }
    for (const auto& [name, val] : customFunctions_) {
        prototype->Set(vm, panda::StringRef::NewFromUtf8(vm, name.c_str()), val.ToLocal());
    }

    for (const auto& [nameGet, valGet] : customGetFunctions_) {
        for (const auto& [nameSet, valSet] : customSetFunctions_) {
            if (nameGet == nameSet) {
                prototype->SetAccessorProperty(
                    vm, panda::StringRef::NewFromUtf8(vm, nameGet.c_str()), valGet.ToLocal(), valSet.ToLocal());
            }
        }
    }

    t->Set(vm, panda::Local<panda::JSValueRef>(panda::StringRef::NewFromUtf8(vm, ThisJSClass::JSName())),
        panda::Local<panda::JSValueRef>(classFunction_.ToLocal()));
}

template<typename C>
template<typename... Args>
void JsiClass<C>::Bind(BindingTarget t, JSDestructorCallback<C> dtor, JSGCMarkCallback<C> gcMark)
{
    auto runtime = std::static_pointer_cast<ArkJSRuntime>(JsiDeclarativeEngineInstance::GetCurrentRuntime());
    auto vm = const_cast<EcmaVM*>(runtime->GetEcmaVm());
    LocalScope scope(vm);
    classFunction_ = panda::Global<panda::FunctionRef>(
        vm, panda::FunctionRef::NewClassFunction(vm, InternalConstructor<Args...>, nullptr, nullptr));
    classFunction_->SetName(vm, StringRef::NewFromUtf8(vm, className_.c_str()));
    auto prototype = panda::Local<panda::ObjectRef>(classFunction_->GetFunctionPrototype(vm));
    prototype->Set(vm, panda::StringRef::NewFromUtf8(vm, "constructor"),
        panda::Local<panda::JSValueRef>(classFunction_.ToLocal()));
    for (const auto& [name, val] : staticFunctions_) {
        classFunction_->Set(vm, panda::StringRef::NewFromUtf8(vm, name.c_str()), val.ToLocal());
    }
    for (const auto& [name, val] : customFunctions_) {
        prototype->Set(vm, panda::StringRef::NewFromUtf8(vm, name.c_str()), val.ToLocal());
    }

    for (const auto& [nameGet, valGet] : customGetFunctions_) {
        for (const auto& [nameSet, valSet] : customSetFunctions_) {
            if (nameGet == nameSet) {
                prototype->SetAccessorProperty(
                    vm, panda::StringRef::NewFromUtf8(vm, nameGet.c_str()), valGet.ToLocal(), valSet.ToLocal());
            }
        }
    }

    t->Set(vm, panda::Local<panda::JSValueRef>(panda::StringRef::NewFromUtf8(vm, ThisJSClass::JSName())),
        panda::Local<panda::JSValueRef>(classFunction_.ToLocal()));
}

template<typename C>
template<typename Base>
void JsiClass<C>::Inherit()
{
    auto& staticFunctions = JsiClass<Base>::GetStaticFunctions();
    for (auto& [name, function] : staticFunctions) {
        if (staticFunctions_.find(name) != staticFunctions_.end()) {
            continue;
        }
        staticFunctions_.emplace(name, function);
    }
    auto& customFunctions = JsiClass<Base>::GetCustomFunctions();
    for (auto& [name, function] : customFunctions) {
        if (customFunctions_.find(name) != customFunctions_.end()) {
            continue;
        }
        customFunctions_.emplace(name, function);
    }
}

template<typename C>
template<typename Base>
void JsiClass<C>::InheritAndBind(
    BindingTarget t, JSFunctionCallback ctor, JSDestructorCallback<C> dtor, JSGCMarkCallback<C> gcMark)
{
    auto runtime = std::static_pointer_cast<ArkJSRuntime>(JsiDeclarativeEngineInstance::GetCurrentRuntime());
    auto vm = const_cast<EcmaVM*>(runtime->GetEcmaVm());
    InheritAndBind<Base>(vm, t, ctor, dtor, gcMark);
}

template<typename C>
template<typename Base>
void JsiClass<C>::InheritAndBind(
    EcmaVM* vm, BindingTarget t, JSFunctionCallback ctor, JSDestructorCallback<C> dtor, JSGCMarkCallback<C> gcMark)
{
    LocalScope scope(vm);
    panda::Local<panda::JSValueRef> hasExistRef =
        t->Get(vm, panda::Local<panda::JSValueRef>(panda::StringRef::NewFromUtf8(vm, ThisJSClass::JSName())));
    if (hasExistRef.IsEmpty()) {
        return;
    }

    auto binding = MakeJsiClassConstructorBinding<C>(ctor, dtor, gcMark);
    classFunction_ = panda::Global<panda::FunctionRef>(vm, JsiNewClassFunction(vm, binding));
    if (classFunction_.IsEmpty() || !classFunction_->IsFunction(vm)) {
        return;
    }
    classFunction_->SetName(vm, StringRef::NewFromUtf8(vm, className_.c_str()));

    panda::Local<panda::JSValueRef> getResult = t->Get(
        vm, panda::Local<panda::JSValueRef>(panda::StringRef::NewFromUtf8(vm, JSClassImpl<Base, JsiClass>::JSName())));
    if (getResult.IsEmpty()) {
        return;
    }

    panda::Local<panda::FunctionRef> baseClassFunction(getResult);
    classFunction_->SetPrototype(vm, baseClassFunction);
    auto prototype = panda::Local<panda::ObjectRef>(classFunction_->GetFunctionPrototype(vm));
    auto baseClassPrototype = panda::Local<panda::ObjectRef>(baseClassFunction->GetFunctionPrototype(vm));
    prototype->SetPrototype(vm, baseClassPrototype);
    prototype->Set(vm, panda::StringRef::NewFromUtf8(vm, "constructor"),
        panda::Local<panda::JSValueRef>(classFunction_.ToLocal()));

    for (const auto& [name, val] : staticFunctions_) {
        classFunction_->Set(vm, panda::StringRef::NewFromUtf8(vm, name.c_str()), val.ToLocal());
    }

    for (const auto& [name, val] : customFunctions_) {
        prototype->Set(vm, panda::StringRef::NewFromUtf8(vm, name.c_str()), val.ToLocal());
    }

    for (const auto& [nameGet, valGet] : customGetFunctions_) {
        for (const auto& [nameSet, valSet] : customSetFunctions_) {
            if (nameGet == nameSet) {
                prototype->SetAccessorProperty(
                    vm, panda::StringRef::NewFromUtf8(vm, nameGet.c_str()), valGet.ToLocal(), valSet.ToLocal());
            }
        }
    }

    t->Set(vm, panda::Local<panda::JSValueRef>(panda::StringRef::NewFromUtf8(vm, ThisJSClass::JSName())),
        panda::Local<panda::JSValueRef>(classFunction_.ToLocal()));
}

template<typename C>
std::unordered_map<std::string, panda::Global<panda::FunctionRef>>& JsiClass<C>::GetStaticFunctions()
{
    return staticFunctions_;
}

template<typename C>
std::unordered_map<std::string, panda::Global<panda::FunctionRef>>& JsiClass<C>::GetCustomFunctions()
{
    return customFunctions_;
}

template<typename C>
template<typename T, typename... Args>
panda::Local<panda::JSValueRef> JsiClass<C>::InternalMemberFunctionCallback(panda::JsiRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    panda::JsiFastNativeScope scope(vm);
    panda::Local<panda::JSValueRef> thisObj = runtimeCallInfo->GetThisRef();
    C* ptr = static_cast<C*>(panda::Local<panda::ObjectRef>(thisObj)->GetNativePointerField(vm, 0));
    T* instance = static_cast<T*>(ptr);
    auto binding = static_cast<FunctionBinding<T, panda::Local<panda::JSValueRef>, panda::JsiRuntimeCallInfo*>*>(
        runtimeCallInfo->GetData());
    if (binding == nullptr) {
        return panda::Local<panda::JSValueRef>(panda::JSValueRef::Undefined(vm));
    }
    STATIC_API_DURATION();
    ACE_BUILD_TRACE_BEGIN("[%s][%s]", ThisJSClass::JSName(), binding->Name());
    auto fnPtr = binding->Get();
    panda::Local<panda::JSValueRef> retVal = (instance->*fnPtr)(runtimeCallInfo);
    ACE_BUILD_TRACE_END()
    return retVal;
}

template<typename C>
template<typename T>
panda::Local<panda::JSValueRef> JsiClass<C>::InternalJSMemberFunctionCallback(
    panda::JsiRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    panda::JsiFastNativeScope scope(vm);
    panda::Local<panda::JSValueRef> thisObj = runtimeCallInfo->GetThisRef();
    C* ptr = static_cast<C*>(panda::Local<panda::ObjectRef>(thisObj)->GetNativePointerField(vm, 0));
    if (thisObj->IsProxy(vm)) {
        panda::Local<panda::ProxyRef> thisProxiedObj = static_cast<panda::Local<panda::ProxyRef>>(thisObj);
        ptr = static_cast<C*>(panda::Local<panda::ObjectRef>(thisProxiedObj->GetTarget(vm))->GetNativePointerField(
            vm, 0));
    }

    T* instance = static_cast<T*>(ptr);

    auto binding = static_cast<FunctionBinding<T, void, const JSCallbackInfo&>*>(runtimeCallInfo->GetData());
    if (binding == nullptr) {
        return panda::Local<panda::JSValueRef>(panda::JSValueRef::Undefined(vm));
    }
    STATIC_API_DURATION();
    ACE_BUILD_TRACE_BEGIN("[%s][%s]", ThisJSClass::JSName(), binding->Name());
    auto fnPtr = binding->Get();
    JsiCallbackInfo info(runtimeCallInfo);
    (instance->*fnPtr)(info);

    std::variant<void*, panda::CopyableGlobal<panda::JSValueRef>> retVal = info.GetReturnValue();
    auto jsVal = std::get_if<panda::CopyableGlobal<panda::JSValueRef>>(&retVal);
    ACE_BUILD_TRACE_END()
    if (jsVal) {
        return jsVal->ToLocal();
    }
    return panda::Local<panda::JSValueRef>(panda::JSValueRef::Undefined(vm));
}

template<typename C>
template<typename Class, typename R, typename... Args>
panda::Local<panda::JSValueRef> JsiClass<C>::MethodCallback(panda::JsiRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    panda::JsiFastNativeScope scope(vm);
    panda::Local<panda::JSValueRef> thisObj = runtimeCallInfo->GetThisRef();
    C* ptr = static_cast<C*>(panda::Local<panda::ObjectRef>(thisObj)->GetNativePointerField(vm, 0));
    Class* instance = static_cast<Class*>(ptr);
    auto binding = static_cast<FunctionBinding<Class, R, Args...>*>(runtimeCallInfo->GetData());
    if (binding == nullptr) {
        return panda::Local<panda::JSValueRef>(panda::JSValueRef::Undefined(vm));
    }
    STATIC_API_DURATION();
    ACE_BUILD_TRACE_BEGIN("[%s][%s]", ThisJSClass::JSName(), binding->Name());
    auto fnPtr = binding->Get();
    auto tuple = __detail__::ToTuple<std::decay_t<Args>...>(runtimeCallInfo);
    bool returnSelf = binding->Options() & MethodOptions::RETURN_SELF;
    constexpr bool isVoid = std::is_void_v<R>;
    constexpr bool hasArguments = sizeof...(Args) != 0;

    if constexpr (isVoid && hasArguments) {
        // C::MemberFunction(Args...)
        FunctionUtils::CallMemberFunction(instance, fnPtr, tuple);
        ACE_BUILD_TRACE_END()
        return returnSelf ? thisObj : panda::Local<panda::JSValueRef>(panda::JSValueRef::Undefined(vm));
    } else if constexpr (isVoid && !hasArguments) {
        // C::MemberFunction()
        (instance->*fnPtr)();
        ACE_BUILD_TRACE_END()
        return returnSelf ? thisObj : panda::Local<panda::JSValueRef>(panda::JSValueRef::Undefined(vm));
    } else if constexpr (!isVoid && hasArguments) {
        // R C::MemberFunction(Args...)
        auto result = FunctionUtils::CallMemberFunction(instance, fnPtr, tuple);
        ACE_BUILD_TRACE_END()
        return JsiValueConvertor::toJsiValueWithVM<R>(vm, result);
    } else if constexpr (!isVoid && !hasArguments) {
        // R C::MemberFunction()
        auto res = (instance->*fnPtr)();
        ACE_BUILD_TRACE_END()
        return JsiValueConvertor::toJsiValueWithVM<R>(vm, res);
    }
    ACE_BUILD_TRACE_END()
}

template<typename C>
template<typename R, typename... Args>
panda::Local<panda::JSValueRef> JsiClass<C>::StaticMethodCallback(panda::JsiRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    panda::JsiFastNativeScope scope(vm);
    auto binding = static_cast<StaticFunctionBinding<R, Args...>*>(runtimeCallInfo->GetData());
    if (binding == nullptr) {
        return panda::Local<panda::JSValueRef>(panda::JSValueRef::Undefined(vm));
    }
    STATIC_API_DURATION();
    ACE_BUILD_TRACE_BEGIN("[%s][%s]", ThisJSClass::JSName(), binding->Name());
    auto fnPtr = binding->Get();
    auto tuple = __detail__::ToTuple<std::decay_t<Args>...>(runtimeCallInfo);
    bool returnSelf = binding->Options() & MethodOptions::RETURN_SELF;
    constexpr bool isVoid = std::is_void_v<R>;
    constexpr bool hasArguments = sizeof...(Args) != 0;

    panda::Local<panda::JSValueRef> thisObj = runtimeCallInfo->GetThisRef();
    if constexpr (isVoid && hasArguments) {
        // void C::MemberFunction(Args...)
        FunctionUtils::CallStaticMemberFunction(fnPtr, tuple);
        ACE_BUILD_TRACE_END()
        return returnSelf ? thisObj : panda::Local<panda::JSValueRef>(panda::JSValueRef::Undefined(vm));
    } else if constexpr (isVoid && !hasArguments) {
        // void C::MemberFunction()
        fnPtr();
        ACE_BUILD_TRACE_END()
        return panda::JSValueRef::Undefined(vm);
    } else if constexpr (!isVoid && hasArguments) {
        // R C::MemberFunction(Args...)
        auto result = FunctionUtils::CallStaticMemberFunction(fnPtr, tuple);
        ACE_BUILD_TRACE_END()
        return JsiValueConvertor::toJsiValueWithVM(vm, result);
    } else if constexpr (!isVoid && !hasArguments) {
        // R C::MemberFunction()
        auto res = fnPtr();
        ACE_BUILD_TRACE_END()
        return JsiValueConvertor::toJsiValueWithVM(vm, res);
    }
    ACE_BUILD_TRACE_END()
}

template<typename C>
panda::Local<panda::JSValueRef> JsiClass<C>::JSStaticMethodCallback(panda::JsiRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    panda::JsiFastNativeScope scope(vm);
    auto binding = static_cast<StaticFunctionBinding<void, const JSCallbackInfo&>*>(runtimeCallInfo->GetData());
    if (binding == nullptr) {
        return panda::Local<panda::JSValueRef>(panda::JSValueRef::Undefined(vm));
    }
    STATIC_API_DURATION();
    ACE_BUILD_TRACE_BEGIN("[%s][%s]", ThisJSClass::JSName(), binding->Name());
    auto fnPtr = binding->Get();
    JsiCallbackInfo info(runtimeCallInfo);
    fnPtr(info);
    std::variant<void*, panda::CopyableGlobal<panda::JSValueRef>> retVal = info.GetReturnValue();
    auto jsVal = std::get_if<panda::CopyableGlobal<panda::JSValueRef>>(&retVal);
    ACE_BUILD_TRACE_END()
    if (jsVal) {
        return jsVal->ToLocal();
    }
    return panda::Local<panda::JSValueRef>(panda::JSValueRef::Undefined(vm));
}

template<typename C>
template<typename... Args>
panda::Local<panda::JSValueRef> JsiClass<C>::InternalConstructor(panda::JsiRuntimeCallInfo* runtimeCallInfo)
{
    panda::Local<panda::JSValueRef> newTarget = runtimeCallInfo->GetNewTargetRef();
    EcmaVM* vm = runtimeCallInfo->GetVM();
    if (!newTarget->IsFunction(vm)) {
        return panda::Local<panda::JSValueRef>(panda::JSValueRef::Undefined(runtimeCallInfo->GetVM()));
    }
    panda::Local<panda::JSValueRef> thisObj = runtimeCallInfo->GetThisRef();
    auto tuple = __detail__::ToTuple<std::decay_t<Args>...>(runtimeCallInfo);
    C* instance = FunctionUtils::ConstructFromTuple<C>(tuple);
    Local<ObjectRef>(thisObj)->SetNativePointerFieldCount(vm, 1);
    panda::Local<panda::ObjectRef>(thisObj)->SetNativePointerField(vm, 0, static_cast<void*>(instance));
    return thisObj;
}

template<typename C>
bool JsiClass<C>::CheckIfConstructCall(panda::JsiRuntimeCallInfo* runtimeCallInfo)
{
    return true;
}

template<typename C>
panda::Local<panda::JSValueRef> JsiClass<C>::ConstructorInterceptor(panda::JsiRuntimeCallInfo* runtimeCallInfo)
{
    panda::Local<panda::JSValueRef> newTarget = runtimeCallInfo->GetNewTargetRef();
    EcmaVM* vm = runtimeCallInfo->GetVM();
    if (!newTarget->IsFunction(vm)) {
        return panda::Local<panda::JSValueRef>(panda::JSValueRef::Undefined(vm));
    }
    return constructor_(runtimeCallInfo);
}

panda::Local<panda::JSValueRef> JsiJSConstructorInterceptor(panda::JsiRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    panda::Local<panda::JSValueRef> newTarget = runtimeCallInfo->GetNewTargetRef();
    auto* owner = static_cast<JsiClassConstructorBindingRef*>(runtimeCallInfo->GetData());
    if (owner == nullptr || !*owner) {
        panda::JSNApi::ThrowException(
            vm, panda::Exception::TypeError(vm, panda::StringRef::NewFromUtf8(vm, "Constructor binding is null")));
        return panda::Local<panda::JSValueRef>(panda::JSValueRef::Undefined(vm));
    }
    auto binding = *owner;
    if (newTarget->IsFunction(vm) && binding->jsConstructor != nullptr) {
        JsiCallbackInfo info(runtimeCallInfo);
        binding->jsConstructor(info);
        auto retVal = info.GetReturnValue();
        if (retVal.valueless_by_exception()) {
            return panda::Local<panda::JSValueRef>(panda::JSValueRef::Undefined(vm));
        }
        auto instance = std::get_if<void*>(&retVal);
        if (instance != nullptr) {
            if (panda::JSNApi::HasPendingException(vm)) {
                if (*instance != nullptr && binding->jsDestructorErased) {
                    binding->jsDestructorErased(*instance);
                }
                return panda::JSValueRef::Undefined(vm);
            }
            panda::Local<panda::JSValueRef> thisObj = runtimeCallInfo->GetThisRef();
            Local<ObjectRef>(thisObj)->SetNativePointerFieldCount(vm, 1);
            size_t nativeSize = info.GetSize();
            auto instanceOwner = std::make_unique<JsiClassConstructorBindingRef>(binding);
            Local<ObjectRef>(thisObj)->SetNativePointerField(
                vm, 0, *instance, JsiJSNativePointerDestructorInterceptor, instanceOwner.get(), nativeSize);
            if (!panda::JSNApi::HasPendingException(vm)) {
                // Instances can outlive their constructor and the registering thread.
                instanceOwner.release();
            }
            return thisObj;
        }
    }
    return panda::Local<panda::JSValueRef>(panda::JSValueRef::Undefined(vm));
}

inline void JsiJSNativePointerDestructorInterceptor(void* env, void* nativePtr, void* data)
{
    std::unique_ptr<JsiClassConstructorBindingRef> owner(static_cast<JsiClassConstructorBindingRef*>(data));
    if (owner && *owner && (*owner)->jsDestructorErased) {
        (*owner)->jsDestructorErased(nativePtr);
    }
}

template<typename C>
panda::Local<panda::JSValueRef> JsiClass<C>::NewInstance()
{
    auto runtime = std::static_pointer_cast<ArkJSRuntime>(JsiDeclarativeEngineInstance::GetCurrentRuntime());
    auto vm = const_cast<EcmaVM*>(runtime->GetEcmaVm());
    return classFunction_->Constructor(vm, nullptr, 0);
}
} // namespace OHOS::Ace::Framework

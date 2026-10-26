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

#include "modal_ui_extension_module.h"

#include "utils/ani_utils.h"
#include "common/common_module.h"
#include "load.h"
#include "ani_common_want.h"
#include "core/common/container.h"
#include "core/common/container_scope.h"
#include "core/interfaces/ani/ani_api.h"
#include "core/interfaces/native/implementation/transition_effect_peer_impl.h"
#include "interfaces/inner_api/ace/ui_content.h"
#include "interfaces/inner_api/ace/modal_ui_extension_config.h"
#include "interfaces/inner_api/ace/modal_ui_extension_proxy.h"
#include "core/components_ng/pattern/ui_extension/ui_extension_component/modal_ui_extension_proxy_impl.h"
#include "core/components_ng/pattern/ui_extension/ui_extension_component/ui_extension_proxy.h"
#include "core/components_ng/pattern/ui_extension/ui_extension_component/ui_extension_pattern.h"
#include "core/components_ng/pattern/overlay/overlay_manager.h"

namespace OHOS::Ace::Ani {
namespace {

enum class ModalRegisterType {
    UNKNOWN = 0,
    SYNC,
    ASYNC,
};

using ModalAniCallbackList =
    std::list<std::pair<ani_ref, std::function<void(const RefPtr<NG::UIExtensionProxy>&)>>>;

struct ModalUIExtensionProxyAniHolder {
    std::shared_ptr<ModalUIExtensionProxy> proxy;
    int32_t instanceId = 0;
    ani_env* callbackEnv = nullptr;
    ModalAniCallbackList onAsyncCallbackList;
    ModalAniCallbackList onSyncCallbackList;
    std::mutex callbackLock;
};

class ScopedAniEnv {
public:
    explicit ScopedAniEnv(ani_vm* vm) : vm_(vm)
    {
        status_ = GetAniEnv(vm_, &env_);
    }
    ~ScopedAniEnv()
    {
        if (vm_ && status_ == ANI_OK) {
            vm_->DetachCurrentThread();
        }
    }
    ani_env* Get() const { return env_; }
private:
    ani_vm* vm_ = nullptr;
    ani_env* env_ = nullptr;
    ani_status status_ = ANI_ERROR;
};

std::shared_ptr<CommonModuleCallbackAni> ExtractCallbackAni(ani_env* env, ani_object callback, const char* name)
{
    ani_ref cbRef = nullptr;
    if (ANI_OK != env->Object_GetPropertyByName_Ref(callback, name, &cbRef)) {
        return nullptr;
    }
    AniLocalRefGuard cbRefGuard(env, cbRef);
    if (AniUtils::IsUndefined(env, cbRef)) {
        return nullptr;
    }
    return std::make_shared<CommonModuleCallbackAni>(env, cbRef);
}

void CallAniCallback(CommonModuleCallbackAni* func, ani_env* callbackEnv, ani_size argc, ani_ref* argv)
{
    CHECK_NULL_VOID(func);
    ani_ref ret = nullptr;
    func->Call(callbackEnv, argc, argv, &ret);
}

std::function<void(const std::shared_ptr<ModalUIExtensionProxy>&)> CreateAniOnRemoteReady(
    ani_env* env, ani_object callback, int32_t instanceId)
{
    auto cb = ExtractCallbackAni(env, callback, "onRemoteReady");
    if (!cb) {
        return nullptr;
    }
    return [env, func = cb, instanceId](const std::shared_ptr<ModalUIExtensionProxy>& proxy) {
        ani_vm* vm = nullptr;
        env->GetVM(&vm);
        CHECK_NULL_VOID(vm);
        ScopedAniEnv scopedEnv(vm);
        ani_env* callbackEnv = scopedEnv.Get();
        CHECK_NULL_VOID(callbackEnv);
        auto* holder = new ModalUIExtensionProxyAniHolder {
            .proxy = proxy,
            .instanceId = instanceId,
            .callbackEnv = callbackEnv,
        };
        ani_long proxyPtr = reinterpret_cast<ani_long>(holder);
        ani_ref arg = AniUtils::CreateLong(callbackEnv, proxyPtr);
        CHECK_NULL_VOID(arg);
        AniLocalRefGuard argGuard(callbackEnv, arg);
        CallAniCallback(func.get(), callbackEnv, 1, &arg);
    };
}

std::function<void(const AAFwk::WantParams&)> CreateAniOnReceive(ani_env* env, ani_object callback)
{
    auto cb = ExtractCallbackAni(env, callback, "onReceive");
    if (!cb) {
        return nullptr;
    }
    return [env, func = cb](const AAFwk::WantParams& wantParams) {
        ani_vm* vm = nullptr;
        env->GetVM(&vm);
        CHECK_NULL_VOID(vm);
        ScopedAniEnv scopedEnv(vm);
        ani_env* callbackEnv = scopedEnv.Get();
        CHECK_NULL_VOID(callbackEnv);
        ani_ref wantParamsRef = OHOS::AppExecFwk::WrapWantParams(callbackEnv, wantParams);
        if (wantParamsRef == nullptr) {
            return;
        }
        AniLocalRefGuard argGuard(callbackEnv, wantParamsRef);
        CallAniCallback(func.get(), callbackEnv, 1, &wantParamsRef);
    };
}

std::function<void(int32_t, const std::string&, const std::string&)> CreateAniOnError(
    ani_env* env, ani_object callback)
{
    auto cb = ExtractCallbackAni(env, callback, "onError");
    if (!cb) {
        return nullptr;
    }
    return [env, func = cb](int32_t code, const std::string& name, const std::string& message) {
        ani_vm* vm = nullptr;
        env->GetVM(&vm);
        CHECK_NULL_VOID(vm);
        ScopedAniEnv scopedEnv(vm);
        ani_env* callbackEnv = scopedEnv.Get();
        CHECK_NULL_VOID(callbackEnv);
        auto errorObj = AniUtils::CreateBusinessError(callbackEnv, message.c_str(), code);
        ani_ref arg = static_cast<ani_ref>(errorObj);
        AniLocalRefGuard argGuard(callbackEnv, arg);
        CallAniCallback(func.get(), callbackEnv, 1, &arg);
    };
}

std::function<void(int32_t)> CreateAniOnTerminated(ani_env* env, ani_object callback)
{
    auto cb = ExtractCallbackAni(env, callback, "onTerminated");
    if (!cb) {
        return nullptr;
    }
    return [env, func = cb](int32_t code) {
        ani_vm* vm = nullptr;
        env->GetVM(&vm);
        CHECK_NULL_VOID(vm);
        ScopedAniEnv scopedEnv(vm);
        ani_env* callbackEnv = scopedEnv.Get();
        CHECK_NULL_VOID(callbackEnv);
        ani_ref arg = AniUtils::CreateInt32(callbackEnv, code);
        CHECK_NULL_VOID(arg);
        AniLocalRefGuard argGuard(callbackEnv, arg);
        CallAniCallback(func.get(), callbackEnv, 1, &arg);
    };
}

std::function<void()> CreateAniOnDrawReady(ani_env* env, ani_object callback)
{
    auto cb = ExtractCallbackAni(env, callback, "onDrawReady");
    if (!cb) {
        return nullptr;
    }
    return [env, func = cb]() {
        ani_vm* vm = nullptr;
        env->GetVM(&vm);
        CHECK_NULL_VOID(vm);
        ScopedAniEnv scopedEnv(vm);
        ani_env* callbackEnv = scopedEnv.Get();
        CHECK_NULL_VOID(callbackEnv);
        CallAniCallback(func.get(), callbackEnv, 0, nullptr);
    };
}

void ParseAniCallbacks(ani_env* env, ani_object callback, ModalUIExtensionCallbacks& callbacks, int32_t instanceId)
{
    callbacks.onRemoteReady = CreateAniOnRemoteReady(env, callback, instanceId);
    callbacks.onReceive = CreateAniOnReceive(env, callback);
    callbacks.onError = CreateAniOnError(env, callback);
    callbacks.onRelease = CreateAniOnTerminated(env, callback);
    callbacks.onDrawReady = CreateAniOnDrawReady(env, callback);
}

void ParseTransitionFromAniOptions(ani_env* env, ani_object options, NG::InnerExtraConfig& extraConfig)
{
    ani_ref transitionRef = nullptr;
    if (ANI_OK != env->Object_GetPropertyByName_Ref(options, "transition", &transitionRef)) {
        return;
    }
    AniLocalRefGuard transitionGuard(env, transitionRef);
    if (AniUtils::IsUndefined(env, transitionRef)) {
        return;
    }
    auto transitionObj = static_cast<ani_object>(transitionRef);
    ani_ref peerRef = nullptr;
    if (ANI_OK != env->Object_GetPropertyByName_Ref(transitionObj, "peer", &peerRef)) {
        return;
    }
    AniLocalRefGuard peerGuard(env, peerRef);
    if (AniUtils::IsUndefined(env, peerRef)) {
        return;
    }
    ani_long peerPtr = 0;
    if (ANI_OK != env->Object_GetFieldByName_Long(
        static_cast<ani_object>(peerRef), "ptr", &peerPtr) || peerPtr == 0) {
        return;
    }
    auto* peer = reinterpret_cast<TransitionEffectPeer*>(peerPtr);
    if (peer && peer->handler) {
        extraConfig.transition = peer->handler;
    }
}

constexpr int32_t FOLLOW_HOST_STRATEGY = 0;

void ParseDpiFollowStrategy(ani_env* env, ani_object options, ModalUIExtensionConfig& config)
{
    ani_ref dpiStrategyRef = nullptr;
    if (ANI_OK != env->Object_GetPropertyByName_Ref(options, "dpiFollowStrategy", &dpiStrategyRef)
        || AniUtils::IsUndefined(env, dpiStrategyRef)) {
        return;
    }
    AniLocalRefGuard dpiGuard(env, dpiStrategyRef);
    auto dpiObj = static_cast<ani_object>(dpiStrategyRef);
    if (!AniUtils::IsClassObject(env, dpiObj, "std.core.Int")) {
        return;
    }
    ani_int dpiStrategy = 0;
    if (ANI_OK != env->Object_CallMethodByName_Int(dpiObj, "toInt", ":i", &dpiStrategy)) {
        return;
    }
    config.isDensityFollowHost = (static_cast<int32_t>(dpiStrategy) == FOLLOW_HOST_STRATEGY);
}

void ParseWindowModeFollowStrategy(ani_env* env, ani_object options, ModalUIExtensionConfig& config)
{
    ani_ref windowModeRef = nullptr;
    if (ANI_OK != env->Object_GetPropertyByName_Ref(options, "isWindowModeFollowHost", &windowModeRef)
        || AniUtils::IsUndefined(env, windowModeRef)) {
        return;
    }
    AniLocalRefGuard windowModeGuard(env, windowModeRef);
    auto windowModeObj = static_cast<ani_object>(windowModeRef);
    if (!AniUtils::IsClassObject(env, windowModeObj, "std.core.Boolean")) {
        return;
    }
    ani_boolean isWindowModeFollowHost = ANI_FALSE;
    if (ANI_OK != env->Object_CallMethodByName_Boolean(
        windowModeObj, "toBoolean", nullptr, &isWindowModeFollowHost)) {
        return;
    }
    config.isWindowModeFollowHost = isWindowModeFollowHost;
}

ModalUIExtensionConfig ParseConfigFromAniOptions(ani_env* env, ani_object options, NG::InnerExtraConfig& extraConfig)
{
    ModalUIExtensionConfig config;
    config.isProhibitBack = true;
    config.prohibitedRemoveByRouter = true;
    config.isAllowAddChildBelowModalUec = true;
    config.isDensityFollowHost = true;
    config.isWindowModeFollowHost = true;
    config.isModalFixFocus = true;
    ParseDpiFollowStrategy(env, options, config);
    ParseWindowModeFollowStrategy(env, options, config);
    ParseTransitionFromAniOptions(env, options, extraConfig);
    return config;
}
} // namespace

void ModalUIExtensionProxySend(ani_env* env, [[maybe_unused]] ani_object obj, ani_long proxyPtr, ani_object data)
{
    CHECK_NULL_VOID(env);
    auto* holder = reinterpret_cast<ModalUIExtensionProxyAniHolder*>(proxyPtr);
    CHECK_NULL_VOID(holder);
    CHECK_NULL_VOID(holder->proxy);
    OHOS::AAFwk::WantParams requestParams;
    if (!OHOS::AppExecFwk::UnwrapWantParams(env, data, requestParams)) {
        return;
    }
    holder->proxy->SendData(requestParams);
}

ani_object ModalUIExtensionProxySendSync(
    ani_env* env, [[maybe_unused]] ani_object obj, ani_long proxyPtr, ani_object data)
{
    ani_object resultObj = {};
    CHECK_NULL_RETURN(env, resultObj);
    auto* holder = reinterpret_cast<ModalUIExtensionProxyAniHolder*>(proxyPtr);
    CHECK_NULL_RETURN(holder, resultObj);
    CHECK_NULL_RETURN(holder->proxy, resultObj);
    OHOS::AAFwk::WantParams requestParams;
    if (!OHOS::AppExecFwk::UnwrapWantParams(env, data, requestParams)) {
        return resultObj;
    }
    OHOS::AAFwk::WantParams replyParams;
    auto* modalProxy = static_cast<OHOS::Ace::NG::ModalUIExtensionProxyImpl*>(holder->proxy.get());
    CHECK_NULL_RETURN(modalProxy, resultObj);
    modalProxy->SendDataSync(requestParams, replyParams);
    ani_ref wantParamsObj = OHOS::AppExecFwk::WrapWantParams(env, replyParams);
    CHECK_NULL_RETURN(wantParamsObj, resultObj);
    return static_cast<ani_object>(wantParamsObj);
}

std::list<std::function<void(const RefPtr<NG::UIExtensionProxy>&)>> GetModalOnFuncList(
    ModalUIExtensionProxyAniHolder* holder, ModalRegisterType type)
{
    std::list<std::function<void(const RefPtr<NG::UIExtensionProxy>&)>> reList;
    if (type == ModalRegisterType::SYNC) {
        for (const auto& item : holder->onSyncCallbackList) {
            reList.emplace_back(item.second);
        }
    } else if (type == ModalRegisterType::ASYNC) {
        for (const auto& item : holder->onAsyncCallbackList) {
            reList.emplace_back(item.second);
        }
    }
    return reList;
}

void PostModalUpdateCallbacksTask(ModalUIExtensionProxyAniHolder* holder, ModalRegisterType type)
{
    CHECK_NULL_VOID(holder);
    auto* modalProxy = static_cast<OHOS::Ace::NG::ModalUIExtensionProxyImpl*>(holder->proxy.get());
    CHECK_NULL_VOID(modalProxy);
    auto innerProxy = modalProxy->GetInnerProxy();
    CHECK_NULL_VOID(innerProxy);
    auto pattern = innerProxy->GetPattern();
    CHECK_NULL_VOID(pattern);
    auto onFuncList = GetModalOnFuncList(holder, type);
    auto instanceId = holder->instanceId;
    auto container = Container::GetContainer(instanceId);
    CHECK_NULL_VOID(container);
    auto taskExecutor = container->GetTaskExecutor();
    CHECK_NULL_VOID(taskExecutor);
    taskExecutor->PostTask([pattern, onFuncList = std::move(onFuncList), type]() {
        if (type == ModalRegisterType::SYNC) {
            pattern->SetSyncCallbacks(std::move(onFuncList));
        } else if (type == ModalRegisterType::ASYNC) {
            pattern->SetAsyncCallbacks(std::move(onFuncList));
        }
        }, TaskExecutor::TaskType::UI, "ArkUIModalUIExtensionProxyUpdateCallbacks");
}

std::function<void(const RefPtr<NG::UIExtensionProxy>&)> CreateAniReceiverCallback(
    ModalUIExtensionProxyAniHolder* holder, ani_ref cbRef)
{
    ani_env* callbackEnv = holder->callbackEnv;
    int32_t instanceId = holder->instanceId;
    auto cb = std::make_shared<CommonModuleCallbackAni>(callbackEnv, cbRef);
    return [callbackEnv, cb, instanceId](const RefPtr<NG::UIExtensionProxy>& session) {
        ani_vm* vm = nullptr;
        callbackEnv->GetVM(&vm);
        CHECK_NULL_VOID(vm);
        ScopedAniEnv scopedEnv(vm);
        ani_env* env = scopedEnv.Get();
        CHECK_NULL_VOID(env);
        ContainerScope scope(instanceId);
        auto modalProxy = std::make_shared<OHOS::Ace::NG::ModalUIExtensionProxyImpl>(session);
        auto* receiverHolder = new ModalUIExtensionProxyAniHolder {
            .proxy = modalProxy,
            .instanceId = instanceId,
            .callbackEnv = env,
        };
        ani_long proxyPtr = reinterpret_cast<ani_long>(receiverHolder);
        ani_ref arg = AniUtils::CreateLong(env, proxyPtr);
        CHECK_NULL_VOID(arg);
        AniLocalRefGuard argGuard(env, arg);
        CallAniCallback(cb.get(), env, 1, &arg);
    };
}

void ModalUIExtensionProxyOnAsyncReceiverRegister(
    ani_env* env, [[maybe_unused]] ani_object obj, ani_long proxyPtr, ani_object callback)
{
    CHECK_NULL_VOID(env);
    auto* holder = reinterpret_cast<ModalUIExtensionProxyAniHolder*>(proxyPtr);
    CHECK_NULL_VOID(holder);
    CHECK_NULL_VOID(holder->proxy);
    if (!callback || AniUtils::IsUndefined(env, callback)) {
        return;
    }
    ani_ref globalCbRef = nullptr;
    if (ANI_OK != env->GlobalReference_Create(callback, &globalCbRef) || globalCbRef == nullptr) {
        return;
    }
    auto jsFunc = CreateAniReceiverCallback(holder, globalCbRef);
    {
        std::lock_guard<std::mutex> lock(holder->callbackLock);
        holder->onAsyncCallbackList.emplace_back(globalCbRef, jsFunc);
    }
    PostModalUpdateCallbacksTask(holder, ModalRegisterType::ASYNC);
}

void ModalUIExtensionProxyOnSyncReceiverRegister(
    ani_env* env, [[maybe_unused]] ani_object obj, ani_long proxyPtr, ani_object callback)
{
    CHECK_NULL_VOID(env);
    auto* holder = reinterpret_cast<ModalUIExtensionProxyAniHolder*>(proxyPtr);
    CHECK_NULL_VOID(holder);
    CHECK_NULL_VOID(holder->proxy);
    if (!callback || AniUtils::IsUndefined(env, callback)) {
        return;
    }
    ani_ref globalCbRef = nullptr;
    if (ANI_OK != env->GlobalReference_Create(callback, &globalCbRef) || globalCbRef == nullptr) {
        return;
    }
    auto jsFunc = CreateAniReceiverCallback(holder, globalCbRef);
    {
        std::lock_guard<std::mutex> lock(holder->callbackLock);
        holder->onSyncCallbackList.emplace_back(globalCbRef, jsFunc);
    }
    PostModalUpdateCallbacksTask(holder, ModalRegisterType::SYNC);
}

void RemoveAniCallbacks(ani_env* env, ani_env* callbackEnv, ani_object callback,
    ModalAniCallbackList& list)
{
    if (!callback || AniUtils::IsUndefined(env, callback)) {
        for (const auto& item : list) {
            if (callbackEnv) {
                callbackEnv->GlobalReference_Delete(item.first);
            }
        }
        list.clear();
        return;
    }
    for (auto iter = list.begin(); iter != list.end();) {
        ani_boolean equals = ANI_FALSE;
        env->Reference_Equals(callback, iter->first, &equals);
        if (equals == ANI_TRUE) {
            if (callbackEnv) {
                callbackEnv->GlobalReference_Delete(iter->first);
            }
            iter = list.erase(iter);
        } else {
            ++iter;
        }
    }
}

void ModalUIExtensionProxyOffAsyncReceiverRegister(
    ani_env* env, [[maybe_unused]] ani_object obj, ani_long proxyPtr, ani_object callback)
{
    CHECK_NULL_VOID(env);
    auto* holder = reinterpret_cast<ModalUIExtensionProxyAniHolder*>(proxyPtr);
    CHECK_NULL_VOID(holder);
    CHECK_NULL_VOID(holder->proxy);
    ani_env* callbackEnv = holder->callbackEnv;
    {
        std::lock_guard<std::mutex> lock(holder->callbackLock);
        RemoveAniCallbacks(env, callbackEnv, callback, holder->onAsyncCallbackList);
    }
    PostModalUpdateCallbacksTask(holder, ModalRegisterType::ASYNC);
}

void ModalUIExtensionProxyOffSyncReceiverRegister(
    ani_env* env, [[maybe_unused]] ani_object obj, ani_long proxyPtr, ani_object callback)
{
    CHECK_NULL_VOID(env);
    auto* holder = reinterpret_cast<ModalUIExtensionProxyAniHolder*>(proxyPtr);
    CHECK_NULL_VOID(holder);
    CHECK_NULL_VOID(holder->proxy);
    ani_env* callbackEnv = holder->callbackEnv;
    {
        std::lock_guard<std::mutex> lock(holder->callbackLock);
        RemoveAniCallbacks(env, callbackEnv, callback, holder->onSyncCallbackList);
    }
    PostModalUpdateCallbacksTask(holder, ModalRegisterType::SYNC);
}

void ModalUIExtensionProxyDestroy(ani_env* env, [[maybe_unused]] ani_object obj, ani_long proxyPtr)
{
    CHECK_NULL_VOID(env);
    auto* holder = reinterpret_cast<ModalUIExtensionProxyAniHolder*>(proxyPtr);
    CHECK_NULL_VOID(holder);
    ani_env* callbackEnv = holder->callbackEnv;
    if (callbackEnv) {
        for (const auto& item : holder->onAsyncCallbackList) {
            callbackEnv->GlobalReference_Delete(item.first);
        }
        for (const auto& item : holder->onSyncCallbackList) {
            callbackEnv->GlobalReference_Delete(item.first);
        }
    }
    delete holder;
}

ani_int CreateModalUIExtension(ani_env* env, [[maybe_unused]] ani_object obj,
    ani_object wantObj, ani_object options, ani_object callback)
{
    CHECK_NULL_RETURN(env, 0);
    CHECK_NULL_RETURN(wantObj, 0);
    if (!callback || AniUtils::IsUndefined(env, callback)) {
        return 0;
    }

    AAFwk::Want want;
    if (!OHOS::AppExecFwk::UnwrapWant(env, wantObj, want)) {
        return 0;
    }

    ModalUIExtensionConfig config;
    NG::InnerExtraConfig extraConfig;
    ArkUI_Int32 instanceId = -1;
    if (options) {
        config = ParseConfigFromAniOptions(env, options, extraConfig);
        ani_ref instanceIdRef = nullptr;
        if (ANI_OK == env->Object_GetPropertyByName_Ref(options, "instanceId", &instanceIdRef)
            && !AniUtils::IsUndefined(env, instanceIdRef)) {
            AniLocalRefGuard instanceIdGuard(env, instanceIdRef);
            ani_int instanceIdVal = 0;
            if (ANI_OK == env->Object_CallMethodByName_Int(
                static_cast<ani_object>(instanceIdRef), "toInt", ":i", &instanceIdVal)) {
                instanceId = instanceIdVal;
            }
        }
    }

    ModalUIExtensionCallbacks callbacks;
    ParseAniCallbacks(env, callback, callbacks, instanceId);

    const auto* modifier = GetNodeAniModifier();
    CHECK_NULL_RETURN(modifier, 0);
    auto commonModifier = modifier->getCommonAniModifier();
    CHECK_NULL_RETURN(commonModifier, 0);
    return commonModifier->createModalUIExtension(want, callbacks, config, extraConfig, instanceId);
}

void CloseModalUIExtension(ani_env* env, [[maybe_unused]] ani_object obj, ani_int sessionId, ani_int instanceId)
{
    CHECK_NULL_VOID(env);
    const auto* modifier = GetNodeAniModifier();
    CHECK_NULL_VOID(modifier);
    auto commonModifier = modifier->getCommonAniModifier();
    CHECK_NULL_VOID(commonModifier);
    commonModifier->closeModalUIExtension(sessionId, instanceId);
}
} // namespace OHOS::Ace::Ani

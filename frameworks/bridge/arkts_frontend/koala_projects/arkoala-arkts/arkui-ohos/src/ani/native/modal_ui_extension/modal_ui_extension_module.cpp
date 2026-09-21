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

namespace OHOS::Ace::Ani {
namespace {

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
    if (!AniUtils::IsFunction(env, static_cast<ani_object>(cbRef))) {
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
    ani_env* env, ani_object callback)
{
    auto cb = ExtractCallbackAni(env, callback, "onRemoteReady");
    if (!cb) {
        return nullptr;
    }
    return [env, func = cb](const std::shared_ptr<ModalUIExtensionProxy>& proxy) {
        ani_vm* vm = nullptr;
        env->GetVM(&vm);
        CHECK_NULL_VOID(vm);
        ScopedAniEnv scopedEnv(vm);
        ani_env* callbackEnv = scopedEnv.Get();
        CHECK_NULL_VOID(callbackEnv);
        auto proxyStorage = std::make_shared<std::shared_ptr<ModalUIExtensionProxy>>(proxy);
        ani_long proxyPtr = reinterpret_cast<ani_long>(proxyStorage.get());
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
        HILOGE("ModalUIExtension onReceive: WantParams to ANI conversion not yet supported");
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

void ParseAniCallbacks(ani_env* env, ani_object callback, ModalUIExtensionCallbacks& callbacks)
{
    callbacks.onRemoteReady = CreateAniOnRemoteReady(env, callback);
    callbacks.onReceive = CreateAniOnReceive(env, callback);
    callbacks.onError = CreateAniOnError(env, callback);
    callbacks.onRelease = CreateAniOnTerminated(env, callback);
    callbacks.onDrawReady = CreateAniOnDrawReady(env, callback);
}

ModalUIExtensionConfig ParseConfigFromAniOptions(ani_env* env, ani_object options)
{
    int32_t dpiStrategy = 0;
    AniUtils::GetIntByName(env, options, "dpiFollowStrategy", dpiStrategy);
    int32_t windowModeStrategy = 0;
    AniUtils::GetIntByName(env, options, "windowModeFollowStrategy", windowModeStrategy);
    ModalUIExtensionConfig config;
    config.isDensityFollowHost = (dpiStrategy == 0);
    config.isWindowModeFollowHost = (windowModeStrategy == 0);

    ani_ref transitionRef = nullptr;
    if (ANI_OK == env->Object_GetPropertyByName_Ref(options, "transition", &transitionRef)) {
        AniLocalRefGuard transitionGuard(env, transitionRef);
        if (!AniUtils::IsUndefined(env, transitionRef)) {
            auto transitionObj = static_cast<ani_object>(transitionRef);
            ani_ref peerRef = nullptr;
            if (ANI_OK == env->Object_GetPropertyByName_Ref(transitionObj, "peer", &peerRef)) {
                AniLocalRefGuard peerGuard(env, peerRef);
                if (!AniUtils::IsUndefined(env, peerRef)) {
                    ani_long peerPtr = 0;
                    if (ANI_OK == env->Object_GetFieldByName_Long(
                        static_cast<ani_object>(peerRef), "ptr", &peerPtr) && peerPtr != 0) {
                        auto* peer = reinterpret_cast<TransitionEffectPeer*>(peerPtr);
                        if (peer && peer->handler) {
                            config.transition = new RefPtr<NG::ChainedTransitionEffect>(peer->handler);
                        }
                    }
                }
            }
        }
    }
    return config;
}
} // namespace

void ModalUIExtensionProxySend(ani_env* env, [[maybe_unused]] ani_object obj, ani_long proxyPtr, ani_object data)
{
    CHECK_NULL_VOID(env);
    auto* proxyStorage = reinterpret_cast<std::shared_ptr<ModalUIExtensionProxy>*>(proxyPtr);
    CHECK_NULL_VOID(proxyStorage && *proxyStorage);
    HILOGE("ModalUIExtensionProxySend: WantParams conversion from ANI not yet supported");
}

ani_int CreateModalUIExtension(ani_env* env, [[maybe_unused]] ani_object obj, ani_object wantObj, ani_object options, ani_object callback)
{
    CHECK_NULL_RETURN(env, 0);
    CHECK_NULL_RETURN(wantObj, 0);
    CHECK_NULL_RETURN(callback, 0);
    HILOGE("CreateModalUIExtension ANI entry called");

    AAFwk::Want want;
    if (!OHOS::AppExecFwk::UnwrapWant(env, wantObj, want)) {
        HILOGE("UnwrapWant failed when CreateModalUIExtension");
        return 0;
    }

    ModalUIExtensionConfig config;
    if (options) {
        config = ParseConfigFromAniOptions(env, options);
    }

    ModalUIExtensionCallbacks callbacks;
    ParseAniCallbacks(env, callback, callbacks);

    const auto* modifier = GetNodeAniModifier();
    CHECK_NULL_RETURN(modifier, 0);
    auto commonModifier = modifier->getCommonAniModifier();
    CHECK_NULL_RETURN(commonModifier, 0);
    return commonModifier->createModalUIExtension(want, callbacks, config);
}

void CloseModalUIExtension(ani_env* env, [[maybe_unused]] ani_object obj, ani_int sessionId)
{
    CHECK_NULL_VOID(env);
    HILOGE("CloseModalUIExtension called, sessionId=%{public}d", static_cast<int32_t>(sessionId));
    const auto* modifier = GetNodeAniModifier();
    CHECK_NULL_VOID(modifier);
    auto commonModifier = modifier->getCommonAniModifier();
    CHECK_NULL_VOID(commonModifier);
    commonModifier->closeModalUIExtension(sessionId);
}
} // namespace OHOS::Ace::Ani

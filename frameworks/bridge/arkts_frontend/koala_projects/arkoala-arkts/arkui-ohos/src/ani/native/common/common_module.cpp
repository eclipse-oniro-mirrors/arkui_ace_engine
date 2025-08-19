/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "common_module.h"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "ani.h"
#include "load.h"
#include "log/log.h"

#include "base/utils/utils.h"
#include "core/interfaces/ani/ani_api.h"
#include "arkoala/framework/native/src/resource_color_helper.h"
#include "pixel_map_taihe_ani.h"
#include "utils/ani_utils.h"

namespace OHOS::Ace::Ani {
ani_status GetAniEnv(ani_vm* vm, ani_env** env)
{
    CHECK_NULL_RETURN(vm, ANI_ERROR);
    ani_options aniOpt {0, nullptr};
    auto status = vm->AttachCurrentThread(&aniOpt, ANI_VERSION_1, env);
    if (status != ANI_OK) {
        vm->GetEnv(ANI_VERSION_1, env);
    }
    return status;
}

class CommonModuleCallbackAni {
public:
    CommonModuleCallbackAni(ani_env* env, ani_ref func)
    {
        CHECK_NULL_VOID(env);
        env->GetVM(&vm_);
        env->GlobalReference_Create(func, &func_);
    }
    ~CommonModuleCallbackAni()
    {
        CHECK_NULL_VOID(vm_);
        CHECK_NULL_VOID(func_);
        ani_env* env = nullptr;
        auto attachCurrentThreadStatus = GetAniEnv(vm_, &env);
        if (attachCurrentThreadStatus == ANI_OK && env != nullptr) {
            env->GlobalReference_Delete(func_);
            vm_->DetachCurrentThread();
        }
    }
    void Call(ani_env* env, ani_size argc, ani_ref* argv, ani_ref* result)
    {
        CHECK_NULL_VOID(env);
        env->FunctionalObject_Call(static_cast<ani_fn_object>(func_), argc, argv, result);
    }
private:
    ani_vm* vm_ = nullptr;
    ani_ref func_ = nullptr;
};
ani_object GetHostContext([[maybe_unused]] ani_env* env)
{
    const auto* modifier = GetNodeAniModifier();
    if (!modifier) {
        return nullptr;
    }
    ani_ref* context = modifier->getCommonAniModifier()->getHostContext();
    if (context) {
        ani_object context_object = reinterpret_cast<ani_object>(*context);
        return context_object;
    }
    return nullptr;
}

void SyncInstanceId([[maybe_unused]] ani_env* env, ani_object obj, ani_int id)
{
    const auto* modifier = GetNodeAniModifier();
    if (!modifier) {
        return;
    }
    modifier->getCommonAniModifier()->syncInstanceId(id);
}

void RestoreInstanceId([[maybe_unused]] ani_env* env)
{
    const auto* modifier = GetNodeAniModifier();
    if (!modifier) {
        return;
    }
    modifier->getCommonAniModifier()->restoreInstanceId();
}

ani_int GetCurrentInstanceId([[maybe_unused]] ani_env* env)
{
    const auto* modifier = GetNodeAniModifier();
    if (!modifier) {
        return -1;
    }
    return modifier->getCommonAniModifier()->getCurrentInstanceId();
}

ani_int GetFocusedInstanceId([[maybe_unused]] ani_env* env)
{
    const auto* modifier = GetNodeAniModifier();
    if (!modifier || !modifier->getCommonAniModifier() || !env) {
        return -1;
    }
    return modifier->getCommonAniModifier()->getFocusedInstanceId();
}

void SetDrawCallback(ani_env* env, ani_object obj, ani_long ptr, ani_fn_object fnObj)
{
    const auto* modifier = GetNodeAniModifier();
    if (!modifier) {
        return;
    }
    modifier->getCommonAniModifier()->setDrawCallback(env, ptr, fnObj);
}

void SetDrawModifier(
    ani_env* env, [[maybe_unused]] ani_object aniClass, ani_long ptr, ani_int flag, ani_object drawModifier)
{
    const auto* modifier = GetNodeAniModifier();
    if (!modifier) {
        return;
    }
    modifier->getArkUIAniDrawModifier()->setDrawModifier(env, ptr, flag, drawModifier);
}

void Invalidate(ani_env* env, [[maybe_unused]] ani_object aniClass, ani_long ptr)
{
    const auto* modifier = GetNodeAniModifier();
    if (!modifier) {
        return;
    }
    modifier->getArkUIAniDrawModifier()->invalidate(env, ptr);
}

ani_long BuilderProxyNodeConstruct(ani_env* env, [[maybe_unused]] ani_object aniClass, ani_int id)
{
    auto nodeId = reinterpret_cast<ArkUI_Int32>(id);
    ani_long nativeObj {};
    const auto* modifier = GetNodeAniModifier();
    CHECK_NULL_RETURN(modifier, nativeObj);
    auto builderProxyNode = modifier->getCommonAniModifier()->builderProxyNodeConstruct(nodeId);
    CHECK_NULL_RETURN(builderProxyNode, nativeObj);
    return reinterpret_cast<ani_long>(builderProxyNode);
}

void RemoveComponentFromFrameNode(ani_env* env, ani_object obj, ani_long node, ani_long content)
{
    const auto* modifier = GetNodeAniModifier();
    if (!modifier || !modifier->getArkUIAniComponentConentModifier() || !env) {
        return;
    }
    modifier->getArkUIAniComponentConentModifier()->removeComponentFromFrameNode(node, content);
}

void AddComponentToFrameNode(ani_env* env, ani_object obj, ani_long node, ani_long content)
{
    const auto* modifier = GetNodeAniModifier();
    if (!modifier || !modifier->getArkUIAniComponentConentModifier() || !env) {
        return;
    }
    modifier->getArkUIAniComponentConentModifier()->addComponentToFrameNode(node, content);
}

ani_object GetSharedLocalStorage([[maybe_unused]] ani_env* env)
{
    const auto* modifier = GetNodeAniModifier();
    if (!modifier) {
        return nullptr;
    }
    ani_ref storage = modifier->getCommonAniModifier()->getSharedLocalStorage();
    if (storage) {
        ani_object storage_object = reinterpret_cast<ani_object>(storage);
        return storage_object;
    }
    return nullptr;
}

void SetBackgroundImagePixelMap([[maybe_unused]] ani_env* env, [[maybe_unused]] ani_object aniClass, ani_object node,
    ani_object pixelMap, ani_int repeat)
{
    auto* arkNode = reinterpret_cast<ArkUINodeHandle>(node);
    auto pixelMapValue = OHOS::Media::PixelMapTaiheAni::GetNativePixelMap(env, pixelMap);
    if (!pixelMapValue) {
        return;
    }
    auto pixelMapPtr = reinterpret_cast<void*>(&pixelMapValue);
    const auto* modifier = GetNodeAniModifier();
    if (!modifier || !modifier->getCommonAniModifier() || !env) {
        return;
    }
    modifier->getCommonAniModifier()->setBackgroundImagePixelMap(
        env, arkNode, reinterpret_cast<ani_ref>(pixelMapPtr), repeat);
}

void SetCustomCallback(ani_env* env, ani_object obj, ani_long ptr,
    ani_fn_object fnObjMeasure, ani_fn_object fnObjLayout)
{
    const auto* modifier = GetNodeAniModifier();
    if (!modifier || !modifier->getCommonAniModifier() || !env) {
        return;
    }
    modifier->getCommonAniModifier()->setCustomCallback(env, ptr, fnObjMeasure, fnObjLayout);
}

ani_int RequireArkoalaNodeId(ani_env* env, ani_object obj, ani_int capacity)
{
    auto idCapacity = reinterpret_cast<ArkUI_Int32>(capacity);
    const auto* modifier = GetNodeAniModifier();
    CHECK_NULL_RETURN(modifier, -1);
    auto cursor = modifier->getCommonAniModifier()->requireArkoalaNodeId(idCapacity);
    return cursor;
}

ani_long GetNodePtrWithPeerPtr(ani_env* env, ani_object obj, ani_long ptr)
{
    const auto* modifier = GetNodeAniModifier();
    CHECK_NULL_RETURN(modifier, -1);
    auto ret = modifier->getCommonAniModifier()->getNodePtrWithPeerPtr(ptr);
    return ret;
}
ani_int GetNodeIdWithNodePtr(ani_env* env, ani_object obj, ani_long ptr)
{
    const auto* modifier = GetNodeAniModifier();
    CHECK_NULL_RETURN(modifier, -1);
    auto ret = modifier->getCommonAniModifier()->getNodeIdWithNodePtr(ptr);
    return ret;
}

ani_int GetNodeIdWithPeerPtr(ani_env* env, ani_object obj, ani_long ptr)
{
    const auto* modifier = GetNodeAniModifier();
    CHECK_NULL_RETURN(modifier, -1);
    auto ret = modifier->getCommonAniModifier()->getNodeIdWithPeerPtr(ptr);
    return ret;
}

ani_long CreateRenderNodePeerWithNodePtr(ani_env* env, ani_object obj, ani_long ptr)
{
    const auto* modifier = GetNodeAniModifier();
    CHECK_NULL_RETURN(modifier, -1);
    auto peerPtr = modifier->getCommonAniModifier()->createRenderNodePeerWithNodePtr(ptr);
    return peerPtr;
}

ani_long ToColorLong(ani_env* env, ani_object obj, ani_int color)
{
    unsigned int uClr = static_cast<unsigned int>(color);
    int64_t clr = static_cast<int64_t>(uClr);
    return clr;
}
ani_int ToColorInt(ani_env* env, ani_object obj, ani_long color)
{
    unsigned int uClr = static_cast<unsigned int>(color);
    int32_t clr = static_cast<int32_t>(uClr);
    return clr;
}

ani_int CheckIsUIThread([[maybe_unused]] ani_env* env, ani_object obj, ani_int id)
{
    const auto* modifier = GetNodeAniModifier();
    if (!modifier) {
        return false;
    }
    return modifier->getCommonAniModifier()->checkIsUIThread(id);
}

ani_int IsDebugMode([[maybe_unused]] ani_env* env, ani_object obj, ani_int id)
{
    const auto* modifier = GetNodeAniModifier();
    if (!modifier) {
        return false;
    }
    return modifier->getCommonAniModifier()->isDebugMode(id);
}
void OnMeasureInnerMeasure(ani_env* env, ani_object obj, ani_long ptr)
{
    const auto* modifier = GetNodeAniModifier();
    if (!modifier || !modifier->getCommonAniModifier() || !env) {
        return;
    }
    modifier->getCommonAniModifier()->onMeasureInnerMeasure(env, ptr);
}

void OnLayoutInnerLayout(ani_env* env, ani_object obj, ani_long ptr)
{
    const auto* modifier = GetNodeAniModifier();
    if (!modifier || !modifier->getCommonAniModifier() || !env) {
        return;
    }
    modifier->getCommonAniModifier()->onLayoutInnerLayout(env, ptr);
}

void SetParallelScoped(ani_env* env, ani_object obj, ani_boolean parallel)
{
    const auto* modifier = GetNodeAniModifier();
    if (!modifier || !modifier->getCommonAniModifier() || !env) {
        return;
    }
    modifier->getCommonAniModifier()->setParallelScoped(parallel);
}

void ConvertRemoveCallbackFun(
    ani_vm* vm, std::function<void()>& callback, const std::shared_ptr<CommonModuleCallbackAni>& callbackAni)
{
    callback = [vm, callbackAni]() {
        CHECK_NULL_VOID(vm);
        CHECK_NULL_VOID(callbackAni);
        ani_env* env = nullptr;
        auto attachCurrentThreadStatus = GetAniEnv(vm, &env);
        CHECK_NULL_VOID(env);

        std::vector<ani_ref> args = {};
        ani_ref ret = nullptr;
        callbackAni->Call(env, args.size(), args.data(), &ret);
        if (attachCurrentThreadStatus == ANI_OK) {
            vm->DetachCurrentThread();
        }
    };
}

void ConvertGetCallbackFun(ani_vm* vm, std::function<std::string(const std::string&)>& callback,
    const std::shared_ptr<CommonModuleCallbackAni>& callbackAni)
{
    callback = [vm, callbackAni](const std::string& key) -> std::string {
        std::string value;
        CHECK_NULL_RETURN(vm, value);
        CHECK_NULL_RETURN(callbackAni, value);
        ani_env* env = nullptr;
        auto attachCurrentThreadStatus = GetAniEnv(vm, &env);
        CHECK_NULL_RETURN(env, value);
        auto keyAni = AniUtils::StdStringToANIString(env, key);
        std::vector<ani_ref> args = { *keyAni };
        ani_ref ret = nullptr;
        callbackAni->Call(env, args.size(), args.data(), &ret);
        if (attachCurrentThreadStatus == ANI_OK) {
            vm->DetachCurrentThread();
            ani_string resultRef = static_cast<ani_string>(ret);
            value = AniUtils::ANIStringToStdString(env, resultRef);
            return value;
        } else {
            return value;
        }
    };
}

void SetCustomPropertyCallBack(
    ani_env* env, ani_object aniClass, ani_long node, ani_fn_object removeCallback, ani_fn_object getCallback)
{
    const auto* modifier = GetNodeAniModifier();
    if (!modifier || !modifier->getCommonAniModifier() || !env) {
        return;
    }
    ArkUINodeHandle frameNode = reinterpret_cast<ArkUINodeHandle>(node);
    CHECK_NULL_VOID(frameNode);

    ani_ref removeCallbackAniRef = static_cast<ani_ref>(removeCallback);
    ani_ref getCallbackAniRef = static_cast<ani_ref>(getCallback);
    auto removeCallbackAni = std::make_shared<CommonModuleCallbackAni>(env, removeCallbackAniRef);
    auto getCallbackAni = std::make_shared<CommonModuleCallbackAni>(env, getCallbackAniRef);
    ani_vm* vm = nullptr;
    env->GetVM(&vm);
    std::function<void()> removeCallbackFun = nullptr;
    std::function<std::string(const std::string&)> getCallbackFun = nullptr;
    ConvertRemoveCallbackFun(vm, removeCallbackFun, removeCallbackAni);
    ConvertGetCallbackFun(vm, getCallbackFun, getCallbackAni);

    modifier->getCommonAniModifier()->setCustomPropertyCallBack(
        env, frameNode, std::move(removeCallbackFun), std::move(getCallbackFun));
}

ani_string GetCustomProperty(
    ani_env* env, ani_object aniClass, ani_long node, ani_string aniKey)
{
    const auto* modifier = GetNodeAniModifier();
    if (!modifier || !modifier->getCommonAniModifier() || !env) {
        return nullptr;
    }
    ArkUINodeHandle frameNode = reinterpret_cast<ArkUINodeHandle>(node);
    CHECK_NULL_RETURN(frameNode, nullptr);
    auto strKey = AniUtils::ANIStringToStdString(env, aniKey);
    auto ret = modifier->getCommonAniModifier()->getCustomProperty(env, frameNode, strKey);
    if (ret.has_value()) {
        auto retValue = AniUtils::StdStringToANIString(env, ret.value());
        if (retValue) {
            return *retValue;
        }
    }
    return nullptr;
}

void GetAlignmentEnum(ani_env* env, ani_object align, AniOverlayOptions& opt)
{
    ani_enum enumType;
    if (ANI_OK != env->FindEnum("Larkui/component/enums/Alignment;", &enumType)) {
        return;
    }

    ani_boolean isAlignment = ANI_FALSE;
    if (ANI_OK != env->Object_InstanceOf(align, enumType, &isAlignment)) {
        return;
    }

    if (!isAlignment) {
        return;
    }

    ani_enum_item enumItem = static_cast<ani_enum_item>(align);
    ani_int value;
    if (ANI_OK != env->EnumItem_GetValue_Int(enumItem, &value)) {
        return;
    }
    opt.alignment = static_cast<int32_t>(value);
}

void ParseOverlayOptions(ani_env* env, ani_object options, AniOverlayOptions& opt)
{
    ani_boolean isUndefined;
    env->Reference_IsUndefined(options, &isUndefined);
    if (isUndefined) {
        return;
    }
    ani_ref align;
    env->Object_GetPropertyByName_Ref(options, "align", &align);
    env->Reference_IsUndefined(align, &isUndefined);
    if (!isUndefined) {
        GetAlignmentEnum(env, static_cast<ani_object>(align), opt);
    }
    ani_ref overlayOffset;
    env->Object_GetPropertyByName_Ref(options, "offset", &overlayOffset);
    env->Reference_IsUndefined(overlayOffset, &isUndefined);
    if (!isUndefined) {
        ani_object offset = static_cast<ani_object>(overlayOffset);
        ani_ref x;
        if (ANI_OK == env->Object_GetPropertyByName_Ref(offset, "x", &x)) {
            ani_double param_value;
            env->Object_CallMethodByName_Double(static_cast<ani_object>(x), "unboxed", ":D", &param_value);
            opt.x = static_cast<float>(param_value);
        }
        ani_ref y;
        if (ANI_OK == env->Object_GetPropertyByName_Ref(offset, "y", &y)) {
            ani_double param_value;
            env->Object_CallMethodByName_Double(static_cast<ani_object>(y), "unboxed", ":D", &param_value);
            opt.y = static_cast<float>(param_value);
        }
    }
}

void SetOverlayComponentContent(ani_env* env, ani_object obj, ani_long ptr, ani_long buildNodePtr, ani_object options)
{
    const auto* modifier = GetNodeAniModifier();
    if (!env || !modifier) {
        return;
    }
    AniOverlayOptions opt;
    ParseOverlayOptions(env, options, opt);
    modifier->getCommonAniModifier()->setOverlayComponent(ptr, buildNodePtr, opt);
}
ani_double Vp2px(ani_env* env, ani_object obj, ani_double value, ani_int instanceId)
{
    const auto* modifier = GetNodeAniModifier();
    if (!modifier || !modifier->getCommonAniModifier() || !env) {
        return 0;
    }
    return modifier->getCommonAniModifier()->vp2px(value, instanceId);
}

ani_double Px2vp(ani_env* env, ani_object obj, ani_double value, ani_int instanceId)
{
    const auto* modifier = GetNodeAniModifier();
    if (!modifier || !modifier->getCommonAniModifier() || !env) {
        return 0;
    }
    return modifier->getCommonAniModifier()->px2vp(value, instanceId);
}

ani_double Fp2px(ani_env* env, ani_object obj, ani_double value, ani_int instanceId)
{
    const auto* modifier = GetNodeAniModifier();
    if (!modifier || !modifier->getCommonAniModifier() || !env) {
        return 0;
    }
    return modifier->getCommonAniModifier()->fp2px(value, instanceId);
}

ani_double Px2fp(ani_env* env, ani_object obj, ani_double value, ani_int instanceId)
{
    const auto* modifier = GetNodeAniModifier();
    if (!modifier || !modifier->getCommonAniModifier() || !env) {
        return 0;
    }
    return modifier->getCommonAniModifier()->px2fp(value, instanceId);
}

ani_double Lpx2px(ani_env* env, ani_object obj, ani_double value, ani_int instanceId)
{
    const auto* modifier = GetNodeAniModifier();
    if (!modifier || !modifier->getCommonAniModifier() || !env) {
        return 0;
    }
    return modifier->getCommonAniModifier()->lpx2px(value, instanceId);
}

ani_double Px2lpx(ani_env* env, ani_object obj, ani_double value, ani_int instanceId)
{
    const auto* modifier = GetNodeAniModifier();
    if (!modifier || !modifier->getCommonAniModifier() || !env) {
        return 0;
    }
    return modifier->getCommonAniModifier()->px2lpx(value, instanceId);
}
void* TransferKeyEventPointer(ani_env* env, ani_object obj, ani_long pointer)
{
    const auto* modifier = GetNodeAniModifier();
    if (!modifier || !modifier->getCommonAniModifier() || !env) {
        return nullptr;
    }
    return modifier->getCommonAniModifier()->transferKeyEventPointer(pointer);
}
void* CreateKeyEventAccessorWithPointer(ani_env* env, [[maybe_unused]] ani_object obj, ani_long pointer)
{
    const auto* modifier = GetNodeAniModifier();
    if (!modifier || !modifier->getCommonAniModifier() || !env) {
        return nullptr;
    }
    return modifier->getCommonAniModifier()->createKeyEventAccessorWithPointer(pointer);
}
void* CreateEventTargetInfoAccessor(ani_env* env, [[maybe_unused]] ani_object obj)
{
    const auto* modifier = GetNodeAniModifier();
    if (!modifier || !modifier->getCommonAniModifier() || !env) {
        return nullptr;
    }
    return modifier->getCommonAniModifier()->createEventTargetInfoAccessor();
}
void EventTargetInfoAccessorWithId(ani_env* env, [[maybe_unused]] ani_object obj, ani_long input, ani_string id)
{
    const auto* modifier = GetNodeAniModifier();
    if (!modifier || !modifier->getCommonAniModifier() || !env) {
        return;
    }
    return modifier->getCommonAniModifier()->eventTargetInfoAccessorWithId(env, input, id);
}
void* CreateScrollableTargetInfoAccessor(ani_env* env, [[maybe_unused]] ani_object obj)
{
    const auto* modifier = GetNodeAniModifier();
    if (!modifier || !modifier->getCommonAniModifier() || !env) {
        return nullptr;
    }
    return modifier->getCommonAniModifier()->createScrollableTargetInfoAccessor();
}
void ScrollableTargetInfoAccessorWithId(ani_env* env, [[maybe_unused]] ani_object obj, ani_long input, ani_string id)
{
    const auto* modifier = GetNodeAniModifier();
    if (!modifier || !modifier->getCommonAniModifier() || !env) {
        return;
    }
    return modifier->getCommonAniModifier()->scrollableTargetInfoAccessorWithId(env, input, id);
}
void ScrollableTargetInfoAccessorWithPointer(
    ani_env* env, [[maybe_unused]] ani_object obj, ani_long input, ani_long pointer)
{
    const auto* modifier = GetNodeAniModifier();
    if (!modifier || !modifier->getCommonAniModifier() || !env) {
        return;
    }
    return modifier->getCommonAniModifier()->scrollableTargetInfoAccessorWithPointer(input, pointer);
}
void* TransferScrollableTargetInfoPointer(ani_env* env, ani_object obj, ani_long pointer)
{
    const auto* modifier = GetNodeAniModifier();
    if (!modifier || !modifier->getCommonAniModifier() || !env) {
        return nullptr;
    }
    return modifier->getCommonAniModifier()->transferScrollableTargetInfoPointer(pointer);
}
ani_long CreateDragEventAccessorWithPointer(ani_env* env, ani_object obj, ani_long pointer)
{
    const auto* modifier = GetNodeAniModifier();
    if (!modifier || !modifier->getCommonAniModifier() || !env) {
        return 0;
    }
    return modifier->getCommonAniModifier()->transferDragEventPointer(pointer);
}

ani_long GetDragEventPointer(ani_env* env, ani_object obj, ani_long pointer)
{
    const auto* modifier = GetNodeAniModifier();
    if (!modifier || !modifier->getCommonAniModifier() || !env) {
        return 0;
    }
    return modifier->getCommonAniModifier()->getDragEventPointer(pointer);
}
void* CreateTouchEventAccessorWithPointer(ani_env* env, [[maybe_unused]] ani_object obj, ani_long pointer)
{
    const auto* modifier = GetNodeAniModifier();
    if (!modifier || !modifier->getCommonAniModifier() || !env) {
        return nullptr;
    }
    return modifier->getCommonAniModifier()->transferTouchEventPointer(pointer);
}

void* CreateMouseEventAccessorWithPointer(ani_env* env, [[maybe_unused]] ani_object obj, ani_long pointer)
{
    const auto* modifier = GetNodeAniModifier();
    if (!modifier || !modifier->getCommonAniModifier() || !env) {
        return nullptr;
    }
    return modifier->getCommonAniModifier()->transferMouseEventPointer(pointer);
}

void* CreateAxisEventAccessorWithPointer(ani_env* env, [[maybe_unused]] ani_object obj, ani_long pointer)
{
    const auto* modifier = GetNodeAniModifier();
    if (!modifier || !modifier->getCommonAniModifier() || !env) {
        return nullptr;
    }
    return modifier->getCommonAniModifier()->transferAxisEventPointer(pointer);
}

void* CreateClickEventAccessorWithPointer(ani_env* env, [[maybe_unused]] ani_object obj, ani_long pointer)
{
    const auto* modifier = GetNodeAniModifier();
    if (!modifier || !modifier->getCommonAniModifier() || !env) {
        return nullptr;
    }
    return modifier->getCommonAniModifier()->transferClickEventPointer(pointer);
}

void* CreateHoverEventAccessorWithPointer(ani_env* env, [[maybe_unused]] ani_object obj, ani_long pointer)
{
    const auto* modifier = GetNodeAniModifier();
    if (!modifier || !modifier->getCommonAniModifier() || !env) {
        return nullptr;
    }
    return modifier->getCommonAniModifier()->transferHoverEventPointer(pointer);
}

void* GetTouchEventPointer(ani_env* env, [[maybe_unused]] ani_object obj, ani_long touchEventPeer)
{
    const auto* modifier = GetNodeAniModifier();
    if (!modifier || !modifier->getCommonAniModifier() || !env) {
        return nullptr;
    }
    return modifier->getCommonAniModifier()->getTouchEventPointer(touchEventPeer);
}

void* GetMouseEventPointer(ani_env* env, [[maybe_unused]] ani_object obj, ani_long mouseEventPeer)
{
    const auto* modifier = GetNodeAniModifier();
    if (!modifier || !modifier->getCommonAniModifier() || !env) {
        return nullptr;
    }
    return modifier->getCommonAniModifier()->getMouseEventPointer(mouseEventPeer);
}

void* GetAxisEventPointer(ani_env* env, [[maybe_unused]] ani_object obj, ani_long axisEventPeer)
{
    const auto* modifier = GetNodeAniModifier();
    if (!modifier || !modifier->getCommonAniModifier() || !env) {
        return nullptr;
    }
    return modifier->getCommonAniModifier()->getAxisEventPointer(axisEventPeer);
}

void* GetClickEventPointer(ani_env* env, [[maybe_unused]] ani_object obj, ani_long clickEventPeer)
{
    const auto* modifier = GetNodeAniModifier();
    if (!modifier || !modifier->getCommonAniModifier() || !env) {
        return nullptr;
    }
    return modifier->getCommonAniModifier()->getMouseEventPointer(clickEventPeer);
}

void* GetHoverEventPointer(ani_env* env, [[maybe_unused]] ani_object obj, ani_long hoverEventPeer)
{
    const auto* modifier = GetNodeAniModifier();
    if (!modifier || !modifier->getCommonAniModifier() || !env) {
        return nullptr;
    }
    return modifier->getCommonAniModifier()->getMouseEventPointer(hoverEventPeer);
}

ani_int GetStringColorValue(ani_env* env, ani_object aniClass, ani_string src)
{
    const auto* modifier = GetNodeAniModifier();
    if (!modifier) {
        return 0;
    }
    auto srcString = AniUtils::ANIStringToStdString(env, static_cast<ani_string>(src));
    return modifier->getCommonAniModifier()->getColorValueByString(srcString);
}

ani_int GetNumberColorValue(ani_env* env, ani_object aniClass, ani_double src)
{
    const auto* modifier = GetNodeAniModifier();
    if (!modifier) {
        return 0;
    }
    return modifier->getCommonAniModifier()->getColorValueByNumber(static_cast<uint32_t>(src));
}

void SendThemeToNative(ani_env* env, ani_object aniClass, ani_long thisArray, ani_double thisLength, ani_int id)
{
    const auto* modifier = GetNodeAniModifier();
    if (!modifier) {
        return;
    }
    std::vector<Ark_ResourceColor> colors = GetResourceColorArray(thisArray, thisLength);
    modifier->getCommonAniModifier()->sendThemeToNative(env, colors, id);
}

void RemoveThemeInNative(ani_env* env, ani_object aniClass, ani_int withThemeId)
{
    const auto* modifier = GetNodeAniModifier();
    if (!modifier) {
        return;
    }
    modifier->getCommonAniModifier()->removeThemeInNative(env, withThemeId);
}

void SetDefaultTheme(ani_env* env, ani_object aniClass, ani_long thisArray, ani_double thisLength, ani_boolean isDark)
{
    const auto* modifier = GetNodeAniModifier();
    if (!modifier) {
        return;
    }
    std::vector<Ark_ResourceColor> colorArray = GetResourceColorArray(thisArray, thisLength);
    modifier->getCommonAniModifier()->setDefaultTheme(env, colorArray, isDark);
}

void UpdateColorMode(ani_env* env, ani_object aniClass, ani_int colorMode)
{
    const auto* modifier = GetNodeAniModifier();
    if (!modifier) {
        return;
    }
    modifier->getCommonAniModifier()->updateColorMode(colorMode);
}

void RestoreColorMode(ani_env* env, ani_object aniClass)
{
    const auto* modifier = GetNodeAniModifier();
    if (!modifier) {
        return;
    }
    modifier->getCommonAniModifier()->restoreColorMode();
}

void SetThemeScopeId(ani_env* env, ani_object aniClass, ani_int themeScopeId)
{
    const auto* modifier = GetNodeAniModifier();
    if (!modifier) {
        return;
    }
    modifier->getCommonAniModifier()->setThemeScopeId(env, themeScopeId);
}

void CreateAndBindTheme(ani_env* env, ani_object aniClass, ani_int themeScopeId, ani_int themeId, ani_long thisArray,
    ani_double thisLength, ani_int colorMode, ani_fn_object onThemeScopeDestroy)
{
    const auto* modifier = GetNodeAniModifier();
    if (!modifier) {
        return;
    }
    std::vector<Ark_ResourceColor> colors = GetResourceColorArray(thisArray, thisLength);
    modifier->getCommonAniModifier()->createAndBindTheme(
        env, themeScopeId, themeId, colors, colorMode, onThemeScopeDestroy);
}

void ApplyParentThemeScopeId(ani_env* env, ani_object aniClass, ani_long self, ani_long parent)
{
    const auto* modifier = GetNodeAniModifier();
    if (!modifier) {
        return;
    }
    modifier->getCommonAniModifier()->applyParentThemeScopeId(env, self, parent);
}
void FrameNodeMarkDirtyNode(ani_env* env, ani_object obj, ani_long ptr)
{
    const auto* modifier = GetNodeAniModifier();
    if (!modifier || !modifier->getCommonAniModifier() || !env) {
        return;
    }
    modifier->getCommonAniModifier()->frameNodeMarkDirtyNode(env, ptr);
}
} // namespace OHOS::Ace::Ani

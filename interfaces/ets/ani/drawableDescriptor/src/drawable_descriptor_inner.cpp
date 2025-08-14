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

#include <memory>

#include "ani.h"
#include "draw/canvas.h"
#include "image/bitmap.h"
#include "image/image_info.h"
#include "interfaces/inner_api/drawable_descriptor/drawable_descriptor.h"
#include "interop_js/arkts_esvalue.h"
#include "pixel_map_taihe_ani.h"
#include "resource_manager.h"
#include "securec.h"

#include "base/image/pixel_map.h"
#include "base/log/log.h"
#include "core/drawable/animated_drawable_descriptor.h"
#include "core/drawable/layered_drawable_descriptor.h"
#include "core/drawable/pixel_map_drawable_descriptor.h"

namespace OHOS::Ace::Ani {
namespace {
constexpr char PIXEL_MAP_CONSTRUCTOR[] = "C{@ohos.multimedia.image.image.PixelMap}:";
constexpr char PIXEL_MAP_DRAWABLE[] = "@ohos.arkui.drawableDescriptor.PixelMapDrawableDescriptor";
constexpr char ARRAY_GET[] = "i:C{std.core.Object}";
constexpr char ANIMATED_CONSTRUCTOR[] = "C{escompat.Array}C{@ohos.arkui.drawableDescriptor.AnimationOptions}:";
constexpr char ANIMATED_DRAWABLE[] = "@ohos.arkui.drawableDescriptor.AnimatedDrawableDescriptor";
constexpr char LAYERED_CONSTRUCTOR[] =
    "C{@ohos.arkui.drawableDescriptor.DrawableDescriptor}C{@ohos.arkui.drawableDescriptor.DrawableDescriptor}C{@ohos."
    "arkui.drawableDescriptor.DrawableDescriptor}:";
constexpr char LAYERED_DRAWABLE[] = "@ohos.arkui.drawableDescriptor.LayeredDrawableDescriptor";

ani_object CreatePixelMapDrawableByPixelMap(ani_env* env, const RefPtr<PixelMap>& pixelMap)
{
    auto pixelAni = Media::PixelMapTaiheAni::CreateEtsPixelMap(env, pixelMap->GetPixelMapSharedPtr());
    if (pixelAni == nullptr) {
        return nullptr;
    }
    // set foreground property to arkts object
    ani_class cls;
    auto status = env->FindClass(PIXEL_MAP_DRAWABLE, &cls);
    if (status != ANI_OK) {
        return nullptr;
    }
    ani_object obj {};
    ani_method ctor {};
    env->Class_FindMethod(cls, "<ctor>", PIXEL_MAP_CONSTRUCTOR, &ctor);
    env->Object_New(cls, ctor, &obj, pixelAni);
    return obj;
}

ani_ref CreateDouble(ani_env* env, ani_int value)
{
    ani_class cls;
    env->FindClass("std.core.Double", &cls);
    ani_method ctor;
    env->Class_FindMethod(cls, "<ctor>", "d:", &ctor);
    ani_object rs;
    env->Object_New(cls, ctor, &rs, value);
    return rs;
}

ani_object CreateAnimatedDrawableByPixelMapList(ani_env* env,
    const std::vector<std::shared_ptr<Media::PixelMap>>& pixelMapList, int32_t duration, int32_t iterations)
{
    ani_object optionsAni {};
    static const char* className = "@ohos.arkui.drawableDescriptor.AnimationOptionsImpl";

    if (pixelMapList.empty()) {
        return nullptr;
    }
    ani_class cls;
    env->FindClass(className, &cls);
    ani_method ctor;
    env->Class_FindMethod(cls, "<ctor>", nullptr, &ctor);

    auto durationAni = CreateDouble(env, duration);
    auto iterationsAni = CreateDouble(env, iterations);

    env->Object_New(cls, ctor, &optionsAni);

    env->Object_SetPropertyByName_Ref(optionsAni, "duration", durationAni);
    env->Object_SetPropertyByName_Ref(optionsAni, "iterations", iterationsAni);

    ani_array pixelMapsAni;
    auto refPixelMap = PixelMap::Create(pixelMapList[0]);

    auto init = Media::PixelMapTaiheAni::CreateEtsPixelMap(env, refPixelMap->GetPixelMapSharedPtr());
    env->Array_New(pixelMapList.size(), init, &pixelMapsAni);

    for (size_t index = 1; index < pixelMapList.size(); index++) {
        auto refPixel = PixelMap::Create(pixelMapList[index]);
        auto pixelAni = Media::PixelMapTaiheAni::CreateEtsPixelMap(env, refPixel->GetPixelMapSharedPtr());
        env->Array_Set(pixelMapsAni, index, pixelAni);
    }

    env->FindClass(ANIMATED_DRAWABLE, &cls);

    ani_object obj {};
    env->Class_FindMethod(cls, "<ctor>", ANIMATED_CONSTRUCTOR, &ctor);
    env->Object_New(cls, ctor, &obj, pixelMapsAni, optionsAni);

    return obj;
}

ani_object CreateLayeredDrawableByPixelMap(
    ani_env* env, const RefPtr<PixelMap>& foreground, const RefPtr<PixelMap>& background, const RefPtr<PixelMap>& mask)
{
    ani_class cls {};
    auto status = env->FindClass(LAYERED_DRAWABLE, &cls);
    if (status != ANI_OK) {
        return nullptr;
    }
    ani_object obj {};
    ani_method ctor {};
    if (ANI_OK != env->Class_FindMethod(cls, "<ctor>", LAYERED_CONSTRUCTOR, &ctor)) {
        return nullptr;
    }

    ani_object objForeground {};
    ani_object objBackground {};
    ani_object objMask {};
    if (foreground) {
        objForeground = CreatePixelMapDrawableByPixelMap(env, foreground);
    }

    if (background) {
        objBackground = CreatePixelMapDrawableByPixelMap(env, background);
    }

    if (mask) {
        objMask = CreatePixelMapDrawableByPixelMap(env, mask);
    }

    status = env->Object_New(cls, ctor, &obj, objForeground, objBackground, objMask);
    if (ANI_OK != status) {
        LOGE("CreateLayeredDrawableByPixelMap [%{public}d]", status);
        return nullptr;
    }
    return obj;
}
} // namespace

void CreatePixelMapDrawable(
    ani_env* env, [[maybe_unused]] ani_class aniClass, ani_object drawableAni, ani_object pixelAni)
{
    ani_boolean isUndefined;
    env->Reference_IsUndefined(pixelAni, &isUndefined);
    // first create native pixel map drawable descriptor
    auto* drawable = new PixelMapDrawableDescriptor();
    auto ptr = reinterpret_cast<ani_long>(drawable);
    env->Object_SetPropertyByName_Long(drawableAni, "nativeObj", ptr);
    if (!isUndefined) {
        auto pixelMap = Media::PixelMapTaiheAni::GetNativePixelMap(env, pixelAni);
        drawable->SetPixelMap(PixelMap::Create(pixelMap));
    }
}

void CreateLayeredDrawable(ani_env* env, [[maybe_unused]] ani_class aniClass, ani_object drawableAni,
    ani_object foregroundAni, ani_object backgroundAni, ani_object maskAni)
{
    ani_boolean isForegroundUndefined;
    ani_boolean isBackgroundUndefined;
    ani_boolean isMaskUndefined;
    env->Reference_IsUndefined(foregroundAni, &isForegroundUndefined);
    env->Reference_IsUndefined(backgroundAni, &isBackgroundUndefined);
    env->Reference_IsUndefined(maskAni, &isMaskUndefined);
    auto* drawable = new LayeredDrawableDescriptor();
    auto ptr = reinterpret_cast<ani_long>(drawable);
    env->Object_SetPropertyByName_Long(drawableAni, "nativeObj", ptr);
    if (!isForegroundUndefined) {
        auto foreground = Media::PixelMapTaiheAni::GetNativePixelMap(env, foregroundAni);
        drawable->SetForeground(PixelMap::Create(foreground));
    }
    if (!isBackgroundUndefined) {
        auto background = Media::PixelMapTaiheAni::GetNativePixelMap(env, backgroundAni);
        drawable->SetBackground(PixelMap::Create(background));
    }
    if (!isMaskUndefined) {
        auto mask = Media::PixelMapTaiheAni::GetNativePixelMap(env, maskAni);
        drawable->SetMask(PixelMap::Create(mask));
    }
}

void CreateAnimatedDrawable(ani_env* env, [[maybe_unused]] ani_class aniClass, ani_object drawableAni,
    ani_array pixelmapsAni, ani_object optionsAni)
{
    ani_boolean isOptionsUndefined;
    env->Reference_IsUndefined(optionsAni, &isOptionsUndefined);
    auto* drawable = new AnimatedDrawableDescriptor();
    auto ptr = reinterpret_cast<ani_long>(drawable);
    env->Object_SetPropertyByName_Long(drawableAni, "nativeObj", ptr);
    ani_size size;
    env->Array_GetLength(pixelmapsAni, &size);
    std::vector<RefPtr<PixelMap>> results;
    ani_class arrayClass;
    env->FindClass("escompat.Array", &arrayClass);
    ani_method getDataMethod;
    env->Class_FindMethod(arrayClass, "$_get", ARRAY_GET, &getDataMethod);
    for (size_t index = 0; index < size; index++) {
        ani_ref pixelmapAni;
        env->Object_CallMethod_Ref(pixelmapsAni, getDataMethod, &pixelmapAni, index);
        auto pixelmap = Media::PixelMapTaiheAni::GetNativePixelMap(env, static_cast<ani_object>(pixelmapAni));
        results.push_back(PixelMap::Create(pixelmap));
    }
    drawable->SetPixelMapList(results);
    if (isOptionsUndefined) {
        return;
    }
    ani_boolean isDurationUndefined;
    ani_boolean isIterationsUndefined;
    ani_ref durationRef;
    ani_ref iterationsRef;
    env->Object_GetPropertyByName_Ref(optionsAni, "duration", &durationRef);
    env->Object_GetPropertyByName_Ref(optionsAni, "iterations", &iterationsRef);
    ani_object durationAni = static_cast<ani_object>(durationRef);
    ani_object iterationsAni = static_cast<ani_object>(iterationsRef);
    env->Reference_IsUndefined(durationAni, &isDurationUndefined);
    env->Reference_IsUndefined(iterationsAni, &isIterationsUndefined);
    if (!isDurationUndefined) {
        ani_double duration;
        env->Object_CallMethodByName_Double(durationAni, "unboxed", ":d", &duration);
        drawable->SetTotalDuration(static_cast<int32_t>(duration));
    }
    if (!isIterationsUndefined) {
        ani_double iterations;
        env->Object_CallMethodByName_Double(iterationsAni, "unboxed", ":d", &iterations);
        drawable->SetIterations(static_cast<int32_t>(iterations));
    }
}

ani_object CreatePixelMap(ani_env* env, [[maybe_unused]] ani_class aniClass, ani_object drawableAni)
{
    ani_long nativeObj = 0;
    env->Object_GetPropertyByName_Long(drawableAni, "nativeObj", &nativeObj);
    auto* pixelMapDrawable = reinterpret_cast<PixelMapDrawableDescriptor*>(nativeObj);
    if (pixelMapDrawable == nullptr) {
        return nullptr;
    }
    auto pixelMap = pixelMapDrawable->GetPixelMap();
    if (pixelMap == nullptr) {
        return nullptr;
    }
    // set pixel map property to arkts object
    auto pixelAni = Media::PixelMapTaiheAni::CreateEtsPixelMap(env, pixelMap->GetPixelMapSharedPtr());
    return pixelAni;
}

ani_object ComposePixelMap(ani_env* env, [[maybe_unused]] ani_class aniClass, ani_object drawableAni)
{
    ani_long nativeObj = 0;
    env->Object_GetPropertyByName_Long(drawableAni, "nativeObj", &nativeObj);
    auto* layeredDrawable = reinterpret_cast<LayeredDrawableDescriptor*>(nativeObj);
    if (layeredDrawable == nullptr) {
        return nullptr;
    }
    auto pixelMap = layeredDrawable->GetPixelMap();
    if (pixelMap == nullptr) {
        return nullptr;
    }
    // set pixel map property to arkts object
    auto pixelAni = Media::PixelMapTaiheAni::CreateEtsPixelMap(env, pixelMap->GetPixelMapSharedPtr());
    return pixelAni;
}

ani_object CreateForefround(ani_env* env, [[maybe_unused]] ani_class aniClass, ani_object drawableAni)
{
    ani_long nativeObj = 0;
    env->Object_GetPropertyByName_Long(drawableAni, "nativeObj", &nativeObj);
    auto* layeredDrawable = reinterpret_cast<LayeredDrawableDescriptor*>(nativeObj);
    if (layeredDrawable == nullptr) {
        return nullptr;
    }
    auto foreground = layeredDrawable->GetForeground();
    if (foreground == nullptr) {
        return nullptr;
    }
    auto obj = CreatePixelMapDrawableByPixelMap(env, foreground);
    return obj;
}

ani_object CreateBackground(ani_env* env, [[maybe_unused]] ani_class aniClass, ani_object drawableAni)
{
    ani_long nativeObj = 0;
    env->Object_GetPropertyByName_Long(drawableAni, "nativeObj", &nativeObj);
    auto* layeredDrawable = reinterpret_cast<LayeredDrawableDescriptor*>(nativeObj);
    if (layeredDrawable == nullptr) {
        return nullptr;
    }
    auto background = layeredDrawable->GetBackground();
    if (background == nullptr) {
        return nullptr;
    }
    auto obj = CreatePixelMapDrawableByPixelMap(env, background);
    return obj;
}

ani_object CreateMask(ani_env* env, [[maybe_unused]] ani_class aniClass, ani_object drawableAni)
{
    ani_long nativeObj = 0;
    env->Object_GetPropertyByName_Long(drawableAni, "nativeObj", &nativeObj);
    auto* layeredDrawable = reinterpret_cast<LayeredDrawableDescriptor*>(nativeObj);
    if (layeredDrawable == nullptr) {
        return nullptr;
    }
    auto mask = layeredDrawable->GetMask();
    if (mask == nullptr) {
        return nullptr;
    }
    auto obj = CreatePixelMapDrawableByPixelMap(env, mask);
    return obj;
}

ani_string DrawableMaskClipPath(ani_env* env)
{
    std::string data;
    std::shared_ptr<Global::Resource::ResourceManager> resMgr(Global::Resource::CreateResourceManager());
    if (resMgr == nullptr) {
        ani_string aniString {};
        return aniString;
    }
    resMgr->GetStringByName("ohos_drawable_descriptor_path", data);
    ani_string aniString {};
    env->String_NewUTF8(data.c_str(), data.size(), &aniString);
    return aniString;
}

ani_object NativeTransferStatic(ani_env* env, [[maybe_unused]] ani_class aniClass, ani_object input)
{
    void* unwrapResult = nullptr;
    if (!arkts_esvalue_unwrap(env, input, &unwrapResult) || unwrapResult == nullptr) {
        return nullptr;
    }
    auto napiDrawable = reinterpret_cast<Napi::DrawableDescriptor*>(unwrapResult);

    ani_object retValue = nullptr;
    switch (napiDrawable->GetDrawableType()) {
        case Napi::DrawableDescriptor::DrawableType::LAYERED: {
            auto layered = static_cast<Napi::LayeredDrawableDescriptor*>(napiDrawable);
            if (layered == nullptr) {
                return nullptr;
            }
            auto foreground = layered->GetForeground();
            auto background = layered->GetBackground();
            auto mask = layered->GetMask();
            RefPtr<PixelMap> ptrForeground =
                (foreground && foreground->GetPixelMap()) ? PixelMap::Create(foreground->GetPixelMap()) : nullptr;
            RefPtr<PixelMap> ptrBackground =
                (background && background->GetPixelMap()) ? PixelMap::Create(background->GetPixelMap()) : nullptr;
            RefPtr<PixelMap> ptrMask = (mask && mask->GetPixelMap()) ? PixelMap::Create(mask->GetPixelMap()) : nullptr;

            retValue = CreateLayeredDrawableByPixelMap(env, ptrForeground, ptrBackground, ptrMask);
            break;
        }
        case Napi::DrawableDescriptor::DrawableType::ANIMATED: {
            auto animated = static_cast<Napi::AnimatedDrawableDescriptor*>(napiDrawable);
            if (animated == nullptr) {
                return nullptr;
            }
            retValue = CreateAnimatedDrawableByPixelMapList(
                env, animated->GetPixelMapList(), animated->GetDuration(), animated->GetIterations());
            break;
        }
        case Napi::DrawableDescriptor::DrawableType::PIXELMAP: {
            auto pixelMap = static_cast<Napi::DrawableDescriptor*>(napiDrawable);
            if (pixelMap == nullptr || pixelMap->GetPixelMap() == nullptr) {
                return nullptr;
            }
            retValue = CreatePixelMapDrawableByPixelMap(env, PixelMap::Create(pixelMap->GetPixelMap()));
            break;
        }
        default:
            break;
    }
    return retValue;
}
} // namespace OHOS::Ace::Ani

ANI_EXPORT ani_status ANI_Constructor(ani_vm* vm, uint32_t* result)
{
    ani_env* env;
    if (ANI_OK != vm->GetEnv(ANI_VERSION_1, &env)) {
        return ANI_ERROR;
    }
    ani_class cls;
    auto ani_status = env->FindClass("@ohos.arkui.drawableDescriptor.DrawableDescriptorInner", &cls);
    if (ani_status != ANI_OK) {
        LOGI("find arkui drawable descriptor inner failed");
        return ANI_ERROR;
    }
    std::array methods = {
        ani_native_function {
            "createPixelMapDrawable", nullptr, reinterpret_cast<void*>(OHOS::Ace::Ani::CreatePixelMapDrawable) },
        ani_native_function {
            "createLayeredDrawable", nullptr, reinterpret_cast<void*>(OHOS::Ace::Ani::CreateLayeredDrawable) },
        ani_native_function {
            "createAnimatedDrawable", nullptr, reinterpret_cast<void*>(OHOS::Ace::Ani::CreateAnimatedDrawable) },
        ani_native_function { "createPixelMap", nullptr, reinterpret_cast<void*>(OHOS::Ace::Ani::CreatePixelMap) },
        ani_native_function { "composePixelMap", nullptr, reinterpret_cast<void*>(OHOS::Ace::Ani::ComposePixelMap) },
        ani_native_function { "createForeground", nullptr, reinterpret_cast<void*>(OHOS::Ace::Ani::CreateForefround) },
        ani_native_function { "createBackground", nullptr, reinterpret_cast<void*>(OHOS::Ace::Ani::CreateBackground) },
        ani_native_function { "createMask", nullptr, reinterpret_cast<void*>(OHOS::Ace::Ani::CreateMask) },
        ani_native_function {
            "getMaskClipPath", nullptr, reinterpret_cast<void*>(OHOS::Ace::Ani::DrawableMaskClipPath) },
        ani_native_function {
            "nativeTransferStatic", nullptr, reinterpret_cast<void*>(OHOS::Ace::Ani::NativeTransferStatic) },
    };
    auto bindRst = env->Class_BindNativeMethods(cls, methods.data(), methods.size());
    if (bindRst != ANI_OK) {
        LOGI("bind native methods failed, id = %{public}d", static_cast<int>(bindRst));
        return bindRst;
    }
    *result = ANI_VERSION_1;
    return ANI_OK;
}

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

#include "canvas_module.h"

#include "load.h"
#include "pixel_map_taihe_ani.h"
#include "utils/ani_utils.h"

#include "base/log/log_wrapper.h"
#include "base/utils/utils.h"
#include "core/interfaces/native/ani/canvas_ani_modifier.h"
#include "core/interfaces/native/implementation/pixel_map_peer.h"

namespace OHOS::Ace::Ani {
void CanvasModule::SetPixelMap(
    ani_env* env, [[maybe_unused]] ani_object aniClass, ani_long peerPtr, ani_object pixelMapObj)
{
    auto* peer = reinterpret_cast<ArkUICanvasRenderer>(peerPtr);
    CHECK_NULL_VOID(peer);
    auto mediaPixelMap = Media::PixelMapTaiheAni::GetNativePixelMap(env, pixelMapObj);
    CHECK_NULL_VOID(mediaPixelMap);
    const auto* modifier = GetNodeAniModifier();
    CHECK_NULL_VOID(modifier);
    auto* canvasModifier = modifier->getCanvasAniModifier();
    CHECK_NULL_VOID(canvasModifier);
    canvasModifier->setPixelMap(peer, reinterpret_cast<void*>(&mediaPixelMap));
}

ani_object CanvasModule::GetPixelMap(ani_env* env, [[maybe_unused]] ani_object aniClass, ani_long peerPtr,
    ani_double sx, ani_double sy, ani_double sw, ani_double sh)
{
    auto* peer = reinterpret_cast<ArkUICanvasRenderer>(peerPtr);
    CHECK_NULL_RETURN(peer, nullptr);
    const auto* modifier = GetNodeAniModifier();
    CHECK_NULL_RETURN(modifier, nullptr);
    auto* canvasModifier = modifier->getCanvasAniModifier();
    CHECK_NULL_RETURN(canvasModifier, nullptr);
    auto* pixelMapPeer = reinterpret_cast<PixelMapPeer*>(canvasModifier->getPixelMap(peer, sx, sy, sw, sh));
    CHECK_NULL_RETURN(pixelMapPeer, nullptr);
    auto pixelMap = pixelMapPeer->pixelMap;
    CHECK_NULL_RETURN(pixelMap, nullptr);
    auto mediaPixelMap = pixelMap->GetPixelMapSharedPtr();
    CHECK_NULL_RETURN(mediaPixelMap, nullptr);
    return Media::PixelMapTaiheAni::CreateEtsPixelMap(env, mediaPixelMap);
}

void CanvasModule::DrawPixelMap0(ani_env* env, [[maybe_unused]] ani_object aniClass, ani_long peerPtr,
    ani_object pixelMapObj, ani_double dx, ani_double dy)
{
    auto* peer = reinterpret_cast<ArkUICanvasRenderer>(peerPtr);
    CHECK_NULL_VOID(peer);
    auto mediaPixelMap = Media::PixelMapTaiheAni::GetNativePixelMap(env, pixelMapObj);
    CHECK_NULL_VOID(mediaPixelMap);
    const auto* modifier = GetNodeAniModifier();
    CHECK_NULL_VOID(modifier);
    auto* canvasModifier = modifier->getCanvasAniModifier();
    CHECK_NULL_VOID(canvasModifier);
    canvasModifier->drawPixelMap0(peer, reinterpret_cast<void*>(&mediaPixelMap), dx, dy);
}

void CanvasModule::DrawPixelMap1(ani_env* env, [[maybe_unused]] ani_object aniClass, ani_long peerPtr,
    ani_object pixelMapObj, ani_double dx, ani_double dy, ani_double dw, ani_double dh)
{
    auto* peer = reinterpret_cast<ArkUICanvasRenderer>(peerPtr);
    CHECK_NULL_VOID(peer);
    auto mediaPixelMap = Media::PixelMapTaiheAni::GetNativePixelMap(env, pixelMapObj);
    CHECK_NULL_VOID(mediaPixelMap);
    const auto* modifier = GetNodeAniModifier();
    CHECK_NULL_VOID(modifier);
    auto* canvasModifier = modifier->getCanvasAniModifier();
    CHECK_NULL_VOID(canvasModifier);
    canvasModifier->drawPixelMap1(peer, reinterpret_cast<void*>(&mediaPixelMap), dx, dy, dw, dh);
}

void CanvasModule::DrawPixelMap2(ani_env* env, [[maybe_unused]] ani_object aniClass, ani_long peerPtr,
    ani_object pixelMapObj, ani_double sx, ani_double sy, ani_double sw, ani_double sh, ani_double dx, ani_double dy,
    ani_double dw, ani_double dh)
{
    auto* peer = reinterpret_cast<ArkUICanvasRenderer>(peerPtr);
    CHECK_NULL_VOID(peer);
    auto mediaPixelMap = Media::PixelMapTaiheAni::GetNativePixelMap(env, pixelMapObj);
    CHECK_NULL_VOID(mediaPixelMap);
    const auto* modifier = GetNodeAniModifier();
    CHECK_NULL_VOID(modifier);
    auto* canvasModifier = modifier->getCanvasAniModifier();
    CHECK_NULL_VOID(canvasModifier);
    canvasModifier->drawPixelMap2(peer, reinterpret_cast<void*>(&mediaPixelMap), sx, sy, sw, sh, dx, dy, dw, dh);
}

ani_long CanvasModule::ImageBitmapConstruct0(
    ani_env* env, [[maybe_unused]] ani_object aniClass, ani_string src, ani_int unit)
{
    const auto* modifier = GetNodeAniModifier();
    CHECK_NULL_RETURN(modifier, {});
    auto* canvasModifier = modifier->getCanvasAniModifier();
    CHECK_NULL_RETURN(canvasModifier, {});
    std::string str = AniUtils::ANIStringToStdString(env, static_cast<ani_string>(src));
    return reinterpret_cast<ani_long>(canvasModifier->imageBitmapConstruct(str, nullptr, unit));
}

ani_long CanvasModule::ImageBitmapConstruct1(
    ani_env* env, [[maybe_unused]] ani_object aniClass, ani_object pixelMapObj, ani_int unit)
{
    const auto* modifier = GetNodeAniModifier();
    CHECK_NULL_RETURN(modifier, {});
    auto* canvasModifier = modifier->getCanvasAniModifier();
    CHECK_NULL_RETURN(canvasModifier, {});
    auto mediaPixelMap = Media::PixelMapTaiheAni::GetNativePixelMap(env, pixelMapObj);
    CHECK_NULL_RETURN(mediaPixelMap, {});
    return reinterpret_cast<ani_long>(
        canvasModifier->imageBitmapConstruct("", reinterpret_cast<void*>(&mediaPixelMap), unit));
}

ani_double CanvasModule::GetCanvasDensity(ani_env* env, [[maybe_unused]] ani_object aniClass, ani_long peerPtr)
{
    auto* peer = reinterpret_cast<ArkUICanvasRenderer>(peerPtr);
    CHECK_NULL_RETURN(peer, 1.0);
    const auto* modifier = GetNodeAniModifier();
    CHECK_NULL_RETURN(modifier, 1.0);
    auto* canvasModifier = modifier->getCanvasAniModifier();
    CHECK_NULL_RETURN(canvasModifier, 1.0);
    return canvasModifier->getCanvasDensity(peer);
}

ani_double CanvasModule::GetSystemDensity(ani_env* env, [[maybe_unused]] ani_object aniClass)
{
    const auto* modifier = GetNodeAniModifier();
    CHECK_NULL_RETURN(modifier, 1.0);
    auto* canvasModifier = modifier->getCanvasAniModifier();
    CHECK_NULL_RETURN(canvasModifier, 1.0);
    return canvasModifier->getSystemDensity();
}

ani_object CanvasModule::GetImageData(ani_env* env, [[maybe_unused]] ani_object aniClass, ani_long peerPtr,
    ani_double sx, ani_double sy, ani_double sw, ani_double sh)
{
    ani_ref aniRef;
    env->GetUndefined(&aniRef);
    ani_object undefined = static_cast<ani_object>(aniRef);

    auto* peer = reinterpret_cast<ArkUICanvasRenderer>(peerPtr);
    CHECK_NULL_RETURN(peer, undefined);
    const auto* modifier = GetNodeAniModifier();
    CHECK_NULL_RETURN(modifier, undefined);
    auto* canvasModifier = modifier->getCanvasAniModifier();
    CHECK_NULL_RETURN(canvasModifier, undefined);
    uint8_t* imageData;
    canvasModifier->getImageData(peer, &imageData, sx, sy, sw, sh);

    ani_arraybuffer arrayBuffer;
    ani_size length = 10;
    if (!env->CreateArrayBuffer(length, reinterpret_cast<void**>(&imageData), &arrayBuffer)) {
        return undefined;
    }

    static const char* className = "Lescompat/Uint8ClampedArray;";
    ani_class cls;
    if (ANI_OK != env->FindClass(className, &cls)) {
        return undefined;
    }
    ani_method ctor;
    if (ANI_OK != env->Class_FindMethod(cls, "<ctor>", "J:Lescompat/ArrayBuffer", &ctor)) {
        return undefined;
    }
    ani_object aniValue;
    if (ANI_OK != env->Object_New(cls, ctor, &aniValue, arrayBuffer)) {
        return undefined;
    }
    return aniValue;
}

void CanvasModule::PutImageData0(ani_env* env, [[maybe_unused]] ani_object aniClass, ani_long peerPtr, ani_object array,
    ani_double dx, ani_double dy, ani_int width, ani_int height)
{
    auto* peer = reinterpret_cast<ArkUICanvasRenderer>(peerPtr);
    CHECK_NULL_VOID(peer);
    ani_ref buffer;
    if ((env->Object_GetFieldByName_Ref(array, "buffer", &buffer)) != ANI_OK) {
        return;
    }
    ani_arraybuffer arrayBuffer = static_cast<ani_arraybuffer>(buffer);
    uint8_t* data = nullptr;
    ani_size length = 0;
    env->ArrayBuffer_GetInfo(arrayBuffer, reinterpret_cast<void**>(&data), &length);
    CHECK_NULL_VOID(data);
    const auto* modifier = GetNodeAniModifier();
    CHECK_NULL_VOID(modifier);
    auto* canvasModifier = modifier->getCanvasAniModifier();
    CHECK_NULL_VOID(canvasModifier);
    canvasModifier->putImageData0(peer, data, length, dx, dy, width, height);
}

void CanvasModule::PutImageData1(ani_env* env, [[maybe_unused]] ani_object aniClass, ani_long peerPtr, ani_object array,
    ani_double dx, ani_double dy, ani_int width, ani_int height, ani_double dirtyX, ani_double dirtyY,
    ani_double dirtyWidth, ani_double dirtyHeight)
{
    auto* peer = reinterpret_cast<ArkUICanvasRenderer>(peerPtr);
    CHECK_NULL_VOID(peer);
    ani_ref buffer;
    if ((env->Object_GetFieldByName_Ref(array, "buffer", &buffer)) != ANI_OK) {
        return;
    }
    ani_arraybuffer arrayBuffer = static_cast<ani_arraybuffer>(buffer);
    uint8_t* data = nullptr;
    ani_size length = 0;
    env->ArrayBuffer_GetInfo(arrayBuffer, reinterpret_cast<void**>(&data), &length);
    CHECK_NULL_VOID(data);
    const auto* modifier = GetNodeAniModifier();
    CHECK_NULL_VOID(modifier);
    auto* canvasModifier = modifier->getCanvasAniModifier();
    CHECK_NULL_VOID(canvasModifier);
    canvasModifier->putImageData1(peer, data, length, dx, dy, width, height, dirtyX, dirtyY, dirtyWidth, dirtyHeight);
}
} // namespace OHOS::Ace::Ani

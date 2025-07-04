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

#include "drawable_descriptor_ani.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <dlfcn.h>
#include <string>
#include <unordered_set>

#include "ani.h"
#include "cJSON.h"
#include "resource_manager.h"

namespace OHOS::Ace::Ani {
namespace {
constexpr char PIXEL_MAP_DRAWABLE[] = "L@ohos/arkui/drawableDescriptor/PixelMapDrawableDescriptor;";
constexpr char LAYERED_DRAWABLE[] = "L@ohos/arkui/drawableDescriptor/LayeredDrawableDescriptor;";
constexpr char BACKGROUND_KEY[] = "background";
constexpr char FOREGROUND_KEY[] = "foreground";
constexpr char DEFAULT_MASK[] = "ohos_icon_mask";
constexpr char PIXELMAP_SET_RAW_DATA[] = "OHOS_ACE_PixelMapDrawableDescriptor_SetRawData";
constexpr char LAYERED_SET_FOREGROUND_DATA[] = "OHOS_ACE_LayeredDrawableDescriptor_SetForegroundData";
constexpr char LAYERED_SET_BACKGROUND_DATA[] = "OHOS_ACE_LayeredDrawableDescriptor_SetBackgroundData";
constexpr char LAYERED_SET_MASK_DATA[] = "OHOS_ACE_LayeredDrawableDescriptor_SetMaskData";
constexpr char LAYERED_SET_MASK_PATH[] = "OHOS_ACE_LayeredDrawableDescriptor_SetMaskPath";
constexpr char LIBACE_MODULE[] = "libace_compatible.z.so";

constexpr int32_t DECIMAL_BASE = 10;
constexpr size_t BUFFER_NUMBER = 2;

using PixelMapDrawableSetRawDataFunc = void (*)(void*, uint8_t*, size_t);
using LayeredDrawableSetForegoundDataFunc = void (*)(void*, uint8_t*, size_t);
using LayeredDrawableSetBackgroundDataFunc = void (*)(void*, uint8_t*, size_t);
using LayeredDrawableSetMaskDataFunc = void (*)(void*, uint8_t*, size_t);
using LayeredDrawableSetMaskPathFunc = void (*)(void*, const char*);
} // namespace

void PixelMapDrawableSetRawDataC(void* drawable, uint8_t* data, size_t len)
{
    void* handle = dlopen(LIBACE_MODULE, RTLD_LAZY | RTLD_LOCAL);
    if (handle == nullptr) {
        return;
    }
    auto entry = reinterpret_cast<PixelMapDrawableSetRawDataFunc>(dlsym(handle, PIXELMAP_SET_RAW_DATA));
    if (entry == nullptr) {
        dlclose(handle);
        return;
    }
    entry(drawable, data, len);
}

void LayeredDrawableSetForegroundDataC(void* drawable, uint8_t* data, size_t len)
{
    void* handle = dlopen(LIBACE_MODULE, RTLD_LAZY | RTLD_LOCAL);
    if (handle == nullptr) {
        return;
    }
    auto entry = reinterpret_cast<LayeredDrawableSetForegoundDataFunc>(dlsym(handle, LAYERED_SET_FOREGROUND_DATA));
    if (entry == nullptr) {
        dlclose(handle);
        return;
    }
    entry(drawable, data, len);
}

void LayeredDrawableSetBackgroundDataC(void* drawable, uint8_t* data, size_t len)
{
    void* handle = dlopen(LIBACE_MODULE, RTLD_LAZY | RTLD_LOCAL);
    if (handle == nullptr) {
        return;
    }
    auto entry = reinterpret_cast<LayeredDrawableSetBackgroundDataFunc>(dlsym(handle, LAYERED_SET_BACKGROUND_DATA));
    if (entry == nullptr) {
        dlclose(handle);
        return;
    }
    entry(drawable, data, len);
}

void LayeredDrawableSetMaskDataC(void* drawable, uint8_t* data, size_t len)
{
    void* handle = dlopen(LIBACE_MODULE, RTLD_LAZY | RTLD_LOCAL);
    if (handle == nullptr) {
        return;
    }
    auto entry = reinterpret_cast<LayeredDrawableSetMaskDataFunc>(dlsym(handle, LAYERED_SET_MASK_DATA));
    if (entry == nullptr) {
        dlclose(handle);
        return;
    }
    entry(drawable, data, len);
}

void LayeredDrawableSetMaskPathC(void* drawable, const char* path)
{
    void* handle = dlopen(LIBACE_MODULE, RTLD_LAZY | RTLD_LOCAL);
    if (handle == nullptr) {
        return;
    }
    auto entry = reinterpret_cast<LayeredDrawableSetMaskPathFunc>(dlsym(handle, LAYERED_SET_MASK_PATH));
    if (entry == nullptr) {
        dlclose(handle);
        return;
    }
    entry(drawable, path);
}

bool CheckResourceType(const std::string& type)
{
    static const std::unordered_set<std::string> types = { "png", "jpg", "bmp", "svg", "gif", "webp", "astc", "sut" };
    return types.count(type);
}

bool SafeStringToUint(const std::string& str, uint32_t& result)
{
    if (str.empty()) {
        return false;
    }
    char* end = nullptr;
    unsigned long value = strtoul(str.c_str(), &end, DECIMAL_BASE);
    if (str.c_str() == end) {
        return false;
    }
    while (*end != '\0') {
        if (!isspace(static_cast<unsigned char>(*end))) {
            return false;
        }
        ++end;
    }
    if (value > std::numeric_limits<uint32_t>::max() || errno == ERANGE) {
        return false;
    }
    if (str.find('-') != std::string::npos) {
        return false;
    }
    result = static_cast<uint32_t>(value);
    return true;
}

std::vector<uint32_t> GetResIdInJson(const MediaData& jsonBuf, const std::vector<std::string>& keys)
{
    std::vector<uint32_t> ids;
    cJSON* roots = cJSON_ParseWithLength(reinterpret_cast<const char*>(jsonBuf.data.get()), jsonBuf.len);
    if (roots == nullptr) {
        return ids;
    }
    for (const auto& key : keys) {
        cJSON* item = cJSON_GetObjectItem(roots->child, key.c_str());
        if (item == nullptr || !cJSON_IsString(item)) {
            cJSON_Delete(roots);
            return ids;
        }
        std::string value(item->valuestring);
        std::string idStr;
        size_t pos = value.find(":");
        if (pos != std::string::npos) {
            idStr = value.substr(pos + 1);
        } else {
            return ids;
        }
        uint32_t resId = 0;
        if (!SafeStringToUint(idStr, resId)) {
            return ids;
        }
        ids.push_back(resId);
    }
    return ids;
}

ani_object CreatePixelMapDrawable(ani_env* env, const MediaData& mediaData)
{
    ani_class cls;
    ani_object obj = nullptr;
    auto status = env->FindClass(PIXEL_MAP_DRAWABLE, &cls);
    if (status != ANI_OK) {
        return obj;
    }
    ani_method ctor;
    env->Class_FindMethod(cls, "<ctor>", ":", &ctor);
    env->Object_New(cls, ctor, &obj);
    // no media data
    if (mediaData.len <= 0) {
        return obj;
    }
    // have media data
    ani_long nativeObj = 0;
    env->Object_GetPropertyByName_Long(obj, "nativeObj", &nativeObj);
    // take native pixelmap drawable
    auto drawable = reinterpret_cast<void*>(nativeObj);
    if (drawable == nullptr) {
        return obj;
    }
    PixelMapDrawableSetRawDataC(drawable, mediaData.data.get(), mediaData.len);
    return obj;
}

ani_object CreateLayeredDrawableByJsonBuffer(ani_env* env, const DrawableInfo& info)
{
    ani_class cls;
    ani_object obj = nullptr;
    auto status = env->FindClass(LAYERED_DRAWABLE, &cls);
    if (status != ANI_OK) {
        return obj;
    }
    ani_method ctor;
    env->Class_FindMethod(cls, "<ctor>", ":", &ctor);
    env->Object_New(cls, ctor, &obj);
    ani_long nativeObj = 0;
    env->Object_GetPropertyByName_Long(obj, "nativeObj", &nativeObj);
    auto* drawable = reinterpret_cast<void*>(nativeObj);
    if (drawable == nullptr) {
        return obj;
    }
    std::string foreground(FOREGROUND_KEY);
    std::string background(BACKGROUND_KEY);
    auto ids = GetResIdInJson(info.firstBuffer, { foreground, background });
    if (ids.size() < BUFFER_NUMBER) {
        return obj;
    }
    // initialize foreground and background buffer
    auto resMgr = info.manager;
    std::vector<std::pair<uint8_t*, size_t>> datas;
    for (const auto id : ids) {
        std::tuple<std::string, size_t, std::string> dinfo;
        std::unique_ptr<uint8_t[]> data;
        auto state = resMgr->GetDrawableInfoById(id, dinfo, data, 0, info.density);
        if (state == Global::Resource::SUCCESS) {
            datas.push_back({ data.get(), std::get<1>(dinfo) });
        }
    }
    if (datas.size() < BUFFER_NUMBER) {
        return obj;
    }
    LayeredDrawableSetForegroundDataC(drawable, datas[0].first, datas[0].second);
    LayeredDrawableSetBackgroundDataC(drawable, datas[1].first, datas[1].second);
    // initialize mask data
    std::unique_ptr<uint8_t[]> maskData;
    size_t maskLen = 0;
    auto state = resMgr->GetMediaDataByName(DEFAULT_MASK, maskLen, maskData);
    if (state == Global::Resource::SUCCESS || maskLen > 0) {
        LayeredDrawableSetMaskDataC(drawable, maskData.get(), maskLen);
    }
    return obj;
}

ani_object CreateLayerdDrawableByTwoBuffer(ani_env* env, const DrawableInfo& info)
{
    ani_class cls;
    ani_object obj = nullptr;
    auto status = env->FindClass(LAYERED_DRAWABLE, &cls);
    if (status != ANI_OK) {
        return obj;
    }
    ani_method ctor;
    env->Class_FindMethod(cls, "<ctor>", ":", &ctor);
    env->Object_New(cls, ctor, &obj);
    ani_long nativeObj = 0;
    env->Object_GetPropertyByName_Long(obj, "nativeObj", &nativeObj);
    auto* drawable = reinterpret_cast<void*>(nativeObj);
    if (drawable == nullptr) {
        return obj;
    }
    LayeredDrawableSetForegroundDataC(drawable, info.firstBuffer.data.get(), info.firstBuffer.len);
    LayeredDrawableSetBackgroundDataC(drawable, info.secondBuffer.data.get(), info.secondBuffer.len);
    // initialize mask data
    auto resMgr = info.manager;
    auto themeMask = resMgr->GetThemeMask();
    LayeredDrawableSetMaskPathC(drawable, themeMask.c_str());
    std::unique_ptr<uint8_t[]> maskData;
    size_t maskLen = 0;
    auto state = resMgr->GetMediaDataByName(DEFAULT_MASK, maskLen, maskData);
    if (state == Global::Resource::SUCCESS && maskLen > 0) {
        LayeredDrawableSetMaskDataC(drawable, maskData.get(), maskLen);
    }
    return obj;
}

ani_object DrawableDescriptorAni::CreateDrawableDescriptor(ani_env* env, const DrawableInfo& info)
{
    ani_object object = nullptr;
    std::string type = info.type;
    std::transform(type.begin(), type.end(), type.begin(), ::tolower);
    // type is none
    if (type == "none" && info.firstBuffer.len > 0) {
        object = CreatePixelMapDrawable(env, info.firstBuffer);
        return object;
    }

    // type is json
    if (type == "json" && info.firstBuffer.len > 0) {
        object = CreateLayeredDrawableByJsonBuffer(env, info);
        return object;
    }

    // type is layered
    if (type == "layered" && info.firstBuffer.len > 0 && info.secondBuffer.len > 0) {
        object = CreateLayerdDrawableByTwoBuffer(env, info);
        return object;
    }

    // type is bitmap
    if (CheckResourceType(info.type) && info.firstBuffer.len > 0) {
        object = CreatePixelMapDrawable(env, info.firstBuffer);
        return object;
    }

    return object;
}
} // namespace OHOS::Ace::Ani

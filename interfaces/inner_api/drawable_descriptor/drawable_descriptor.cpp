/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "drawable_descriptor.h"

#include <cstddef>
#include <memory>
#include <string>

#include "include/core/SkSamplingOptions.h"
#include "third_party/cJSON/cJSON.h"

#ifndef PREVIEW
#include "image_source.h"
#endif
#include "include/core/SkImage.h"
#include "include/core/SkRect.h"

#ifdef PREVIEW
#ifdef WINDOWS_PLATFORM
#include <direct.h>
#include <windows.h>
#elif defined(MAC_PLATFORM)
#include <mach-o/dyld.h>
#else
#include <unistd.h>
#endif
#endif

namespace OHOS::Ace::Napi {
namespace {
#ifndef PREVIEW
const char DRAWABLEDESCRIPTOR_JSON_KEY_BACKGROUND[] = "background";
const char DRAWABLEDESCRIPTOR_JSON_KEY_FOREGROUND[] = "foreground";
#endif
constexpr float SIDE = 192.0f;
const int DEFAULT_DURATION = 1000;
const std::string DEFAULT_MASK = "ohos_icon_mask";

// define for get resource path in preview scenes
const static char PREVIEW_LOAD_RESOURCE_ID[] = "ohos_drawable_descriptor_path";
#ifdef PREVIEW
#ifdef WINDOWS_PLATFORM
constexpr static char PREVIEW_LOAD_RESOURCE_PATH[] = "\\resources\\resources.index";
#else
constexpr static char PREVIEW_LOAD_RESOURCE_PATH[] = "/resources/resources.index";
#endif

#ifdef LINUX_PLATFORM
const static size_t MAX_PATH_LEN = 255;
#endif
#endif
inline bool IsNumber(const std::string& value)
{
    if (value.empty()) {
        return false;
    }
    return std::all_of(value.begin(), value.end(), [](char i) { return isdigit(i); });
}
} // namespace

DrawableItem LayeredDrawableDescriptor::PreGetDrawableItem(
    const std::shared_ptr<Global::Resource::ResourceManager>& resourceMgr, const char* item)
{
    std::string itemStr = item;
    std::string idStr = itemStr.substr(itemStr.find(':') + 1);
    DrawableItem resItem;
    if (!IsNumber(idStr)) {
        return resItem;
    }

    std::tuple<std::string, size_t, std::string> info;
    auto state = resourceMgr->GetDrawableInfoById(
        static_cast<uint32_t>(std::stoul(idStr)), info, resItem.data_, iconType_, density_);
    resItem.len_ = std::get<1>(info);
    resItem.state_ = state;
    return resItem;
}

bool LayeredDrawableDescriptor::PreGetPixelMapFromJsonBuf(
    const std::shared_ptr<Global::Resource::ResourceManager>& resourceMgr, bool isBackground)
{
#ifndef PREVIEW
    cJSON* roots = cJSON_ParseWithLength(reinterpret_cast<const char*>(jsonBuf_.get()), len_);

    if (roots == nullptr) {
        return false;
    }

    cJSON* item = nullptr;
    if (isBackground) {
        item = cJSON_GetObjectItem(roots->child, DRAWABLEDESCRIPTOR_JSON_KEY_BACKGROUND);
    } else {
        item = cJSON_GetObjectItem(roots->child, DRAWABLEDESCRIPTOR_JSON_KEY_FOREGROUND);
    }
    if (item == nullptr) {
        cJSON_Delete(roots);
        HILOGE("GetObjectItem from json buffer failed");
        return false;
    }
    if (cJSON_IsString(item)) {
        DrawableItem& drawableiItem = isBackground ? backgroundItem_ : foregroundItem_;
        drawableiItem = PreGetDrawableItem(resourceMgr, item->valuestring);
    } else {
        cJSON_Delete(roots);
        return false;
    }
    cJSON_Delete(roots);
    return true;
#else
    return false;
#endif
}

void LayeredDrawableDescriptor::InitialResource(const std::shared_ptr<Global::Resource::ResourceManager>& resourceMgr)
{
    if (!resourceMgr) {
        HILOGE("Global resource manager is null!");
        return;
    }
    InitialMask(resourceMgr);
    // preprocess get background and foreground
    if (!PreGetPixelMapFromJsonBuf(resourceMgr, true)) {
        HILOGE("Create background Item imageSource from json buffer failed");
    }
    if (!PreGetPixelMapFromJsonBuf(resourceMgr, false)) {
        HILOGE("Create foreground Item imageSource from json buffer failed");
    }
}

void LayeredDrawableDescriptor::InitialMask(const std::shared_ptr<Global::Resource::ResourceManager>& resourceMgr)
{
    resourceMgr->GetMediaDataByName(DEFAULT_MASK.c_str(), defaultMaskDataLength_, defaultMaskData_);
}

bool DrawableDescriptor::GetPixelMapFromBuffer()
{
    Media::SourceOptions opts;
    uint32_t errorCode = 0;
    std::unique_ptr<Media::ImageSource> imageSource =
        Media::ImageSource::CreateImageSource(mediaData_.get(), len_, opts, errorCode);
    if (errorCode != 0) {
        HILOGE("CreateImageSource from buffer failed");
        return false;
    }
    mediaData_.reset();
    Media::DecodeOptions decodeOpts;
    decodeOpts.desiredPixelFormat = Media::PixelFormat::BGRA_8888;
    if (imageSource) {
        auto pixelMapPtr = imageSource->CreatePixelMap(decodeOpts, errorCode);
        pixelMap_ = std::shared_ptr<Media::PixelMap>(pixelMapPtr.release());
    }
    if (errorCode != 0 || !pixelMap_) {
        HILOGE("Get PixelMap from buffer failed");
        return false;
    }
    return true;
}

std::shared_ptr<Media::PixelMap> DrawableDescriptor::GetPixelMap()
{
    if (pixelMap_.has_value()) {
        return pixelMap_.value();
    }
    if (GetPixelMapFromBuffer()) {
        return pixelMap_.value();
    }
    HILOGE("Failed to GetPixelMap!");
    return nullptr;
}

DrawableDescriptor::DrawableType DrawableDescriptor::GetDrawableType()
{
    return DrawableType::BASE;
}

std::unique_ptr<Media::ImageSource> LayeredDrawableDescriptor::CreateImageSource(
    DrawableItem& drawableItem, uint32_t& errorCode)
{
    if (drawableItem.state_ != Global::Resource::SUCCESS) {
        HILOGE("GetDrawableInfoById failed");
        return nullptr;
    }

    Media::SourceOptions opts;
    return Media::ImageSource::CreateImageSource(drawableItem.data_.get(), drawableItem.len_, opts, errorCode);
}

bool LayeredDrawableDescriptor::GetPixelMapFromJsonBuf(bool isBackground)
{
#ifndef PREVIEW
    if ((isBackground && background_.has_value()) || (!isBackground && foreground_.has_value())) {
        return true;
    }
    if ((isBackground && backgroundItem_.state_ == Global::Resource::SUCCESS) ||
        (!isBackground && foregroundItem_.state_ == Global::Resource::SUCCESS)) {
        uint32_t errorCode = 0;
        std::unique_ptr<Media::ImageSource> imageSource =
            LayeredDrawableDescriptor::CreateImageSource(isBackground ? backgroundItem_ : foregroundItem_, errorCode);
        if (errorCode != 0) {
            HILOGE("CreateImageSource from json buffer failed");
            return false;
        }
        Media::DecodeOptions decodeOpts;
        decodeOpts.desiredPixelFormat = Media::PixelFormat::BGRA_8888;
        if (imageSource) {
            auto pixelMapPtr = imageSource->CreatePixelMap(decodeOpts, errorCode);
            if (errorCode != 0) {
                HILOGE("Get PixelMap from json buffer failed");
                return false;
            }

            if (isBackground) {
                background_ = std::shared_ptr<Media::PixelMap>(pixelMapPtr.release());
            } else {
                foreground_ = std::shared_ptr<Media::PixelMap>(pixelMapPtr.release());
            }
        }
    } else {
        HILOGE("Get background from json buffer failed");
        return false;
    }
    if (isBackground) {
        backgroundItem_.data_.reset();
    } else {
        foregroundItem_.data_.reset();
    }
    return true;
#else
    return false;
#endif
}

bool LayeredDrawableDescriptor::GetDefaultMask()
{
    Media::SourceOptions opts;
    uint32_t errorCode = 0;
    std::unique_ptr<Media::ImageSource> imageSource =
        Media::ImageSource::CreateImageSource(
            defaultMaskData_.get(), defaultMaskDataLength_, opts, errorCode);
    Media::DecodeOptions decodeOpts;
    decodeOpts.desiredPixelFormat = Media::PixelFormat::BGRA_8888;
    if (imageSource) {
        auto pixelMapPtr = imageSource->CreatePixelMap(decodeOpts, errorCode);
        mask_ = std::shared_ptr<Media::PixelMap>(pixelMapPtr.release());
    }
    if (errorCode != 0 || !mask_) {
        HILOGE("Get mask failed");
        return false;
    }
    return true;
}

void LayeredDrawableDescriptor::InitLayeredParam(std::pair<std::unique_ptr<uint8_t[]>, size_t> &foregroundInfo,
    std::pair<std::unique_ptr<uint8_t[]>, size_t> &backgroundInfo)
{
    Media::SourceOptions opts;
    uint32_t errorCode = 0;
    auto foreground = Media::ImageSource::CreateImageSource(foregroundInfo.first.get(), foregroundInfo.second, opts,
        errorCode);
    if (errorCode == 0) {
        Media::DecodeOptions decodeOpts;
        decodeOpts.desiredPixelFormat = Media::PixelFormat::BGRA_8888;
        auto pixelMapPtr = foreground->CreatePixelMap(decodeOpts, errorCode);
        foreground_ = std::shared_ptr<Media::PixelMap>(pixelMapPtr.release());
    }
    auto background = Media::ImageSource::CreateImageSource(backgroundInfo.first.get(), backgroundInfo.second, opts,
        errorCode);
    if (errorCode == 0) {
        Media::DecodeOptions decodeOpts;
        decodeOpts.desiredPixelFormat = Media::PixelFormat::BGRA_8888;
        auto pixelMapPtr = background->CreatePixelMap(decodeOpts, errorCode);
        background_ = std::shared_ptr<Media::PixelMap>(pixelMapPtr.release());
    }
}

bool LayeredDrawableDescriptor::GetMaskByPath()
{
    if (maskPath_.empty()) {
        HILOGD("maskPath is null");
        return false;
    }
    Media::SourceOptions opts;
    uint32_t errorCode = 0;
    std::unique_ptr<Media::ImageSource> imageSource =
        Media::ImageSource::CreateImageSource(maskPath_, opts, errorCode);
    Media::DecodeOptions decodeOpts;
    decodeOpts.desiredPixelFormat = Media::PixelFormat::BGRA_8888;
    if (imageSource) {
        auto pixelMapPtr = imageSource->CreatePixelMap(decodeOpts, errorCode);
        mask_ = std::shared_ptr<Media::PixelMap>(pixelMapPtr.release());
    }
    if (errorCode != 0 || !mask_) {
        HILOGE("Get mask failed");
        return false;
    }
    return true;
}

bool LayeredDrawableDescriptor::GetMaskByName(
    std::shared_ptr<Global::Resource::ResourceManager>& resourceMgr, const std::string& name)
{
    size_t len = 0;
    std::unique_ptr<uint8_t[]> data;
    resourceMgr->GetMediaDataByName(name.c_str(), len, data);
    Media::SourceOptions opts;
    uint32_t errorCode = 0;
    std::unique_ptr<Media::ImageSource> imageSource =
        Media::ImageSource::CreateImageSource(data.get(), len, opts, errorCode);
    Media::DecodeOptions decodeOpts;
    decodeOpts.desiredPixelFormat = Media::PixelFormat::BGRA_8888;
    if (imageSource) {
        auto pixelMapPtr = imageSource->CreatePixelMap(decodeOpts, errorCode);
        mask_ = std::shared_ptr<Media::PixelMap>(pixelMapPtr.release());
    }
    if (errorCode != 0 || !mask_) {
        HILOGE("Get mask failed");
        return false;
    }
    return true;
}

std::unique_ptr<DrawableDescriptor> LayeredDrawableDescriptor::GetForeground()
{
    if (foreground_.has_value()) {
        return std::make_unique<DrawableDescriptor>(foreground_.value());
    }

    if (GetPixelMapFromJsonBuf(false)) {
        return std::make_unique<DrawableDescriptor>(foreground_.value());
    }

    HILOGE("GetForeground failed");
    return nullptr;
}

std::unique_ptr<DrawableDescriptor> LayeredDrawableDescriptor::GetBackground()
{
    if (background_.has_value()) {
        return std::make_unique<DrawableDescriptor>(background_.value());
    }

    if (GetPixelMapFromJsonBuf(true)) {
        return std::make_unique<DrawableDescriptor>(background_.value());
    }
    HILOGE("GetBackground failed");
    return nullptr;
}

std::unique_ptr<DrawableDescriptor> LayeredDrawableDescriptor::GetMask()
{
    if (mask_.has_value()) {
        return std::make_unique<DrawableDescriptor>(mask_.value());
    }

    if (GetMaskByPath()) {
        return std::make_unique<DrawableDescriptor>(mask_.value());
    }

    if (GetDefaultMask()) {
        return std::make_unique<DrawableDescriptor>(mask_.value());
    }

    HILOGE("GetMask failed");
    return nullptr;
}

#ifndef USE_ROSEN_DRAWING
void LayeredDrawableDescriptor::DrawOntoCanvas(
    const std::shared_ptr<SkBitmap>& bitMap, float width, float height, SkCanvas& canvas, const SkPaint& paint)
{
    auto x = static_cast<float>((bitMap->width() - static_cast<float>(width)) / 2);
    auto y = static_cast<float>((bitMap->height() - static_cast<float>(height)) / 2);
    auto rect1 = SkRect::MakeXYWH(x, y, static_cast<float>(width), static_cast<float>(width));
    auto rect2 = SkRect::MakeWH(static_cast<float>(width), static_cast<float>(width));

    canvas.drawImageRect(
        SkImage::MakeFromBitmap(*bitMap), rect1, rect2, SkSamplingOptions(), &paint, SkCanvas::kFast_SrcRectConstraint);
}
#else
void LayeredDrawableDescriptor::DrawOntoCanvas(
    const std::shared_ptr<Rosen::Drawing::Bitmap>& bitMap, float width, float height, Rosen::Drawing::Canvas& canvas)
{
    auto x = static_cast<float>((bitMap->GetWidth() - static_cast<float>(width)) / 2);
    auto y = static_cast<float>((bitMap->GetHeight() - static_cast<float>(height)) / 2);
    Rosen::Drawing::Rect rect1(x, y, static_cast<float>(width) + x, static_cast<float>(width) + y);
    Rosen::Drawing::Rect rect2(0, 0, static_cast<float>(width), static_cast<float>(width));
    Rosen::Drawing::Image image;
    image.BuildFromBitmap(*bitMap);
    canvas.DrawImageRect(image, rect1, rect2, Rosen::Drawing::SamplingOptions(),
        Rosen::Drawing::SrcRectConstraint::FAST_SRC_RECT_CONSTRAINT);
}
#endif

#ifndef USE_ROSEN_DRAWING
bool LayeredDrawableDescriptor::CreatePixelMap()
{
    std::shared_ptr<SkBitmap> foreground;
    if (foreground_.has_value()) {
        foreground = ImageConverter::PixelMapToBitmap(foreground_.value());
    } else if (GetPixelMapFromJsonBuf(false) && foreground_.has_value()) {
        foreground = ImageConverter::PixelMapToBitmap(foreground_.value());
    } else {
        HILOGI("Get pixelMap of foreground failed.");
        return false;
    }

    std::shared_ptr<SkBitmap> background;
    if (background_.has_value()) {
        background = ImageConverter::PixelMapToBitmap(background_.value());
    } else if (GetPixelMapFromJsonBuf(true) && background_.has_value()) {
        background = ImageConverter::PixelMapToBitmap(background_.value());
    } else {
        HILOGE("Get pixelMap of background failed.");
        return false;
    }

    std::shared_ptr<SkBitmap> mask;
    if (mask_.has_value()) {
        mask = ImageConverter::PixelMapToBitmap(mask_.value());
    } else if (GetMaskByPath() && mask_.has_value()) {
        mask = ImageConverter::PixelMapToBitmap(mask_.value());
    } else if (GetDefaultMask() && mask_.has_value()) {
        mask = ImageConverter::PixelMapToBitmap(mask_.value());
    } else {
        HILOGE("Get pixelMap of mask failed.");
        return false;
    }

    SkPaint paint;
    paint.setAntiAlias(true);
    auto colorType = ImageConverter::PixelFormatToSkColorType(background_.value()->GetPixelFormat());
    auto alphaType = ImageConverter::AlphaTypeToSkAlphaType(background_.value()->GetAlphaType());
    auto imageInfo = SkImageInfo::Make(SIDE, SIDE, colorType, alphaType);
    SkBitmap tempCache;
    tempCache.allocPixels(imageInfo);
    SkCanvas bitmapCanvas(tempCache);

    paint.setBlendMode(SkBlendMode::kSrc);
    DrawOntoCanvas(background, SIDE, SIDE, bitmapCanvas, paint);
    paint.setBlendMode(SkBlendMode::kDstATop);
    DrawOntoCanvas(mask, SIDE, SIDE, bitmapCanvas, paint);
    paint.setBlendMode(SkBlendMode::kSrcATop);
    DrawOntoCanvas(foreground, SIDE, SIDE, bitmapCanvas, paint);
    bitmapCanvas.readPixels(tempCache, 0, 0);

    // convert bitmap back to pixelMap
    Media::InitializationOptions opts;
    opts.alphaType = background_.value()->GetAlphaType();
    opts.pixelFormat = Media::PixelFormat::BGRA_8888;
    layeredPixelMap_ = ImageConverter::BitmapToPixelMap(std::make_shared<SkBitmap>(tempCache), opts);
    return true;
}
#else
bool LayeredDrawableDescriptor::CreatePixelMap()
{
    // if customizedParam_.HasParamCustomized() true,
    // meaning this descriptor is not created by resource manager,
    // therefore some params might not be valid.
    // Otherwise if HasParamCustomized() false,
    // meaning this descriptor is created by resource manager or 
    // napi directly but has no param passed in, then we should return if any param is missing
    std::shared_ptr<Rosen::Drawing::Bitmap> foreground;
    if (foreground_.has_value() || GetPixelMapFromJsonBuf(false)) {
        foreground = ImageConverter::PixelMapToBitmap(foreground_.value());
    } else if (!customized_) {
        HILOGI("Get pixelMap of foreground failed.");
        return false;
    }

    std::shared_ptr<Rosen::Drawing::Bitmap> background;
    if (background_.has_value() || GetPixelMapFromJsonBuf(true)) {
        background = ImageConverter::PixelMapToBitmap(background_.value());
    } else if (!customized_) {
        HILOGE("Get pixelMap of background failed.");
        return false;
    }

    std::shared_ptr<Rosen::Drawing::Bitmap> mask;
    if (mask_.has_value() || GetMaskByPath() || GetDefaultMask()) {
        mask = ImageConverter::PixelMapToBitmap(mask_.value());
    } else if (!customized_) {
        HILOGE("Get pixelMap of mask failed.");
        return false;
    }

    Rosen::Drawing::Brush brush;
    brush.SetAntiAlias(true);
    Rosen::Drawing::ColorType colorType;
    if (background_.has_value()) {
        colorType = ImageConverter::PixelFormatToColorType(background_.value()->GetPixelFormat());
    } else {
        colorType = ImageConverter::PixelFormatToColorType(Media::PixelFormat::RGBA_8888);
    }
    Rosen::Drawing::AlphaType alphaType;
    if (background_.has_value()) {
        alphaType = ImageConverter::AlphaTypeToAlphaType(background_.value()->GetAlphaType());
    } else {
        alphaType = ImageConverter::AlphaTypeToAlphaType(Media::AlphaType::IMAGE_ALPHA_TYPE_PREMUL);
    }
    Rosen::Drawing::ImageInfo imageInfo(SIDE, SIDE, colorType, alphaType);
    Rosen::Drawing::Bitmap tempCache;
    tempCache.Build(imageInfo);
    Rosen::Drawing::Canvas bitmapCanvas;
    bitmapCanvas.Bind(tempCache);

    // if developer uses customized param, foreground, background, mask might be null
    if (background) {
        brush.SetBlendMode(Rosen::Drawing::BlendMode::SRC);
        bitmapCanvas.AttachBrush(brush);
        DrawOntoCanvas(background, SIDE, SIDE, bitmapCanvas);
        bitmapCanvas.DetachBrush();
    }
    if (mask) {
        brush.SetBlendMode(Rosen::Drawing::BlendMode::DST_ATOP);
        bitmapCanvas.AttachBrush(brush);
        DrawOntoCanvas(mask, SIDE, SIDE, bitmapCanvas);
        bitmapCanvas.DetachBrush();
    }
    if (foreground) {
        brush.SetBlendMode(Rosen::Drawing::BlendMode::SRC_ATOP);
        bitmapCanvas.AttachBrush(brush);
        DrawOntoCanvas(foreground, SIDE, SIDE, bitmapCanvas);
        bitmapCanvas.DetachBrush();
    }
    bitmapCanvas.ReadPixels(imageInfo, tempCache.GetPixels(), tempCache.GetRowBytes(), 0, 0);
    // convert bitmap back to pixelMap
    Media::InitializationOptions opts;
    if (background_.has_value()) {
        opts.alphaType = background_.value()->GetAlphaType();
    } else {
        opts.alphaType = Media::AlphaType::IMAGE_ALPHA_TYPE_PREMUL;
    }
    opts.pixelFormat = Media::PixelFormat::BGRA_8888;
    layeredPixelMap_ = ImageConverter::BitmapToPixelMap(std::make_shared<Rosen::Drawing::Bitmap>(tempCache), opts);
    return true;
}
#endif

std::shared_ptr<Media::PixelMap> LayeredDrawableDescriptor::GetPixelMap()
{
    if (layeredPixelMap_.has_value()) {
        return layeredPixelMap_.value();
    }

    if (CreatePixelMap()) {
        return layeredPixelMap_.value();
    }

    HILOGE("Failed to GetPixelMap!");
    return nullptr;
}

DrawableDescriptor::DrawableType LayeredDrawableDescriptor::GetDrawableType()
{
    return DrawableType::LAYERED;
}

std::string LayeredDrawableDescriptor::GetStaticMaskClipPath()
{
    std::string data;
    std::shared_ptr<Global::Resource::ResourceManager> resMgr(Global::Resource::CreateResourceManager());

#ifdef PREVIEW
    std::string pathTmp = "";
#ifdef WINDOWS_PLATFORM
    char pathBuf[MAX_PATH];
    _getcwd(pathBuf, MAX_PATH);
    pathTmp = pathBuf;
#elif defined(MAC_PLATFORM)
    uint32_t size = 0;
    _NSGetExecutablePath(nullptr, &size);

    char pathBuf[size + 1];
    if (_NSGetExecutablePath(pathBuf, &size) != 0) {
        pathBuf[0] = '\0';
        HILOGE("Failed, buffer too small!");
    }
    pathBuf[size] = '\0';

    std::string previewFullPath = pathBuf;
    size_t lastPathIdx = previewFullPath.find_last_of("\\/");
    pathTmp = (lastPathIdx != std::string::npos) ? previewFullPath.substr(0, lastPathIdx) : "";
#else
    char pathBuf[MAX_PATH_LEN];
    readlink("/proc/self/exe", pathBuf, MAX_PATH_LEN);
    pathTmp = pathBuf;
#endif
    size_t lastPathSepLoc = pathTmp.find_last_of("\\/");
    std::string path = (lastPathSepLoc != std::string::npos) ? pathTmp.substr(0, lastPathSepLoc) : "";
    path += PREVIEW_LOAD_RESOURCE_PATH;
    resMgr->AddResource(path.c_str());
#endif
    resMgr->GetStringByName(PREVIEW_LOAD_RESOURCE_ID, data);
    return data;
}

std::shared_ptr<Media::PixelMap> AnimatedDrawableDescriptor::GetPixelMap()
{
    if (pixelMapList_.empty()) {
        return nullptr;
    }
    return pixelMapList_[0];
}

DrawableDescriptor::DrawableType AnimatedDrawableDescriptor::GetDrawableType()
{
    return DrawableType::ANIMATED;
}

std::vector<std::shared_ptr<Media::PixelMap>> AnimatedDrawableDescriptor::GetPixelMapList()
{
    return pixelMapList_;
}

int32_t AnimatedDrawableDescriptor::GetDuration()
{
    if (duration_ <= 0) {
        duration_ = DEFAULT_DURATION * static_cast<int32_t>(pixelMapList_.size());
    }
    return duration_;
}

int32_t AnimatedDrawableDescriptor::GetIterations()
{
    if (iterations_ < -1) {
        iterations_ = 1;
    }
    return iterations_;
}

void AnimatedDrawableDescriptor::SetDuration(int32_t duration)
{
    if (duration <= 0) {
        duration_ = DEFAULT_DURATION * static_cast<int32_t>(pixelMapList_.size());
    } else {
        duration_ = duration;
    }
}

void AnimatedDrawableDescriptor::SetIterations(int32_t iterations)
{
    if (iterations < -1) {
        iterations_ = 1;
    } else {
        iterations_ = iterations;
    }
}
} // namespace OHOS::Ace::Napi

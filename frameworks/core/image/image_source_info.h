/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_IMAGE_ACE_IMAGE_SOURCE_INFO_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_IMAGE_ACE_IMAGE_SOURCE_INFO_H

#include <optional>

#include "base/geometry/dimension.h"
#include "base/geometry/size.h"
#include "base/image/pixel_map.h"
#include "base/resource/internal_resource.h"
#include "core/components/common/layout/constants.h"
#include "core/components/common/properties/color.h"

namespace OHOS::Ace {
class ImageSourceInfo {
public:
    static bool IsSVGSource(const std::string& imageSrc, InternalResource::ResourceId resourceId);
    static SrcType ResolveURIType(const std::string& uri);
    static bool IsValidBase64Head(const std::string& uri, const std::string& pattern);
    static bool IsUriOfDataAbilityEncoded(const std::string& uri, const std::string& pattern);

    explicit ImageSourceInfo(const std::string& imageSrc, Dimension width = Dimension(-1),
        Dimension height = Dimension(-1), InternalResource::ResourceId resourceId = InternalResource::ResourceId::NO_ID,
        const RefPtr<PixelMap>& pixmap = nullptr);

    ImageSourceInfo() = default;
    ~ImageSourceInfo() = default;

    bool operator==(const ImageSourceInfo& info) const;

    bool operator!=(const ImageSourceInfo& info) const;

    void SetSrc(const std::string& src, std::optional<Color> fillColor = std::nullopt)
    {
        src_ = src;
        resourceId_ = InternalResource::ResourceId::NO_ID;
        isSvg_ = IsSVGSource(src_, resourceId_);
        fillColor_ = fillColor;
        pixmap_ = nullptr;
    }

    const std::string& GetSrc() const
    {
        return src_;
    }

    void SetResourceId(InternalResource::ResourceId id, std::optional<Color> fillColor = std::nullopt)
    {
        resourceId_ = id;
        src_.clear();
        isSvg_ = IsSVGSource(src_, resourceId_);
        fillColor_ = fillColor;
        pixmap_ = nullptr;
    }

    InternalResource::ResourceId GetResourceId() const
    {
        return resourceId_;
    }

    bool IsInternalResource() const
    {
        return src_.empty() && resourceId_ != InternalResource::ResourceId::NO_ID && !pixmap_;
    }

    bool IsValid() const
    {
        return (src_.empty() && resourceId_ != InternalResource::ResourceId::NO_ID) ||
               (!src_.empty() && resourceId_ == InternalResource::ResourceId::NO_ID) || pixmap_;
    }

    bool IsSvg() const
    {
        return isSvg_;
    }

    bool IsPixmap() const
    {
        return pixmap_ != nullptr;
    }

    SrcType GetSrcType() const
    {
        return srcType_;
    }

    std::string ToString() const;

    void SetDimension(Dimension width, Dimension Height)
    {
        sourceWidth_ = width;
        sourceHeight_ = Height;
    }

    bool IsSourceDimensionValid() const
    {
        return sourceWidth_.IsValid() && sourceHeight_.IsValid();
    }

    Size GetSourceSize() const
    {
        return Size(sourceWidth_.Value(), sourceHeight_.Value());
    }

    void Reset()
    {
        src_.clear();
        sourceWidth_ = Dimension(-1);
        sourceHeight_ = Dimension(-1);
        resourceId_ = InternalResource::ResourceId::NO_ID;
        isSvg_ = false;
        fillColor_.reset();
        pixmap_ = nullptr;
        cacheKey_.clear();
    }

    void SetFillColor(const Color& color);

    void UpdateSrcType()
    {
        srcType_ = ResolveSrcType();
    }

    std::optional<Color> GetFillColor() const
    {
        return fillColor_;
    }

    const RefPtr<PixelMap>& GetPixmap() const
    {
        return pixmap_;
    }

    std::string GetCacheKey() const;

private:
    SrcType ResolveSrcType() const;

    std::string src_;
    std::string cacheKey_;
    Dimension sourceWidth_ = Dimension(-1);
    Dimension sourceHeight_ = Dimension(-1);
    InternalResource::ResourceId resourceId_ = InternalResource::ResourceId::NO_ID;
    RefPtr<PixelMap> pixmap_;
    bool isSvg_ = false;

    // only Svg will set it.
    std::optional<Color> fillColor_;

    // image source type for example:FILE, ASSET, NETWORK, MEMORY, BASE64, INTERNAL, RESOURCE or DATA_ABILITY,
    SrcType srcType_ = SrcType::UNSUPPORTED;
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_IMAGE_ACE_IMAGE_SOURCE_INFO_H
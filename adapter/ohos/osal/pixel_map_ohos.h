/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_ADAPTER_OHOS_OSAL_PIXEL_MAP_OHOS_H
#define FOUNDATION_ACE_ADAPTER_OHOS_OSAL_PIXEL_MAP_OHOS_H

#include "pixel_map.h"

#include "base/image/pixel_map.h"

namespace OHOS::Ace {

class PixelMapOhos : public PixelMap {
    DECLARE_ACE_TYPE(PixelMapOhos, PixelMap)

public:
    explicit PixelMapOhos(std::shared_ptr<Media::PixelMap> pixmap) : pixmap_(std::move(pixmap)) {}
    ~PixelMapOhos() override = default;
    static PixelFormat PixelFormatConverter(Media::PixelFormat pixelFormat);
    static Media::PixelFormat ConvertToMediaPixelFormat(Ace::PixelFormat pixelFormat);
    static AlphaType AlphaTypeConverter(Media::AlphaType alphaType);
    static Media::AlphaType ConvertToMediaAlphaType(Ace::AlphaType alphaType);
    static AllocatorType AllocatorTypeConverter(Media::AllocatorType allocatorType);
    static Media::ScaleMode ConvertToMediaScaleMode(Ace::ScaleMode scaleMode);
    int32_t GetWidth() const override;
    int32_t GetHeight() const override;
    bool GetPixelsVec(std::vector<uint8_t>& data) const override;
    const uint8_t* GetPixels() const override;
    PixelFormat GetPixelFormat() const override;
    AlphaType GetAlphaType() const override;
    int32_t GetRowStride() const override;
    int32_t GetRowBytes() const override;
    int32_t GetByteCount() const override;
    AllocatorType GetAllocatorType() const override;
    bool IsHdr() const override;
    void* GetPixelManager() const override;
    void* GetRawPixelMapPtr() const override;
    std::string GetId() override;
    std::string GetModifyId() override;
    std::shared_ptr<Media::PixelMap> GetPixelMapSharedPtr() override;
    void* GetWritablePixels() const override;
    void Scale(float xAxis, float yAxis) override;
    void Scale(float xAxis, float yAxis, const AceAntiAliasingOption &option) override;
    void SavePixelMapToFile(const std::string& dst) const override;
    RefPtr<PixelMap> GetCropPixelMap(const Rect& srcRect) override;
    bool EncodeTlv(std::vector<uint8_t>& buff) override;
    uint32_t WritePixels(const WritePixelsOptions& opts) override;
    uint32_t GetInnerColorGamut() const override;
    void SetMemoryName(std::string pixelMapName) const override;

private:
    std::shared_ptr<Media::PixelMap> pixmap_;
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_ADAPTER_OHOS_OSAL_PIXEL_MAP_OHOS_H
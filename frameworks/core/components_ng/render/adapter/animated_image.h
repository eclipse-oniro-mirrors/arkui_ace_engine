/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_RENDER_ADAPTER_ANIMATED_IMAGE_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_RENDER_ADAPTER_ANIMATED_IMAGE_H

#include <memory>
#include <utility>

#include "include/codec/SkCodec.h"
#include "include/core/SkImage.h"

#include "core/components_ng/image_provider/image_data.h"
#include "core/components_ng/render/adapter/skia_image.h"

namespace OHOS::Ace::NG {
class AnimatedImage : public SkiaImage {
    DECLARE_ACE_TYPE(AnimatedImage, SkiaImage)
public:
    AnimatedImage(std::unique_ptr<SkCodec> codec, std::string url);

    ~AnimatedImage() override = default;

    static RefPtr<CanvasImage> Create(const RefPtr<ImageData>& data, const std::string& url);

    sk_sp<SkImage> GetImage() const override;

    RefPtr<CanvasImage> Clone() override
    {
        return Claim(this);
    }

    bool IsStatic() override
    {
        return false;
    }
    void SetRedrawCallback(std::function<void()>&& callback) override
    {
        redraw_ = std::move(callback);
    }
    void ControlAnimation(bool play) override;

private:
    AnimatedImage(std::unique_ptr<SkCodec> codec, const SkISize& size, std::string key);

    void RenderFrame(uint32_t idx);
    void DecodeFrame(uint32_t idx);

    void CacheFrame(uint32_t idx);
    bool GetCachedFrame(uint32_t idx);

    // ensure frames decode serially, protect bitmap
    std::mutex decodeMtx_;
    SkBitmap requiredFrame_;

    // protect currentFrame_
    mutable std::mutex frameMtx_;
    sk_sp<SkImage> currentFrame_;

    std::function<void()> redraw_;
    RefPtr<Animator> animator_;
    std::unique_ptr<SkCodec> codec_;

    const std::string cacheKey_;
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_RENDER_ADAPTER_ANIMATED_IMAGE_H

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

#ifndef FOUNDATION_ACE_FRAMEWORKS_BASE_RESOURCE_SHARED_IMAGE_MANAGER_H
#define FOUNDATION_ACE_FRAMEWORKS_BASE_RESOURCE_SHARED_IMAGE_MANAGER_H

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <mutex>
#include <new>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "base/memory/ace_type.h"
#include "base/thread/cancelable_callback.h"
#include "base/thread/task_executor.h"
#include "base/utils/noncopyable.h"

namespace OHOS::Ace {

constexpr int32_t DEFAULT_FORM_SHARED_IMAGE_CACHE_THRESHOLD = 20;
constexpr int32_t FORM_MULTI_IMAGE_CACHE_THRESHOLD = 30;

using SharedImage = std::vector<uint8_t>;
using SharedImageMap = std::unordered_map<std::string, SharedImage>;

class ImageProviderLoader;
using ProviderMapToReload = std::unordered_map<std::string, std::set<WeakPtr<ImageProviderLoader>>>;

class SharedImageManager : public AceType {
    DECLARE_ACE_TYPE(SharedImageManager, AceType);

public:
    explicit SharedImageManager(const RefPtr<TaskExecutor>& taskExecutor)
        : sharedImageTotalSize_(0), sharedImageCacheThreshold_(DEFAULT_FORM_SHARED_IMAGE_CACHE_THRESHOLD),
          taskExecutor_(taskExecutor) {
        LOGI("SharedImageManager threshold %{public}d", sharedImageCacheThreshold_.load());
    }
    ~SharedImageManager() override = default;
    void AddSharedImage(const std::string& name, SharedImage&& sharedImage);
    void AddPictureNamesToReloadMap(std::string&& name);
    bool FindImageInSharedImageMap(const std::string& name, const WeakPtr<ImageProviderLoader>& providerWp);

    void SetImageCacheThreshold(int32_t threshold)
    {
        sharedImageCacheThreshold_.store(threshold);
        LOGI("SetImageCacheThreshold %{public}d", threshold);
    }

    int32_t GetImageCacheThreshold() const
    {
        return sharedImageCacheThreshold_.load();
    }

    const SharedImageMap& GetSharedImageMap() const
    {
        return sharedImageMap_;
    }

    bool Remove(const std::string& name);

    // return true if successfully registered
    bool RegisterLoader(const std::string& name, const WeakPtr<ImageProviderLoader>& providerWp);

private:
    void PostDelayedTaskToClearImageData(const std::string& name, size_t dataSize);
    std::function<void()> GenerateClearImageDataCallback(const std::string& name, size_t dataSize);
    bool UpdateImageMap(const std::string& name, const SharedImage& sharedImage);

    std::mutex sharedImageMapMutex_;
    std::mutex providerMapMutex_;
    std::mutex cancelableCallbackMapMutex_;
    SharedImageMap sharedImageMap_;
    size_t sharedImageTotalSize_;
    std::atomic<int32_t> sharedImageCacheThreshold_;
    ProviderMapToReload providerMapToReload_;
    std::function<void()> clearImageDataCallback_;
    std::unordered_map<std::string, CancelableCallback<void()>> cancelableCallbackMap_;
    WeakPtr<TaskExecutor> taskExecutor_;
};

class ImageProviderLoader : public virtual AceType {
    DECLARE_ACE_TYPE(ImageProviderLoader, AceType);

public:
    ~ImageProviderLoader() override = default;
    virtual void UpdateData(const std::string& uri, const std::vector<uint8_t>& memData) = 0;

protected:
    ImageProviderLoader() = default;

    ACE_DISALLOW_COPY_AND_MOVE(ImageProviderLoader);
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_BASE_RESOURCE_SHARED_IMAGE_MANAGER_H

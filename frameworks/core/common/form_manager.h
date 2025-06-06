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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMMON_FORM_MANAGER_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMMON_FORM_MANAGER_H

#include <mutex>
#include <unordered_map>

#include "base/utils/macros.h"
#include "base/utils/noncopyable.h"
#include "base/utils/singleton.h"
#include "core/components/form/resource/form_utils.h"
#include "core/components/form/sub_container.h"

namespace OHOS::Ace {

class ACE_EXPORT FormManager final : public Singleton<FormManager> {
    DECLARE_SINGLETON(FormManager);

public:
    void AddSubContainer(int64_t formId, const RefPtr<SubContainer>& subContainer);
    void RemoveSubContainer(int64_t formId);
    RefPtr<SubContainer> GetSubContainer(int64_t formId);
    void SetFormUtils(const std::shared_ptr<FormUtils>& formUtils);
    std::shared_ptr<FormUtils> GetFormUtils();
    void NotifyIsSizeChangeByRotate(
        bool isRotate, const std::shared_ptr<Rosen::RSTransaction>& rsTransaction);
    std::weak_ptr<Rosen::RSTransaction> GetRSTransaction()
    {
        return rsTransaction_;
    }
    bool IsSizeChangeByRotate()
    {
        return isRotate_;
    }

private:
    std::mutex mutex_;
    std::unordered_map<int64_t, RefPtr<SubContainer>> subContainerMap_;
    std::mutex nonmatchedContainerMutex_;
    std::unordered_map<std::string, RefPtr<SubContainer>> nonmatchedContainerMap_;
    std::mutex formUtilsMutex_;
    std::shared_ptr<FormUtils> formUtils_;
    std::weak_ptr<Rosen::RSTransaction> rsTransaction_;
    bool isRotate_ = false;
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMMON_FORM_MANAGER_H

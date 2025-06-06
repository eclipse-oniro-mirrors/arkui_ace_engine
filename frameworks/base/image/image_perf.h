/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_BASE_IMAGE_ACE_IMAGE_PERF_H
#define FOUNDATION_ACE_FRAMEWORKS_BASE_IMAGE_ACE_IMAGE_PERF_H

#include <cstdint>
#include <memory>

#include "ui/base/macros.h"

namespace OHOS::Ace {

class ACE_FORCE_EXPORT ImagePerf {
public:
    virtual void StartRecordImageLoadStat(int64_t id) {}
    virtual void EndRecordImageLoadStat(int64_t id, const std::string& imageType, std::pair<int, int> size, int state)
    {}

    virtual ~ImagePerf() = default;

    static ImagePerf* GetPerfMonitor();
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_BASE_IMAGE_ACE_IMAGE_PERF_H

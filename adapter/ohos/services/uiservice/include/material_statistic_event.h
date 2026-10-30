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

#ifndef OHOS_ACE_UI_SERVICE_MATERIAL_STATISTIC_EVENT_H
#define OHOS_ACE_UI_SERVICE_MATERIAL_STATISTIC_EVENT_H

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include "base/log/material_statistic_event_info.h"

namespace OHOS::Ace {

struct MaterialStatSummary {
    std::string navName;
    int32_t tabIndex = -1;
    std::string pageName;
    double minArea = std::numeric_limits<double>::max();
    double maxArea = 0.0;
    int32_t count = 0;

    bool Matches(const RichMaterialNodeStatData& data) const
    {
        return navName == data.navName && tabIndex == data.tabIndex && pageName == data.pageName;
    }
};

struct OverlayStatSummary {
    std::string tag;
    double minArea = std::numeric_limits<double>::max();
    double maxArea = 0.0;
    int32_t count = 0;

    bool Matches(const RichOverlayMaterialAreaData& data) const
    {
        return tag == data.tag;
    }
};

struct BundleStatSummary {
    std::string bundleName;
    std::vector<MaterialStatSummary> materialStats;
    std::vector<OverlayStatSummary> overlayStats;
};

struct MaterialStatisticsFields {
    std::vector<std::string> bundleNames;
    std::vector<std::string> pageInfos;
    std::vector<std::string> navInfos;
    std::vector<std::string> tabsInfos;
};

class MaterialStatisticEvent {
public:
    MaterialStatisticEvent() = default;
    ~MaterialStatisticEvent() = default;

    void ReportEvent();
    void Append(const MaterialStatisticEventInfo& info);
    size_t GetPendingCount() const
    {
        return summaries_.size();
    }
    size_t GetTotalSummaryCount() const;

private:
    static MaterialStatisticsFields BuildFieldsFromStatistics(const std::vector<BundleStatSummary>& summaries);

    std::vector<BundleStatSummary> summaries_;
};

} // namespace OHOS::Ace

#endif // OHOS_ACE_UI_SERVICE_MATERIAL_STATISTIC_EVENT_H

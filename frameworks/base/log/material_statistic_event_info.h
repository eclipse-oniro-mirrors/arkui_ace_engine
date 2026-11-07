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

#ifndef FOUNDATION_ACE_FRAMEWORKS_BASE_LOG_MATERIAL_STATISTIC_EVENT_INFO_H
#define FOUNDATION_ACE_FRAMEWORKS_BASE_LOG_MATERIAL_STATISTIC_EVENT_INFO_H

#include <string>
#include <vector>

namespace OHOS::Ace {

enum class MaterialStatisticEventType {
    MATERIAL_NODE_STAT = 0,
    MATERIAL_OVERLAY_AREA = 1,
};

struct RichMaterialNodeEntry {
    int32_t nodeId = 0;
    std::string tag;
    int32_t containerId = -1;
    double width = 0.0;
    double height = 0.0;
    double area = 0.0;
    bool inTitleBar = false;
    bool inBottomTabBar = false;
};

struct RichMaterialNodeStatData {
    int32_t containerId = -1;
    std::string containerType;
    std::vector<RichMaterialNodeEntry> entries;
    int32_t navCount = 0;
    int32_t tabsCount = 0;
    int32_t pageCount = 0;
    double screenWidth = 0.0;
    double screenHeight = 0.0;
    double areaPercent = 0.0;
    std::string navName;
    std::string pageName;
    int32_t tabIndex = -1;
};

struct RichOverlayMaterialAreaData {
    int32_t nodeId = 0;
    std::string tag;
    double width = 0.0;
    double height = 0.0;
    double area = 0.0;
    double screenWidth = 0.0;
    double screenHeight = 0.0;
    double areaPercent = 0.0;
};

struct MaterialStatisticEventInfo {
    MaterialStatisticEventType type = MaterialStatisticEventType::MATERIAL_NODE_STAT;
    std::string bundleName;
    RichMaterialNodeStatData materialNodeStat;
    RichOverlayMaterialAreaData overlayMaterialArea;

    double GetAreaPercent() const
    {
        return type == MaterialStatisticEventType::MATERIAL_NODE_STAT ? materialNodeStat.areaPercent
                                                                      : overlayMaterialArea.areaPercent;
    }

    friend bool operator<(const MaterialStatisticEventInfo& a, const MaterialStatisticEventInfo& b)
    {
        return a.GetAreaPercent() > b.GetAreaPercent();
    }
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_BASE_LOG_MATERIAL_STATISTIC_EVENT_INFO_H

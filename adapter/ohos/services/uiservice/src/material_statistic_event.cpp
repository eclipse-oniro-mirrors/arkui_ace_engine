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

#include "material_statistic_event.h"

#include <algorithm>
#include <string>

#include "hisysevent.h"
#include "ui_service_hilog.h"

namespace OHOS::Ace {
namespace {
constexpr char EVENT_NAME_MATERIAL_NODE_STAT[] = "MATERIAL_NODE_STAT";
constexpr char EVENT_NAME_MATERIAL_STAT_JSON[] = "MATERIAL_PAGE_INFO";
static constexpr size_t MAX_SUMMARY_COUNT = 500;
} // namespace

void MaterialStatisticEvent::Append(const MaterialStatisticEventInfo& info)
{
    BundleStatSummary* bundle = nullptr;
    for (auto& bundleStat : summaries_) {
        if (bundleStat.bundleName == info.bundleName) {
            bundle = &bundleStat;
            break;
        }
    }
    if (!bundle) {
        summaries_.push_back({});
        bundle = &summaries_.back();
        bundle->bundleName = info.bundleName;
    }

    if (info.type == MaterialStatisticEventType::MATERIAL_NODE_STAT) {
        const auto& materialData = info.materialNodeStat;
        MaterialStatSummary* materialSummary = nullptr;
        for (auto& matStat : bundle->materialStats) {
            if (matStat.Matches(materialData)) {
                materialSummary = &matStat;
                break;
            }
        }
        if (!materialSummary) {
            bundle->materialStats.push_back({});
            materialSummary = &bundle->materialStats.back();
            materialSummary->navName = materialData.navName;
            materialSummary->tabIndex = materialData.tabIndex;
            materialSummary->pageName = materialData.pageName;
        }
        materialSummary->minArea = std::min(materialSummary->minArea, materialData.areaPercent);
        materialSummary->maxArea = std::max(materialSummary->maxArea, materialData.areaPercent);
        materialSummary->count++;
    } else if (info.type == MaterialStatisticEventType::MATERIAL_OVERLAY_AREA) {
        const auto& overlayData = info.overlayMaterialArea;
        OverlayStatSummary* overlaySummary = nullptr;
        for (auto& overlayStat : bundle->overlayStats) {
            if (overlayStat.Matches(overlayData)) {
                overlaySummary = &overlayStat;
                break;
            }
        }
        if (!overlaySummary) {
            bundle->overlayStats.push_back({});
            overlaySummary = &bundle->overlayStats.back();
            overlaySummary->tag = overlayData.tag;
        }
        overlaySummary->minArea = std::min(overlaySummary->minArea, overlayData.areaPercent);
        overlaySummary->maxArea = std::max(overlaySummary->maxArea, overlayData.areaPercent);
        overlaySummary->count++;
    }

    if (GetTotalSummaryCount() >= MAX_SUMMARY_COUNT) {
        ReportEvent();
    }
}

size_t MaterialStatisticEvent::GetTotalSummaryCount() const
{
    size_t count = 0;
    for (const auto& bundle : summaries_) {
        count += bundle.materialStats.size() + bundle.overlayStats.size();
    }
    return count;
}

void MaterialStatisticEvent::ReportEvent()
{
    auto json = BuildStatisticsJson();
    HiSysEventWrite(OHOS::HiviewDFX::HiSysEvent::Domain::ACE, EVENT_NAME_MATERIAL_NODE_STAT,
        OHOS::HiviewDFX::HiSysEvent::EventType::STATISTIC, EVENT_NAME_MATERIAL_STAT_JSON, json);
    summaries_.clear();
}

std::string MaterialStatisticEvent::BuildJsonFromStatistics(const std::vector<BundleStatSummary>& summaries)
{
    std::ostringstream oss;
    oss << "[";
    bool firstBundle = true;
    for (const auto& bundle : summaries) {
        if (!firstBundle) {
            oss << ",";
        }
        firstBundle = false;
        oss << "{\"bundleName\":\"" << bundle.bundleName << "\",\"pageInfo\":[";
        bool firstInfo = true;
        for (const auto& matStat : bundle.materialStats) {
            if (!firstInfo) {
                oss << ",";
            }
            firstInfo = false;
            oss << "{\"type\":\"materialNodeStat\","
                << "\"url\":\"" << matStat.navName << "|" << matStat.tabIndex << "|" << matStat.pageName << "\","
                << "\"area\":{\"min\":" << matStat.minArea << ",\"max\":" << matStat.maxArea << "},"
                << "\"count\":" << matStat.count << "}";
        }
        for (const auto& overlayStat : bundle.overlayStats) {
            if (!firstInfo) {
                oss << ",";
            }
            firstInfo = false;
            oss << "{\"type\":\"overlay\","
                << "\"tag\":\"" << overlayStat.tag << "\","
                << "\"area\":{\"min\":" << overlayStat.minArea << ",\"max\":" << overlayStat.maxArea << "},"
                << "\"count\":" << overlayStat.count << "}";
        }
        oss << "]}";
    }
    oss << "]";
    return oss.str();
}

std::string MaterialStatisticEvent::BuildStatisticsJson() const
{
    return BuildJsonFromStatistics(summaries_);
}

} // namespace OHOS::Ace

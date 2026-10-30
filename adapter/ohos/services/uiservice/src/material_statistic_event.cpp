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
#include <set>
#include <string>

#include "hisysevent.h"
#include "ui_service_hilog.h"

namespace OHOS::Ace {
namespace {
constexpr char EVENT_NAME_MATERIAL_NODE_STAT[] = "MATERIAL_NODE_STAT";
constexpr char KEY_BUNDLE_NAME[] = "BUNDLE_NAME";
constexpr char KEY_PAGE_INFO[] = "PAGE_INFO";
constexpr char KEY_NAV_INFO[] = "NAV_INFO";
constexpr char KEY_TABS_INFO[] = "TABS_INFO";
static constexpr size_t MAX_SUMMARY_COUNT = 500;
static constexpr size_t MAX_BUNDLE_COUNT = 100;

std::string JoinStringList(const std::vector<std::string>& parts, char sep)
{
    std::string result;
    for (size_t i = 0; i < parts.size(); ++i) {
        if (i > 0) {
            result += sep;
        }
        result += parts[i];
    }
    return result;
}
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
        if (summaries_.size() >= MAX_BUNDLE_COUNT || GetTotalSummaryCount() >= MAX_SUMMARY_COUNT) {
            return;
        }
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
    if (summaries_.empty()) {
        return;
    }
    auto fields = BuildFieldsFromStatistics(summaries_);
    HiSysEventWrite(OHOS::HiviewDFX::HiSysEvent::Domain::ACE, EVENT_NAME_MATERIAL_NODE_STAT,
        OHOS::HiviewDFX::HiSysEvent::EventType::STATISTIC, KEY_BUNDLE_NAME, fields.bundleNames, KEY_PAGE_INFO,
        fields.pageInfos, KEY_NAV_INFO, fields.navInfos, KEY_TABS_INFO, fields.tabsInfos);
    summaries_.clear();
}

MaterialStatisticsFields MaterialStatisticEvent::BuildFieldsFromStatistics(
    const std::vector<BundleStatSummary>& summaries)
{
    MaterialStatisticsFields fields;
    for (const auto& bundle : summaries) {
        fields.bundleNames.push_back(bundle.bundleName);

        std::vector<std::string> pageParts;
        std::set<std::string> seenPages;
        for (const auto& matStat : bundle.materialStats) {
            if (matStat.pageName.empty() || !seenPages.insert(matStat.pageName).second) {
                continue;
            }
            pageParts.push_back(matStat.pageName);
        }
        std::set<std::string> seenTags;
        for (const auto& overlayStat : bundle.overlayStats) {
            if (overlayStat.tag.empty() || !seenTags.insert(overlayStat.tag).second) {
                continue;
            }
            pageParts.push_back(overlayStat.tag);
        }
        fields.pageInfos.push_back(JoinStringList(pageParts, ';'));

        std::vector<std::string> navParts;
        std::set<std::string> seenNav;
        for (const auto& matStat : bundle.materialStats) {
            if (matStat.navName.empty()) {
                continue;
            }
            std::string entry = matStat.pageName + ':' + matStat.navName;
            if (seenNav.insert(entry).second) {
                navParts.push_back(entry);
            }
        }
        fields.navInfos.push_back(JoinStringList(navParts, ';'));

        std::vector<std::string> tabsParts;
        std::set<std::string> seenTabs;
        for (const auto& matStat : bundle.materialStats) {
            if (matStat.tabIndex < 0) {
                continue;
            }
            std::string entry = matStat.pageName + ':' + std::to_string(matStat.tabIndex);
            if (seenTabs.insert(entry).second) {
                tabsParts.push_back(entry);
            }
        }
        fields.tabsInfos.push_back(JoinStringList(tabsParts, ';'));
    }
    return fields;
}

} // namespace OHOS::Ace

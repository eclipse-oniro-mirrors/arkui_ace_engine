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

#include "ui_service_statistic_event.h"

namespace OHOS::Ace {
namespace {
constexpr int32_t MAX_ENTRIES = 1024;

#define WRITE_OR_RET(parcel, method, ...)  \
    do {                                   \
        if (!(parcel.method(__VA_ARGS__))) \
            return false;                  \
    } while (0)

#define READ_OR_RET(parcel, method, ...)     \
    do {                                     \
        if (!(parcel.method(__VA_ARGS__))) { \
            delete result;                   \
            return nullptr;                  \
        }                                    \
    } while (0)

bool WriteNodeStat(Parcel& parcel, const RichMaterialNodeStatData& d)
{
    WRITE_OR_RET(parcel, WriteInt32, d.containerId);
    WRITE_OR_RET(parcel, WriteString, d.containerType);
    std::vector<int32_t> inTitleBarArr;
    std::vector<int32_t> inBottomTabBarArr;
    std::vector<int32_t> nodeIds;
    std::vector<std::string> tags;
    std::vector<double> widths;
    std::vector<double> heights;
    std::vector<double> areas;
    for (const auto& entry : d.entries) {
        inTitleBarArr.push_back(static_cast<int32_t>(entry.inTitleBar));
        inBottomTabBarArr.push_back(static_cast<int32_t>(entry.inBottomTabBar));
        nodeIds.push_back(entry.nodeId);
        tags.push_back(entry.tag);
        widths.push_back(entry.width);
        heights.push_back(entry.height);
        areas.push_back(entry.area);
    }
    WRITE_OR_RET(parcel, WriteInt32Vector, inTitleBarArr);
    WRITE_OR_RET(parcel, WriteInt32Vector, inBottomTabBarArr);
    WRITE_OR_RET(parcel, WriteInt32Vector, nodeIds);
    WRITE_OR_RET(parcel, WriteStringVector, tags);
    WRITE_OR_RET(parcel, WriteDoubleVector, widths);
    WRITE_OR_RET(parcel, WriteDoubleVector, heights);
    WRITE_OR_RET(parcel, WriteDoubleVector, areas);
    WRITE_OR_RET(parcel, WriteInt32, d.navCount);
    WRITE_OR_RET(parcel, WriteInt32, d.tabsCount);
    WRITE_OR_RET(parcel, WriteInt32, d.pageCount);
    WRITE_OR_RET(parcel, WriteDouble, d.screenWidth);
    WRITE_OR_RET(parcel, WriteDouble, d.screenHeight);
    WRITE_OR_RET(parcel, WriteDouble, d.areaPercent);
    WRITE_OR_RET(parcel, WriteString, d.navName);
    WRITE_OR_RET(parcel, WriteString, d.pageName);
    WRITE_OR_RET(parcel, WriteInt32, d.tabIndex);
    return true;
}

bool WriteOverlayArea(Parcel& parcel, const RichOverlayMaterialAreaData& d)
{
    WRITE_OR_RET(parcel, WriteInt32, d.nodeId);
    WRITE_OR_RET(parcel, WriteString, d.tag);
    WRITE_OR_RET(parcel, WriteDouble, d.width);
    WRITE_OR_RET(parcel, WriteDouble, d.height);
    WRITE_OR_RET(parcel, WriteDouble, d.area);
    WRITE_OR_RET(parcel, WriteDouble, d.screenWidth);
    WRITE_OR_RET(parcel, WriteDouble, d.screenHeight);
    WRITE_OR_RET(parcel, WriteDouble, d.areaPercent);
    return true;
}

MaterialStatisticEventInfoParcel* ReadNodeStat(Parcel& parcel, MaterialStatisticEventInfoParcel* result)
{
    auto& d = result->info_.materialNodeStat;
    READ_OR_RET(parcel, ReadInt32, d.containerId);
    READ_OR_RET(parcel, ReadString, d.containerType);
    std::vector<int32_t> inTitleBarArr;
    std::vector<int32_t> inBottomTabBarArr;
    std::vector<int32_t> nodeIds;
    std::vector<std::string> tags;
    std::vector<double> widths;
    std::vector<double> heights;
    std::vector<double> areas;
    READ_OR_RET(parcel, ReadInt32Vector, &inTitleBarArr);
    READ_OR_RET(parcel, ReadInt32Vector, &inBottomTabBarArr);
    READ_OR_RET(parcel, ReadInt32Vector, &nodeIds);
    READ_OR_RET(parcel, ReadStringVector, &tags);
    READ_OR_RET(parcel, ReadDoubleVector, &widths);
    READ_OR_RET(parcel, ReadDoubleVector, &heights);
    READ_OR_RET(parcel, ReadDoubleVector, &areas);
    READ_OR_RET(parcel, ReadInt32, d.navCount);
    READ_OR_RET(parcel, ReadInt32, d.tabsCount);
    READ_OR_RET(parcel, ReadInt32, d.pageCount);
    READ_OR_RET(parcel, ReadDouble, d.screenWidth);
    READ_OR_RET(parcel, ReadDouble, d.screenHeight);
    READ_OR_RET(parcel, ReadDouble, d.areaPercent);
    READ_OR_RET(parcel, ReadString, d.navName);
    READ_OR_RET(parcel, ReadString, d.pageName);
    READ_OR_RET(parcel, ReadInt32, d.tabIndex);
    size_t n = nodeIds.size();
    if (n > MAX_ENTRIES) {
        return nullptr;
    }
    d.entries.reserve(n);
    for (size_t i = 0; i < n; ++i) {
        RichMaterialNodeEntry entry;
        entry.nodeId = nodeIds[i];
        entry.tag = i < tags.size() ? tags[i] : "";
        entry.width = i < widths.size() ? widths[i] : 0.0;
        entry.height = i < heights.size() ? heights[i] : 0.0;
        entry.area = i < areas.size() ? areas[i] : 0.0;
        entry.inTitleBar = i < inTitleBarArr.size() ? (inTitleBarArr[i] != 0) : false;
        entry.inBottomTabBar = i < inBottomTabBarArr.size() ? (inBottomTabBarArr[i] != 0) : false;
        d.entries.emplace_back(std::move(entry));
    }
    return result;
}

MaterialStatisticEventInfoParcel* ReadOverlayArea(Parcel& parcel, MaterialStatisticEventInfoParcel* result)
{
    auto& d = result->info_.overlayMaterialArea;
    READ_OR_RET(parcel, ReadInt32, d.nodeId);
    READ_OR_RET(parcel, ReadString, d.tag);
    READ_OR_RET(parcel, ReadDouble, d.width);
    READ_OR_RET(parcel, ReadDouble, d.height);
    READ_OR_RET(parcel, ReadDouble, d.area);
    READ_OR_RET(parcel, ReadDouble, d.screenWidth);
    READ_OR_RET(parcel, ReadDouble, d.screenHeight);
    READ_OR_RET(parcel, ReadDouble, d.areaPercent);
    return result;
}
} // namespace

AppInfoParcel::AppInfoParcel(const std::string& bundleName)
{
    bundleName_ = bundleName;
}

const std::string& AppInfoParcel::GetBundleName() const
{
    return bundleName_;
}

bool AppInfoParcel::Marshalling(Parcel& parcel) const
{
    if (!parcel.WriteString(bundleName_)) {
        return false;
    }
    return true;
}

AppInfoParcel *AppInfoParcel::Unmarshalling(Parcel& parcel)
{
    std::string bundleName;
    if (!parcel.ReadString(bundleName)) {
        return nullptr;
    }
    AppInfoParcel* appInfoParcel = new AppInfoParcel(bundleName);
    return appInfoParcel;
}

StatisticEventInfoParcel::StatisticEventInfoParcel(const std::string& eventName,
    const std::string& subEventName, int32_t eventCount)
{
    eventName_ = eventName;
    subEventName_ = subEventName;
    eventCount_ = eventCount;
}

bool StatisticEventInfoParcel::Marshalling(Parcel& parcel) const
{
    if (!parcel.WriteString(eventName_)) {
        return false;
    }
    if (!parcel.WriteString(subEventName_)) {
        return false;
    }
    if (!parcel.WriteInt32(eventCount_)) {
        return false;
    }
    return true;
}

StatisticEventInfoParcel *StatisticEventInfoParcel::Unmarshalling(Parcel& parcel)
{
    std::string eventName;
    if (!parcel.ReadString(eventName)) {
        return nullptr;
    }
    std::string subEventName;
    if (!parcel.ReadString(subEventName)) {
        return nullptr;
    }
    int32_t eventCount;
    if (!parcel.ReadInt32(eventCount)) {
        return nullptr;
    }
    StatisticEventInfoParcel* statisticEventParcel = new StatisticEventInfoParcel(eventName, subEventName, eventCount);
    return statisticEventParcel;
}

const std::string& StatisticEventInfoParcel::GetEventName() const
{
    return eventName_;
}

const std::string& StatisticEventInfoParcel::GetSubEventName() const
{
    return subEventName_;
}

int32_t StatisticEventInfoParcel::GetEventCount() const
{
    return eventCount_;
}

bool MaterialStatisticEventInfoParcel::Marshalling(Parcel& parcel) const
{
    WRITE_OR_RET(parcel, WriteInt32, static_cast<int32_t>(info_.type));
    if (info_.type == MaterialStatisticEventType::MATERIAL_NODE_STAT) {
        return WriteNodeStat(parcel, info_.materialNodeStat);
    }
    if (info_.type == MaterialStatisticEventType::MATERIAL_OVERLAY_AREA) {
        return WriteOverlayArea(parcel, info_.overlayMaterialArea);
    }
    return false;
}

MaterialStatisticEventInfoParcel* MaterialStatisticEventInfoParcel::Unmarshalling(Parcel& parcel)
{
    int32_t type = 0;
    if (!parcel.ReadInt32(type)) {
        return nullptr;
    }
    auto* result = new MaterialStatisticEventInfoParcel();
    result->info_.type = static_cast<MaterialStatisticEventType>(type);
    if (type == static_cast<int32_t>(MaterialStatisticEventType::MATERIAL_NODE_STAT)) {
        return ReadNodeStat(parcel, result);
    }
    if (type == static_cast<int32_t>(MaterialStatisticEventType::MATERIAL_OVERLAY_AREA)) {
        return ReadOverlayArea(parcel, result);
    }
    delete result;
    return nullptr;
}

#undef WRITE_OR_RET
#undef READ_OR_RET
} // namespace OHOS::Ace

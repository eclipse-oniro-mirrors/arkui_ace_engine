/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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
#include "test/mock/frameworks/core/common/mock_resource_adapter_v2.h"

#include "core/components/theme/resource_adapter.h"
#include <map>
#include <mutex>

namespace OHOS::Ace {
namespace {
std::map<uint32_t, Color> g_colorMap;
constexpr uint32_t FAKE_RES_ID = 1234;
constexpr uint32_t FAKE_RES_ID_2 = 1235;
std::map<uint32_t, Dimension> MakeDefaultDimensionResources()
{
    return {
        { FAKE_RES_ID, Dimension(MOCK_DIMENSION_DEFAULT, DimensionUnit::PX) },
        { FAKE_RES_ID_2, Dimension(MOCK_DIMENSION_DEFAULT, DimensionUnit::PX) },
    };
}
std::map<uint32_t, Dimension> g_dimensionMap = MakeDefaultDimensionResources();
std::map<uint32_t, std::string> g_strMap;
std::map<uint32_t, std::vector<std::string>> g_strsMap;
std::map<uint32_t, double> g_doubleMap;
std::map<uint32_t, int32_t> g_int32Map;
std::map<std::string, Color> g_colorNameMap;
std::map<std::string, Dimension> g_dimensionNameMap;
std::map<std::string, std::string> g_strNameMap;
std::map<std::string, std::vector<std::string>> g_strsNameMap;
std::map<std::string, double> g_doubleNameMap;
std::map<std::string, int32_t> g_int32NameMap;
std::mutex g_mapMutex;
int32_t g_resourceAdapterInstanceId = -1;
} // namespace

RefPtr<ResourceAdapter> ResourceAdapter::Create()
{
    return AceType::MakeRefPtr<MockResourceAdapterV2>();
}

RefPtr<ResourceAdapter> ResourceAdapter::CreateNewResourceAdapter(
    const std::string& bundleName, const std::string& moduleName, int32_t& actualInstanceId)
{
    if (bundleName.empty() && moduleName.empty()) {
        return nullptr;
    }
    std::lock_guard<std::mutex> lock(g_mapMutex);
    if (g_resourceAdapterInstanceId != -1) {
        actualInstanceId = g_resourceAdapterInstanceId;
    }
    return AceType::MakeRefPtr<MockResourceAdapterV2>();
}

bool MockResourceAdapterV2::GetMockResourceData(uint32_t id, Color& data)
{
    std::lock_guard<std::mutex> lock(g_mapMutex);
    if (g_colorMap.count(id) == 0) {
        return false;
    }
    data = g_colorMap[id];
    return true;
}

bool MockResourceAdapterV2::GetMockResourceData(uint32_t id, Dimension& data)
{
    std::lock_guard<std::mutex> lock(g_mapMutex);
    if (g_dimensionMap.count(id) == 0) {
        return false;
    }
    data = g_dimensionMap[id];
    return true;
}

bool MockResourceAdapterV2::GetMockResourceData(uint32_t id, std::string& data)
{
    std::lock_guard<std::mutex> lock(g_mapMutex);
    if (g_strMap.count(id) == 0) {
        return false;
    }
    data = g_strMap[id];
    return true;
}
bool MockResourceAdapterV2::GetMockResourceData(uint32_t id, std::vector<std::string>& data)
{
    std::lock_guard<std::mutex> lock(g_mapMutex);
    if (g_strsMap.count(id) == 0) {
        return false;
    }
    data = g_strsMap[id];
    return true;
}
bool MockResourceAdapterV2::GetMockResourceData(uint32_t id, double& data)
{
    std::lock_guard<std::mutex> lock(g_mapMutex);
    if (g_doubleMap.count(id) == 0) {
        return false;
    }
    data = g_doubleMap[id];
    return true;
}
bool MockResourceAdapterV2::GetMockResourceData(uint32_t id, int32_t& data)
{
    std::lock_guard<std::mutex> lock(g_mapMutex);
    if (g_int32Map.count(id) == 0) {
        return false;
    }
    data = g_int32Map[id];
    return true;
}

template <typename T>
bool LookupByName(const std::map<std::string, T>& map, const std::string& name, T& data)
{
    std::lock_guard<std::mutex> lock(g_mapMutex);
    auto it = map.find(name);
    if (it == map.end()) {
        return false;
    }
    data = it->second;
    return true;
}

bool MockResourceAdapterV2::GetMockResourceDataByName(const std::string& name, Color& data)
{
    return LookupByName(g_colorNameMap, name, data);
}

bool MockResourceAdapterV2::GetMockResourceDataByName(const std::string& name, Dimension& data)
{
    return LookupByName(g_dimensionNameMap, name, data);
}

bool MockResourceAdapterV2::GetMockResourceDataByName(const std::string& name, std::string& data)
{
    return LookupByName(g_strNameMap, name, data);
}

bool MockResourceAdapterV2::GetMockResourceDataByName(const std::string& name, std::vector<std::string>& data)
{
    return LookupByName(g_strsNameMap, name, data);
}

bool MockResourceAdapterV2::GetMockResourceDataByName(const std::string& name, double& data)
{
    return LookupByName(g_doubleNameMap, name, data);
}

bool MockResourceAdapterV2::GetMockResourceDataByName(const std::string& name, int32_t& data)
{
    return LookupByName(g_int32NameMap, name, data);
}

void AddMockResourceData(uint32_t id, const Color& data)
{
    std::lock_guard<std::mutex> lock(g_mapMutex);
    g_colorMap[id] = data;
}

void AddMockResourceData(uint32_t id, const Dimension& data)
{
    std::lock_guard<std::mutex> lock(g_mapMutex);
    g_dimensionMap[id] = data;
}

void AddMockResourceData(uint32_t id, const std::string& data)
{
    std::lock_guard<std::mutex> lock(g_mapMutex);
    g_strMap[id] = data;
}
void AddMockResourceData(uint32_t id, const std::vector<std::string>& data)
{
    std::lock_guard<std::mutex> lock(g_mapMutex);
    g_strsMap[id] = data;
}

void AddMockResourceData(uint32_t id, const double& data)
{
    std::lock_guard<std::mutex> lock(g_mapMutex);
    g_doubleMap[id] = data;
}

void AddMockResourceData(uint32_t id, const int32_t& data)
{
    std::lock_guard<std::mutex> lock(g_mapMutex);
    g_int32Map[id] = data;
}

void AddMockResourceDataByName(const std::string& name, const Color& data)
{
    std::lock_guard<std::mutex> lock(g_mapMutex);
    g_colorNameMap[name] = data;
}

void AddMockResourceDataByName(const std::string& name, const Dimension& data)
{
    std::lock_guard<std::mutex> lock(g_mapMutex);
    g_dimensionNameMap[name] = data;
}

void AddMockResourceDataByName(const std::string& name, const std::string& data)
{
    std::lock_guard<std::mutex> lock(g_mapMutex);
    g_strNameMap[name] = data;
}

void AddMockResourceDataByName(const std::string& name, const std::vector<std::string>& data)
{
    std::lock_guard<std::mutex> lock(g_mapMutex);
    g_strsNameMap[name] = data;
}

void AddMockResourceDataByName(const std::string& name, const double& data)
{
    std::lock_guard<std::mutex> lock(g_mapMutex);
    g_doubleNameMap[name] = data;
}

void AddMockResourceDataByName(const std::string& name, const int32_t& data)
{
    std::lock_guard<std::mutex> lock(g_mapMutex);
    g_int32NameMap[name] = data;
}

void ResetMockResourceData()
{
    std::lock_guard<std::mutex> lock(g_mapMutex);
    g_colorMap.clear();
    g_dimensionMap.clear();
    g_strMap.clear();
    g_strsMap.clear();
    g_doubleMap.clear();
    g_int32Map.clear();
    g_colorNameMap.clear();
    g_dimensionNameMap.clear();
    g_strNameMap.clear();
    g_strsNameMap.clear();
    g_doubleNameMap.clear();
    g_int32NameMap.clear();
    g_dimensionMap = MakeDefaultDimensionResources();
    g_resourceAdapterInstanceId = -1;
}

void SetMockResourceAdapterInstanceId(int32_t instanceId)
{
    std::lock_guard<std::mutex> lock(g_mapMutex);
    g_resourceAdapterInstanceId = instanceId;
}
} // namespace OHOS::Ace

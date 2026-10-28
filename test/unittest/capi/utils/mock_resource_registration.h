/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ARKUI_ACE_ENGINE_TEST_UNITTEST_CAPI_UTILS_MOCK_RESOURCE_REGISTRATION_H
#define FOUNDATION_ARKUI_ACE_ENGINE_TEST_UNITTEST_CAPI_UTILS_MOCK_RESOURCE_REGISTRATION_H

#include <algorithm>
#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

#include "core/components/theme/theme_style.h"
#include "test/mock/frameworks/core/common/mock_resource_adapter_v2.h"

namespace OHOS::Ace::NG {
constexpr int32_t DECIMAL_RADIX = 10;

inline bool IsDecimalDigit(char c)
{
    return c >= '0' && c <= '9';
}
inline bool TryParseAsResourceId(const std::string& key, uint32_t& id)
{
    if (key.empty() || !std::all_of(key.begin(), key.end(), IsDecimalDigit)) {
        return false;
    }
    errno = 0;
    const unsigned long long parsed = std::strtoull(key.c_str(), nullptr, DECIMAL_RADIX);
    if (errno == ERANGE || parsed > UINT32_MAX) {
        return false;
    }
    id = static_cast<uint32_t>(parsed);
    return true;
}
template <typename T>
inline void AddMockResourceById(uint32_t id, const T& value)
{
    if constexpr (std::is_same_v<T, Color> || std::is_same_v<T, Dimension> ||
        std::is_same_v<T, int32_t> || std::is_same_v<T, double>) {
        AddMockResourceData(id, value);
    }
}
inline void AddMockResourceById(uint32_t id, InternalResource::ResourceId resId)
{
    const int32_t value = static_cast<int32_t>(resId);
    AddMockResourceData(id, value);
    AddMockResourceData(id, static_cast<double>(value));
}

inline void AddMockResourceById(uint32_t id, uint32_t value)
{
    AddMockResourceData(id, static_cast<int32_t>(value));
    AddMockResourceData(id, static_cast<double>(value));
}

inline void AddMockResourceById(uint32_t id, const std::string& value)
{
    AddMockResourceData(id, value);
    AddMockResourceData(id, std::vector<std::string>{ value });
}

inline void RegisterResourceById(uint32_t id, const ResRawValue& value)
{
    std::visit([id](auto&& item) { AddMockResourceById(id, item); }, value);
}

template <typename T>
inline void AddMockResourceByName(const std::string& name, const T& value)
{
    if constexpr (std::is_same_v<T, Color> || std::is_same_v<T, Dimension> ||
        std::is_same_v<T, int32_t> || std::is_same_v<T, double>) {
        AddMockResourceDataByName(name, value);
    }
}
inline void AddMockResourceByName(const std::string& name, InternalResource::ResourceId resId)
{
    const int32_t value = static_cast<int32_t>(resId);
    AddMockResourceDataByName(name, value);
    AddMockResourceDataByName(name, static_cast<double>(value));
}

inline void AddMockResourceByName(const std::string& name, uint32_t value)
{
    AddMockResourceDataByName(name, static_cast<int32_t>(value));
    AddMockResourceDataByName(name, static_cast<double>(value));
}

inline void AddMockResourceByName(const std::string& name, const std::string& value)
{
    AddMockResourceDataByName(name, value);
    AddMockResourceDataByName(name, std::vector<std::string>{ value });
}

inline void RegisterResourceByName(const std::string& name, const ResRawValue& value)
{
    std::visit([&name](auto&& item) { AddMockResourceByName(name, item); }, value);
}
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ARKUI_ACE_ENGINE_TEST_UNITTEST_CAPI_UTILS_MOCK_RESOURCE_REGISTRATION_H

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
#include <cstdint>
#include <cstdlib>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

#include "core/components/theme/theme_style.h"
#include "test/mock/frameworks/core/common/mock_resource_adapter_v2.h"

namespace OHOS::Ace::NG {
// CAPI 资源解析（Converter 资源路径）从 MockResourceAdapterV2 读取数据，而
// MockThemeStyle 只服务主题查找，因此资源注册必须双写两侧。modifier 与
// accessor 两套测试基类共用本文件，避免注册逻辑复制后各自漂移。

// 十进制基数（抽自字面量，避免魔法数）。
constexpr int32_t DECIMAL_RADIX = 10;

inline bool IsDecimalDigit(char c)
{
    return c >= '0' && c <= '9';
}

// 资源 id 键为纯十进制数字（对应 AddResource(int64_t) 的 std::to_string 正向输出）；
// 含 '-' 或其它非数字字符的键一律走按名注册路径。
inline bool TryParseAsResourceId(const std::string& key, uint32_t& id)
{
    if (key.empty() || !std::all_of(key.begin(), key.end(), IsDecimalDigit)) {
        return false;
    }
    id = static_cast<uint32_t>(std::strtoll(key.c_str(), nullptr, DECIMAL_RADIX));
    return true;
}

// uint32_t/string 变体按消费侧查表要求双写（int32+double / string+vector<string>）；
// 其余 ResRawValue 变体（ResourceId/ThemePtr 等）no-op 跳过，与原 if-constexpr 链一致。
template <typename T>
inline void AddMockResourceById(uint32_t id, const T& value)
{
    if constexpr (std::is_same_v<T, Color> || std::is_same_v<T, Dimension> ||
        std::is_same_v<T, int32_t> || std::is_same_v<T, double>) {
        AddMockResourceData(id, value);
    }
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

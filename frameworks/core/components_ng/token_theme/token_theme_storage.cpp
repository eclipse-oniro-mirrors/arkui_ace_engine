/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "core/components_ng/token_theme/token_theme_storage.h"

#include <memory>

#include "base/utils/utils.h"
#include "base/utils/system_properties.h"
#include "core/common/container.h"
#include "core/common/container_scope.h"
#include "core/pipeline_ng/pipeline_context.h"
#include "core/common/resource/resource_parse_utils.h"
#include "core/components/theme/theme_constants.h"

namespace OHOS::Ace::NG {

RefPtr<TokenTheme> TokenThemeStorage::defaultLightTheme_ = nullptr;
RefPtr<TokenTheme> TokenThemeStorage::defaultDarkTheme_ = nullptr;

namespace {
const RefPtr<TokenTheme>& GetEmptyTokenTheme()
{
    static const RefPtr<TokenTheme> emptyTheme = nullptr;
    return emptyTheme;
}
} // namespace

TokenThemeStorage* TokenThemeStorage::GetInstance()
{
    static TokenThemeStorage instance;
    return &instance;
}

TokenThemeStorage::TokenThemeStorage() = default;

void TokenThemeStorage::StoreThemeScope(TokenThemeScopeId themeScopeId, int32_t themeId)
{
    std::lock_guard<std::mutex> lock(themeScopeMutex_);
    themeScopeMap_[themeScopeId] = themeId;
}

void TokenThemeStorage::RemoveThemeScope(TokenThemeScopeId themeScopeId, bool removeToken /* = false */)
{
    int32_t themeId = 0;
    bool hasTheme = false;
    {
        std::lock_guard<std::mutex> lock(themeScopeMutex_);
        auto iter = themeScopeMap_.find(themeScopeId);
        if (iter != themeScopeMap_.end()) {
            themeId = iter->second;
            hasTheme = true;
            themeScopeMap_.erase(iter);
        }
    }
    if (removeToken && hasTheme) {
        CacheRemove(themeId);
    }
}

const RefPtr<TokenTheme>& TokenThemeStorage::GetTheme(TokenThemeScopeId themeScopeId)
{
    if (themeScopeId == 0) {
        return GetDefaultThemeWithInstance(Container::CurrentIdSafelyWithCheck());
    }
    int32_t themeId = 0;
    {
        std::lock_guard<std::mutex> lock(themeScopeMutex_);
        auto iter = themeScopeMap_.find(themeScopeId);
        if (iter == themeScopeMap_.end()) {
            return GetEmptyTokenTheme();
        }
        themeId = iter->second;
    }
    return CacheGet(themeId);
}

void TokenThemeStorage::SetDefaultTheme(const RefPtr<TokenTheme>& theme, ColorMode colorMode)
{
    (colorMode == ColorMode::DARK ? defaultDarkTheme_ : defaultLightTheme_) = theme;
}

void TokenThemeStorage::UpdateDefaultThemeBySystemTheme(ColorMode colorMode)
{
    RefPtr<TokenTheme>& theme = colorMode == ColorMode::DARK ? defaultDarkTheme_ : defaultLightTheme_;
    if (!theme) {
        return;
    }
    auto sysTheme = ObtainSystemTheme();
    if (!sysTheme) {
        return;
    }
    auto& colorsAvailable = colorMode == ColorMode::DARK ? darkThemeColorsAvailable_ : lightThemeColorsAvailable_;
    for (size_t i = 0; i < TokenColors::TOTAL_NUMBER; i++) {
        if (!colorsAvailable[i]) {
            auto color = sysTheme->Colors()->GetByIndex(i);
            theme->Colors()->SetColor(i, color);
            colorsAvailable[i] = true;
        }
    }
}

const RefPtr<TokenTheme>& TokenThemeStorage::GetDefaultTheme()
{
    auto colorMode = CheckLocalAndSystemColorMode();
    if (systemTokenThemeCreated_[static_cast<uint32_t>(colorMode)]) {
        return colorMode == ColorMode::DARK ? defaultDarkTheme_ : defaultLightTheme_;
    }

    UpdateDefaultThemeBySystemTheme(colorMode);
    return colorMode == ColorMode::DARK ? defaultDarkTheme_ : defaultLightTheme_;
}

const RefPtr<TokenTheme>& TokenThemeStorage::GetDefaultThemeWithInstance(int32_t instanceId)
{
    std::lock_guard<std::mutex> lock(instanceBrandColorMutex_);
    if (instanceDefaultThemerMap_.find(instanceId) != instanceDefaultThemerMap_.end()) {
        return instanceDefaultThemerMap_[instanceId];
    }
    return GetDefaultTheme();
}

ColorMode TokenThemeStorage::CheckLocalAndSystemColorMode()
{
    auto sysColorMode = Container::CurrentColorMode();
    auto pipelineContext = NG::PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipelineContext, sysColorMode);
    auto colorMode = pipelineContext->GetLocalColorMode();
    if (colorMode == ColorMode::COLOR_MODE_UNDEFINED) {
        colorMode = sysColorMode;
    }
    return colorMode;
}

void TokenThemeStorage::CacheClear()
{
    std::lock_guard<std::mutex> lock(themeCacheMutex_);
    themeCache_.clear();
}

void TokenThemeStorage::ResetThemeColor(int32_t themeId, RefPtr<TokenTheme>& theme, RefPtr<TokenTheme>& defaultTheme,
    ColorMode& colorMode)
{
    auto resObjs = theme->GetResObjs();
    if (resObjs.size() != TokenColors::TOTAL_NUMBER) {
        return;
    }
    auto tokenColors = theme->Colors();
    auto defaultColors = defaultTheme ? defaultTheme->Colors() : tokenColors;
    auto userFlags = themeColorSetByUser_[themeId][(colorMode == ColorMode::DARK)];
    for (int32_t i = 0; i < TokenColors::TOTAL_NUMBER; i++) {
        if (!resObjs[i]) {
            if (userFlags.size() == TokenColors::TOTAL_NUMBER && !userFlags[i]) {
                tokenColors->SetColor(i, defaultColors->GetByIndex(i));
            }
            continue;
        }
        Color colorValue;
        auto state = ResourceParseUtils::ParseResColor(resObjs[i], colorValue);
        if (!state) {
            continue;
        }
        tokenColors->SetColor(i, colorValue);
    }
}

void TokenThemeStorage::CacheResetColor()
{
    auto colorMode = CheckLocalAndSystemColorMode();
    auto defaultTheme = GetDefaultTheme();
    if (!defaultTheme) {
        defaultTheme = CreateSystemTokenTheme(colorMode);
        SetDefaultTheme(defaultTheme, colorMode);
        CacheSet(defaultTheme);
    }
    std::lock_guard<std::mutex> lock(themeCacheMutex_);
    if (themeCache_.empty()) {
        return;
    }
    for (auto& [themeId, theme] : themeCache_) {
        LOGD("Theme reset colors with id %{public}d", themeId);
        if (!theme || (theme->GetColorMode() != ColorMode::COLOR_MODE_UNDEFINED)) {
            continue;
        }
        ResetThemeColor(themeId, theme, defaultTheme, colorMode);
    }
    TAG_LOGI(AceLogTag::ACE_THEME, "Update %{public}d themes in cache, colorMode: %{public}s",
        static_cast<int32_t>(themeCache_.size()), colorMode == ColorMode::DARK ? "dark" : "light");
}

void TokenThemeStorage::CacheSet(const RefPtr<TokenTheme>& theme)
{
    CHECK_NULL_VOID(theme);
    std::lock_guard<std::mutex> lock(themeCacheMutex_);
    themeCache_[theme->GetId()] = theme;
}

const RefPtr<TokenTheme>& TokenThemeStorage::CacheGet(int32_t themeId)
{
    std::lock_guard<std::mutex> lock(themeCacheMutex_);
    auto iter = themeCache_.find(themeId);
    if (iter == themeCache_.end()) {
        return GetEmptyTokenTheme();
    }
    return iter->second;
}

void TokenThemeStorage::CacheRemove(int32_t themeId)
{
    std::lock_guard<std::mutex> lock(themeCacheMutex_);
    themeCache_.erase(themeId);
}

RefPtr<TokenTheme> TokenThemeStorage::ObtainSystemTheme(ColorMode themeColorMode)
{
    RefPtr<TokenTheme> theme = nullptr;
    auto colorMode = CheckLocalAndSystemColorMode();
    if (themeColorMode != ColorMode::COLOR_MODE_UNDEFINED) {
        colorMode = themeColorMode;
    }
    systemTokenThemeCreated_[static_cast<uint32_t>(colorMode)] = false;
    if (colorMode == ColorMode::DARK) {
        theme = CacheGet(TokenThemeStorage::SYSTEM_THEME_DARK_ID);
    } else {
        theme = CacheGet(TokenThemeStorage::SYSTEM_THEME_LIGHT_ID);
    }
    if (!theme) {
        theme = CreateSystemTokenTheme(colorMode);
        CacheSet(theme);
    }
    if (theme) {
        systemTokenThemeCreated_[static_cast<uint32_t>(colorMode)] = true;
    }
    return theme;
}

RefPtr<TokenTheme> TokenThemeStorage::CreateSystemTokenTheme(
    ColorMode colorMode, const std::optional<int32_t>& specifiedThemeId)
{
    auto container = Container::Current();
    CHECK_NULL_RETURN(container, nullptr);
    auto pipelineContext = container->GetPipelineContext();
    CHECK_NULL_RETURN(pipelineContext, nullptr);
    auto themeManager = pipelineContext->GetThemeManager();
    CHECK_NULL_RETURN(themeManager, nullptr);
    auto themeConstants = themeManager->GetThemeConstants();
    CHECK_NULL_RETURN(themeConstants, nullptr);

    int32_t themeId = 0;
    if (specifiedThemeId.has_value()) {
        themeId = specifiedThemeId.value();
    } else {
        themeId = colorMode == ColorMode::DARK ?
            TokenThemeStorage::SYSTEM_THEME_DARK_ID : TokenThemeStorage::SYSTEM_THEME_LIGHT_ID;
    }
    auto tokenColors = AceType::MakeRefPtr<TokenColors>();
    auto tokenDarkColors = AceType::MakeRefPtr<TokenColors>();
    auto tokenTheme = AceType::MakeRefPtr<TokenTheme>(themeId);
    tokenTheme->SetColors(tokenColors);
    tokenTheme->SetDarkColors(tokenDarkColors);

    std::vector<Color> colors;
    std::vector<Color> darkColors;
    colors.reserve(TokenColors::TOTAL_NUMBER);
    darkColors.reserve(TokenColors::TOTAL_NUMBER);
    for (size_t resId = 0; resId < TokenColors::TOTAL_NUMBER; ++resId) {
        colors.push_back(themeConstants->GetColor(TokenColors::GetSystemColorResIdByIndex(resId), colorMode));
        darkColors.push_back(themeConstants->GetColor(TokenColors::GetSystemColorResIdByIndex(resId), colorMode));
    }
    tokenColors->SetColors(std::move(colors));
    tokenDarkColors->SetColors(std::move(darkColors));
    return tokenTheme;
}

void TokenThemeStorage::SetIsThemeColorAvailable(bool isDark, int32_t index, bool isColorAvailable)
{
    if (index >= 0 && index < TokenColors::TOTAL_NUMBER) {
        auto& colorsAvailable = isDark ? darkThemeColorsAvailable_ : lightThemeColorsAvailable_;
        colorsAvailable[index] = isColorAvailable;
    }
}

void TokenThemeStorage::SetIsThemeColorSetByUser(int32_t themeId, bool isDark, int32_t index, bool isColorSetByUser)
{
    if (index < 0 || index >= TokenColors::TOTAL_NUMBER) {
        return;
    }
    auto& themeIdMap = themeColorSetByUser_[themeId];
    if (themeIdMap.find(isDark) == themeIdMap.end()) {
        themeIdMap[isDark] = std::vector<bool> (TokenColors::TOTAL_NUMBER, false);
    }
    themeIdMap[isDark][index] = isColorSetByUser;
}

bool TokenThemeStorage::GetIsThemeColorSetByUser(int32_t themeId, bool isDark, int32_t index) const
{
    if (index < 0 || index >= TokenColors::TOTAL_NUMBER) {
        return false;
    }
    auto it = themeColorSetByUser_.find(themeId);
    if (it == themeColorSetByUser_.end()) {
        return false;
    }
    auto modeIt = it->second.find(isDark);
    if (modeIt == it->second.end() || modeIt->second.size() <= static_cast<size_t>(index)) {
        return false;
    }
    return modeIt->second[index];
}

void TokenThemeStorage::InitDarkThemeMapWithoutUserSet(int32_t themeId, bool isDark)
{
    if (themeColorSetByUser_.find(themeId) == themeColorSetByUser_.end()) {
        return;
    }
    auto& themeIdMap = themeColorSetByUser_[themeId];
    themeIdMap[isDark] = themeIdMap[!isDark];
}

void TokenThemeStorage::StoreInstanceBrandColor(int32_t instanceId, const BrandColorInfo& info)
{
    std::lock_guard<std::mutex> lock(instanceBrandColorMutex_);
    instanceBrandColorMap_[instanceId] = info;
    TAG_LOGD(AceLogTag::ACE_THEME,
        "StoreInstanceBrandColor instanceId: %{public}d, hasBrandColor: %{public}d, hasStrategy: %{public}d",
        instanceId, info.brandColor.has_value(), info.colorStrategy.has_value());
    if (instanceDefaultThemerMap_.find(instanceId) == instanceDefaultThemerMap_.end()) {
        ContainerScope scope(instanceId);
        instanceDefaultThemerMap_[instanceId] = CreateSystemTokenTheme(CheckLocalAndSystemColorMode());
    }
}

BrandColorInfo TokenThemeStorage::GetInstanceBrandColor(int32_t instanceId)
{
    std::lock_guard<std::mutex> lock(instanceBrandColorMutex_);
    auto it = instanceBrandColorMap_.find(instanceId);
    if (it != instanceBrandColorMap_.end()) {
        return it->second;
    }
    return BrandColorInfo {};
}

std::optional<Color> TokenThemeStorage::GetThemeScopeBrandColor(int32_t themeScopeId)
{
    auto tokenTheme = GetTheme(themeScopeId);
    auto themeColors = tokenTheme ? tokenTheme->Colors() : nullptr;
    if (themeColors) {
        Color brandColor = themeColors->Brand();
        return brandColor;
    }
    return std::nullopt;
}

void TokenThemeStorage::RemoveInstanceBrandColor(int32_t instanceId)
{
    std::lock_guard<std::mutex> lock(instanceBrandColorMutex_);
    instanceBrandColorMap_.erase(instanceId);
    instanceDefaultThemerMap_.erase(instanceId);
    TAG_LOGI(AceLogTag::ACE_THEME, "RemoveInstanceBrandColor instanceId: %{public}d", instanceId);
}

void TokenThemeStorage::ClearAllInstanceBrandColors()
{
    std::lock_guard<std::mutex> lock(instanceBrandColorMutex_);
    instanceBrandColorMap_.clear();
    instanceDefaultThemerMap_.clear();
}

void TokenThemeStorage::RegisterBrandColorChangeListener(int32_t instanceId, const BrandColorChangeCallback& callback)
{
    std::lock_guard<std::mutex> lock(brandColorListenerMutex_);
    brandColorChangeListeners_[instanceId].push_back(callback);
}

void TokenThemeStorage::UnregisterBrandColorChangeListener(int32_t instanceId)
{
    std::lock_guard<std::mutex> lock(brandColorListenerMutex_);
    brandColorChangeListeners_.erase(instanceId);
}

void TokenThemeStorage::NotifyBrandColorChange(
    int32_t instanceId, const std::optional<Color>& brandColor,
    const std::optional<BrandColorStrategy>& colorStrategy)
{
    std::vector<BrandColorChangeCallback> callbacks;
    {
        std::lock_guard<std::mutex> lock(brandColorListenerMutex_);
        auto it = brandColorChangeListeners_.find(instanceId);
        if (it != brandColorChangeListeners_.end()) {
            callbacks = it->second;
        }
    }
    if (callbacks.empty()) {
        TAG_LOGW(AceLogTag::ACE_THEME, "NotifyBrandColorChange no listener for instanceId: %{public}d", instanceId);
        return;
    }
    TAG_LOGD(AceLogTag::ACE_THEME,
        "NotifyBrandColorChange instanceId: %{public}d, hasBrandColor: %{public}d, hasStrategy: %{public}d, "
        "callbackCount: %{public}zu",
        instanceId, brandColor.has_value(), colorStrategy.has_value(), callbacks.size());
    for (const auto& callback : callbacks) {
        if (callback) {
            callback(instanceId, brandColor, colorStrategy);
        }
    }
}

} // namespace OHOS::Ace::NG

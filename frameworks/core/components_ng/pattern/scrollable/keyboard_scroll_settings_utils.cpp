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

#include "core/components_ng/pattern/scrollable/keyboard_scroll_settings_utils.h"

#include "errors.h"

#include "base/json/json_util.h"
#include "base/log/log_wrapper.h"
#include "core/common/setting_data_manager.h"

namespace OHOS::Ace::NG {
namespace {
const std::string SETTING_MOUSE_SCROLL = "settings.input.mouse";
const std::string SETTING_MOUSE_SCROLL_ROWS_KEY = "rows";
constexpr int32_t DEFAULT_KEYBOARD_SCROLL_ROWS = 3;
} // namespace

std::atomic<bool> KeyboardScrollSettingsUtils::rowsValid_ { false };
std::atomic<bool> KeyboardScrollSettingsUtils::observerRegistered_ { false };
std::atomic<int32_t> KeyboardScrollSettingsUtils::cachedRows_ { DEFAULT_KEYBOARD_SCROLL_ROWS };
std::mutex KeyboardScrollSettingsUtils::initMutex_;

int32_t KeyboardScrollSettingsUtils::GetMouseScrollRows()
{
    if (rowsValid_) {
        return cachedRows_.load();
    }
    std::lock_guard<std::mutex> lock(initMutex_);
    if (rowsValid_) {
        return cachedRows_.load();
    }
    auto& manager = SettingDataManager::GetInstance();
    if (!manager.IsInitialized()) {
        auto code = manager.Initialize();
        if (code != ERR_OK || !manager.IsInitialized()) {
            // Settings subsystem unreachable (e.g. cross-platform environment): settle on the
            // default so arrow keys never re-probe/re-register on every press.
            TAG_LOGW(AceLogTag::ACE_SCROLLABLE,
                "GetMouseScrollRows: SettingDataManager init failed(%{public}d), settle on default rows", code);
            cachedRows_.store(DEFAULT_KEYBOARD_SCROLL_ROWS);
            rowsValid_ = true;
            return DEFAULT_KEYBOARD_SCROLL_ROWS;
        }
    }
    int32_t userId = manager.GetCurrentUserId();
    if (!observerRegistered_) {
        // Register once per process; the observer only touches static state, outlives every
        // component instance and is never unregistered (same lifecycle as SettingDataManager).
        auto ret = manager.RegisterObserver(
            SETTING_MOUSE_SCROLL, &KeyboardScrollSettingsUtils::OnSettingChanged, userId);
        if (ret == ERR_OK) {
            observerRegistered_ = true;
        } else {
            TAG_LOGW(AceLogTag::ACE_SCROLLABLE,
                "GetMouseScrollRows: RegisterObserver failed(%{public}d), rows will not auto-sync", ret);
        }
    }
    int32_t rows = DEFAULT_KEYBOARD_SCROLL_ROWS;
    std::string value;
    auto code = manager.GetStringValue(SETTING_MOUSE_SCROLL, value, userId);
    if (code == ERR_OK && !value.empty()) {
        auto json = JsonUtil::ParseJsonString(value);
        if (json) {
            rows = json->GetInt(SETTING_MOUSE_SCROLL_ROWS_KEY, DEFAULT_KEYBOARD_SCROLL_ROWS);
        }
        if (rows <= 0) {
            rows = DEFAULT_KEYBOARD_SCROLL_ROWS;
        }
    } else {
        // Key missing or read failed: settle on the default. If the observer is registered, a
        // later settings change invalidates the cache and the value self-heals on next read.
        TAG_LOGW(AceLogTag::ACE_SCROLLABLE,
            "GetMouseScrollRows: read setting failed(%{public}d), settle on default rows", code);
    }
    cachedRows_.store(rows);
    rowsValid_ = true;
    return rows;
}

void KeyboardScrollSettingsUtils::OnSettingChanged(const std::string& key, int32_t userId)
{
    // Runs on the settings observer (binder) thread: only invalidate the cache. Do not do IPC or
    // touch non-atomic state here; the next UI-thread GetMouseScrollRows() re-reads under lock.
    rowsValid_ = false;
}
} // namespace OHOS::Ace::NG

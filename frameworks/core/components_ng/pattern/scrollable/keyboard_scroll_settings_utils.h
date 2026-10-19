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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_SCROLLABLE_KEYBOARD_SCROLL_SETTINGS_UTILS_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_SCROLLABLE_KEYBOARD_SCROLL_SETTINGS_UTILS_H

#include <atomic>
#include <cstdint>
#include <mutex>
#include <string>

namespace OHOS::Ace::NG {

// Reads and caches the mouse wheel scroll rows (settings key "settings.input.mouse", JSON field
// "rows"), which is the step source of keyboard arrow-key scrolling.
// Process-wide static state: the settings observer is registered exactly once (lazily) and never
// unregistered; the observer callback runs on the settings binder thread and only invalidates the
// cache, the actual re-read happens lazily on the next UI-thread GetMouseScrollRows() call under
// initMutex_. If the settings subsystem is unreachable (e.g. cross-platform environment), the
// default rows settle permanently and are never re-probed on subsequent key presses.
class KeyboardScrollSettingsUtils {
public:
    static int32_t GetMouseScrollRows();

private:
    static void OnSettingChanged(const std::string& key, int32_t userId);

    static std::atomic<bool> rowsValid_;
    static std::atomic<bool> observerRegistered_;
    static std::atomic<int32_t> cachedRows_;
    static std::mutex initMutex_;
};
} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_SCROLLABLE_KEYBOARD_SCROLL_SETTINGS_UTILS_H

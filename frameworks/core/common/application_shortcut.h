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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMMON_APPLICATION_SHORTCUT_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMMON_APPLICATION_SHORTCUT_H

#include <cstdint>
#include <functional>
#include <map>
#include <mutex>
#include <string>

#include "base/utils/macros.h"
#include "base/utils/noncopyable.h"

namespace OHOS::Ace {
struct KeyEvent;

class ACE_FORCE_EXPORT ApplicationShortcutManager final : public NonCopyable {
public:
    static ApplicationShortcutManager& GetInstance();

    int32_t RegisterApplicationShortcut(const std::string& value, uint8_t keys,
        std::function<void(int32_t)>&& onKeyboardShortcutAction);

    void UnregisterApplicationShortcut(int32_t registrationId);

    bool TryTriggerApplicationShortcut(const KeyEvent& event, int32_t triggerInstanceId);

private:
    ApplicationShortcutManager() = default;
    ~ApplicationShortcutManager() = default;

    struct ApplicationKeyboardShortcut {
        std::string value;
        uint8_t keys = 0;
        std::function<void(int32_t)> onKeyboardShortcutAction;
    };

    std::map<int32_t, ApplicationKeyboardShortcut> applicationKeyboardShortcuts_;
    int32_t applicationShortcutId_ = 0;
    std::mutex applicationShortcutMutex_;
};
} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMMON_APPLICATION_SHORTCUT_H

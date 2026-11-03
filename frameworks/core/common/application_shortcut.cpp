/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "core/common/ace_application_info.h"

#include <algorithm>
#include <cctype>
#include <limits>

#include "core/event/key_event.h"

namespace OHOS::Ace {

int32_t AceApplicationInfo::RegisterApplicationShortcut(const std::string& value, uint8_t keys,
    std::function<void(int32_t)>&& onKeyboardShortcutAction, int32_t keyCode)
{
    std::lock_guard<std::mutex> lock(applicationShortcutMutex_);
    if (applicationShortcutId_ == std::numeric_limits<int32_t>::max()) {
        return 0;
    }
    const auto registrationId = ++applicationShortcutId_;
    applicationKeyboardShortcuts_.emplace(registrationId,
        ApplicationKeyboardShortcut { value, keys, std::move(onKeyboardShortcutAction), keyCode });
    return registrationId;
}

void AceApplicationInfo::UnregisterApplicationShortcut(int32_t registrationId)
{
    // Release the callback outside the lock: its destructor may release frontend resources.
    std::map<int32_t, ApplicationKeyboardShortcut>::node_type removed;
    {
        std::lock_guard<std::mutex> lock(applicationShortcutMutex_);
        removed = applicationKeyboardShortcuts_.extract(registrationId);
    }
}

static uint8_t GetEventModifierKeys(const KeyEvent& event)
{
    uint8_t modifiers = 0;
    for (auto code : event.pressedCodes) {
        switch (code) {
            case KeyCode::KEY_CTRL_LEFT:
            case KeyCode::KEY_CTRL_RIGHT:
                modifiers |= 1;   // CTRL bit
                break;
            case KeyCode::KEY_SHIFT_LEFT:
            case KeyCode::KEY_SHIFT_RIGHT:
                modifiers |= 2;   // SHIFT bit
                break;
            case KeyCode::KEY_ALT_LEFT:
            case KeyCode::KEY_ALT_RIGHT:
                modifiers |= 4;   // ALT bit
                break;
            default:
                break;
        }
    }
    return modifiers;
}

static bool MatchShortcutValue(const KeyEvent& event, const std::string& value)
{
    auto keyStr = event.ConvertInputCodeToString();
    std::string lowerKey = keyStr;
    std::string lowerValue = value;
    auto toLower = [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); };
    std::transform(lowerKey.begin(), lowerKey.end(), lowerKey.begin(), toLower);
    std::transform(lowerValue.begin(), lowerValue.end(), lowerValue.begin(), toLower);
    if (lowerValue.empty()) {
        return false;
    }
    if (lowerValue.size() > 1 || event.IsLetterKey()) {
        return lowerKey == lowerValue;
    }
    // Function-key names such as F1 and TAB are not character-key candidates.
    if (std::any_of(lowerKey.begin(), lowerKey.end(), [](unsigned char ch) { return std::isalpha(ch); })) {
        return false;
    }
    return lowerKey.find(lowerValue) != std::string::npos;
}

bool AceApplicationInfo::TryTriggerApplicationShortcut(const KeyEvent& event, int32_t triggerInstanceId)
{
    std::vector<std::function<void(int32_t)>> callbacks;
    const uint8_t eventModifiers = GetEventModifierKeys(event);
    {
        std::lock_guard<std::mutex> lock(applicationShortcutMutex_);
        for (const auto& [registrationId, shortcut] : applicationKeyboardShortcuts_) {
            const bool keyMatches = shortcut.keyCode >= 0 ?
                static_cast<int32_t>(event.code) == shortcut.keyCode : MatchShortcutValue(event, shortcut.value);
            if (keyMatches && eventModifiers == shortcut.keys &&
                shortcut.onKeyboardShortcutAction) {
                callbacks.emplace_back(shortcut.onKeyboardShortcutAction);
            }
        }
    }
    // Registrations changed by a callback take effect on the next dispatch.
    for (const auto& callback : callbacks) {
        callback(triggerInstanceId);
    }
    return !callbacks.empty();
}

} // namespace OHOS::Ace

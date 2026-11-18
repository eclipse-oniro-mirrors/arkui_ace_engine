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

#include "core/common/application_shortcut.h"

#include <algorithm>
#include <cctype>
#include <limits>
#include <vector>

#include "base/log/ace_trace.h"
#include "base/log/log_wrapper.h"
#include "core/common/container.h"
#include "core/event/key_event.h"

namespace OHOS::Ace {

namespace {
bool IsUnsupportedShortcutContainer(const RefPtr<Container>& container, const char* operation)
{
    if (container && (container->IsFormRender() || container->IsDynamicRender())) {
        LOGE("Application shortcut %{public}s is not supported for form render or dynamic render", operation);
        return true;
    }
    return false;
}
} // namespace

ApplicationShortcutManager& ApplicationShortcutManager::GetInstance()
{
    static ApplicationShortcutManager instance;
    return instance;
}

int32_t ApplicationShortcutManager::RegisterApplicationShortcut(const std::string& value, uint8_t keys,
    std::function<void(int32_t)>&& onKeyboardShortcutAction)
{
    if (IsUnsupportedShortcutContainer(Container::CurrentSafely(), "registration")) {
        return 0;
    }
    if (applicationShortcutId_ == std::numeric_limits<int32_t>::max()) {
        LOGE("Application shortcut registration IDs exhausted");
        return 0;
    }
    const auto registrationId = ++applicationShortcutId_;
    applicationKeyboardShortcuts_.emplace(registrationId,
        ApplicationKeyboardShortcut { value, keys, std::move(onKeyboardShortcutAction) });
    return registrationId;
}

void ApplicationShortcutManager::UnregisterApplicationShortcut(int32_t registrationId)
{
    if (IsUnsupportedShortcutContainer(Container::CurrentSafely(), "unregistration")) {
        return;
    }
    applicationKeyboardShortcuts_.erase(registrationId);
}

void ApplicationShortcutManager::RemoveApplicationShortcutsForCleanup(const std::vector<int32_t>& registrationIds)
{
    for (auto id : registrationIds) {
        applicationKeyboardShortcuts_.erase(id);
    }
}

static uint8_t GetEventModifierKeys(const KeyEvent& event)
{
    uint8_t modifiers = 0;
    for (auto code : event.pressedCodes) {
        switch (code) {
            case KeyCode::KEY_CTRL_LEFT:
            case KeyCode::KEY_CTRL_RIGHT:
                modifiers |= static_cast<uint8_t>(ModifierKeyName::ModifierKeyCtrl);
                break;
            case KeyCode::KEY_SHIFT_LEFT:
            case KeyCode::KEY_SHIFT_RIGHT:
                modifiers |= static_cast<uint8_t>(ModifierKeyName::ModifierKeyShift);
                break;
            case KeyCode::KEY_ALT_LEFT:
            case KeyCode::KEY_ALT_RIGHT:
                modifiers |= static_cast<uint8_t>(ModifierKeyName::ModifierKeyAlt);
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
    std::transform(lowerKey.begin(), lowerKey.end(), lowerKey.begin(), ::tolower);
    std::transform(lowerValue.begin(), lowerValue.end(), lowerValue.begin(), ::tolower);
    return lowerKey.find(lowerValue) != std::string::npos;
}

bool ApplicationShortcutManager::TryTriggerApplicationShortcut(const KeyEvent& event, int32_t triggerInstanceId)
{
    if (IsUnsupportedShortcutContainer(Container::GetContainer(triggerInstanceId), "dispatch")) {
        return false;
    }
    std::vector<ApplicationKeyboardShortcut> matched;
    const uint8_t eventModifiers = GetEventModifierKeys(event);
    for (const auto& [registrationId, shortcut] : applicationKeyboardShortcuts_) {
        if (MatchShortcutValue(event, shortcut.value) && eventModifiers == shortcut.keys &&
            shortcut.onKeyboardShortcutAction) {
            matched.push_back(shortcut);
        }
    }
    // Registrations changed by a callback take effect on the next dispatch.
    if (!matched.empty()) {
        ACE_SCOPED_TRACE("ApplicationShortcut dispatch: count[%zu]", matched.size());
        for (const auto& item : matched) {
            ACE_SCOPED_TRACE("ApplicationShortcut callback: value[%s] keys[%u]",
                item.value.c_str(), static_cast<uint32_t>(item.keys));
            item.onKeyboardShortcutAction(triggerInstanceId);
        }
    }
    return !matched.empty();
}

} // namespace OHOS::Ace

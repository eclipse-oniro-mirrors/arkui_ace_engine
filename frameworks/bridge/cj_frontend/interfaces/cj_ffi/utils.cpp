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

#include "bridge/cj_frontend/interfaces/cj_ffi/utils.h"
#include "core/event/function_key.h"


using namespace OHOS::Ace;
using namespace OHOS::Ace::Framework;

ExternalString Utils::MallocCString(const std::string& origin)
{
    if (origin.empty()) {
        return {nullptr, nullptr};
    }
    auto len = origin.length() + 1;
    char* res = (char*)malloc(sizeof(char) * len);
    if (res == nullptr) {
        return {nullptr, nullptr};
    }
    std::char_traits<char>::copy(res, origin.c_str(), len);
    return {res, reinterpret_cast<void(*)(const void*)>(free)};
}

RefPtr<Frontend> Utils::GetCurrentFrontend()
{
    auto container = Container::Current();
    if (!container) {
        container = Container::GetActive();
        if (!container) {
            LOGE("Can not found valid container");
            return nullptr;
        }
    }
    return container->GetFrontend();
}

std::string Utils::GetFunctionKey(int32_t functionKey)
{
    static const std::map<FunctionKey, std::string> keyNameMap {
        {FunctionKey::FK_ESC, "ESC"},
        {FunctionKey::FK_F1, "F1"},
        {FunctionKey::FK_F2, "F2"},
        {FunctionKey::FK_F3, "F3"},
        {FunctionKey::FK_F4, "F4"},
        {FunctionKey::FK_F5, "F5"},
        {FunctionKey::FK_F6, "F6"},
        {FunctionKey::FK_F7, "F7"},
        {FunctionKey::FK_F8, "F8"},
        {FunctionKey::FK_F9, "F9"},
        {FunctionKey::FK_F10, "F10"},
        {FunctionKey::FK_F11, "F11"},
        {FunctionKey::FK_F12, "F12"},
        {FunctionKey::FK_TAB, "TAB"},
        {FunctionKey::FK_DPAD_UP, "DPAD_UP"},
        {FunctionKey::FK_DPAD_DOWN, "DPAD_DOWN"},
        {FunctionKey::FK_DPAD_LEFT, "DPAD_LEFT"},
        {FunctionKey::FK_DPAD_RIGHT, "DPAD_RIGHT"},
        {FunctionKey::FK_CAPS_LOCK, "CAPS_LOCK"},
        {FunctionKey::FK_SCROLL_LOCK, "SCROLL_LOCK"},
        {FunctionKey::FK_PAUSE_BREAK, "PAUSE_BREAK"},
        {FunctionKey::FK_NUM_LOCK, "NUM_LOCK"},
        {FunctionKey::FK_INSERT, "INSERT"},
        {FunctionKey::FK_DELETE, "DELETE"},
        {FunctionKey::FK_HOME, "HOME"},
        {FunctionKey::FK_END, "END"},
        {FunctionKey::FK_PAGE_UP, "PAGE_UP"},
        {FunctionKey::FK_PAGE_DOWN, "PAGE_DOWN"},
        {FunctionKey::FK_ENTER, "ENTER"},
        {FunctionKey::FK_BACKSPACE, "BACKSPACE"}
    };
    auto result = keyNameMap.find(static_cast<FunctionKey>(functionKey));
    return (result != keyNameMap.end()) ? result->second : std::string();
}

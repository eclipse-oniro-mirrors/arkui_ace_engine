/*
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_EVENT_EVENT_HUB_CALLBACK_SETS_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_EVENT_EVENT_HUB_CALLBACK_SETS_H

#include "core/components_ng/event/event_hub.h"

namespace OHOS::Ace::NG {

struct AppearCallbackSet : public AceType {
    DECLARE_ACE_TYPE(AppearCallbackSet, AceType);

public:
    AppearCallbackSet() = default;
    ~AppearCallbackSet() override = default;
    std::function<void()> onAppear;
    std::function<void()> onJSFrameNodeAppear;
    std::function<void()> onDisappear;
    std::function<void()> onJSFrameNodeDisappear;
};

struct AreaCallbackSet : public AceType {
    DECLARE_ACE_TYPE(AreaCallbackSet, AceType);

public:
    AreaCallbackSet() = default;
    ~AreaCallbackSet() override = default;
    OnAreaChangedFunc onAreaChanged;
    std::unordered_map<int32_t, OnAreaChangedFunc> onAreaChangedInnerCallbacks;
    std::vector<int32_t> hasInnerAreaChangeUntriggered;
};

struct SizeCallbackSet : public AceType {
    DECLARE_ACE_TYPE(SizeCallbackSet, AceType);

public:
    SizeCallbackSet() = default;
    ~SizeCallbackSet() override = default;
    OnSizeChangedFunc onSizeChanged;
    std::unordered_map<int32_t, OnSizeChangedFunc> onSizeChangedInnerCallbacks;
    OnSizeChangedFunc onJsFrameNodeSizeChanged;
};

struct AttachDetachCallbackSet : public AceType {
    DECLARE_ACE_TYPE(AttachDetachCallbackSet, AceType);

public:
    AttachDetachCallbackSet() = default;
    ~AttachDetachCallbackSet() override = default;
    std::function<void()> onAttach;
    std::function<void()> onDetach;
};

struct EnabledNdkCallbackSet : public AceType {
    DECLARE_ACE_TYPE(EnabledNdkCallbackSet, AceType);

public:
    EnabledNdkCallbackSet() = default;
    ~EnabledNdkCallbackSet() override = default;
    std::function<void()> enabledFunc;
    std::function<void()> ndkDrawCompletedCallback;
    std::function<void()> ndkLayoutCallback;
};

struct KeyboardShortcutSet : public AceType {
    DECLARE_ACE_TYPE(KeyboardShortcutSet, AceType);

public:
    KeyboardShortcutSet() = default;
    ~KeyboardShortcutSet() override = default;
    std::vector<KeyboardShortcut> keyboardShortcut;
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_EVENT_EVENT_HUB_CALLBACK_SETS_H

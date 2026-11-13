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

#ifndef FOUNDATION_ACE_ADAPTER_OHOS_OSAL_UI_SESSION_ACCESSIBILITY_ACTION_H
#define FOUNDATION_ACE_ADAPTER_OHOS_OSAL_UI_SESSION_ACCESSIBILITY_ACTION_H

#include <cstdint>
#include <map>
#include <optional>
#include <string>

#include "accessibility_def.h"
#include "base/memory/ace_type.h"

namespace OHOS::Ace::NG {
class FrameNode;
}

namespace OHOS::Ace {
enum class UiSessionAccessibilityResult {
    SUCCESS,
    NODE_NOT_FOUND,
    INVALID_PARAM,
    EXECUTE_FAIL,
    NOT_SUPPORTED,
};

bool IsUiSessionAccessibilityUnsupportedHost(const RefPtr<NG::FrameNode>& host);

UiSessionAccessibilityResult ExecuteUiSessionAccessibilityAction(const RefPtr<NG::FrameNode>& host,
    const std::optional<int64_t>& elementId, Accessibility::ActionType action,
    const std::map<std::string, std::string>& arguments, bool argumentsValid = true);
}

#endif // FOUNDATION_ACE_ADAPTER_OHOS_OSAL_UI_SESSION_ACCESSIBILITY_ACTION_H

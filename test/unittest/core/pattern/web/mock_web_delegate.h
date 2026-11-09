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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_WEB_MOCK_WEB_DELEGATE_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_WEB_MOCK_WEB_DELEGATE_H

#include <string>
#include <memory>
#include <vector>
#include "core/components_ng/pattern/web/web_pattern.h"

namespace OHOS::NWeb {
class NWebAccessibilityNodeInfo;
class NWebAgentManager;
}
namespace OHOS::Ace {
void SetReturnStatus(const std::string& status);
void SetReturnNode(std::shared_ptr<OHOS::NWeb::NWebAccessibilityNodeInfo> node);
void SetComponentType(const std::string& type);
// Test-only knobs to control WebDelegate::SerializeWebState / RestoreWebState return values.
// SetMockSerializeWebState enables configurable mode and sets the returned state; the empty-by-default
// behavior is restored by ResetMockSerializeWebState. RestoreWebState returns false by default.
void SetMockSerializeWebState(const std::vector<uint8_t>& result);
void ResetMockSerializeWebState();
void SetMockRestoreWebState(bool result);
void ResetMockRestoreWebState();

// Test-only knobs to capture WebDelegate::RequestNativeDomTree calls.
// SetMockRequestNativeDomTreeCallback installs a callback invoked on every
// RequestNativeDomTree call with (offset, rectJson); the captured values can
// be read via GetMockRequestNativeDomTreeLastOffset/...LastRectJson. Reset
// via ResetMockRequestNativeDomTree.
struct MockRequestNativeDomTreeCall {
    bool called = false;
    NG::OffsetF offset;
    std::string rectJson;
};
void SetMockRequestNativeDomTreeCallback(std::function<void(const NG::OffsetF&, const std::string&)> cb);
const MockRequestNativeDomTreeCall& GetMockRequestNativeDomTreeLastCall();
void ResetMockRequestNativeDomTree();
void SetMockRequestNativeDomTreeResult(const std::string& result);
} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_WEB_MOCK_WEB_DELEGATE_H
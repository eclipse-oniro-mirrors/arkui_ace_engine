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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_COMMAND_UI_COMMAND_SECURITY_GATE_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_COMMAND_UI_COMMAND_SECURITY_GATE_H

#include <cstdint>

#include "base/input_manager/input_manager.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/node_sensitive_input.h"

namespace OHOS::Ace::NG {

struct CommandSecurityContext {
    bool needSecurityAuth = false;
    uint64_t tokenId = UINT64_MAX;
};

// Central security gate for AI injected commands (UISession SendCommand).
// A command is refused only when the caller declared needSecurityAuth, the
// target node itself declares AgentControlMode::DENY and the control-component
// permission check succeeds. Any other combination proceeds.
class UiCommandSecurityGate {
public:
    // Thread-local context for the current AI command. Set by the adapter
    // before dispatching, cleared after. Executors read it via GetContext().
    static void SetContext(const CommandSecurityContext& security)
    {
        context_ = security;
    }

    static void ClearContext()
    {
        context_ = CommandSecurityContext();
    }

    static const CommandSecurityContext& GetContext()
    {
        return context_;
    }

    static bool IsCommandDenied(const RefPtr<FrameNode>& target, bool needSecurityAuth);

    static void ReportDeniedCommand(const RefPtr<FrameNode>& target, const CommandSecurityContext& security);

private:
    static thread_local CommandSecurityContext context_;
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_COMMAND_UI_COMMAND_SECURITY_GATE_H

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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_BASE_INSPECTOR_TREE_REQUEST_CONTEXT_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_BASE_INSPECTOR_TREE_REQUEST_CONTEXT_H

#include <functional>
#include <optional>
#include <set>

#include "interfaces/inner_api/ui_session/inspector_tree_request.h"
#include "interfaces/inner_api/ui_session/param_config.h"

#include "base/json/json_util.h"
#include "base/memory/referenced.h"
#include "base/utils/macros.h"
#include "core/components_ng/base/inspector_tree_dependency_result.h"

struct ArkUI_AccessibilityProvider;
struct ThirdProviderTreeExportBudget;

namespace OHOS::Ace::NG {
class FrameNode;
enum class PrivacyReason { PASSWORD, OBSCURED, PRIVACY_WINDOW };
enum class InspectorTextKind { DISPLAY_TEXT, INPUT_VALUE, EDITABLE_TEXT, PLACEHOLDER, HINT };

// UI-thread request state. No component retains this object or its borrowed config pointer.
class ACE_FORCE_EXPORT InspectorTreeRequestContext {
public:
    InspectorTreeRequestContext(ParamConfig config, InspectorWindowSnapshot window);

    bool CanExportText(InspectorTextKind kind, bool password, bool obscured);
    void CollectWithWindowPrivacy(
        const std::optional<InspectorWindowSnapshot>& snapshot, const std::function<void()>& collect);
    bool MarkPrivacyReasonOnce(PrivacyReason reason);
    void FinalizeNativeTree(const std::shared_ptr<JsonValue>& root);
    std::shared_ptr<JsonValue> FinalizeResult(std::shared_ptr<JsonValue> root);
    void CollectCustomContent(ArkUI_AccessibilityProvider* provider, const RefPtr<FrameNode>& host,
        std::shared_ptr<JsonValue>& node);
    bool AcceptExtensionTree(JsonValue& tree);

    void ReportError(InspectorTreeError error)
    {
        dependencyResult.ReportError(error);
    }

    void CancelResult()
    {
        cancelled_ = true;
    }

    bool ShouldStop() const
    {
        return cancelled_ || dependencyResult.HasError();
    }

    ParamConfig config;
    InspectorWindowSnapshot window;
    InspectorTreeDependencyResult dependencyResult;

private:
    std::set<PrivacyReason> loggedReasons_;
    bool cancelled_ = false;
    std::shared_ptr<ThirdProviderTreeExportBudget> providerBudget_;
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_BASE_INSPECTOR_TREE_REQUEST_CONTEXT_H

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

#include "core/components_ng/base/inspector_tree_request_context.h"

#include <vector>

#include "base/log/log_wrapper.h"

namespace OHOS::Ace::NG {
InspectorTreeRequestContext::InspectorTreeRequestContext(
    ParamConfig options, InspectorTreeRequestRoute requestRoute, InspectorWindowSnapshot snapshot)
    : config(options), route(requestRoute), window(std::move(snapshot))
{
    config.inspectorContext = nullptr;
}

bool InspectorTreeRequestContext::MarkPrivacyReasonOnce(PrivacyReason reason)
{
    if (!loggedReasons_.insert(reason).second) {
        return false;
    }
    TAG_LOGI(AceLogTag::ACE_ACCESSIBILITY, "InspectorTree protected text omitted, reason=%{public}d",
        static_cast<int32_t>(reason));
    return true;
}

bool InspectorTreeRequestContext::CanExportText(InspectorTextKind kind, bool password, bool obscured)
{
    bool allowed = true;
    const bool editable = kind == InspectorTextKind::INPUT_VALUE || kind == InspectorTextKind::EDITABLE_TEXT;
    if (editable && password) {
        MarkPrivacyReasonOnce(PrivacyReason::PASSWORD);
        allowed = false;
    }
    if (obscured) {
        MarkPrivacyReasonOnce(PrivacyReason::OBSCURED);
        allowed = false;
    }
    if (editable && window.privacyWindow) {
        MarkPrivacyReasonOnce(PrivacyReason::PRIVACY_WINDOW);
        allowed = false;
    }
    return allowed;
}

void InspectorTreeRequestContext::FinalizeNativeTree(const std::shared_ptr<JsonValue>& root)
{
    std::vector<std::shared_ptr<JsonValue>> pending { root };
    while (!pending.empty()) {
        auto node = std::move(pending.back());
        pending.pop_back();
        auto children = node->GetValue("$children");
        std::set<std::string> marqueeText;
        const bool marquee = node->GetString("$type") == "Marquee";
        for (int32_t i = 0; i < children->GetArraySize(); ++i) {
            auto child = children->GetArrayItem(i);
            if (marquee && child->GetString("$type") == "Text") {
                auto attrs = child->GetObject("$attrs");
                auto content = attrs->GetValue("content");
                if (content->IsString() && !marqueeText.insert(content->GetString()).second) {
                    attrs->Delete("content");
                }
            }
            pending.emplace_back(std::move(child));
        }
    }
}

} // namespace OHOS::Ace::NG

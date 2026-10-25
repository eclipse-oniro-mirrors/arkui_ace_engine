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
#include "base/utils/utils.h"
#include "core/accessibility/third_provider_tree_exporter.h"

namespace OHOS::Ace::NG {
namespace {
InspectorTreeError MapProviderError(ThirdProviderExportStatus status)
{
    switch (status) {
        case ThirdProviderExportStatus::SUCCESS:
        case ThirdProviderExportStatus::NOT_REGISTERED:
            return InspectorTreeError::NONE;
        case ThirdProviderExportStatus::QUERY_FAILED:
            return InspectorTreeError::PROVIDER_QUERY_FAILED;
        case ThirdProviderExportStatus::INVALID_DATA:
            return InspectorTreeError::INVALID_PROVIDER_DATA;
        case ThirdProviderExportStatus::NODE_LIMIT:
            return InspectorTreeError::NODE_COUNT_EXCEEDS_LIMIT;
        case ThirdProviderExportStatus::SIZE_LIMIT:
            return InspectorTreeError::TREE_DATA_SIZE_EXCEEDS_LIMIT;
        default:
            return InspectorTreeError::COLLECTION_FAILED;
    }
}
} // namespace

InspectorTreeRequestContext::InspectorTreeRequestContext(ParamConfig options, InspectorWindowSnapshot snapshot)
    : config(options), window(std::move(snapshot)), providerBudget_(std::make_shared<ThirdProviderTreeExportBudget>())
{
    config.inspectorContext = nullptr;
}

void InspectorTreeRequestContext::CollectCustomContent(
    ArkUI_AccessibilityProvider* provider, const RefPtr<FrameNode>& host, std::shared_ptr<JsonValue>& node)
{
    if (!config.withCustomContent || ShouldStop()) {
        return;
    }
    auto result = ThirdProviderTreeExporter::ExportFlatToInspectorJson(provider, host, *providerBudget_);
    ReportError(MapProviderError(result.status));
    if (result.status == ThirdProviderExportStatus::SUCCESS) {
        if (!result.nodes || !node || !node->Put("accessibilityTree", result.nodes)) {
            ReportError(InspectorTreeError::COLLECTION_FAILED);
        }
    }
}

bool InspectorTreeRequestContext::AcceptExtensionTree(JsonValue& tree)
{
    if (!tree.IsObject() || !tree.GetChild()->IsValid()) {
        ReportError(InspectorTreeError::COLLECTION_FAILED);
    } else if (tree.Contains("errorMessage")) {
        dependencyResult.ReportErrorMessage(tree.GetString("errorMessage"));
    }
    if (ShouldStop()) {
        return false;
    }
    if (config.withCustomContent) {
        ReportError(MapProviderError(ThirdProviderTreeExporter::AccumulateInspectorTree(tree, *providerBudget_)));
    }
    tree.Delete("errorMessage");
    return !ShouldStop();
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

void InspectorTreeRequestContext::CollectWithWindowPrivacy(
    const std::optional<InspectorWindowSnapshot>& snapshot, const std::function<void()>& collect)
{
    if (ShouldStop()) {
        return;
    }
    if (!snapshot) {
        ReportError(InspectorTreeError::COLLECTION_FAILED);
        return;
    }
    // Cross-window collection is synchronous and shares the request's budget, errors and privacy reasons.
    const bool previousPrivacy = window.privacyWindow;
    window.privacyWindow = snapshot->privacyWindow;
    collect();
    window.privacyWindow = previousPrivacy;
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

std::shared_ptr<JsonValue> InspectorTreeRequestContext::FinalizeResult(std::shared_ptr<JsonValue> root)
{
    if (!root || !root->IsObject()) {
        ReportError(InspectorTreeError::COLLECTION_FAILED);
    }
    if (dependencyResult.HasError()) {
        root = JsonUtil::CreateSharedPtrJson(true);
    }
    CHECK_NULL_RETURN(root, nullptr);
    root->Delete("errorMessage");
    if (config.withResponseRegion || config.withCustomContent) {
        root->Put("errorMessage", dependencyResult.GetErrorMessage());
    }
    return root;
}

} // namespace OHOS::Ace::NG

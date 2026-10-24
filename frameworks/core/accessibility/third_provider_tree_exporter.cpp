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

#include "core/accessibility/third_provider_tree_exporter.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "base/json/json_util_internal.h"
#include "base/log/log_wrapper.h"
#include "core/accessibility/native_interface_accessibility_impl.h"
#include "core/accessibility/native_interface_accessibility_provider.h"
#include "core/components_ng/base/frame_node.h"

using namespace OHOS::Ace;

namespace {
constexpr size_t MAX_PROVIDER_NODES = 10000;
constexpr size_t MAX_PROVIDER_DEPTH = 64;
constexpr size_t MAX_PROVIDER_BYTES = 10 * 1024 * 1024;
constexpr int64_t PROVIDER_ROOT_ELEMENT_ID = -1;
constexpr int32_t PROVIDER_REQUEST_ID = 0;
constexpr float OPACITY_TOLERANCE = 0.001f;
using ProviderElementInfo = ArkUI_AccessibilityElementInfo;
using NodeIndex = std::unordered_map<int64_t, size_t>;

struct RectTransform {
    double left = 0.0;
    double top = 0.0;
    double scaleX = 1.0;
    double scaleY = 1.0;
};

std::string FormatRect(const ArkUI_AccessibleRect& rect, const RectTransform& transform)
{
    char buffer[256];
    auto length = std::snprintf(buffer, sizeof(buffer), "[%.2f, %.2f],[%.2f,%.2f]",
        transform.left + rect.leftTopX * transform.scaleX,
        transform.top + rect.leftTopY * transform.scaleY,
        transform.left + rect.rightBottomX * transform.scaleX,
        transform.top + rect.rightBottomY * transform.scaleY);
    return length > 0 && static_cast<size_t>(length) < sizeof(buffer) ? std::string(buffer) : std::string();
}

bool GetTransform(const RefPtr<NG::FrameNode>& host, RectTransform& transform)
{
    if (!host) {
        return true;
    }
    // Provider rectangles are host-local, despite the GetScreenRect accessor name.
    // Both export formats report window-relative coordinates like the host node.
    const auto rect = host->GetTransformRectRelativeToWindow();
    const auto scale = host->GetTransformScaleRelativeToWindow();
    transform = { rect.Left(), rect.Top(), scale.x, scale.y };
    return std::isfinite(transform.left) && std::isfinite(transform.top) &&
        std::isfinite(transform.scaleX) && std::isfinite(transform.scaleY);
}

void PutText(JsonValue& node, const char* key, const std::string& value)
{
    if (!value.empty()) {
        node.Put(key, value.c_str());
    }
}

void PutStates(JsonValue& node, const ProviderElementInfo& info)
{
    node.Put("visible", info.IsVisible());
    node.Put("checkable", info.IsCheckable());
    node.Put("checked", info.IsChecked());
    node.Put("clickable", info.IsClickable());
    node.Put("longClickable", info.IsLongClickable());
    node.Put("enabled", info.IsEnabled());
    node.Put("focusable", info.IsFocusable());
    node.Put("focused", info.IsFocused());
    node.Put("accessibilityFocused", info.IsAccessibilityFocused());
    node.Put("selected", info.IsSelected());
    node.Put("editable", info.IsEditable());
    node.Put("scrollable", info.IsScrollable());
    node.Put("password", info.IsPassword());
    node.Put("accessibilityGroup", info.GetAccessibilityGroup());
}

std::unique_ptr<JsonValue> RangeToJson(const ArkUI_AccessibleRangeInfo& range)
{
    auto json = JsonUtil::Create(true);
    json->Put("min", range.min);
    json->Put("max", range.max);
    json->Put("current", range.current);
    return json;
}

std::unique_ptr<JsonValue> GridToJson(const ArkUI_AccessibleGridInfo& grid)
{
    auto json = JsonUtil::Create(true);
    json->Put("rowCount", grid.rowCount);
    json->Put("columnCount", grid.columnCount);
    json->Put("selectionMode", grid.selectionMode);
    return json;
}

std::unique_ptr<JsonValue> GridItemToJson(const ArkUI_AccessibleGridItemInfo& item)
{
    auto json = JsonUtil::Create(true);
    json->Put("heading", item.heading);
    json->Put("selected", item.selected);
    json->Put("rowIndex", item.rowIndex);
    json->Put("rowSpan", item.rowSpan);
    json->Put("columnIndex", item.columnIndex);
    json->Put("columnSpan", item.columnSpan);
    return json;
}

std::unique_ptr<JsonValue> ActionsToJson(const ProviderElementInfo& info)
{
    auto array = JsonUtil::CreateArray(true);
    for (const auto& action : info.GetOperationActions()) {
        auto json = JsonUtil::Create(true);
        json->Put("action", static_cast<int32_t>(action.actionType));
        if (action.description && action.description[0] != '\0') {
            json->Put("description", action.description);
        }
        array->PutRef(std::move(json));
    }
    return array;
}

void PutExtendedAttributes(JsonValue& node, const ProviderElementInfo& info)
{
    auto attrs = JsonUtil::Create(true);
    PutText(*attrs, "backgroundColor", info.GetBackgroundColor());
    PutText(*attrs, "backgroundImage", info.GetBackgroundImage());
    PutText(*attrs, "blur", info.GetBlur());
    PutText(*attrs, "hitTestBehavior", info.GetHitTestBehavior());
    const auto range = info.GetRangeInfo();
    if (range.min != 0 || range.max != 0 || range.current != 0) {
        attrs->PutRef("range", RangeToJson(range));
    }
    const auto grid = info.GetGridInfo();
    if (grid.rowCount != 0 || grid.columnCount != 0 || grid.selectionMode != 0) {
        attrs->PutRef("grid", GridToJson(grid));
    }
    const auto item = info.GetGridItemInfo();
    if (item.heading || item.selected || item.rowIndex != 0 || item.columnIndex != 0 ||
        item.rowSpan != 0 || item.columnSpan != 0) {
        attrs->PutRef("gridItem", GridItemToJson(item));
    }
    if (!info.GetOperationActions().empty()) {
        attrs->PutRef("actions", ActionsToJson(info));
    }
    if (attrs->GetChild()->IsValid()) {
        node.PutRef("$attrs", std::move(attrs));
    }
}

std::unique_ptr<JsonValue> ElementToTreeJson(const ProviderElementInfo& info, const RectTransform& transform)
{
    auto node = JsonUtil::Create(true);
    node->Put("$ID", static_cast<int64_t>(info.GetElementId()));
    node->Put("$parentID", info.GetParentId());
    const auto type = info.GetComponentType();
    node->Put("$type", type.empty() ? "third" : type.c_str());
    node->Put("type", "third");
    node->Put("$rect", FormatRect(info.GetScreenRect(), transform).c_str());
    PutText(*node, "contents", info.GetContents());
    PutText(*node, "hintText", info.GetHintText());
    PutText(*node, "accessibilityText", info.GetAccessibilityText());
    PutText(*node, "accessibilityDescription", info.GetAccessibilityDescription());
    PutText(*node, "accessibilityLevel", info.GetAccessibilityLevel());
    PutText(*node, "compid", info.GetComponentIdentifier());
    PutStates(*node, info);
    if (info.GetZIndex() != 0) {
        node->Put("zIndex", info.GetZIndex());
    }
    if (info.GetOpacity() > 0 && std::abs(info.GetOpacity() - 1.0f) > OPACITY_TOLERANCE) {
        node->Put("opacity", static_cast<double>(info.GetOpacity()));
    }
    PutExtendedAttributes(*node, info);
    return node;
}

ThirdProviderExportStatus QueryProvider(ArkUI_AccessibilityProvider* provider, std::vector<ProviderElementInfo>& infos)
{
    if (!provider || !provider->IsRegister()) {
        TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY, "Third provider export skipped: unregistered");
        return ThirdProviderExportStatus::NOT_REGISTERED;
    }
    const auto result = provider->FindAccessibilityNodeInfosById(PROVIDER_ROOT_ELEMENT_ID,
        ARKUI_ACCESSIBILITY_NATIVE_SEARCH_MODE_PREFETCH_RECURSIVE_CHILDREN, PROVIDER_REQUEST_ID, infos);
    if (result != 0) {
        TAG_LOGW(AceLogTag::ACE_ACCESSIBILITY, "Third provider query failed: %{public}d", result);
        return ThirdProviderExportStatus::QUERY_FAILED;
    }
    if (infos.size() > MAX_PROVIDER_NODES) {
        TAG_LOGW(AceLogTag::ACE_ACCESSIBILITY, "Third provider node limit exceeded: %{public}zu", infos.size());
        return ThirdProviderExportStatus::NODE_LIMIT;
    }
    return ThirdProviderExportStatus::SUCCESS;
}

// First occurrence owns both the node payload and its parent index. Never let a
// duplicate attach the same ID to another parent or replace the original data.
NodeIndex IndexNodes(const std::vector<ProviderElementInfo>& infos)
{
    NodeIndex index;
    index.reserve(infos.size());
    for (size_t i = 0; i < infos.size(); ++i) {
        index.emplace(infos[i].GetElementId(), i);
    }
    return index;
}

size_t SelectRoot(const std::vector<ProviderElementInfo>& infos, const NodeIndex& index)
{
    size_t orphan = infos.size();
    size_t fallback = infos.size();
    for (size_t i = 0; i < infos.size(); ++i) {
        const auto id = infos[i].GetElementId();
        const auto parent = infos[i].GetParentId();
        if (index.at(id) != i || id == parent) {
            continue;
        }
        if (fallback == infos.size()) {
            fallback = i;
        }
        if (parent == -1) {
            return i;
        }
        if (orphan == infos.size() && index.count(parent) == 0) {
            orphan = i;
        }
    }
    return orphan != infos.size() ? orphan : fallback;
}

using ChildrenIndex = std::unordered_map<int64_t, std::vector<size_t>>;

// Reject oversized text before cJSON duplicates it. This is a lower bound on
// the node's serialized size; Charge below also counts keys and JSON syntax.
bool FitsJsonText(const char* text, size_t& remaining)
{
    if (!text) {
        return true;
    }
    for (; *text; ++text) {
        const auto ch = static_cast<unsigned char>(*text);
        const bool shortEscape = ch == '"' || ch == '\\' || ch == '\b' || ch == '\f' ||
            ch == '\n' || ch == '\r' || ch == '\t';
        const size_t bytes = shortEscape ? 2 : (ch < 0x20 ? 6 : 1);
        if (bytes > remaining) {
            return false;
        }
        remaining -= bytes;
    }
    return true;
}

bool FitsElementText(const ProviderElementInfo& info, size_t remaining)
{
    using TextGetter = std::string (ProviderElementInfo::*)() const;
    constexpr TextGetter getters[] = {
        &ProviderElementInfo::GetComponentType, &ProviderElementInfo::GetContents,
        &ProviderElementInfo::GetHintText, &ProviderElementInfo::GetAccessibilityText,
        &ProviderElementInfo::GetAccessibilityDescription, &ProviderElementInfo::GetAccessibilityLevel,
        &ProviderElementInfo::GetComponentIdentifier, &ProviderElementInfo::GetBackgroundColor,
        &ProviderElementInfo::GetBackgroundImage, &ProviderElementInfo::GetBlur,
        &ProviderElementInfo::GetHitTestBehavior,
    };
    for (const auto getter : getters) {
        if (!FitsJsonText((info.*getter)().c_str(), remaining)) {
            return false;
        }
    }
    for (const auto& action : info.GetOperationActions()) {
        constexpr size_t MIN_ACTION_BYTES = sizeof("{\"action\":0}") - 1;
        if (remaining < MIN_ACTION_BYTES) {
            return false;
        }
        remaining -= MIN_ACTION_BYTES;
        if (!FitsJsonText(action.description, remaining)) {
            return false;
        }
    }
    return true;
}

ThirdProviderExportStatus ChargeJsonBytes(const JsonValue& json, size_t& bytes)
{
    if (bytes > MAX_PROVIDER_BYTES) {
        return ThirdProviderExportStatus::SIZE_LIMIT;
    }
    char* data = nullptr;
    size_t size = 0;
    const auto status = WriteJsonBounded(json, false, MAX_PROVIDER_BYTES - bytes, &data, &size);
    std::free(data);
    if (status != JsonWriteResult::SUCCESS) {
        return status == JsonWriteResult::SIZE_LIMIT ? ThirdProviderExportStatus::SIZE_LIMIT :
            ThirdProviderExportStatus::COLLECTION_FAILED;
    }
    bytes += size;
    return ThirdProviderExportStatus::SUCCESS;
}

struct TreeBuildState {
    std::unordered_set<int64_t> visited;
    size_t bytes = 0;
    bool failed = false;

    bool Charge(size_t extra)
    {
        if (extra > MAX_PROVIDER_BYTES - bytes) {
            failed = true;
            return false;
        }
        bytes += extra;
        return true;
    }

    bool Charge(const JsonValue& node)
    {
        if (ChargeJsonBytes(node, bytes) != ThirdProviderExportStatus::SUCCESS) {
            failed = true;
            return false;
        }
        return true;
    }
};

std::unique_ptr<JsonValue> BuildSubtree(size_t index, size_t depth, const std::vector<ProviderElementInfo>& infos,
    const ChildrenIndex& children, const RectTransform& transform, TreeBuildState& state)
{
    const auto id = infos[index].GetElementId();
    if (depth > MAX_PROVIDER_DEPTH || !state.visited.insert(id).second) {
        TAG_LOGW(AceLogTag::ACE_ACCESSIBILITY, "Third provider tree pruned at depth %{public}zu", depth);
        return nullptr;
    }
    if (!FitsElementText(infos[index], MAX_PROVIDER_BYTES - state.bytes)) {
        state.failed = true;
        return nullptr;
    }
    auto node = ElementToTreeJson(infos[index], transform);
    if (!state.Charge(*node)) {
        return nullptr;
    }
    const auto it = children.find(id);
    if (it == children.end()) {
        return node;
    }
    auto array = JsonUtil::CreateArray(true);
    bool hasChildren = false;
    for (const auto childIndex : it->second) {
        auto child = BuildSubtree(childIndex, depth + 1, infos, children, transform, state);
        if (state.failed) {
            return nullptr;
        }
        if (child) {
            const size_t extra = hasChildren ? 1 : sizeof(",\"$children\":[]") - 1;
            if (!state.Charge(extra)) {
                return nullptr;
            }
            // Transfer the complete subtree using the exported unique_ptr overload;
            // do not copy all descendants again at each ancestor.
            if (!array->PutRef(std::move(child))) {
                state.failed = true;
                return nullptr;
            }
            hasChildren = true;
        }
    }
    if (hasChildren) {
        if (!node->PutRef("$children", std::move(array))) {
            state.failed = true;
            return nullptr;
        }
    }
    return node;
}

std::unique_ptr<JsonValue> ElementToFlatJson(const ProviderElementInfo& info, const RectTransform& transform)
{
    auto node = JsonUtil::Create(true);
    node->Put("elementId", std::to_string(info.GetElementId()).c_str());
    node->Put("parentId", std::to_string(info.GetParentId()).c_str());
    auto ids = JsonUtil::CreateArray(true);
    for (const auto id : info.GetChildNodeIds()) {
        auto value = JsonUtil::ParseJsonString("\"" + std::to_string(id) + "\"");
        ids->PutRef(std::move(value));
    }
    node->PutRef("childNodeIds", std::move(ids));
    node->Put("componentType", info.GetComponentType().c_str());
    node->Put("contents", info.GetContents().c_str());
    node->Put("hintText", info.GetHintText().c_str());
    node->Put("accessibilityText", info.GetAccessibilityText().c_str());
    node->Put("accessibilityDescription", info.GetAccessibilityDescription().c_str());
    node->Put("accessibilityLevel", info.GetAccessibilityLevel().c_str());
    node->Put("componentIdentifier", info.GetComponentIdentifier().c_str());
    node->Put("backgroundColor", info.GetBackgroundColor().c_str());
    node->Put("backgroundImage", info.GetBackgroundImage().c_str());
    node->Put("blur", info.GetBlur().c_str());
    node->Put("hitTestBehavior", info.GetHitTestBehavior().c_str());
    PutStates(*node, info);
    node->Put("hint", info.IsHint());
    node->Put("pageId", info.GetPageId());
    node->Put("textBeginSelected", info.GetTextBeginSelected());
    node->Put("textEndSelected", info.GetTextEndSelected());
    node->Put("currentIndex", info.GetCurrentIndex());
    node->Put("beginIndex", info.GetBeginIndex());
    node->Put("endIndex", info.GetEndIndex());
    node->Put("itemCount", info.GetItemCount());
    node->Put("offset", info.GetOffset());
    node->Put("zIndex", info.GetZIndex());
    node->Put("opacity", static_cast<double>(info.GetOpacity()));
    const auto rect = info.GetScreenRect();
    auto screen = JsonUtil::Create(true);
    screen->Put("leftTopX", rect.leftTopX);
    screen->Put("leftTopY", rect.leftTopY);
    screen->Put("rightBottomX", rect.rightBottomX);
    screen->Put("rightBottomY", rect.rightBottomY);
    node->PutRef("screenRect", std::move(screen));
    node->Put("$rect", FormatRect(rect, transform).c_str());
    node->PutRef("rangeInfo", RangeToJson(info.GetRangeInfo()));
    node->PutRef("gridInfo", GridToJson(info.GetGridInfo()));
    node->PutRef("gridItemInfo", GridItemToJson(info.GetGridItemInfo()));
    node->PutRef("operationActions", ActionsToJson(info));
    return node;
}

bool ValidateChildReferences(const std::vector<ProviderElementInfo>& infos, const NodeIndex& index, int64_t rootId)
{
    std::unordered_set<int64_t> referenced;
    for (const auto& info : infos) {
        for (const auto childId : info.GetChildNodeIds()) {
            const auto child = index.find(childId);
            if (child == index.end() || childId == rootId || childId == info.GetElementId() ||
                infos[child->second].GetParentId() != info.GetElementId() || !referenced.insert(childId).second) {
                return false;
            }
        }
    }
    // Each non-root node must have exactly one incoming child reference.
    return referenced.size() == infos.size() - 1;
}

bool ValidateFlatTree(const std::vector<ProviderElementInfo>& infos, int64_t rootId)
{
    const auto index = IndexNodes(infos);
    if (index.size() != infos.size() || index.count(rootId) == 0) {
        return false;
    }
    // The provider root may name an external parent, but never a node in its own subtree.
    if (index.count(infos[index.at(rootId)].GetParentId()) != 0 ||
        !ValidateChildReferences(infos, index, rootId)) {
        return false;
    }
    std::unordered_set<int64_t> valid { rootId };
    for (const auto& info : infos) {
        std::vector<int64_t> path;
        std::unordered_set<int64_t> active;
        auto id = static_cast<int64_t>(info.GetElementId());
        while (valid.count(id) == 0) {
            if (index.count(id) == 0 || !active.insert(id).second) {
                return false;
            }
            path.push_back(id);
            id = infos[index.at(id)].GetParentId();
        }
        valid.insert(path.begin(), path.end());
    }
    return true;
}

ThirdProviderExportStatus ChargeArray(const JsonValue& array, ThirdProviderTreeExportBudget& budget)
{
    if (!array.IsArray()) {
        return ThirdProviderExportStatus::INVALID_DATA;
    }
    const auto count = static_cast<size_t>(array.GetArraySize());
    if (budget.nodes > MAX_PROVIDER_NODES || count > MAX_PROVIDER_NODES - budget.nodes) {
        return ThirdProviderExportStatus::NODE_LIMIT;
    }
    const auto status = ChargeJsonBytes(array, budget.bytes);
    if (status != ThirdProviderExportStatus::SUCCESS) {
        return status;
    }
    budget.nodes += count;
    return ThirdProviderExportStatus::SUCCESS;
}
} // namespace

std::shared_ptr<JsonValue> ThirdProviderTreeExporter::ExportToInspectorJson(
    ArkUI_AccessibilityProvider* provider, const RefPtr<NG::FrameNode>& host)
{
    std::vector<ProviderElementInfo> infos;
    if (QueryProvider(provider, infos) != ThirdProviderExportStatus::SUCCESS) {
        return nullptr;
    }
    if (infos.empty()) {
        TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY, "Third provider export skipped: empty");
        return nullptr;
    }
    RectTransform transform;
    if (!GetTransform(host, transform)) {
        return nullptr;
    }
    const auto index = IndexNodes(infos);
    const auto root = SelectRoot(infos, index);
    if (root == infos.size()) {
        return nullptr;
    }
    ChildrenIndex children;
    for (size_t i = 0; i < infos.size(); ++i) {
        const auto id = infos[i].GetElementId();
        if (index.at(id) == i && id != infos[i].GetParentId()) {
            children[infos[i].GetParentId()].push_back(i);
        }
    }
    JsonWriteErrorScope errors;
    TreeBuildState state;
    auto tree = BuildSubtree(root, 1, infos, children, transform, state);
    if (state.failed || errors.HasError()) {
        TAG_LOGW(AceLogTag::ACE_ACCESSIBILITY, "Third provider tree exceeded size limit or collection failed");
        return nullptr;
    }
    TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY, "Third provider exported %{public}zu nodes, root %{public}d",
        state.visited.size(), infos[root].GetElementId());
    if (state.visited.size() != infos.size()) {
        TAG_LOGW(AceLogTag::ACE_ACCESSIBILITY, "Third provider omitted duplicate or unreachable nodes");
    }
    return std::shared_ptr<JsonValue>(std::move(tree));
}

ThirdProviderTreeExportResult ThirdProviderTreeExporter::ExportFlatToInspectorJson(
    ArkUI_AccessibilityProvider* provider, const RefPtr<NG::FrameNode>& host, ThirdProviderTreeExportBudget& budget)
{
    std::vector<ProviderElementInfo> infos;
    auto status = QueryProvider(provider, infos);
    if (status != ThirdProviderExportStatus::SUCCESS) {
        return { status, nullptr };
    }
    if (budget.nodes > MAX_PROVIDER_NODES || infos.size() > MAX_PROVIDER_NODES - budget.nodes) {
        return { ThirdProviderExportStatus::NODE_LIMIT, nullptr };
    }
    constexpr size_t ARRAY_BYTES = sizeof("[]") - 1;
    if (budget.bytes > MAX_PROVIDER_BYTES - ARRAY_BYTES) {
        return { ThirdProviderExportStatus::SIZE_LIMIT, nullptr };
    }
    if (!infos.empty()) {
        std::vector<ProviderElementInfo> roots;
        if (provider->FindAccessibilityNodeInfosById(PROVIDER_ROOT_ELEMENT_ID,
            ARKUI_ACCESSIBILITY_NATIVE_SEARCH_MODE_PREFETCH_CURRENT, PROVIDER_REQUEST_ID, roots) != 0) {
            return { ThirdProviderExportStatus::QUERY_FAILED, nullptr };
        }
        if (roots.size() != 1 || !ValidateFlatTree(infos, roots.front().GetElementId())) {
            return { ThirdProviderExportStatus::INVALID_DATA, nullptr };
        }
    }
    RectTransform transform;
    if (!GetTransform(host, transform)) {
        return { ThirdProviderExportStatus::COLLECTION_FAILED, nullptr };
    }
    JsonWriteErrorScope errors;
    // Commit the request budget only after every node succeeds. Charge each node
    // before attaching it, including array brackets and separators exactly once.
    auto next = budget;
    next.bytes += ARRAY_BYTES;
    std::shared_ptr<JsonValue> nodes = JsonUtil::CreateArray(true);
    if (errors.HasError()) {
        return { ThirdProviderExportStatus::COLLECTION_FAILED, nullptr };
    }
    bool hasNodes = false;
    for (const auto& info : infos) {
        if (hasNodes) {
            if (next.bytes == MAX_PROVIDER_BYTES) {
                return { ThirdProviderExportStatus::SIZE_LIMIT, nullptr };
            }
            ++next.bytes;
        }
        if (!FitsElementText(info, MAX_PROVIDER_BYTES - next.bytes)) {
            return { ThirdProviderExportStatus::SIZE_LIMIT, nullptr };
        }
        auto node = ElementToFlatJson(info, transform);
        if (errors.HasError()) {
            return { ThirdProviderExportStatus::COLLECTION_FAILED, nullptr };
        }
        status = ChargeJsonBytes(*node, next.bytes);
        if (status != ThirdProviderExportStatus::SUCCESS) {
            return { status, nullptr };
        }
        if (!nodes->PutRef(std::move(node))) {
            return { ThirdProviderExportStatus::COLLECTION_FAILED, nullptr };
        }
        ++next.nodes;
        hasNodes = true;
    }
    budget = next;
    return { ThirdProviderExportStatus::SUCCESS, nodes };
}

ThirdProviderExportStatus ThirdProviderTreeExporter::AccumulateInspectorTree(
    const JsonValue& tree, ThirdProviderTreeExportBudget& budget)
{
    // Walk the JSON tree once; only accessibilityTree arrays consume the shared
    // request budget. Commit the budget only after the entire extension succeeds.
    auto next = budget;
    std::vector<std::unique_ptr<JsonValue>> pending;
    pending.push_back(tree.GetChild());
    while (!pending.empty()) {
        auto node = std::move(pending.back());
        pending.pop_back();
        for (; node->IsValid(); node = node->GetNext()) {
            if (node->GetKey() == "accessibilityTree") {
                const auto status = ChargeArray(*node, next);
                if (status != ThirdProviderExportStatus::SUCCESS) {
                    return status;
                }
            } else if (node->IsArray() || node->IsObject()) {
                pending.push_back(node->GetChild());
            }
        }
    }
    budget = next;
    return ThirdProviderExportStatus::SUCCESS;
}

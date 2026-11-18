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

#include "core/components_ng/base/inspector_external_subtree.h"

#include <array>
#include <cctype>
#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include "base/geometry/ng/offset_t.h"
#include "base/geometry/ng/rect_t.h"
#include "base/json/json_util.h"
#include "base/log/log_wrapper.h"
#include "core/components_ng/base/distributed_ui.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/inspector_filter.h"
#include "core/components_ng/base/inspector_tree_provider_registry.h"
#include "core/components_ng/base/ui_node.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/pipeline/pipeline_base.h"
#include "core/pipeline_ng/pipeline_context.h"

#if defined(WEB_SUPPORTED) && !defined(ACE_UNITTEST)
#include "core/components_ng/pattern/web/web_pattern.h"
#endif

namespace OHOS::Ace::NG {
namespace {
// Inspector node-schema keys shared with inspector.cpp/simplified_inspector.cpp
// (const namespace-scope arrays have internal linkage per TU).
const char INSPECTOR_TYPE[] = "$type";
const char INSPECTOR_ID[] = "$ID";
const char INSPECTOR_RECT[] = "$rect";
const char INSPECTOR_ATTRS[] = "$attrs";
const char INSPECTOR_CHILDREN[] = "$children";

// Max size of an inspector subtree payload, excluding the terminating NUL
// (4 MiB, spec R-1/R-7). Oversized payloads are skipped with a warning.
constexpr uint32_t INSPECTOR_EXTERNAL_SUBTREE_MAX_BYTES = 4194304;

constexpr int32_t INSPECTOR_EXTERNAL_SUBTREE_MAX_DEPTH = 128;

// Keys of the external host format used by CustomNode/XComponent providers
// (elementId/parentId/componentType/screenRect/attributes/children).
constexpr char INSPECTOR_EXTERNAL_KEY_ELEMENT_ID[] = "elementId";
constexpr char INSPECTOR_EXTERNAL_KEY_COMPONENT_TYPE[] = "componentType";
constexpr char INSPECTOR_EXTERNAL_KEY_SCREEN_RECT[] = "screenRect";
constexpr char INSPECTOR_EXTERNAL_KEY_ATTRIBUTES[] = "attributes";
constexpr char INSPECTOR_EXTERNAL_KEY_CHILDREN[] = "children";
constexpr char INSPECTOR_EXTERNAL_RECT_KEY_X[] = "x";
constexpr char INSPECTOR_EXTERNAL_RECT_KEY_Y[] = "y";
constexpr char INSPECTOR_EXTERNAL_RECT_KEY_WIDTH[] = "width";
constexpr char INSPECTOR_EXTERNAL_RECT_KEY_HEIGHT[] = "height";

// Detects the external host format: an object carrying elementId/
// componentType that does not already follow the inspector node schema
// ("$type"). Native-schema payloads are spliced verbatim, never converted.
bool IsExternalFormatNode(const std::unique_ptr<OHOS::Ace::JsonValue>& node)
{
    return node != nullptr && node->IsObject() && !node->Contains(INSPECTOR_TYPE) &&
           (node->Contains(INSPECTOR_EXTERNAL_KEY_COMPONENT_TYPE) || node->Contains(INSPECTOR_EXTERNAL_KEY_ELEMENT_ID));
}

// Window-origin offset (screen-space) used to convert external "screenRect"
// values into the inspector "$rect" window-relative space: engine nodes emit
// GetTransformRectRelativeToWindow (vp), external payload screenRect values
// are in vp as well, and GetDisplayWindowRectInfo gives the window position
// on screen in physical px (set from the window rect in
// ui_content_impl.cpp UpdateViewportConfig), so the px origin must be scaled
// to vp before the subtraction: windowPos(vp) = screenPos(vp) - origin(px) / density.
OffsetF GetExternalRectWindowOffset(const RefPtr<NG::UINode>& parent)
{
    CHECK_NULL_RETURN(parent, OffsetF(0.0f, 0.0f));
    auto* context = parent->GetContext();
    if (context == nullptr) {
        TAG_LOGW(AceLogTag::ACE_LAYOUT_INSPECTOR,
            "Failed to resolve pipeline context, screen->window rect conversion skipped for node %{private}d",
            parent->GetId());
        return OffsetF(0.0f, 0.0f);
    }
    const auto& windowRect = context->GetDisplayWindowRectInfo();
    // G.EXP.22-CPP: the divisions below must never divide by zero. The
    // negative form !(density > 0.0) also rejects NaN. A non-positive or NaN
    // density falls back to 1.0 after the warning, which keeps the legacy
    // px-passthrough semantics for the window origin.
    double density = context->GetDensity();
    if (!(density > 0.0)) {
        TAG_LOGW(AceLogTag::ACE_LAYOUT_INSPECTOR,
            "Invalid density %{public}lf, window origin left unscaled for node %{private}d", density, parent->GetId());
        density = 1.0;
    }
    return OffsetF(static_cast<float>(windowRect.Left() / density), static_cast<float>(windowRect.Top() / density));
}

std::string ConvertExternalRectToBounds(const std::unique_ptr<OHOS::Ace::JsonValue>& node, const OffsetF& windowOffset)
{
    auto rect = node->GetValue(INSPECTOR_EXTERNAL_KEY_SCREEN_RECT);
    if (rect == nullptr || !rect->IsObject()) {
        TAG_LOGW(AceLogTag::ACE_LAYOUT_INSPECTOR,
            "External subtree node missing or invalid screenRect, $rect omitted: elementId %{public}d",
            node->GetInt(INSPECTOR_EXTERNAL_KEY_ELEMENT_ID, -1));
        return "";
    }
    RectF bounds(rect->GetDouble(INSPECTOR_EXTERNAL_RECT_KEY_X, 0.0) - windowOffset.GetX(),
        rect->GetDouble(INSPECTOR_EXTERNAL_RECT_KEY_Y, 0.0) - windowOffset.GetY(),
        rect->GetDouble(INSPECTOR_EXTERNAL_RECT_KEY_WIDTH, 0.0),
        rect->GetDouble(INSPECTOR_EXTERNAL_RECT_KEY_HEIGHT, 0.0));
    return bounds.ToBounds();
}

std::unique_ptr<OHOS::Ace::JsonValue> ConvertExternalSubtreeNode(
    const std::unique_ptr<OHOS::Ace::JsonValue>& node, int32_t depth, const OffsetF& windowOffset);

// Fills the non-children fields of one converted node ("$type"/"$ID"/
// "$rect"/"$attrs"), with per-field degradation warnings for missing or
// malformed external fields.
void PutExternalNodeFields(const std::unique_ptr<OHOS::Ace::JsonValue>& jsonNode,
    const std::unique_ptr<OHOS::Ace::JsonValue>& node, const OffsetF& windowOffset)
{
    if (!node->Contains(INSPECTOR_EXTERNAL_KEY_COMPONENT_TYPE)) {
        TAG_LOGW(AceLogTag::ACE_LAYOUT_INSPECTOR,
            "External subtree node missing componentType, empty $type emitted: elementId %{public}d",
            node->GetInt(INSPECTOR_EXTERNAL_KEY_ELEMENT_ID, -1));
    }
    if (!node->Contains(INSPECTOR_EXTERNAL_KEY_ELEMENT_ID)) {
        TAG_LOGW(AceLogTag::ACE_LAYOUT_INSPECTOR,
            "External subtree node missing elementId, $ID -1 emitted: componentType %{public}s",
            node->GetString(INSPECTOR_EXTERNAL_KEY_COMPONENT_TYPE, "").c_str());
    }
    jsonNode->Put(INSPECTOR_TYPE, node->GetString(INSPECTOR_EXTERNAL_KEY_COMPONENT_TYPE, "").c_str());
    jsonNode->Put(INSPECTOR_ID, node->GetInt(INSPECTOR_EXTERNAL_KEY_ELEMENT_ID, -1));
    std::string bounds = ConvertExternalRectToBounds(node, windowOffset);
    if (!bounds.empty()) {
        jsonNode->Put(INSPECTOR_RECT, bounds.c_str());
    }
    auto attributes = node->GetValue(INSPECTOR_EXTERNAL_KEY_ATTRIBUTES);
    if (attributes != nullptr && attributes->IsObject()) {
        jsonNode->PutRef(INSPECTOR_ATTRS, std::move(attributes));
    } else if (node->Contains(INSPECTOR_EXTERNAL_KEY_ATTRIBUTES)) {
        TAG_LOGW(AceLogTag::ACE_LAYOUT_INSPECTOR,
            "External subtree node has non-object attributes, $attrs omitted: elementId %{public}d",
            node->GetInt(INSPECTOR_EXTERNAL_KEY_ELEMENT_ID, -1));
    }
}

// Converts the "children" array of one external node and attaches
// "$children" to the converted node: external children are converted
// recursively, native-schema children are spliced verbatim; over-depth and
// non-object children are skipped with warnings.
void PutExternalNodeChildren(const std::unique_ptr<OHOS::Ace::JsonValue>& jsonNode,
    const std::unique_ptr<OHOS::Ace::JsonValue>& node, int32_t depth, const OffsetF& windowOffset)
{
    auto children = node->GetValue(INSPECTOR_EXTERNAL_KEY_CHILDREN);
    bool childrenValid = children != nullptr && children->IsArray();
    if (!childrenValid) {
        if (node->Contains(INSPECTOR_EXTERNAL_KEY_CHILDREN)) {
            TAG_LOGW(AceLogTag::ACE_LAYOUT_INSPECTOR,
                "External subtree node has non-array children, $children omitted: elementId %{public}d",
                node->GetInt(INSPECTOR_EXTERNAL_KEY_ELEMENT_ID, -1));
        }
        return;
    }
    int32_t size = children->GetArraySize();
    if (size == 0) {
        return;
    }
    auto jsonChildrenArray = JsonUtil::CreateArray(true);
    for (int32_t index = 0; index < size; ++index) {
        auto child = children->GetArrayItem(index);
        CHECK_NULL_CONTINUE(child);
        if (IsExternalFormatNode(child)) {
            auto convertedChild = ConvertExternalSubtreeNode(child, depth - 1, windowOffset);
            if (convertedChild == nullptr) {
                TAG_LOGW(AceLogTag::ACE_LAYOUT_INSPECTOR,
                    "External subtree child exceeds max depth %{public}d, skipped: elementId %{public}d",
                    INSPECTOR_EXTERNAL_SUBTREE_MAX_DEPTH, child->GetInt(INSPECTOR_EXTERNAL_KEY_ELEMENT_ID, -1));
                continue;
            }
            jsonChildrenArray->PutRef(std::move(convertedChild));
        } else {
            if (!child->IsObject()) {
                TAG_LOGW(
                    AceLogTag::ACE_LAYOUT_INSPECTOR, "External subtree child is not a JSON object, spliced verbatim");
            }
            jsonChildrenArray->PutRef(std::move(child));
        }
    }
    if (jsonChildrenArray->GetArraySize() > 0) {
        jsonNode->PutRef(INSPECTOR_CHILDREN, std::move(jsonChildrenArray));
    }
}

// Converts one external-format node (recursively) to the inspector node
// schema: elementId -> "$ID", componentType -> "$type", screenRect ->
// "$rect" (screen -> window-relative), attributes -> "$attrs", children ->
// "$children"; "parentId" and unknown fields are dropped. Returns nullptr
// when the max depth is exceeded.
std::unique_ptr<OHOS::Ace::JsonValue> ConvertExternalSubtreeNode(
    const std::unique_ptr<OHOS::Ace::JsonValue>& node, int32_t depth, const OffsetF& windowOffset)
{
    CHECK_NULL_RETURN(node, nullptr);
    if (depth <= 0) {
        return nullptr;
    }
    auto jsonNode = JsonUtil::Create(true);
    PutExternalNodeFields(jsonNode, node, windowOffset);
    PutExternalNodeChildren(jsonNode, node, depth, windowOffset);
    return jsonNode;
}

void SpliceInspectorSubtreeItem(std::unique_ptr<OHOS::Ace::JsonValue> item, const RefPtr<NG::UINode>& parent,
    std::unique_ptr<OHOS::Ace::JsonValue>& jsonChildrenArray)
{
    if (IsExternalFormatNode(item)) {
        OffsetF windowOffset = GetExternalRectWindowOffset(parent);
        auto converted = ConvertExternalSubtreeNode(item, INSPECTOR_EXTERNAL_SUBTREE_MAX_DEPTH, windowOffset);
        if (converted == nullptr) {
            TAG_LOGW(AceLogTag::ACE_LAYOUT_INSPECTOR,
                "Inspector subtree is too deep (max %{public}d), skipped for node %{private}d",
                INSPECTOR_EXTERNAL_SUBTREE_MAX_DEPTH, parent->GetId());
            return;
        }
        item = std::move(converted);
        TAG_LOGD(AceLogTag::ACE_LAYOUT_INSPECTOR,
            "External host-format subtree converted and spliced for node %{private}d", parent->GetId());
    }
    jsonChildrenArray->PutRef(std::move(item));
}

void SpliceInspectorSubtreeJson(const std::string& subtreeJson, const RefPtr<NG::UINode>& parent,
    std::unique_ptr<OHOS::Ace::JsonValue>& jsonChildrenArray)
{
    if (subtreeJson.empty()) {
        return;
    }
    auto parsed = JsonUtil::ParseJsonString(subtreeJson);
    if (parsed == nullptr || !parsed->IsValid()) {
        TAG_LOGW(AceLogTag::ACE_LAYOUT_INSPECTOR, "Inspector subtree is not valid JSON, skipped for node %{private}d",
            parent->GetId());
        return;
    }
    // Per the API contract the payload MUST be a JSON array of node objects;
    // single-object payloads are rejected.
    if (!parsed->IsArray()) {
        TAG_LOGW(AceLogTag::ACE_LAYOUT_INSPECTOR,
            "Inspector subtree is not a JSON array (single objects are not supported), skipped for node %{private}d",
            parent->GetId());
        return;
    }
    int32_t arraySize = parsed->GetArraySize();
    for (int32_t index = 0; index < arraySize; ++index) {
        auto item = parsed->GetArrayItem(index);
        CHECK_NULL_CONTINUE(item);
        SpliceInspectorSubtreeItem(std::move(item), parent, jsonChildrenArray);
    }
}

#if defined(WEB_SUPPORTED) && !defined(ACE_UNITTEST)

// Expected format "[x, y, width, height]": 4 comma-separated values.
constexpr size_t WEB_RECT_COMPONENT_COUNT = 4;

void SkipWebRectWhitespace(const char*& current, const char* end)
{
    while (current != end && std::isspace(static_cast<unsigned char>(*current))) {
        ++current;
    }
}

bool ParseWebRectString(const std::string& rectStr, RectF& rect)
{
    const char* current = rectStr.c_str();
    const char* end = current + rectStr.size();
    SkipWebRectWhitespace(current, end);
    if (current == end || *current != '[') {
        return false;
    }
    ++current;
    std::array<float, WEB_RECT_COMPONENT_COUNT> values {};
    for (size_t index = 0; index < values.size(); ++index) {
        SkipWebRectWhitespace(current, end);
        errno = 0;
        char* parsedEnd = nullptr;
        values[index] = std::strtof(current, &parsedEnd);
        if (parsedEnd == current || errno == ERANGE || !std::isfinite(values[index])) {
            return false;
        }
        current = parsedEnd;
        SkipWebRectWhitespace(current, end);
        char delimiter = index + 1 == values.size() ? ']' : ',';
        if (current == end || *current != delimiter) {
            return false;
        }
        ++current;
    }
    SkipWebRectWhitespace(current, end);
    const auto& [x, y, width, height] = values;
    if (current != end || width < 0.0f || height < 0.0f) {
        return false;
    }
    rect = RectF(x, y, width, height);
    return true;
}

void ConvertWebNodeRect(const std::unique_ptr<OHOS::Ace::JsonValue>& node, const OffsetF& hostOffset)
{
    RectF webRect;
    if (!ParseWebRectString(node->GetString(INSPECTOR_RECT, ""), webRect)) {
        return;
    }
    RectF windowRect(
        webRect.Left() + hostOffset.GetX(), webRect.Top() + hostOffset.GetY(), webRect.Width(), webRect.Height());
    if (!std::isfinite(windowRect.Left()) || !std::isfinite(windowRect.Top()) || !std::isfinite(windowRect.Right()) ||
        !std::isfinite(windowRect.Bottom())) {
        return;
    }
    node->Replace(INSPECTOR_RECT, windowRect.ToBounds().c_str());
}

void ConvertWebSubtreeRects(const std::unique_ptr<OHOS::Ace::JsonValue>& root, const OffsetF& hostOffset)
{
    if (root == nullptr || !root->IsObject()) {
        return;
    }
    ConvertWebNodeRect(root, hostOffset);

    std::vector<std::unique_ptr<OHOS::Ace::JsonValue>> worklist;
    auto pushChildren = [&worklist](const std::unique_ptr<OHOS::Ace::JsonValue>& node) {
        auto children = node->GetValue(INSPECTOR_CHILDREN);
        if (children == nullptr || !children->IsArray()) {
            return;
        }
        int32_t size = children->GetArraySize();
        for (int32_t index = 0; index < size; ++index) {
            auto child = children->GetArrayItem(index);
            if (child != nullptr) {
                worklist.push_back(std::move(child));
            }
        }
    };

    pushChildren(root);
    while (!worklist.empty()) {
        auto node = std::move(worklist.back());
        worklist.pop_back();
        if (node == nullptr || !node->IsObject()) {
            continue;
        }
        // Convert this node's rect.
        ConvertWebNodeRect(node, hostOffset);
        // Queue this node's children.
        pushChildren(node);
    }
}

void SpliceWebInspectorSubtree(const std::string& subtreeJson, const RefPtr<NG::UINode>& parent,
    std::unique_ptr<OHOS::Ace::JsonValue>& jsonChildrenArray)
{
    if (subtreeJson.empty()) {
        return;
    }
    auto parsed = JsonUtil::ParseJsonString(subtreeJson);
    if (parsed == nullptr || !parsed->IsValid()) {
        TAG_LOGW(AceLogTag::ACE_LAYOUT_INSPECTOR, "Web inspector subtree is not valid JSON, skipped");
        return;
    }

    // Compute the Web host's window-relative offset (vp).
    auto frameNode = AceType::DynamicCast<FrameNode>(parent);
    OffsetF hostOffset(0.0f, 0.0f);
    if (frameNode != nullptr) {
        RectF hostRect = frameNode->GetTransformRectRelativeToWindow();
        hostOffset = OffsetF(hostRect.Left(), hostRect.Top());
    }

    // Convert all node rects (iteratively, no recursion).
    ConvertWebSubtreeRects(parsed, hostOffset);

    jsonChildrenArray->PutRef(std::move(parsed));
}

void AppendWebSubtreeToChildren(
    const RefPtr<NG::UINode>& parent, std::unique_ptr<OHOS::Ace::JsonValue>& jsonChildrenArray)
{
    CHECK_NULL_VOID(parent);
    auto frameNode = AceType::DynamicCast<FrameNode>(parent);
    CHECK_NULL_VOID(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    CHECK_NULL_VOID(webPattern);
    std::string subtreeJson = webPattern->GetAccessibilityJsonString(webPattern->GetTreeId());
    if (subtreeJson.empty()) {
        TAG_LOGW(AceLogTag::ACE_LAYOUT_INSPECTOR,
            "Web subtree JSON is empty, "
            "skipped for node %{private}d",
            parent->GetId());
        return;
    }
    if (subtreeJson.size() > INSPECTOR_EXTERNAL_SUBTREE_MAX_BYTES) {
        TAG_LOGW(AceLogTag::ACE_LAYOUT_INSPECTOR,
            "Web subtree exceeds size limit %{public}u bytes, skipped for node %{private}d",
            INSPECTOR_EXTERNAL_SUBTREE_MAX_BYTES, parent->GetId());
        return;
    }
    TAG_LOGD(AceLogTag::ACE_LAYOUT_INSPECTOR, "Web inspector subtree payload %{public}u bytes for node %{private}d",
        static_cast<uint32_t>(subtreeJson.size()), parent->GetId());
    SpliceWebInspectorSubtree(subtreeJson, parent, jsonChildrenArray);
}
#else
void AppendWebSubtreeToChildren(
    const RefPtr<NG::UINode>& parent, std::unique_ptr<OHOS::Ace::JsonValue>& jsonChildrenArray)
{
    (void)parent;
    (void)jsonChildrenArray;
}
#endif
} // namespace

void AppendExternalSubtreeToChildren(const RefPtr<NG::UINode>& parent,
    std::unique_ptr<OHOS::Ace::JsonValue>& jsonChildrenArray, const InspectorFilter& filter)
{
    CHECK_NULL_VOID(parent);
    if (parent->GetTag() == V2::WEB_ETS_TAG) {
        if (filter.IsWebEnable()) {
            AppendWebSubtreeToChildren(parent, jsonChildrenArray);
        }
        return;
    }
    std::vector<InspectorTreeProviderRegistry::GetterEntry> getters;
    InspectorTreeProviderRegistry::GetInspectorGettersForNode(parent, getters);
    if (getters.empty()) {
        return;
    }
    for (const auto& entry : getters) {
        std::string subtreeJson;
        bool oversize = false;
        {
            char* payload = nullptr;
            entry.getter(entry.userData, &payload);
            if (payload != nullptr) {
                size_t length = strnlen(payload, INSPECTOR_EXTERNAL_SUBTREE_MAX_BYTES + 1);
                if (length > INSPECTOR_EXTERNAL_SUBTREE_MAX_BYTES) {
                    oversize = true;
                } else {
                    subtreeJson.assign(payload, length);
                }
                free(payload);
            }
        }
        if (oversize) {
            TAG_LOGW(AceLogTag::ACE_LAYOUT_INSPECTOR,
                "External subtree exceeds size limit %{public}u bytes, skipped for node %{private}d",
                INSPECTOR_EXTERNAL_SUBTREE_MAX_BYTES, parent->GetId());
            continue;
        }
        TAG_LOGD(AceLogTag::ACE_LAYOUT_INSPECTOR, "External subtree payload %{public}u bytes for node %{private}d",
            static_cast<uint32_t>(subtreeJson.size()), parent->GetId());
        SpliceInspectorSubtreeJson(subtreeJson, parent, jsonChildrenArray);
    }
}
} // namespace OHOS::Ace::NG

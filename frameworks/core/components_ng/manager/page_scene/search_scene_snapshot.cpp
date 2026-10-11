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

#include "core/components_ng/manager/page_scene/search_scene_snapshot.h"

#include <utility>

#include "base/log/log_wrapper.h"
#include "base/utils/utf_helper.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/ui_node.h"
#include "core/components_ng/pattern/custom/custom_node_base.h"
#include "core/components_ng/pattern/swiper/swiper_paint_property.h"
#include "core/components_ng/pattern/text_field/text_field_layout_property.h"
#include "core/components_ng/pattern/text_field/text_field_pattern.h"
#include "core/components_ng/property/accessibility_property.h"
#include "core/components_ng/render/render_context.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
namespace {
constexpr float ZERO_RECT_SIZE = 0.0f;
// Keyword source names align with spec R-3 contract:
// text / buttonText / placeholder / value / customName / subtreeText / a11y / inspectorId.
const char SOURCE_PLACEHOLDER[] = "placeholder";
const char SOURCE_VALUE[] = "value";
const char SOURCE_TEXT[] = "text";
const char SOURCE_BUTTON_TEXT[] = "buttonText";
const char SOURCE_A11Y[] = "a11y";
const char SOURCE_CUSTOM_NAME[] = "customName";
const char SOURCE_SUBTREE_TEXT[] = "subtreeText";

void AddSource(SearchSceneNodeFeature& feature, const std::string& name, const std::string& value)
{
    if (!value.empty()) {
        feature.keywordSources.emplace_back(SearchKeywordSource { name, value });
    }
}

void CollectPlaceholderValue(const RefPtr<FrameNode>& frameNode, SearchSceneNodeFeature& feature)
{
    CHECK_NULL_VOID(frameNode);
    auto layoutProperty = frameNode->GetLayoutProperty<TextFieldLayoutProperty>();
    if (layoutProperty && layoutProperty->HasPlaceholder()) {
        auto placeholder = UtfUtils::Str16DebugToStr8(layoutProperty->GetPlaceholderValue());
        AddSource(feature, SOURCE_PLACEHOLDER, placeholder);
    }
    auto textFieldPattern = frameNode->GetPattern<TextFieldPattern>();
    if (textFieldPattern) {
        AddSource(feature, SOURCE_VALUE, textFieldPattern->GetTextValue());
    }
}

void CollectAccessibilitySources(const RefPtr<FrameNode>& frameNode, SearchSceneNodeFeature& feature)
{
    CHECK_NULL_VOID(frameNode);
    auto accessibilityProperty = frameNode->GetAccessibilityProperty<AccessibilityProperty>();
    if (!accessibilityProperty) {
        return;
    }
    // a11y text.
    AddSource(feature, SOURCE_A11Y, accessibilityProperty->GetAccessibilityText());
    AddSource(feature, SOURCE_A11Y, accessibilityProperty->GetAccessibilityDescription());
    // Generic node text / button text (R-3: 文本/按钮文字).
    auto nodeText = accessibilityProperty->GetText();
    if (!nodeText.empty()) {
        auto tag = frameNode->GetTag();
        if (tag == V2::BUTTON_ETS_TAG) {
            AddSource(feature, SOURCE_BUTTON_TEXT, nodeText);
        } else {
            AddSource(feature, SOURCE_TEXT, nodeText);
        }
    }
    // Subtree text.
    AddSource(feature, SOURCE_SUBTREE_TEXT, accessibilityProperty->GetGroupText(true));
}

void CollectCustomName(const RefPtr<FrameNode>& frameNode, SearchSceneNodeFeature& feature)
{
    CHECK_NULL_VOID(frameNode);
    auto customNode = AceType::DynamicCast<CustomNodeBase>(frameNode);
    if (customNode) {
        AddSource(feature, SOURCE_CUSTOM_NAME, customNode->GetJSViewName());
    }
}

bool HasPositiveArea(const RectF& rect)
{
    return rect.Width() > ZERO_RECT_SIZE && rect.Height() > ZERO_RECT_SIZE;
}

// Web and UIExtension do not participate in search recognition.
bool IsSearchExcludedTag(const std::string& tag)
{
    return tag == V2::WEB_ETS_TAG || tag == V2::UI_EXTENSION_COMPONENT_ETS_TAG ||
           tag == V2::UI_EXTENSION_COMPONENT_TAG || tag == V2::UI_EXTENSION_SURFACE_TAG;
}

// Pragmatic clip container (方案2 / O-1): scroll-family containers and nodes
// whose render context has clipEdge enabled clip their content to their bounds.
bool IsSearchClipContainer(const RefPtr<FrameNode>& node)
{
    CHECK_NULL_RETURN(node, false);
    auto renderContext = node->GetRenderContext();
    if (renderContext && renderContext->GetClipEdge() && renderContext->GetClipEdge().value()) {
        return true;
    }
    auto tag = node->GetTag();
    return tag == V2::SCROLL_ETS_TAG || tag == V2::LIST_ETS_TAG ||
           tag == V2::GRID_ETS_TAG || tag == V2::SWIPER_ETS_TAG;
}
} // namespace

SearchSceneSnapshot SearchSceneSnapshotBuilder::Build(
    const std::string& pageName, const std::string& navDestinationName, const std::string& pageUrl,
    const std::vector<SearchWindowContext>& contexts) const
{
    SearchSceneSnapshot snapshot;
    snapshot.pageName = pageName;
    snapshot.navDestinationName = navDestinationName;
    snapshot.pageUrl = pageUrl;
    for (const auto& window : contexts) {
        CollectNode(window, window.root, snapshot);
    }
    return snapshot;
}

void SearchSceneSnapshotBuilder::CollectNode(
    const SearchWindowContext& window, const RefPtr<UINode>& node, SearchSceneSnapshot& snapshot) const
{
    CHECK_NULL_VOID(node);
    auto frameNode = AceType::DynamicCast<FrameNode>(node);
    if (frameNode && IsSearchExcludedTag(frameNode->GetTag())) {
        // Excluded subtrees (Web/UIExtension) are not included in search evidence.
        return;
    }
    if (frameNode && !BuildNodeFeature(window, frameNode, snapshot.nodes)) {
        // Node is invisible or fully clipped: no need to traverse children.
        return;
    }
    for (const auto& child : node->GetChildrenForInspector(true)) {
        CollectNode(window, child, snapshot);
    }
}

SearchSceneSnapshot SearchSceneSnapshotBuilder::BuildLocal(
    const RefPtr<FrameNode>& responseNode, int32_t maxDepth) const
{
    SearchSceneSnapshot snapshot;
    if (!responseNode || maxDepth <= 0) {
        return snapshot;
    }
    SearchWindowContext window { responseNode, 0.0f, 0.0f, false };
    CollectNodeWithDepth(window, responseNode, maxDepth, 0, snapshot);
    return snapshot;
}

void SearchSceneSnapshotBuilder::CollectNodeWithDepth(
    const SearchWindowContext& window, const RefPtr<UINode>& node,
    int32_t maxDepth, int32_t depth, SearchSceneSnapshot& snapshot) const
{
    CHECK_NULL_VOID(node);
    auto frameNode = AceType::DynamicCast<FrameNode>(node);
    if (frameNode) {
        BuildNodeFeature(window, frameNode, snapshot.nodes);
    }
    if (depth >= maxDepth) {
        return;
    }
    if (frameNode && IsSearchExcludedTag(frameNode->GetTag())) {
        return;
    }
    for (const auto& child : node->GetChildrenForInspector(true)) {
        CollectNodeWithDepth(window, child, maxDepth, depth + 1, snapshot);
    }
}

bool SearchSceneSnapshotBuilder::BuildNodeFeature(
    const SearchWindowContext& window, const RefPtr<FrameNode>& frameNode,
    std::vector<SearchSceneNodeFeature>& nodes) const
{
    CHECK_NULL_RETURN(frameNode, false);
    // Final visible rect is relative to the node's own window; the collector
    // already provided the main-window coordinate offset for subwindows.
    RectF rect = frameNode->GetTransformRectRelativeToWindowOnlyVisible(true);
    if (!HasPositiveArea(rect)) {
        return false;
    }
    // Clip against ancestors that actually clip content (scroll containers /
    // clipEdge), so content scrolled out of the visible area does not match
    //. Only ancestors that clip participate as K1..Kn.
    auto ancestor = frameNode->GetAncestorNodeOfFrame(true);
    while (ancestor) {
        if (IsSearchClipContainer(ancestor)) {
            RectF clipRect = ancestor->GetTransformRectRelativeToWindow(true);
            rect = rect.IntersectRectT(clipRect);
            if (!HasPositiveArea(rect)) {
                return false;
            }
        }
        ancestor = ancestor->GetAncestorNodeOfFrame(true);
    }
    rect.SetLeft(rect.GetX() + window.offsetX);
    rect.SetTop(rect.GetY() + window.offsetY);
    if (!HasPositiveArea(rect)) {
        return false;
    }
    // Convert px to vp so that region/maxSize in rule JSON (vp units) can be
    // compared directly with node rects. The paint rect returned above is in
    // physical pixels (px); rule JSON regions/maxSize are authored in vp.
    // For declarative frontend GetViewScale() returns 1.0 (no-op), so the
    // correct px->vp factor is the device density (vp = px / density).
    auto pipeline = NG::PipelineContext::GetCurrentContextSafely();
    double density = pipeline ? pipeline->GetDensity() : 0.0;
    if (density > 0.0) {
        float scale = static_cast<float>(1.0 / density);
        rect.SetLeft(rect.GetX() * scale);
        rect.SetTop(rect.GetY() * scale);
        rect.SetWidth(rect.Width() * scale);
        rect.SetHeight(rect.Height() * scale);
    }

    SearchSceneNodeFeature feature;
    feature.nodeId = frameNode->GetId();
    feature.inspectorId = frameNode->GetInspectorId().value_or("");
    feature.nodeType = frameNode->GetTag();
    feature.visibleRect = rect;
    feature.isSubwindow = window.isSubwindow;
    CollectAccessibilitySources(frameNode, feature);
    CollectCustomName(frameNode, feature);
    CollectPlaceholderValue(frameNode, feature);
    auto swiperPaintProperty = frameNode->GetPaintProperty<SwiperPaintProperty>();
    if (swiperPaintProperty) {
        feature.swiperAutoPlay = swiperPaintProperty->GetAutoPlay().value_or(false);
    }
    nodes.emplace_back(std::move(feature));
    return true;
}

} // namespace OHOS::Ace::NG

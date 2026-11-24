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

#include "core/components_ng/pattern/tabs/tabs_side_bar_pattern.h"

#include <optional>
#include <string>

#include "interfaces/native/ui_input_event.h"

#include "base/geometry/axis.h"
#include "base/geometry/dimension.h"
#include "base/log/ace_checker.h"
#include "base/log/log_wrapper.h"
#include "base/utils/utf_helper.h"
#include "base/utils/utils.h"
#include "core/components/common/layout/constants.h"
#include "core/components/tab_bar/tabs_event.h"
#include "core/components_ng/base/observer_handler.h"
#include "core/components_ng/base/view_stack_model.h"
#include "core/components_ng/event/pan_event.h"
#include "core/components_ng/pattern/divider/divider_layout_property.h"
#include "core/components_ng/pattern/divider/divider_render_property.h"
#include "core/components_ng/pattern/stack/stack_pattern.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/pattern/shape/polygon_model_ng.h"
#include "core/components_ng/pattern/shape/polygon_pattern.h"
#include "core/components_ng/pattern/shape/shape_abstract_model_ng.h"
#include "core/components_ng/pattern/search/bridge/search_custom_modifier.h"
#include "core/components_ng/pattern/swiper/swiper_model.h"
#include "core/components_ng/pattern/swiper/swiper_pattern.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"
#include "core/components_ng/pattern/scroll/scroll_layout_property.h"
#include "core/components_ng/pattern/scroll/scroll_pattern.h"
#include "core/components_ng/pattern/scrollable/scrollable_model_ng.h"
#include "core/components_ng/pattern/scrollable/scrollable_properties.h"
#include "core/components_ng/pattern/tabs/tab_bar_pattern.h"
#include "core/components_ng/pattern/tabs/tab_content_node.h"
#include "core/components_ng/pattern/tabs/tab_content_layout_property.h"
#include "core/components_ng/pattern/tabs/tab_content_pattern.h"
#include "core/components_ng/pattern/tabs/tabs_layout_property.h"
#include "core/components_ng/pattern/tabs/tabs_pattern.h"
#include "core/components_ng/pattern/tabs/tabs_declaration.h"
#include "core/components_ng/pattern/tabs/tabs_controller.h"
#include "core/components_ng/pattern/tabs/tabs_node.h"
#include "core/components_ng/pattern/tabs/tabs_side_bar_tab_list_pattern.h"
#include "core/components_ng/pattern/button/button_pattern.h"
#include "core/components_ng/pattern/text/text_layout_property.h"
#include "core/components_ng/pattern/text/text_pattern.h"
#include "core/components_ng/pattern/symbol/symbol_source_info.h"
#include "core/components_ng/pattern/image/image_pattern.h"
#include "core/image/image_source_info.h"
#include "core/components_ng/pattern/image/image_layout_property.h"
#include "core/components_ng/pattern/image/image_render_property.h"
#include "core/common/resource/resource_manager.h"
#include "core/interfaces/native/node/node_button_modifier.h"
#include "core/components/tab_bar/tab_theme.h"
#include "core/components_ng/property/gradient_property.h"
#include "core/components_ng/property/property.h"
#include "core/components_ng/render/animation_utils.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/components/common/properties/animation_option.h"
#include "core/components/common/properties/shadow_config.h"
#include "core/animation/spring_curve.h"
#include "core/gestures/gesture_info.h"
#include "core/interfaces/native/node/search_modifier.h"
#include "core/components_ng/manager/avoid_info/avoid_info_manager.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
namespace {
constexpr int32_t HEADER_CONTAINER_MASK_BLUR_ZINDEX = 1;
constexpr int32_t HEADER_CONTAINER_MASK_ZINDEX = 2;
constexpr int32_t HEADER_CONTAINER_NODE_ZINDEX = 3;
constexpr int32_t BOTTOM_BAR_CONTAINER_MASK_BLUR_ZINDEX = 1;
constexpr int32_t BOTTOM_BAR_CONTAINER_MASK_ZINDEX = 2;
constexpr int32_t BOTTOM_BAR_CONTAINER_NODE_ZINDEX = 3;
const Dimension DEFAULT_SEARCH_NODE_HEIGHT = 56.0_vp;

// Gradual blur constants (referencing TitleBar's GRADUAL_BLUR parameters)
const Dimension GRADUAL_BLUR_SCROLL_THRESHOLD = 56.0_vp;
const Dimension GRADUAL_BLUR_MAX_RADIUS = 12.0_vp;
constexpr double GRADUAL_BLUR_MAX_OPACITY = 0.8;

const Color DEFAULT_GRADIENT_MASK_BLEND_COLOR = Color(0xFFF1F3F5);
const Dimension COLLAPSE_BUTTON_SIZE = 40.0_vp;
const Dimension COLLAPSE_BUTTON_HORIZONTAL_PADDING = 8.0_vp;
const Dimension TITLE_BAR_ICON_SIZE = 24.0_vp;
const Dimension COLLAPSE_BUTTON_ICON_SIZE = 24.0_vp;
constexpr int32_t COLLAPSE_BUTTON_ZINDEX = 4;
const Dimension TITLE_BAR_ROW_HEIGHT =
    COLLAPSE_BUTTON_SIZE + COLLAPSE_BUTTON_HORIZONTAL_PADDING + COLLAPSE_BUTTON_HORIZONTAL_PADDING;
const Dimension TITLE_BAR_ICON_TEXT_PADDING = 12.0_vp;
const Dimension TITLE_BAR_RIGHT_PADDING_NO_BUTTON = 8.0_vp;
const Dimension TITLE_BAR_BUTTON_AREA_WIDTH = COLLAPSE_BUTTON_SIZE + COLLAPSE_BUTTON_HORIZONTAL_PADDING;

// Collapse icon geometry derived from the design assets close_sidebar.svg /
// open_sidebar.svg in the repo root (24x24 box): the three hamburger bars stay
// static while the arrow triangle rotates 180° on its own.
// Note: Path.commands coordinates are px (not vp), so the bars are plain
// FrameNodes painted via the render context background color/radius
// (Dimension-based, density-safe) instead of a Path node. Shape Rect nodes are
// NOT used here on purpose: the Rect pattern/paint chain lives in the shape
// component library, which some binaries linking this pattern (e.g. the tabs
// unit-test binary) do not link.
constexpr float COLLAPSE_ICON_BOX_VP = 24.0f;
// Local node tags: V2::POLYGON_ETS_TAG is not exported to libarkui_tabswiper
// (missing from its version script), so keep a local copy here; the bars use a
// private tag since they are not real shape nodes.
const char COLLAPSE_ICON_BAR_TAG[] = "CollapseIconButtonBar";
const char COLLAPSE_ICON_POLYGON_TAG[] = "Polygon";
// Hamburger bars in the 24x24 icon box (capsule shape, corner radius = height/2).
struct CollapseIconBar {
    float left;
    float top;
    float width;
    float height;
};
constexpr CollapseIconBar COLLAPSE_ICON_BARS[] = {
    { 2.016f, 4.418f, 19.969f, 1.488f },  // top bar
    { 2.016f, 11.086f, 12.984f, 1.512f }, // middle bar (short, leaves room for the triangle)
    { 2.016f, 17.830f, 19.969f, 1.512f }, // bottom bar
};
// The arrow triangle is a rounded-corner glyph. Both end-state outlines are
// sampled from the SVG assets (9 samples per feature arc: apex / top corner /
// bottom corner, including the arc endpoints; straight edges are implicit
// between consecutive arcs). Each ring holds 27 points in ring-traversal order:
// RING_CLOSE = [apex, topCorner, bottomCorner], RING_OPEN = [apex, bottomCorner,
// topCorner]. The animation morphs between the two rings: vertices roll by
// rotation correspondence (NOT a mirror flip), so when collapsing every vertex
// moves counterclockwise (as seen by the user); expanding reverses to clockwise.
// The node box is the union bbox of both rings; the lerped outline always stays
// inside it (no clipping).
struct CollapseIconPoint {
    float x;
    float y;
};
constexpr int32_t COLLAPSE_ICON_TRIANGLE_SAMPLES_PER_ARC = 9;
constexpr float COLLAPSE_ICON_TRIANGLE_LEFT_VP = 17.566f;
constexpr float COLLAPSE_ICON_TRIANGLE_TOP_VP = 8.615f;
constexpr float COLLAPSE_ICON_TRIANGLE_WIDTH_VP = 4.295f;
constexpr float COLLAPSE_ICON_TRIANGLE_HEIGHT_VP = 5.858f;
constexpr CollapseIconPoint COLLAPSE_ICON_TRIANGLE_RING_CLOSE[] = {
    { 4.031f, 3.431f },
    { 4.147f, 3.327f },
    { 4.229f, 3.209f },
    { 4.278f, 3.075f },
    { 4.295f, 2.928f },
    { 4.278f, 2.780f },
    { 4.229f, 2.646f },
    { 4.147f, 2.528f },
    { 4.031f, 2.424f },
    { 1.008f, 0.121f },
    { 0.860f, 0.036f },
    { 0.697f, 0.000f },
    { 0.521f, 0.012f },
    { 0.357f, 0.064f },
    { 0.213f, 0.157f },
    { 0.096f, 0.285f },
    { 0.024f, 0.441f },
    { 0.000f, 0.625f },
    { 0.000f, 5.230f },
    { 0.024f, 5.415f },
    { 0.096f, 5.573f },
    { 0.213f, 5.704f },
    { 0.357f, 5.797f },
    { 0.521f, 5.849f },
    { 0.697f, 5.858f },
    { 0.860f, 5.820f },
    { 1.008f, 5.734f },
};
constexpr CollapseIconPoint COLLAPSE_ICON_TRIANGLE_RING_OPEN[] = {
    { 0.240f, 2.424f },
    { 0.135f, 2.528f },
    { 0.060f, 2.646f },
    { 0.015f, 2.780f },
    { 0.000f, 2.928f },
    { 0.015f, 3.075f },
    { 0.060f, 3.209f },
    { 0.135f, 3.327f },
    { 0.240f, 3.431f },
    { 3.287f, 5.734f },
    { 3.439f, 5.820f },
    { 3.604f, 5.858f },
    { 3.779f, 5.849f },
    { 3.939f, 5.797f },
    { 4.074f, 5.704f },
    { 4.183f, 5.573f },
    { 4.249f, 5.415f },
    { 4.271f, 5.230f },
    { 4.271f, 0.625f },
    { 4.249f, 0.441f },
    { 4.183f, 0.285f },
    { 4.074f, 0.157f },
    { 3.939f, 0.064f },
    { 3.779f, 0.012f },
    { 3.604f, 0.000f },
    { 3.439f, 0.036f },
    { 3.287f, 0.121f },
};

const std::vector<std::pair<float, float>> MASK_BLUR_STOPS = {
    { 1.0f, 0.0f }, { 0.6f, 0.6f }, { 0.0f, 1.0f }
};

const std::vector<std::pair<float, float>> FADE_OUT_GRADIENT_STOPS = {
    {1.0f, 0.0f}, {1.0f, 0.3f}, {0.99764f, 0.335f}, {0.99010f, 0.370f},
    {0.97627f, 0.405f}, {0.95574f, 0.440f}, {0.92808f, 0.475f},
    {0.89108f, 0.510f}, {0.84375f, 0.545f}, {0.78547f, 0.580f},
    {0.71344f, 0.615f}, {0.63048f, 0.650f}, {0.53513f, 0.685f},
    {0.43280f, 0.720f}, {0.33021f, 0.755f}, {0.23699f, 0.790f},
    {0.15625f, 0.825f}, {0.09588f, 0.860f}, {0.05096f, 0.895f},
    {0.02089f, 0.930f}, {0.00491f, 0.965f}, {0.0f, 1.0f}
};

constexpr int32_t COLLAPSE_ICON_TRIANGLE_ANIM_DURATION = 200; // ms
}

RefPtr<FrameNode> TabsSideBarPattern::CreateCollapseIconButtonBar(
    const Color& color, float left, float top, float width, float height)
{
    auto barNode = FrameNode::CreateFrameNode(
        COLLAPSE_ICON_BAR_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<Pattern>());
    CHECK_NULL_RETURN(barNode, nullptr);
    auto layoutProperty = barNode->GetLayoutProperty();
    CHECK_NULL_RETURN(layoutProperty, nullptr);
    layoutProperty->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(width, DimensionUnit::VP), CalcLength(height, DimensionUnit::VP)));
    // Capsule bar matching the glyph outline: painted with the render context
    // background color, corner radius = half the bar height.
    auto renderContext = barNode->GetRenderContext();
    CHECK_NULL_RETURN(renderContext, nullptr);
    BorderRadiusProperty barRadius;
    barRadius.SetRadius(Dimension(height / 2.0f, DimensionUnit::VP));
    renderContext->UpdateBackgroundColor(color);
    renderContext->UpdateBorderRadius(barRadius);
    // Stack centers each bar; shift it from the centered position to (left, top).
    renderContext->UpdateOffset(OffsetT<Dimension>(
        Dimension(left - (COLLAPSE_ICON_BOX_VP - width) / 2.0f, DimensionUnit::VP),
        Dimension(top - (COLLAPSE_ICON_BOX_VP - height) / 2.0f, DimensionUnit::VP)));
    return barNode;
}

bool TabsSideBarPattern::IsSidebarOnEndSide() const
{
    auto tabsNode = tabsNode_.Upgrade();
    if (!tabsNode) {
        return false;
    }
    auto tabsLayoutProp = tabsNode->GetLayoutProperty<TabsLayoutProperty>();
    auto sideBarPosition =
        tabsLayoutProp ? tabsLayoutProp->GetSidebarPositionValue(BarPosition::START) : BarPosition::START;
    bool isRTL = tabsLayoutProp ? tabsLayoutProp->GetNonAutoLayoutDirection() == TextDirection::RTL : false;
    return (isRTL && sideBarPosition == BarPosition::START) || (!isRTL && sideBarPosition == BarPosition::END);
}

// UX-aligned icon model (2026-11-20): the triangle sits on the side given by
// the reading direction (right in LTR, left in RTL) regardless of
// sidebarPosition, so the whole glyph (bar layout, triangle position, triangle
// outline) is mirrored horizontally in RTL layouts only. The triangle's
// pointing direction is decided separately by the sidebar's on-screen side
// (see UpdateCollapseTrianglePoints).
bool TabsSideBarPattern::IsIconMirroredByRtl() const
{
    auto tabsNode = tabsNode_.Upgrade();
    if (!tabsNode) {
        return false;
    }
    auto tabsLayoutProp = tabsNode->GetLayoutProperty<TabsLayoutProperty>();
    return tabsLayoutProp && tabsLayoutProp->GetNonAutoLayoutDirection() == TextDirection::RTL;
}

void TabsSideBarPattern::UpdateCollapseTrianglePosition(const RefPtr<FrameNode>& triangleNode) const
{
    CHECK_NULL_VOID(triangleNode);
    auto triangleRenderContext = triangleNode->GetRenderContext();
    CHECK_NULL_VOID(triangleRenderContext);
    // Stack centers the node; shift it from the centered position to the union-bbox
    // position of the glyph inside the 24x24 icon box. In RTL layouts the icon is
    // mirrored horizontally: the triangle sits left of the hamburger bars instead
    // of right of them (reading direction decides the triangle side,
    // sidebarPosition does not).
    float triangleLeft = COLLAPSE_ICON_TRIANGLE_LEFT_VP;
    if (IsIconMirroredByRtl()) {
        triangleLeft = COLLAPSE_ICON_BOX_VP - COLLAPSE_ICON_TRIANGLE_LEFT_VP - COLLAPSE_ICON_TRIANGLE_WIDTH_VP;
    }
    triangleRenderContext->UpdateOffset(OffsetT<Dimension>(
        Dimension(triangleLeft - (COLLAPSE_ICON_BOX_VP - COLLAPSE_ICON_TRIANGLE_WIDTH_VP) / 2.0f, DimensionUnit::VP),
        Dimension(COLLAPSE_ICON_TRIANGLE_TOP_VP - (COLLAPSE_ICON_BOX_VP - COLLAPSE_ICON_TRIANGLE_HEIGHT_VP) / 2.0f,
            DimensionUnit::VP)));
}

void TabsSideBarPattern::UpdateCollapseTrianglePoints(
    const RefPtr<FrameNode>& triangleNode, float progress)
{
    CHECK_NULL_VOID(triangleNode);
    // UX-aligned four-quadrant model:
    // progress 0.0 = expanded, 1.0 = collapsed. In VISUAL space the expanded
    // triangle points toward the sidebar's on-screen side and the collapsed
    // triangle away from it. The rings below live in the base (pre-mirror)
    // space, so the expanded ring must be picked per reading direction: the
    // RTL mirroring at the end flips the apex direction, hence
    //   LTR: sidebar visually right (IsSidebarOnEndSide()) -> expanded = RING_CLOSE,
    //        visually left -> expanded = RING_OPEN;
    //   RTL: inverted (the mirror flips the apex back to the visual target).
    // Both rings are ordered as three feature blocks of SAMPLES_PER_ARC points
    // each (ring traversal order, arcs run from one straight edge to the next):
    //   RING_CLOSE = [apex, topCorner, bottomCorner]
    //   RING_OPEN  = [apex, bottomCorner, topCorner]
    // Rotation correspondence (NOT a mirror flip): identical vertex pairs for
    // both directions (CLOSE(begin)->OPEN(end) uses block mapping {2,0,1},
    // OPEN(begin)->CLOSE(end) uses its inverse {1,2,0}), giving the spec'd
    // roll direction: left-pointing -> right-pointing (OPEN->CLOSE, expanding)
    // rolls clockwise, right-pointing -> left-pointing (CLOSE->OPEN,
    // collapsing) rolls counterclockwise.
    bool isEnd = IsSidebarOnEndSide();
    bool mirroredByRtl = IsIconMirroredByRtl();
    // RING_CLOSE has its apex at max-x (points right in base space).
    bool expandedRingClose = (isEnd != mirroredByRtl);
    const CollapseIconPoint* beginRing =
        expandedRingClose ? COLLAPSE_ICON_TRIANGLE_RING_CLOSE : COLLAPSE_ICON_TRIANGLE_RING_OPEN;
    const CollapseIconPoint* endRing =
        expandedRingClose ? COLLAPSE_ICON_TRIANGLE_RING_OPEN : COLLAPSE_ICON_TRIANGLE_RING_CLOSE;
    static constexpr int32_t CLOSE_TO_OPEN_BLOCK[3] = { 2, 0, 1 };
    static constexpr int32_t OPEN_TO_CLOSE_BLOCK[3] = { 1, 2, 0 };
    const int32_t* targetBlock = expandedRingClose ? CLOSE_TO_OPEN_BLOCK : OPEN_TO_CLOSE_BLOCK;
    const float mirroredWidth = COLLAPSE_ICON_TRIANGLE_WIDTH_VP;
    ShapePoints points;
    points.reserve(std::size(COLLAPSE_ICON_TRIANGLE_RING_CLOSE));
    constexpr int32_t BLOCK_COUNT = 3;
    for (int32_t block = 0; block < BLOCK_COUNT; ++block) {
        for (int32_t idx = 0; idx < COLLAPSE_ICON_TRIANGLE_SAMPLES_PER_ARC; ++idx) {
            const auto& beginPoint = beginRing[block * COLLAPSE_ICON_TRIANGLE_SAMPLES_PER_ARC + idx];
            const auto& endPoint = endRing[targetBlock[block] * COLLAPSE_ICON_TRIANGLE_SAMPLES_PER_ARC + idx];
            float x = beginPoint.x + (endPoint.x - beginPoint.x) * progress;
            // RTL layouts mirror the whole glyph horizontally: the triangle
            // outline flips inside its own box.
            if (mirroredByRtl) {
                x = mirroredWidth - x;
            }
            points.emplace_back(
                Dimension(x, DimensionUnit::VP),
                Dimension(beginPoint.y + (endPoint.y - beginPoint.y) * progress, DimensionUnit::VP));
        }
    }
    PolygonModelNG::SetPoints(AceType::RawPtr(triangleNode), points);
    triangleNode->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
}

void TabsSideBarPattern::AnimateCollapseButtonIconRotation(bool isCollapse)
{
    CHECK_NULL_VOID(collapseButtonTriangleNode_);
    auto triangleNode = collapseButtonTriangleNode_;
    auto context = triangleNode->GetContextRefPtr();
    CHECK_NULL_VOID(context);
    // Only the arrow triangle morphs (rotation correspondence between the two
    // rounded outlines); the hamburger bars stay static.
    // progress: 0.0 = expanded triangle, 1.0 = collapsed triangle.
    const std::string progressPropertyName = "collapse_button_triangle_progress";
    float begin = isCollapse ? 0.0f : 1.0f;
    float end = isCollapse ? 1.0f : 0.0f;
    auto weakPattern = AceType::WeakClaim(this);
    triangleNode->CreateAnimatablePropertyFloat(progressPropertyName, begin,
        [weakTriangle = AceType::WeakClaim(AceType::RawPtr(triangleNode)), weakPattern](float value) {
            auto triangle = weakTriangle.Upgrade();
            CHECK_NULL_VOID(triangle);
            auto pattern = weakPattern.Upgrade();
            CHECK_NULL_VOID(pattern);
            pattern->UpdateCollapseTrianglePoints(triangle, value);
        });
    // Reset to the begin value before starting so repeated toggles animate from the
    // correct origin instead of the previous animation's end value.
    triangleNode->UpdateAnimatablePropertyFloat(progressPropertyName, begin);
    AnimationOption option;
    option.SetDuration(COLLAPSE_ICON_TRIANGLE_ANIM_DURATION);
    option.SetCurve(Curves::LINEAR);
    option.SetFillMode(FillMode::FORWARDS);
    AnimationUtils::StartAnimation(option,
        [weakTriangle = AceType::WeakClaim(AceType::RawPtr(triangleNode)), end, progressPropertyName]() {
            auto triangle = weakTriangle.Upgrade();
            CHECK_NULL_VOID(triangle);
            triangle->UpdateAnimatablePropertyFloat(progressPropertyName, end);
        },
        nullptr, nullptr, context);
}

void TabsSideBarPattern::UpdateCollapseButtonBarPositions()
{
    CHECK_NULL_VOID(collapseButtonIconNode_);
    bool mirroredByRtl = IsIconMirroredByRtl();
    int32_t barIndex = 0;
    for (const auto& child : collapseButtonIconNode_->GetChildren()) {
        auto barNode = AceType::DynamicCast<FrameNode>(child);
        if (!barNode || barNode->GetTag() != COLLAPSE_ICON_BAR_TAG ||
            barIndex >= static_cast<int32_t>(std::size(COLLAPSE_ICON_BARS))) {
            continue;
        }
        const auto& bar = COLLAPSE_ICON_BARS[barIndex++];
        float left = mirroredByRtl ? (COLLAPSE_ICON_BOX_VP - bar.left - bar.width) : bar.left;
        auto renderContext = barNode->GetRenderContext();
        CHECK_NULL_VOID(renderContext);
        // Same centering shift as at creation: Stack centers each bar, shift it
        // from the centered position to (left, top).
        renderContext->UpdateOffset(OffsetT<Dimension>(
            Dimension(left - (COLLAPSE_ICON_BOX_VP - bar.width) / 2.0f, DimensionUnit::VP),
            Dimension(bar.top - (COLLAPSE_ICON_BOX_VP - bar.height) / 2.0f, DimensionUnit::VP)));
    }
}

void TabsSideBarPattern::CreateHeaderContainerIfNeeded()
{
    if (headerContainerNode_) {
        return;
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto columnNode = FrameNode::GetOrCreateFrameNode(
        V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        []() { return AceType::MakeRefPtr<LinearLayoutPattern>(true); });
    CHECK_NULL_VOID(columnNode);
    auto property = columnNode->GetLayoutProperty<LinearLayoutProperty>();
    CHECK_NULL_VOID(property);
    property->UpdateFlexDirection(FlexDirection::COLUMN);
    // Expand header background/blur into status bar (top) and navigation bar (bottom).
    property->UpdateSafeAreaExpandOpts(
        { .type = SAFE_AREA_TYPE_SYSTEM, .edges = SAFE_AREA_EDGE_TOP + SAFE_AREA_EDGE_BOTTOM });
    auto renderContext = columnNode->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    renderContext->UpdateZIndex(HEADER_CONTAINER_NODE_ZINDEX);
    headerContainerNode_ = columnNode;
    columnNode->MountToParent(host);
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
}

void TabsSideBarPattern::CreateBottomBarContainerIfNeeded()
{
    if (bottomBarContainerNode_) {
        return;
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto columnNode = FrameNode::GetOrCreateFrameNode(
        V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        []() { return AceType::MakeRefPtr<LinearLayoutPattern>(true); });
    CHECK_NULL_VOID(columnNode);
    auto property = columnNode->GetLayoutProperty<LinearLayoutProperty>();
    CHECK_NULL_VOID(property);
    property->UpdateFlexDirection(FlexDirection::COLUMN);
    // Expand bottom bar background/blur into status bar (top) and navigation bar (bottom).
    property->UpdateSafeAreaExpandOpts(
        { .type = SAFE_AREA_TYPE_SYSTEM, .edges = SAFE_AREA_EDGE_TOP + SAFE_AREA_EDGE_BOTTOM });
    auto renderContext = columnNode->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    renderContext->UpdateZIndex(BOTTOM_BAR_CONTAINER_NODE_ZINDEX);
    bottomBarContainerNode_ = columnNode;
    columnNode->MountToParent(host);
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
}

void TabsSideBarPattern::CreateMaskNodeIfNeeded()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    do {
        if (headerContainerMaskBlurNode_) {
            break;
        }
        auto maskBlurNode = CreateEffectNode("TabsSideBarHeaderMaskBlur");
        CHECK_NULL_BREAK(maskBlurNode);
        auto property = maskBlurNode->GetLayoutProperty();
        CHECK_NULL_BREAK(property);
        property->UpdateVisibility(VisibleType::INVISIBLE);
        // Expand mask blur into status bar (top) and navigation bar (bottom).
        property->UpdateSafeAreaExpandOpts(
            { .type = SAFE_AREA_TYPE_SYSTEM, .edges = SAFE_AREA_EDGE_TOP + SAFE_AREA_EDGE_BOTTOM });
        auto maskBlurRenderContext = maskBlurNode->GetRenderContext();
        CHECK_NULL_BREAK(maskBlurRenderContext);
        maskBlurRenderContext->UpdateZIndex(HEADER_CONTAINER_MASK_BLUR_ZINDEX);
        headerContainerMaskBlurNode_ = maskBlurNode;
        maskBlurNode->MountToParent(host);
    } while (false);
    if (headerContainerMaskNode_) {
        return;
    }
    auto maskNode = CreateEffectNode("TabsSideBarHeaderMask");
    CHECK_NULL_VOID(maskNode);
    auto property = maskNode->GetLayoutProperty();
    CHECK_NULL_VOID(property);
    property->UpdateVisibility(VisibleType::INVISIBLE);
    // Expand mask into status bar (top) and navigation bar (bottom).
    property->UpdateSafeAreaExpandOpts(
        { .type = SAFE_AREA_TYPE_SYSTEM, .edges = SAFE_AREA_EDGE_TOP + SAFE_AREA_EDGE_BOTTOM });
    auto maskRenderContext = maskNode->GetRenderContext();
    CHECK_NULL_VOID(maskRenderContext);
    maskRenderContext->UpdateZIndex(HEADER_CONTAINER_MASK_ZINDEX);
    headerContainerMaskNode_ = maskNode;
    maskNode->MountToParent(host);
}

void TabsSideBarPattern::CreateBottomBarMaskNodeIfNeeded()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    do {
        if (bottomBarMaskBlurNode_) {
            break;
        }
        auto maskBlurNode = CreateEffectNode("TabsSideBarBottomBarMaskBlur");
        CHECK_NULL_BREAK(maskBlurNode);
        auto property = maskBlurNode->GetLayoutProperty();
        CHECK_NULL_BREAK(property);
        property->UpdateVisibility(VisibleType::INVISIBLE);
        // Expand mask blur into status bar (top) and navigation bar (bottom).
        property->UpdateSafeAreaExpandOpts(
            { .type = SAFE_AREA_TYPE_SYSTEM, .edges = SAFE_AREA_EDGE_TOP + SAFE_AREA_EDGE_BOTTOM });
        auto maskBlurRenderContext = maskBlurNode->GetRenderContext();
        CHECK_NULL_BREAK(maskBlurRenderContext);
        maskBlurRenderContext->UpdateZIndex(BOTTOM_BAR_CONTAINER_MASK_BLUR_ZINDEX);
        bottomBarMaskBlurNode_ = maskBlurNode;
        maskBlurNode->MountToParent(host);
    } while (false);
    if (bottomBarMaskNode_) {
        return;
    }
    auto maskNode = CreateEffectNode("TabsSideBarBottomBarMask");
    CHECK_NULL_VOID(maskNode);
    auto property = maskNode->GetLayoutProperty();
    CHECK_NULL_VOID(property);
    property->UpdateVisibility(VisibleType::INVISIBLE);
    // Expand mask into status bar (top) and navigation bar (bottom).
    property->UpdateSafeAreaExpandOpts(
        { .type = SAFE_AREA_TYPE_SYSTEM, .edges = SAFE_AREA_EDGE_TOP + SAFE_AREA_EDGE_BOTTOM });
    auto maskRenderContext = maskNode->GetRenderContext();
    CHECK_NULL_VOID(maskRenderContext);
    maskRenderContext->UpdateZIndex(BOTTOM_BAR_CONTAINER_MASK_ZINDEX);
    bottomBarMaskNode_ = maskNode;
    maskNode->MountToParent(host);
}

RefPtr<FrameNode> TabsSideBarPattern::CreateSearchContainer()
{
    auto containerNode = FrameNode::GetOrCreateFrameNode(V2::STACK_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<StackPattern>(); });
    CHECK_NULL_RETURN(containerNode, nullptr);
    auto property = containerNode->GetLayoutProperty();
    CHECK_NULL_RETURN(property, nullptr);
    // get previously user defined ideal width
    property->MarkUserDefinedHeightConfigured();
    std::optional<CalcLength> width = std::nullopt;
    auto&& layoutConstraint = property->GetCalcLayoutConstraint();
    if (layoutConstraint && layoutConstraint->selfIdealSize) {
        width = layoutConstraint->selfIdealSize->Width();
    }
    std::optional<CalcLength> height = CalcLength(DEFAULT_SEARCH_NODE_HEIGHT);
    property->UpdateUserDefinedIdealSize(CalcSize(width, height));
    property->UpdateAlignment(Alignment::CENTER);
    return containerNode;
}

void TabsSideBarPattern::CreateChildNodeIfNeeded(const RefPtr<FrameNode>& tabsNode)
{
    CreateHeaderContainerIfNeeded();
    CreateMaskNodeIfNeeded();
    CreateTabListIfNeeded(tabsNode);
    CreateBottomBarContainerIfNeeded();
    CreateBottomBarMaskNodeIfNeeded();
}

void TabsSideBarPattern::CreateTabListIfNeeded(const RefPtr<FrameNode>& tabsNode)
{
    if (tabListNode_) {
        return;
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);

    // Create SideBarTabListNode with TabsSideBarTabListPattern (NOT TabBarPattern)
    auto tabListNode = FrameNode::GetOrCreateFrameNode(
        V2::TABS_SIDE_BAR_TAB_LIST_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        []() { return AceType::MakeRefPtr<TabsSideBarTabListPattern>(); });
    CHECK_NULL_VOID(tabListNode);
    auto tabListPattern = tabListNode->GetPattern<TabsSideBarTabListPattern>();
    CHECK_NULL_VOID(tabListPattern);
    tabListPattern->SetTabsNode(tabsNode);
    tabsNode_ = tabsNode;

    // Create ScrollNode (vertical scrolling)
    auto scrollNode = FrameNode::GetOrCreateFrameNode(
        V2::SCROLL_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        []() { return AceType::MakeRefPtr<ScrollPattern>(); });
    CHECK_NULL_VOID(scrollNode);
    auto scrollPattern = scrollNode->GetPattern<ScrollPattern>();
    CHECK_NULL_VOID(scrollPattern);
    scrollPattern->SetEdgeEffect(EdgeEffect::SPRING, true, EffectEdge::ALL);
    scrollPattern->SetNeedFullSafeArea(true);
    auto scrollLayoutProperty = scrollNode->GetLayoutProperty<ScrollLayoutProperty>();
    if (scrollLayoutProperty) {
        scrollLayoutProperty->UpdateAxis(Axis::VERTICAL);
        // Align content to top when Column is shorter than Scroll viewport
        // (Scroll defaults to CENTER alignment).
        scrollLayoutProperty->UpdateAlignment(Alignment::TOP_CENTER);
    }
    auto controller = scrollPattern->GetOrCreatePositionController();
    CHECK_NULL_VOID(controller);
    ScrollerObserver observer;
    observer.onDidScrollEvent = [weakPattern = WeakClaim(this), weakScrollPattern = WeakPtr(scrollPattern)](
        Dimension, ScrollSource, bool, bool) {
        auto pattern = weakPattern.Upgrade();
        CHECK_NULL_VOID(pattern);
        auto scrollPattern = weakScrollPattern.Upgrade();
        CHECK_NULL_VOID(scrollPattern);
        float totalOffset = static_cast<float>(scrollPattern->GetTotalOffset());
        float scrollableDistance = scrollPattern->GetScrollableDistance();
        pattern->OnTabListScroll(totalOffset, scrollableDistance);
    };
    controller->SetObserver(observer);
    // Hide scrollbar for sidebar tab list
    ScrollableModelNG::SetScrollBarMode(AceType::RawPtr(scrollNode), DisplayMode::OFF);
    // clipContent(SAFE_AREA): expand clip rect to include safeAreaPadding area,
    // so scroll content renders behind the header.
    ScrollableModelNG::SetContentClip(AceType::RawPtr(scrollNode),
        ContentClipMode::SAFE_AREA, nullptr);

    // Create content container (parent of item column and footer container, goes inside Scroll)
    auto contentContainer = FrameNode::GetOrCreateFrameNode(
        V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        []() { return AceType::MakeRefPtr<LinearLayoutPattern>(true); });
    CHECK_NULL_VOID(contentContainer);
    auto contentLayoutProperty = contentContainer->GetLayoutProperty<LinearLayoutProperty>();
    if (contentLayoutProperty) {
        contentLayoutProperty->UpdateFlexDirection(FlexDirection::COLUMN);
        contentLayoutProperty->UpdateMeasureType(MeasureType::MATCH_PARENT_CROSS_AXIS);
        contentLayoutProperty->UpdateCrossAxisAlign(FlexAlign::FLEX_START);
    }

    // Create ColumnNode (vertical container for tab items)
    auto columnNode = FrameNode::GetOrCreateFrameNode(
        V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        []() { return AceType::MakeRefPtr<LinearLayoutPattern>(true); });
    CHECK_NULL_VOID(columnNode);
    auto columnLayoutProperty = columnNode->GetLayoutProperty<LinearLayoutProperty>();
    if (columnLayoutProperty) {
        columnLayoutProperty->UpdateFlexDirection(FlexDirection::COLUMN);
        // Column fills parent width so tab items can be left-aligned
        // within the full width. Height remains content-based for scrolling.
        columnLayoutProperty->UpdateMeasureType(MeasureType::MATCH_PARENT_CROSS_AXIS);
        // Children left-aligned (default, explicit for clarity)
        columnLayoutProperty->UpdateCrossAxisAlign(FlexAlign::FLEX_START);
    }
    tabListPattern->SetColumnNode(columnNode);

    // Create footer container (sibling of item column inside content container)
    auto footerContainer = FrameNode::GetOrCreateFrameNode(
        V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        []() { return AceType::MakeRefPtr<LinearLayoutPattern>(true); });
    CHECK_NULL_VOID(footerContainer);
    auto footerLayoutProperty = footerContainer->GetLayoutProperty<LinearLayoutProperty>();
    if (footerLayoutProperty) {
        footerLayoutProperty->UpdateFlexDirection(FlexDirection::COLUMN);
        footerLayoutProperty->UpdateMeasureType(MeasureType::MATCH_PARENT_CROSS_AXIS);
        footerLayoutProperty->UpdateCrossAxisAlign(FlexAlign::FLEX_START);
    }
    footerContainerNode_ = footerContainer;

    // Assemble: columnNode + footerContainer -> contentContainer -> Scroll
    //           -> SideBarTabListNode -> SideBarNode(host)
    columnNode->MountToParent(contentContainer);
    footerContainer->MountToParent(contentContainer);
    contentContainer->MountToParent(scrollNode);
    scrollNode->MountToParent(tabListNode);
    tabListNode->MountToParent(host);
    tabListNode_ = tabListNode;
    columnNode->MarkModifyDone();
    scrollNode->MarkModifyDone();
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
}

void TabsSideBarPattern::UpdateTabListIfNeeded()
{
    // Bind SwiperController (shared with bottom TabBar)
    CHECK_NULL_VOID(tabListNode_);
    auto tabListPattern = tabListNode_->GetPattern<TabsSideBarTabListPattern>();
    CHECK_NULL_VOID(tabListPattern);
    tabListPattern->SetSwiperController(swiperController_);
}

void TabsSideBarPattern::UpdateHeaderNodeIfNeeded()
{
    CHECK_NULL_VOID(headerContainerNode_);
    if (headerNode_ == curHeaderNode_) {
        return;
    }

    // Remove old header
    if (curHeaderNode_) {
        headerContainerNode_->RemoveChild(curHeaderNode_);
        headerContainerNode_->MarkNeedSyncRenderTree();
        curHeaderNode_ = nullptr;
    }

    // Mount new header after title bar row (title bar row stays at index 0)
    if (headerNode_) {
        int32_t insertIndex = titleBarRowNode_ ? 1 : 0;
        headerNode_->MountToParent(headerContainerNode_, insertIndex);
        curHeaderNode_ = headerNode_;
    }
    headerContainerNode_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
}

void TabsSideBarPattern::UpdateFooterNodeIfNeeded()
{
    CHECK_NULL_VOID(footerContainerNode_);

    if (footerNode_ == curFooterNode_) {
        return;
    }
    // Remove old footer
    if (curFooterNode_) {
        footerContainerNode_->RemoveChild(curFooterNode_);
        footerContainerNode_->MarkNeedSyncRenderTree();
        curFooterNode_ = nullptr;
    }
    // Mount new footer to footer container
    if (footerNode_) {
        footerNode_->MountToParent(footerContainerNode_);
        curFooterNode_ = footerNode_;
    }
    footerContainerNode_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
}

void TabsSideBarPattern::UpdateBottomBarNodeIfNeeded()
{
    CHECK_NULL_VOID(bottomBarContainerNode_);
    if (bottomBarNode_ == curBottomBarNode_) {
        return;
    }
    // Remove old bottomBar
    if (curBottomBarNode_) {
        bottomBarContainerNode_->RemoveChild(curBottomBarNode_);
        bottomBarContainerNode_->MarkNeedSyncRenderTree();
        curBottomBarNode_ = nullptr;
    }
    // Mount new bottomBar
    if (bottomBarNode_) {
        bottomBarNode_->MountToParent(bottomBarContainerNode_);
        curBottomBarNode_ = bottomBarNode_;
    }
    bottomBarContainerNode_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
}

void TabsSideBarPattern::UpdateSearchNodeIfNeeded()
{
    CHECK_NULL_VOID(headerContainerNode_);
    if (!incommingOptions_.has_value()) {
        return;
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto newOptions = incommingOptions_.value();
    incommingOptions_ = std::nullopt;

    if (newOptions.isNull) {
        bool needClearSearchFilter = false;
        if (searchContainerNode_) {
            needClearSearchFilter = true;
            headerContainerNode_->RemoveChild(searchContainerNode_);
            searchContainerNode_ = nullptr;
            headerContainerNode_->MarkNeedSyncRenderTree();
            headerContainerNode_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
        }
        searchableOptions_ = newOptions;
        if (!needClearSearchFilter) {
            return;
        }
        // Clear active search state and reset tab visibility —
        // without this, tabs remain filtered by the previous search text.
        CHECK_NULL_VOID(tabListNode_);
        auto tabListPattern = tabListNode_->GetPattern<TabsSideBarTabListPattern>();
        CHECK_NULL_VOID(tabListPattern);
        tabListPattern->ApplySearchFilter(nullptr, u"");
        return;
    }
    bool needRecreateSearchNode = searchableOptions_ != newOptions;
    searchableOptions_ = newOptions;
    if (!needRecreateSearchNode) {
        return;
    }

    std::optional<std::u16string> text;
    if (newOptions.searchText.has_value()) {
        text = UtfUtils::Str8DebugToStr16(newOptions.searchText.value());
    }
    std::optional<std::u16string> placeholder;
    if (newOptions.placeholder.has_value()) {
        placeholder = UtfUtils::Str8DebugToStr16(newOptions.placeholder.value());
    }
    ArkUISearchCreateResourceParams searchResParams;
    searchResParams.stringValueRawPtr = AceType::RawPtr(newOptions.searchTextResObj);
    searchResParams.placeholderRawPtr = AceType::RawPtr(newOptions.placeholderResObj);
    searchResParams.parseValueResult = newOptions.searchTextResObj != nullptr;
    searchResParams.parsePlaceholderResult = newOptions.placeholderResObj != nullptr;
    auto customModifier = NodeModifier::GetSearchCustomModifier();
    CHECK_NULL_VOID(customModifier);
    RefPtr<FrameNode> searchNode = nullptr;
    {
        std::optional<std::string> icon;
        ScopedViewStackProcessor scopedViewStackProcessor;
        customModifier->createNormalSearch(text, placeholder, icon, &searchResParams);
        searchNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
        CHECK_NULL_VOID(searchNode);
    }
    customModifier->setOnChangeEvent(AceType::RawPtr(searchNode),
        [weakPattern = WeakClaim(this)](const std::u16string& newStr) {
            auto pattern = weakPattern.Upgrade();
            CHECK_NULL_VOID(pattern);
            pattern->OnSearchChange(newStr);
        });
    // Remove old search container before creating new one
    if (searchContainerNode_) {
        headerContainerNode_->RemoveChild(searchContainerNode_);
        searchContainerNode_ = nullptr;
        headerContainerNode_->MarkNeedSyncRenderTree();
    }
    auto searchContainerNode = CreateSearchContainer();
    CHECK_NULL_VOID(searchContainerNode);
    searchNode->MountToParent(searchContainerNode);
    searchContainerNode->MountToParent(headerContainerNode_);
    searchContainerNode_ = searchContainerNode;
    headerContainerNode_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
}

void TabsSideBarPattern::OnSearchChange(const std::u16string& newText)
{
    CHECK_NULL_VOID(searchContainerNode_);
    if (searchableOptions_.isNull) {
        return;
    }
    auto text = UtfUtils::Str16ToStr8(newText);
    auto onChangeCallback = searchableOptions_.searchCallback;
    auto searchFilter = searchableOptions_.searchFilter;
    if (onChangeCallback) {
        onChangeCallback(text);
    }
    CHECK_NULL_VOID(tabListNode_);
    auto tabListPattern = tabListNode_->GetPattern<TabsSideBarTabListPattern>();
    CHECK_NULL_VOID(tabListPattern);
    tabListPattern->ApplySearchFilter(searchFilter, newText);
}

void TabsSideBarPattern::OnModifyDone()
{
    Pattern::OnModifyDone();

    UpdateTabListIfNeeded();
    UpdateHeaderNodeIfNeeded();
    UpdateFooterNodeIfNeeded();
    UpdateSearchNodeIfNeeded();
    UpdateBottomBarNodeIfNeeded();
    bool isHeaderContainerVisible;
    bool isScrollEffectEnabled;
    if (!curHeaderNode_ && !searchContainerNode_ && !collapseButtonNode_ && !titleBarRowNode_) {
        isScrollEffectEnabled = false;
        isHeaderContainerVisible = false;
    } else {
        isScrollEffectEnabled = true;
        isHeaderContainerVisible = true;
    }
    if (headerContainerNode_) {
        auto headerContainerProperty = headerContainerNode_->GetLayoutProperty();
        if (headerContainerProperty) {
            headerContainerProperty->UpdateVisibility(
                isHeaderContainerVisible ? VisibleType::VISIBLE : VisibleType::GONE);
        }
    }
    if (bottomBarContainerNode_) {
        auto bottomBarProperty = bottomBarContainerNode_->GetLayoutProperty();
        if (bottomBarProperty) {
            bool isBottomBarVisible = curBottomBarNode_ != nullptr;
            bottomBarProperty->UpdateVisibility(
                isBottomBarVisible ? VisibleType::VISIBLE : VisibleType::GONE);
        }
    }
    InitHeaderContainerScrollEffect(isScrollEffectEnabled);

    bool isBottomBarScrollEffect = curBottomBarNode_ != nullptr;
    InitBottomBarScrollEffect(isBottomBarScrollEffect);

    // Refresh collapse button icon in case sidebarPosition or layout direction
    // changed without a collapse/expand action (e.g., developer updates
    // sidebarPosition at runtime). Without this, the icon stays stale until
    // the next collapse/expand click.
    if (collapseButtonIconNode_) {
        UpdateCollapseButtonIcon();
    }

    // Update title bar content (icon + text) based on latest sidebarTitleBar options.
    UpdateTitleBarContent();
}

RefPtr<FrameNode> TabsSideBarPattern::CreateEffectNode(const std::string& tag)
{
    auto node = FrameNode::CreateFrameNode(
        tag, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    // Effect nodes don't participate in hit test
    CHECK_NULL_RETURN(node, nullptr);
    auto gestureHub = node->GetOrCreateGestureEventHub();
    CHECK_NULL_RETURN(gestureHub, nullptr);
    gestureHub->SetHitTestMode(HitTestMode::HTMNONE);
    return node;
}

void TabsSideBarPattern::OnTabListScroll(float totalOffset, float scrollableDistance)
{
    auto threshold = static_cast<float>(GRADUAL_BLUR_SCROLL_THRESHOLD.ConvertToPx());
    float scrollScale = (threshold > 0.0f) ? std::clamp(totalOffset / threshold, 0.0f, 1.0f) : 0.0f;
    UpdateHeaderContainerBlurStyle(scrollScale);

    float exceed = scrollableDistance - totalOffset;
    float bottomScrollScale = (threshold > 0.0f && exceed > 0.0f)
        ? std::clamp(exceed / threshold, 0.0f, 1.0f)
        : 0.0f;
    UpdateBottomBarBlurStyle(bottomScrollScale);
}

void TabsSideBarPattern::InitHeaderContainerScrollEffect(bool isScrollEffectEnabled)
{
    InitScrollEffectImpl(isScrollEffectEnabled, isScrollEffectEnabled_, scrollScale_,
        headerContainerMaskBlurNode_, headerContainerMaskNode_, GradientDirection::BOTTOM);
}

void TabsSideBarPattern::InitBottomBarScrollEffect(bool isScrollEffectEnabled)
{
    InitScrollEffectImpl(isScrollEffectEnabled, isBottomBarScrollEffectEnabled_, bottomBarScrollScale_,
        bottomBarMaskBlurNode_, bottomBarMaskNode_, GradientDirection::TOP);
}

void TabsSideBarPattern::InitScrollEffectImpl(bool isScrollEffectEnabled, bool& isEnabledFlag,
    float& cachedScale, const RefPtr<FrameNode>& maskBlurNode, const RefPtr<FrameNode>& maskNode,
    GradientDirection direction)
{
    if (isScrollEffectEnabled == isEnabledFlag) {
        return;
    }
    isEnabledFlag = isScrollEffectEnabled;
    do {
        CHECK_NULL_BREAK(maskBlurNode);
        auto property = maskBlurNode->GetLayoutProperty();
        CHECK_NULL_BREAK(property);
        property->UpdateVisibility(isEnabledFlag ? VisibleType::VISIBLE : VisibleType::INVISIBLE);
    } while (false);
    do {
        CHECK_NULL_BREAK(maskNode);
        auto property = maskNode->GetLayoutProperty();
        CHECK_NULL_BREAK(property);
        property->UpdateVisibility(isEnabledFlag ? VisibleType::VISIBLE : VisibleType::INVISIBLE);
    } while (false);
    if (!isEnabledFlag) {
        return;
    }
    auto scale = std::clamp(cachedScale, 0.0f, 1.0f);
    UpdateBlurStyleImpl(scale, cachedScale, isEnabledFlag, maskBlurNode, maskNode, direction);
}

void TabsSideBarPattern::UpdateHeaderContainerBlurStyle(float scrollScale)
{
    UpdateBlurStyleImpl(scrollScale, scrollScale_, isScrollEffectEnabled_,
        headerContainerMaskBlurNode_, headerContainerMaskNode_, GradientDirection::BOTTOM);
}

void TabsSideBarPattern::UpdateBottomBarBlurStyle(float scrollScale)
{
    UpdateBlurStyleImpl(scrollScale, bottomBarScrollScale_, isBottomBarScrollEffectEnabled_,
        bottomBarMaskBlurNode_, bottomBarMaskNode_, GradientDirection::TOP);
}

void TabsSideBarPattern::UpdateBlurStyleImpl(float scrollScale, float& cachedScale, bool isEnabled,
    const RefPtr<FrameNode>& maskBlurNode, const RefPtr<FrameNode>& maskNode,
    GradientDirection direction)
{
    if (NearEqual(cachedScale, scrollScale)) {
        return;
    }
    cachedScale = scrollScale;
    if (!isEnabled) {
        return;
    }
    UpdateBlurStyleImplInner(scrollScale, maskBlurNode, maskNode, direction);
}

void TabsSideBarPattern::UpdateBlurStyleImplInner(
    float scrollScale, const RefPtr<FrameNode>& maskBlurNode, const RefPtr<FrameNode>& maskNode,
    GradientDirection direction)
{
    CHECK_NULL_VOID(maskBlurNode && maskNode);
    auto maskBlurRenderContext = maskBlurNode->GetRenderContext();
    auto maskRenderContext = maskNode->GetRenderContext();
    CHECK_NULL_VOID(maskBlurRenderContext && maskRenderContext);

    float blurRadius = scrollScale * static_cast<float>(GRADUAL_BLUR_MAX_RADIUS.ConvertToPx());
    maskBlurRenderContext->UpdateBackgroundColor(Color::TRANSPARENT);
    if (NearZero(blurRadius)) {
        maskBlurRenderContext->UpdateBackBlurRadius(Dimension(0.0, DimensionUnit::VP));
        maskBlurRenderContext->ResetRadiusGradientBlur();
    } else {
        maskBlurRenderContext->UpdateBackBlurRadius(Dimension());
        LinearGradientBlurPara gradientBlurPara(
            Dimension(blurRadius, DimensionUnit::PX), MASK_BLUR_STOPS, direction);
        maskBlurRenderContext->UpdateRadiusGradientBlur(gradientBlurPara);
    }

    maskRenderContext->UpdateBackgroundColor(Color::TRANSPARENT);
    double opacity = scrollScale * GRADUAL_BLUR_MAX_OPACITY;
    if (NearZero(opacity)) {
        Gradient emptyGradient;
        emptyGradient.CreateGradientWithType(GradientType::LINEAR);
        maskRenderContext->UpdateLinearGradient(emptyGradient);
    } else {
        auto bgColor = GetGradientMaskBlendColor();
        Color blendColor = bgColor.BlendOpacity(opacity);
        Gradient gradient;
        gradient.CreateGradientWithType(GradientType::LINEAR);
        gradient.SetDirection(direction);
        for (const auto& fractionStop : FADE_OUT_GRADIENT_STOPS) {
            GradientColor stepColor(blendColor.BlendOpacity(fractionStop.first));
            stepColor.SetDimension(fractionStop.second * 100.0f, DimensionUnit::PERCENT);
            gradient.AddColor(stepColor);
        }
        maskRenderContext->UpdateLinearGradient(gradient);
    }
}

Color TabsSideBarPattern::GetGradientMaskBlendColor()
{
    auto tabsNode = tabsNode_.Upgrade();
    CHECK_NULL_RETURN(tabsNode, DEFAULT_GRADIENT_MASK_BLEND_COLOR);
    auto themeScopeId = tabsNode->GetThemeScopeId();
    auto tokenTheme = TokenThemeStorage::GetInstance()->GetTheme(themeScopeId);
    if (!tokenTheme) {
        tokenTheme = TokenThemeStorage::GetInstance()->ObtainSystemTheme();
    }
    CHECK_NULL_RETURN(tokenTheme, DEFAULT_GRADIENT_MASK_BLEND_COLOR);
    auto tokenColors = tokenTheme->Colors();
    CHECK_NULL_RETURN(tokenColors, DEFAULT_GRADIENT_MASK_BLEND_COLOR);
    return tokenColors->CompBackgroundGray();
}

void TabsSideBarPattern::OnColorConfigurationUpdate()
{
    // The collapse (hamburger) button glyph colors are baked into the Shape
    // paint properties at creation time, so re-apply them from the theme on
    // dark/light mode switches.
    UpdateCollapseButtonIconColor();
    if (isScrollEffectEnabled_) {
        UpdateBlurStyleImplInner(scrollScale_,
            headerContainerMaskBlurNode_, headerContainerMaskNode_, GradientDirection::BOTTOM);
    }
    if (isBottomBarScrollEffectEnabled_) {
        UpdateBlurStyleImplInner(bottomBarScrollScale_,
            bottomBarMaskBlurNode_, bottomBarMaskNode_, GradientDirection::TOP);
    }
}

RefPtr<FrameNode> TabsSideBarPattern::CreateCollapseButton(const RefPtr<FrameNode>& tabsNode)
{
    CHECK_NULL_RETURN(tabsNode, nullptr);
    auto buttonNode = CreateCollapseButtonNode();
    CHECK_NULL_RETURN(buttonNode, nullptr);
    auto iconNode = CreateCollapseButtonIcon();
    CHECK_NULL_RETURN(iconNode, nullptr);

    auto buttonHub = buttonNode->GetEventHub<EventHub>();
    CHECK_NULL_RETURN(buttonHub, nullptr);
    auto gestureHub = buttonHub->GetOrCreateGestureEventHub();
    CHECK_NULL_RETURN(gestureHub, nullptr);
    InitCollapseButtonClickEvent(gestureHub);

    iconNode->MountToParent(buttonNode);
    iconNode->MarkModifyDone();

    // Mount button as direct child of tabsNode (stays visible when sidebar is collapsed)
    buttonNode->MountToParent(tabsNode);
    buttonNode->MarkModifyDone();

    // Ensure title bar row exists in headerContainer to reserve space for button (56vp height).
    CreateTitleBarRowIfNeeded();

    collapseButtonNode_ = buttonNode;
    collapseButtonIconNode_ = iconNode;
    return buttonNode;
}

RefPtr<FrameNode> TabsSideBarPattern::CreateCollapseButtonNode()
{
    int32_t buttonId = ElementRegister::GetInstance()->MakeUniqueId();
    auto buttonNode = FrameNode::GetOrCreateFrameNode(V2::BUTTON_ETS_TAG, buttonId, []() -> RefPtr<Pattern> {
        auto* buttonModifier = NodeModifier::GetButtonCustomModifier();
        CHECK_NULL_RETURN(buttonModifier, nullptr);
        return buttonModifier->createButtonPattern();
    });
    CHECK_NULL_RETURN(buttonNode, nullptr);

    auto* btnCustomModifier = NodeModifier::GetButtonCustomModifier();
    CHECK_NULL_RETURN(btnCustomModifier, nullptr);
    ArkUINodeHandle buttonHandle = reinterpret_cast<ArkUINodeHandle>(AceType::RawPtr(buttonNode));
    btnCustomModifier->updateTypeToLayoutProp(buttonHandle, ButtonType::NORMAL);
    btnCustomModifier->updateBorderRadiusToLayoutProp(buttonHandle, BorderRadiusProperty(COLLAPSE_BUTTON_SIZE / 2));
    btnCustomModifier->updateCreateWithLabelToLayoutProp(buttonHandle, false);

    auto renderContext = buttonNode->GetRenderContext();
    CHECK_NULL_RETURN(renderContext, nullptr);
    renderContext->UpdateBackgroundColor(Color::TRANSPARENT);
    btnCustomModifier->updateBackgroundColorFlagByUserToLayoutProp(buttonHandle, true);
    renderContext->UpdateZIndex(COLLAPSE_BUTTON_ZINDEX);
    BlurStyleOption blurStyleOption;
    blurStyleOption.blurStyle = BlurStyle::COMPONENT_ULTRA_THIN;
    renderContext->UpdateBackBlurStyle(blurStyleOption);
    renderContext->UpdateBackShadow(ShadowConfig::DefaultShadowXS);

    auto layoutProperty = buttonNode->GetLayoutProperty();
    CHECK_NULL_RETURN(layoutProperty, nullptr);
    layoutProperty->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(COLLAPSE_BUTTON_SIZE), CalcLength(COLLAPSE_BUTTON_SIZE)));

    auto focusHub = buttonNode->GetOrCreateFocusHub();
    CHECK_NULL_RETURN(focusHub, nullptr);
    focusHub->SetFocusDependence(FocusDependence::SELF);

    return buttonNode;
}

RefPtr<FrameNode> TabsSideBarPattern::CreateCollapseButtonIcon()
{
    // The icon is split into two parts, replicating the system symbol glyphs
    // (sys.symbol.close_sidebar / sys.symbol.open_sidebar): static hamburger bars
    // + an arrow triangle that morphs (rotates 180°) on its own.
    auto host = GetHost();
    CHECK_NULL_RETURN(host, nullptr);
    auto tabTheme = host->GetTheme<TabTheme>();
    Color iconColor = tabTheme ? tabTheme->GetIconPrimary() : Color::BLACK;

    auto iconNode = FrameNode::CreateFrameNode(
        V2::STACK_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<StackPattern>());
    CHECK_NULL_RETURN(iconNode, nullptr);
    auto iconLayoutProperty = iconNode->GetLayoutProperty();
    CHECK_NULL_RETURN(iconLayoutProperty, nullptr);
    iconLayoutProperty->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(COLLAPSE_BUTTON_ICON_SIZE), CalcLength(COLLAPSE_BUTTON_ICON_SIZE)));

    // Three static hamburger bars: plain FrameNodes sized/positioned in vp and
    // painted via render-context background color/radius (see
    // CreateCollapseIconButtonBar for why shape Rect nodes are not used). In RTL layouts the whole glyph is mirrored
    // horizontally: the triangle sits left of the bars, so the middle bar leaves
    // room on the left instead of the right.
    bool mirroredByRtl = IsIconMirroredByRtl();
    for (const auto& bar : COLLAPSE_ICON_BARS) {
        float left = mirroredByRtl ? (COLLAPSE_ICON_BOX_VP - bar.left - bar.width) : bar.left;
        auto barNode = CreateCollapseIconButtonBar(iconColor, left, bar.top, bar.width, bar.height);
        CHECK_NULL_RETURN(barNode, nullptr);
        barNode->MountToParent(iconNode);
        barNode->MarkModifyDone();
    }

    auto triangleNode = FrameNode::CreateFrameNode(
        COLLAPSE_ICON_POLYGON_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<PolygonPattern>(true));
    CHECK_NULL_RETURN(triangleNode, nullptr);
    auto triangleLayoutProperty = triangleNode->GetLayoutProperty();
    CHECK_NULL_RETURN(triangleLayoutProperty, nullptr);
    // Node box = union bbox of both end-state outlines; the morph (lerp) always
    // stays inside it.
    triangleLayoutProperty->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(COLLAPSE_ICON_TRIANGLE_WIDTH_VP, DimensionUnit::VP),
            CalcLength(COLLAPSE_ICON_TRIANGLE_HEIGHT_VP, DimensionUnit::VP)));
    ShapeAbstractModelNG::SetFill(AceType::RawPtr(triangleNode), iconColor);
    UpdateCollapseTrianglePosition(triangleNode);
    UpdateCollapseTrianglePoints(triangleNode, isCollapsed_ ? 1.0f : 0.0f);

    triangleNode->MountToParent(iconNode);
    triangleNode->MarkModifyDone();

    collapseButtonTriangleNode_ = triangleNode;
    return iconNode;
}

void TabsSideBarPattern::InitCollapseButtonClickEvent(const RefPtr<GestureEventHub>& gestureHub)
{
    auto clickTask = [weakPattern = WeakClaim(this)](const GestureEvent& info) {
        auto pattern = weakPattern.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->OnCollapseButtonClick();
    };
    auto clickEvent = MakeRefPtr<ClickEvent>(std::move(clickTask));
    gestureHub->AddClickEvent(clickEvent);
}

void TabsSideBarPattern::OnCollapseButtonClick()
{
    // Debounce via the TabsPattern switch state machine (single source of
    // truth): one switch at a time, clicks during a running request/animation
    // are ignored entirely.
    auto tabsNode = tabsNode_.Upgrade();
    CHECK_NULL_VOID(tabsNode);
    auto tabsPattern = tabsNode->GetPattern<TabsPattern>();
    CHECK_NULL_VOID(tabsPattern);

    if (tabsPattern->IsSidebarCollapseSwitchBusy()) {
        return;
    }
    // Toggle the steady collapsed flag and orchestrate the unified two-pass
    // collapse/expand switch, in the same style as
    // TabsPattern::StartDisplayModeSwitchAnimation:
    // 1. collapse button icon: triangle rotation-correspondence morph;
    // 2. two-phase switch: the pass-1 layout keeps the sidebar arrangement but
    //    additionally lays out the entering component (bottom tab bar when
    //    collapsing, sidebar when expanding) so it has valid geometry; one
    //    animation then slides the sidebar out/in via a re-entrant
    //    "to"-arrangement layout
    //    (see TabsPattern::StartSidebarCollapseSwitchAnimation).
    // showBottomBarOnSidebarCollapse only decides - inside
    // TabsPattern::StartSidebarCollapseSwitchAnimation - whether the bottom
    // tab bar additionally animates in; the sidebar slide itself always runs.
    bool toCollapsed = !isCollapsed_;
    isCollapsed_ = toCollapsed;
    tabsPattern->RequestSidebarCollapseSwitchAnimation(toCollapsed);
    tabsNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    TAG_LOGI(AceLogTag::ACE_TABS,
        "Collapse button clicked: sidebar collapse switch requested, toCollapsed=%{public}d", toCollapsed);
}

void TabsSideBarPattern::UpdateCollapseButtonIcon()
{
    CHECK_NULL_VOID(collapseButtonTriangleNode_);
    // Sync the icon to the current state without animation: used when
    // sidebarPosition / layout direction change at runtime (which re-mirrors
    // the bars and the triangle). The bars are only positioned for the
    // direction known at creation time, so they must be refreshed here too,
    // otherwise e.g. an icon created before an RTL layout switch keeps
    // unmirrored bars and the triangle overlaps the short middle bar.
    UpdateCollapseButtonBarPositions();
    UpdateCollapseTrianglePosition(collapseButtonTriangleNode_);
    UpdateCollapseTrianglePoints(collapseButtonTriangleNode_, isCollapsed_ ? 1.0f : 0.0f);
}

void TabsSideBarPattern::UpdateCollapseButtonIconColor()
{
    CHECK_NULL_VOID(collapseButtonIconNode_);
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto tabTheme = host->GetTheme<TabTheme>();
    Color iconColor = tabTheme ? tabTheme->GetIconPrimary() : Color::BLACK;
    for (const auto& child : collapseButtonIconNode_->GetChildren()) {
        auto barNode = AceType::DynamicCast<FrameNode>(child);
        if (barNode && barNode->GetTag() == COLLAPSE_ICON_BAR_TAG) {
            auto renderContext = barNode->GetRenderContext();
            CHECK_NULL_VOID(renderContext);
            renderContext->UpdateBackgroundColor(iconColor);
        }
    }
    if (collapseButtonTriangleNode_) {
        ShapeAbstractModelNG::SetFill(AceType::RawPtr(collapseButtonTriangleNode_), iconColor);
    }
}

void TabsSideBarPattern::ClearCollapseButtonState()
{
    collapseButtonNode_ = nullptr;
    collapseButtonIconNode_ = nullptr;
    collapseButtonTriangleNode_ = nullptr;
}

Dimension TabsSideBarPattern::ComputeTitleBarRowHeight() const
{
    if (!ShouldAlignWithControlButtons()) {
        return TITLE_BAR_ROW_HEIGHT;
    }
    auto host = GetHost();
    CHECK_NULL_RETURN(host, TITLE_BAR_ROW_HEIGHT);
    auto pipeline = host->GetContext();
    CHECK_NULL_RETURN(pipeline, TITLE_BAR_ROW_HEIGHT);
    int32_t titleHeight = pipeline->GetContainerModalTitleHeight();
    if (titleHeight <= 0) {
        return TITLE_BAR_ROW_HEIGHT;
    }
    return Dimension(static_cast<float>(titleHeight), DimensionUnit::PX);
}

bool TabsSideBarPattern::ShouldAlignWithControlButtons() const
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    auto pipeline = host->GetContext();
    CHECK_NULL_RETURN(pipeline, false);
    if (pipeline->GetContainerCustomTitleVisible() || !pipeline->GetContainerControlButtonVisible()) {
        return false;
    }
    int32_t titleHeight = pipeline->GetContainerModalTitleHeight();
    if (titleHeight <= 0) {
        return false;
    }
    return true;
}

void TabsSideBarPattern::CreateTitleBarRowIfNeeded()
{
    if (!headerContainerNode_ || titleBarRowNode_) {
        return;
    }
    auto row = FrameNode::GetOrCreateFrameNode(
        "TitleBarRow", ElementRegister::GetInstance()->MakeUniqueId(),
        []() { return AceType::MakeRefPtr<LinearLayoutPattern>(false); });
    CHECK_NULL_VOID(row);
    auto rowProp = row->GetLayoutProperty<LinearLayoutProperty>();
    if (rowProp) {
        rowProp->UpdateFlexDirection(FlexDirection::ROW);
        rowProp->UpdateMeasureType(MeasureType::MATCH_PARENT_CROSS_AXIS);
        rowProp->UpdateUserDefinedIdealSize(
            CalcSize(std::nullopt, CalcLength(ComputeTitleBarRowHeight())));
        bool visible = showControlButton_ || !titleBarOptions_.isNull;
        rowProp->UpdateVisibility(visible ? VisibleType::VISIBLE : VisibleType::GONE);
    }
    row->MountToParent(headerContainerNode_, 0);
    row->MarkModifyDone();
    headerContainerNode_->MarkNeedSyncRenderTree();
    headerContainerNode_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);

    titleBarRowNode_ = row;
}

RefPtr<FrameNode> TabsSideBarPattern::CreateTitleBarIconNode()
{
    auto iconNode = FrameNode::GetOrCreateFrameNode(V2::IMAGE_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<ImagePattern>(); });
    CHECK_NULL_RETURN(iconNode, nullptr);
    auto imageLayoutProp = iconNode->GetLayoutProperty<ImageLayoutProperty>();
    if (imageLayoutProp) {
        imageLayoutProp->UpdateUserDefinedIdealSize(
            CalcSize(CalcLength(TITLE_BAR_ICON_SIZE), CalcLength(TITLE_BAR_ICON_SIZE)));
    }
    return iconNode;
}

RefPtr<FrameNode> TabsSideBarPattern::CreateTitleBarTextNode()
{
    auto textNode = FrameNode::GetOrCreateFrameNode(V2::TEXT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextPattern>(); });
    CHECK_NULL_RETURN(textNode, nullptr);
    auto textLayoutProp = textNode->GetLayoutProperty<TextLayoutProperty>();
    if (textLayoutProp) {
        textLayoutProp->UpdateMaxLines(1);
        textLayoutProp->UpdateTextOverflow(TextOverflow::ELLIPSIS);
    }
    // Allow text to shrink so it fits the remaining width in the row
    auto layoutProp = textNode->GetLayoutProperty();
    if (layoutProp) {
        layoutProp->UpdateFlexShrink(1.0f);
    }
    return textNode;
}

void TabsSideBarPattern::UpdateTitleBarContent()
{
    CreateTitleBarRowIfNeeded();
    CHECK_NULL_VOID(titleBarRowNode_);

    bool hasIcon = !titleBarOptions_.isNull && titleBarOptions_.icon.has_value() &&
                   !titleBarOptions_.icon->empty();
    bool hasText = !titleBarOptions_.isNull && titleBarOptions_.text.has_value() &&
                   !titleBarOptions_.text->empty();
    bool visible = showControlButton_ || hasIcon || hasText;

    auto rowProp = titleBarRowNode_->GetLayoutProperty<LinearLayoutProperty>();
    if (rowProp) {
        rowProp->UpdateVisibility(visible ? VisibleType::VISIBLE : VisibleType::GONE);
        PaddingProperty padding;
        padding.start = CalcLength(COLLAPSE_BUTTON_HORIZONTAL_PADDING);
        padding.end = CalcLength(showControlButton_ ? TITLE_BAR_BUTTON_AREA_WIDTH
                                                    : TITLE_BAR_RIGHT_PADDING_NO_BUTTON);
        rowProp->UpdatePadding(padding);
    }

    if (hasIcon) {
        if (!titleBarIconNode_) {
            titleBarIconNode_ = CreateTitleBarIconNode();
            if (titleBarIconNode_) {
                titleBarIconNode_->MountToParent(titleBarRowNode_);
            }
        }
        if (titleBarIconNode_) {
            auto imageLayoutProp = titleBarIconNode_->GetLayoutProperty<ImageLayoutProperty>();
            if (imageLayoutProp) {
                ImageSourceInfo info(*titleBarOptions_.icon);
                imageLayoutProp->UpdateImageSourceInfo(info);
            }
            titleBarIconNode_->MarkModifyDone();
        }
    } else if (titleBarIconNode_) {
        titleBarRowNode_->RemoveChild(titleBarIconNode_);
        titleBarIconNode_ = nullptr;
    }

    // Handle text node
    if (hasText) {
        if (!titleBarTextNode_) {
            titleBarTextNode_ = CreateTitleBarTextNode();
            if (titleBarTextNode_) {
                titleBarTextNode_->MountToParent(titleBarRowNode_);
            }
        }
        if (titleBarTextNode_) {
            auto textLayoutProp = titleBarTextNode_->GetLayoutProperty<TextLayoutProperty>();
            if (textLayoutProp) {
                textLayoutProp->UpdateContent(*titleBarOptions_.text);
            }
            if (titleBarIconNode_) {
                auto iconProp = titleBarIconNode_->GetLayoutProperty();
                if (iconProp) {
                    MarginProperty iconMargin;
                    iconMargin.end = CalcLength(TITLE_BAR_ICON_TEXT_PADDING);
                    iconProp->UpdateMargin(iconMargin);
                }
            }
            titleBarTextNode_->MarkModifyDone();
        }
    } else if (titleBarTextNode_) {
        titleBarRowNode_->RemoveChild(titleBarTextNode_);
        titleBarTextNode_ = nullptr;
    }
    titleBarRowNode_->MarkModifyDone();
    titleBarRowNode_->MarkNeedSyncRenderTree();
    titleBarRowNode_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
}

} // namespace OHOS::Ace::NG

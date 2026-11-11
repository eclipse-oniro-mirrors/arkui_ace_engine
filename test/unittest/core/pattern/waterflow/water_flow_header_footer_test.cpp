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

#include "water_flow_test_ng.h"

#define private public
#include "core/components_ng/pattern/lazy_column_layout/lazy_column_layout_algorithm.h"
#undef private

#include <limits>
#include <memory>

#include "test/mock/frameworks/core/components_ng/render/mock_render_context.h"
#include "core/components_ng/pattern/lazy_layout/header_footer_utils.h"
#include "core/components_ng/pattern/lazy_layout/lazy_layout_pattern.h"
#include "core/components_ng/pattern/lazy_layout/lazy_layout_utils.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"
#include "core/components_ng/pattern/refresh/refresh_pattern.h"
#include "core/components_ng/pattern/scroll/scroll_pattern.h"
#include "core/components_ng/pattern/scrollable/scroll_viewport_coordinator.h"
#include "core/components_ng/pattern/swiper/swiper_pattern.h"
#include "core/components_ng/pattern/waterflow/layout/sliding_window/water_flow_layout_sw.h"
#include "core/components_ng/pattern/waterflow/layout/top_down/water_flow_segmented_layout.h"

namespace OHOS::Ace::NG {
namespace {
class EdgeTransformRenderContext : public MockRenderContext {
    DECLARE_ACE_TYPE(EdgeTransformRenderContext, MockRenderContext);
public:
    Matrix4 GetMatrixWithTransformRotate() override
    {
        ++queries_;
        return transform_;
    }

    Matrix4 transform_ = Matrix4::CreateIdentity();
    int32_t queries_ = 0;
};

} // namespace
class WaterFlowHeaderFooterTest : public WaterFlowTestNg {
protected:
    RefPtr<FrameNode> Edge(float height)
    {
        auto node = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG,
            ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<LinearLayoutPattern>(true));
        node->GetLayoutProperty()->UpdateUserDefinedIdealSize(CalcSize(std::nullopt, CalcLength(height)));
        return node;
    }

    void SetEdges(float header = 50.0f, float footer = 40.0f)
    {
        pattern_->SetEdge(true, Edge(header), pattern_->BeginEdgeUpdate(true));
        pattern_->SetEdge(false, Edge(footer), pattern_->BeginEdgeUpdate(false));
    }

    RefPtr<FrameNode> ParentScroll(float height)
    {
        auto parent = FrameNode::CreateFrameNode(V2::SCROLL_ETS_TAG,
            ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<ScrollPattern>());
        parent->GetPattern<ScrollPattern>()->SetAxis(Axis::VERTICAL);
        parent->GetGeometryNode()->SetFrameSize(SizeF(WATER_FLOW_WIDTH, height));
        parent->GetRenderContext()->UpdatePaintRect(parent->GetGeometryNode()->GetFrameRect());
        parent->AddChild(frameNode_);
        return parent;
    }

    void MoveHost(float y)
    {
        frameNode_->GetGeometryNode()->SetFrameOffset(OffsetF(0.0f, y));
        frameNode_->GetRenderContext()->UpdatePaintRect(frameNode_->GetGeometryNode()->GetFrameRect());
    }

    RefPtr<FrameNode> Header() const
    {
        return AceType::DynamicCast<FrameNode>(pattern_->GetEdgeState().header.Upgrade());
    }
};

/**
 * @tc.name: WFHF_FlowAndIndices
 * @tc.desc: All three algorithms exclude edges from data.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowHeaderFooterTest, WFHF_FlowAndIndices, TestSize.Level1)
{
    CreateWaterFlow();
    CreateWaterFlowItems(2);
    SetEdges();
    CreateDone();
    auto map = pattern_->GetEdgeIndexMap();
    EXPECT_EQ(map.DataToNode(0), 1);
    EXPECT_EQ(map.HeaderIndex(), 0);
    EXPECT_EQ(map.FooterIndex(), 3);
    EXPECT_EQ(map.NodeToData(3), -1);
    EXPECT_EQ(pattern_->GetDataCount(), 2);
    EXPECT_EQ(accessibilityProperty_->GetCollectionItemCounts(), 2);
    EXPECT_FLOAT_EQ(GetChildRect(frameNode_, map.HeaderIndex()).Top(), 0.0f);
    EXPECT_FLOAT_EQ(GetChildRect(frameNode_, map.DataToNode(0)).Top(), 50.0f);
    EXPECT_FLOAT_EQ(GetChildRect(frameNode_, map.FooterIndex()).Top(), 350.0f);
    EXPECT_EQ(pattern_->GetBeginIndex(), 0);
    EXPECT_EQ(pattern_->GetEndIndex(), 1);
}

/**
 * @tc.name: WFHF_Empty
 * @tc.desc: Edges have a natural position without FlowItems.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowHeaderFooterTest, WFHF_Empty, TestSize.Level1)
{
    CreateWaterFlow();
    SetEdges();
    CreateDone();
    EXPECT_EQ(pattern_->GetDataCount(), 0);
    EXPECT_FLOAT_EQ(GetChildRect(frameNode_, pattern_->GetEdgeIndexMap().HeaderIndex()).Top(), 0.0f);
    EXPECT_FLOAT_EQ(GetChildRect(frameNode_, pattern_->GetEdgeIndexMap().FooterIndex()).Top(), 50.0f);
}

/**
 * @tc.name: WFHF_InvalidDataKeepsEdges
 * @tc.desc: A transient section/data mismatch still lays out active edge wrappers.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowHeaderFooterTest, WFHF_InvalidDataKeepsEdges, TestSize.Level1)
{
    CreateWaterFlow();
    CreateWaterFlowItems(20);
    SetEdges();
    CreateDone();
    auto algorithm = pattern_->CreateLayoutAlgorithm();
    if (!AceType::DynamicCast<WaterFlowLayoutSW>(algorithm) &&
        !AceType::DynamicCast<WaterFlowSegmentedLayout>(algorithm)) {
        GTEST_SKIP() << "Only segmented and sliding-window layouts validate section data";
    }
    algorithm->Measure(AceType::RawPtr(frameNode_));
    pattern_->layoutInfo_->isDataValid_ = false;
    auto footer = AceType::DynamicCast<FrameNode>(
        frameNode_->GetChildAtIndex(pattern_->GetEdgeIndexMap().FooterIndex()));
    ASSERT_NE(footer, nullptr);
    Header()->SetActive(false);
    footer->SetActive(false);
    algorithm->Layout(AceType::RawPtr(frameNode_));
    EXPECT_TRUE(Header()->IsActive());
    EXPECT_TRUE(footer->IsActive());
    EXPECT_FLOAT_EQ(footer->GetGeometryNode()->GetMarginFrameOffset().GetY(),
        Header()->GetGeometryNode()->GetMarginFrameOffset().GetY() + 50.0f);
}

/**
 * @tc.name: WFHF_TargetMeasureKeepsFooter
 * @tc.desc: The sliding-window target-measure frame must not deactivate the new footer slot.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowHeaderFooterTest, WFHF_TargetMeasureKeepsFooter, TestSize.Level1)
{
    CreateWaterFlow();
    CreateWaterFlowItems(40);
    SetEdges();
    CreateDone();
    auto algorithm = AceType::DynamicCast<WaterFlowLayoutSW>(pattern_->CreateLayoutAlgorithm());
    if (!algorithm) {
        GTEST_SKIP() << "Target-measure early return is specific to sliding-window layout";
    }
    algorithm->Measure(AceType::RawPtr(frameNode_));
    pattern_->layoutInfo_->targetIndex_ = 30;
    auto footer = AceType::DynamicCast<FrameNode>(
        frameNode_->GetChildAtIndex(pattern_->GetEdgeIndexMap().FooterIndex()));
    ASSERT_NE(footer, nullptr);
    footer->SetActive(false);
    algorithm->Layout(AceType::RawPtr(frameNode_));
    EXPECT_TRUE(footer->IsActive());
    EXPECT_TRUE(Header()->IsActive());
}

/**
 * @tc.name: WFHF_Standalone
 * @tc.desc: Sticky never pins to WaterFlow's own viewport.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowHeaderFooterTest, WFHF_Standalone, TestSize.Level1)
{
    CreateWaterFlow();
    CreateWaterFlowItems(20);
    SetEdges();
    CreateDone();
    UpdateCurrentOffset(-100.0f);
    const auto natural = Header()->GetGeometryNode()->GetMarginFrameOffset();
    EXPECT_FLOAT_EQ(natural.GetY(), -100.0f);
    for (auto style : { StickyStyle::NONE, StickyStyle::HEADER, StickyStyle::FOOTER, StickyStyle::BOTH }) {
        pattern_->SetSticky(style);
        pattern_->UpdateEdgePositions();
        EXPECT_EQ(Header()->GetGeometryNode()->GetMarginFrameOffset(), natural);
    }
}

/**
 * @tc.name: WFHF_StickyOnlyKeepsLegacyFooter
 * @tc.desc: Sticky configuration alone cannot wrap the constructor footer or change its indices.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowHeaderFooterTest, WFHF_StickyOnlyKeepsLegacyFooter, TestSize.Level1)
{
    CreateWaterFlow();
    CreateWaterFlowItems(2);
    auto legacy = Edge(70.0f);
    pattern_->AddFooter(legacy);
    for (auto style : { StickyStyle::NONE, StickyStyle::HEADER, StickyStyle::FOOTER, StickyStyle::BOTH }) {
        pattern_->SetSticky(style);
        EXPECT_FALSE(pattern_->GetEdgeState().enabled);
        EXPECT_FALSE(pattern_->GetEdgeState().footerControlled);
        EXPECT_FALSE(pattern_->layoutInfo_->edgeSlots_);
        EXPECT_FALSE(pattern_->edgeViewportRegistered_);
        EXPECT_EQ(legacy->GetParent(), frameNode_);
        EXPECT_EQ(frameNode_->GetChildAtIndex(0), legacy);
        EXPECT_EQ(pattern_->GetChildrenCount(), 3);
    }
    // Resetting an unused header must also leave the constructor footer untouched.
    pattern_->SetEdge(true, nullptr, pattern_->BeginEdgeUpdate(true));
    EXPECT_FALSE(pattern_->GetEdgeState().enabled);
    EXPECT_EQ(legacy->GetParent(), frameNode_);
    EXPECT_EQ(frameNode_->GetChildAtIndex(0), legacy);
    CreateDone();
}

/**
 * @tc.name: WFHF_EmptyEdgeUpdatesKeepLegacyMode
 * @tc.desc: Initial undefined/reset updates do not enable edge layout or viewport callbacks.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowHeaderFooterTest, WFHF_EmptyEdgeUpdatesKeepLegacyMode, TestSize.Level1)
{
    CreateWaterFlow();
    CreateWaterFlowItems(2);
    CreateDone();
    const auto originalRect = GetChildRect(frameNode_, 0);
    pattern_->SetSticky(StickyStyle::BOTH);
    pattern_->SetEdge(true, nullptr, pattern_->BeginEdgeUpdate(true));
    pattern_->SetEdge(false, nullptr, pattern_->BeginEdgeUpdate(false));
    pattern_->SyncEdgeSlots();
    pattern_->SetEdgeGeometry(OffsetF(), 1000.0f);
    FlushUITasks();
    EXPECT_FALSE(pattern_->GetEdgeState().enabled);
    EXPECT_FALSE(pattern_->layoutInfo_->edgeSlots_);
    EXPECT_FALSE(pattern_->edgeViewportRegistered_);
    EXPECT_FALSE(layoutProperty_->GetNeedLazyLayout());
    EXPECT_EQ(pattern_->GetChildrenCount(), 2);
    EXPECT_EQ(GetChildRect(frameNode_, 0), originalRect);
}

/**
 * @tc.name: WFHF_UnreservedGenerationKeepsLegacyTree
 * @tc.desc: An edge result without a reserved update token cannot activate the new layout.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowHeaderFooterTest, WFHF_UnreservedGenerationKeepsLegacyTree, TestSize.Level1)
{
    CreateWaterFlow();
    CreateWaterFlowItems(2);
    auto header = Edge(50.0f);
    auto footer = Edge(40.0f);
    pattern_->SetEdge(true, header, 0);
    pattern_->SetEdge(false, footer, 0);
    EXPECT_FALSE(pattern_->GetEdgeState().enabled);
    EXPECT_EQ(header->GetParent(), nullptr);
    EXPECT_EQ(footer->GetParent(), nullptr);
    EXPECT_EQ(pattern_->GetChildrenCount(), 2);
    CreateDone();
}

/**
 * @tc.name: WFHF_StaleBuilder
 * @tc.desc: Independent generations reject late construction.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowHeaderFooterTest, WFHF_StaleBuilder, TestSize.Level1)
{
    CreateWaterFlow();
    CreateWaterFlowItems(2);
    const auto oldHeader = pattern_->BeginEdgeUpdate(true);
    const auto footer = pattern_->BeginEdgeUpdate(false);
    const auto clearHeader = pattern_->BeginEdgeUpdate(true);
    pattern_->SetEdge(true, nullptr, clearHeader);
    pattern_->SetEdge(false, Edge(40.0f), footer);
    pattern_->SetEdge(true, Edge(80.0f), oldHeader);
    CreateDone();
    EXPECT_EQ(Header(), nullptr);
    EXPECT_TRUE(pattern_->hasFooter());
    EXPECT_EQ(pattern_->GetDataCount(), 2);
}

/**
 * @tc.name: WFHF_StaleBuilderKeepsLegacyTree
 * @tc.desc: Starting or rejecting a builder cannot reparent the legacy footer.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowHeaderFooterTest, WFHF_StaleBuilderKeepsLegacyTree, TestSize.Level1)
{
    CreateWaterFlow();
    CreateWaterFlowItems(2);
    auto legacy = Edge(70.0f);
    pattern_->AddFooter(legacy);
    const auto stale = pattern_->BeginEdgeUpdate(true);
    const auto current = pattern_->BeginEdgeUpdate(true);
    EXPECT_EQ(legacy->GetParent(), frameNode_);
    EXPECT_FALSE(pattern_->GetEdgeState().enabled);
    pattern_->SetEdge(true, Edge(50.0f), stale);
    EXPECT_EQ(legacy->GetParent(), frameNode_);
    EXPECT_FALSE(pattern_->GetEdgeState().enabled);
    pattern_->SetEdge(true, Edge(50.0f), current);
    EXPECT_NE(legacy->GetParent(), frameNode_);
    EXPECT_TRUE(pattern_->GetEdgeState().enabled);
    EXPECT_EQ(pattern_->GetDataCount(), 2);
    CreateDone();
}

/**
 * @tc.name: WFHF_StickyKeepsPendingBuilder
 * @tc.desc: Enabling sticky does not invalidate a pending header builder.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowHeaderFooterTest, WFHF_StickyKeepsPendingBuilder, TestSize.Level1)
{
    CreateWaterFlow();
    CreateWaterFlowItems(2);
    const auto generation = pattern_->BeginEdgeUpdate(true);
    pattern_->SetSticky(StickyStyle::HEADER);
    EXPECT_FALSE(pattern_->GetEdgeState().enabled);
    EXPECT_FALSE(pattern_->layoutInfo_->edgeSlots_);
    EXPECT_FALSE(pattern_->edgeViewportRegistered_);
    pattern_->SetEdge(true, Edge(50.0f), generation);
    CreateDone();
    ASSERT_NE(Header(), nullptr);
    EXPECT_TRUE(pattern_->GetEdgeState().enabled);
    EXPECT_EQ(pattern_->GetEdgeState().sticky, StickyStyle::HEADER);
    EXPECT_EQ(pattern_->GetDataCount(), 2);
    EXPECT_FLOAT_EQ(Header()->GetGeometryNode()->GetFrameSize().Height(), 50.0f);
}

/**
 * @tc.name: WFHF_IntrinsicEdgeHeight
 * @tc.desc: An edge with no explicit height measures its content, including beyond the viewport.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowHeaderFooterTest, WFHF_IntrinsicEdgeHeight, TestSize.Level1)
{
    CreateWaterFlow();
    CreateWaterFlowItems(2);
    auto content = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<LinearLayoutPattern>(true));
    content->AddChild(Edge(30.0f));
    content->AddChild(Edge(WATER_FLOW_HEIGHT));
    pattern_->SetEdge(true, content, pattern_->BeginEdgeUpdate(true));
    CreateDone();
    EXPECT_FLOAT_EQ(Header()->GetGeometryNode()->GetFrameSize().Height(), WATER_FLOW_HEIGHT + 30.0f);
}

/**
 * @tc.name: WFHF_DetachInvalidatesBuilder
 * @tc.desc: A detached or recycled host rejects its previous asynchronous builder result.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowHeaderFooterTest, WFHF_DetachInvalidatesBuilder, TestSize.Level1)
{
    CreateWaterFlow();
    CreateWaterFlowItems(2);
    CreateDone();
    const auto generation = pattern_->BeginEdgeUpdate(true);
    pattern_->OnDetachFromMainTree();
    pattern_->SetEdge(true, Edge(50.0f), generation);
    EXPECT_EQ(Header(), nullptr);
    EXPECT_EQ(pattern_->GetDataCount(), 2);
}

/**
 * @tc.name: WFHF_FooterTakeover
 * @tc.desc: Clearing the new footer never resurrects legacy content.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowHeaderFooterTest, WFHF_FooterTakeover, TestSize.Level1)
{
    CreateWaterFlow();
    CreateWaterFlowItems(2);
    auto legacy = Edge(70.0f);
    pattern_->AddFooter(legacy);
    CreateDone();
    pattern_->SetEdge(false, nullptr, pattern_->BeginEdgeUpdate(false));
    EXPECT_FALSE(pattern_->GetEdgeState().enabled);
    EXPECT_FALSE(pattern_->layoutInfo_->edgeSlots_);
    EXPECT_EQ(pattern_->layoutInfo_->footerIndex_, -1);
    EXPECT_FLOAT_EQ(pattern_->layoutInfo_->footerHeight_, 0.0f);
    pattern_->AddFooter(legacy);
    FlushUITasks();
    EXPECT_FALSE(pattern_->hasFooter());
    EXPECT_EQ(pattern_->GetChildrenCount(), 2);
    EXPECT_FLOAT_EQ(GetChildRect(frameNode_, 0).Top(), 0.0f);
}

/**
 * @tc.name: WFHF_FooterEnablesPendingSticky
 * @tc.desc: A valid footer activates previously stored sticky after an initial empty reset.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowHeaderFooterTest, WFHF_FooterEnablesPendingSticky, TestSize.Level1)
{
    CreateWaterFlow();
    CreateWaterFlowItems(20);
    pattern_->SetSticky(StickyStyle::FOOTER);
    pattern_->SetEdge(false, nullptr, pattern_->BeginEdgeUpdate(false));
    EXPECT_FALSE(pattern_->GetEdgeState().enabled);
    const auto generation = pattern_->BeginEdgeUpdate(false);
    EXPECT_FALSE(pattern_->GetEdgeState().enabled);
    pattern_->SetEdge(false, Edge(40.0f), generation);
    CreateDone();
    auto parent = ParentScroll(300.0f);
    MoveHost(-100.0f);
    pattern_->SetEdgeGeometry(OffsetF(), 1000.0f);
    auto pipeline = parent->GetContext();
    ASSERT_NE(pipeline, nullptr);
    pipeline->GetScrollViewportCoordinator()->Flush(pipeline);
    EXPECT_TRUE(pattern_->GetEdgeState().enabled);
    EXPECT_TRUE(pattern_->layoutInfo_->edgeSlots_);
    EXPECT_EQ(pattern_->GetEdgeState().sticky, StickyStyle::FOOTER);
    EXPECT_EQ(Header(), nullptr);
    EXPECT_EQ(pattern_->GetDataCount(), 20);
    EXPECT_FLOAT_EQ(GetChildRect(frameNode_, pattern_->GetEdgeIndexMap().FooterIndex()).Top(), 360.0f);
}

/**
 * @tc.name: WFHF_DirectionReset
 * @tc.desc: Unsupported direction hides new edges and restores Column.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowHeaderFooterTest, WFHF_DirectionReset, TestSize.Level1)
{
    CreateWaterFlow();
    CreateWaterFlowItems(2);
    SetEdges();
    CreateDone();
    WaterFlowModelNG::SetLayoutDirection(AceType::RawPtr(frameNode_), FlexDirection::COLUMN_REVERSE);
    FlushUITasks();
    EXPECT_FALSE(Header()->IsActive());
    WaterFlowModelNG::SetLayoutDirection(AceType::RawPtr(frameNode_), FlexDirection::COLUMN);
    FlushUITasks();
    EXPECT_TRUE(Header()->IsActive());
    EXPECT_FLOAT_EQ(GetChildRect(frameNode_, pattern_->GetEdgeIndexMap().DataToNode(0)).Top(), 50.0f);
}

/**
 * @tc.name: WFHF_ParentOnlyMove
 * @tc.desc: Post-layout registry handles movement without child layout.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowHeaderFooterTest, WFHF_ParentOnlyMove, TestSize.Level1)
{
    CreateWaterFlow();
    CreateWaterFlowItems(20);
    SetEdges();
    CreateDone();
    auto parent = ParentScroll(300.0f);
    MoveHost(-200.0f);
    pattern_->SetEdgeGeometry(OffsetF(), 1000.0f);
    pattern_->SetSticky(StickyStyle::BOTH);
    auto pipeline = frameNode_->GetContext();
    ASSERT_NE(pipeline, nullptr);
    pipeline->GetScrollViewportCoordinator()->Flush(pipeline);
    EXPECT_FLOAT_EQ(Header()->GetGeometryNode()->GetMarginFrameOffset().GetY(), 200.0f);
    EXPECT_FLOAT_EQ(GetChildRect(frameNode_, pattern_->GetEdgeIndexMap().FooterIndex()).Top(), 460.0f);
    MoveHost(-250.0f);
    pipeline->GetScrollViewportCoordinator()->Flush(pipeline);
    EXPECT_FLOAT_EQ(Header()->GetGeometryNode()->GetMarginFrameOffset().GetY(), 250.0f);
    EXPECT_FLOAT_EQ(GetChildRect(frameNode_, pattern_->GetEdgeIndexMap().FooterIndex()).Top(), 510.0f);
    EXPECT_EQ(Header()->GetRenderContext()->GetZIndexValue(-1), HeaderFooterUtils::STICKY_HEADER_Z_INDEX);
    pattern_->SetSticky(StickyStyle::NONE);
    EXPECT_FALSE(pattern_->edgeViewportRegistered_);
    EXPECT_FLOAT_EQ(Header()->GetGeometryNode()->GetMarginFrameOffset().GetY(), 0.0f);
    EXPECT_FLOAT_EQ(GetChildRect(frameNode_, pattern_->GetEdgeIndexMap().FooterIndex()).Top(), 960.0f);
    EXPECT_EQ(Header()->GetRenderContext()->GetZIndexValue(-1), 0);
}

/**
 * @tc.name: WFHF_ZeroClipRestoresNaturalPosition
 * @tc.desc: A zero-height clipping ancestor blocks pinning until its next layout gives it a viewport.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowHeaderFooterTest, WFHF_ZeroClipRestoresNaturalPosition, TestSize.Level1)
{
    CreateWaterFlow();
    CreateWaterFlowItems(20);
    SetEdges();
    CreateDone();
    auto parent = ParentScroll(300.0f);
    auto clip = Edge(0.0f);
    clip->GetRenderContext()->UpdateClipEdge(true);
    clip->AddChild(parent);
    MoveHost(-200.0f);
    pattern_->SetEdgeGeometry(OffsetF(), 1000.0f);
    pattern_->SetSticky(StickyStyle::HEADER);
    auto pipeline = frameNode_->GetContext();
    ASSERT_NE(pipeline, nullptr);
    pipeline->GetScrollViewportCoordinator()->Flush(pipeline);
    EXPECT_FLOAT_EQ(Header()->GetGeometryNode()->GetMarginFrameOffset().GetY(), 0.0f);
    clip->GetGeometryNode()->SetFrameSize(SizeF(WATER_FLOW_WIDTH, 300.0f));
    clip->GetRenderContext()->UpdatePaintRect(clip->GetGeometryNode()->GetFrameRect());
    pipeline->GetScrollViewportCoordinator()->Flush(pipeline);
    EXPECT_FLOAT_EQ(Header()->GetGeometryNode()->GetMarginFrameOffset().GetY(), 200.0f);
}

/**
 * @tc.name: WFHF_ReverseParentBlocksOuterViewport
 * @tc.desc: A reverse WaterFlow parent cancels pinning without borrowing an outer Scroll viewport.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowHeaderFooterTest, WFHF_ReverseParentBlocksOuterViewport, TestSize.Level1)
{
    CreateWaterFlow();
    CreateWaterFlowItems(20);
    SetEdges();
    CreateDone();
    auto outer = ParentScroll(600.0f);
    auto parent = FrameNode::CreateFrameNode(V2::WATERFLOW_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<WaterFlowPattern>());
    parent->GetGeometryNode()->SetFrameSize(SizeF(WATER_FLOW_WIDTH, 300.0f));
    parent->GetRenderContext()->UpdatePaintRect(parent->GetGeometryNode()->GetFrameRect());
    outer->RemoveChild(frameNode_);
    outer->AddChild(parent);
    parent->AddChild(frameNode_);
    MoveHost(-200.0f);
    pattern_->SetEdgeGeometry(OffsetF(), 1000.0f);
    pattern_->SetSticky(StickyStyle::BOTH);
    pattern_->UpdateEdgePositions();
    EXPECT_FLOAT_EQ(Header()->GetGeometryNode()->GetMarginFrameOffset().GetY(), 200.0f);
    EXPECT_FLOAT_EQ(GetChildRect(frameNode_, pattern_->GetEdgeIndexMap().FooterIndex()).Top(), 460.0f);

    auto property = parent->GetLayoutProperty<WaterFlowLayoutProperty>();
    property->UpdateWaterflowDirection(FlexDirection::COLUMN_REVERSE);
    ASSERT_TRUE(parent->GetPattern<WaterFlowPattern>()->IsReverse());
    pattern_->UpdateEdgePositions();
    EXPECT_FLOAT_EQ(Header()->GetGeometryNode()->GetMarginFrameOffset().GetY(), 0.0f);
    EXPECT_FLOAT_EQ(GetChildRect(frameNode_, pattern_->GetEdgeIndexMap().FooterIndex()).Top(), 960.0f);

    property->UpdateWaterflowDirection(FlexDirection::COLUMN);
    pattern_->UpdateEdgePositions();
    EXPECT_FLOAT_EQ(Header()->GetGeometryNode()->GetMarginFrameOffset().GetY(), 200.0f);
    EXPECT_FLOAT_EQ(GetChildRect(frameNode_, pattern_->GetEdgeIndexMap().FooterIndex()).Top(), 460.0f);
}

/**
 * @tc.name: WFHF_UnsupportedParentBlocksOuterViewport
 * @tc.desc: Swiper and Refresh block viewport lookup, but preserve an eligible viewport nested inside them.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowHeaderFooterTest, WFHF_UnsupportedParentBlocksOuterViewport, TestSize.Level1)
{
    CreateWaterFlow();
    CreateWaterFlowItems(20);
    SetEdges();
    CreateDone();
    const std::vector<std::pair<std::string, RefPtr<Pattern>>> parents {
        { V2::SWIPER_ETS_TAG, AceType::MakeRefPtr<SwiperPattern>() },
        { V2::REFRESH_ETS_TAG, AceType::MakeRefPtr<RefreshPattern>() },
    };
    for (const auto& [tag, parentPattern] : parents) {
        SCOPED_TRACE(tag);
        auto outer = ParentScroll(300.0f);
        auto parent = FrameNode::CreateFrameNode(tag,
            ElementRegister::GetInstance()->MakeUniqueId(), parentPattern);
        parent->GetGeometryNode()->SetFrameSize(SizeF(WATER_FLOW_WIDTH, 600.0f));
        parent->GetRenderContext()->UpdatePaintRect(parent->GetGeometryNode()->GetFrameRect());
        outer->RemoveChild(frameNode_);
        outer->AddChild(parent);
        parent->AddChild(frameNode_);
        MoveHost(-200.0f);
        pattern_->SetEdgeGeometry(OffsetF(), 1000.0f);
        pattern_->SetSticky(StickyStyle::BOTH);
        pattern_->UpdateEdgePositions();
        EXPECT_FLOAT_EQ(Header()->GetGeometryNode()->GetMarginFrameOffset().GetY(), 0.0f);
        EXPECT_FLOAT_EQ(GetChildRect(frameNode_, pattern_->GetEdgeIndexMap().FooterIndex()).Top(), 960.0f);

        outer->RemoveChild(parent);
        parent->RemoveChild(frameNode_);
        parent->AddChild(outer);
        outer->AddChild(frameNode_);
        pattern_->UpdateEdgePositions();
        EXPECT_FLOAT_EQ(Header()->GetGeometryNode()->GetMarginFrameOffset().GetY(), 200.0f);
        EXPECT_FLOAT_EQ(GetChildRect(frameNode_, pattern_->GetEdgeIndexMap().FooterIndex()).Top(), 460.0f);
        outer->RemoveChild(frameNode_);
        parent->RemoveChild(outer);
    }
}

/**
 * @tc.name: WFHF_CoordinatorMutationAndReentry
 * @tc.desc: Registry mutations use the next snapshot and recursive flush does not re-enter callbacks.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowHeaderFooterTest, WFHF_CoordinatorMutationAndReentry, TestSize.Level1)
{
    CreateWaterFlow();
    CreateDone();
    auto pipeline = frameNode_->GetContext();
    ASSERT_NE(pipeline, nullptr);
    auto coordinator = AceType::MakeRefPtr<ScrollViewportCoordinator>();
    auto* rawCoordinator = AceType::RawPtr(coordinator);
    auto calls = std::make_shared<int32_t>(0);
    coordinator->Register(nullptr, [calls]() { ++*calls; });
    coordinator->Register(frameNode_, {});
    coordinator->Flush(pipeline);
    EXPECT_EQ(*calls, 0);
    WeakPtr<FrameNode> weak = frameNode_;
    coordinator->Register(frameNode_, [rawCoordinator, weak, pipeline, calls]() {
        ++*calls;
        auto node = weak.Upgrade();
        ASSERT_NE(node, nullptr);
        rawCoordinator->Unregister(node->GetId());
        rawCoordinator->Register(node, [calls]() { ++*calls; });
        rawCoordinator->Flush(pipeline);
    });
    coordinator->Flush(pipeline);
    EXPECT_EQ(*calls, 1);
    coordinator->Flush(pipeline);
    EXPECT_EQ(*calls, 2);
}

/**
 * @tc.name: WFHF_ExplicitZeroZIndex
 * @tc.desc: A user rank of zero is preserved.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowHeaderFooterTest, WFHF_ExplicitZeroZIndex, TestSize.Level1)
{
    CreateWaterFlow();
    CreateWaterFlowItems(20);
    auto content = Edge(50.0f);
    content->GetRenderContext()->UpdateZIndex(0);
    pattern_->SetEdge(true, content, pattern_->BeginEdgeUpdate(true));
    CreateDone();
    auto parent = ParentScroll(300.0f);
    pattern_->SetSticky(StickyStyle::HEADER);
    pattern_->UpdateEdgePositions();
    EXPECT_EQ(Header()->GetRenderContext()->GetZIndexValue(-1), 0);
    EXPECT_EQ(content->GetRenderContext()->GetZIndexValue(-1), 0);
    content->GetRenderContext()->UpdateZIndex(7);
    pattern_->UpdateEdgePositions();
    EXPECT_EQ(Header()->GetRenderContext()->GetZIndexValue(-1), 7);
    pattern_->SetSticky(StickyStyle::NONE);
    EXPECT_EQ(Header()->GetRenderContext()->GetZIndexValue(-1), 7);
    EXPECT_EQ(content->GetRenderContext()->GetZIndexValue(-1), 7);
}

/**
 * @tc.name: WFHF_InvalidTransformRestoresNaturalPosition
 * @tc.desc: Singular and non-finite transforms cannot leave stale pinned geometry.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowHeaderFooterTest, WFHF_InvalidTransformRestoresNaturalPosition, TestSize.Level1)
{
    CreateWaterFlow();
    CreateWaterFlowItems(20);
    SetEdges();
    CreateDone();
    auto parent = ParentScroll(300.0f);
    auto render = AceType::MakeRefPtr<EdgeTransformRenderContext>();
    render->SetHostNode(frameNode_);
    frameNode_->renderContext_ = render;
    MoveHost(-200.0f);
    pattern_->SetEdgeGeometry(OffsetF(), 1000.0f);
    pattern_->SetSticky(StickyStyle::BOTH);
    EXPECT_FLOAT_EQ(Header()->GetGeometryNode()->GetMarginFrameOffset().GetY(), 200.0f);
    render->transform_ = Matrix4::CreateScale(1.0f, 0.0f, 1.0f);
    pattern_->UpdateEdgePositions();
    EXPECT_FLOAT_EQ(Header()->GetGeometryNode()->GetMarginFrameOffset().GetY(), 0.0f);
    EXPECT_FLOAT_EQ(GetChildRect(frameNode_, pattern_->GetEdgeIndexMap().FooterIndex()).Top(), 960.0f);
    render->transform_ = Matrix4::CreateTranslate(0.0f, std::numeric_limits<float>::quiet_NaN(), 0.0f);
    pattern_->UpdateEdgePositions();
    EXPECT_FLOAT_EQ(Header()->GetGeometryNode()->GetMarginFrameOffset().GetY(), 0.0f);
    render->transform_ = Matrix4::CreateIdentity();
    pattern_->UpdateEdgePositions();
    EXPECT_FLOAT_EQ(Header()->GetGeometryNode()->GetMarginFrameOffset().GetY(), 200.0f);
}

/**
 * @tc.name: WFHF_ComposedStickyInsets
 * @tc.desc: Layout-provided cumulative sticky insets replace, rather than double, parent offsets.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowHeaderFooterTest, WFHF_ComposedStickyInsets, TestSize.Level1)
{
    CreateWaterFlow();
    CreateWaterFlowItems(20);
    SetEdges();
    CreateDone();
    auto parent = ParentScroll(300.0f);
    parent->GetPattern<ScrollPattern>()->contentStartOffset_ = 10.0f;
    parent->GetPattern<ScrollPattern>()->contentEndOffset_ = 15.0f;
    layoutProperty_->ClearUserDefinedIdealSize(false, true);
    ASSERT_FALSE(layoutProperty_->GetNeedLazyLayout());
    auto constraint = layoutProperty_->GetLayoutConstraint().value();
    constraint.viewPosRef = ViewPosReference {
        .viewPosStart = 0.0f, .viewPosEnd = 300.0f, .referencePos = -200.0f,
        .stickyInsetStart = 60.0f, .stickyInsetEnd = 40.0f,
        .referenceEdge = ReferenceEdge::START, .axis = Axis::VERTICAL,
    };
    layoutProperty_->UpdateLayoutConstraint(constraint);
    MoveHost(-200.0f);
    pattern_->SetEdgeGeometry(OffsetF(), 1000.0f);
    pattern_->SetSticky(StickyStyle::BOTH);
    // Ordinary layout discards the lazy reference and uses the scroll parent's own offsets.
    ASSERT_FALSE(layoutProperty_->GetLayoutConstraint()->viewPosRef.has_value());
    EXPECT_FLOAT_EQ(Header()->GetGeometryNode()->GetMarginFrameOffset().GetY(), 210.0f);
    EXPECT_FLOAT_EQ(GetChildRect(frameNode_, pattern_->GetEdgeIndexMap().FooterIndex()).Top(), 445.0f);
    // An already established lazy context may supply cumulative insets; edges never enable it.
    layoutProperty_->SetNeedLazyLayout(true);
    layoutProperty_->UpdateLayoutConstraint(constraint);
    ASSERT_TRUE(layoutProperty_->GetLayoutConstraint()->viewPosRef.has_value());
    pattern_->UpdateEdgePositions();
    EXPECT_FLOAT_EQ(Header()->GetGeometryNode()->GetMarginFrameOffset().GetY(), 260.0f);
    EXPECT_FLOAT_EQ(GetChildRect(frameNode_, pattern_->GetEdgeIndexMap().FooterIndex()).Top(), 420.0f);
    constraint.viewPosRef->stickyInsetStart = 80.0f;
    layoutProperty_->UpdateLayoutConstraint(constraint);
    pattern_->UpdateEdgePositions();
    EXPECT_FLOAT_EQ(Header()->GetGeometryNode()->GetMarginFrameOffset().GetY(), 280.0f);
    // Inset boundaries may cross; never normalize them into a swapped viewport.
    constraint.viewPosRef->stickyInsetStart = 240.0f;
    constraint.viewPosRef->stickyInsetEnd = 200.0f;
    layoutProperty_->UpdateLayoutConstraint(constraint);
    pattern_->UpdateEdgePositions();
    EXPECT_FLOAT_EQ(Header()->GetGeometryNode()->GetMarginFrameOffset().GetY(), 440.0f);
    EXPECT_FLOAT_EQ(GetChildRect(frameNode_, pattern_->GetEdgeIndexMap().FooterIndex()).Top(), 260.0f);
    pattern_->SetSticky(StickyStyle::NONE);
    EXPECT_FLOAT_EQ(Header()->GetGeometryNode()->GetMarginFrameOffset().GetY(), 0.0f);
}

/**
 * @tc.name: WFHF_EdgesPreserveParentLayout
 * @tc.desc: Adding edges to an expanded WaterFlow does not enable parent-driven lazy layout.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowHeaderFooterTest, WFHF_EdgesPreserveParentLayout, TestSize.Level1)
{
    CreateWaterFlow();
    layoutProperty_->ClearUserDefinedIdealSize(false, true);
    auto builds = std::make_shared<int32_t>(0);
    CreateItemsInLazyForEach(20, [builds](int32_t) {
        ++*builds;
        return 100.0f;
    });
    ViewStackProcessor::GetInstance()->Finish();
    ViewStackProcessor::GetInstance()->StopGetAccessRecording();
    auto parent = ParentScroll(300.0f);
    parent->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(WATER_FLOW_WIDTH), CalcLength(300.0f)));
    parent->MarkModifyDone();
    MountToStageNode(parent);
    FlushUITasks(parent);
    const auto originalSize = frameNode_->GetGeometryNode()->GetFrameSize();
    const auto originalBuilds = *builds;
    const auto originalCache = layoutProperty_->GetCachedCount();
    ASSERT_FALSE(layoutProperty_->GetNeedLazyLayout());
    SetEdges();
    pattern_->SetSticky(StickyStyle::BOTH);
    FlushUITasks(parent);
    EXPECT_FALSE(layoutProperty_->GetNeedLazyLayout());
    EXPECT_EQ(layoutProperty_->GetCachedCount(), originalCache);
    EXPECT_EQ(pattern_->GetDataCount(), 20);
    EXPECT_EQ(*builds, originalBuilds);
    EXPECT_FLOAT_EQ(frameNode_->GetGeometryNode()->GetFrameSize().Height(), originalSize.Height() + 90.0f);
    pattern_->SetEdge(true, nullptr, pattern_->BeginEdgeUpdate(true));
    pattern_->SetEdge(false, nullptr, pattern_->BeginEdgeUpdate(false));
    FlushUITasks(parent);
    EXPECT_FALSE(layoutProperty_->GetNeedLazyLayout());
    EXPECT_EQ(frameNode_->GetGeometryNode()->GetFrameSize(), originalSize);
}

/**
 * @tc.name: WFHF_StickyOnlyKeepsLegacyParentLayout
 * @tc.desc: Sticky without a new edge preserves the expanded layout under a real Scroll parent.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowHeaderFooterTest, WFHF_StickyOnlyKeepsLegacyParentLayout, TestSize.Level1)
{
    CreateWaterFlow();
    layoutProperty_->ClearUserDefinedIdealSize(false, true);
    CreateItemsInLazyForEach(20, [](int32_t) { return 100.0f; });
    ViewStackProcessor::GetInstance()->Finish();
    ViewStackProcessor::GetInstance()->StopGetAccessRecording();
    auto parent = ParentScroll(300.0f);
    parent->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(WATER_FLOW_WIDTH), CalcLength(300.0f)));
    parent->MarkModifyDone();
    MountToStageNode(parent);
    FlushUITasks(parent);
    const auto originalSize = frameNode_->GetGeometryNode()->GetFrameSize();
    const auto originalRect = GetChildRect(frameNode_, 0);

    pattern_->SetSticky(StickyStyle::BOTH);
    frameNode_->MarkModifyDone();
    FlushUITasks(parent);
    EXPECT_FALSE(pattern_->GetEdgeState().enabled);
    EXPECT_FALSE(pattern_->edgeViewportRegistered_);
    EXPECT_FALSE(layoutProperty_->GetNeedLazyLayout());
    EXPECT_EQ(frameNode_->GetGeometryNode()->GetFrameSize(), originalSize);
    EXPECT_EQ(GetChildRect(frameNode_, 0), originalRect);
}

/**
 * @tc.name: WFHF_ClearedEdgesPreserveLazyMode
 * @tc.desc: Clearing and re-adding edges only changes sticky subscription, not lazy mode.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowHeaderFooterTest, WFHF_ClearedEdgesPreserveLazyMode, TestSize.Level1)
{
    CreateWaterFlow();
    layoutProperty_->ClearUserDefinedIdealSize(false, true);
    CreateItemsInLazyForEach(20, [](int32_t) { return 100.0f; });
    SetEdges();
    pattern_->SetSticky(StickyStyle::BOTH);
    ViewStackProcessor::GetInstance()->Finish();
    ViewStackProcessor::GetInstance()->StopGetAccessRecording();
    auto parent = ParentScroll(300.0f);
    parent->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(WATER_FLOW_WIDTH), CalcLength(300.0f)));
    parent->MarkModifyDone();
    MountToStageNode(parent);
    FlushUITasks(parent);
    ASSERT_TRUE(pattern_->edgeViewportRegistered_);

    pattern_->SetEdge(true, nullptr, pattern_->BeginEdgeUpdate(true));
    EXPECT_FALSE(layoutProperty_->GetNeedLazyLayout());
    pattern_->SetEdge(false, nullptr, pattern_->BeginEdgeUpdate(false));
    EXPECT_FALSE(layoutProperty_->GetNeedLazyLayout());
    EXPECT_FALSE(pattern_->edgeViewportRegistered_);
    FlushUITasks(parent);
    EXPECT_EQ(pattern_->GetDataCount(), 20);
    EXPECT_FALSE(pattern_->hasFooter());
    EXPECT_EQ(Header(), nullptr);
    pattern_->AddFooter(Edge(70.0f));
    EXPECT_FALSE(pattern_->hasFooter());

    SetEdges(60.0f, 30.0f);
    EXPECT_FALSE(layoutProperty_->GetNeedLazyLayout());
    FlushUITasks(parent);
    EXPECT_TRUE(pattern_->edgeViewportRegistered_);
    EXPECT_EQ(pattern_->GetEdgeState().sticky, StickyStyle::BOTH);
    EXPECT_EQ(pattern_->GetDataCount(), 20);
    ASSERT_NE(Header(), nullptr);
    EXPECT_FLOAT_EQ(Header()->GetGeometryNode()->GetFrameSize().Height(), 60.0f);
    EXPECT_TRUE(pattern_->hasFooter());
}

/**
 * @tc.name: WFHF_LegacyFooterRemovalPreservesLazyMode
 * @tc.desc: Removing the last legacy edge clears sticky subscription without changing lazy mode.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowHeaderFooterTest, WFHF_LegacyFooterRemovalPreservesLazyMode, TestSize.Level1)
{
    CreateWaterFlow();
    layoutProperty_->ClearUserDefinedIdealSize(false, true);
    CreateItemsInLazyForEach(20, [](int32_t) { return 100.0f; });
    auto legacy = Edge(70.0f);
    pattern_->AddFooter(legacy);
    pattern_->SetEdge(true, Edge(50.0f), pattern_->BeginEdgeUpdate(true));
    pattern_->SetSticky(StickyStyle::BOTH);
    ViewStackProcessor::GetInstance()->Finish();
    ViewStackProcessor::GetInstance()->StopGetAccessRecording();
    auto parent = ParentScroll(300.0f);
    parent->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(WATER_FLOW_WIDTH), CalcLength(300.0f)));
    parent->MarkModifyDone();
    MountToStageNode(parent);
    FlushUITasks(parent);
    ASSERT_TRUE(pattern_->edgeViewportRegistered_);
    ASSERT_FALSE(pattern_->GetEdgeState().footerControlled);

    pattern_->SetEdge(true, nullptr, pattern_->BeginEdgeUpdate(true));
    EXPECT_FALSE(layoutProperty_->GetNeedLazyLayout());
    EXPECT_TRUE(pattern_->hasFooter());
    pattern_->AddFooter(nullptr);
    EXPECT_FALSE(layoutProperty_->GetNeedLazyLayout());
    EXPECT_FALSE(pattern_->edgeViewportRegistered_);
    FlushUITasks(parent);
    EXPECT_FALSE(pattern_->hasFooter());
    EXPECT_EQ(Header(), nullptr);
    EXPECT_EQ(pattern_->GetDataCount(), 20);
}

/**
 * @tc.name: WFHF_AncestorTraversalIsLinear
 * @tc.desc: Each intermediate transform is sampled once per update, independent of depth.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowHeaderFooterTest, WFHF_AncestorTraversalIsLinear, TestSize.Level1)
{
    CreateWaterFlow();
    CreateWaterFlowItems(20);
    SetEdges();
    CreateDone();
    auto parent = ParentScroll(300.0f);
    parent->RemoveChild(frameNode_);
    auto ancestor = parent;
    std::vector<RefPtr<EdgeTransformRenderContext>> contexts;
    for (int32_t depth = 0; depth < 100; ++depth) {
        auto wrapper = Edge(1000.0f);
        auto render = AceType::MakeRefPtr<EdgeTransformRenderContext>();
        render->SetHostNode(wrapper);
        wrapper->renderContext_ = render;
        contexts.push_back(render);
        ancestor->AddChild(wrapper);
        ancestor = wrapper;
    }
    ancestor->AddChild(frameNode_);
    MoveHost(-200.0f);
    pattern_->SetEdgeGeometry(OffsetF(), 1000.0f);
    pattern_->SetSticky(StickyStyle::HEADER);
    for (auto& render : contexts) {
        render->queries_ = 0;
    }
    pattern_->UpdateEdgePositions();
    EXPECT_FLOAT_EQ(Header()->GetGeometryNode()->GetMarginFrameOffset().GetY(), 200.0f);
    for (const auto& render : contexts) {
        EXPECT_EQ(render->queries_, 1);
    }
}
/**
 * @tc.name: WFHF_ComponentContentIdentity
 * @tc.desc: Content replay preserves generation and wrapper; a newer content update invalidates a pending builder.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowHeaderFooterTest, WFHF_ComponentContentIdentity, TestSize.Level1)
{
    CreateWaterFlow();
    CreateWaterFlowItems(2);
    auto content = Edge(50.0f);
    pattern_->SetEdgeContent(true, content);
    const auto generation = pattern_->GetEdgeState().headerGeneration;
    auto wrapper = Header();
    pattern_->SetEdgeContent(true, content);
    EXPECT_EQ(pattern_->GetEdgeState().headerGeneration, generation);
    EXPECT_EQ(Header(), wrapper);
    const auto pending = pattern_->BeginEdgeUpdate(true);
    pattern_->SetEdgeContent(true, content);
    EXPECT_GT(pattern_->GetEdgeState().headerGeneration, pending);
    pattern_->SetEdge(true, Edge(80.0f), pending);
    EXPECT_EQ(Header(), wrapper);
    auto replacement = Edge(60.0f);
    pattern_->SetEdgeContent(true, replacement);
    EXPECT_NE(Header(), wrapper);
    EXPECT_EQ(Header()->GetFirstChild(), replacement);
    pattern_->SetEdge(true, nullptr, pattern_->BeginEdgeUpdate(true));
    EXPECT_EQ(Header(), nullptr);
    CreateDone();
}

/**
 * @tc.name: WFHF_InvalidContentPreservesTree
 * @tc.desc: Self, ancestor, peer content and already parented content cannot replace a valid edge.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowHeaderFooterTest, WFHF_InvalidContentPreservesTree, TestSize.Level1)
{
    CreateWaterFlow();
    CreateWaterFlowItems(2);
    auto body = frameNode_->GetFirstChild();
    auto header = Edge(50.0f);
    auto footer = Edge(40.0f);
    pattern_->SetEdgeContent(true, header);
    pattern_->SetEdgeContent(false, footer);
    CreateDone();
    auto original = Header();
    auto owner = Edge(100.0f);
    auto owned = Edge(20.0f);
    owner->AddChild(owned);
    auto ancestor = ParentScroll(300.0f);
    const std::vector<RefPtr<UINode>> invalid { frameNode_, ancestor, footer, body, owned };
    for (const auto& content : invalid) {
        pattern_->SetEdge(true, content, pattern_->BeginEdgeUpdate(true));
        EXPECT_EQ(Header(), original);
        EXPECT_EQ(original->GetFirstChild(), header);
        EXPECT_EQ(pattern_->GetDataCount(), 2);
    }
    EXPECT_EQ(owned->GetParent(), owner);
    EXPECT_EQ(frameNode_->GetParent(), ancestor);
    pattern_->SetEdge(false, header, pattern_->BeginEdgeUpdate(false));
    EXPECT_EQ(footer->GetParent()->GetParent(), frameNode_);
}

/**
 * @tc.name: WFHF_ExistingLazyFlagPreserved
 * @tc.desc: Edge configuration and cleanup never overwrite another capability's lazy marker.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowHeaderFooterTest, WFHF_ExistingLazyFlagPreserved, TestSize.Level1)
{
    CreateWaterFlow();
    CreateWaterFlowItems(2);
    layoutProperty_->SetNeedLazyLayout(true);
    SetEdges();
    CreateDone();
    pattern_->SetSticky(StickyStyle::BOTH);
    pattern_->SetEdge(true, nullptr, pattern_->BeginEdgeUpdate(true));
    pattern_->SetEdge(false, nullptr, pattern_->BeginEdgeUpdate(false));
    EXPECT_TRUE(layoutProperty_->GetNeedLazyLayout());
    pattern_->OnDetachFromMainTree();
    EXPECT_TRUE(layoutProperty_->GetNeedLazyLayout());
}

/**
 * @tc.name: WFHF_HeaderHandoffGeometry
 * @tc.desc: A sibling gap extends the header boundary only, and is reset on disable or detach.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowHeaderFooterTest, WFHF_HeaderHandoffGeometry, TestSize.Level1)
{
    CreateWaterFlow();
    CreateWaterFlowItems(20);
    SetEdges();
    CreateDone();
    auto parent = ParentScroll(300.0f);
    MoveHost(-980.0f);
    pattern_->SetEdgeGeometry(OffsetF(), 1000.0f);
    pattern_->SetSticky(StickyStyle::BOTH);
    EXPECT_FLOAT_EQ(Header()->GetGeometryNode()->GetMarginFrameOffset().GetY(), 910.0f);
    RefPtr<Pattern> section = pattern_;
    EXPECT_TRUE(section->HasStickyHeader());
    section->PublishNextStickyHeaderGap(20.0f);
    pattern_->UpdateEdgePositions();
    EXPECT_FLOAT_EQ(Header()->GetGeometryNode()->GetMarginFrameOffset().GetY(), 970.0f);
    EXPECT_FLOAT_EQ(GetChildRect(frameNode_, pattern_->GetEdgeIndexMap().FooterIndex()).Top(), 960.0f);
    EXPECT_FALSE(layoutProperty_->GetNeedLazyLayout());
    EXPECT_EQ(LazyLayoutUtils::GetStickyHeaderPattern(frameNode_), section);
    EXPECT_EQ(LazyLayoutUtils::GetLazyLayoutPattern(frameNode_), nullptr);
    pattern_->SetSticky(StickyStyle::NONE);
    pattern_->SetSticky(StickyStyle::HEADER);
    EXPECT_FLOAT_EQ(Header()->GetGeometryNode()->GetMarginFrameOffset().GetY(), 910.0f);
    section->PublishNextStickyHeaderGap(20.0f);
    pattern_->OnDetachFromMainTree();
    EXPECT_FALSE(pattern_->nextStickyHeaderGap_.has_value());
}
/**
 * @tc.name: WFHF_LazyColumnPublishesHandoff
 * @tc.desc: The parent publishes and clears the adjacent header gap without enabling lazy layout on WaterFlow.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowHeaderFooterTest, WFHF_LazyColumnPublishesHandoff, TestSize.Level1)
{
    CreateWaterFlow();
    CreateWaterFlowItems(2);
    SetEdges();
    CreateDone();
    pattern_->SetSticky(StickyStyle::HEADER);
    auto sibling = FrameNode::CreateFrameNode(V2::WATERFLOW_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<WaterFlowPattern>());
    auto siblingPattern = sibling->GetPattern<WaterFlowPattern>();
    siblingPattern->SetEdgeContent(true, Edge(60.0f));
    siblingPattern->SetSticky(StickyStyle::HEADER);
    auto parent = Edge(1000.0f);
    parent->AddChild(frameNode_);
    parent->AddChild(sibling);
    auto first = parent->GetOrCreateChildByIndex(0);
    auto second = parent->GetOrCreateChildByIndex(1);
    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);
    LazyColumnLayoutAlgorithm algorithm(AceType::MakeRefPtr<LazyColumnLayoutInfo>());
    algorithm.totalItemCount_ = 2;
    algorithm.space_ = 12.0f;
    algorithm.UpdateSiblingHandoffGap(AceType::RawPtr(parent), second, 1);
    ASSERT_TRUE(pattern_->nextStickyHeaderGap_.has_value());
    EXPECT_FLOAT_EQ(pattern_->nextStickyHeaderGap_.value(), 12.0f);
    EXPECT_FALSE(siblingPattern->nextStickyHeaderGap_.has_value());
    EXPECT_FALSE(layoutProperty_->GetNeedLazyLayout());
    EXPECT_FALSE(sibling->GetLayoutProperty()->GetNeedLazyLayout());
    siblingPattern->SetSticky(StickyStyle::NONE);
    algorithm.UpdateSiblingHandoffGap(AceType::RawPtr(parent), second, 1);
    EXPECT_FALSE(pattern_->nextStickyHeaderGap_.has_value());
}
} // namespace OHOS::Ace::NG

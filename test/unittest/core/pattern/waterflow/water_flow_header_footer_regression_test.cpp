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
#include "core/components_ng/pattern/scroll/scroll_pattern.h"
#include "core/components_ng/pattern/waterflow/layout/top_down/water_flow_layout_algorithm.h"
#undef private

#include <limits>
#include <memory>

#include "test/mock/frameworks/core/components_ng/render/mock_render_context.h"
#include "core/components_ng/pattern/scrollable/header_footer_utils.h"
#include "core/components_ng/pattern/lazy_layout/lazy_layout_pattern.h"
#include "core/components_ng/pattern/lazy_layout/lazy_layout_utils.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"
#include "core/components_ng/pattern/refresh/refresh_pattern.h"
#include "core/components_ng/base/scroll_viewport_coordinator.h"
#include "core/components_ng/pattern/swiper/swiper_pattern.h"
#include "core/components_ng/syntax/lazy_for_each_node.h"
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

class HandoffCountingLazyPattern : public LazyLayoutPattern {
    DECLARE_ACE_TYPE(HandoffCountingLazyPattern, LazyLayoutPattern);
public:
    AdjustOffset GetAdjustOffset() const override
    {
        return {};
    }

    AdjustOffset GetAndResetAdjustOffset() override
    {
        return {};
    }

    bool HasStickyHeader() const override
    {
        return true;
    }

    void PublishNextStickyHeaderGap(const std::optional<float>& gap) override
    {
        ++publications_;
        LazyLayoutPattern::PublishNextStickyHeaderGap(gap);
    }

    int32_t publications_ = 0;
};

} // namespace
class WaterFlowHeaderFooterRegressionTest : public WaterFlowTestNg {
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
 * @tc.name: WFHF_DeviceScrollEdgeIncludesFooter
 * @tc.desc: Bottom targets the footer while an explicit last index targets the last FlowItem.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowHeaderFooterRegressionTest, WFHF_DeviceScrollEdgeIncludesFooter, TestSize.Level1)
{
    CreateWaterFlow();
    CreateWaterFlowItems(20);
    SetEdges(60.0f, 40.0f);
    CreateDone();
    ScrollToIndex(19, false, ScrollAlign::END);
    auto map = pattern_->GetEdgeIndexMap();
    EXPECT_FLOAT_EQ(GetChildRect(frameNode_, map.DataToNode(19)).Bottom(), WATER_FLOW_HEIGHT);
    pattern_->ScrollToEdge(ScrollEdgeType::SCROLL_BOTTOM, false);
    FlushUITasks();
    EXPECT_FLOAT_EQ(GetChildRect(frameNode_, map.FooterIndex()).Bottom(), WATER_FLOW_HEIGHT);
    EXPECT_EQ(pattern_->GetDataCount(), 20);
    EXPECT_EQ(pattern_->GetEndIndex(), 19);
}

/**
 * @tc.name: WFHF_DeviceClearScrolledSections
 * @tc.desc: Clearing scrolled sections restores the same edge positions as initially empty data.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowHeaderFooterRegressionTest, WFHF_DeviceClearScrolledSections, TestSize.Level1)
{
    CreateWaterFlow();
    CreateWaterFlowItems(20);
    SetEdges(60.0f, 40.0f);
    auto sections = pattern_->GetOrCreateWaterFlowSections();
    WaterFlowSections::Section section;
    section.itemsCount = 20;
    section.crossCount = 1;
    sections->ChangeData(0, 0, { section });
    CreateDone();
    ScrollToIndex(19, false, ScrollAlign::END);
    EXPECT_GT(pattern_->GetTotalOffset(), 0.0f);
    for (int32_t index = 19; index >= 0; --index) {
        frameNode_->RemoveChildAtIndex(pattern_->GetEdgeIndexMap().DataToNode(index));
    }
    sections->ChangeData(0, 1, {});
    frameNode_->ChildrenUpdatedFrom(0);
    frameNode_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    MockPipelineContext::GetCurrent()->FlushBuildFinishCallbacks();
    FlushUITasks();
    EXPECT_EQ(pattern_->GetDataCount(), 0);
    EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), 0.0f);
    EXPECT_FLOAT_EQ(GetChildRect(frameNode_, pattern_->GetEdgeIndexMap().HeaderIndex()).Top(), 0.0f);
    EXPECT_FLOAT_EQ(GetChildRect(frameNode_, pattern_->GetEdgeIndexMap().FooterIndex()).Top(), 60.0f);
}

/**
 * @tc.name: WFHF_Empty
 * @tc.desc: Edges have a natural position without FlowItems.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowHeaderFooterRegressionTest, WFHF_Empty, TestSize.Level1)
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
HWTEST_F(WaterFlowHeaderFooterRegressionTest, WFHF_InvalidDataKeepsEdges, TestSize.Level1)
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
HWTEST_F(WaterFlowHeaderFooterRegressionTest, WFHF_TargetMeasureKeepsFooter, TestSize.Level1)
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
HWTEST_F(WaterFlowHeaderFooterRegressionTest, WFHF_Standalone, TestSize.Level1)
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
HWTEST_F(WaterFlowHeaderFooterRegressionTest, WFHF_StickyOnlyKeepsLegacyFooter, TestSize.Level1)
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
        EXPECT_FALSE(pattern_->edgeState_.viewportRegistered);
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
HWTEST_F(WaterFlowHeaderFooterRegressionTest, WFHF_EmptyEdgeUpdatesKeepLegacyMode, TestSize.Level1)
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
    EXPECT_FALSE(pattern_->edgeState_.viewportRegistered);
    EXPECT_FALSE(layoutProperty_->GetNeedLazyLayout());
    EXPECT_EQ(pattern_->GetChildrenCount(), 2);
    EXPECT_EQ(GetChildRect(frameNode_, 0), originalRect);
}

/**
 * @tc.name: WFHF_UnreservedGenerationKeepsLegacyTree
 * @tc.desc: An edge result without a reserved update token cannot activate the new layout.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowHeaderFooterRegressionTest, WFHF_UnreservedGenerationKeepsLegacyTree, TestSize.Level1)
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
HWTEST_F(WaterFlowHeaderFooterRegressionTest, WFHF_StaleBuilder, TestSize.Level1)
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
HWTEST_F(WaterFlowHeaderFooterRegressionTest, WFHF_StaleBuilderKeepsLegacyTree, TestSize.Level1)
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
HWTEST_F(WaterFlowHeaderFooterRegressionTest, WFHF_StickyKeepsPendingBuilder, TestSize.Level1)
{
    CreateWaterFlow();
    CreateWaterFlowItems(2);
    const auto generation = pattern_->BeginEdgeUpdate(true);
    pattern_->SetSticky(StickyStyle::HEADER);
    EXPECT_FALSE(pattern_->GetEdgeState().enabled);
    EXPECT_FALSE(pattern_->layoutInfo_->edgeSlots_);
    EXPECT_FALSE(pattern_->edgeState_.viewportRegistered);
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
HWTEST_F(WaterFlowHeaderFooterRegressionTest, WFHF_IntrinsicEdgeHeight, TestSize.Level1)
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
HWTEST_F(WaterFlowHeaderFooterRegressionTest, WFHF_DetachInvalidatesBuilder, TestSize.Level1)
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
HWTEST_F(WaterFlowHeaderFooterRegressionTest, WFHF_FooterTakeover, TestSize.Level1)
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
HWTEST_F(WaterFlowHeaderFooterRegressionTest, WFHF_FooterEnablesPendingSticky, TestSize.Level1)
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
HWTEST_F(WaterFlowHeaderFooterRegressionTest, WFHF_DirectionReset, TestSize.Level1)
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
HWTEST_F(WaterFlowHeaderFooterRegressionTest, WFHF_ParentOnlyMove, TestSize.Level1)
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
    EXPECT_FALSE(pattern_->edgeState_.viewportRegistered);
    EXPECT_FLOAT_EQ(Header()->GetGeometryNode()->GetMarginFrameOffset().GetY(), 0.0f);
    EXPECT_FLOAT_EQ(GetChildRect(frameNode_, pattern_->GetEdgeIndexMap().FooterIndex()).Top(), 960.0f);
    EXPECT_EQ(Header()->GetRenderContext()->GetZIndexValue(-1), 0);
}

/**
 * @tc.name: WFHF_ZeroClipRestoresNaturalPosition
 * @tc.desc: A zero-height clipping ancestor blocks pinning until its next layout gives it a viewport.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowHeaderFooterRegressionTest, WFHF_ZeroClipRestoresNaturalPosition, TestSize.Level1)
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
HWTEST_F(WaterFlowHeaderFooterRegressionTest, WFHF_ReverseParentBlocksOuterViewport, TestSize.Level1)
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
HWTEST_F(WaterFlowHeaderFooterRegressionTest, WFHF_UnsupportedParentBlocksOuterViewport, TestSize.Level1)
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
HWTEST_F(WaterFlowHeaderFooterRegressionTest, WFHF_CoordinatorMutationAndReentry, TestSize.Level1)
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
 * @tc.name: WFHF_UnknownTailBoundary
 * @tc.desc: Unmeasured short items must not let estimated length move sticky edges past the actual content.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowHeaderFooterRegressionTest, WFHF_UnknownTailBoundary, TestSize.Level1)
{
    CreateWaterFlow();
    CreateItemWithHeight(1000.0f);
    for (int32_t i = 0; i < 99; ++i) {
        CreateItemWithHeight(1.0f);
    }
    SetEdges();
    CreateDone();
    ASSERT_LT(pattern_->GetEndIndex(), 99);
    const auto naturalSize = pattern_->edgeState_.totalSize;
    auto parent = ParentScroll(2000.0f);
    pattern_->SetSticky(StickyStyle::BOTH);
    auto pipeline = frameNode_->GetContext();
    ASSERT_NE(pipeline, nullptr);
    pipeline->GetScrollViewportCoordinator()->Flush(pipeline);
    const auto footerIndex = pattern_->GetEdgeIndexMap().FooterIndex();
    // The fixture knows the complete data heights; the layout has only measured the first window.
    EXPECT_LE(GetChildRect(frameNode_, footerIndex).Bottom(), 50.0f + 1000.0f + 99.0f + 40.0f);
    EXPECT_FLOAT_EQ(pattern_->edgeState_.totalSize, naturalSize);
    EXPECT_LT(pattern_->GetEndIndex(), 99);

    pattern_->ScrollToIndex(99, false, ScrollAlign::END);
    FlushUITasks();
    // FlushUITasks also measures the synthetic Scroll; restore this test's large viewport.
    parent->GetGeometryNode()->SetFrameSize(SizeF(WATER_FLOW_WIDTH, 2000.0f));
    parent->GetRenderContext()->UpdatePaintRect(parent->GetGeometryNode()->GetFrameRect());
    pipeline->GetScrollViewportCoordinator()->Flush(pipeline);
    EXPECT_EQ(pattern_->GetEndIndex(), 99);
    EXPECT_FLOAT_EQ(GetChildRect(frameNode_, footerIndex).Top(),
        GetChildRect(frameNode_, pattern_->GetEdgeIndexMap().DataToNode(99)).Bottom());
    const auto finalSize = pattern_->edgeState_.totalSize;
    pattern_->SetSticky(StickyStyle::NONE);
    EXPECT_FLOAT_EQ(pattern_->edgeState_.totalSize, finalSize);
}

/**
 * @tc.name: WFHF_CoordinatorNestedMovement
 * @tc.desc: A child created before its parent observes the parent's new edge position in the same flush.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowHeaderFooterRegressionTest, WFHF_CoordinatorNestedMovement, TestSize.Level1)
{
    CreateWaterFlow();
    CreateWaterFlowItems(20);
    SetEdges();
    CreateDone();
    auto parent = ParentScroll(300.0f);
    // ViewStack IDs and ElementRegister IDs use different counters in this fixture.
    auto movingEdge = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, frameNode_->GetId() + 100000,
        AceType::MakeRefPtr<LinearLayoutPattern>(true));
    parent->RemoveChild(frameNode_);
    parent->AddChild(movingEdge);
    movingEdge->AddChild(frameNode_);
    ASSERT_LT(frameNode_->GetId(), movingEdge->GetId());
    MoveHost(-200.0f);
    pattern_->SetEdgeGeometry(OffsetF(), 1000.0f);
    pattern_->SetSticky(StickyStyle::BOTH);
    auto pipeline = frameNode_->GetContext();
    ASSERT_NE(pipeline, nullptr);
    auto coordinator = pipeline->GetScrollViewportCoordinator();
    coordinator->Register(movingEdge, [weak = WeakPtr<FrameNode>(movingEdge)]() {
        auto node = weak.Upgrade();
        ASSERT_NE(node, nullptr);
        node->GetGeometryNode()->SetFrameOffset(OffsetF(0.0f, 100.0f));
        node->GetRenderContext()->UpdatePaintRect(node->GetGeometryNode()->GetFrameRect());
    });
    coordinator->Flush(pipeline);
    EXPECT_FLOAT_EQ(Header()->GetGeometryNode()->GetMarginFrameOffset().GetY(), 100.0f);
    EXPECT_FLOAT_EQ(GetChildRect(frameNode_, pattern_->GetEdgeIndexMap().FooterIndex()).Top(), 360.0f);

    // Reattach under a different moving ancestor; registration order and old depths must not persist.
    pattern_->OnDetachFromFrameNode(AceType::RawPtr(frameNode_));
    coordinator->Unregister(movingEdge->GetId());
    movingEdge->RemoveChild(frameNode_);
    auto replacement = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, frameNode_->GetId() + 100001,
        AceType::MakeRefPtr<LinearLayoutPattern>(true));
    parent->AddChild(replacement);
    replacement->AddChild(frameNode_);
    pattern_->SetEdgeGeometry(OffsetF(), 1000.0f);
    coordinator->Register(replacement, [weak = WeakPtr<FrameNode>(replacement)]() {
        auto node = weak.Upgrade();
        ASSERT_NE(node, nullptr);
        node->GetGeometryNode()->SetFrameOffset(OffsetF(0.0f, 50.0f));
        node->GetRenderContext()->UpdatePaintRect(node->GetGeometryNode()->GetFrameRect());
    });
    coordinator->Flush(pipeline);
    EXPECT_FLOAT_EQ(Header()->GetGeometryNode()->GetMarginFrameOffset().GetY(), 150.0f);
    EXPECT_FLOAT_EQ(GetChildRect(frameNode_, pattern_->GetEdgeIndexMap().FooterIndex()).Top(), 410.0f);
    coordinator->Unregister(replacement->GetId());
}

/**
 * @tc.name: WFHF_UnknownTailLazyChanges
 * @tc.desc: Sticky uses the measured window across scroll/resize/reload without building the unknown tail.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowHeaderFooterRegressionTest, WFHF_UnknownTailLazyChanges, TestSize.Level1)
{
    CreateWaterFlow();
    auto built = std::make_shared<int32_t>(0);
    auto lazy = CreateItemsInLazyForEach(1000, [built](int32_t index) {
        ++*built;
        return 70.0f + (index % 3) * 130.0f;
    });
    SetEdges();
    CreateDone();
    ASSERT_LT(*built, 1000);
    auto lazyNode = AceType::DynamicCast<LazyForEachNode>(frameNode_->GetChildAtIndex(1));
    ASSERT_NE(lazyNode, nullptr);
    auto parent = ParentScroll(300.0f);
    auto pipeline = frameNode_->GetContext();
    ASSERT_NE(pipeline, nullptr);
    auto refresh = [this, parent, pipeline](float offset) {
        parent->GetGeometryNode()->SetFrameSize(SizeF(WATER_FLOW_WIDTH, 300.0f));
        parent->GetRenderContext()->UpdatePaintRect(parent->GetGeometryNode()->GetFrameRect());
        MoveHost(offset);
        pipeline->GetScrollViewportCoordinator()->Flush(pipeline);
    };
    const auto before = *built;
    const auto naturalSize = pattern_->edgeState_.totalSize;
    pattern_->SetSticky(StickyStyle::BOTH);
    refresh(-200.0f);
    EXPECT_EQ(*built, before);
    EXPECT_FLOAT_EQ(pattern_->edgeState_.totalSize, naturalSize);
    for (int32_t i = 0; i < 6; ++i) {
        UpdateCurrentOffset(-150.0f);
        refresh(-200.0f);
        EXPECT_FLOAT_EQ(Header()->GetGeometryNode()->GetMarginFrameOffset().GetY(), 200.0f);
        EXPECT_FLOAT_EQ(GetChildRect(frameNode_, pattern_->GetEdgeIndexMap().FooterIndex()).Top(), 460.0f);
        EXPECT_LT(*built, 1000);
    }
    pattern_->SetEdge(true, Edge(80.0f), pattern_->BeginEdgeUpdate(true));
    FlushUITasks();
    refresh(-200.0f);
    EXPECT_FLOAT_EQ(Header()->GetGeometryNode()->GetMarginFrameOffset().GetY(), 200.0f);
    EXPECT_FLOAT_EQ(GetChildRect(frameNode_, pattern_->GetEdgeIndexMap().FooterIndex()).Top(), 460.0f);

    lazy->SetTotalCount(3);
    lazyNode->OnDataReloaded();
    pattern_->ScrollToIndex(0, false, ScrollAlign::START);
    FlushUITasks();
    refresh(0.0f);
    EXPECT_FLOAT_EQ(pattern_->edgeState_.totalSize, 80.0f + 70.0f + 200.0f + 330.0f + 40.0f);
    pattern_->SetSticky(StickyStyle::NONE);
    EXPECT_FLOAT_EQ(GetChildRect(frameNode_, pattern_->GetEdgeIndexMap().FooterIndex()).Top(), 680.0f);

    lazy->SetTotalCount(0);
    lazyNode->OnDataReloaded();
    FlushUITasks();
    refresh(0.0f);
    EXPECT_EQ(pattern_->GetDataCount(), 0);
    EXPECT_FLOAT_EQ(GetChildRect(frameNode_, pattern_->GetEdgeIndexMap().FooterIndex()).Top(), 80.0f);
    EXPECT_FLOAT_EQ(pattern_->edgeState_.totalSize, 120.0f);
}

/**
 * @tc.name: WFHF_CoordinatorThreeLevels
 * @tc.desc: Reverse IDs and depth changes preserve a parent-before-descendant refresh snapshot.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowHeaderFooterRegressionTest, WFHF_CoordinatorThreeLevels, TestSize.Level1)
{
    CreateWaterFlow();
    CreateDone();
    auto middle = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, frameNode_->GetId() + 100000,
        AceType::MakeRefPtr<LinearLayoutPattern>(true));
    auto ancestor = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, frameNode_->GetId() + 100001,
        AceType::MakeRefPtr<LinearLayoutPattern>(true));
    ancestor->AddChild(middle);
    middle->AddChild(frameNode_);
    auto coordinator = AceType::MakeRefPtr<ScrollViewportCoordinator>();
    auto calls = std::make_shared<std::vector<int32_t>>();
    coordinator->Register(frameNode_, [calls]() { calls->push_back(3); });
    coordinator->Register(middle, [calls]() { calls->push_back(2); });
    coordinator->Register(ancestor, [calls]() { calls->push_back(1); });
    auto pipeline = frameNode_->GetContext();
    ASSERT_NE(pipeline, nullptr);
    coordinator->Flush(pipeline);
    EXPECT_EQ(*calls, (std::vector<int32_t> { 1, 2, 3 }));
    calls->clear();
    middle->RemoveChild(frameNode_);
    ancestor->RemoveChild(middle);
    ancestor->AddChild(frameNode_);
    frameNode_->AddChild(middle);
    coordinator->Flush(pipeline);
    EXPECT_EQ(*calls, (std::vector<int32_t> { 1, 3, 2 }));
    calls->clear();
    coordinator->Unregister(frameNode_->GetId());
    coordinator->Flush(pipeline);
    EXPECT_EQ(*calls, (std::vector<int32_t> { 1, 2 }));
}

/**
 * @tc.name: WFHF_CoordinatorInvalidatedSnapshot
 * @tc.desc: A parent removing or replacing a descendant registration must invalidate its collected callback.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowHeaderFooterRegressionTest, WFHF_CoordinatorInvalidatedSnapshot, TestSize.Level1)
{
    CreateWaterFlow();
    CreateDone();
    auto parent = ParentScroll(300.0f);
    auto coordinator = AceType::MakeRefPtr<ScrollViewportCoordinator>();
    auto* raw = AceType::RawPtr(coordinator);
    auto calls = std::make_shared<std::vector<int32_t>>();
    auto replaced = std::make_shared<bool>(false);
    coordinator->Register(frameNode_, [calls]() { calls->push_back(2); });
    coordinator->Register(parent, [raw, calls, replaced, weak = WeakPtr<FrameNode>(frameNode_)]() {
        calls->push_back(1);
        if (*replaced) {
            return;
        }
        *replaced = true;
        auto child = weak.Upgrade();
        ASSERT_NE(child, nullptr);
        raw->Unregister(child->GetId());
        raw->Register(child, [calls]() { calls->push_back(3); });
    });
    auto pipeline = frameNode_->GetContext();
    ASSERT_NE(pipeline, nullptr);
    coordinator->Flush(pipeline);
    EXPECT_EQ(*calls, (std::vector<int32_t> { 1 }));
    coordinator->Flush(pipeline);
    EXPECT_EQ(*calls, (std::vector<int32_t> { 1, 1, 3 }));
}

/**
 * @tc.name: WFHF_ExplicitZeroZIndex
 * @tc.desc: A user rank of zero is preserved.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowHeaderFooterRegressionTest, WFHF_ExplicitZeroZIndex, TestSize.Level1)
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
HWTEST_F(WaterFlowHeaderFooterRegressionTest, WFHF_InvalidTransformRestoresNaturalPosition, TestSize.Level1)
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
HWTEST_F(WaterFlowHeaderFooterRegressionTest, WFHF_ComposedStickyInsets, TestSize.Level1)
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
HWTEST_F(WaterFlowHeaderFooterRegressionTest, WFHF_EdgesPreserveParentLayout, TestSize.Level1)
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
HWTEST_F(WaterFlowHeaderFooterRegressionTest, WFHF_StickyOnlyKeepsLegacyParentLayout, TestSize.Level1)
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
    EXPECT_FALSE(pattern_->edgeState_.viewportRegistered);
    EXPECT_FALSE(layoutProperty_->GetNeedLazyLayout());
    EXPECT_EQ(frameNode_->GetGeometryNode()->GetFrameSize(), originalSize);
    EXPECT_EQ(GetChildRect(frameNode_, 0), originalRect);
}

/**
 * @tc.name: WFHF_ClearedEdgesPreserveLazyMode
 * @tc.desc: Clearing and re-adding edges only changes sticky subscription, not lazy mode.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowHeaderFooterRegressionTest, WFHF_ClearedEdgesPreserveLazyMode, TestSize.Level1)
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
    ASSERT_TRUE(pattern_->edgeState_.viewportRegistered);

    pattern_->SetEdge(true, nullptr, pattern_->BeginEdgeUpdate(true));
    EXPECT_FALSE(layoutProperty_->GetNeedLazyLayout());
    pattern_->SetEdge(false, nullptr, pattern_->BeginEdgeUpdate(false));
    EXPECT_FALSE(layoutProperty_->GetNeedLazyLayout());
    EXPECT_FALSE(pattern_->edgeState_.viewportRegistered);
    FlushUITasks(parent);
    EXPECT_EQ(pattern_->GetDataCount(), 20);
    EXPECT_FALSE(pattern_->hasFooter());
    EXPECT_EQ(Header(), nullptr);
    pattern_->AddFooter(Edge(70.0f));
    EXPECT_FALSE(pattern_->hasFooter());

    SetEdges(60.0f, 30.0f);
    EXPECT_FALSE(layoutProperty_->GetNeedLazyLayout());
    FlushUITasks(parent);
    EXPECT_TRUE(pattern_->edgeState_.viewportRegistered);
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
HWTEST_F(WaterFlowHeaderFooterRegressionTest, WFHF_LegacyFooterRemovalPreservesLazyMode, TestSize.Level1)
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
    ASSERT_TRUE(pattern_->edgeState_.viewportRegistered);
    ASSERT_FALSE(pattern_->GetEdgeState().footerControlled);

    pattern_->SetEdge(true, nullptr, pattern_->BeginEdgeUpdate(true));
    EXPECT_FALSE(layoutProperty_->GetNeedLazyLayout());
    EXPECT_TRUE(pattern_->hasFooter());
    pattern_->AddFooter(nullptr);
    EXPECT_FALSE(layoutProperty_->GetNeedLazyLayout());
    EXPECT_FALSE(pattern_->edgeState_.viewportRegistered);
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
HWTEST_F(WaterFlowHeaderFooterRegressionTest, WFHF_AncestorTraversalIsLinear, TestSize.Level1)
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
HWTEST_F(WaterFlowHeaderFooterRegressionTest, WFHF_ComponentContentIdentity, TestSize.Level1)
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
HWTEST_F(WaterFlowHeaderFooterRegressionTest, WFHF_InvalidContentPreservesTree, TestSize.Level1)
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
HWTEST_F(WaterFlowHeaderFooterRegressionTest, WFHF_ExistingLazyFlagPreserved, TestSize.Level1)
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
 * @tc.name: WFHF_StickyCapabilityIgnoresTags
 * @tc.desc: Sticky handoff depends on the pattern capability, not a node's display tag.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowHeaderFooterRegressionTest, WFHF_StickyCapabilityIgnoresTags, TestSize.Level1)
{
    auto fake = FrameNode::CreateFrameNode(V2::WATERFLOW_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    EXPECT_EQ(LazyLayoutUtils::GetStickyHeaderPattern(fake), nullptr);
    auto actual = FrameNode::CreateFrameNode("StickySection",
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<WaterFlowPattern>());
    EXPECT_NE(LazyLayoutUtils::GetStickyHeaderPattern(actual), nullptr);
    EXPECT_EQ(LazyLayoutUtils::GetLazyLayoutPattern(actual), nullptr);
}

/**
 * @tc.name: WFHF_HeaderHandoffGeometry
 * @tc.desc: A sibling gap extends the header boundary only, and is reset on disable or detach.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowHeaderFooterRegressionTest, WFHF_HeaderHandoffGeometry, TestSize.Level1)
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
    RefPtr<Pattern> base = pattern_;
    auto section = AceType::DynamicCast<StickyHeaderPattern>(base);
    ASSERT_NE(section, nullptr);
    EXPECT_EQ(AceType::DynamicCast<WaterFlowPattern>(section), pattern_);
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
    EXPECT_FALSE(pattern_->edgeState_.nextStickyHeaderGap.has_value());
}
/**
 * @tc.name: WFHF_LazyColumnPublishesHandoff
 * @tc.desc: The parent publishes and clears the adjacent header gap without enabling lazy layout on WaterFlow.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowHeaderFooterRegressionTest, WFHF_LazyColumnPublishesHandoff, TestSize.Level1)
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
    auto info = AceType::MakeRefPtr<LazyColumnLayoutInfo>();
    LazyColumnLayoutAlgorithm algorithm(info);
    algorithm.totalItemCount_ = 2;
    algorithm.space_ = 12.0f;
    algorithm.UpdateSiblingHandoffGap(AceType::RawPtr(parent), second, 1);
    ASSERT_TRUE(pattern_->edgeState_.nextStickyHeaderGap.has_value());
    EXPECT_FLOAT_EQ(pattern_->edgeState_.nextStickyHeaderGap.value(), 12.0f);
    EXPECT_FALSE(siblingPattern->edgeState_.nextStickyHeaderGap.has_value());
    EXPECT_FALSE(layoutProperty_->GetNeedLazyLayout());
    EXPECT_FALSE(sibling->GetLayoutProperty()->GetNeedLazyLayout());
    info->posMap_ = { { 0, { 0.0f, 500.0f } }, { 1, { 512.0f, 1012.0f } } };
    info->endIndex_ = 0;
    algorithm.cacheEndPos_ = 1200.0f;
    siblingPattern->SetSticky(StickyStyle::NONE);
    algorithm.LayoutCachedItemsForward(AceType::RawPtr(parent), WATER_FLOW_WIDTH, OffsetF());
    EXPECT_FALSE(pattern_->edgeState_.nextStickyHeaderGap.has_value());
    siblingPattern->SetSticky(StickyStyle::HEADER);
    info->startIndex_ = 2;
    algorithm.cacheStartPos_ = 500.0f;
    algorithm.LayoutCachedItemsBackward(AceType::RawPtr(parent), WATER_FLOW_WIDTH, OffsetF());
    ASSERT_TRUE(pattern_->edgeState_.nextStickyHeaderGap.has_value());
    EXPECT_FLOAT_EQ(pattern_->edgeState_.nextStickyHeaderGap.value(), 12.0f);
}
/**
 * @tc.name: WFHF_LazyOnlyLayoutKeepsExistingHandoff
 * @tc.desc: Lazy-only layout refreshes clear stale tail state without republishing the predecessor's measured gap.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowHeaderFooterRegressionTest, WFHF_LazyOnlyLayoutKeepsExistingHandoff, TestSize.Level1)
{
    auto firstPattern = AceType::MakeRefPtr<HandoffCountingLazyPattern>();
    auto secondPattern = AceType::MakeRefPtr<HandoffCountingLazyPattern>();
    auto parent = Edge(1000.0f);
    for (const auto& pattern : { firstPattern, secondPattern }) {
        parent->AddChild(FrameNode::CreateFrameNode(V2::LAZY_COLUMN_LAYOUT_ETS_TAG,
            ElementRegister::GetInstance()->MakeUniqueId(), pattern));
    }
    ASSERT_NE(parent->GetOrCreateChildByIndex(0), nullptr);
    auto second = parent->GetOrCreateChildByIndex(1);
    ASSERT_NE(second, nullptr);
    auto info = AceType::MakeRefPtr<LazyColumnLayoutInfo>();
    info->posMap_ = { { 0, { 0.0f, 500.0f } }, { 1, { 512.0f, 1012.0f } } };
    LazyColumnLayoutAlgorithm algorithm(info);
    algorithm.totalItemCount_ = 2;
    algorithm.space_ = 12.0f;
    info->startIndex_ = 0;
    info->endIndex_ = 1;
    firstPattern->PublishNextStickyHeaderGap(12.0f);
    secondPattern->PublishNextStickyHeaderGap(24.0f);
    firstPattern->publications_ = secondPattern->publications_ = 0;
    algorithm.LayoutItems(AceType::RawPtr(parent), WATER_FLOW_WIDTH, OffsetF());
    EXPECT_EQ(firstPattern->publications_, 0);
    EXPECT_EQ(secondPattern->publications_, 1);
    EXPECT_EQ(firstPattern->GetNextStickyHeaderGap(), std::optional<float>(12.0f));
    EXPECT_FALSE(secondPattern->GetNextStickyHeaderGap().has_value());
    secondPattern->PublishNextStickyHeaderGap(24.0f);
    firstPattern->publications_ = secondPattern->publications_ = 0;
    info->endIndex_ = 0;
    algorithm.cacheEndPos_ = 1200.0f;
    algorithm.LayoutCachedItemsForward(AceType::RawPtr(parent), WATER_FLOW_WIDTH, OffsetF());
    EXPECT_EQ(firstPattern->publications_, 0);
    EXPECT_EQ(secondPattern->publications_, 1);
    EXPECT_EQ(firstPattern->GetNextStickyHeaderGap(), std::optional<float>(12.0f));
    EXPECT_FALSE(secondPattern->GetNextStickyHeaderGap().has_value());
    secondPattern->PublishNextStickyHeaderGap(24.0f);
    firstPattern->publications_ = secondPattern->publications_ = 0;
    info->startIndex_ = 2;
    algorithm.cacheStartPos_ = 500.0f;
    algorithm.LayoutCachedItemsBackward(AceType::RawPtr(parent), WATER_FLOW_WIDTH, OffsetF());
    EXPECT_EQ(firstPattern->publications_, 0);
    EXPECT_EQ(secondPattern->publications_, 1);
    EXPECT_EQ(firstPattern->GetNextStickyHeaderGap(), std::optional<float>(12.0f));
    EXPECT_FALSE(secondPattern->GetNextStickyHeaderGap().has_value());
    // The existing measurement publication remains available to Lazy-only sections.
    firstPattern->publications_ = secondPattern->publications_ = 0;
    algorithm.UpdateSiblingHandoffGap(AceType::RawPtr(parent), second, 1);
    EXPECT_EQ(firstPattern->publications_, 1);
    EXPECT_EQ(secondPattern->publications_, 1);
}
/**
 * @tc.name: WFHF_PaddingUsesParentContentViewport
 * @tc.desc: Natural edge origins use own padding; sticky edges track the parent's padded content viewport.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowHeaderFooterRegressionTest, WFHF_PaddingUsesParentContentViewport, TestSize.Level1)
{
    CreateWaterFlow();
    PaddingProperty ownPadding;
    ownPadding.top = CalcLength(10.0f);
    ownPadding.bottom = CalcLength(15.0f);
    ownPadding.left = CalcLength(12.0f);
    ownPadding.right = CalcLength(24.0f);
    ViewAbstract::SetPadding(ownPadding);
    CreateWaterFlowItems(20);
    SetEdges();
    CreateDone();
    EXPECT_EQ(pattern_->GetEdgeState().origin, OffsetF(12.0f, 10.0f));
    EXPECT_FLOAT_EQ(Header()->GetGeometryNode()->GetMarginFrameOffset().GetY(), 10.0f);
    auto parent = ParentScroll(300.0f);
    PaddingProperty parentPadding;
    parentPadding.top = CalcLength(20.0f);
    parentPadding.bottom = CalcLength(30.0f);
    parent->GetLayoutProperty()->UpdatePadding(parentPadding);
    MoveHost(-100.0f);
    pattern_->SetSticky(StickyStyle::BOTH);
    EXPECT_EQ(Header()->GetGeometryNode()->GetMarginFrameOffset(), OffsetF(12.0f, 120.0f));
    EXPECT_FLOAT_EQ(GetChildRect(frameNode_, pattern_->GetEdgeIndexMap().FooterIndex()).Top(), 330.0f);
    parentPadding.top = CalcLength(40.0f);
    parentPadding.bottom = CalcLength(50.0f);
    parent->GetLayoutProperty()->UpdatePadding(parentPadding);
    pattern_->UpdateEdgePositions();
    EXPECT_EQ(Header()->GetGeometryNode()->GetMarginFrameOffset(), OffsetF(12.0f, 140.0f));
    EXPECT_FLOAT_EQ(GetChildRect(frameNode_, pattern_->GetEdgeIndexMap().FooterIndex()).Top(), 310.0f);
    pattern_->SetSticky(StickyStyle::NONE);
    EXPECT_EQ(Header()->GetGeometryNode()->GetMarginFrameOffset(), OffsetF(12.0f, 10.0f));
}

/**
 * @tc.name: WFHF_MixedLazyHandoffRefreshesBothOrders
 * @tc.desc: An opted-in WaterFlow refreshes its Lazy neighbor in either order, including sticky removal.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowHeaderFooterRegressionTest, WFHF_MixedLazyHandoffRefreshesBothOrders, TestSize.Level1)
{
    CreateWaterFlow();
    CreateWaterFlowItems(2);
    SetEdges();
    CreateDone();
    auto info = AceType::MakeRefPtr<LazyColumnLayoutInfo>();
    info->posMap_ = { { 0, { 0.0f, 500.0f } }, { 1, { 512.0f, 1012.0f } } };
    info->startIndex_ = 0;
    info->endIndex_ = 1;
    LazyColumnLayoutAlgorithm algorithm(info);
    algorithm.totalItemCount_ = 2;
    algorithm.space_ = 12.0f;
    for (const bool waterFlowFirst : { true, false }) {
        auto parent = Edge(1000.0f);
        auto lazyPattern = AceType::MakeRefPtr<HandoffCountingLazyPattern>();
        auto lazy = FrameNode::CreateFrameNode(V2::LAZY_COLUMN_LAYOUT_ETS_TAG,
            ElementRegister::GetInstance()->MakeUniqueId(), lazyPattern);
        parent->AddChild(waterFlowFirst ? frameNode_ : lazy);
        parent->AddChild(waterFlowFirst ? lazy : frameNode_);
        pattern_->SetSticky(StickyStyle::HEADER);
        algorithm.LayoutItems(AceType::RawPtr(parent), WATER_FLOW_WIDTH, OffsetF());
        EXPECT_EQ(lazyPattern->publications_, 1);
        EXPECT_FALSE(layoutProperty_->GetNeedLazyLayout());
        if (waterFlowFirst) {
            EXPECT_EQ(pattern_->GetNextStickyHeaderGap(), std::optional<float>(12.0f));
        }
        lazyPattern->publications_ = 0;
        pattern_->SetSticky(StickyStyle::NONE);
        algorithm.LayoutItems(AceType::RawPtr(parent), WATER_FLOW_WIDTH, OffsetF());
        EXPECT_EQ(lazyPattern->publications_, 1);
        EXPECT_FALSE(pattern_->GetNextStickyHeaderGap().has_value());
        parent->RemoveChild(frameNode_);
    }
}

/**
 * @tc.name: WFHF_EmptyTallEdgesScroll
 * @tc.desc: Empty data does not prevent scrolling when header and footer exceed the viewport.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowHeaderFooterRegressionTest, WFHF_EmptyTallEdgesScroll, TestSize.Level1)
{
    CreateWaterFlow();
    SetEdges(600.0f, 600.0f);
    CreateDone();
    EXPECT_FALSE(pattern_->IsAtBottom());
    EXPECT_TRUE(pattern_->IsScrollable());
    pattern_->ScrollBy(200.0f);
    FlushUITasks();
    EXPECT_FLOAT_EQ(Header()->GetGeometryNode()->GetMarginFrameOffset().GetY(), -200.0f);
    EXPECT_FLOAT_EQ(GetChildRect(frameNode_, pattern_->GetEdgeIndexMap().FooterIndex()).Bottom(), 1000.0f);
    frameNode_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
    FlushUITasks();
    EXPECT_FLOAT_EQ(Header()->GetGeometryNode()->GetMarginFrameOffset().GetY(), -200.0f);
    pattern_->ScrollBy(10000.0f);
    FlushUITasks();
    EXPECT_TRUE(pattern_->IsAtBottom());
    EXPECT_FLOAT_EQ(GetChildRect(frameNode_, pattern_->GetEdgeIndexMap().FooterIndex()).Bottom(), WATER_FLOW_HEIGHT);
    pattern_->ScrollBy(-10000.0f);
    FlushUITasks();
    EXPECT_TRUE(pattern_->IsAtTop());
    EXPECT_FLOAT_EQ(Header()->GetGeometryNode()->GetMarginFrameOffset().GetY(), 0.0f);
}

/**
 * @tc.name: WFHF_EmptySectionsTallEdgesScroll
 * @tc.desc: Explicit empty sections preserve the edge-only scroll range across remeasurement.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowHeaderFooterRegressionTest, WFHF_EmptySectionsTallEdgesScroll, TestSize.Level1)
{
    CreateWaterFlow();
    SetEdges(600.0f, 600.0f);
    ASSERT_TRUE(pattern_->GetOrCreateWaterFlowSections()->GetSectionInfo().empty());
    CreateDone();
    EXPECT_FALSE(pattern_->IsAtBottom());
    EXPECT_TRUE(pattern_->IsScrollable());
    pattern_->ScrollBy(200.0f);
    FlushUITasks();
    EXPECT_FLOAT_EQ(Header()->GetGeometryNode()->GetMarginFrameOffset().GetY(), -200.0f);
    EXPECT_FLOAT_EQ(GetChildRect(frameNode_, pattern_->GetEdgeIndexMap().FooterIndex()).Bottom(), 1000.0f);
    frameNode_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
    FlushUITasks();
    EXPECT_FLOAT_EQ(Header()->GetGeometryNode()->GetMarginFrameOffset().GetY(), -200.0f);
    pattern_->ScrollBy(10000.0f);
    FlushUITasks();
    EXPECT_TRUE(pattern_->IsAtBottom());
    EXPECT_FLOAT_EQ(GetChildRect(frameNode_, pattern_->GetEdgeIndexMap().FooterIndex()).Bottom(), WATER_FLOW_HEIGHT);
    pattern_->ScrollBy(-10000.0f);
    FlushUITasks();
    EXPECT_TRUE(pattern_->IsAtTop());
    EXPECT_FLOAT_EQ(Header()->GetGeometryNode()->GetMarginFrameOffset().GetY(), 0.0f);
}

/**
 * @tc.name: WFHF_EmptyRowWrapKeepsCrossSize
 * @tc.desc: Hidden edges in Row layout must not overwrite the fixed cross-axis size.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowHeaderFooterRegressionTest, WFHF_EmptyRowWrapKeepsCrossSize, TestSize.Level1)
{
    CreateWaterFlow();
    SetEdges();
    CreateDone();
    WaterFlowModelNG::SetLayoutDirection(AceType::RawPtr(frameNode_), FlexDirection::ROW);
    layoutProperty_->ClearUserDefinedIdealSize(true, false);
    ViewAbstract::UpdateLayoutPolicyProperty(AceType::RawPtr(frameNode_), LayoutCalPolicy::FIX_AT_IDEAL_SIZE, true);
    frameNode_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    FlushUITasks();
    EXPECT_FLOAT_EQ(frameNode_->GetGeometryNode()->GetFrameSize().Height(), WATER_FLOW_HEIGHT);
    EXPECT_TRUE(std::isfinite(frameNode_->GetGeometryNode()->GetFrameSize().Width()));
}

/**
 * @tc.name: WFHF_LegacyZeroTailEndState
 * @tc.desc: The edge data-count correction must preserve legacy footer end-state behavior.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowHeaderFooterRegressionTest, WFHF_LegacyZeroTailEndState, TestSize.Level1)
{
    auto info = AceType::MakeRefPtr<WaterFlowLayoutInfo>();
    info->footerIndex_ = 0;
    info->childrenCount_ = 4;
    info->items_.resize(1);
    info->items_[0][0] = { { 0, { 0.0f, 400.0f } }, { 1, { 400.0f, 400.0f } }, { 2, { 800.0f, 0.0f } } };
    WaterFlowLayoutAlgorithm algorithm(info);
    algorithm.HandleItemEnd(2);
    EXPECT_FALSE(info->itemEnd_);
    info->edgeSlots_ = true;
    info->edgeIndexMap_ = { false, true, 4 };
    info->footerIndex_ = 3;
    algorithm.HandleItemEnd(2);
    EXPECT_TRUE(info->itemEnd_);
}
} // namespace OHOS::Ace::NG

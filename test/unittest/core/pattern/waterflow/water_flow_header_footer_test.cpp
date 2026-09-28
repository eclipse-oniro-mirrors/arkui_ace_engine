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

#include "core/components_ng/pattern/lazy_layout/header_footer_utils.h"
#include "core/components_ng/pattern/lazy_layout/lazy_layout_utils.h"

#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"
#include "core/components_ng/pattern/scroll/scroll_pattern.h"
#include "core/components_ng/pattern/scrollable/scroll_viewport_coordinator.h"

namespace OHOS::Ace::NG {
class WaterFlowHeaderFooterTest : public WaterFlowTestNg {
protected:
    RefPtr<FrameNode> Edge(float height)
    {
        auto node = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG,
            ElementRegister::GetInstance()->MakeUniqueId(), MakeRefPtr<LinearLayoutPattern>(true));
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
            ElementRegister::GetInstance()->MakeUniqueId(), MakeRefPtr<ScrollPattern>());
        parent->GetPattern<ScrollPattern>()->SetAxis(Axis::VERTICAL);
        parent->GetGeometryNode()->SetFrameSize(SizeF(WATER_FLOW_WIDTH, height));
        parent->AddChild(frameNode_);
        return parent;
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
    EXPECT_EQ(map.DataToNode(0), 2);
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
    EXPECT_FLOAT_EQ(GetChildRect(frameNode_, 1).Top(), 0.0f);
    EXPECT_FLOAT_EQ(GetChildRect(frameNode_, 0).Top(), 50.0f);
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
    pattern_->SetEdge(false, nullptr, pattern_->BeginEdgeUpdate(false));
    pattern_->AddFooter(legacy);
    CreateDone();
    EXPECT_FALSE(pattern_->hasFooter());
    EXPECT_EQ(pattern_->GetChildrenCount(), 2);
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
    EXPECT_FLOAT_EQ(GetChildRect(frameNode_, 2).Top(), 50.0f);
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
    frameNode_->GetGeometryNode()->SetFrameOffset(OffsetF(0.0f, -200.0f));
    pattern_->SetEdgeGeometry(OffsetF(), 1000.0f);
    pattern_->SetSticky(StickyStyle::BOTH);
    auto pipeline = frameNode_->GetContext();
    ASSERT_NE(pipeline, nullptr);
    pipeline->GetScrollViewportCoordinator()->Flush(pipeline);
    EXPECT_FLOAT_EQ(Header()->GetGeometryNode()->GetMarginFrameOffset().GetY(), 200.0f);
    EXPECT_FLOAT_EQ(GetChildRect(frameNode_, 0).Top(), 460.0f);
    frameNode_->GetGeometryNode()->SetFrameOffset(OffsetF(0.0f, -250.0f));
    pipeline->GetScrollViewportCoordinator()->Flush(pipeline);
    EXPECT_FLOAT_EQ(Header()->GetGeometryNode()->GetMarginFrameOffset().GetY(), 250.0f);
    EXPECT_FLOAT_EQ(GetChildRect(frameNode_, 0).Top(), 510.0f);
    pattern_->SetSticky(StickyStyle::NONE);
    EXPECT_FLOAT_EQ(Header()->GetGeometryNode()->GetMarginFrameOffset().GetY(), 0.0f);
    EXPECT_FLOAT_EQ(GetChildRect(frameNode_, 0).Top(), 960.0f);
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
}

/**
 * @tc.name: WFHF_RemoveSharedLazyRequest
 * @tc.desc: One child cannot clear another lazy request.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowHeaderFooterTest, WFHF_RemoveSharedLazyRequest, TestSize.Level1)
{
    auto property = MakeRefPtr<LayoutProperty>();
    property->SetLazyLayoutRequest(1, true);
    property->SetLazyLayoutRequest(2, true);
    property->SetLazyLayoutRequest(1, false);
    EXPECT_TRUE(property->GetNeedLazyLayout());
    property->SetLazyLayoutRequest(2, false);
    EXPECT_FALSE(property->GetNeedLazyLayout());
    property->SetNeedLazyLayout(true);
    property->SetLazyLayoutRequest(1, true);
    property->SetLazyLayoutRequest(1, false);
    EXPECT_TRUE(property->GetNeedLazyLayout());
}

/**
 * @tc.name: WFHF_ParentAdjustConsumedOnce
 * @tc.desc: Nested parents cannot compensate twice.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowHeaderFooterTest, WFHF_ParentAdjustConsumedOnce, TestSize.Level1)
{
    CreateWaterFlow();
    CreateWaterFlowItems(2);
    CreateDone();
    layoutProperty_->SetLazyLayoutRequest(frameNode_->GetId(), true);
    pattern_->layoutInfo_->parentDriven_ = true;
    pattern_->layoutInfo_->parentAdjustStart_ = 15.0f;
    pattern_->layoutInfo_->parentAdjustEnd_ = -3.0f;
    const auto first = LazyLayoutUtils::ConsumeNestedLazyAdjustOffset(frameNode_);
    const auto second = LazyLayoutUtils::ConsumeNestedLazyAdjustOffset(frameNode_);
    EXPECT_FLOAT_EQ(first.start, 15.0f);
    EXPECT_FLOAT_EQ(first.end, -3.0f);
    EXPECT_FLOAT_EQ(second.start, 0.0f);
    EXPECT_FLOAT_EQ(second.end, 0.0f);
}

/**
 * @tc.name: WFHF_PredictHalfViewport
 * @tc.desc: Parent prediction extends each edge by half a viewport.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowHeaderFooterTest, WFHF_PredictHalfViewport, TestSize.Level1)
{
    ViewPosReference reference { .viewPosStart = 0.0f, .viewPosEnd = 400.0f,
        .referencePos = -100.0f, .referenceEdge = ReferenceEdge::START, .axis = Axis::VERTICAL };
    auto range = LazyLayoutUtils::CalculateViewRange(reference, 1000.0f);
    EXPECT_FLOAT_EQ(range.start, 100.0f);
    EXPECT_FLOAT_EQ(range.end, 500.0f);
    reference.deadline = 12345;
    range = LazyLayoutUtils::CalculateViewRange(reference, 1000.0f);
    EXPECT_FLOAT_EQ(range.start, -100.0f);
    EXPECT_FLOAT_EQ(range.end, 700.0f);
}
} // namespace OHOS::Ace::NG

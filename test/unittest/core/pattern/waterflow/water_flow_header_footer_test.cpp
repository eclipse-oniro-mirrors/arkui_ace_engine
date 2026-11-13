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

#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"

namespace OHOS::Ace::NG {
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

} // namespace OHOS::Ace::NG

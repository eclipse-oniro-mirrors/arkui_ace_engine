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

#include "gtest/gtest.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"

#include "base/geometry/calc_dimension_rect.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/inspector_tree_request_context.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/pattern/text/text_model_ng.h"
#include "core/components_ng/pattern/text/text_pattern.h"

namespace OHOS::Ace::NG {
class InspectorTreeTask04 : public testing::Test {
public:
    void SetUp() override
    {
        MockPipelineContext::SetUp();
    }
    void TearDown() override
    {
        MockPipelineContext::TearDown();
    }
};

TEST_F(InspectorTreeTask04, ToolKeysAreConfiguredOnlyAndFeatureIsIndependent)
{
    auto frame = FrameNode::CreateFrameNode("test", 20, AceType::MakeRefPtr<Pattern>());
    auto gesture = frame->GetOrCreateGestureEventHub();
    CalcDimensionRect region(CalcDimension(40, DimensionUnit::PX), CalcDimension(20, DimensionUnit::PX),
        CalcDimension(10, DimensionUnit::PX), CalcDimension(15, DimensionUnit::PX));
    gesture->SetResponseRegionMap({ { ResponseRegionSupportedTool::PEN, { region } } });
    ParamConfig config;
    config.withResponseRegion = true;
    InspectorTreeRequestContext context(config, {}, {});
    config.inspectorContext = &context;
    auto json = JsonUtil::CreateSharedPtrJson();
    frame->DumpSimplifyTreeNode(json, config);
    auto map = json->GetObject("responseRegionMap");
    ASSERT_TRUE(map->IsObject());
    ASSERT_TRUE(map->GetValue("pen")->IsArray());
    EXPECT_EQ(map->GetValue("pen")->GetArraySize(), 1);
    EXPECT_TRUE(map->GetValue("pen")->GetArrayItem(0)->IsString());
    EXPECT_FALSE(map->Contains("finger"));
    EXPECT_FALSE(map->Contains("all"));
    EXPECT_FALSE(map->Contains("mouse"));
    EXPECT_FALSE(json->Contains("responseRegion"));
    EXPECT_FALSE(json->Contains("mouseResponseRegion"));
    config.withResponseRegion = false;
    auto disabled = JsonUtil::CreateSharedPtrJson();
    frame->DumpSimplifyTreeNode(disabled, config);
    EXPECT_FALSE(disabled->Contains("responseRegionMap"));
}

TEST_F(InspectorTreeTask04, TextExportsOnlyExplicitResponseRegion)
{
    auto frame = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 21, AceType::MakeRefPtr<TextPattern>());
    auto gesture = frame->GetOrCreateGestureEventHub();
    frame->GetGeometryNode()->SetFrameSize(SizeF(100.0f, 20.0f));
    frame->GetRenderContext()->UpdateClipEdge(true);
    frame->GetPattern<TextPattern>()->CreateNodePaintMethod();
    const auto regions = gesture->GetResponseRegion();
    ASSERT_FALSE(regions.empty());
    gesture->SetMouseResponseRegion(regions);

    ParamConfig config;
    config.withResponseRegion = true;
    InspectorTreeRequestContext context(config, {}, {});
    config.inspectorContext = &context;
    auto implicit = JsonUtil::CreateSharedPtrJson();
    frame->DumpSimplifyTreeNode(implicit, config);
    EXPECT_FALSE(implicit->Contains("responseRegion"));
    EXPECT_TRUE(implicit->GetValue("mouseResponseRegion")->IsArray());
    ASSERT_EQ(gesture->GetResponseRegion().size(), regions.size());
    EXPECT_EQ(gesture->GetResponseRegion().front().ToString(), regions.front().ToString());

    TextModelNG::SetResponseRegion(AceType::RawPtr(frame), regions);
    auto explicitRegion = JsonUtil::CreateSharedPtrJson();
    frame->DumpSimplifyTreeNode(explicitRegion, config);
    ASSERT_TRUE(explicitRegion->GetValue("responseRegion")->IsArray());
    EXPECT_EQ(explicitRegion->GetValue("responseRegion")->GetArraySize(), regions.size());

    TextModelNG::ClearResponseRegion(AceType::RawPtr(frame));
    auto reset = JsonUtil::CreateSharedPtrJson();
    frame->DumpSimplifyTreeNode(reset, config);
    EXPECT_FALSE(reset->Contains("responseRegion"));
    EXPECT_FALSE(gesture->GetResponseRegion().empty());

    auto other = FrameNode::CreateFrameNode("test", 22, AceType::MakeRefPtr<Pattern>());
    other->GetOrCreateGestureEventHub()->SetResponseRegion(regions);
    auto otherRegion = JsonUtil::CreateSharedPtrJson();
    other->DumpSimplifyTreeNode(otherRegion, config);
    EXPECT_TRUE(otherRegion->GetValue("responseRegion")->IsArray());
}
} // namespace OHOS::Ace::NG

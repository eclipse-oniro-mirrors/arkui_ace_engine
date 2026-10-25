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

#include <tuple>

#include "gtest/gtest.h"
#include "test/mock/frameworks/core/components_ng/render/mock_render_context.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"

#include "base/geometry/calc_dimension_rect.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/inspector_tree_request_context.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/pattern/text/text_model_ng.h"
#include "core/components_ng/pattern/text/text_pattern.h"

namespace OHOS::Ace::NG {
namespace {
constexpr int32_t EXTENDED_REGION_WIDTH = 140;
constexpr int32_t EXTENDED_REGION_HEIGHT = 120;
constexpr int32_t EXTENDED_REGION_OFFSET_X = -20;
constexpr int32_t EXTENDED_REGION_OFFSET_Y = -10;
} // namespace
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

    RefPtr<FrameNode> CreateFrame(int32_t id, const RectF& rect)
    {
        auto frame = FrameNode::CreateFrameNode("test", id, AceType::MakeRefPtr<Pattern>());
        frame->SetActive(true);
        SetPaintRect(frame, rect);
        return frame;
    }

    void SetPaintRect(const RefPtr<FrameNode>& frame, const RectF& rect)
    {
        auto render = AceType::DynamicCast<MockRenderContext>(frame->GetRenderContext());
        ASSERT_NE(render, nullptr);
        render->SetPaintRectWithTransform(rect);
    }

    std::shared_ptr<JsonValue> DumpRegions(const RefPtr<FrameNode>& frame, bool visible)
    {
        ParamConfig config;
        config.withResponseRegion = true;
        InspectorTreeRequestContext context(config, {});
        config.inspectorContext = &context;
        auto json = JsonUtil::CreateSharedPtrJson();
        frame->DumpSimplifyTreeWithParamConfig(0, json, visible, config);
        return json;
    }
};

TEST_F(InspectorTreeTask04, ToolKeysAreConfiguredOnlyAndFeatureIsIndependent)
{
    auto frame = FrameNode::CreateFrameNode("test", 20, AceType::MakeRefPtr<Pattern>());
    auto gesture = frame->GetOrCreateGestureEventHub();
    constexpr int32_t REGION_WIDTH = 40;
    constexpr int32_t REGION_HEIGHT = 20;
    constexpr int32_t REGION_OFFSET_X = 10;
    constexpr int32_t REGION_OFFSET_Y = 15;
    CalcDimensionRect region(CalcDimension(REGION_WIDTH, DimensionUnit::PX),
        CalcDimension(REGION_HEIGHT, DimensionUnit::PX), CalcDimension(REGION_OFFSET_X, DimensionUnit::PX),
        CalcDimension(REGION_OFFSET_Y, DimensionUnit::PX));
    gesture->SetResponseRegionMap({ { ResponseRegionSupportedTool::PEN, { region } } });
    ParamConfig config;
    config.withResponseRegion = true;
    InspectorTreeRequestContext context(config, {});
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
    InspectorTreeRequestContext context(config, {});
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

TEST_F(InspectorTreeTask04, SelfClipPreservesExplicitRegionsOutsidePaintBounds)
{
    auto frame = CreateFrame(23, RectF(40.0f, 50.0f, 100.0f, 100.0f));
    auto gesture = frame->GetOrCreateGestureEventHub();
    DimensionRect region(Dimension(EXTENDED_REGION_WIDTH, DimensionUnit::PX),
        Dimension(EXTENDED_REGION_HEIGHT, DimensionUnit::PX),
        DimensionOffset(Dimension(EXTENDED_REGION_OFFSET_X, DimensionUnit::PX),
            Dimension(EXTENDED_REGION_OFFSET_Y, DimensionUnit::PX)));
    gesture->SetResponseRegion({ region });
    gesture->SetMouseResponseRegion({ region });
    for (bool clip : { false, true }) {
        frame->GetRenderContext()->UpdateClipEdge(clip);
        for (bool visible : { false, true }) {
            auto json = DumpRegions(frame, visible);
            for (const auto* key : { "responseRegion", "mouseResponseRegion" }) {
                auto regions = json->GetValue(key);
                ASSERT_EQ(regions->GetArraySize(), 1);
                EXPECT_EQ(regions->GetArrayItem(0)->GetString(), "[20.00, 40.00],[160.00,160.00]");
            }
        }
        for (auto source : { SourceType::TOUCH, SourceType::MOUSE }) {
            TouchEvent event;
            event.sourceType = source;
            EXPECT_FALSE(frame->IsOutOfTouchTestRegion(PointF(25.0f, 45.0f), event));
            EXPECT_TRUE(frame->IsOutOfTouchTestRegion(PointF(15.0f, 35.0f), event));
        }
    }
    const std::tuple<ResponseRegionSupportedTool, SourceTool, const char*> tools[] = {
        { ResponseRegionSupportedTool::ALL, SourceTool::UNKNOWN, "all" },
        { ResponseRegionSupportedTool::FINGER, SourceTool::FINGER, "finger" },
        { ResponseRegionSupportedTool::PEN, SourceTool::PEN, "pen" },
        { ResponseRegionSupportedTool::MOUSE, SourceTool::MOUSE, "mouse" }
    };
    for (const auto& [tool, source, key] : tools) {
        gesture->SetResponseRegionMap({ { tool, { region } } });
        TouchEvent event;
        event.sourceType = source == SourceTool::MOUSE ? SourceType::MOUSE : SourceType::TOUCH;
        event.sourceTool = source;
        EXPECT_FALSE(frame->IsOutOfTouchTestRegion(PointF(25.0f, 45.0f), event));
        for (bool visible : { false, true }) {
            auto json = DumpRegions(frame, visible);
            auto regions = json->GetObject("responseRegionMap")->GetValue(key);
            ASSERT_EQ(regions->GetArraySize(), 1);
            EXPECT_EQ(regions->GetArrayItem(0)->GetString(), "[20.00, 40.00],[160.00,160.00]");
        }
    }
}

TEST_F(InspectorTreeTask04, AncestorClipUsesToolResponseRegionsAndPreservesGaps)
{
    auto parent = CreateFrame(24, RectF(0.0f, 0.0f, 100.0f, 100.0f));
    auto frame = CreateFrame(25, RectF(0.0f, 0.0f, 100.0f, 100.0f));
    frame->MountToParent(parent);
    parent->GetRenderContext()->UpdateClipEdge(true);
    frame->GetRenderContext()->UpdateClipEdge(true);
    DimensionRect region(Dimension(EXTENDED_REGION_WIDTH, DimensionUnit::PX),
        Dimension(EXTENDED_REGION_HEIGHT, DimensionUnit::PX),
        DimensionOffset(Dimension(EXTENDED_REGION_OFFSET_X, DimensionUnit::PX),
            Dimension(EXTENDED_REGION_OFFSET_Y, DimensionUnit::PX)));
    constexpr int32_t LEFT_REGION_WIDTH = 40;
    constexpr int32_t LEFT_REGION_OFFSET_X = -30;
    constexpr int32_t RIGHT_REGION_WIDTH = 50;
    constexpr int32_t RIGHT_REGION_OFFSET_X = 80;
    constexpr int32_t CLIPPED_REGION_COUNT = 2;
    DimensionRect left(Dimension(LEFT_REGION_WIDTH, DimensionUnit::PX),
        Dimension(EXTENDED_REGION_HEIGHT, DimensionUnit::PX),
        DimensionOffset(Dimension(LEFT_REGION_OFFSET_X, DimensionUnit::PX),
            Dimension(EXTENDED_REGION_OFFSET_Y, DimensionUnit::PX)));
    DimensionRect right(Dimension(RIGHT_REGION_WIDTH, DimensionUnit::PX),
        Dimension(EXTENDED_REGION_HEIGHT, DimensionUnit::PX),
        DimensionOffset(Dimension(RIGHT_REGION_OFFSET_X, DimensionUnit::PX),
            Dimension(EXTENDED_REGION_OFFSET_Y, DimensionUnit::PX)));
    auto gesture = frame->GetOrCreateGestureEventHub();
    auto parentGesture = parent->GetOrCreateGestureEventHub();
    gesture->SetResponseRegion({ region });
    gesture->SetMouseResponseRegion({ region });
    parentGesture->SetResponseRegion({ left, right });
    parentGesture->SetMouseResponseRegion({ right });
    TouchEvent event;
    event.sourceType = SourceType::TOUCH;
    EXPECT_FALSE(parent->IsOutOfTouchTestRegion(PointF(-15.0f, 0.0f), event));
    EXPECT_FALSE(parent->IsOutOfTouchTestRegion(PointF(115.0f, 0.0f), event));
    EXPECT_TRUE(parent->IsOutOfTouchTestRegion(PointF(50.0f, 50.0f), event));
    EXPECT_FALSE(frame->IsOutOfTouchTestRegion(PointF(-15.0f, 0.0f), event));
    for (bool visible : { false, true }) {
        auto json = DumpRegions(frame, visible);
        auto touch = json->GetValue("responseRegion");
        ASSERT_EQ(touch->GetArraySize(), CLIPPED_REGION_COUNT);
        EXPECT_EQ(touch->GetArrayItem(0)->GetString(), "[-20.00, -10.00],[10.00,110.00]");
        EXPECT_EQ(touch->GetArrayItem(1)->GetString(), "[80.00, -10.00],[120.00,110.00]");
        auto mouse = json->GetValue("mouseResponseRegion");
        ASSERT_EQ(mouse->GetArraySize(), 1);
        EXPECT_EQ(mouse->GetArrayItem(0)->GetString(), "[80.00, -10.00],[120.00,110.00]");
    }
    gesture->SetResponseRegionMap({ { ResponseRegionSupportedTool::MOUSE, { region } } });
    event.sourceType = SourceType::MOUSE;
    event.sourceTool = SourceTool::MOUSE;
    EXPECT_TRUE(parent->IsOutOfTouchTestRegion(PointF(-15.0f, 0.0f), event));
    EXPECT_FALSE(parent->IsOutOfTouchTestRegion(PointF(115.0f, 0.0f), event));
    for (bool visible : { false, true }) {
        auto json = DumpRegions(frame, visible);
        auto mouse = json->GetObject("responseRegionMap")->GetValue("mouse");
        ASSERT_EQ(mouse->GetArraySize(), 1);
        EXPECT_EQ(mouse->GetArrayItem(0)->GetString(), "[80.00, -10.00],[120.00,110.00]");
    }
    parentGesture->SetResponseRegionMap(
        { { ResponseRegionSupportedTool::ALL, { left } }, { ResponseRegionSupportedTool::MOUSE, { right } } });
    EXPECT_FALSE(parent->IsOutOfTouchTestRegion(PointF(-15.0f, 0.0f), event));
    auto json = DumpRegions(frame, false);
    auto mouse = json->GetObject("responseRegionMap")->GetValue("mouse");
    ASSERT_EQ(mouse->GetArraySize(), CLIPPED_REGION_COUNT);
    EXPECT_EQ(mouse->GetArrayItem(0)->GetString(), "[-20.00, -10.00],[10.00,110.00]");
    EXPECT_EQ(mouse->GetArrayItem(1)->GetString(), "[80.00, -10.00],[120.00,110.00]");
}

TEST_F(InspectorTreeTask04, OverlappingAncestorRegionsDoNotMultiply)
{
    auto frame = CreateFrame(100, RectF(0.0f, 0.0f, 10.0f, 10.0f));
    auto root = frame;
    constexpr int32_t REGION_SIDE = 10;
    constexpr int32_t ANCESTOR_REGION_LONG_SIDE = 20;
    constexpr int32_t ANCESTOR_REGION_SHORT_SIDE = 12;
    constexpr int32_t ANCESTOR_REGION_LONG_OFFSET = -5;
    constexpr int32_t ANCESTOR_REGION_SHORT_OFFSET = -1;
    constexpr int32_t ANCESTOR_DEPTH = 8;
    constexpr size_t ANCESTOR_REGION_COUNT = 2;
    DimensionRect region(Dimension(REGION_SIDE, DimensionUnit::PX),
        Dimension(REGION_SIDE, DimensionUnit::PX), DimensionOffset());
    DimensionRect horizontal(Dimension(ANCESTOR_REGION_LONG_SIDE, DimensionUnit::PX),
        Dimension(ANCESTOR_REGION_SHORT_SIDE, DimensionUnit::PX),
        DimensionOffset(Dimension(ANCESTOR_REGION_LONG_OFFSET, DimensionUnit::PX),
            Dimension(ANCESTOR_REGION_SHORT_OFFSET, DimensionUnit::PX)));
    DimensionRect vertical(Dimension(ANCESTOR_REGION_SHORT_SIDE, DimensionUnit::PX),
        Dimension(ANCESTOR_REGION_LONG_SIDE, DimensionUnit::PX),
        DimensionOffset(Dimension(ANCESTOR_REGION_SHORT_OFFSET, DimensionUnit::PX),
            Dimension(ANCESTOR_REGION_LONG_OFFSET, DimensionUnit::PX)));
    std::vector<RefPtr<FrameNode>> ancestors;
    for (int32_t depth = 0; depth < ANCESTOR_DEPTH; ++depth) {
        auto parent = CreateFrame(101 + depth, RectF(0.0f, 0.0f, 100.0f, 100.0f));
        parent->GetRenderContext()->UpdateClipEdge(true);
        auto gesture = parent->GetOrCreateGestureEventHub();
        gesture->SetResponseRegion({ horizontal, vertical });
        gesture->SetMouseResponseRegion({ horizontal, vertical });
        root->MountToParent(parent);
        ancestors.emplace_back(parent);
        root = parent;
    }
    auto gesture = frame->GetOrCreateGestureEventHub();
    gesture->SetResponseRegion({ region });
    gesture->SetMouseResponseRegion({ region });
    for (bool visible : { false, true }) {
        auto json = DumpRegions(frame, visible);
        for (const auto* key : { "responseRegion", "mouseResponseRegion" }) {
            auto regions = json->GetValue(key);
            ASSERT_EQ(regions->GetArraySize(), 1);
            EXPECT_EQ(regions->GetArrayItem(0)->GetString(), "[0.00, 0.00],[10.00,10.00]");
        }
    }
    gesture->SetResponseRegionMap(
        { { ResponseRegionSupportedTool::ALL, { region } }, { ResponseRegionSupportedTool::FINGER, { region } },
            { ResponseRegionSupportedTool::PEN, { region } }, { ResponseRegionSupportedTool::MOUSE, { region } } });
    for (bool visible : { false, true }) {
        auto json = DumpRegions(frame, visible);
        auto map = json->GetObject("responseRegionMap");
        for (const auto* key : { "all", "finger", "pen", "mouse" }) {
            auto regions = map->GetValue(key);
            ASSERT_EQ(regions->GetArraySize(), 1);
            EXPECT_EQ(regions->GetArrayItem(0)->GetString(), "[0.00, 0.00],[10.00,10.00]");
        }
    }
    for (const auto& ancestor : ancestors) {
        auto parentGesture = ancestor->GetOrCreateGestureEventHub();
        EXPECT_EQ(parentGesture->GetResponseRegion().size(), ANCESTOR_REGION_COUNT);
        EXPECT_EQ(parentGesture->GetMouseResponseRegion().size(), ANCESTOR_REGION_COUNT);
    }
}

TEST_F(InspectorTreeTask04, AncestorClipHonorsDefaultRegionsAndEmptyIntersections)
{
    auto parent = CreateFrame(26, RectF(0.0f, 0.0f, 100.0f, 100.0f));
    auto frame = CreateFrame(27, RectF(0.0f, 0.0f, 100.0f, 100.0f));
    frame->MountToParent(parent);
    DimensionRect region(Dimension(EXTENDED_REGION_WIDTH, DimensionUnit::PX),
        Dimension(EXTENDED_REGION_HEIGHT, DimensionUnit::PX),
        DimensionOffset(Dimension(EXTENDED_REGION_OFFSET_X, DimensionUnit::PX),
            Dimension(EXTENDED_REGION_OFFSET_Y, DimensionUnit::PX)));
    frame->GetOrCreateGestureEventHub()->SetResponseRegion({ region });
    for (bool clip : { false, true }) {
        parent->GetRenderContext()->UpdateClipEdge(clip);
        for (bool visible : { false, true }) {
            auto json = DumpRegions(frame, visible);
            auto regions = json->GetValue("responseRegion");
            ASSERT_EQ(regions->GetArraySize(), 1);
            EXPECT_EQ(regions->GetArrayItem(0)->GetString(),
                clip ? "[0.00, 0.00],[100.00,100.00]" : "[-20.00, -10.00],[120.00,110.00]");
        }
    }
    SetPaintRect(frame, RectF(200.0f, 200.0f, 100.0f, 100.0f));
    for (bool visible : { false, true }) {
        auto json = DumpRegions(frame, visible);
        auto regions = json->GetValue("responseRegion");
        ASSERT_TRUE(regions->IsArray());
        EXPECT_EQ(regions->GetArraySize(), 0);
    }
}
} // namespace OHOS::Ace::NG

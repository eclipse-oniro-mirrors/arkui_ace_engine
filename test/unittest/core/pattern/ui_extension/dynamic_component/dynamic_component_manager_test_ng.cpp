/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "base/memory/ace_type.h"
#include "wm/wm_common.h"
#include "wm/occupied_area_change_info.h"
#include "interfaces/inner_api/ace/viewport_config.h"
#define private public
#define protected public
#include "adapter/ohos/entrance/ace_container.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"
#include "test/mock/frameworks/base/thread/mock_task_executor.h"
#include "test/mock/frameworks/core/common/mock_container.h"
#include "frameworks/core/components_ng/pattern/ui_extension/dynamic_component/dynamic_component_manager.h"

#include "core/components_ng/event/event_hub_callback_sets.h"
#include "core/components_ng/pattern/stage/stage_manager.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
#ifdef WINDOW_SCENE_SUPPORTED
OHOS::Rosen::Rect IntersectRect(const OHOS::Rosen::Rect& rect, const OHOS::Ace::ViewportConfig& config);
#endif
namespace {
    const float POSITIVE_SIZE = 600.0f;
    const float NEGATIVE_SIZE = -1.0f;
} // namespace

class DynamicPatternManagerTestNg : public testing::Test {
public:
    void SetUp() override;
    void TearDown() override;
    static void SetUpTestCase();
    static void TearDownTestCase();
};

void DynamicPatternManagerTestNg::SetUp() {}

void DynamicPatternManagerTestNg::TearDown() {}

void DynamicPatternManagerTestNg::SetUpTestCase()
{
    MockPipelineContext::SetUp();
    MockContainer::SetUp();
    MockContainer::Current()->taskExecutor_ = AceType::MakeRefPtr<MockTaskExecutor>();
    MockContainer::Current()->pipelineContext_ = MockPipelineContext::GetCurrentContext();
    MockContainer::Current()->pipelineContext_->taskExecutor_ = MockContainer::Current()->taskExecutor_;
}

void DynamicPatternManagerTestNg::TearDownTestCase()
{
    MockPipelineContext::TearDown();
    MockContainer::TearDown();
}

/**
 * @tc.name: DynamicPatternManagerTestNg001
 * @tc.desc: Test DynamicPattern TriggerOnAreaChangeCallback
 * @tc.type: FUNC
 */
HWTEST_F(DynamicPatternManagerTestNg, DynamicPatternManagerTestNg001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. test method TriggerOnAreaChangeCallback
     */
    auto context = PipelineContext::GetCurrentContext();
    EXPECT_NE(context, nullptr);
    auto stageManager = context->GetStageManager();
    EXPECT_NE(stageManager, nullptr);
    auto stageNode = stageManager->GetStageNode();
    EXPECT_NE(stageNode, nullptr);
    auto geometryNode = stageNode->GetGeometryNode();
    EXPECT_NE(geometryNode, nullptr);
    EXPECT_FALSE(geometryNode->GetMarginFrameSize().IsPositive());
    auto frameNodeRef =
        FrameNode::CreateFrameNode("main", 1, AceType::MakeRefPtr<Pattern>(), true);
    ASSERT_NE(frameNodeRef, nullptr);
    FrameNode* frameNode = &(*frameNodeRef);
    DynamicComponentManager::TriggerOnAreaChangeCallback(frameNode, 1);

    geometryNode->frame_.rect_.SetWidth(POSITIVE_SIZE);
    geometryNode->frame_.rect_.SetHeight(POSITIVE_SIZE);
    EXPECT_TRUE(geometryNode->GetMarginFrameSize().IsPositive());
    DynamicComponentManager::TriggerOnAreaChangeCallback(frameNode, 1);

    auto eventHub = frameNode->GetEventHub<EventHub>();
    ASSERT_NE(eventHub, nullptr);
    ASSERT_FALSE(eventHub->HasOnAreaChanged());
    DynamicComponentManager::TriggerOnAreaChangeCallback(frameNode, 1);

    auto func =
        [](const RectF& oldRect, const OffsetF& oldOrigin, const RectF& rect, const OffsetF& origin) {};
    eventHub->SetOnAreaChanged(std::move(func));
    ASSERT_TRUE(eventHub->HasOnAreaChanged());
    DynamicComponentManager::TriggerOnAreaChangeCallback(frameNode, 1);
}

/**
 * @tc.name: DynamicPatternManagerTestNg002
 * @tc.desc: Test DynamicPattern HandleDynamicRenderOnAreaChange
 * @tc.type: FUNC
 */
HWTEST_F(DynamicPatternManagerTestNg, DynamicPatternManagerTestNg002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. test method TriggerOnAreaChangeCallback
     */
    auto frameNodeRef =
        FrameNode::CreateFrameNode("main", 1, AceType::MakeRefPtr<Pattern>(), true);
    ASSERT_NE(frameNodeRef, nullptr);
    FrameNode* frameNode = &(*frameNodeRef);
    DynamicComponentManager::TriggerOnAreaChangeCallback(frameNode, 1);

    /**
     * @tc.steps: step2. test method HandleDynamicRenderOnAreaChange
     */
    auto eventHub = frameNode->GetEventHub<EventHub>();
    ASSERT_NE(eventHub, nullptr);
    ASSERT_FALSE(eventHub->HasInnerOnAreaChanged());
    RectF rectF(5, 5, 1, 1);
    OffsetF offsetF(2.0, 3.0);
    OffsetF offsetF2(2.0, 3.0);
    frameNode->lastFrameRect_ = std::make_unique<RectF>();
    frameNode->lastParentOffsetToWindow_ =
        std::make_unique<OffsetF>(OffsetF(50.0f, 50.0f));
    frameNode->lastHostParentOffsetToWindow_ = std::make_shared<OffsetF>();
    DynamicComponentManager::HandleDynamicRenderOnAreaChange(frameNode, rectF, offsetF, offsetF2);

    auto func = [](const RectF& oldRect, const OffsetF& oldOrigin, const RectF& rect, const OffsetF& origin) {};
    eventHub->GetOrCreateAreaCallbackSet()->onAreaChangedInnerCallbacks[0] = std::move(func);
    ASSERT_TRUE(eventHub->HasInnerOnAreaChanged());

    DynamicComponentManager::HandleDynamicRenderOnAreaChange(frameNode, rectF, offsetF, offsetF2);
    ASSERT_FALSE(eventHub->HasOnAreaChanged());
    eventHub->SetOnAreaChanged(std::move(func));
    ASSERT_TRUE(eventHub->HasOnAreaChanged());
    DynamicComponentManager::HandleDynamicRenderOnAreaChange(frameNode, rectF, offsetF, offsetF2);
}

/**
 * @tc.name: DynamicPatternManagerTestNg003
 * @tc.desc: Test DynamicPattern TriggerOnAreaChangeCallback
 * @tc.type: FUNC
 */
HWTEST_F(DynamicPatternManagerTestNg, DynamicPatternManagerTestNg003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. test method TriggerOnAreaChangeCallback
     */
    auto context = PipelineContext::GetCurrentContext();
    EXPECT_NE(context, nullptr);
    auto stageManager = context->GetStageManager();
    EXPECT_NE(stageManager, nullptr);
    auto stageNode = stageManager->GetStageNode();
    EXPECT_NE(stageNode, nullptr);
    auto geometryNode = stageNode->GetGeometryNode();
    EXPECT_NE(geometryNode, nullptr);
    EXPECT_TRUE(geometryNode->GetMarginFrameSize().IsPositive());
    auto frameNodeRef =
        FrameNode::CreateFrameNode("main", 1, AceType::MakeRefPtr<Pattern>(), true);
    ASSERT_NE(frameNodeRef, nullptr);
    FrameNode* frameNode = &(*frameNodeRef);
    geometryNode->frame_.rect_.SetWidth(NEGATIVE_SIZE);
    geometryNode->frame_.rect_.SetHeight(NEGATIVE_SIZE);
    EXPECT_FALSE(geometryNode->GetMarginFrameSize().IsPositive());
    auto eventHub = frameNode->GetEventHub<EventHub>();
    ASSERT_NE(eventHub, nullptr);
    auto func = [](const RectF& oldRect, const OffsetF& oldOrigin, const RectF& rect, const OffsetF& origin) {};
    eventHub->SetOnAreaChanged(std::move(func));
    ASSERT_TRUE(eventHub->HasOnAreaChanged());
    frameNode->lastHostParentOffsetToWindow_ = std::make_shared<OffsetF>();
    ASSERT_NE(frameNode->GetLastHostParentOffsetToWindow(), nullptr);
    auto renderContext = frameNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    ASSERT_EQ(renderContext->GetPositionProperty(), nullptr);

    auto top = CalcDimension(1.0);
    auto bottom = CalcDimension(2.0);
    auto start = CalcDimension(3.0);
    auto end = CalcDimension(4.0);

    EdgesParam edges;
    edges.SetTop(top);
    edges.SetBottom(bottom);
    edges.start = start;
    edges.end = end;
    renderContext->UpdateOffsetEdges(edges);
    ASSERT_NE(renderContext->GetPositionProperty(), nullptr);
    DynamicComponentManager::TriggerOnAreaChangeCallback(frameNode, 1);
}

/**
 * @tc.name: DynamicPatternManagerTestNg004
 * @tc.desc: Test IntersectRect with intersection returns viewport-relative coordinates
 * @tc.type: FUNC
 */
HWTEST_F(DynamicPatternManagerTestNg, DynamicPatternManagerTestNg004, TestSize.Level1)
{
#ifdef WINDOW_SCENE_SUPPORTED
    /**
     * @tc.steps: step1. viewport at (100,200) size (800,600); rect partially overlapping
     * @tc.expected: returned rect coords are relative to viewport origin
     */
    OHOS::Ace::ViewportConfig config;
    config.SetPosition(100, 200);
    config.SetSize(800, 600);

    OHOS::Rosen::Rect rect = { 50, 150, 300, 200 };
    auto result = IntersectRect(rect, config);

    EXPECT_EQ(result.posX_, 0);
    EXPECT_EQ(result.posY_, 0);
    EXPECT_EQ(result.width_, 250u);
    EXPECT_EQ(result.height_, 150u);
#endif
}

/**
 * @tc.name: DynamicPatternManagerTestNg005
 * @tc.desc: Test IntersectRect with no intersection returns zero rect
 * @tc.type: FUNC
 */
HWTEST_F(DynamicPatternManagerTestNg, DynamicPatternManagerTestNg005, TestSize.Level1)
{
#ifdef WINDOW_SCENE_SUPPORTED
    /**
     * @tc.steps: step1. rect completely outside viewport
     * @tc.expected: returned rect is {0,0,0,0}
     */
    OHOS::Ace::ViewportConfig config;
    config.SetPosition(0, 0);
    config.SetSize(1080, 2400);

    OHOS::Rosen::Rect rect = { 2000, 2000, 100, 100 };
    auto result = IntersectRect(rect, config);

    EXPECT_EQ(result.posX_, 0);
    EXPECT_EQ(result.posY_, 0);
    EXPECT_EQ(result.width_, 0u);
    EXPECT_EQ(result.height_, 0u);
#endif
}

/**
 * @tc.name: DynamicPatternManagerTestNg006
 * @tc.desc: Test IntersectRect with non-zero viewport offset returns viewport-relative coordinates
 * @tc.type: FUNC
 */
HWTEST_F(DynamicPatternManagerTestNg, DynamicPatternManagerTestNg006, TestSize.Level1)
{
#ifdef WINDOW_SCENE_SUPPORTED
    /**
     * @tc.steps: step1. viewport at (200,300) size (500,500); rect fully inside
     * @tc.expected: returned rect coords subtract viewport Left()/Top()
     */
    OHOS::Ace::ViewportConfig config;
    config.SetPosition(200, 300);
    config.SetSize(500, 500);

    OHOS::Rosen::Rect rect = { 250, 350, 200, 100 };
    auto result = IntersectRect(rect, config);

    EXPECT_EQ(result.posX_, 50);
    EXPECT_EQ(result.posY_, 50);
    EXPECT_EQ(result.width_, 200u);
    EXPECT_EQ(result.height_, 100u);
#endif
}

/**
 * @tc.name: DynamicPatternManagerTestNg007
 * @tc.desc: Test GetAvoidAreaIntersection includes avoid area with no intersection
 * @tc.type: FUNC
 */
HWTEST_F(DynamicPatternManagerTestNg, DynamicPatternManagerTestNg007, TestSize.Level1)
{
#ifdef WINDOW_SCENE_SUPPORTED
    /**
     * @tc.steps: step1. avoid area rects completely outside viewport
     * @tc.expected: avoid area still present in result (not filtered out)
     */
    auto safeManager = AceType::MakeRefPtr<DynamicComponentSafeManager>();
    ASSERT_NE(safeManager, nullptr);

    OHOS::Ace::ViewportConfig config;
    config.SetPosition(0, 0);
    config.SetSize(1080, 2400);

    std::map<OHOS::Rosen::AvoidAreaType, OHOS::Rosen::AvoidArea> avoidAreas;
    OHOS::Rosen::AvoidArea avoidArea;
    avoidArea.topRect_ = { 2000, 2000, 100, 100 };
    avoidAreas.emplace(OHOS::Rosen::AvoidAreaType::TYPE_SYSTEM, avoidArea);

    auto result = safeManager->GetAvoidAreaIntersection(avoidAreas, config);

    EXPECT_EQ(result.size(), 1u);
    auto it = result.find(OHOS::Rosen::AvoidAreaType::TYPE_SYSTEM);
    ASSERT_NE(it, result.end());
    EXPECT_EQ(it->second.topRect_.width_, 0u);
    EXPECT_EQ(it->second.topRect_.height_, 0u);
#endif
}

/**
 * @tc.name: DynamicPatternManagerTestNg008
 * @tc.desc: Test GetAvoidAreaIntersection returns viewport-relative coordinates
 * @tc.type: FUNC
 */
HWTEST_F(DynamicPatternManagerTestNg, DynamicPatternManagerTestNg008, TestSize.Level1)
{
#ifdef WINDOW_SCENE_SUPPORTED
    /**
     * @tc.steps: step1. viewport at (100,200); avoid area partially overlapping
     * @tc.expected: intersection coords are relative to viewport origin
     */
    auto safeManager = AceType::MakeRefPtr<DynamicComponentSafeManager>();
    ASSERT_NE(safeManager, nullptr);

    OHOS::Ace::ViewportConfig config;
    config.SetPosition(100, 200);
    config.SetSize(800, 600);

    std::map<OHOS::Rosen::AvoidAreaType, OHOS::Rosen::AvoidArea> avoidAreas;
    OHOS::Rosen::AvoidArea avoidArea;
    avoidArea.topRect_ = { 50, 150, 300, 200 };
    avoidArea.leftRect_ = { 80, 180, 300, 200 };
    avoidAreas.emplace(OHOS::Rosen::AvoidAreaType::TYPE_SYSTEM, avoidArea);

    auto result = safeManager->GetAvoidAreaIntersection(avoidAreas, config);

    EXPECT_EQ(result.size(), 1u);
    auto it = result.find(OHOS::Rosen::AvoidAreaType::TYPE_SYSTEM);
    ASSERT_NE(it, result.end());
    EXPECT_EQ(it->second.topRect_.posX_, 0);
    EXPECT_EQ(it->second.topRect_.posY_, 0);
    EXPECT_EQ(it->second.topRect_.width_, 250u);
    EXPECT_EQ(it->second.topRect_.height_, 150u);
    EXPECT_EQ(it->second.leftRect_.posX_, 0);
    EXPECT_EQ(it->second.leftRect_.posY_, 0);
    EXPECT_EQ(it->second.leftRect_.width_, 280u);
    EXPECT_EQ(it->second.leftRect_.height_, 180u);
#endif
}

/**
 * @tc.name: DynamicPatternManagerTestNg009
 * @tc.desc: Test GetAvoidAreaIntersection with multiple avoid area types all included
 * @tc.type: FUNC
 */
HWTEST_F(DynamicPatternManagerTestNg, DynamicPatternManagerTestNg009, TestSize.Level1)
{
#ifdef WINDOW_SCENE_SUPPORTED
    /**
     * @tc.steps: step1. two avoid area types, one intersecting and one not
     * @tc.expected: both types present in result
     */
    auto safeManager = AceType::MakeRefPtr<DynamicComponentSafeManager>();
    ASSERT_NE(safeManager, nullptr);

    OHOS::Ace::ViewportConfig config;
    config.SetPosition(0, 0);
    config.SetSize(1080, 2400);

    std::map<OHOS::Rosen::AvoidAreaType, OHOS::Rosen::AvoidArea> avoidAreas;

    OHOS::Rosen::AvoidArea intersectingArea;
    intersectingArea.topRect_ = { 0, 0, 1080, 100 };
    avoidAreas.emplace(OHOS::Rosen::AvoidAreaType::TYPE_SYSTEM, intersectingArea);

    OHOS::Rosen::AvoidArea nonIntersectingArea;
    nonIntersectingArea.bottomRect_ = { 5000, 5000, 100, 100 };
    avoidAreas.emplace(OHOS::Rosen::AvoidAreaType::TYPE_CUTOUT, nonIntersectingArea);

    auto result = safeManager->GetAvoidAreaIntersection(avoidAreas, config);

    EXPECT_EQ(result.size(), 2u);
    auto systemIt = result.find(OHOS::Rosen::AvoidAreaType::TYPE_SYSTEM);
    ASSERT_NE(systemIt, result.end());
    EXPECT_EQ(systemIt->second.topRect_.width_, 1080u);
    EXPECT_EQ(systemIt->second.topRect_.height_, 100u);

    auto cutoutIt = result.find(OHOS::Rosen::AvoidAreaType::TYPE_CUTOUT);
    ASSERT_NE(cutoutIt, result.end());
    EXPECT_EQ(cutoutIt->second.bottomRect_.width_, 0u);
    EXPECT_EQ(cutoutIt->second.bottomRect_.height_, 0u);
#endif
}
} // namespace OHOS::Ace::NG

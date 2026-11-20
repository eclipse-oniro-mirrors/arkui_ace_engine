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

#include "core/common/event_manager.h"

#include "base/geometry/ng/point_t.h"
#include "base/memory/ace_type.h"
#include "core/pipeline_ng/pipeline_context.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/geometry_node.h"
#include "core/components_ng/event/event_constants.h"
#include "core/components_ng/gestures/recognizers/pan_recognizer.h"
#include "core/components_ng/pattern/command/ui_command_security_gate.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/pattern/scroll/scroll_pattern.h"
#include "core/components_ng/relaxed_interaction/executors/strict_content_switch_executor.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "test/mock/adapter/ohos/osal/mock_input_manager.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {

namespace {
constexpr int32_t TEST_NODE_ID = 100;
const PointF TEST_COORDINATES(100.0f, 200.0f);
}

class TestableStrictContentSwitchExecutor : public StrictContentSwitchExecutor {
public:
    TestableStrictContentSwitchExecutor(WeakPtr<PipelineContext> context, const std::string mode,
        const std::string direction, const uint32_t count, const PointF& coordinates)
        : StrictContentSwitchExecutor(context, mode, direction, count, coordinates) {}

    ExecutorResult TestTriggerContentSwitch(const RefPtr<FrameNode> frameNode)
    {
        return TriggerContentSwitch(frameNode);
    }
};

class StrictContentSwitchExecutorTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        mockPipelineContext_ = AceType::MakeRefPtr<MockPipelineContext>();
        ASSERT_NE(mockPipelineContext_, nullptr);
    }

    static void TearDownTestCase()
    {
        mockPipelineContext_.Reset();
    }

protected:
    inline static RefPtr<MockPipelineContext> mockPipelineContext_ = nullptr;
};

class StrictContentSwitchExecutorConstructorTest : public StrictContentSwitchExecutorTest {};
class StrictContentSwitchExecutorExecuteStepTest : public StrictContentSwitchExecutorTest {};
class StrictContentSwitchExecutorGetTypeTest : public StrictContentSwitchExecutorTest {};
class StrictContentSwitchExecutorIsSingleStepTest : public StrictContentSwitchExecutorTest {};

HWTEST_F(StrictContentSwitchExecutorConstructorTest, StrictContentSwitchExecutor_Constructor, TestSize.Level1)
{
    auto context = WeakPtr<PipelineContext>(mockPipelineContext_);
    PointF coordinates(100.0f, 200.0f);
    StrictContentSwitchExecutor executor(context, "target", "forward", 1, coordinates);
    EXPECT_EQ(executor.context_, context);
    EXPECT_FLOAT_EQ(executor.coordinates_.GetX(), 100.0f);
    EXPECT_FLOAT_EQ(executor.coordinates_.GetY(), 200.0f);
}

HWTEST_F(StrictContentSwitchExecutorConstructorTest, StrictContentSwitchExecutor_Constructor_ZeroCoordinates,
    TestSize.Level1)
{
    auto context = WeakPtr<PipelineContext>(mockPipelineContext_);
    PointF coordinates(0.0f, 0.0f);
    StrictContentSwitchExecutor executor(context, "target", "forward", 1, coordinates);
    EXPECT_FLOAT_EQ(executor.coordinates_.GetX(), 0.0f);
    EXPECT_FLOAT_EQ(executor.coordinates_.GetY(), 0.0f);
}

HWTEST_F(StrictContentSwitchExecutorConstructorTest, StrictContentSwitchExecutor_Constructor_NegativeCoordinates,
    TestSize.Level1)
{
    auto context = WeakPtr<PipelineContext>(mockPipelineContext_);
    PointF coordinates(-100.0f, -200.0f);
    StrictContentSwitchExecutor executor(context, "target", "forward", 1, coordinates);
    EXPECT_FLOAT_EQ(executor.coordinates_.GetX(), -100.0f);
    EXPECT_FLOAT_EQ(executor.coordinates_.GetY(), -200.0f);
}

HWTEST_F(StrictContentSwitchExecutorConstructorTest, StrictContentSwitchExecutor_Constructor_LargeCoordinates,
    TestSize.Level1)
{
    auto context = WeakPtr<PipelineContext>(mockPipelineContext_);
    PointF coordinates(10000.0f, 20000.0f);
    StrictContentSwitchExecutor executor(context, "target", "forward", 1, coordinates);
    EXPECT_FLOAT_EQ(executor.coordinates_.GetX(), 10000.0f);
    EXPECT_FLOAT_EQ(executor.coordinates_.GetY(), 20000.0f);
}

HWTEST_F(StrictContentSwitchExecutorExecuteStepTest, ExecuteStep_NullContext_ReturnsFailed, TestSize.Level1)
{
    WeakPtr<PipelineContext> nullContext;
    PointF coordinates(100.0f, 200.0f);
    StrictContentSwitchExecutor executor(nullContext, "target", "forward", 1, coordinates);

    auto result = executor.ExecuteStep();
    EXPECT_EQ(result, ExecutorResult::FAILED);
}

HWTEST_F(StrictContentSwitchExecutorExecuteStepTest, ExecuteStep_NoFrameNodeFound_ReturnsFailed, TestSize.Level1)
{
    auto context = WeakPtr<PipelineContext>(mockPipelineContext_);
    PointF coordinates(100.0f, 200.0f);
    StrictContentSwitchExecutor executor(context, "target", "forward", 1, coordinates);

    auto result = executor.ExecuteStep();
    EXPECT_EQ(result, ExecutorResult::FAILED);
}

namespace {
class MockFrameNodeForSwitch : public FrameNode {
    DECLARE_ACE_TYPE(MockFrameNodeForSwitch, FrameNode);

public:
    MockFrameNodeForSwitch(const std::string& tag, int32_t nodeId, const RefPtr<Pattern>& pattern)
        : FrameNode(tag, nodeId, pattern) {}

    HitTestResult TouchTest(const PointF& globalPoint, const PointF& parentLocalPoint, // NOLINT
        const PointF& parentRevertPoint, TouchRestrict& touchRestrict, TouchTestResult& result,
        int32_t touchId, ResponseLinkResult& responseLinkResult, bool isDispatch = false) override
    {
        if (onTouchTest_) {
            onTouchTest_(result);
        }
        return HitTestResult::STOP_BUBBLING;
    }

    void SetTouchTestCallback(std::function<void(TouchTestResult&)> callback)
    {
        onTouchTest_ = std::move(callback);
    }

private:
    std::function<void(TouchTestResult&)> onTouchTest_;
};
} // namespace

/**
 * @tc.name: ExecuteStep_SecurityDeniedRefusesBeforeSwitch
 * @tc.desc: AC-4/R-8: with the security auth request on and the swiper target found by
 *           the recognizer declaring DENY plus the permission granted, the content
 *           switch command is refused before any side effect.
 * @tc.type: FUNC
 */
HWTEST_F(StrictContentSwitchExecutorExecuteStepTest, ExecuteStep_SecurityDeniedRefusesBeforeSwitch, TestSize.Level1)
{
    auto swiperNode = AceType::MakeRefPtr<FrameNode>(
        V2::SWIPER_ETS_TAG, 431, AceType::MakeRefPtr<Pattern>());
    swiperNode->SetAgentControlMode(AgentControlMode::DENY);
    auto panRecognizer = AceType::MakeRefPtr<PanRecognizer>(1, PanDirection(), 5.0);
    panRecognizer->AttachFrameNode(swiperNode);

    auto context = WeakPtr<PipelineContext>(mockPipelineContext_);
    auto rootNode = AceType::MakeRefPtr<MockFrameNodeForSwitch>("Root", 432, AceType::MakeRefPtr<Pattern>());
    rootNode->SetTouchTestCallback([&panRecognizer](TouchTestResult& result) {
        result.push_back(panRecognizer);
    });
    auto rootBackup = mockPipelineContext_->rootNode_;
    mockPipelineContext_->SetEventManager(AceType::MakeRefPtr<EventManager>());
    mockPipelineContext_->rootNode_ = rootNode;

    int reportCount = 0;
    auto savedReporter = MockInputManager::agentInputReporter;
    const bool savedGranted = MockInputManager::controlComponentPermissionGranted;
    auto savedCache = MockInputManager::controlComponentPermissionCache;
    MockInputManager::controlComponentPermissionGranted = true;
    MockInputManager::controlComponentPermissionCache.reset();
    MockInputManager::agentInputReporter = [&reportCount](const AgentInputResult& result) {
        EXPECT_FALSE(result.allowed);
        ++reportCount;
    };

    PointF coordinates(100.0f, 200.0f);
    StrictContentSwitchExecutor executor(context, "target", "forward", 1, coordinates);
    CommandSecurityContext security;
    security.needSecurityAuth = true;
    NG::UiCommandSecurityGate::SetContext(security);

    EXPECT_EQ(reportCount, 0);

    MockInputManager::agentInputReporter = savedReporter;
    MockInputManager::controlComponentPermissionGranted = savedGranted;
    MockInputManager::controlComponentPermissionCache = savedCache;
    NG::UiCommandSecurityGate::ClearContext();
        mockPipelineContext_->eventManager_.Reset();
    mockPipelineContext_->rootNode_ = rootBackup;
}

HWTEST_F(StrictContentSwitchExecutorGetTypeTest, GetType_ReturnsStrictContentSwitch, TestSize.Level1)
{
    auto context = WeakPtr<PipelineContext>(mockPipelineContext_);
    PointF coordinates(100.0f, 200.0f);
    StrictContentSwitchExecutor executor(context, "target", "forward", 1, coordinates);

    auto type = executor.GetType();
    EXPECT_EQ(type, "strict_content_switch");
}

HWTEST_F(StrictContentSwitchExecutorGetTypeTest, GetType_ReturnsSameValueForDifferentCoordinates, TestSize.Level1)
{
    auto context = WeakPtr<PipelineContext>(mockPipelineContext_);
    PointF coordinates1(100.0f, 200.0f);
    PointF coordinates2(300.0f, 400.0f);
    StrictContentSwitchExecutor executor1(context, "target", "forward", 1, coordinates1);
    StrictContentSwitchExecutor executor2(context, "target", "forward", 1, coordinates2);

    EXPECT_EQ(executor1.GetType(), executor2.GetType());
}

HWTEST_F(StrictContentSwitchExecutorIsSingleStepTest, IsSingleStep_ReturnsTrue, TestSize.Level1)
{
    auto context = WeakPtr<PipelineContext>(mockPipelineContext_);
    PointF coordinates(100.0f, 200.0f);
    StrictContentSwitchExecutor executor(context, "target", "forward", 1, coordinates);

    auto isSingleStep = executor.IsSingleStep();
    EXPECT_TRUE(isSingleStep);
}

HWTEST_F(StrictContentSwitchExecutorIsSingleStepTest, IsSingleStep_ReturnsTrueForDifferentCoordinates, TestSize.Level1)
{
    auto context = WeakPtr<PipelineContext>(mockPipelineContext_);
    PointF coordinates1(100.0f, 200.0f);
    PointF coordinates2(300.0f, 400.0f);
    StrictContentSwitchExecutor executor1(context, "target", "forward", 1, coordinates1);
    StrictContentSwitchExecutor executor2(context, "target", "forward", 1, coordinates2);

    EXPECT_TRUE(executor1.IsSingleStep());
    EXPECT_TRUE(executor2.IsSingleStep());
}

HWTEST_F(StrictContentSwitchExecutorExecuteStepTest, ExecuteStep_InvalidMode_ReturnsFailed, TestSize.Level1)
{
    auto context = WeakPtr<PipelineContext>(mockPipelineContext_);
    PointF coordinates(100.0f, 200.0f);
    StrictContentSwitchExecutor executor(context, "auto", "forward", 1, coordinates);

    auto result = executor.ExecuteStep();
    EXPECT_EQ(result, ExecutorResult::FAILED);
}

HWTEST_F(StrictContentSwitchExecutorExecuteStepTest, ExecuteStep_InvalidDirection_ReturnsFailed, TestSize.Level1)
{
    auto context = WeakPtr<PipelineContext>(mockPipelineContext_);
    PointF coordinates(100.0f, 200.0f);
    StrictContentSwitchExecutor executor(context, "target", "left", 1, coordinates);

    auto result = executor.ExecuteStep();
    EXPECT_EQ(result, ExecutorResult::FAILED);
}

HWTEST_F(StrictContentSwitchExecutorExecuteStepTest, ExecuteStep_InvalidCount_ReturnsFailed, TestSize.Level1)
{
    auto context = WeakPtr<PipelineContext>(mockPipelineContext_);
    PointF coordinates(100.0f, 200.0f);
    StrictContentSwitchExecutor executor(context, "target", "forward", -1, coordinates);

    auto result = executor.ExecuteStep();
    EXPECT_EQ(result, ExecutorResult::FAILED);
}

HWTEST_F(StrictContentSwitchExecutorExecuteStepTest, ExecuteStep_DirectionBackward_ReturnsFailed, TestSize.Level1)
{
    auto context = WeakPtr<PipelineContext>(mockPipelineContext_);
    PointF coordinates(0.0f, 0.0f);
    StrictContentSwitchExecutor executor(context, "target", "backward", 1, coordinates);

    auto result = executor.ExecuteStep();
    EXPECT_EQ(result, ExecutorResult::FAILED);
}

HWTEST_F(StrictContentSwitchExecutorExecuteStepTest, ExecuteStep_DirectionForward_ReturnsFailed, TestSize.Level1)
{
    auto context = WeakPtr<PipelineContext>(mockPipelineContext_);
    PointF coordinates(0.0f, 0.0f);
    StrictContentSwitchExecutor executor(context, "target", "forward", 1, coordinates);

    auto result = executor.ExecuteStep();
    EXPECT_EQ(result, ExecutorResult::FAILED);
}

HWTEST_F(StrictContentSwitchExecutorExecuteStepTest, ExecuteStep_FindFrameNode_ReturnsFailed, TestSize.Level1)
{
    auto context = WeakPtr<PipelineContext>(mockPipelineContext_);
    PointF coordinates(500.0f, 500.0f);
    StrictContentSwitchExecutor executor(context, "target", "forward", 1, coordinates);

    auto result = executor.ExecuteStep();
    EXPECT_EQ(result, ExecutorResult::FAILED);
}

HWTEST_F(StrictContentSwitchExecutorConstructorTest, Constructor_ModeStoredCorrectly, TestSize.Level1)
{
    auto context = WeakPtr<PipelineContext>(mockPipelineContext_);
    PointF coordinates(100.0f, 200.0f);
    StrictContentSwitchExecutor executor(context, "target", "forward", 1, coordinates);

    EXPECT_EQ(executor.mode_, "target");
    EXPECT_EQ(executor.direction_, "forward");
    EXPECT_EQ(executor.count_, 1u);
}

HWTEST_F(StrictContentSwitchExecutorConstructorTest, Constructor_BackwardDirection, TestSize.Level1)
{
    auto context = WeakPtr<PipelineContext>(mockPipelineContext_);
    PointF coordinates(100.0f, 200.0f);
    StrictContentSwitchExecutor executor(context, "target", "backward", 2, coordinates);

    EXPECT_EQ(executor.direction_, "backward");
    EXPECT_EQ(executor.count_, 2u);
}

HWTEST_F(StrictContentSwitchExecutorConstructorTest, Constructor_DifferentModes, TestSize.Level1)
{
    auto context = WeakPtr<PipelineContext>(mockPipelineContext_);
    PointF coordinates(100.0f, 200.0f);
    StrictContentSwitchExecutor executor1(context, "target", "forward", 1, coordinates);
    StrictContentSwitchExecutor executor2(context, "auto", "forward", 1, coordinates);

    EXPECT_EQ(executor1.mode_, "target");
    EXPECT_EQ(executor2.mode_, "auto");
}

HWTEST_F(StrictContentSwitchExecutorGetTypeTest, GetType_DifferentDirections_ReturnsSameType, TestSize.Level1)
{
    auto context = WeakPtr<PipelineContext>(mockPipelineContext_);
    PointF coordinates(100.0f, 200.0f);
    StrictContentSwitchExecutor executor1(context, "target", "forward", 1, coordinates);
    StrictContentSwitchExecutor executor2(context, "target", "backward", 1, coordinates);

    EXPECT_EQ(executor1.GetType(), executor2.GetType());
    EXPECT_EQ(executor1.GetType(), "strict_content_switch");
}

HWTEST_F(StrictContentSwitchExecutorIsSingleStepTest, IsSingleStep_DifferentDirections_ReturnsTrue, TestSize.Level1)
{
    auto context = WeakPtr<PipelineContext>(mockPipelineContext_);
    PointF coordinates(100.0f, 200.0f);
    StrictContentSwitchExecutor executor1(context, "target", "forward", 1, coordinates);
    StrictContentSwitchExecutor executor2(context, "target", "backward", 1, coordinates);

    EXPECT_TRUE(executor1.IsSingleStep());
    EXPECT_TRUE(executor2.IsSingleStep());
}

HWTEST_F(StrictContentSwitchExecutorExecuteStepTest, ExecuteStep_NonScrollablePattern_ReturnsFailed, TestSize.Level1)
{
    auto context = WeakPtr<PipelineContext>(mockPipelineContext_);

    auto frameNode = FrameNode::CreateFrameNode(
        "test", TEST_NODE_ID, AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(frameNode, nullptr);

    TestableStrictContentSwitchExecutor executor(context, "target", "forward", 1, TEST_COORDINATES);
    auto result = executor.TestTriggerContentSwitch(frameNode);
    EXPECT_EQ(result, ExecutorResult::FAILED);
}

HWTEST_F(StrictContentSwitchExecutorExecuteStepTest, ExecuteStep_ScrollablePattern_ReturnsFailed, TestSize.Level1)
{
    auto context = WeakPtr<PipelineContext>(mockPipelineContext_);

    auto scrollPattern = AceType::MakeRefPtr<ScrollPattern>();
    auto frameNode = FrameNode::CreateFrameNode(
        "scroll", TEST_NODE_ID, scrollPattern);
    ASSERT_NE(frameNode, nullptr);
    frameNode->SetGeometryNode(AceType::MakeRefPtr<GeometryNode>());

    TestableStrictContentSwitchExecutor executor(context, "target", "forward", 1, TEST_COORDINATES);
    auto result = executor.TestTriggerContentSwitch(frameNode);
    EXPECT_EQ(result, ExecutorResult::FAILED);
}

HWTEST_F(StrictContentSwitchExecutorExecuteStepTest, ExecuteStep_ScrollablePattern_BackwardDirection_ReturnsFailed,
    TestSize.Level1)
{
    auto context = WeakPtr<PipelineContext>(mockPipelineContext_);

    auto scrollPattern = AceType::MakeRefPtr<ScrollPattern>();
    auto frameNode = FrameNode::CreateFrameNode(
        "scroll", TEST_NODE_ID, scrollPattern);
    ASSERT_NE(frameNode, nullptr);
    frameNode->SetGeometryNode(AceType::MakeRefPtr<GeometryNode>());

    TestableStrictContentSwitchExecutor executor(context, "target", "backward", 1, TEST_COORDINATES);
    auto result = executor.TestTriggerContentSwitch(frameNode);
    EXPECT_EQ(result, ExecutorResult::FAILED);
}

} // namespace OHOS::Ace::NG
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
#include "base/memory/referenced.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/event/event_constants.h"
#include "core/components_ng/gestures/recognizers/click_recognizer.h"
#include "core/components_ng/pattern/command/ui_command_security_gate.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/relaxed_interaction/executors/scroll_simulating_executor.h"
#include "core/components_ng/relaxed_interaction/relaxed_interaction_types.h"
#include "core/pipeline_ng/pipeline_context.h"
#include "test/mock/adapter/ohos/osal/mock_input_manager.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {

class ScrollSimulatingExecutorTest : public testing::Test {
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

class ScrollSimulatingExecutorConstructorTest : public ScrollSimulatingExecutorTest {};
class ScrollSimulatingExecutorStepTest : public ScrollSimulatingExecutorTest {};
class ScrollSimulatingExecutorGetTypeTest : public ScrollSimulatingExecutorTest {};
class ScrollSimulatingExecutorGetDescriptionTest : public ScrollSimulatingExecutorTest {};
class ScrollSimulatingExecutorIsSingleStepTest : public ScrollSimulatingExecutorTest {};
class TouchEventBuilderBuildTest : public ScrollSimulatingExecutorTest {};
class TouchEventBuilderMoveByStepsTest : public ScrollSimulatingExecutorTest {};
class TouchEventBuilderSumStepsTest : public ScrollSimulatingExecutorTest {};

HWTEST_F(
    ScrollSimulatingExecutorConstructorTest, ScrollSimulatingExecutor_Constructor_LargeCoordinates, TestSize.Level1)
{
    auto context = WeakPtr<PipelineContext>(mockPipelineContext_);
    Command tmpCmd;
    tmpCmd.type = "scroll";
    tmpCmd.mode = ExecuteMode::SIMULATING;
    tmpCmd.actionInfo.distance = 1200.0;
    tmpCmd.actionInfo.coordinates.x1 = 100.0;
    tmpCmd.actionInfo.coordinates.y1 = 100.0;
    tmpCmd.actionInfo.coordinates.x2 = 120.0;
    tmpCmd.actionInfo.coordinates.y2 = 700.0;
    ScrollSimulatingExecutor executor(context, tmpCmd);

    EXPECT_FLOAT_EQ(executor.cmd_.actionInfo.coordinates.x1, 100.0f);
    EXPECT_FLOAT_EQ(executor.cmd_.actionInfo.coordinates.y1, 100.0f);
    EXPECT_FLOAT_EQ(executor.cmd_.actionInfo.coordinates.x2, 120.0f);
    EXPECT_FLOAT_EQ(executor.cmd_.actionInfo.coordinates.y2, 700.0f);
}

HWTEST_F(ScrollSimulatingExecutorStepTest, ExecuteStep_NullContext_ReturnsFailed, TestSize.Level1)
{
    WeakPtr<PipelineContext> nullContext;
    Command tmpCmd;
    tmpCmd.type = "scroll";
    tmpCmd.mode = ExecuteMode::SIMULATING;
    tmpCmd.actionInfo.distance = 1200.0;
    tmpCmd.actionInfo.coordinates.x1 = 0.0;
    tmpCmd.actionInfo.coordinates.y1 = 0.0;
    tmpCmd.actionInfo.coordinates.x2 = 60.0;
    tmpCmd.actionInfo.coordinates.y2 = 700.0;
    ScrollSimulatingExecutor executor(nullContext, tmpCmd);

    auto result = executor.ExecuteStep();
    EXPECT_EQ(result, ExecutorResult::FAILED);
}

HWTEST_F(ScrollSimulatingExecutorStepTest, ExecuteStep_EmptyTouchEvents_ReturnsFailed, TestSize.Level1)
{
    auto context = WeakPtr<PipelineContext>(mockPipelineContext_);
    Command tmpCmd;
    tmpCmd.type = "scroll";
    tmpCmd.mode = ExecuteMode::SIMULATING;
    tmpCmd.actionInfo.distance = 1200.0;
    tmpCmd.actionInfo.coordinates.x1 = 60.0;
    tmpCmd.actionInfo.coordinates.y1 = 60.0;
    tmpCmd.actionInfo.coordinates.x2 = 60.0;
    tmpCmd.actionInfo.coordinates.y2 = 60.0;
    tmpCmd.isY2Set = true;

    ScrollSimulatingExecutor::TouchEventBuilder builder;
    builder.MoveBySteps(tmpCmd.actionInfo.coordinates.x1, tmpCmd.actionInfo.coordinates.y1,
        tmpCmd.actionInfo.coordinates.x2, tmpCmd.actionInfo.coordinates.y2);
    EXPECT_EQ(builder.touchEvents_.size(), 1);

    while (!builder.touchEvents_.empty()) {
        builder.touchEvents_.pop();
    }
    ScrollSimulatingExecutor executor(context, tmpCmd);
    while (!executor.touchEvents_.empty()) {
        executor.touchEvents_.pop();
    }
    auto result = executor.ExecuteStep();
    EXPECT_EQ(result, ExecutorResult::FAILED);
}

HWTEST_F(ScrollSimulatingExecutorStepTest, ExecuteStep_ReturnsSUCCESS, TestSize.Level1)
{
    auto context = WeakPtr<PipelineContext>(mockPipelineContext_);
    Command tmpCmd;
    tmpCmd.type = "scroll";
    tmpCmd.mode = ExecuteMode::SIMULATING;
    tmpCmd.actionInfo.distance = 200.0;
    tmpCmd.actionInfo.coordinates.x1 = 80.0;
    tmpCmd.actionInfo.coordinates.y1 = 80.0;
    tmpCmd.actionInfo.coordinates.x2 = 80.0;
    tmpCmd.actionInfo.coordinates.y2 = 80.0;
    tmpCmd.isY2Set = true;

    ScrollSimulatingExecutor::TouchEventBuilder builder;
    builder.MoveBySteps(tmpCmd.actionInfo.coordinates.x1, tmpCmd.actionInfo.coordinates.y1,
        tmpCmd.actionInfo.coordinates.x2, tmpCmd.actionInfo.coordinates.y2);
    EXPECT_EQ(builder.touchEvents_.size(), 1);
    ScrollSimulatingExecutor executor(context, tmpCmd);
    while (executor.touchEvents_.size() > 1) {
        executor.touchEvents_.pop();
    }
    auto result = executor.ExecuteStep();
    EXPECT_EQ(result, ExecutorResult::SUCCESS);
}

HWTEST_F(ScrollSimulatingExecutorStepTest, ExecuteStep_ReturnsPENDING, TestSize.Level1)
{
    auto context = WeakPtr<PipelineContext>(mockPipelineContext_);
    Command tmpCmd;
    tmpCmd.type = "scroll";
    tmpCmd.mode = ExecuteMode::SIMULATING;
    tmpCmd.actionInfo.distance = 1200.0;
    tmpCmd.actionInfo.coordinates.x1 = 0.0;
    tmpCmd.actionInfo.coordinates.y1 = 0.0;
    tmpCmd.actionInfo.coordinates.x2 = 600.0;
    tmpCmd.actionInfo.coordinates.y2 = 600.0;
    tmpCmd.isY2Set = true;

    ScrollSimulatingExecutor::TouchEventBuilder builder;
    builder.MoveBySteps(tmpCmd.actionInfo.coordinates.x1, tmpCmd.actionInfo.coordinates.y1,
        tmpCmd.actionInfo.coordinates.x2, tmpCmd.actionInfo.coordinates.y2);
    EXPECT_EQ(builder.touchEvents_.size(), 8);
    ScrollSimulatingExecutor executor(context, tmpCmd);
    auto result = executor.ExecuteStep();
    EXPECT_EQ(result, ExecutorResult::PENDING);
}

HWTEST_F(ScrollSimulatingExecutorStepTest, ExecuteStep_ReturnsFAILED, TestSize.Level1)
{
    auto context = WeakPtr<PipelineContext>(mockPipelineContext_);
    Command tmpCmd;
    tmpCmd.type = "scroll";
    tmpCmd.mode = ExecuteMode::SIMULATING;
    tmpCmd.actionInfo.distance = 1200.0;
    tmpCmd.actionInfo.coordinates.x1 = 0.0;
    tmpCmd.actionInfo.coordinates.y1 = 0.0;
    tmpCmd.actionInfo.coordinates.x2 = 600.0;
    tmpCmd.actionInfo.coordinates.y2 = 600.0;
    tmpCmd.isY2Set = false;

    ScrollSimulatingExecutor::TouchEventBuilder builder;
    builder.MoveBySteps(tmpCmd.actionInfo.coordinates.x1, tmpCmd.actionInfo.coordinates.y1,
        tmpCmd.actionInfo.coordinates.x2, tmpCmd.actionInfo.coordinates.y2);
    ScrollSimulatingExecutor executor(context, tmpCmd);
    auto result = executor.ExecuteStep();
    EXPECT_EQ(result, ExecutorResult::FAILED);
}

HWTEST_F(ScrollSimulatingExecutorStepTest, ExecuteStep_ReturnFAILED, TestSize.Level1)
{
    auto context = WeakPtr<PipelineContext>(mockPipelineContext_);
    Command tmpCmd;
    tmpCmd.type = "scroll";
    tmpCmd.mode = ExecuteMode::AUTO;
    tmpCmd.actionInfo.distance = 1200.0;
    tmpCmd.actionInfo.coordinates.x1 = 0.0;
    tmpCmd.actionInfo.coordinates.y1 = 0.0;
    tmpCmd.actionInfo.coordinates.x2 = 600.0;
    tmpCmd.actionInfo.coordinates.y2 = 600.0;
    tmpCmd.isY2Set = false;
    ScrollSimulatingExecutor executor(context, tmpCmd);
    auto result = executor.ExecuteStep();
    EXPECT_EQ(result, ExecutorResult::FAILED);
}

HWTEST_F(ScrollSimulatingExecutorGetTypeTest, GetType_ReturnsType, TestSize.Level1)
{
    auto context = WeakPtr<PipelineContext>(mockPipelineContext_);
    Command tmpCmd;
    tmpCmd.type = "scroll";
    tmpCmd.mode = ExecuteMode::SIMULATING;
    tmpCmd.actionInfo.distance = 1200.0;
    tmpCmd.actionInfo.coordinates.x1 = 60.0;
    tmpCmd.actionInfo.coordinates.y1 = 60.0;
    tmpCmd.actionInfo.coordinates.x2 = 60.0;
    tmpCmd.actionInfo.coordinates.y2 = 700.0;
    tmpCmd.actionInfo.direction = RelaxedScrollDirection::BACKWARD;
    ScrollSimulatingExecutor executor(context, tmpCmd);

    auto type = executor.GetType();
    EXPECT_EQ(type, "scroll_simulating");
}

HWTEST_F(ScrollSimulatingExecutorGetTypeTest, GetType_ReturnsSameValueForDifferentCoordinates, TestSize.Level1)
{
    auto context = WeakPtr<PipelineContext>(mockPipelineContext_);
    Command tmpCmd;
    tmpCmd.type = "scroll";
    tmpCmd.mode = ExecuteMode::SIMULATING;
    tmpCmd.actionInfo.distance = 1200.0;
    tmpCmd.actionInfo.coordinates.x1 = 60.0;
    tmpCmd.actionInfo.coordinates.y1 = 60.0;
    tmpCmd.actionInfo.coordinates.x2 = 60.0;
    tmpCmd.actionInfo.coordinates.y2 = 700.0;
    tmpCmd.actionInfo.direction = RelaxedScrollDirection::BACKWARD;

    Command tmpCmd0;
    tmpCmd0.type = "scroll";
    tmpCmd0.mode = ExecuteMode::SIMULATING;
    tmpCmd0.actionInfo.distance = 1200.0;
    tmpCmd0.actionInfo.coordinates.x1 = 60.0;
    tmpCmd0.actionInfo.coordinates.y1 = 60.0;
    tmpCmd0.actionInfo.coordinates.x2 = 60.0;
    tmpCmd0.actionInfo.coordinates.y2 = 700.0;
    tmpCmd0.actionInfo.direction = RelaxedScrollDirection::BACKWARD;
    ScrollSimulatingExecutor executor1(context, tmpCmd);
    ScrollSimulatingExecutor executor2(context, tmpCmd0);

    EXPECT_EQ(executor1.GetType(), executor2.GetType());
}

HWTEST_F(ScrollSimulatingExecutorGetDescriptionTest, GetDescription_ReturnsNonEmpty, TestSize.Level1)
{
    auto context = WeakPtr<PipelineContext>(mockPipelineContext_);
    Command tmpCmd;
    tmpCmd.type = "scroll";
    tmpCmd.mode = ExecuteMode::SIMULATING;
    tmpCmd.actionInfo.distance = 1200.0;
    tmpCmd.actionInfo.coordinates.x1 = 60.0;
    tmpCmd.actionInfo.coordinates.y1 = 60.0;
    tmpCmd.actionInfo.coordinates.x2 = 60.0;
    tmpCmd.actionInfo.coordinates.y2 = 700.0;
    tmpCmd.actionInfo.direction = RelaxedScrollDirection::BACKWARD;
    ScrollSimulatingExecutor executor(context, tmpCmd);

    auto description = executor.GetDescription();
    EXPECT_FALSE(description.empty());
    EXPECT_TRUE(description.find("simulating") != std::string::npos);
}

HWTEST_F(ScrollSimulatingExecutorGetDescriptionTest, GetDescription_ContainsExpectedKeywords, TestSize.Level1)
{
    auto context = WeakPtr<PipelineContext>(mockPipelineContext_);
    Command tmpCmd;
    tmpCmd.type = "scroll";
    tmpCmd.mode = ExecuteMode::SIMULATING;
    tmpCmd.actionInfo.distance = 1200.0;
    tmpCmd.actionInfo.coordinates.x1 = 60.0;
    tmpCmd.actionInfo.coordinates.y1 = 60.0;
    tmpCmd.actionInfo.coordinates.x2 = 60.0;
    tmpCmd.actionInfo.coordinates.y2 = 700.0;
    tmpCmd.actionInfo.direction = RelaxedScrollDirection::BACKWARD;
    ScrollSimulatingExecutor executor(context, tmpCmd);

    auto description = executor.GetDescription();
    EXPECT_TRUE(description.find("scroll") != std::string::npos);
    EXPECT_TRUE(description.find("simulating") != std::string::npos);
}

HWTEST_F(ScrollSimulatingExecutorIsSingleStepTest, IsSingleStep_ReturnsFalse, TestSize.Level1)
{
    auto context = WeakPtr<PipelineContext>(mockPipelineContext_);
    Command tmpCmd;
    tmpCmd.type = "scroll";
    tmpCmd.mode = ExecuteMode::SIMULATING;
    tmpCmd.actionInfo.distance = 1200.0;
    tmpCmd.actionInfo.coordinates.x1 = 60.0;
    tmpCmd.actionInfo.coordinates.y1 = 60.0;
    tmpCmd.actionInfo.coordinates.x2 = 60.0;
    tmpCmd.actionInfo.coordinates.y2 = 700.0;
    tmpCmd.actionInfo.direction = RelaxedScrollDirection::BACKWARD;
    ScrollSimulatingExecutor executor(context, tmpCmd);

    auto isSingleStep = executor.IsSingleStep();
    EXPECT_FALSE(isSingleStep);
}

HWTEST_F(ScrollSimulatingExecutorIsSingleStepTest, IsSingleStep_ReturnsFalseForDifferentCoordinates, TestSize.Level1)
{
    auto context = WeakPtr<PipelineContext>(mockPipelineContext_);

    Command tmpCmd;
    tmpCmd.type = "scroll";
    tmpCmd.mode = ExecuteMode::SIMULATING;
    tmpCmd.actionInfo.distance = 1200.0;
    tmpCmd.actionInfo.coordinates.x1 = 60.0;
    tmpCmd.actionInfo.coordinates.y1 = 60.0;
    tmpCmd.actionInfo.coordinates.x2 = 60.0;
    tmpCmd.actionInfo.coordinates.y2 = 700.0;
    tmpCmd.actionInfo.direction = RelaxedScrollDirection::BACKWARD;

    Command tmpCmd0;
    tmpCmd0.type = "scroll";
    tmpCmd0.mode = ExecuteMode::SIMULATING;
    tmpCmd0.actionInfo.distance = 1200.0;
    tmpCmd0.actionInfo.coordinates.x1 = 30.0;
    tmpCmd0.actionInfo.coordinates.y1 = 120.0;
    tmpCmd0.actionInfo.coordinates.x2 = 30.0;
    tmpCmd0.actionInfo.coordinates.y2 = 700.0;
    tmpCmd0.actionInfo.direction = RelaxedScrollDirection::BACKWARD;
    ScrollSimulatingExecutor executor1(context, tmpCmd);
    ScrollSimulatingExecutor executor2(context, tmpCmd0);

    EXPECT_FALSE(executor1.IsSingleStep());
    EXPECT_FALSE(executor2.IsSingleStep());
}

HWTEST_F(TouchEventBuilderBuildTest, Build_NormalCoordinates_CreatesValidEvents, TestSize.Level1)
{
    ScrollSimulatingExecutor::TouchEventBuilder builder;
    Coordinates coords;
    coords.x1 = 100.0;
    coords.y1 = 200.0;
    coords.x2 = 100.0;
    coords.y2 = 500.0;

    auto events = builder.Build(coords);
    ASSERT_FALSE(events.empty());

    auto downEvent = events.front();
    EXPECT_EQ(downEvent.type, TouchType::DOWN);
    EXPECT_FLOAT_EQ(downEvent.x, 100.0f);
    EXPECT_FLOAT_EQ(downEvent.y, 200.0f);
    events.pop();

    bool hasMove = false;
    while (events.size() > 1) {
        auto moveEvent = events.front();
        EXPECT_EQ(moveEvent.type, TouchType::MOVE);
        hasMove = true;
        events.pop();
    }
    EXPECT_TRUE(hasMove);

    auto upEvent = events.front();
    EXPECT_EQ(upEvent.type, TouchType::UP);
    EXPECT_FLOAT_EQ(upEvent.x, 100.0f);
    EXPECT_FLOAT_EQ(upEvent.y, 500.0f);
}

HWTEST_F(TouchEventBuilderBuildTest, Build_SameStartEnd_OnlyDownAndUp, TestSize.Level1)
{
    ScrollSimulatingExecutor::TouchEventBuilder builder;
    Coordinates coords;
    coords.x1 = 50.0;
    coords.y1 = 50.0;
    coords.x2 = 50.0;
    coords.y2 = 50.0;

    auto events = builder.Build(coords);
    ASSERT_EQ(events.size(), 3);

    auto first = events.front();
    EXPECT_EQ(first.type, TouchType::DOWN);
    events.pop();

    auto second = events.front();
    EXPECT_EQ(second.type, TouchType::MOVE);
    events.pop();

    auto third = events.front();
    EXPECT_EQ(third.type, TouchType::UP);
}

HWTEST_F(TouchEventBuilderMoveByStepsTest, MoveBySteps_VerticalScroll_CreatesCorrectMoveEvents, TestSize.Level1)
{
    ScrollSimulatingExecutor::TouchEventBuilder builder;

    Coordinates coords;
    coords.x1 = 100.0;
    coords.y1 = 100.0;
    coords.x2 = 100.0;
    coords.y2 = 700.0;
    auto events = builder.Build(coords);

    int moveCount = 0;
    while (!events.empty()) {
        if (events.front().type == TouchType::MOVE) {
            moveCount++;
        }
        events.pop();
    }
    EXPECT_GT(moveCount, 1);
}

HWTEST_F(TouchEventBuilderMoveByStepsTest, MoveBySteps_HorizontalScroll_CreatesSmoothSteps, TestSize.Level1)
{
    ScrollSimulatingExecutor::TouchEventBuilder builder;
    Coordinates coords;
    coords.x1 = 50.0;
    coords.y1 = 300.0;
    coords.x2 = 350.0;
    coords.y2 = 300.0;
    auto events = builder.Build(coords);

    int moveCount = 0;
    while (!events.empty()) {
        auto e = events.front();
        if (e.type == TouchType::MOVE) {
            moveCount++;
            EXPECT_FLOAT_EQ(e.y, 300.0f);
        }
        events.pop();
    }
    EXPECT_GT(moveCount, 1);
}

HWTEST_F(TouchEventBuilderSumStepsTest, SumSteps_NormalDistance_ReturnsValidSteps, TestSize.Level1)
{
    ScrollSimulatingExecutor::TouchEventBuilder builder;

    int steps = builder.SumSteps(100.0, 100.0, 100.0, 700.0, 100);
    EXPECT_EQ(steps, 6);

    steps = builder.SumSteps(0.0, 0.0, 10.0, 0.0, 100);
    EXPECT_EQ(steps, 0);

    // Diagonal scrolling
    steps = builder.SumSteps(0.0, 0.0, 300.0, 400.0, 100);
    EXPECT_EQ(steps, 5);
}

HWTEST_F(TouchEventBuilderSumStepsTest, SumSteps_ZeroDistance_ReturnsZero, TestSize.Level1)
{
    ScrollSimulatingExecutor::TouchEventBuilder builder;
    int steps = builder.SumSteps(50.0, 50.0, 50.0, 50.0, 100);
    EXPECT_EQ(steps, 0);
}

HWTEST_F(TouchEventBuilderSumStepsTest, SumSteps_ZeroDistance_ReturnsZero1, TestSize.Level1)
{
    ScrollSimulatingExecutor::TouchEventBuilder builder;
    int steps = builder.SumSteps(50.0, 50.0, 50.0, 50.0, 0);
    EXPECT_EQ(steps, 0);
}

HWTEST_F(ScrollSimulatingExecutorStepTest, ExecuteStep_ReturnPENDING, TestSize.Level1)
{
    auto context = WeakPtr<PipelineContext>(mockPipelineContext_);
    Command tmpCmd;
    tmpCmd.type = "scroll";
    tmpCmd.mode = ExecuteMode::TARGET;
    tmpCmd.actionInfo.distance = 1200.0;
    tmpCmd.actionInfo.coordinates.x1 = 0.0;
    tmpCmd.actionInfo.coordinates.y1 = 0.0;
    tmpCmd.actionInfo.coordinates.x2 = 600.0;
    tmpCmd.actionInfo.coordinates.y2 = 600.0;
    tmpCmd.isY2Set = false;
    ScrollSimulatingExecutor executor(context, tmpCmd);
    auto result = executor.ExecuteStep();
    EXPECT_EQ(result, ExecutorResult::PENDING);
}

HWTEST_F(ScrollSimulatingExecutorStepTest, ExecuteStep_ReturnPENDING1, TestSize.Level1)
{
    auto context = WeakPtr<PipelineContext>(mockPipelineContext_);
    Command tmpCmd;
    tmpCmd.type = "scroll";
    tmpCmd.mode = ExecuteMode::TARGET;
    tmpCmd.actionInfo.distance = 1200.0;
    tmpCmd.actionInfo.coordinates.x1 = 0.0;
    tmpCmd.actionInfo.coordinates.y1 = 0.0;
    tmpCmd.actionInfo.coordinates.x2 = 600.0;
    tmpCmd.actionInfo.coordinates.y2 = 600.0;
    tmpCmd.isY2Set = true;
    ScrollSimulatingExecutor executor(context, tmpCmd);
    auto result = executor.ExecuteStep();
    EXPECT_EQ(result, ExecutorResult::PENDING);
}

namespace {
class MockFrameNodeForScroll : public FrameNode {
    DECLARE_ACE_TYPE(MockFrameNodeForScroll, FrameNode);

public:
    MockFrameNodeForScroll(const std::string& tag, int32_t nodeId, const RefPtr<Pattern>& pattern)
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
 * @tc.name: ExecuteStep_SecurityDeniedRefusesBeforeScrollDispatch
 * @tc.desc: AC-4/R-8: with the security auth request on and the hit target at the scroll
 *           start point declaring DENY plus the permission granted, the scroll is refused
 *           before any touch event is dispatched.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollSimulatingExecutorStepTest, ExecuteStep_SecurityDeniedRefusesBeforeScrollDispatch, TestSize.Level1)
{
    auto buttonNode = AceType::MakeRefPtr<FrameNode>("Button", 411, AceType::MakeRefPtr<Pattern>());
    buttonNode->SetAgentControlMode(AgentControlMode::DENY);
    auto clickRecognizer = AceType::MakeRefPtr<ClickRecognizer>();
    clickRecognizer->AttachFrameNode(buttonNode);

    auto context = WeakPtr<PipelineContext>(mockPipelineContext_);
    auto rootNode = AceType::MakeRefPtr<MockFrameNodeForScroll>("Root", 412, AceType::MakeRefPtr<Pattern>());
    rootNode->SetTouchTestCallback([&clickRecognizer](TouchTestResult& result) {
        result.push_back(clickRecognizer);
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

    Command tmpCmd;
    tmpCmd.type = "scroll";
    tmpCmd.mode = ExecuteMode::SIMULATING;
    tmpCmd.actionInfo.distance = 1200.0;
    tmpCmd.actionInfo.coordinates.x1 = 60.0;
    tmpCmd.actionInfo.coordinates.y1 = 60.0;
    tmpCmd.actionInfo.coordinates.x2 = 60.0;
    tmpCmd.actionInfo.coordinates.y2 = 700.0;
    tmpCmd.isY2Set = true;
    ScrollSimulatingExecutor executor(context, tmpCmd);
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

/**
 * @tc.name: ExecuteStep_SecurityAuthFailureProceedsWithScroll
 * @tc.desc: AC-6/R-3: the auth flag is on and the scroll start target is DENY, but the
 *           permission check fails; the scroll proceeds through the original path.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollSimulatingExecutorStepTest, ExecuteStep_SecurityAuthFailureProceedsWithScroll, TestSize.Level1)
{
    auto buttonNode = AceType::MakeRefPtr<FrameNode>("Button", 413, AceType::MakeRefPtr<Pattern>());
    buttonNode->SetAgentControlMode(AgentControlMode::DENY);
    auto clickRecognizer = AceType::MakeRefPtr<ClickRecognizer>();
    clickRecognizer->AttachFrameNode(buttonNode);

    auto context = WeakPtr<PipelineContext>(mockPipelineContext_);
    auto rootNode = AceType::MakeRefPtr<MockFrameNodeForScroll>("Root", 414, AceType::MakeRefPtr<Pattern>());
    rootNode->SetTouchTestCallback([&clickRecognizer](TouchTestResult& result) {
        result.push_back(clickRecognizer);
    });
    auto rootBackup = mockPipelineContext_->rootNode_;
    mockPipelineContext_->SetEventManager(AceType::MakeRefPtr<EventManager>());
    mockPipelineContext_->rootNode_ = rootNode;

    const bool savedGranted = MockInputManager::controlComponentPermissionGranted;
    auto savedCache = MockInputManager::controlComponentPermissionCache;
    MockInputManager::controlComponentPermissionGranted = false;
    MockInputManager::controlComponentPermissionCache.reset();

    Command tmpCmd;
    tmpCmd.type = "scroll";
    tmpCmd.mode = ExecuteMode::SIMULATING;
    tmpCmd.actionInfo.distance = 1200.0;
    tmpCmd.actionInfo.coordinates.x1 = 60.0;
    tmpCmd.actionInfo.coordinates.y1 = 60.0;
    tmpCmd.actionInfo.coordinates.x2 = 60.0;
    tmpCmd.actionInfo.coordinates.y2 = 700.0;
    tmpCmd.isY2Set = true;
    ScrollSimulatingExecutor executor(context, tmpCmd);
    CommandSecurityContext security;
    security.needSecurityAuth = true;
    NG::UiCommandSecurityGate::SetContext(security);

    auto result = executor.ExecuteStep();
    EXPECT_NE(result, ExecutorResult::FAILED);

    MockInputManager::controlComponentPermissionGranted = savedGranted;
    MockInputManager::controlComponentPermissionCache = savedCache;
    NG::UiCommandSecurityGate::ClearContext();
        mockPipelineContext_->eventManager_.Reset();
    mockPipelineContext_->rootNode_ = rootBackup;
}
} // namespace OHOS::Ace::NG

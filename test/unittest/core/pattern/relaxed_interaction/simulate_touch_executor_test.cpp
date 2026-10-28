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
#include <vector>

#include "mock_pipeline_destruction.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"

#include "base/geometry/ng/point_t.h"
#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"
#include "core/components_ng/relaxed_interaction/executors/simulate_touch_executor.h"
#include "core/pipeline_ng/pipeline_context.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {

namespace {
PipelineBase* g_destroyedContext = nullptr;
}

void SetDestroyedContextForTest(PipelineBase* context)
{
    g_destroyedContext = context;
}

extern "C" bool RealPipelineIsDestroyed(PipelineBase* context)
    asm("__real__ZN4OHOS3Ace12PipelineBase11IsDestroyedEv");
extern "C" bool WrapPipelineIsDestroyed(PipelineBase* context)
    asm("__wrap__ZN4OHOS3Ace12PipelineBase11IsDestroyedEv");

extern "C" bool WrapPipelineIsDestroyed(PipelineBase* context)
{
    return context == g_destroyedContext || RealPipelineIsDestroyed(context);
}

class SimulateTouchExecutorTest : public testing::Test {
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

class SimulateTouchExecutorConstructorTest : public SimulateTouchExecutorTest {};
class SimulateTouchExecutorExecuteStepTest : public SimulateTouchExecutorTest {};
class SimulateTouchExecutorGetTypeTest : public SimulateTouchExecutorTest {};
class SimulateTouchExecutorGetDescriptionTest : public SimulateTouchExecutorTest {};
class SimulateTouchExecutorIsSingleStepTest : public SimulateTouchExecutorTest {};

HWTEST_F(SimulateTouchExecutorExecuteStepTest, DestructionDuringDownStopsDispatch, TestSize.Level1)
{
    auto context = AceType::MakeRefPtr<MockPipelineContext>();
    std::vector<TouchEvent> events;
    EXPECT_CALL(*context, OnTouchEvent(testing::_, testing::_)).WillOnce([&](const TouchEvent& event, bool) {
        events.push_back(event);
        SetDestroyedContextForTest(context.GetRawPtr());
        EXPECT_FALSE(mockPipelineContext_->IsDestroyed());
    });
    SimulateTouchExecutor executor(context, PointF(10.0f, 20.0f));
    const auto result = executor.ExecuteStep();
    SetDestroyedContextForTest(nullptr);
    EXPECT_FALSE(context->IsDestroyed());
    EXPECT_EQ(result, ExecutorResult::FAILED);
    EXPECT_EQ(executor.GetFailureState(), ExecutionState::TARGET_NOT_FOUND);
    ASSERT_EQ(events.size(), 1u);
    EXPECT_EQ(events[0].type, TouchType::DOWN);
    EXPECT_TRUE(events[0].isInjected);
}

HWTEST_F(SimulateTouchExecutorConstructorTest, SimulateTouchExecutor_Constructor, TestSize.Level1)
{
    auto context = WeakPtr<PipelineContext>(mockPipelineContext_);
    PointF coordinates(100.0f, 200.0f);
    SimulateTouchExecutor executor(context, coordinates);
    EXPECT_EQ(executor.context_, context);
    EXPECT_FLOAT_EQ(executor.coordinates_.GetX(), 100.0f);
    EXPECT_FLOAT_EQ(executor.coordinates_.GetY(), 200.0f);
}

HWTEST_F(SimulateTouchExecutorConstructorTest, SimulateTouchExecutor_Constructor_ZeroCoordinates, TestSize.Level1)
{
    auto context = WeakPtr<PipelineContext>(mockPipelineContext_);
    PointF coordinates(0.0f, 0.0f);
    SimulateTouchExecutor executor(context, coordinates);
    EXPECT_FLOAT_EQ(executor.coordinates_.GetX(), 0.0f);
    EXPECT_FLOAT_EQ(executor.coordinates_.GetY(), 0.0f);
}

HWTEST_F(SimulateTouchExecutorConstructorTest, SimulateTouchExecutor_Constructor_NegativeCoordinates, TestSize.Level1)
{
    auto context = WeakPtr<PipelineContext>(mockPipelineContext_);
    PointF coordinates(-100.0f, -200.0f);
    SimulateTouchExecutor executor(context, coordinates);
    EXPECT_FLOAT_EQ(executor.coordinates_.GetX(), -100.0f);
    EXPECT_FLOAT_EQ(executor.coordinates_.GetY(), -200.0f);
}

HWTEST_F(SimulateTouchExecutorConstructorTest, SimulateTouchExecutor_Constructor_LargeCoordinates, TestSize.Level1)
{
    auto context = WeakPtr<PipelineContext>(mockPipelineContext_);
    PointF coordinates(10000.0f, 20000.0f);
    SimulateTouchExecutor executor(context, coordinates);
    EXPECT_FLOAT_EQ(executor.coordinates_.GetX(), 10000.0f);
    EXPECT_FLOAT_EQ(executor.coordinates_.GetY(), 20000.0f);
}

HWTEST_F(SimulateTouchExecutorConstructorTest, SimulateTouchExecutor_Constructor_FloatCoordinates, TestSize.Level1)
{
    auto context = WeakPtr<PipelineContext>(mockPipelineContext_);
    PointF coordinates(123.45f, 678.90f);
    SimulateTouchExecutor executor(context, coordinates);
    EXPECT_FLOAT_EQ(executor.coordinates_.GetX(), 123.45f);
    EXPECT_FLOAT_EQ(executor.coordinates_.GetY(), 678.90f);
}

HWTEST_F(SimulateTouchExecutorExecuteStepTest, ExecuteStep_NullContext_ReturnsFailed, TestSize.Level1)
{
    WeakPtr<PipelineContext> nullContext;
    PointF coordinates(100.0f, 200.0f);
    SimulateTouchExecutor executor(nullContext, coordinates);
    auto result = executor.ExecuteStep();
    EXPECT_EQ(result, ExecutorResult::FAILED);
}

HWTEST_F(SimulateTouchExecutorExecuteStepTest, ExecuteStep_ValidContext_ReturnsSuccess, TestSize.Level1)
{
    auto context = WeakPtr<PipelineContext>(mockPipelineContext_);
    PointF coordinates(100.0f, 200.0f);
    SimulateTouchExecutor executor(context, coordinates);
    auto result = executor.ExecuteStep();
    EXPECT_EQ(result, ExecutorResult::SUCCESS);
}

HWTEST_F(SimulateTouchExecutorExecuteStepTest, ExecuteStep_DifferentCoordinates_ReturnsSuccess, TestSize.Level1)
{
    auto context = WeakPtr<PipelineContext>(mockPipelineContext_);
    PointF coordinates(50.5f, 75.5f);
    SimulateTouchExecutor executor(context, coordinates);
    auto result = executor.ExecuteStep();
    EXPECT_EQ(result, ExecutorResult::SUCCESS);
}

HWTEST_F(SimulateTouchExecutorExecuteStepTest, ExecuteStep_ZeroCoordinates_ReturnsSuccess, TestSize.Level1)
{
    auto context = WeakPtr<PipelineContext>(mockPipelineContext_);
    PointF coordinates(0.0f, 0.0f);
    SimulateTouchExecutor executor(context, coordinates);
    auto result = executor.ExecuteStep();
    EXPECT_EQ(result, ExecutorResult::SUCCESS);
}

HWTEST_F(SimulateTouchExecutorGetTypeTest, GetType_ReturnsSimulateTouch, TestSize.Level1)
{
    auto context = WeakPtr<PipelineContext>(mockPipelineContext_);
    PointF coordinates(100.0f, 200.0f);
    SimulateTouchExecutor executor(context, coordinates);
    auto type = executor.GetType();
    EXPECT_EQ(type, "simulate_touch");
}

HWTEST_F(SimulateTouchExecutorGetDescriptionTest, GetDescription_ReturnsNonEmpty, TestSize.Level1)
{
    auto context = WeakPtr<PipelineContext>(mockPipelineContext_);
    PointF coordinates(100.0f, 200.0f);
    SimulateTouchExecutor executor(context, coordinates);
    auto description = executor.GetDescription();
    EXPECT_FALSE(description.empty());
}

HWTEST_F(SimulateTouchExecutorGetDescriptionTest, GetDescription_ContainsExpectedKeywords, TestSize.Level1)
{
    auto context = WeakPtr<PipelineContext>(mockPipelineContext_);
    PointF coordinates(100.0f, 200.0f);
    SimulateTouchExecutor executor(context, coordinates);
    auto description = executor.GetDescription();
    EXPECT_TRUE(description.find("simulate_touch") != std::string::npos);
}

HWTEST_F(SimulateTouchExecutorIsSingleStepTest, IsSingleStep_ReturnsTrue, TestSize.Level1)
{
    auto context = WeakPtr<PipelineContext>(mockPipelineContext_);
    PointF coordinates(100.0f, 200.0f);
    SimulateTouchExecutor executor(context, coordinates);
    auto isSingleStep = executor.IsSingleStep();
    EXPECT_TRUE(isSingleStep);
}

HWTEST_F(SimulateTouchExecutorIsSingleStepTest, IsSingleStep_ReturnsTrueForDifferentCoordinates, TestSize.Level1)
{
    auto context = WeakPtr<PipelineContext>(mockPipelineContext_);
    PointF coordinates1(100.0f, 200.0f);
    PointF coordinates2(300.0f, 400.0f);
    SimulateTouchExecutor executor1(context, coordinates1);
    SimulateTouchExecutor executor2(context, coordinates2);
    EXPECT_TRUE(executor1.IsSingleStep());
    EXPECT_TRUE(executor2.IsSingleStep());
}

} // namespace OHOS::Ace::NG

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

#include <vector>

#include "gtest/gtest.h"

#include "frameworks/core/components_ng/base/frame_node.h"
#include "frameworks/core/components_ng/base/frame_node_adopted_compensation.h"
#include "frameworks/core/components_ng/base/frame_node_adopted_render_adapter.h"
#include "frameworks/core/components_ng/base/mount_policy.h"
#include "frameworks/core/components_ng/pattern/text/text_base.h"
#include "frameworks/core/components_ng/render/adapter/rosen_render_context.h"
#include "ui/rs_canvas_node.h"
#include "test/mock/frameworks/base/thread/mock_task_executor.h"
#include "test/mock/frameworks/core/common/mock_container.h"
#include "test/mock/frameworks/core/common/mock_theme_manager.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace {
namespace {
constexpr int32_t NODE_SIZE = 100;
constexpr int32_t INNER_NODE_HEIGHT = 80;
constexpr int32_t OUTER_POSITION_X = 10;
constexpr int32_t OUTER_POSITION_Y = 20;
constexpr int32_t INNER_POSITION_X = 30;
constexpr int32_t INNER_POSITION_Y = 40;
constexpr int32_t UPDATED_OUTER_POSITION_X = 20;
constexpr int32_t UPDATED_OUTER_POSITION_Y = 30;
constexpr int32_t TWO_LAYERS = 2;
constexpr float HALF_PIVOT = 0.5f;
constexpr float TWO_LAYER_SCALE = 2.0f;
constexpr float INNER_ROTATION = 90.0f;
constexpr float LAYER_TRANSLATION_X = 3.0f;
constexpr float LAYER_TRANSLATION_Y = 4.0f;
constexpr float DEVELOPER_OFFSET_X = 40.0f;
constexpr float DEVELOPER_OFFSET_Y = 60.0f;
constexpr float MATRIX_OFFSET_X = 20.0f;
constexpr float MATRIX_OFFSET_Y = 50.0f;
constexpr float UPDATED_DEVELOPER_OFFSET_X = 500.0f;
constexpr float UPDATED_DEVELOPER_OFFSET_Y = 600.0f;
constexpr float INPUT_POINT_X = 1.0f;
constexpr float INPUT_POINT_Y = 2.0f;
constexpr float INPUT_RECT_WIDTH = 3.0f;
constexpr float INPUT_RECT_HEIGHT = 4.0f;
constexpr float WINDOW_POINT = 100.0f;
constexpr float EXPECTED_INITIAL_OFFSET_X = -90.0f;
constexpr float EXPECTED_INITIAL_OFFSET_Y = 10.0f;
constexpr float EXPECTED_POINT_X = 16.0f;
constexpr float EXPECTED_POINT_Y = 52.0f;
constexpr float EXPECTED_RECT_X = -138.0f;
constexpr float EXPECTED_RECT_Y = 54.0f;
constexpr float EXPECTED_RECT_WIDTH = 6.0f;
constexpr float EXPECTED_RECT_HEIGHT = 8.0f;
constexpr float EXPECTED_WINDOW_X = -60.0f;
constexpr float EXPECTED_WINDOW_Y = 50.0f;
constexpr float EXPECTED_UPDATED_OFFSET_X = -80.0f;
constexpr float EXPECTED_UPDATED_OFFSET_Y = 20.0f;
constexpr float COMBINED_TRANSLATION_X = 13.0f;
constexpr float COMBINED_TRANSLATION_Y = 24.0f;
constexpr float EXPECTED_FROM_WINDOW_X = 93.0f;
constexpr float EXPECTED_FROM_WINDOW_Y = 84.0f;
constexpr float TEST_TOLERANCE = 0.001f;
} // namespace

class NativeRenderNodeAdoptedCompensationTest : public testing::Test {
public:
    static void SetUpTestSuite()
    {
        NG::MockPipelineContext::SetUp();
        auto pipelineContext = NG::MockPipelineContext::GetCurrent();
        MockContainer::SetUp(pipelineContext);
        auto taskExecutor = AceType::MakeRefPtr<MockTaskExecutor>();
        MockContainer::Current()->SetTaskExecutor(taskExecutor);
        pipelineContext->SetTaskExecutor(taskExecutor);
        auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
        PipelineBase::GetCurrentContext()->SetThemeManager(themeManager);
    }

    static void TearDownTestSuite()
    {
        NG::MockPipelineContext::TearDown();
        MockContainer::TearDown();
    }

    void TearDown() override
    {
        if (parentNode_ && childNode_) {
            parentNode_->RemoveAdoptedChild(childNode_);
        }
        adoptedRenderNode_.reset();
        innerRenderNode_.reset();
        outerRenderNode_.reset();
        childNode_.Reset();
        parentNode_.Reset();
    }

protected:
    void CreateAdoptedNodes()
    {
        parentNode_ = NG::FrameNode::CreateFrameNode("adoptParent",
            ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<NG::Pattern>());
        childNode_ = NG::FrameNode::CreateFrameNode("adoptedChild",
            ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<NG::Pattern>());
        ASSERT_NE(parentNode_, nullptr);
        ASSERT_NE(childNode_, nullptr);
        parentNode_->AdoptChild(childNode_);
        childFrameNode_ = childNode_.GetRawPtr();
        NG::FrameNodeAdoptedCompensation::SetUseAdoptedCompensation(childFrameNode_, true);
        parentNode_->SetMountPolicy(NG::MountPolicy::MIXED);
        auto childContext = AceType::DynamicCast<NG::RosenRenderContext>(childNode_->GetRenderContext());
        ASSERT_NE(childContext, nullptr);
        adoptedRenderNode_ = childContext->GetRSNode();
        ASSERT_NE(adoptedRenderNode_, nullptr);
    }

    void MountTwoLayerTree()
    {
        outerRenderNode_ = Rosen::RSCanvasNode::Create();
        innerRenderNode_ = Rosen::RSCanvasNode::Create();
        ASSERT_NE(outerRenderNode_, nullptr);
        ASSERT_NE(innerRenderNode_, nullptr);
        outerRenderNode_->SetBounds(OUTER_POSITION_X, OUTER_POSITION_Y, NODE_SIZE, NODE_SIZE);
        outerRenderNode_->SetPivot(HALF_PIVOT, HALF_PIVOT);
        outerRenderNode_->SetScale(TWO_LAYER_SCALE, TWO_LAYER_SCALE);
        innerRenderNode_->SetBounds(INNER_POSITION_X, INNER_POSITION_Y, NODE_SIZE, INNER_NODE_HEIGHT);
        innerRenderNode_->SetPivot(0.0f, 0.0f);
        innerRenderNode_->SetRotation(INNER_ROTATION);
        auto parentContext = AceType::DynamicCast<NG::RosenRenderContext>(parentNode_->GetRenderContext());
        ASSERT_NE(parentContext, nullptr);
        parentContext->InsertPureRenderChildAt(outerRenderNode_, -1);
        outerRenderNode_->AddChild(innerRenderNode_, -1);
        innerRenderNode_->AddChild(adoptedRenderNode_, -1);
    }

    void SetTwoLayerCompensation()
    {
        // Matrix4's constructor takes row-major elements; preserve the original C API matrix.
        Matrix4 transform(
            0.0, -TWO_LAYER_SCALE, 0.0, MATRIX_OFFSET_X,
            TWO_LAYER_SCALE, 0.0, 0.0, MATRIX_OFFSET_Y,
            0.0, 0.0, 1.0, 0.0,
            0.0, 0.0, 0.0, 1.0);
        NG::FrameNodeAdoptedCompensation::SetAdoptedCompensation(childFrameNode_,
            NG::OffsetF(DEVELOPER_OFFSET_X, DEVELOPER_OFFSET_Y), transform);
    }

    void SetUpTwoLayerScenario()
    {
        ASSERT_NO_FATAL_FAILURE(CreateAdoptedNodes());
        ASSERT_NO_FATAL_FAILURE(MountTwoLayerTree());
        ASSERT_NO_FATAL_FAILURE(SetTwoLayerCompensation());
        ASSERT_NO_FATAL_FAILURE(RebuildAndVerifyLayers(TWO_LAYERS));
    }

    void RebuildAndVerifyLayers(size_t expectedLayerCount)
    {
        // MIXED children are attached to the parent's RS node during render-tree sync.
        parentNode_->MarkNeedSyncRenderTree();
        parentNode_->RebuildRenderContextTree();
        std::vector<NG::FrameNodeAdoptedRenderAdapter::RSNodePtr> layers;
        ASSERT_TRUE(NG::FrameNodeAdoptedRenderAdapter::CollectLayers(childFrameNode_, layers));
        ASSERT_EQ(layers.size(), expectedLayerCount);
    }

    void VerifyTwoLayerPointGeometry()
    {
        EXPECT_EQ(NG::FrameNodeAdoptedCompensation::GetAdoptedCompensationOffset(childFrameNode_),
            NG::OffsetF(DEVELOPER_OFFSET_X, DEVELOPER_OFFSET_Y));
        auto transformedOffset = NG::FrameNodeAdoptedCompensation::GetTransformedAdoptedCompensationOffset(
            childFrameNode_);
        EXPECT_NEAR(transformedOffset.GetX(), EXPECTED_INITIAL_OFFSET_X, TEST_TOLERANCE);
        EXPECT_NEAR(transformedOffset.GetY(), EXPECTED_INITIAL_OFFSET_Y, TEST_TOLERANCE);
        auto scale = NG::FrameNodeAdoptedCompensation::GetAdoptedCompensationScale(childFrameNode_);
        EXPECT_FLOAT_EQ(scale.x, TWO_LAYER_SCALE);
        EXPECT_FLOAT_EQ(scale.y, TWO_LAYER_SCALE);
        EXPECT_TRUE(NG::FrameNodeAdoptedCompensation::HasAdoptedCompensationNonTranslationTransform(
            childFrameNode_));
        NG::PointF point(INPUT_POINT_X, INPUT_POINT_Y);
        NG::FrameNodeAdoptedCompensation::ApplyAdoptedCompensationToPoint(childFrameNode_, point);
        EXPECT_NEAR(point.GetX(), EXPECTED_POINT_X, TEST_TOLERANCE);
        EXPECT_NEAR(point.GetY(), EXPECTED_POINT_Y, TEST_TOLERANCE);
        auto actualTransform = NG::FrameNodeAdoptedCompensation::GetAdoptedCompensationTransform(childFrameNode_);
        auto transformedPoint = actualTransform * Point(INPUT_POINT_X, INPUT_POINT_Y);
        EXPECT_NEAR(transformedPoint.GetX(), EXPECTED_POINT_X, TEST_TOLERANCE);
        EXPECT_NEAR(transformedPoint.GetY(), EXPECTED_POINT_Y, TEST_TOLERANCE);
        NG::FrameNodeAdoptedCompensation::RevertAdoptedCompensationFromPoint(childFrameNode_, point);
        EXPECT_NEAR(point.GetX(), INPUT_POINT_X, TEST_TOLERANCE);
        EXPECT_NEAR(point.GetY(), INPUT_POINT_Y, TEST_TOLERANCE);
    }

    void VerifyTwoLayerRectGeometry()
    {
        NG::RectF rect(INPUT_POINT_X, INPUT_POINT_Y, INPUT_RECT_WIDTH, INPUT_RECT_HEIGHT);
        NG::FrameNodeAdoptedCompensation::ApplyAdoptedCompensationToRect(childFrameNode_, rect);
        EXPECT_NEAR(rect.GetX(), EXPECTED_RECT_X, TEST_TOLERANCE);
        EXPECT_NEAR(rect.GetY(), EXPECTED_RECT_Y, TEST_TOLERANCE);
        EXPECT_NEAR(rect.Width(), EXPECTED_RECT_WIDTH, TEST_TOLERANCE);
        EXPECT_NEAR(rect.Height(), EXPECTED_RECT_HEIGHT, TEST_TOLERANCE);
        NG::PointF center(INPUT_POINT_X, INPUT_POINT_Y);
        float width = INPUT_RECT_WIDTH;
        float height = INPUT_RECT_HEIGHT;
        NG::FrameNodeAdoptedCompensation::ApplyAdoptedCompensationToCenterAndSize(
            childFrameNode_, center, width, height);
        EXPECT_NEAR(center.GetX(), EXPECTED_POINT_X, TEST_TOLERANCE);
        EXPECT_NEAR(center.GetY(), EXPECTED_POINT_Y, TEST_TOLERANCE);
        EXPECT_NEAR(width, EXPECTED_RECT_WIDTH, TEST_TOLERANCE);
        EXPECT_NEAR(height, EXPECTED_RECT_HEIGHT, TEST_TOLERANCE);
        auto fromWindow = childFrameNode_->ConvertPositionToWindow(NG::OffsetF(WINDOW_POINT, WINDOW_POINT), true);
        EXPECT_NEAR(fromWindow.GetX(), EXPECTED_WINDOW_X, TEST_TOLERANCE);
        EXPECT_NEAR(fromWindow.GetY(), EXPECTED_WINDOW_Y, TEST_TOLERANCE);
    }

    void VerifyLiveLayerAndDeveloperSeparation()
    {
        outerRenderNode_->SetBounds(
            UPDATED_OUTER_POSITION_X, UPDATED_OUTER_POSITION_Y, NODE_SIZE, NODE_SIZE);
        auto transformedOffset = NG::FrameNodeAdoptedCompensation::GetTransformedAdoptedCompensationOffset(
            childFrameNode_);
        EXPECT_NEAR(transformedOffset.GetX(), EXPECTED_UPDATED_OFFSET_X, TEST_TOLERANCE);
        EXPECT_NEAR(transformedOffset.GetY(), EXPECTED_UPDATED_OFFSET_Y, TEST_TOLERANCE);
        NG::FrameNodeAdoptedCompensation::SetAdoptedCompensation(childFrameNode_,
            NG::OffsetF(UPDATED_DEVELOPER_OFFSET_X, UPDATED_DEVELOPER_OFFSET_Y), Matrix4());
        EXPECT_EQ(NG::FrameNodeAdoptedCompensation::GetAdoptedCompensationOffset(childFrameNode_),
            NG::OffsetF(UPDATED_DEVELOPER_OFFSET_X, UPDATED_DEVELOPER_OFFSET_Y));
        EXPECT_TRUE(NG::FrameNodeAdoptedCompensation::GetAdoptedCompensationTransform(
            childFrameNode_).IsIdentityMatrix());
        NG::PointF developerSupportedPoint(INPUT_POINT_X, INPUT_POINT_Y);
        NG::FrameNodeAdoptedCompensation::ApplyAdoptedCompensationToPoint(
            childFrameNode_, developerSupportedPoint);
        EXPECT_EQ(developerSupportedPoint, NG::PointF(INPUT_POINT_X, INPUT_POINT_Y));
        auto scale = NG::FrameNodeAdoptedCompensation::GetAdoptedCompensationScale(childFrameNode_);
        EXPECT_FLOAT_EQ(scale.x, TWO_LAYER_SCALE);
        EXPECT_FLOAT_EQ(scale.y, TWO_LAYER_SCALE);
        EXPECT_TRUE(NG::FrameNodeAdoptedCompensation::HasAdoptedCompensationNonTranslationTransform(
            childFrameNode_));
        transformedOffset = NG::FrameNodeAdoptedCompensation::GetTransformedAdoptedCompensationOffset(
            childFrameNode_);
        EXPECT_NEAR(transformedOffset.GetX(), EXPECTED_UPDATED_OFFSET_X, TEST_TOLERANCE);
        EXPECT_NEAR(transformedOffset.GetY(), EXPECTED_UPDATED_OFFSET_Y, TEST_TOLERANCE);
    }

    void MountTranslationLayerAndSetCompensation()
    {
        outerRenderNode_ = Rosen::RSCanvasNode::Create();
        ASSERT_NE(outerRenderNode_, nullptr);
        outerRenderNode_->SetBounds(OUTER_POSITION_X, OUTER_POSITION_Y, NODE_SIZE, INNER_NODE_HEIGHT);
        outerRenderNode_->SetTranslate(LAYER_TRANSLATION_X, LAYER_TRANSLATION_Y, 0.0f);
        auto parentContext = AceType::DynamicCast<NG::RosenRenderContext>(parentNode_->GetRenderContext());
        ASSERT_NE(parentContext, nullptr);
        parentContext->InsertPureRenderChildAt(outerRenderNode_, -1);
        outerRenderNode_->AddChild(adoptedRenderNode_, -1);
        NG::FrameNodeAdoptedCompensation::SetAdoptedCompensation(childFrameNode_,
            NG::OffsetF(OUTER_POSITION_X, OUTER_POSITION_Y),
            Matrix4::CreateTranslate(COMBINED_TRANSLATION_X, COMBINED_TRANSLATION_Y, 0.0));
    }

    void VerifyTranslationSemantics()
    {
        EXPECT_FALSE(NG::FrameNodeAdoptedCompensation::HasAdoptedCompensationNonTranslationTransform(
            childFrameNode_));
        EXPECT_TRUE(NG::FrameNodeAdoptedCompensation::HasAdoptedCompensationPaintRectTransform(
            childFrameNode_));
        EXPECT_TRUE(NG::TextBase::HasRenderTransform(Referenced::Claim<NG::FrameNode>(childFrameNode_)));
        auto [translateOffset, translateError] =
            NG::FrameNodeAdoptedCompensation::GetAdoptedCompensationOffsetWithTranslate(childFrameNode_);
        EXPECT_EQ(translateOffset, NG::OffsetF(COMBINED_TRANSLATION_X, COMBINED_TRANSLATION_Y));
        EXPECT_FALSE(translateError);
        auto [globalTranslateOffset, globalTranslateError] =
            childFrameNode_->GetPaintRectGlobalOffsetWithTranslate(false, true);
        EXPECT_EQ(globalTranslateOffset, NG::OffsetF(COMBINED_TRANSLATION_X, COMBINED_TRANSLATION_Y));
        EXPECT_FALSE(globalTranslateError);
        auto fromWindow = childFrameNode_->ConvertPositionToWindow(NG::OffsetF(WINDOW_POINT, WINDOW_POINT), true);
        EXPECT_EQ(fromWindow, NG::OffsetF(EXPECTED_FROM_WINDOW_X, EXPECTED_FROM_WINDOW_Y));
    }

private:
    RefPtr<NG::FrameNode> parentNode_;
    RefPtr<NG::FrameNode> childNode_;
    NG::FrameNode* childFrameNode_ = nullptr;
    std::shared_ptr<Rosen::RSNode> outerRenderNode_;
    std::shared_ptr<Rosen::RSNode> innerRenderNode_;
    std::shared_ptr<Rosen::RSNode> adoptedRenderNode_;
};

/**
 * @tc.name: NativeRenderNodeAdopterTest016
 * @tc.desc: Test compensation requiring per-layer semantics follows the live pure-render RSNode parent chain.
 * @tc.type: FUNC
 */
HWTEST_F(NativeRenderNodeAdoptedCompensationTest, NativeRenderNodeAdopterTest016, TestSize.Level1)
{
    ASSERT_NO_FATAL_FAILURE(SetUpTwoLayerScenario());
    VerifyTwoLayerPointGeometry();
    VerifyTwoLayerRectGeometry();
    ASSERT_NO_FATAL_FAILURE(VerifyLiveLayerAndDeveloperSeparation());
}

/**
 * @tc.name: NativeRenderNodeAdopterTest017
 * @tc.desc: Test adopted translation and from-window calculations reproduce normal per-layer semantics.
 * @tc.type: FUNC
 */
HWTEST_F(NativeRenderNodeAdoptedCompensationTest, NativeRenderNodeAdopterTest017, TestSize.Level1)
{
    ASSERT_NO_FATAL_FAILURE(CreateAdoptedNodes());
    ASSERT_NO_FATAL_FAILURE(MountTranslationLayerAndSetCompensation());
    ASSERT_NO_FATAL_FAILURE(RebuildAndVerifyLayers(1));
    VerifyTranslationSemantics();
}

} // namespace OHOS::Ace

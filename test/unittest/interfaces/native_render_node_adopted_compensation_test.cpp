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

#include "native_interface.h"
#include "native_node.h"
#include "native_render.h"
#include "native_type.h"

#include "frameworks/core/components_ng/base/frame_node.h"
#include "frameworks/core/components_ng/base/frame_node_adopted_compensation.h"
#include "frameworks/core/components_ng/pattern/text/text_base.h"
#include "frameworks/core/components_ng/render/render_context.h"
#include "interfaces/native/node/node_model.h"
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
        if (identityMatrix_) {
            OH_ArkUI_Matrix4_Dispose(identityMatrix_);
        }
        if (matrix_) {
            OH_ArkUI_Matrix4_Dispose(matrix_);
        }
        DisposeRenderNode(adoptedRenderNode_);
        DisposeRenderNode(innerRenderNode_);
        DisposeRenderNode(outerRenderNode_);
    }

protected:
    void CreateAdoptedNodes()
    {
        nodeAPI_ = reinterpret_cast<ArkUI_NativeNodeAPI_1*>(
            OH_ArkUI_QueryModuleInterfaceByName(ARKUI_NATIVE_NODE, "ArkUI_NativeNodeAPI_1"));
        ASSERT_NE(nodeAPI_, nullptr);
        parentNode_ = nodeAPI_->createNode(ARKUI_NODE_CUSTOM);
        childNode_ = nodeAPI_->createNode(ARKUI_NODE_CUSTOM);
        ASSERT_NE(parentNode_, nullptr);
        ASSERT_NE(childNode_, nullptr);
        ASSERT_EQ(OH_ArkUI_NativeModule_AdoptChildWithPolicy(parentNode_, childNode_,
            OH_ARKUI_NATIVE_MODULE_ADOPT_POLICY_OVERLAY_POSITION_FOLLOWS_ADOPT_PARENT_CHAIN),
            ARKUI_ERROR_CODE_NO_ERROR);
        ASSERT_EQ(OH_ArkUI_NativeModule_SetChildMountPolicy(
            parentNode_, OH_ARKUI_NODE_MOUNT_POLICY_MIXED), ARKUI_ERROR_CODE_NO_ERROR);
        ASSERT_EQ(OH_ArkUI_RenderNodeUtils_GetRenderNode(childNode_, &adoptedRenderNode_),
            ARKUI_ERROR_CODE_NO_ERROR);
        ASSERT_NE(adoptedRenderNode_, nullptr);
        childFrameNode_ = reinterpret_cast<NG::FrameNode*>(childNode_->uiNodeHandle);
        ASSERT_NE(childFrameNode_, nullptr);
    }

    void MountTwoLayerTree()
    {
        outerRenderNode_ = OH_ArkUI_RenderNodeUtils_CreateNode();
        innerRenderNode_ = OH_ArkUI_RenderNodeUtils_CreateNode();
        ASSERT_NE(outerRenderNode_, nullptr);
        ASSERT_NE(innerRenderNode_, nullptr);
        ASSERT_EQ(OH_ArkUI_RenderNodeUtils_SetSize(outerRenderNode_, NODE_SIZE, NODE_SIZE),
            ARKUI_ERROR_CODE_NO_ERROR);
        ASSERT_EQ(OH_ArkUI_RenderNodeUtils_SetPosition(
            outerRenderNode_, OUTER_POSITION_X, OUTER_POSITION_Y), ARKUI_ERROR_CODE_NO_ERROR);
        ASSERT_EQ(OH_ArkUI_RenderNodeUtils_SetPivot(outerRenderNode_, HALF_PIVOT, HALF_PIVOT),
            ARKUI_ERROR_CODE_NO_ERROR);
        ASSERT_EQ(OH_ArkUI_RenderNodeUtils_SetScale(outerRenderNode_, TWO_LAYER_SCALE, TWO_LAYER_SCALE),
            ARKUI_ERROR_CODE_NO_ERROR);
        ASSERT_EQ(OH_ArkUI_RenderNodeUtils_SetSize(innerRenderNode_, NODE_SIZE, INNER_NODE_HEIGHT),
            ARKUI_ERROR_CODE_NO_ERROR);
        ASSERT_EQ(OH_ArkUI_RenderNodeUtils_SetPosition(
            innerRenderNode_, INNER_POSITION_X, INNER_POSITION_Y), ARKUI_ERROR_CODE_NO_ERROR);
        ASSERT_EQ(OH_ArkUI_RenderNodeUtils_SetPivot(innerRenderNode_, 0.0f, 0.0f),
            ARKUI_ERROR_CODE_NO_ERROR);
        ASSERT_EQ(OH_ArkUI_RenderNodeUtils_SetRotation(innerRenderNode_, 0.0f, 0.0f, INNER_ROTATION),
            ARKUI_ERROR_CODE_NO_ERROR);
        ASSERT_EQ(OH_ArkUI_RenderNodeUtils_AddRenderNode(parentNode_, outerRenderNode_),
            ARKUI_ERROR_CODE_NO_ERROR);
        ASSERT_EQ(OH_ArkUI_RenderNodeUtils_AddChild(outerRenderNode_, innerRenderNode_),
            ARKUI_ERROR_CODE_NO_ERROR);
        ASSERT_EQ(OH_ArkUI_RenderNodeUtils_AddChild(innerRenderNode_, adoptedRenderNode_),
            ARKUI_ERROR_CODE_NO_ERROR);
    }

    void SetTwoLayerCompensation()
    {
        const float compensationElements[] = {
            0.0f, TWO_LAYER_SCALE, 0.0f, 0.0f,
            -TWO_LAYER_SCALE, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            MATRIX_OFFSET_X, MATRIX_OFFSET_Y, 0.0f, 1.0f,
        };
        matrix_ = OH_ArkUI_Matrix4_CreateByElements(compensationElements);
        ASSERT_NE(matrix_, nullptr);
        ArkUI_PointF offset = { DEVELOPER_OFFSET_X, DEVELOPER_OFFSET_Y };
        ASSERT_EQ(OH_ArkUI_NativeModule_SetAdoptedChildOverlayPositionCompensation(
            childNode_, &offset, matrix_), ARKUI_ERROR_CODE_NO_ERROR);
    }

    void SetUpTwoLayerScenario()
    {
        ASSERT_NO_FATAL_FAILURE(CreateAdoptedNodes());
        ASSERT_NO_FATAL_FAILURE(MountTwoLayerTree());
        ASSERT_NO_FATAL_FAILURE(SetTwoLayerCompensation());
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
        ASSERT_EQ(OH_ArkUI_RenderNodeUtils_SetPosition(
            outerRenderNode_, UPDATED_OUTER_POSITION_X, UPDATED_OUTER_POSITION_Y), ARKUI_ERROR_CODE_NO_ERROR);
        auto transformedOffset = NG::FrameNodeAdoptedCompensation::GetTransformedAdoptedCompensationOffset(
            childFrameNode_);
        EXPECT_NEAR(transformedOffset.GetX(), EXPECTED_UPDATED_OFFSET_X, TEST_TOLERANCE);
        EXPECT_NEAR(transformedOffset.GetY(), EXPECTED_UPDATED_OFFSET_Y, TEST_TOLERANCE);
        identityMatrix_ = OH_ArkUI_Matrix4_CreateIdentity();
        ASSERT_NE(identityMatrix_, nullptr);
        ArkUI_PointF changedOffset = { UPDATED_DEVELOPER_OFFSET_X, UPDATED_DEVELOPER_OFFSET_Y };
        ASSERT_EQ(OH_ArkUI_NativeModule_SetAdoptedChildOverlayPositionCompensation(
            childNode_, &changedOffset, identityMatrix_), ARKUI_ERROR_CODE_NO_ERROR);
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
        outerRenderNode_ = OH_ArkUI_RenderNodeUtils_CreateNode();
        ASSERT_NE(outerRenderNode_, nullptr);
        ASSERT_EQ(OH_ArkUI_RenderNodeUtils_SetSize(outerRenderNode_, NODE_SIZE, INNER_NODE_HEIGHT),
            ARKUI_ERROR_CODE_NO_ERROR);
        ASSERT_EQ(OH_ArkUI_RenderNodeUtils_SetPosition(
            outerRenderNode_, OUTER_POSITION_X, OUTER_POSITION_Y), ARKUI_ERROR_CODE_NO_ERROR);
        ASSERT_EQ(OH_ArkUI_RenderNodeUtils_SetTranslation(
            outerRenderNode_, LAYER_TRANSLATION_X, LAYER_TRANSLATION_Y),
            ARKUI_ERROR_CODE_NO_ERROR);
        ASSERT_EQ(OH_ArkUI_RenderNodeUtils_AddRenderNode(parentNode_, outerRenderNode_),
            ARKUI_ERROR_CODE_NO_ERROR);
        ASSERT_EQ(OH_ArkUI_RenderNodeUtils_AddChild(outerRenderNode_, adoptedRenderNode_),
            ARKUI_ERROR_CODE_NO_ERROR);
        const float compensationElements[] = {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            COMBINED_TRANSLATION_X, COMBINED_TRANSLATION_Y, 0.0f, 1.0f,
        };
        matrix_ = OH_ArkUI_Matrix4_CreateByElements(compensationElements);
        ASSERT_NE(matrix_, nullptr);
        ArkUI_PointF offset = {
            static_cast<float>(OUTER_POSITION_X), static_cast<float>(OUTER_POSITION_Y) };
        ASSERT_EQ(OH_ArkUI_NativeModule_SetAdoptedChildOverlayPositionCompensation(
            childNode_, &offset, matrix_), ARKUI_ERROR_CODE_NO_ERROR);
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
    static void DisposeRenderNode(ArkUI_RenderNodeHandle& renderNode)
    {
        if (!renderNode) {
            return;
        }
        EXPECT_EQ(OH_ArkUI_RenderNodeUtils_DisposeNode(renderNode), ARKUI_ERROR_CODE_NO_ERROR);
        renderNode = nullptr;
    }

    ArkUI_NativeNodeAPI_1* nodeAPI_ = nullptr;
    ArkUI_NodeHandle parentNode_ = nullptr;
    ArkUI_NodeHandle childNode_ = nullptr;
    NG::FrameNode* childFrameNode_ = nullptr;
    ArkUI_RenderNodeHandle outerRenderNode_ = nullptr;
    ArkUI_RenderNodeHandle innerRenderNode_ = nullptr;
    ArkUI_RenderNodeHandle adoptedRenderNode_ = nullptr;
    ArkUI_Matrix4* matrix_ = nullptr;
    ArkUI_Matrix4* identityMatrix_ = nullptr;
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
    VerifyTranslationSemantics();
}

} // namespace OHOS::Ace

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

#include <cstdint>

#include "gtest/gtest.h"
#define private public
#define protected public
#include "interfaces/native/node/node_extened.h"
#include "interfaces/native/node/node_model.h"
#include "native_interface.h"
#include "native_node.h"
#include "native_type.h"
#include "test/mock/frameworks/base/thread/mock_task_executor.h"
#include "test/mock/frameworks/core/common/mock_container.h"
#include "test/mock/frameworks/core/common/mock_theme_manager.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"

#include "base/error/error_code.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/ui_node.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS::Ace::NG {

class NativeNodeThreePartTest : public testing::Test {
public:
    static void SetUpTestSuite()
    {
        MockPipelineContext::SetUp();
        MockContainer::SetUp();
        MockContainer::Current()->pipelineContext_ = MockPipelineContext::GetCurrent();
        MockContainer::Current()->taskExecutor_ = AceType::MakeRefPtr<MockTaskExecutor>();
        MockContainer::Current()->pipelineContext_->taskExecutor_ = MockContainer::Current()->taskExecutor_;
        auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
        PipelineBase::GetCurrentContext()->SetThemeManager(themeManager);
    }
    static void TearDownTestSuite()
    {
        MockPipelineContext::TearDown();
        MockContainer::TearDown();
    }
    void SetUp() {}
    void TearDown() {}
};

HWTEST_F(NativeNodeThreePartTest, LinearGradientBlur_SetAttributeNormal, TestSize.Level1)
{
    auto nodeAPI = reinterpret_cast<ArkUI_NativeNodeAPI_1*>(
        OH_ArkUI_QueryModuleInterfaceByName(ARKUI_NATIVE_NODE, "ArkUI_NativeNodeAPI_1"));
    ASSERT_NE(nodeAPI, nullptr);
    auto rootNode = nodeAPI->createNode(ARKUI_NODE_STACK);
    ASSERT_NE(rootNode, nullptr);

    OH_ArkUI_LinearGradientBlurOptions* options = OH_ArkUI_NativeModule_LinearGradientBlurOptions_Create();
    ASSERT_NE(options, nullptr);
    OH_ArkUI_NativeModule_LinearGradientBlurOptions_SetBlurRadius(options, 10.0f);
    float stops[] = { 0.0f, 0.0f, 0.5f, 0.8f, 1.0f, 1.0f };
    OH_ArkUI_NativeModule_LinearGradientBlurOptions_SetFractionStops(options, stops, 6);
    OH_ArkUI_NativeModule_LinearGradientBlurOptions_SetDirection(
        options, OH_ARKUI_LINEAR_GRADIENT_BLUR_DIRECTION_BOTTOM);

    ArkUI_AttributeItem item = { nullptr, 0 };
    item.object = reinterpret_cast<void*>(options);
    EXPECT_EQ(nodeAPI->setAttribute(rootNode, NODE_LINEAR_GRADIENT_BLUR, &item), ARKUI_ERROR_CODE_NO_ERROR);

    auto* frameNode = reinterpret_cast<FrameNode*>(rootNode->uiNodeHandle);
    ASSERT_NE(frameNode, nullptr);
    auto renderContext = frameNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    auto blurPara = renderContext->GetLinearGradientBlur();
    ASSERT_TRUE(blurPara.has_value());
    EXPECT_FLOAT_EQ(blurPara->blurRadius_.Value(), 10.0f);
    EXPECT_EQ(static_cast<int32_t>(blurPara->fractionStops_.size()), 3);
    EXPECT_FLOAT_EQ(blurPara->fractionStops_[0].first, 0.0f);
    EXPECT_FLOAT_EQ(blurPara->fractionStops_[0].second, 0.0f);
    EXPECT_FLOAT_EQ(blurPara->fractionStops_[1].first, 0.5f);
    EXPECT_FLOAT_EQ(blurPara->fractionStops_[1].second, 0.8f);
    EXPECT_FLOAT_EQ(blurPara->fractionStops_[2].first, 1.0f);
    EXPECT_FLOAT_EQ(blurPara->fractionStops_[2].second, 1.0f);
    EXPECT_EQ(blurPara->direction_, GradientDirection::BOTTOM);

    OH_ArkUI_NativeModule_LinearGradientBlurOptions_Destroy(options);
    nodeAPI->disposeNode(rootNode);
}

HWTEST_F(NativeNodeThreePartTest, LinearGradientBlur_ResetAttributeNormal, TestSize.Level1)
{
    auto nodeAPI = reinterpret_cast<ArkUI_NativeNodeAPI_1*>(
        OH_ArkUI_QueryModuleInterfaceByName(ARKUI_NATIVE_NODE, "ArkUI_NativeNodeAPI_1"));
    ASSERT_NE(nodeAPI, nullptr);
    auto rootNode = nodeAPI->createNode(ARKUI_NODE_STACK);
    ASSERT_NE(rootNode, nullptr);

    OH_ArkUI_LinearGradientBlurOptions* options = OH_ArkUI_NativeModule_LinearGradientBlurOptions_Create();
    ASSERT_NE(options, nullptr);
    OH_ArkUI_NativeModule_LinearGradientBlurOptions_SetBlurRadius(options, 10.0f);
    float setStops[] = { 0.0f, 0.0f, 1.0f, 1.0f };
    OH_ArkUI_NativeModule_LinearGradientBlurOptions_SetFractionStops(options, setStops, 4);
    OH_ArkUI_NativeModule_LinearGradientBlurOptions_SetDirection(options, OH_ARKUI_LINEAR_GRADIENT_BLUR_DIRECTION_LEFT);

    ArkUI_AttributeItem item = { nullptr, 0 };
    item.object = reinterpret_cast<void*>(options);
    EXPECT_EQ(nodeAPI->setAttribute(rootNode, NODE_LINEAR_GRADIENT_BLUR, &item), ARKUI_ERROR_CODE_NO_ERROR);
    OH_ArkUI_NativeModule_LinearGradientBlurOptions_Destroy(options);

    EXPECT_EQ(nodeAPI->resetAttribute(rootNode, NODE_LINEAR_GRADIENT_BLUR), ARKUI_ERROR_CODE_NO_ERROR);

    auto* frameNode = reinterpret_cast<FrameNode*>(rootNode->uiNodeHandle);
    ASSERT_NE(frameNode, nullptr);
    auto renderContext = frameNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    auto blurPara = renderContext->GetLinearGradientBlur();
    ASSERT_TRUE(blurPara.has_value());
    EXPECT_FLOAT_EQ(blurPara->blurRadius_.Value(), 0.0f);
    EXPECT_EQ(static_cast<int32_t>(blurPara->fractionStops_.size()), 2);
    EXPECT_FLOAT_EQ(blurPara->fractionStops_[0].first, 0.0f);
    EXPECT_FLOAT_EQ(blurPara->fractionStops_[0].second, 0.0f);
    EXPECT_FLOAT_EQ(blurPara->fractionStops_[1].first, 0.0f);
    EXPECT_FLOAT_EQ(blurPara->fractionStops_[1].second, 1.0f);
    EXPECT_EQ(blurPara->direction_, GradientDirection::BOTTOM);

    nodeAPI->disposeNode(rootNode);
}

HWTEST_F(NativeNodeThreePartTest, LinearGradientBlur_SetAttributeNullNode, TestSize.Level1)
{
    auto nodeAPI = reinterpret_cast<ArkUI_NativeNodeAPI_1*>(
        OH_ArkUI_QueryModuleInterfaceByName(ARKUI_NATIVE_NODE, "ArkUI_NativeNodeAPI_1"));
    ASSERT_NE(nodeAPI, nullptr);
    OH_ArkUI_LinearGradientBlurOptions* options = OH_ArkUI_NativeModule_LinearGradientBlurOptions_Create();
    ASSERT_NE(options, nullptr);
    OH_ArkUI_NativeModule_LinearGradientBlurOptions_SetBlurRadius(options, 5.0f);
    ArkUI_AttributeItem item = { nullptr, 0 };
    item.object = reinterpret_cast<void*>(options);
    EXPECT_EQ(nodeAPI->setAttribute(nullptr, NODE_LINEAR_GRADIENT_BLUR, &item), ARKUI_ERROR_CODE_PARAM_INVALID);
    OH_ArkUI_NativeModule_LinearGradientBlurOptions_Destroy(options);
}

HWTEST_F(NativeNodeThreePartTest, LinearGradientBlur_SetAttributeNullItem, TestSize.Level1)
{
    auto nodeAPI = reinterpret_cast<ArkUI_NativeNodeAPI_1*>(
        OH_ArkUI_QueryModuleInterfaceByName(ARKUI_NATIVE_NODE, "ArkUI_NativeNodeAPI_1"));
    ASSERT_NE(nodeAPI, nullptr);
    auto rootNode = nodeAPI->createNode(ARKUI_NODE_STACK);
    ASSERT_NE(rootNode, nullptr);
    EXPECT_EQ(nodeAPI->setAttribute(rootNode, NODE_LINEAR_GRADIENT_BLUR, nullptr), ARKUI_ERROR_CODE_PARAM_INVALID);
    nodeAPI->disposeNode(rootNode);
}

HWTEST_F(NativeNodeThreePartTest, LinearGradientBlur_SetAttributeNullObject, TestSize.Level1)
{
    auto nodeAPI = reinterpret_cast<ArkUI_NativeNodeAPI_1*>(
        OH_ArkUI_QueryModuleInterfaceByName(ARKUI_NATIVE_NODE, "ArkUI_NativeNodeAPI_1"));
    ASSERT_NE(nodeAPI, nullptr);
    auto rootNode = nodeAPI->createNode(ARKUI_NODE_STACK);
    ASSERT_NE(rootNode, nullptr);
    ArkUI_AttributeItem item = { nullptr, 0 };
    item.object = nullptr;
    EXPECT_EQ(nodeAPI->setAttribute(rootNode, NODE_LINEAR_GRADIENT_BLUR, &item), ARKUI_ERROR_CODE_PARAM_INVALID);
    nodeAPI->disposeNode(rootNode);
}

HWTEST_F(NativeNodeThreePartTest, LinearGradientBlur_MultipleFractionStops, TestSize.Level1)
{
    auto nodeAPI = reinterpret_cast<ArkUI_NativeNodeAPI_1*>(
        OH_ArkUI_QueryModuleInterfaceByName(ARKUI_NATIVE_NODE, "ArkUI_NativeNodeAPI_1"));
    ASSERT_NE(nodeAPI, nullptr);
    auto rootNode = nodeAPI->createNode(ARKUI_NODE_STACK);
    ASSERT_NE(rootNode, nullptr);

    OH_ArkUI_LinearGradientBlurOptions* options = OH_ArkUI_NativeModule_LinearGradientBlurOptions_Create();
    ASSERT_NE(options, nullptr);
    OH_ArkUI_NativeModule_LinearGradientBlurOptions_SetBlurRadius(options, 5.0f);
    float stops[] = { 0.0f, 0.1f, 0.25f, 0.3f, 0.5f, 0.6f, 0.75f, 0.8f, 1.0f, 1.0f };
    OH_ArkUI_NativeModule_LinearGradientBlurOptions_SetFractionStops(options, stops, 10);
    OH_ArkUI_NativeModule_LinearGradientBlurOptions_SetDirection(
        options, OH_ARKUI_LINEAR_GRADIENT_BLUR_DIRECTION_RIGHT);

    ArkUI_AttributeItem item = { nullptr, 0 };
    item.object = reinterpret_cast<void*>(options);
    EXPECT_EQ(nodeAPI->setAttribute(rootNode, NODE_LINEAR_GRADIENT_BLUR, &item), ARKUI_ERROR_CODE_NO_ERROR);

    auto* frameNode = reinterpret_cast<FrameNode*>(rootNode->uiNodeHandle);
    ASSERT_NE(frameNode, nullptr);
    auto renderContext = frameNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    auto blurPara = renderContext->GetLinearGradientBlur();
    ASSERT_TRUE(blurPara.has_value());
    EXPECT_EQ(static_cast<int32_t>(blurPara->fractionStops_.size()), 5);
    EXPECT_FLOAT_EQ(blurPara->fractionStops_[0].first, 0.0f);
    EXPECT_FLOAT_EQ(blurPara->fractionStops_[0].second, 0.1f);
    EXPECT_FLOAT_EQ(blurPara->fractionStops_[1].first, 0.25f);
    EXPECT_FLOAT_EQ(blurPara->fractionStops_[1].second, 0.3f);
    EXPECT_FLOAT_EQ(blurPara->fractionStops_[2].first, 0.5f);
    EXPECT_FLOAT_EQ(blurPara->fractionStops_[2].second, 0.6f);
    EXPECT_FLOAT_EQ(blurPara->fractionStops_[3].first, 0.75f);
    EXPECT_FLOAT_EQ(blurPara->fractionStops_[3].second, 0.8f);
    EXPECT_FLOAT_EQ(blurPara->fractionStops_[4].first, 1.0f);
    EXPECT_FLOAT_EQ(blurPara->fractionStops_[4].second, 1.0f);

    OH_ArkUI_NativeModule_LinearGradientBlurOptions_Destroy(options);
    nodeAPI->disposeNode(rootNode);
}

HWTEST_F(NativeNodeThreePartTest, LinearGradientBlur_AllDirections, TestSize.Level1)
{
    auto nodeAPI = reinterpret_cast<ArkUI_NativeNodeAPI_1*>(
        OH_ArkUI_QueryModuleInterfaceByName(ARKUI_NATIVE_NODE, "ArkUI_NativeNodeAPI_1"));
    ASSERT_NE(nodeAPI, nullptr);
    auto rootNode = nodeAPI->createNode(ARKUI_NODE_STACK);
    ASSERT_NE(rootNode, nullptr);

    OH_ArkUI_LinearGradientBlurDirection directions[] = {
        OH_ARKUI_LINEAR_GRADIENT_BLUR_DIRECTION_LEFT,
        OH_ARKUI_LINEAR_GRADIENT_BLUR_DIRECTION_TOP,
        OH_ARKUI_LINEAR_GRADIENT_BLUR_DIRECTION_RIGHT,
        OH_ARKUI_LINEAR_GRADIENT_BLUR_DIRECTION_BOTTOM,
        OH_ARKUI_LINEAR_GRADIENT_BLUR_DIRECTION_LEFT_TOP,
        OH_ARKUI_LINEAR_GRADIENT_BLUR_DIRECTION_LEFT_BOTTOM,
        OH_ARKUI_LINEAR_GRADIENT_BLUR_DIRECTION_RIGHT_TOP,
        OH_ARKUI_LINEAR_GRADIENT_BLUR_DIRECTION_RIGHT_BOTTOM,
    };
    GradientDirection expectedDirections[] = {
        GradientDirection::LEFT,
        GradientDirection::TOP,
        GradientDirection::RIGHT,
        GradientDirection::BOTTOM,
        GradientDirection::LEFT_TOP,
        GradientDirection::LEFT_BOTTOM,
        GradientDirection::RIGHT_TOP,
        GradientDirection::RIGHT_BOTTOM,
    };

    for (int32_t i = 0; i < 8; i++) {
        OH_ArkUI_LinearGradientBlurOptions* options = OH_ArkUI_NativeModule_LinearGradientBlurOptions_Create();
        ASSERT_NE(options, nullptr);
        OH_ArkUI_NativeModule_LinearGradientBlurOptions_SetBlurRadius(options, 1.0f);
        float stops[] = { 0.0f, 0.0f, 1.0f, 1.0f };
        OH_ArkUI_NativeModule_LinearGradientBlurOptions_SetFractionStops(options, stops, 4);
        EXPECT_EQ(OH_ArkUI_NativeModule_LinearGradientBlurOptions_SetDirection(options, directions[i]),
            ARKUI_ERROR_CODE_NO_ERROR);

        ArkUI_AttributeItem item = { nullptr, 0 };
        item.object = reinterpret_cast<void*>(options);
        EXPECT_EQ(nodeAPI->setAttribute(rootNode, NODE_LINEAR_GRADIENT_BLUR, &item), ARKUI_ERROR_CODE_NO_ERROR);

        auto* frameNode = reinterpret_cast<FrameNode*>(rootNode->uiNodeHandle);
        ASSERT_NE(frameNode, nullptr);
        auto renderContext = frameNode->GetRenderContext();
        ASSERT_NE(renderContext, nullptr);
        auto blurPara = renderContext->GetLinearGradientBlur();
        ASSERT_TRUE(blurPara.has_value());
        EXPECT_EQ(blurPara->direction_, expectedDirections[i]);

        OH_ArkUI_NativeModule_LinearGradientBlurOptions_Destroy(options);
    }

    nodeAPI->disposeNode(rootNode);
}

HWTEST_F(NativeNodeThreePartTest, LinearGradientBlur_RepeatSetAttribute, TestSize.Level1)
{
    auto nodeAPI = reinterpret_cast<ArkUI_NativeNodeAPI_1*>(
        OH_ArkUI_QueryModuleInterfaceByName(ARKUI_NATIVE_NODE, "ArkUI_NativeNodeAPI_1"));
    ASSERT_NE(nodeAPI, nullptr);
    auto rootNode = nodeAPI->createNode(ARKUI_NODE_STACK);
    ASSERT_NE(rootNode, nullptr);

    OH_ArkUI_LinearGradientBlurOptions* optionsA = OH_ArkUI_NativeModule_LinearGradientBlurOptions_Create();
    ASSERT_NE(optionsA, nullptr);
    OH_ArkUI_NativeModule_LinearGradientBlurOptions_SetBlurRadius(optionsA, 5.0f);
    float stopsA[] = { 0.0f, 0.0f, 1.0f, 1.0f };
    OH_ArkUI_NativeModule_LinearGradientBlurOptions_SetFractionStops(optionsA, stopsA, 4);
    OH_ArkUI_NativeModule_LinearGradientBlurOptions_SetDirection(
        optionsA, OH_ARKUI_LINEAR_GRADIENT_BLUR_DIRECTION_LEFT);

    ArkUI_AttributeItem itemA = { nullptr, 0 };
    itemA.object = reinterpret_cast<void*>(optionsA);
    EXPECT_EQ(nodeAPI->setAttribute(rootNode, NODE_LINEAR_GRADIENT_BLUR, &itemA), ARKUI_ERROR_CODE_NO_ERROR);
    OH_ArkUI_NativeModule_LinearGradientBlurOptions_Destroy(optionsA);

    OH_ArkUI_LinearGradientBlurOptions* optionsB = OH_ArkUI_NativeModule_LinearGradientBlurOptions_Create();
    ASSERT_NE(optionsB, nullptr);
    OH_ArkUI_NativeModule_LinearGradientBlurOptions_SetBlurRadius(optionsB, 20.0f);
    float stopsB[] = { 0.0f, 0.1f, 0.5f, 0.6f, 1.0f, 1.0f };
    OH_ArkUI_NativeModule_LinearGradientBlurOptions_SetFractionStops(optionsB, stopsB, 6);
    OH_ArkUI_NativeModule_LinearGradientBlurOptions_SetDirection(
        optionsB, OH_ARKUI_LINEAR_GRADIENT_BLUR_DIRECTION_RIGHT_TOP);

    ArkUI_AttributeItem itemB = { nullptr, 0 };
    itemB.object = reinterpret_cast<void*>(optionsB);
    EXPECT_EQ(nodeAPI->setAttribute(rootNode, NODE_LINEAR_GRADIENT_BLUR, &itemB), ARKUI_ERROR_CODE_NO_ERROR);

    auto* frameNode = reinterpret_cast<FrameNode*>(rootNode->uiNodeHandle);
    ASSERT_NE(frameNode, nullptr);
    auto renderContext = frameNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    auto blurPara = renderContext->GetLinearGradientBlur();
    ASSERT_TRUE(blurPara.has_value());
    EXPECT_FLOAT_EQ(blurPara->blurRadius_.Value(), 20.0f);
    EXPECT_EQ(static_cast<int32_t>(blurPara->fractionStops_.size()), 3);
    EXPECT_FLOAT_EQ(blurPara->fractionStops_[0].first, 0.0f);
    EXPECT_FLOAT_EQ(blurPara->fractionStops_[0].second, 0.1f);
    EXPECT_FLOAT_EQ(blurPara->fractionStops_[1].first, 0.5f);
    EXPECT_FLOAT_EQ(blurPara->fractionStops_[1].second, 0.6f);
    EXPECT_FLOAT_EQ(blurPara->fractionStops_[2].first, 1.0f);
    EXPECT_FLOAT_EQ(blurPara->fractionStops_[2].second, 1.0f);
    EXPECT_EQ(blurPara->direction_, GradientDirection::RIGHT_TOP);

    OH_ArkUI_NativeModule_LinearGradientBlurOptions_Destroy(optionsB);
    nodeAPI->disposeNode(rootNode);
}

HWTEST_F(NativeNodeThreePartTest, LinearGradientBlur_GetAfterSet, TestSize.Level1)
{
    auto nodeAPI = reinterpret_cast<ArkUI_NativeNodeAPI_1*>(
        OH_ArkUI_QueryModuleInterfaceByName(ARKUI_NATIVE_NODE, "ArkUI_NativeNodeAPI_1"));
    ASSERT_NE(nodeAPI, nullptr);
    auto rootNode = nodeAPI->createNode(ARKUI_NODE_STACK);
    ASSERT_NE(rootNode, nullptr);

    OH_ArkUI_LinearGradientBlurOptions* options = OH_ArkUI_NativeModule_LinearGradientBlurOptions_Create();
    ASSERT_NE(options, nullptr);
    OH_ArkUI_NativeModule_LinearGradientBlurOptions_SetBlurRadius(options, 12.0f);
    float stops[] = { 0.0f, 0.0f, 1.0f, 1.0f };
    OH_ArkUI_NativeModule_LinearGradientBlurOptions_SetFractionStops(options, stops, 4);
    OH_ArkUI_NativeModule_LinearGradientBlurOptions_SetDirection(
        options, OH_ARKUI_LINEAR_GRADIENT_BLUR_DIRECTION_LEFT_TOP);

    float getBlurRadius = 0.0f;
    EXPECT_EQ(OH_ArkUI_NativeModule_LinearGradientBlurOptions_GetBlurRadius(options, &getBlurRadius),
        ARKUI_ERROR_CODE_NO_ERROR);
    EXPECT_FLOAT_EQ(getBlurRadius, 12.0f);

    float getStops[4] = { 0 };
    int32_t writeLength = 0;
    EXPECT_EQ(OH_ArkUI_NativeModule_LinearGradientBlurOptions_GetFractionStops(options, getStops, 4, &writeLength),
        ARKUI_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(writeLength, 4);
    EXPECT_FLOAT_EQ(getStops[0], 0.0f);
    EXPECT_FLOAT_EQ(getStops[1], 0.0f);
    EXPECT_FLOAT_EQ(getStops[2], 1.0f);
    EXPECT_FLOAT_EQ(getStops[3], 1.0f);

    OH_ArkUI_LinearGradientBlurDirection getDirection = OH_ARKUI_LINEAR_GRADIENT_BLUR_DIRECTION_LEFT;
    EXPECT_EQ(OH_ArkUI_NativeModule_LinearGradientBlurOptions_GetDirection(options, &getDirection),
        ARKUI_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(getDirection, OH_ARKUI_LINEAR_GRADIENT_BLUR_DIRECTION_LEFT_TOP);

    OH_ArkUI_NativeModule_LinearGradientBlurOptions_Destroy(options);
    nodeAPI->disposeNode(rootNode);
}

HWTEST_F(NativeNodeThreePartTest, LinearGradientBlur_GetAttributeAfterSet, TestSize.Level1)
{
    auto nodeAPI = reinterpret_cast<ArkUI_NativeNodeAPI_1*>(
        OH_ArkUI_QueryModuleInterfaceByName(ARKUI_NATIVE_NODE, "ArkUI_NativeNodeAPI_1"));
    ASSERT_NE(nodeAPI, nullptr);
    auto rootNode = nodeAPI->createNode(ARKUI_NODE_STACK);
    ASSERT_NE(rootNode, nullptr);

    OH_ArkUI_LinearGradientBlurOptions* options = OH_ArkUI_NativeModule_LinearGradientBlurOptions_Create();
    ASSERT_NE(options, nullptr);
    OH_ArkUI_NativeModule_LinearGradientBlurOptions_SetBlurRadius(options, 12.0f);
    float stops[] = { 0.0f, 0.0f, 0.5f, 0.8f, 1.0f, 1.0f };
    OH_ArkUI_NativeModule_LinearGradientBlurOptions_SetFractionStops(options, stops, 6);
    OH_ArkUI_NativeModule_LinearGradientBlurOptions_SetDirection(
        options, OH_ARKUI_LINEAR_GRADIENT_BLUR_DIRECTION_LEFT_TOP);

    ArkUI_AttributeItem item = { nullptr, 0 };
    item.object = reinterpret_cast<void*>(options);
    EXPECT_EQ(nodeAPI->setAttribute(rootNode, NODE_LINEAR_GRADIENT_BLUR, &item), ARKUI_ERROR_CODE_NO_ERROR);
    OH_ArkUI_NativeModule_LinearGradientBlurOptions_Destroy(options);

    const ArkUI_AttributeItem* result = nodeAPI->getAttribute(rootNode, NODE_LINEAR_GRADIENT_BLUR);
    ASSERT_NE(result, nullptr);
    ASSERT_NE(result->object, nullptr);
    auto* opts = reinterpret_cast<OH_ArkUI_LinearGradientBlurOptions*>(result->object);
    float blurRadius = 0.0f;
    EXPECT_EQ(
        OH_ArkUI_NativeModule_LinearGradientBlurOptions_GetBlurRadius(opts, &blurRadius), ARKUI_ERROR_CODE_NO_ERROR);
    EXPECT_FLOAT_EQ(blurRadius, 12.0f);
    float getStops[6] = { 0 };
    int32_t writeLength = 0;
    EXPECT_EQ(OH_ArkUI_NativeModule_LinearGradientBlurOptions_GetFractionStops(opts, getStops, 6, &writeLength),
        ARKUI_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(writeLength, 6);
    EXPECT_FLOAT_EQ(getStops[0], 0.0f);
    EXPECT_FLOAT_EQ(getStops[1], 0.0f);
    EXPECT_FLOAT_EQ(getStops[2], 0.5f);
    EXPECT_FLOAT_EQ(getStops[3], 0.8f);
    EXPECT_FLOAT_EQ(getStops[4], 1.0f);
    EXPECT_FLOAT_EQ(getStops[5], 1.0f);
    OH_ArkUI_LinearGradientBlurDirection direction = OH_ARKUI_LINEAR_GRADIENT_BLUR_DIRECTION_LEFT;
    EXPECT_EQ(
        OH_ArkUI_NativeModule_LinearGradientBlurOptions_GetDirection(opts, &direction), ARKUI_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(direction, OH_ARKUI_LINEAR_GRADIENT_BLUR_DIRECTION_LEFT_TOP);

    nodeAPI->disposeNode(rootNode);
}

HWTEST_F(NativeNodeThreePartTest, LinearGradientBlur_GetAttributeNotSet, TestSize.Level1)
{
    auto nodeAPI = reinterpret_cast<ArkUI_NativeNodeAPI_1*>(
        OH_ArkUI_QueryModuleInterfaceByName(ARKUI_NATIVE_NODE, "ArkUI_NativeNodeAPI_1"));
    ASSERT_NE(nodeAPI, nullptr);
    auto rootNode = nodeAPI->createNode(ARKUI_NODE_STACK);
    ASSERT_NE(rootNode, nullptr);

    const ArkUI_AttributeItem* result = nodeAPI->getAttribute(rootNode, NODE_LINEAR_GRADIENT_BLUR);
    ASSERT_NE(result, nullptr);
    ASSERT_NE(result->object, nullptr);
    auto* opts = reinterpret_cast<OH_ArkUI_LinearGradientBlurOptions*>(result->object);
    float blurRadius = -1.0f;
    EXPECT_EQ(
        OH_ArkUI_NativeModule_LinearGradientBlurOptions_GetBlurRadius(opts, &blurRadius), ARKUI_ERROR_CODE_NO_ERROR);
    EXPECT_FLOAT_EQ(blurRadius, 0.0f);
    float getStops[4] = { 0 };
    int32_t writeLength = 0;
    EXPECT_EQ(OH_ArkUI_NativeModule_LinearGradientBlurOptions_GetFractionStops(opts, getStops, 4, &writeLength),
        ARKUI_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(writeLength, 4);
    EXPECT_FLOAT_EQ(getStops[0], 0.0f);
    EXPECT_FLOAT_EQ(getStops[1], 0.0f);
    EXPECT_FLOAT_EQ(getStops[2], 0.0f);
    EXPECT_FLOAT_EQ(getStops[3], 1.0f);
    OH_ArkUI_LinearGradientBlurDirection direction = OH_ARKUI_LINEAR_GRADIENT_BLUR_DIRECTION_LEFT;
    EXPECT_EQ(
        OH_ArkUI_NativeModule_LinearGradientBlurOptions_GetDirection(opts, &direction), ARKUI_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(direction, OH_ARKUI_LINEAR_GRADIENT_BLUR_DIRECTION_BOTTOM);

    nodeAPI->disposeNode(rootNode);
}

HWTEST_F(NativeNodeThreePartTest, LinearGradientBlur_GetAttributeAfterReset, TestSize.Level1)
{
    auto nodeAPI = reinterpret_cast<ArkUI_NativeNodeAPI_1*>(
        OH_ArkUI_QueryModuleInterfaceByName(ARKUI_NATIVE_NODE, "ArkUI_NativeNodeAPI_1"));
    ASSERT_NE(nodeAPI, nullptr);
    auto rootNode = nodeAPI->createNode(ARKUI_NODE_STACK);
    ASSERT_NE(rootNode, nullptr);

    OH_ArkUI_LinearGradientBlurOptions* options = OH_ArkUI_NativeModule_LinearGradientBlurOptions_Create();
    ASSERT_NE(options, nullptr);
    OH_ArkUI_NativeModule_LinearGradientBlurOptions_SetBlurRadius(options, 10.0f);
    float stops[] = { 0.0f, 0.0f, 1.0f, 1.0f };
    OH_ArkUI_NativeModule_LinearGradientBlurOptions_SetFractionStops(options, stops, 4);
    OH_ArkUI_NativeModule_LinearGradientBlurOptions_SetDirection(options, OH_ARKUI_LINEAR_GRADIENT_BLUR_DIRECTION_LEFT);

    ArkUI_AttributeItem item = { nullptr, 0 };
    item.object = reinterpret_cast<void*>(options);
    EXPECT_EQ(nodeAPI->setAttribute(rootNode, NODE_LINEAR_GRADIENT_BLUR, &item), ARKUI_ERROR_CODE_NO_ERROR);
    OH_ArkUI_NativeModule_LinearGradientBlurOptions_Destroy(options);

    EXPECT_EQ(nodeAPI->resetAttribute(rootNode, NODE_LINEAR_GRADIENT_BLUR), ARKUI_ERROR_CODE_NO_ERROR);

    const ArkUI_AttributeItem* result = nodeAPI->getAttribute(rootNode, NODE_LINEAR_GRADIENT_BLUR);
    ASSERT_NE(result, nullptr);
    ASSERT_NE(result->object, nullptr);
    auto* opts = reinterpret_cast<OH_ArkUI_LinearGradientBlurOptions*>(result->object);
    float blurRadius = -1.0f;
    EXPECT_EQ(
        OH_ArkUI_NativeModule_LinearGradientBlurOptions_GetBlurRadius(opts, &blurRadius), ARKUI_ERROR_CODE_NO_ERROR);
    EXPECT_FLOAT_EQ(blurRadius, 0.0f);
    float getStops[4] = { 0 };
    int32_t writeLength = 0;
    EXPECT_EQ(OH_ArkUI_NativeModule_LinearGradientBlurOptions_GetFractionStops(opts, getStops, 4, &writeLength),
        ARKUI_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(writeLength, 4);
    EXPECT_FLOAT_EQ(getStops[0], 0.0f);
    EXPECT_FLOAT_EQ(getStops[1], 0.0f);
    EXPECT_FLOAT_EQ(getStops[2], 0.0f);
    EXPECT_FLOAT_EQ(getStops[3], 1.0f);
    OH_ArkUI_LinearGradientBlurDirection direction = OH_ARKUI_LINEAR_GRADIENT_BLUR_DIRECTION_LEFT;
    EXPECT_EQ(
        OH_ArkUI_NativeModule_LinearGradientBlurOptions_GetDirection(opts, &direction), ARKUI_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(direction, OH_ARKUI_LINEAR_GRADIENT_BLUR_DIRECTION_BOTTOM);

    nodeAPI->disposeNode(rootNode);
}
} // namespace OHOS::Ace::NG

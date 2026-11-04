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

#include <cstddef>
#include <cstdint>
#include "gtest/gtest.h"
#define private public
#define protected public
#include "base/geometry/dimension.h"
#include "native_interface.h"
#include "native_node.h"
#include "native_type.h"
#include "event_converter.h"
#include "interfaces/inner_api/ace/node_module_inner.h"
#include "interfaces/native/node/node_extened.h"
#include "interfaces/native/node/node_model.h"
#include "interfaces/native/node/list_option.h"
#include "test/mock/frameworks/base/thread/mock_task_executor.h"
#include "test/mock/frameworks/core/common/mock_container.h"
#include "test/mock/frameworks/core/common/mock_theme_manager.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"
#include "frameworks/base/error/error_code.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace {

class NodeStyleModifierTest : public testing::Test {
public:
    static void SetUpTestSuite()
    {
        NG::MockPipelineContext::SetUp();
        MockContainer::SetUp();
        MockContainer::Current()->pipelineContext_ = NG::MockPipelineContext::GetCurrent();
        MockContainer::Current()->taskExecutor_ = AceType::MakeRefPtr<MockTaskExecutor>();
        MockContainer::Current()->pipelineContext_->taskExecutor_ = MockContainer::Current()->taskExecutor_;
        auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
        PipelineBase::GetCurrentContext()->SetThemeManager(themeManager);
    }
    static void TearDownTestSuite()
    {
        NG::MockPipelineContext::TearDown();
        MockContainer::TearDown();
    }
    void SetUp() {}
    void TearDown() {}
};

namespace {
struct GridFastScrollCall {
    int32_t legacyCalls = 0;
    int32_t optionsCalls = 0;
    int32_t conversionCalls = 0;
    int32_t index = 0;
    int32_t originUnit = -1;
    int32_t targetUnit = -1;
    ArkUINodeHandle node = nullptr;
    ArkUIScrollToIndexOptions options {};
};

GridFastScrollCall g_gridFastScrollCall;
ArkUIGridModifier g_gridFastScrollModifier {};
ArkUINodeModifiers g_gridFastScrollNodeModifiers {};
ArkUIBasicAPI g_gridFastScrollBasicApi {};

const ArkUIGridModifier* GetFastScrollGridModifier()
{
    return &g_gridFastScrollModifier;
}

const ArkUINodeModifiers* GetFastScrollNodeModifiers()
{
    return &g_gridFastScrollNodeModifiers;
}

const ArkUIBasicAPI* GetFastScrollBasicApi()
{
    return &g_gridFastScrollBasicApi;
}
} // namespace

class GridFastScrollNodeStyleTest : public NodeStyleModifierTest {
public:
    void SetUp() override
    {
        ASSERT_TRUE(NodeModel::InitialFullImpl());
        nodeApi_ = reinterpret_cast<ArkUI_NativeNodeAPI_1*>(
            OH_ArkUI_QueryModuleInterfaceByName(ARKUI_NATIVE_NODE, "ArkUI_NativeNodeAPI_1"));
        ASSERT_NE(nodeApi_, nullptr);
        node_ = nodeApi_->createNode(ARKUI_NODE_GRID);
        ASSERT_NE(node_, nullptr);
        fullImpl_ = NodeModel::GetFullImpl();
        ASSERT_NE(fullImpl_, nullptr);
        previousModifiers_ = fullImpl_->getNodeModifiers;
        previousBasicApi_ = fullImpl_->getBasicAPI;
        ASSERT_NE(previousModifiers_, nullptr);
        ASSERT_NE(previousBasicApi_, nullptr);
        ASSERT_NE(previousModifiers_(), nullptr);
        ASSERT_NE(previousBasicApi_(), nullptr);
        g_gridFastScrollNodeModifiers = *previousModifiers_();
        g_gridFastScrollBasicApi = *previousBasicApi_();
        g_gridFastScrollModifier = {};
        g_gridFastScrollModifier.setScrollToIndex = [](ArkUINodeHandle node, ArkUI_Int32 index,
            ArkUI_Int32 smooth, ArkUI_Int32 align, ArkUI_Float32 offset) {
            ++g_gridFastScrollCall.legacyCalls;
            g_gridFastScrollCall.node = node;
            g_gridFastScrollCall.index = index;
            g_gridFastScrollCall.options = { smooth != 0, align, offset, 0 };
        };
        g_gridFastScrollModifier.setScrollToIndexWithOptions = [](ArkUINodeHandle node, ArkUI_Int32 index,
            const ArkUIScrollToIndexOptions* options) {
            ASSERT_NE(options, nullptr);
            ++g_gridFastScrollCall.optionsCalls;
            g_gridFastScrollCall.node = node;
            g_gridFastScrollCall.index = index;
            g_gridFastScrollCall.options = *options;
        };
        g_gridFastScrollBasicApi.convertLengthMetricsUnit = [](double value, int32_t origin, int32_t target) {
            ++g_gridFastScrollCall.conversionCalls;
            g_gridFastScrollCall.originUnit = origin;
            g_gridFastScrollCall.targetUnit = target;
            constexpr double TEST_LENGTH_SCALE = 2.0;
            return value * TEST_LENGTH_SCALE;
        };
        g_gridFastScrollNodeModifiers.getGridModifier = GetFastScrollGridModifier;
        fullImpl_->getNodeModifiers = GetFastScrollNodeModifiers;
        fullImpl_->getBasicAPI = GetFastScrollBasicApi;
        g_gridFastScrollCall = {};
    }

    void TearDown() override
    {
        if (fullImpl_ && previousModifiers_ && previousBasicApi_) {
            fullImpl_->getNodeModifiers = previousModifiers_;
            fullImpl_->getBasicAPI = previousBasicApi_;
        }
        if (nodeApi_ && node_) {
            nodeApi_->disposeNode(node_);
        }
    }

protected:
    ArkUI_NativeNodeAPI_1* nodeApi_ = nullptr;
    ArkUI_NodeHandle node_ = nullptr;
    ArkUIFullNodeAPI* fullImpl_ = nullptr;
    decltype(ArkUIFullNodeAPI::getNodeModifiers) previousModifiers_ = nullptr;
    decltype(ArkUIFullNodeAPI::getBasicAPI) previousBasicApi_ = nullptr;
};

/**
 * @tc.name: FastScrollGridPublicOptions
 * @tc.desc: Public Grid setAttribute forwards modes 1/2 and converts the node unit exactly once.
 * @tc.type: FUNC
 */
HWTEST_F(GridFastScrollNodeStyleTest, FastScrollGridPublicOptions, TestSize.Level1)
{
    for (const auto unit : { ARKUI_LENGTH_METRIC_UNIT_VP, ARKUI_LENGTH_METRIC_UNIT_PX,
             ARKUI_LENGTH_METRIC_UNIT_FP }) {
        ASSERT_EQ(nodeApi_->setLengthMetricUnit(node_, unit), ERROR_CODE_NO_ERROR);
        for (int32_t mode : { 1, 2 }) {
            g_gridFastScrollCall = {};
            ArkUI_NumberValue values[] = { { .i32 = 90 }, { .i32 = 1 },
                { .i32 = ARKUI_SCROLL_ALIGNMENT_NONE }, { .f32 = -11.0f }, { .i32 = mode } };
            ArkUI_AttributeItem item { values, 5 };
            EXPECT_EQ(nodeApi_->setAttribute(node_, NODE_GRID_SCROLL_TO_INDEX, &item), ERROR_CODE_NO_ERROR);
            EXPECT_EQ(g_gridFastScrollCall.legacyCalls, 0);
            EXPECT_EQ(g_gridFastScrollCall.optionsCalls, 1);
            EXPECT_EQ(g_gridFastScrollCall.node, node_->uiNodeHandle);
            EXPECT_EQ(g_gridFastScrollCall.index, 90);
            EXPECT_TRUE(g_gridFastScrollCall.options.smooth);
            EXPECT_EQ(g_gridFastScrollCall.options.align, ARKUI_SCROLL_ALIGNMENT_AUTO);
            EXPECT_EQ(g_gridFastScrollCall.options.fastScrollMode, mode);
            const bool pixels = unit == ARKUI_LENGTH_METRIC_UNIT_PX;
            EXPECT_FLOAT_EQ(g_gridFastScrollCall.options.extraOffset, pixels ? -11.0f : -22.0f);
            EXPECT_EQ(g_gridFastScrollCall.conversionCalls, pixels ? 0 : 1);
            if (!pixels) {
                EXPECT_EQ(g_gridFastScrollCall.originUnit, static_cast<int32_t>(
                    unit == ARKUI_LENGTH_METRIC_UNIT_VP ? DimensionUnit::VP : DimensionUnit::FP));
                EXPECT_EQ(g_gridFastScrollCall.targetUnit, static_cast<int32_t>(DimensionUnit::PX));
            }
        }
    }
}

/**
 * @tc.name: FastScrollGridPublicLegacyIsolation
 * @tc.desc: Omitted/invalid mode preserves the old slot; non-animation preserves the raw offset.
 * @tc.type: FUNC
 */
HWTEST_F(GridFastScrollNodeStyleTest, FastScrollGridPublicLegacyIsolation, TestSize.Level1)
{
    ASSERT_EQ(nodeApi_->setLengthMetricUnit(node_, ARKUI_LENGTH_METRIC_UNIT_VP), ERROR_CODE_NO_ERROR);
    ArkUI_NumberValue values[] = { { .i32 = 90 }, { .i32 = 1 },
        { .i32 = ARKUI_SCROLL_ALIGNMENT_AUTO }, { .f32 = -11.0f }, { .i32 = 2 } };
    ArkUI_AttributeItem item { values, 5 };
    ASSERT_EQ(nodeApi_->setAttribute(node_, NODE_GRID_SCROLL_TO_INDEX, &item), ERROR_CODE_NO_ERROR);
    EXPECT_EQ(g_gridFastScrollCall.optionsCalls, 1);
    for (int32_t mode : { 0, -1, 3 }) {
        g_gridFastScrollCall = {};
        values[4].i32 = mode;
        EXPECT_EQ(nodeApi_->setAttribute(node_, NODE_GRID_SCROLL_TO_INDEX, &item), ERROR_CODE_NO_ERROR);
        EXPECT_EQ(g_gridFastScrollCall.legacyCalls, 1);
        EXPECT_EQ(g_gridFastScrollCall.optionsCalls, 0);
        EXPECT_EQ(g_gridFastScrollCall.conversionCalls, 0);
        EXPECT_FLOAT_EQ(g_gridFastScrollCall.options.extraOffset, -11.0f);
    }
    g_gridFastScrollCall = {};
    item.size = 4;
    EXPECT_EQ(nodeApi_->setAttribute(node_, NODE_GRID_SCROLL_TO_INDEX, &item), ERROR_CODE_NO_ERROR);
    EXPECT_EQ(g_gridFastScrollCall.legacyCalls, 1);
    EXPECT_EQ(g_gridFastScrollCall.optionsCalls, 0);
    item.size = 5;
    values[1].i32 = 0;
    for (int32_t mode : { 1, 2 }) {
        g_gridFastScrollCall = {};
        values[4].i32 = mode;
        EXPECT_EQ(nodeApi_->setAttribute(node_, NODE_GRID_SCROLL_TO_INDEX, &item), ERROR_CODE_NO_ERROR);
        EXPECT_EQ(g_gridFastScrollCall.optionsCalls, 1);
        EXPECT_FALSE(g_gridFastScrollCall.options.smooth);
        EXPECT_EQ(g_gridFastScrollCall.conversionCalls, 0);
        EXPECT_FLOAT_EQ(g_gridFastScrollCall.options.extraOffset, -11.0f);
    }
}

/**
 * @tc.name: FastScrollGridPublicInvalidItem
 * @tc.desc: Null and empty attribute items keep the public parameter error and never dispatch.
 * @tc.type: FUNC
 */
HWTEST_F(GridFastScrollNodeStyleTest, FastScrollGridPublicInvalidItem, TestSize.Level1)
{
    EXPECT_EQ(nodeApi_->setAttribute(node_, NODE_GRID_SCROLL_TO_INDEX, nullptr), ERROR_CODE_PARAM_INVALID);
    ArkUI_AttributeItem empty {};
    EXPECT_EQ(nodeApi_->setAttribute(node_, NODE_GRID_SCROLL_TO_INDEX, &empty), ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(g_gridFastScrollCall.legacyCalls, 0);
    EXPECT_EQ(g_gridFastScrollCall.optionsCalls, 0);
}

/**
 * @tc.name: SetDashGapTest001
 * @tc.desc: Test SetDashGap with nullptr item parameter.
 * @tc.type: FUNC
 */
HWTEST_F(NodeStyleModifierTest, SetDashGapTest001, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    auto nodeAPI = reinterpret_cast<ArkUI_NativeNodeAPI_1*>(
        OH_ArkUI_QueryModuleInterfaceByName(ARKUI_NATIVE_NODE, "ArkUI_NativeNodeAPI_1"));
    ASSERT_NE(nodeAPI, nullptr);
    auto rootNode = nodeAPI->createNode(ARKUI_NODE_STACK);
    ASSERT_NE(rootNode, nullptr);

    int32_t result = nodeAPI->setAttribute(rootNode, NODE_DASH_GAP, nullptr);
    EXPECT_EQ(result, ERROR_CODE_PARAM_INVALID);

    nodeAPI->disposeNode(rootNode);
}

/**
 * @tc.name: SetDashGapTest002
 * @tc.desc: Test SetDashGap with item size equals 0.
 * @tc.type: FUNC
 */
HWTEST_F(NodeStyleModifierTest, SetDashGapTest002, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    auto nodeAPI = reinterpret_cast<ArkUI_NativeNodeAPI_1*>(
        OH_ArkUI_QueryModuleInterfaceByName(ARKUI_NATIVE_NODE, "ArkUI_NativeNodeAPI_1"));
    ASSERT_NE(nodeAPI, nullptr);
    auto rootNode = nodeAPI->createNode(ARKUI_NODE_STACK);
    ASSERT_NE(rootNode, nullptr);

    ArkUI_NumberValue value[] = { { .f32 = 10.0f } };
    ArkUI_AttributeItem item = { value, 0 };
    int32_t result = nodeAPI->setAttribute(rootNode, NODE_DASH_GAP, &item);
    EXPECT_EQ(result, ERROR_CODE_PARAM_INVALID);

    nodeAPI->disposeNode(rootNode);
}

/**
 * @tc.name: SetDashGapTest003
 * @tc.desc: Test SetDashGap with item size not equals 4.
 * @tc.type: FUNC
 */
HWTEST_F(NodeStyleModifierTest, SetDashGapTest003, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    auto nodeAPI = reinterpret_cast<ArkUI_NativeNodeAPI_1*>(
        OH_ArkUI_QueryModuleInterfaceByName(ARKUI_NATIVE_NODE, "ArkUI_NativeNodeAPI_1"));
    ASSERT_NE(nodeAPI, nullptr);
    auto rootNode = nodeAPI->createNode(ARKUI_NODE_STACK);
    ASSERT_NE(rootNode, nullptr);

    ArkUI_NumberValue value1[] = { { .f32 = 10.0f } };
    ArkUI_AttributeItem item1 = { value1, 1 };
    int32_t result = nodeAPI->setAttribute(rootNode, NODE_DASH_GAP, &item1);
    EXPECT_EQ(result, ERROR_CODE_PARAM_INVALID);

    ArkUI_NumberValue value2[] = { { .f32 = 10.0f }, { .f32 = 10.0f } };
    ArkUI_AttributeItem item2 = { value2, 2 };
    result = nodeAPI->setAttribute(rootNode, NODE_DASH_GAP, &item2);
    EXPECT_EQ(result, ERROR_CODE_PARAM_INVALID);

    ArkUI_NumberValue value3[] = { { .f32 = 10.0f }, { .f32 = 10.0f }, { .f32 = 10.0f } };
    ArkUI_AttributeItem item3 = { value3, 3 };
    result = nodeAPI->setAttribute(rootNode, NODE_DASH_GAP, &item3);
    EXPECT_EQ(result, ERROR_CODE_PARAM_INVALID);

    ArkUI_NumberValue value5[] = { { .f32 = 10.0f }, { .f32 = 10.0f }, { .f32 = 10.0f }, { .f32 = 10.0f },
        { .f32 = 10.0f } };
    ArkUI_AttributeItem item5 = { value5, 5 };
    result = nodeAPI->setAttribute(rootNode, NODE_DASH_GAP, &item5);
    EXPECT_EQ(result, ERROR_CODE_PARAM_INVALID);

    nodeAPI->disposeNode(rootNode);
}

/**
 * @tc.name: SetDashWidthTest001
 * @tc.desc: Test SetDashWidth with nullptr item parameter.
 * @tc.type: FUNC
 */
HWTEST_F(NodeStyleModifierTest, SetDashWidthTest001, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    auto nodeAPI = reinterpret_cast<ArkUI_NativeNodeAPI_1*>(
        OH_ArkUI_QueryModuleInterfaceByName(ARKUI_NATIVE_NODE, "ArkUI_NativeNodeAPI_1"));
    ASSERT_NE(nodeAPI, nullptr);
    auto rootNode = nodeAPI->createNode(ARKUI_NODE_STACK);
    ASSERT_NE(rootNode, nullptr);

    int32_t result = nodeAPI->setAttribute(rootNode, NODE_DASH_WIDTH, nullptr);
    EXPECT_EQ(result, ERROR_CODE_PARAM_INVALID);

    nodeAPI->disposeNode(rootNode);
}

/**
 * @tc.name: SetDashWidthTest002
 * @tc.desc: Test SetDashWidth with item size equals 0.
 * @tc.type: FUNC
 */
HWTEST_F(NodeStyleModifierTest, SetDashWidthTest002, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    auto nodeAPI = reinterpret_cast<ArkUI_NativeNodeAPI_1*>(
        OH_ArkUI_QueryModuleInterfaceByName(ARKUI_NATIVE_NODE, "ArkUI_NativeNodeAPI_1"));
    ASSERT_NE(nodeAPI, nullptr);
    auto rootNode = nodeAPI->createNode(ARKUI_NODE_STACK);
    ASSERT_NE(rootNode, nullptr);

    ArkUI_NumberValue value[] = { { .f32 = 10.0f } };
    ArkUI_AttributeItem item = { value, 0 };
    int32_t result = nodeAPI->setAttribute(rootNode, NODE_DASH_WIDTH, &item);
    EXPECT_EQ(result, ERROR_CODE_PARAM_INVALID);

    nodeAPI->disposeNode(rootNode);
}

/**
 * @tc.name: SetDashWidthTest003
 * @tc.desc: Test SetDashWidth with item size not equals 4.
 * @tc.type: FUNC
 */
HWTEST_F(NodeStyleModifierTest, SetDashWidthTest003, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    auto nodeAPI = reinterpret_cast<ArkUI_NativeNodeAPI_1*>(
        OH_ArkUI_QueryModuleInterfaceByName(ARKUI_NATIVE_NODE, "ArkUI_NativeNodeAPI_1"));
    ASSERT_NE(nodeAPI, nullptr);
    auto rootNode = nodeAPI->createNode(ARKUI_NODE_STACK);
    ASSERT_NE(rootNode, nullptr);

    ArkUI_NumberValue value1[] = { { .f32 = 10.0f } };
    ArkUI_AttributeItem item1 = { value1, 1 };
    int32_t result = nodeAPI->setAttribute(rootNode, NODE_DASH_WIDTH, &item1);
    EXPECT_EQ(result, ERROR_CODE_PARAM_INVALID);

    ArkUI_NumberValue value2[] = { { .f32 = 10.0f }, { .f32 = 10.0f } };
    ArkUI_AttributeItem item2 = { value2, 2 };
    result = nodeAPI->setAttribute(rootNode, NODE_DASH_WIDTH, &item2);
    EXPECT_EQ(result, ERROR_CODE_PARAM_INVALID);

    ArkUI_NumberValue value3[] = { { .f32 = 10.0f }, { .f32 = 10.0f }, { .f32 = 10.0f } };
    ArkUI_AttributeItem item3 = { value3, 3 };
    result = nodeAPI->setAttribute(rootNode, NODE_DASH_WIDTH, &item3);
    EXPECT_EQ(result, ERROR_CODE_PARAM_INVALID);

    ArkUI_NumberValue value5[] = { { .f32 = 10.0f }, { .f32 = 10.0f }, { .f32 = 10.0f }, { .f32 = 10.0f },
        { .f32 = 10.0f } };
    ArkUI_AttributeItem item5 = { value5, 5 };
    result = nodeAPI->setAttribute(rootNode, NODE_DASH_WIDTH, &item5);
    EXPECT_EQ(result, ERROR_CODE_PARAM_INVALID);

    nodeAPI->disposeNode(rootNode);
}

/**
 * @tc.name: SetChainWeightTest001
 * @tc.desc: Test SetChainWeight with nullptr item parameter.
 * @tc.type: FUNC
 */
HWTEST_F(NodeStyleModifierTest, SetChainWeightTest001, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    auto nodeAPI = reinterpret_cast<ArkUI_NativeNodeAPI_1*>(
        OH_ArkUI_QueryModuleInterfaceByName(ARKUI_NATIVE_NODE, "ArkUI_NativeNodeAPI_1"));
    ASSERT_NE(nodeAPI, nullptr);
    auto rootNode = nodeAPI->createNode(ARKUI_NODE_STACK);
    ASSERT_NE(rootNode, nullptr);

    int32_t result = nodeAPI->setAttribute(rootNode, NODE_CHAIN_WEIGHT, nullptr);
    EXPECT_EQ(result, ERROR_CODE_PARAM_INVALID);

    nodeAPI->disposeNode(rootNode);
}

/**
 * @tc.name: SetChainWeightTest002
 * @tc.desc: Test SetChainWeight with item size equals 0.
 * @tc.type: FUNC
 */
HWTEST_F(NodeStyleModifierTest, SetChainWeightTest002, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    auto nodeAPI = reinterpret_cast<ArkUI_NativeNodeAPI_1*>(
        OH_ArkUI_QueryModuleInterfaceByName(ARKUI_NATIVE_NODE, "ArkUI_NativeNodeAPI_1"));
    ASSERT_NE(nodeAPI, nullptr);
    auto rootNode = nodeAPI->createNode(ARKUI_NODE_STACK);
    ASSERT_NE(rootNode, nullptr);

    ArkUI_NumberValue value[] = { { .f32 = 0.5f } };
    ArkUI_AttributeItem item = { value, 0 };
    int32_t result = nodeAPI->setAttribute(rootNode, NODE_CHAIN_WEIGHT, &item);
    EXPECT_EQ(result, ERROR_CODE_PARAM_INVALID);

    nodeAPI->disposeNode(rootNode);
}

/**
 * @tc.name: SetChainWeightTest003
 * @tc.desc: Test SetChainWeight with item size greater than 2.
 * @tc.type: FUNC
 */
HWTEST_F(NodeStyleModifierTest, SetChainWeightTest003, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    auto nodeAPI = reinterpret_cast<ArkUI_NativeNodeAPI_1*>(
        OH_ArkUI_QueryModuleInterfaceByName(ARKUI_NATIVE_NODE, "ArkUI_NativeNodeAPI_1"));
    ASSERT_NE(nodeAPI, nullptr);
    auto rootNode = nodeAPI->createNode(ARKUI_NODE_STACK);
    ASSERT_NE(rootNode, nullptr);

    ArkUI_NumberValue value3[] = { { .f32 = 0.5f }, { .f32 = 0.5f }, { .f32 = 0.5f } };
    ArkUI_AttributeItem item3 = { value3, 3 };
    int32_t result = nodeAPI->setAttribute(rootNode, NODE_CHAIN_WEIGHT, &item3);
    EXPECT_EQ(result, ERROR_CODE_PARAM_INVALID);

    nodeAPI->disposeNode(rootNode);
}

/**
 * @tc.name: ResetDashGapTest001
 * @tc.desc: Test ResetDashGap with nullptr node.
 * @tc.type: FUNC
 */
HWTEST_F(NodeStyleModifierTest, ResetDashGapTest001, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    auto nodeAPI = reinterpret_cast<ArkUI_NativeNodeAPI_1*>(
        OH_ArkUI_QueryModuleInterfaceByName(ARKUI_NATIVE_NODE, "ArkUI_NativeNodeAPI_1"));
    ASSERT_NE(nodeAPI, nullptr);

    int32_t result = nodeAPI->resetAttribute(nullptr, NODE_DASH_GAP);
    EXPECT_EQ(result, ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: ResetDashWidthTest001
 * @tc.desc: Test ResetDashWidth with nullptr node.
 * @tc.type: FUNC
 */
HWTEST_F(NodeStyleModifierTest, ResetDashWidthTest001, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    auto nodeAPI = reinterpret_cast<ArkUI_NativeNodeAPI_1*>(
        OH_ArkUI_QueryModuleInterfaceByName(ARKUI_NATIVE_NODE, "ArkUI_NativeNodeAPI_1"));
    ASSERT_NE(nodeAPI, nullptr);

    int32_t result = nodeAPI->resetAttribute(nullptr, NODE_DASH_WIDTH);
    EXPECT_EQ(result, ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: ResetChainWeightTest001
 * @tc.desc: Test ResetChainWeight with nullptr node.
 * @tc.type: FUNC
 */
HWTEST_F(NodeStyleModifierTest, ResetChainWeightTest001, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    auto nodeAPI = reinterpret_cast<ArkUI_NativeNodeAPI_1*>(
        OH_ArkUI_QueryModuleInterfaceByName(ARKUI_NATIVE_NODE, "ArkUI_NativeNodeAPI_1"));
    ASSERT_NE(nodeAPI, nullptr);

    int32_t result = nodeAPI->resetAttribute(nullptr, NODE_CHAIN_WEIGHT);
    EXPECT_EQ(result, ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: GetFocusBoxTest001
 * @tc.desc: Test GetFocusBox after setting focus box style.
 * @tc.type: FUNC
 */
HWTEST_F(NodeStyleModifierTest, GetFocusBoxTest001, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    auto nodeAPI = reinterpret_cast<ArkUI_NativeNodeAPI_1*>(
        OH_ArkUI_QueryModuleInterfaceByName(ARKUI_NATIVE_NODE, "ArkUI_NativeNodeAPI_1"));
    ASSERT_NE(nodeAPI, nullptr);
    auto rootNode = nodeAPI->createNode(ARKUI_NODE_STACK);
    ASSERT_NE(rootNode, nullptr);

    constexpr float margin = 2.0f;
    constexpr float width = 3.0f;
    constexpr uint32_t color = 0xff112233;
    ArkUI_NumberValue value[] = { { .f32 = margin }, { .f32 = width }, { .u32 = color } };
    ArkUI_AttributeItem item = { value, 3 };
    int32_t result = nodeAPI->setAttribute(rootNode, NODE_FOCUS_BOX, &item);
    ASSERT_EQ(result, ERROR_CODE_NO_ERROR);

    auto getItem = nodeAPI->getAttribute(rootNode, NODE_FOCUS_BOX);
    ASSERT_NE(getItem, nullptr);
    ASSERT_NE(getItem->value, nullptr);
    ASSERT_EQ(getItem->size, 3);
    EXPECT_FLOAT_EQ(getItem->value[0].f32, margin);
    EXPECT_FLOAT_EQ(getItem->value[1].f32, width);
    EXPECT_EQ(getItem->value[2].u32, color);

    nodeAPI->disposeNode(rootNode);
}

/**
 * @tc.name: GetFocusBoxTest002
 * @tc.desc: Test GetFocusBox when focus box style is not set, should return default values.
 * @tc.type: FUNC
 */
HWTEST_F(NodeStyleModifierTest, GetFocusBoxTest002, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    auto nodeAPI = reinterpret_cast<ArkUI_NativeNodeAPI_1*>(
        OH_ArkUI_QueryModuleInterfaceByName(ARKUI_NATIVE_NODE, "ArkUI_NativeNodeAPI_1"));
    ASSERT_NE(nodeAPI, nullptr);
    auto rootNode = nodeAPI->createNode(ARKUI_NODE_STACK);
    ASSERT_NE(rootNode, nullptr);

    auto getItem = nodeAPI->getAttribute(rootNode, NODE_FOCUS_BOX);
    ASSERT_NE(getItem, nullptr);
    ASSERT_NE(getItem->value, nullptr);
    ASSERT_EQ(getItem->size, 3);

    nodeAPI->disposeNode(rootNode);
}

/**
 * @tc.name: GetFocusBoxTest003
 * @tc.desc: Test GetFocusBox when only partial fields are set, should return set values for set fields
 *           and default values for unset fields.
 * @tc.type: FUNC
 */
HWTEST_F(NodeStyleModifierTest, GetFocusBoxTest003, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    auto nodeAPI = reinterpret_cast<ArkUI_NativeNodeAPI_1*>(
        OH_ArkUI_QueryModuleInterfaceByName(ARKUI_NATIVE_NODE, "ArkUI_NativeNodeAPI_1"));
    ASSERT_NE(nodeAPI, nullptr);
    auto rootNode = nodeAPI->createNode(ARKUI_NODE_STACK);
    ASSERT_NE(rootNode, nullptr);

    constexpr float margin = 5.0f;
    constexpr uint32_t color = 0xff445566;
    ArkUI_NumberValue value[] = { { .f32 = margin }, { .f32 = FLT_MAX }, { .u32 = color } };
    ArkUI_AttributeItem item = { value, 3 };
    int32_t result = nodeAPI->setAttribute(rootNode, NODE_FOCUS_BOX, &item);
    ASSERT_EQ(result, ERROR_CODE_NO_ERROR);

    auto getItem = nodeAPI->getAttribute(rootNode, NODE_FOCUS_BOX);
    ASSERT_NE(getItem, nullptr);
    ASSERT_NE(getItem->value, nullptr);
    ASSERT_EQ(getItem->size, 3);
    EXPECT_FLOAT_EQ(getItem->value[0].f32, margin);
    EXPECT_EQ(getItem->value[2].u32, color);

    nodeAPI->disposeNode(rootNode);
}

/**
 * @tc.name: GetNextFocusTest001
 * @tc.desc: Test GetNextFocus after setting next focus node.
 * @tc.type: FUNC
 */
HWTEST_F(NodeStyleModifierTest, GetNextFocusTest001, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    auto nodeAPI = reinterpret_cast<ArkUI_NativeNodeAPI_1*>(
        OH_ArkUI_QueryModuleInterfaceByName(ARKUI_NATIVE_NODE, "ArkUI_NativeNodeAPI_1"));
    ASSERT_NE(nodeAPI, nullptr);
    auto rootNode = nodeAPI->createNode(ARKUI_NODE_STACK);
    ASSERT_NE(rootNode, nullptr);
    auto focusNode = nodeAPI->createNode(ARKUI_NODE_STACK);
    ASSERT_NE(focusNode, nullptr);

    ArkUI_NumberValue value[] = { { .i32 = ARKUI_FOCUS_MOVE_FORWARD } };
    ArkUI_AttributeItem item = { value, 1, nullptr, focusNode };
    int32_t result = nodeAPI->setAttribute(rootNode, NODE_NEXT_FOCUS, &item);
    ASSERT_EQ(result, ERROR_CODE_NO_ERROR);

    auto getItem = nodeAPI->getAttribute(rootNode, NODE_NEXT_FOCUS);
    ASSERT_NE(getItem, nullptr);
    ASSERT_NE(getItem->value, nullptr);
    ASSERT_NE(getItem->object, nullptr);
    ASSERT_EQ(getItem->size, 1);
    auto nextFocusNodes = reinterpret_cast<ArkUI_NodeHandle*>(getItem->object);
    EXPECT_EQ(getItem->value[0].i32, ARKUI_FOCUS_MOVE_FORWARD);
    EXPECT_EQ(nextFocusNodes[0], focusNode);

    nodeAPI->disposeNode(focusNode);
    nodeAPI->disposeNode(rootNode);
}

/**
 * @tc.name: GetNextFocusTest002
 * @tc.desc: Test GetNextFocus when no next focus is set.
 * @tc.type: FUNC
 */
HWTEST_F(NodeStyleModifierTest, GetNextFocusTest002, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    auto nodeAPI = reinterpret_cast<ArkUI_NativeNodeAPI_1*>(
        OH_ArkUI_QueryModuleInterfaceByName(ARKUI_NATIVE_NODE, "ArkUI_NativeNodeAPI_1"));
    ASSERT_NE(nodeAPI, nullptr);
    auto rootNode = nodeAPI->createNode(ARKUI_NODE_STACK);
    ASSERT_NE(rootNode, nullptr);

    auto getItem = nodeAPI->getAttribute(rootNode, NODE_NEXT_FOCUS);
    EXPECT_EQ(getItem, nullptr);

    nodeAPI->disposeNode(rootNode);
}

/**
 * @tc.name: GetNextFocusTest003
 * @tc.desc: Test GetNextFocus with multiple directions set.
 * @tc.type: FUNC
 */
HWTEST_F(NodeStyleModifierTest, GetNextFocusTest003, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    auto nodeAPI = reinterpret_cast<ArkUI_NativeNodeAPI_1*>(
        OH_ArkUI_QueryModuleInterfaceByName(ARKUI_NATIVE_NODE, "ArkUI_NativeNodeAPI_1"));
    ASSERT_NE(nodeAPI, nullptr);
    auto rootNode = nodeAPI->createNode(ARKUI_NODE_STACK);
    ASSERT_NE(rootNode, nullptr);
    auto forwardNode = nodeAPI->createNode(ARKUI_NODE_STACK);
    ASSERT_NE(forwardNode, nullptr);
    auto upNode = nodeAPI->createNode(ARKUI_NODE_STACK);
    ASSERT_NE(upNode, nullptr);

    ArkUI_NumberValue valueForward[] = { { .i32 = ARKUI_FOCUS_MOVE_FORWARD } };
    ArkUI_AttributeItem itemForward = { valueForward, 1, nullptr, forwardNode };
    ASSERT_EQ(nodeAPI->setAttribute(rootNode, NODE_NEXT_FOCUS, &itemForward), ERROR_CODE_NO_ERROR);

    ArkUI_NumberValue valueUp[] = { { .i32 = ARKUI_FOCUS_MOVE_UP } };
    ArkUI_AttributeItem itemUp = { valueUp, 1, nullptr, upNode };
    ASSERT_EQ(nodeAPI->setAttribute(rootNode, NODE_NEXT_FOCUS, &itemUp), ERROR_CODE_NO_ERROR);

    auto getItem = nodeAPI->getAttribute(rootNode, NODE_NEXT_FOCUS);
    ASSERT_NE(getItem, nullptr);
    ASSERT_NE(getItem->value, nullptr);
    ASSERT_NE(getItem->object, nullptr);
    ASSERT_EQ(getItem->size, 2);
    auto nextFocusNodes = reinterpret_cast<ArkUI_NodeHandle*>(getItem->object);
    EXPECT_EQ(getItem->value[0].i32, ARKUI_FOCUS_MOVE_FORWARD);
    EXPECT_EQ(getItem->value[1].i32, ARKUI_FOCUS_MOVE_UP);
    EXPECT_EQ(nextFocusNodes[0], forwardNode);
    EXPECT_EQ(nextFocusNodes[1], upNode);

    nodeAPI->disposeNode(upNode);
    nodeAPI->disposeNode(forwardNode);
    nodeAPI->disposeNode(rootNode);
}

} // namespace OHOS::Ace

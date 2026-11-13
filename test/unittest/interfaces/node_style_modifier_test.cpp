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
#include "native_interface.h"
#include "native_node.h"
#include "native_type.h"
#include "event_converter.h"
#include "interfaces/inner_api/ace/node_module_inner.h"
#include "interfaces/native/node/node_extened.h"
#include "interfaces/native/node/node_model.h"
#include "test/mock/frameworks/base/thread/mock_task_executor.h"
#include "test/mock/frameworks/core/common/mock_container.h"
#include "test/mock/frameworks/core/common/mock_theme_manager.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"
#include "frameworks/base/error/error_code.h"
#include "core/components_ng/pattern/waterflow/layout/sliding_window/water_flow_layout_sw.h"
#include "core/components_ng/pattern/waterflow/water_flow_pattern.h"
#include "core/interfaces/arkoala/arkoala_api.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace {
namespace {
constexpr float WATER_FLOW_TEST_EXTRA_OFFSET = 40.5f;
constexpr int32_t WATER_FLOW_OFFSET_PARAM_COUNT = 4;
constexpr int32_t WATER_FLOW_FAST_SCROLL_PARAM_COUNT = 5;
ArkUIBasicAPI g_waterFlowUnitBasicApi;
int32_t g_waterFlowConversionCalls = 0;
double g_waterFlowTestDipScale = 1.0;

class WaterFlowUnitConversionScope {
public:
    WaterFlowUnitConversionScope() : fullImpl_(NodeModel::GetFullImpl()), previous_(fullImpl_->getBasicAPI)
    {
        // InitialFullImpl loads the device backend. Its pipeline is independent
        // of MockPipelineContext, so inject only this external conversion dependency.
        g_waterFlowUnitBasicApi = *previous_();
        g_waterFlowUnitBasicApi.convertLengthMetricsUnit = [](double value, int32_t origin, int32_t target) {
            ++g_waterFlowConversionCalls;
            EXPECT_EQ(origin, static_cast<int32_t>(DimensionUnit::VP));
            EXPECT_EQ(target, static_cast<int32_t>(DimensionUnit::PX));
            EXPECT_DOUBLE_EQ(value, WATER_FLOW_TEST_EXTRA_OFFSET);
            return value * g_waterFlowTestDipScale;
        };
        fullImpl_->getBasicAPI = []() -> const ArkUIBasicAPI* { return &g_waterFlowUnitBasicApi; };
    }
    ~WaterFlowUnitConversionScope()
    {
        fullImpl_->getBasicAPI = previous_;
    }
private:
    ArkUIFullNodeAPI* fullImpl_;
    const ArkUIBasicAPI* (*previous_)();
};

void CheckWaterFlowOffsetParameters(ArkUI_NativeNodeAPI_1* nodeAPI, ArkUI_NodeHandle node,
    const RefPtr<NG::WaterFlowPattern>& pattern, bool fastLayout, float convertedOffset)
{
    ArkUI_NumberValue values[] = { { .i32 = 0 }, { .i32 = 1 }, { .i32 = ARKUI_SCROLL_ALIGNMENT_START },
        { .f32 = WATER_FLOW_TEST_EXTRA_OFFSET }, { .i32 = 0 } };
    ArkUI_AttributeItem item { values, WATER_FLOW_FAST_SCROLL_PARAM_COUNT };
    for (int32_t size : { WATER_FLOW_OFFSET_PARAM_COUNT, WATER_FLOW_FAST_SCROLL_PARAM_COUNT }) {
        item.size = size;
        for (int32_t mode : { -1, 0, 1, 2, 3 }) {
            values[WATER_FLOW_FAST_SCROLL_PARAM_COUNT - 1].i32 = mode;
            for (int32_t smooth : { 0, 1 }) {
                values[1].i32 = smooth;
                SCOPED_TRACE(testing::Message() << size << "/" << mode << "/" << smooth);
                const bool converted = fastLayout && smooth && size == WATER_FLOW_FAST_SCROLL_PARAM_COUNT &&
                    (mode == 1 || mode == 2) && node->lengthMetricUnit != ARKUI_LENGTH_METRIC_UNIT_PX;
                const float expected = converted ? convertedOffset : WATER_FLOW_TEST_EXTRA_OFFSET;
                g_waterFlowConversionCalls = 0;
                EXPECT_EQ(nodeAPI->setAttribute(node, NODE_WATER_FLOW_SCROLL_TO_INDEX, &item), ERROR_CODE_NO_ERROR);
                EXPECT_EQ(g_waterFlowConversionCalls, static_cast<int32_t>(converted));
                if (!smooth) {
                    EXPECT_FLOAT_EQ(pattern->layoutInfo_->extraOffset_.value_or(0.0f), -expected);
                } else if (pattern->fastScrollState_) {
                    EXPECT_FLOAT_EQ(pattern->fastScrollState_->extraOffset, expected);
                } else {
                    EXPECT_FLOAT_EQ(pattern->GetExtraOffset().value_or(0.0f), expected);
                }
            }
        }
    }
}
} // namespace

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

/**
 * @tc.name: WFFastScrollCapiParameters
 * @tc.desc: The optional fifth value of NODE_WATER_FLOW_SCROLL_TO_INDEX is accepted in every unit and layout mode.
 * @tc.type: FUNC
 */
HWTEST_F(NodeStyleModifierTest, WFFastScrollCapiParameters, TestSize.Level1)
{
    ASSERT_TRUE(NodeModel::InitialFullImpl());
    auto* nodeAPI = reinterpret_cast<ArkUI_NativeNodeAPI_1*>(
        OH_ArkUI_QueryModuleInterfaceByName(ARKUI_NATIVE_NODE, "ArkUI_NativeNodeAPI_1"));
    ASSERT_NE(nodeAPI, nullptr);
    auto node = nodeAPI->createNode(ARKUI_NODE_WATER_FLOW);
    auto child = nodeAPI->createNode(ARKUI_NODE_FLOW_ITEM);
    ASSERT_NE(node, nullptr);
    ASSERT_NE(child, nullptr);
    ASSERT_EQ(nodeAPI->addChild(node, child), ERROR_CODE_NO_ERROR);

    constexpr size_t fastScrollModeSlot = 4;
    constexpr int32_t legacyValueCount = 4;
    constexpr int32_t fastScrollValueCount = 5;
    ArkUI_NumberValue layoutValue[] = { { .i32 = ARKUI_WATER_FLOW_LAYOUT_MODE_SLIDING_WINDOW } };
    ArkUI_AttributeItem layoutItem { layoutValue, 1 };
    ArkUI_NumberValue values[] = { { .i32 = 0 }, { .i32 = 1 }, { .i32 = ARKUI_SCROLL_ALIGNMENT_START },
        { .f32 = 10.0f }, { .i32 = 1 } };
    ArkUI_AttributeItem item { values, fastScrollValueCount };
    for (auto layoutMode :
        { ARKUI_WATER_FLOW_LAYOUT_MODE_SLIDING_WINDOW, ARKUI_WATER_FLOW_LAYOUT_MODE_ALWAYS_TOP_DOWN }) {
        layoutValue[0].i32 = layoutMode;
        ASSERT_EQ(nodeAPI->setAttribute(node, NODE_WATER_FLOW_LAYOUT_MODE, &layoutItem), ERROR_CODE_NO_ERROR);
        for (auto unit : { ARKUI_LENGTH_METRIC_UNIT_DEFAULT, ARKUI_LENGTH_METRIC_UNIT_VP,
                 ARKUI_LENGTH_METRIC_UNIT_PX }) {
            node->lengthMetricUnit = unit;
            // Enabled modes, the disabled mode and out-of-range modes, with and without animation.
            for (int32_t mode : { 1, 2, 0, -1, 3 }) {
                values[fastScrollModeSlot].i32 = mode;
                for (int32_t smooth : { 1, 0 }) {
                    values[1].i32 = smooth;
                    EXPECT_EQ(nodeAPI->setAttribute(node, NODE_WATER_FLOW_SCROLL_TO_INDEX, &item),
                        ERROR_CODE_NO_ERROR) << layoutMode << " " << unit << " " << mode << " " << smooth;
                }
            }
        }
    }
    item.size = legacyValueCount;
    EXPECT_EQ(nodeAPI->setAttribute(node, NODE_WATER_FLOW_SCROLL_TO_INDEX, &item), ERROR_CODE_NO_ERROR);
    nodeAPI->removeChild(node, child);
    nodeAPI->disposeNode(child);
    nodeAPI->disposeNode(node);
}

/**
 * @tc.name: WFFastScrollCapiNullValues
 * @tc.desc: Invalid numeric arrays return 401 without accessing a null value pointer.
 * @tc.type: FUNC
 */
HWTEST_F(NodeStyleModifierTest, WFFastScrollCapiNullValues, TestSize.Level1)
{
    ASSERT_TRUE(NodeModel::InitialFullImpl());
    auto* nodeAPI = reinterpret_cast<ArkUI_NativeNodeAPI_1*>(
        OH_ArkUI_QueryModuleInterfaceByName(ARKUI_NATIVE_NODE, "ArkUI_NativeNodeAPI_1"));
    ASSERT_NE(nodeAPI, nullptr);
    auto node = nodeAPI->createNode(ARKUI_NODE_WATER_FLOW);
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(nodeAPI->setAttribute(node, NODE_WATER_FLOW_SCROLL_TO_INDEX, nullptr), ERROR_CODE_PARAM_INVALID);
    for (int32_t size : { -1, 0, 1, 4, 5 }) {
        ArkUI_AttributeItem item { nullptr, size };
        EXPECT_EQ(nodeAPI->setAttribute(node, NODE_WATER_FLOW_SCROLL_TO_INDEX, &item), ERROR_CODE_PARAM_INVALID);
    }
    ArkUI_NumberValue values[] = { { .i32 = 0 } };
    ArkUI_AttributeItem valid { values, 1 };
    EXPECT_EQ(nodeAPI->setAttribute(node, NODE_WATER_FLOW_SCROLL_TO_INDEX, &valid), ERROR_CODE_NO_ERROR);
    nodeAPI->disposeNode(node);
}

/**
 * @tc.name: WFFastScrollCapiOffsetUnits
 * @tc.desc: Only effective fast scrolling converts VP offsets; legacy and fallback paths keep their original units.
 * @tc.type: FUNC
 */
HWTEST_F(NodeStyleModifierTest, WFFastScrollCapiOffsetUnits, TestSize.Level1)
{
    ASSERT_TRUE(NodeModel::InitialFullImpl());
    ASSERT_NE(NodeModel::GetFullImpl(), nullptr);
    ASSERT_NE(NodeModel::GetFullImpl()->getBasicAPI(), nullptr);
    WaterFlowUnitConversionScope conversionScope;
    auto* nodeAPI = reinterpret_cast<ArkUI_NativeNodeAPI_1*>(
        OH_ArkUI_QueryModuleInterfaceByName(ARKUI_NATIVE_NODE, "ArkUI_NativeNodeAPI_1"));
    ASSERT_NE(nodeAPI, nullptr);
    auto node = nodeAPI->createNode(ARKUI_NODE_WATER_FLOW);
    auto child = nodeAPI->createNode(ARKUI_NODE_FLOW_ITEM);
    ASSERT_NE(node, nullptr);
    ASSERT_NE(child, nullptr);
    ASSERT_EQ(nodeAPI->addChild(node, child), ERROR_CODE_NO_ERROR);
    auto* frameNode = reinterpret_cast<NG::FrameNode*>(node->uiNodeHandle);
    auto pattern = frameNode->GetPattern<NG::WaterFlowPattern>();
    ASSERT_NE(pattern, nullptr);
    ArkUI_NumberValue layoutValue[] = { { .i32 = ARKUI_WATER_FLOW_LAYOUT_MODE_SLIDING_WINDOW } };
    ArkUI_AttributeItem layoutItem { layoutValue, 1 };
    for (auto layout : { ARKUI_WATER_FLOW_LAYOUT_MODE_SLIDING_WINDOW,
             ARKUI_WATER_FLOW_LAYOUT_MODE_ALWAYS_TOP_DOWN }) {
        layoutValue[0].i32 = layout;
        EXPECT_EQ(nodeAPI->setAttribute(node, NODE_WATER_FLOW_LAYOUT_MODE, &layoutItem), ERROR_CODE_NO_ERROR);
        for (double density : { 1.0, 1.5, 2.0, 3.0 }) {
            g_waterFlowTestDipScale = density;
            for (auto unit : { ARKUI_LENGTH_METRIC_UNIT_DEFAULT, ARKUI_LENGTH_METRIC_UNIT_VP,
                     ARKUI_LENGTH_METRIC_UNIT_PX }) {
                node->lengthMetricUnit = unit;
                SCOPED_TRACE(testing::Message() << layout << "/" << density << "/" << unit);
                CheckWaterFlowOffsetParameters(nodeAPI, node, pattern,
                    layout == ARKUI_WATER_FLOW_LAYOUT_MODE_SLIDING_WINDOW, WATER_FLOW_TEST_EXTRA_OFFSET * density);
            }
        }
    }
    nodeAPI->removeChild(node, child);
    nodeAPI->disposeNode(child);
    nodeAPI->disposeNode(node);
}

} // namespace OHOS::Ace

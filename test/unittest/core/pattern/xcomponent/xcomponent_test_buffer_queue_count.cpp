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

#include <iostream>
#include <optional>

#include "gtest/gtest.h"

// arm32 pre-existing narrowing warnings from ace headers (modifier.h hash constants)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconstant-conversion"

#define private public
#define protected public

#include "test/mock/frameworks/core/components_ng/render/mock_render_context.h"
#include "test/mock/frameworks/core/components_ng/render/mock_render_surface.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"

#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/xcomponent/xcomponent_controller_ng.h"
#include "core/components_ng/pattern/xcomponent/xcomponent_layout_property.h"
#include "core/components_ng/pattern/xcomponent/xcomponent_model_ng.h"
#include "core/components_ng/pattern/xcomponent/xcomponent_pattern.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {
const std::string XCOMPONENT_ID = "xcomponent";
const std::string XCOMPONENT_LIBRARY_NAME = "native_render";
constexpr XComponentType XCOMPONENT_SURFACE_TYPE_VALUE = XComponentType::SURFACE;
constexpr XComponentType XCOMPONENT_COMPONENT_TYPE_VALUE = XComponentType::COMPONENT;
constexpr XComponentType XCOMPONENT_TEXTURE_TYPE_VALUE = XComponentType::TEXTURE;
} // namespace

class XComponentTestBufferQueueCount : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void TearDown() override {}

protected:
    static RefPtr<FrameNode> CreateXComponentNode(XComponentType type);
    static RefPtr<XComponentPattern> GetPattern(const RefPtr<FrameNode>& frameNode);
};

RefPtr<FrameNode> XComponentTestBufferQueueCount::CreateXComponentNode(XComponentType type)
{
    auto xcomponentController = std::make_shared<XComponentControllerNG>();
    XComponentModelNG().Create(XCOMPONENT_ID, type, XCOMPONENT_LIBRARY_NAME, xcomponentController);
    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish();
    return AceType::DynamicCast<FrameNode>(element);
}

RefPtr<XComponentPattern> XComponentTestBufferQueueCount::GetPattern(const RefPtr<FrameNode>& frameNode)
{
    return frameNode->GetPattern<XComponentPattern>();
}

void XComponentTestBufferQueueCount::SetUpTestSuite()
{
    MockPipelineContext::SetUp();
}

void XComponentTestBufferQueueCount::TearDownTestSuite()
{
    MockPipelineContext::TearDown();
}

/**
 * @tc.name: BufferQueueCountTest001
 * @tc.desc: Test default buffer queue count without configuration
 * @tc.type: FUNC
 */
HWTEST_F(XComponentTestBufferQueueCount, BufferQueueCountTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create xcomponent node with SURFACE type without buffer queue count config
     * @tc.expected: bufferQueueCount_ is default value 5
     */
    auto frameNode = CreateXComponentNode(XCOMPONENT_SURFACE_TYPE_VALUE);
    ASSERT_NE(frameNode, nullptr);
    auto pattern = GetPattern(frameNode);
    ASSERT_NE(pattern, nullptr);
    EXPECT_EQ(pattern->bufferQueueCount_, DEFAULT_BUFFER_QUEUE_COUNT);
    EXPECT_EQ(pattern->bufferQueueCount_, 5);
    EXPECT_EQ(pattern->GetBufferQueueCount(), 5);
}

/**
 * @tc.name: BufferQueueCountTest002
 * @tc.desc: Test SetBufferQueueCount with valid values in [2, 5]
 * @tc.type: FUNC
 */
HWTEST_F(XComponentTestBufferQueueCount, BufferQueueCountTest002, TestSize.Level1)
{
    auto frameNode = CreateXComponentNode(XCOMPONENT_SURFACE_TYPE_VALUE);
    ASSERT_NE(frameNode, nullptr);
    auto pattern = GetPattern(frameNode);
    ASSERT_NE(pattern, nullptr);

    pattern->SetBufferQueueCount(2);
    EXPECT_EQ(pattern->bufferQueueCount_, 2);
    pattern->SetBufferQueueCount(3);
    EXPECT_EQ(pattern->bufferQueueCount_, 3);
    pattern->SetBufferQueueCount(5);
    EXPECT_EQ(pattern->bufferQueueCount_, 5);
}

/**
 * @tc.name: BufferQueueCountTest003
 * @tc.desc: Test SetBufferQueueCount clamping for out-of-range values
 * @tc.type: FUNC
 */
HWTEST_F(XComponentTestBufferQueueCount, BufferQueueCountTest003, TestSize.Level1)
{
    auto frameNode = CreateXComponentNode(XCOMPONENT_SURFACE_TYPE_VALUE);
    ASSERT_NE(frameNode, nullptr);
    auto pattern = GetPattern(frameNode);
    ASSERT_NE(pattern, nullptr);

    pattern->SetBufferQueueCount(1);
    EXPECT_EQ(pattern->bufferQueueCount_, MIN_BUFFER_QUEUE_COUNT);
    pattern->SetBufferQueueCount(0);
    EXPECT_EQ(pattern->bufferQueueCount_, MIN_BUFFER_QUEUE_COUNT);
    pattern->SetBufferQueueCount(-1);
    EXPECT_EQ(pattern->bufferQueueCount_, MIN_BUFFER_QUEUE_COUNT);
    pattern->SetBufferQueueCount(6);
    EXPECT_EQ(pattern->bufferQueueCount_, MAX_BUFFER_QUEUE_COUNT);
    pattern->SetBufferQueueCount(100);
    EXPECT_EQ(pattern->bufferQueueCount_, MAX_BUFFER_QUEUE_COUNT);
    pattern->SetBufferQueueCount(INT32_MAX);
    EXPECT_EQ(pattern->bufferQueueCount_, MAX_BUFFER_QUEUE_COUNT);
    pattern->SetBufferQueueCount(INT32_MIN);
    EXPECT_EQ(pattern->bufferQueueCount_, MIN_BUFFER_QUEUE_COUNT);
}

/**
 * @tc.name: BufferQueueCountTest004
 * @tc.desc: Test Model layer clamping for both instance and static entry
 * @tc.type: FUNC
 */
HWTEST_F(XComponentTestBufferQueueCount, BufferQueueCountTest004, TestSize.Level1)
{
    auto frameNode = CreateXComponentNode(XCOMPONENT_SURFACE_TYPE_VALUE);
    ASSERT_NE(frameNode, nullptr);
    auto pattern = GetPattern(frameNode);
    ASSERT_NE(pattern, nullptr);

    /**
     * @tc.steps: step1. Call ModelNG static entry with out-of-range values
     * @tc.expected: values are clamped to [2, 5]
     */
    XComponentModelNG::SetBufferQueueCount(AceType::RawPtr(frameNode), 1);
    EXPECT_EQ(pattern->bufferQueueCount_, 2);
    XComponentModelNG::SetBufferQueueCount(AceType::RawPtr(frameNode), 10);
    EXPECT_EQ(pattern->bufferQueueCount_, 5);
    XComponentModelNG::SetBufferQueueCount(AceType::RawPtr(frameNode), 4);
    EXPECT_EQ(pattern->bufferQueueCount_, 4);
    EXPECT_EQ(XComponentModelNG::GetBufferQueueCount(AceType::RawPtr(frameNode)), 4);
}

/**
 * @tc.name: BufferQueueCountTest005
 * @tc.desc: Test runtime re-apply when renderSurface_ exists
 * @tc.type: FUNC
 */
HWTEST_F(XComponentTestBufferQueueCount, BufferQueueCountTest005, TestSize.Level1)
{
    auto frameNode = CreateXComponentNode(XCOMPONENT_SURFACE_TYPE_VALUE);
    ASSERT_NE(frameNode, nullptr);
    auto pattern = GetPattern(frameNode);
    ASSERT_NE(pattern, nullptr);

    /**
     * @tc.steps: step1. Attach a mock render surface
     * @tc.expected: SetSurfaceQueueSize and UpdateSurfaceConfig are called on set
     */
    auto mockRenderSurface = AceType::MakeRefPtr<MockRenderSurface>();
    pattern->renderSurface_ = mockRenderSurface;
    ASSERT_NE(pattern->renderSurface_, nullptr);

    EXPECT_CALL(*mockRenderSurface, SetSurfaceQueueSize(3));
    EXPECT_CALL(*mockRenderSurface, UpdateSurfaceConfig()).Times(1);
    pattern->SetBufferQueueCount(3);
    EXPECT_EQ(pattern->bufferQueueCount_, 3);
    testing::Mock::VerifyAndClearExpectations(&(*mockRenderSurface));

    /**
     * @tc.steps: step2. Change value again at runtime
     * @tc.expected: re-applied with new clamped value
     */
    EXPECT_CALL(*mockRenderSurface, SetSurfaceQueueSize(5));
    EXPECT_CALL(*mockRenderSurface, UpdateSurfaceConfig()).Times(1);
    pattern->SetBufferQueueCount(100);
    EXPECT_EQ(pattern->bufferQueueCount_, 5);
    testing::Mock::VerifyAndClearExpectations(&(*mockRenderSurface));
}

/**
 * @tc.name: BufferQueueCountTest006
 * @tc.desc: Test set before surface creation only stores member
 * @tc.type: FUNC
 */
HWTEST_F(XComponentTestBufferQueueCount, BufferQueueCountTest006, TestSize.Level1)
{
    auto frameNode = CreateXComponentNode(XCOMPONENT_SURFACE_TYPE_VALUE);
    ASSERT_NE(frameNode, nullptr);
    auto pattern = GetPattern(frameNode);
    ASSERT_NE(pattern, nullptr);

    /**
     * @tc.steps: step1. renderSurface_ is null (surface not created yet / disposed)
     * @tc.expected: member updated, no crash
     */
    pattern->renderSurface_ = nullptr;
    pattern->SetBufferQueueCount(4);
    EXPECT_EQ(pattern->bufferQueueCount_, 4);

    /**
     * @tc.steps: step2. Simulate surface created later, InitSurface injection consumes member
     * @tc.expected: mock surface receives the stored value via SetSurfaceQueueSize
     */
    auto mockRenderSurface = AceType::MakeRefPtr<MockRenderSurface>();
    pattern->renderSurface_ = mockRenderSurface;
    EXPECT_CALL(*mockRenderSurface, SetSurfaceQueueSize(4));
    mockRenderSurface->SetSurfaceQueueSize(pattern->bufferQueueCount_);
    testing::Mock::VerifyAndClearExpectations(&(*mockRenderSurface));
}

/**
 * @tc.name: BufferQueueCountTest007
 * @tc.desc: Test COMPONENT type (no surface) only stores member without crash
 * @tc.type: FUNC
 */
HWTEST_F(XComponentTestBufferQueueCount, BufferQueueCountTest007, TestSize.Level1)
{
    auto frameNode = CreateXComponentNode(XCOMPONENT_COMPONENT_TYPE_VALUE);
    ASSERT_NE(frameNode, nullptr);
    auto pattern = GetPattern(frameNode);
    ASSERT_NE(pattern, nullptr);

    pattern->SetBufferQueueCount(3);
    EXPECT_EQ(pattern->bufferQueueCount_, 3);
    EXPECT_EQ(pattern->GetBufferQueueCount(), 3);
}

/**
 * @tc.name: BufferQueueCountTest008
 * @tc.desc: Test TEXTURE type applies queue count when surface exists
 * @tc.type: FUNC
 */
HWTEST_F(XComponentTestBufferQueueCount, BufferQueueCountTest008, TestSize.Level1)
{
    auto frameNode = CreateXComponentNode(XCOMPONENT_TEXTURE_TYPE_VALUE);
    ASSERT_NE(frameNode, nullptr);
    auto pattern = GetPattern(frameNode);
    ASSERT_NE(pattern, nullptr);

    auto mockRenderSurface = AceType::MakeRefPtr<MockRenderSurface>();
    pattern->renderSurface_ = mockRenderSurface;

    EXPECT_CALL(*mockRenderSurface, SetSurfaceQueueSize(2));
    EXPECT_CALL(*mockRenderSurface, UpdateSurfaceConfig()).Times(1);
    pattern->SetBufferQueueCount(2);
    EXPECT_EQ(pattern->bufferQueueCount_, 2);
}

/**
 * @tc.name: BufferQueueCountTest009
 * @tc.desc: Test reset semantics restore default 5
 * @tc.type: FUNC
 */
HWTEST_F(XComponentTestBufferQueueCount, BufferQueueCountTest009, TestSize.Level1)
{
    auto frameNode = CreateXComponentNode(XCOMPONENT_SURFACE_TYPE_VALUE);
    ASSERT_NE(frameNode, nullptr);
    auto pattern = GetPattern(frameNode);
    ASSERT_NE(pattern, nullptr);

    pattern->SetBufferQueueCount(2);
    EXPECT_EQ(pattern->bufferQueueCount_, 2);

    auto mockRenderSurface = AceType::MakeRefPtr<MockRenderSurface>();
    pattern->renderSurface_ = mockRenderSurface;
    EXPECT_CALL(*mockRenderSurface, SetSurfaceQueueSize(DEFAULT_BUFFER_QUEUE_COUNT));
    EXPECT_CALL(*mockRenderSurface, UpdateSurfaceConfig()).Times(1);
    pattern->SetBufferQueueCount(DEFAULT_BUFFER_QUEUE_COUNT);
    EXPECT_EQ(pattern->bufferQueueCount_, DEFAULT_BUFFER_QUEUE_COUNT);
}

/**
 * @tc.name: BufferQueueCountTest010
 * @tc.desc: Test C API modifier entry via XComponentModelNG frameNode path
 * @tc.type: FUNC
 */
HWTEST_F(XComponentTestBufferQueueCount, BufferQueueCountTest010, TestSize.Level1)
{
    auto frameNode = CreateXComponentNode(XCOMPONENT_SURFACE_TYPE_VALUE);
    ASSERT_NE(frameNode, nullptr);
    auto pattern = GetPattern(frameNode);
    ASSERT_NE(pattern, nullptr);

    /**
     * @tc.steps: step1. Simulate modifier table entry: SetBufferQueueCount(frameNode, value)
     * @tc.expected: same as direct pattern call (single ModelNG choke point)
     */
    XComponentModelNG::SetBufferQueueCount(AceType::RawPtr(frameNode), 3);
    EXPECT_EQ(pattern->bufferQueueCount_, 3);
    XComponentModelNG::SetBufferQueueCount(AceType::RawPtr(frameNode), -5);
    EXPECT_EQ(pattern->bufferQueueCount_, 2);
}

/**
 * @tc.name: BufferQueueCountTest011
 * @tc.desc: Test null safety of ModelNG entries
 * @tc.type: FUNC
 */
HWTEST_F(XComponentTestBufferQueueCount, BufferQueueCountTest011, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Call with null frameNode
     * @tc.expected: no crash, getter returns default
     */
    XComponentModelNG::SetBufferQueueCount(nullptr, 3);
    EXPECT_EQ(XComponentModelNG::GetBufferQueueCount(nullptr), DEFAULT_BUFFER_QUEUE_COUNT);
}

#pragma clang diagnostic pop
} // namespace OHOS::Ace::NG

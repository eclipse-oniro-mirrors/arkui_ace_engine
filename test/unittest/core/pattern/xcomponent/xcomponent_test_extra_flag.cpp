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

#define private public
#define protected public
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"

#include "base/json/json_util.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/inspector_filter.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/xcomponent/xcomponent_controller_ng.h"
#include "core/components_ng/pattern/xcomponent/xcomponent_layout_property.h"
#include "core/components_ng/pattern/xcomponent/xcomponent_model_ng.h"
#include "core/components_ng/pattern/xcomponent/xcomponent_pattern.h"
#include "core/components_ng/render/render_context.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {
const std::string XCOMPONENT_ID = "xcomponent";
const std::string XCOMPONENT_LIBRARY_NAME = "native_render";
constexpr uint32_t XCOMPONENT_EXTRA_FLAG_ENABLE_SOLID_COLOR = 0x01;
constexpr XComponentType XCOMPONENT_SURFACE_TYPE_VALUE = XComponentType::SURFACE;
constexpr XComponentType XCOMPONENT_COMPONENT_TYPE_VALUE = XComponentType::COMPONENT;
constexpr XComponentType XCOMPONENT_TEXTURE_TYPE_VALUE = XComponentType::TEXTURE;
constexpr uint32_t XCOMPONENT_EXTRA_FLAG_WITHOUT_SOLID_COLOR = 0x10;
} // namespace

class XComponentTestExtraFlag : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void TearDown() override {}

protected:
    static RefPtr<FrameNode> CreateXComponentNode(XComponentType type);
    static void InitSurfaceRenderContext(const RefPtr<XComponentPattern>& pattern);
    static uint32_t GetExtraFlag(const RefPtr<XComponentPattern>& pattern);
    static void DumpToJsonValue(
        const RefPtr<XComponentPattern>& pattern, std::unique_ptr<JsonValue>& json, const InspectorFilter& filter);
};

RefPtr<FrameNode> XComponentTestExtraFlag::CreateXComponentNode(XComponentType type)
{
    auto* stack = ViewStackProcessor::GetInstance();
    stack->ClaimNodeId();
    auto xcomponentController = std::make_shared<XComponentControllerNG>();
    XComponentModelNG().Create(XCOMPONENT_ID, type, XCOMPONENT_LIBRARY_NAME, xcomponentController);
    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish();
    return AceType::DynamicCast<FrameNode>(element);
}

void XComponentTestExtraFlag::InitSurfaceRenderContext(const RefPtr<XComponentPattern>& pattern)
{
    pattern->renderContextForSurface_ = AceType::MakeRefPtr<RenderContext>();
}

uint32_t XComponentTestExtraFlag::GetExtraFlag(const RefPtr<XComponentPattern>& pattern)
{
    return pattern->extraFlag_;
}

void XComponentTestExtraFlag::DumpToJsonValue(
    const RefPtr<XComponentPattern>& pattern, std::unique_ptr<JsonValue>& json, const InspectorFilter& filter)
{
    pattern->ToJsonValue(json, filter);
}

void XComponentTestExtraFlag::SetUpTestSuite()
{
    MockPipelineContext::SetUp();
}

void XComponentTestExtraFlag::TearDownTestSuite()
{
    MockPipelineContext::TearDown();
}

/**
 * @tc.name: SetExtraFlag001
 * @tc.desc: Test SetExtraFlag with ENABLE_SOLID_COLOR on SURFACE type
 * @tc.type: FUNC
 */
HWTEST_F(XComponentTestExtraFlag, SetExtraFlag001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create xcomponent node with SURFACE type
     * @tc.expected: frameNode created successfully
     */
    auto frameNode = CreateXComponentNode(XCOMPONENT_SURFACE_TYPE_VALUE);
    ASSERT_NE(frameNode, nullptr);
    ASSERT_EQ(frameNode->GetTag(), V2::XCOMPONENT_ETS_TAG);

    auto pattern = frameNode->GetPattern<XComponentPattern>();
    ASSERT_NE(pattern, nullptr);

    /**
     * @tc.steps: step2. Initialize renderContextForSurface_
     * @tc.expected: renderContextForSurface_ initialized
     */
    InitSurfaceRenderContext(pattern);

    /**
     * @tc.steps: step3. Call SetExtraFlag with ENABLE_SOLID_COLOR
     * @tc.expected: extraFlag_ recorded, forwarding reaches render context without crash
     */
    pattern->SetExtraFlag(XCOMPONENT_EXTRA_FLAG_ENABLE_SOLID_COLOR);
    EXPECT_EQ(GetExtraFlag(pattern), XCOMPONENT_EXTRA_FLAG_ENABLE_SOLID_COLOR);
}

/**
 * @tc.name: SetExtraFlagTexture001
 * @tc.desc: Test SetExtraFlag stores flag on TEXTURE type without surface forwarding
 * @tc.type: FUNC
 */
HWTEST_F(XComponentTestExtraFlag, SetExtraFlagTexture001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create xcomponent node with TEXTURE type
     * @tc.expected: frameNode created successfully
     */
    auto frameNode = CreateXComponentNode(XCOMPONENT_TEXTURE_TYPE_VALUE);
    ASSERT_NE(frameNode, nullptr);

    auto pattern = frameNode->GetPattern<XComponentPattern>();
    ASSERT_NE(pattern, nullptr);

    /**
     * @tc.steps: step2. Call SetExtraFlag with ENABLE_SOLID_COLOR
     * @tc.expected: flag value is stored without surface forwarding on TEXTURE type
     */
    pattern->SetExtraFlag(XCOMPONENT_EXTRA_FLAG_ENABLE_SOLID_COLOR);
    EXPECT_EQ(GetExtraFlag(pattern), XCOMPONENT_EXTRA_FLAG_ENABLE_SOLID_COLOR);
}

/**
 * @tc.name: SetExtraFlagRepeatedSet001
 * @tc.desc: Test SetExtraFlag can be set repeatedly and the latest value takes effect
 * @tc.type: FUNC
 */
HWTEST_F(XComponentTestExtraFlag, SetExtraFlagRepeatedSet001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create xcomponent node with SURFACE type
     * @tc.expected: frameNode created successfully
     */
    auto frameNode = CreateXComponentNode(XCOMPONENT_SURFACE_TYPE_VALUE);
    ASSERT_NE(frameNode, nullptr);

    auto pattern = frameNode->GetPattern<XComponentPattern>();
    ASSERT_NE(pattern, nullptr);

    InitSurfaceRenderContext(pattern);

    /**
     * @tc.steps: step2. Call SetExtraFlag twice, first enable then disable solid color bit
     * @tc.expected: extraFlag_ keeps the latest value and solid color feature is off
     */
    pattern->SetExtraFlag(XCOMPONENT_EXTRA_FLAG_ENABLE_SOLID_COLOR);
    pattern->SetExtraFlag(XCOMPONENT_EXTRA_FLAG_WITHOUT_SOLID_COLOR);
    EXPECT_EQ(GetExtraFlag(pattern), XCOMPONENT_EXTRA_FLAG_WITHOUT_SOLID_COLOR);

    InspectorFilter filter;
    auto jsonValue = JsonUtil::Create(true);
    DumpToJsonValue(pattern, jsonValue, filter);
    EXPECT_EQ(jsonValue->GetString("extraFlag"), "");
}

/**
 * @tc.name: SetExtraFlagInvalidValue001
 * @tc.desc: Test SetExtraFlag without solid color bit set keeps feature off
 * @tc.type: FUNC
 */
HWTEST_F(XComponentTestExtraFlag, SetExtraFlagInvalidValue001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create xcomponent node with SURFACE type
     * @tc.expected: frameNode created successfully
     */
    auto frameNode = CreateXComponentNode(XCOMPONENT_SURFACE_TYPE_VALUE);
    ASSERT_NE(frameNode, nullptr);

    auto pattern = frameNode->GetPattern<XComponentPattern>();
    ASSERT_NE(pattern, nullptr);

    InitSurfaceRenderContext(pattern);

    /**
     * @tc.steps: step2. Call SetExtraFlag with a value whose solid color bit is not set
     * @tc.expected: extraFlag_ stores the value and solid color feature stays off
     */
    pattern->SetExtraFlag(XCOMPONENT_EXTRA_FLAG_WITHOUT_SOLID_COLOR);
    EXPECT_EQ(GetExtraFlag(pattern), XCOMPONENT_EXTRA_FLAG_WITHOUT_SOLID_COLOR);

    InspectorFilter filter;
    auto jsonValue = JsonUtil::Create(true);
    DumpToJsonValue(pattern, jsonValue, filter);
    EXPECT_EQ(jsonValue->GetString("extraFlag"), "");
}

/**
 * @tc.name: SetExtraFlagCombinedBits001
 * @tc.desc: Test SetExtraFlag with combined feature bits enables solid color feature
 * @tc.type: FUNC
 */
HWTEST_F(XComponentTestExtraFlag, SetExtraFlagCombinedBits001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create xcomponent node with SURFACE type
     * @tc.expected: frameNode created successfully
     */
    auto frameNode = CreateXComponentNode(XCOMPONENT_SURFACE_TYPE_VALUE);
    ASSERT_NE(frameNode, nullptr);

    auto pattern = frameNode->GetPattern<XComponentPattern>();
    ASSERT_NE(pattern, nullptr);

    InitSurfaceRenderContext(pattern);

    /**
     * @tc.steps: step2. Call SetExtraFlag with combined bits (solid color bit | reserved bit)
     * @tc.expected: solid color feature takes effect and extraFlag_ records the combined value
     */
    constexpr uint32_t xcomponentExtraFlagCombined = 0x11;
    pattern->SetExtraFlag(xcomponentExtraFlagCombined);
    EXPECT_EQ(GetExtraFlag(pattern), xcomponentExtraFlagCombined);

    InspectorFilter filter;
    auto jsonValue = JsonUtil::Create(true);
    DumpToJsonValue(pattern, jsonValue, filter);
    EXPECT_EQ(jsonValue->GetString("extraFlag"), "ENABLE_SOLID_COLOR");
}

/**
 * @tc.name: ToJsonValue001
 * @tc.desc: Test ToJsonValue outputs extraFlag field for set and unset cases
 * @tc.type: FUNC
 */
HWTEST_F(XComponentTestExtraFlag, ToJsonValue001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create xcomponent node with SURFACE type and set extraFlag
     * @tc.expected: dump outputs ENABLE_SOLID_COLOR
     */
    auto frameNode = CreateXComponentNode(XCOMPONENT_SURFACE_TYPE_VALUE);
    ASSERT_NE(frameNode, nullptr);

    auto pattern = frameNode->GetPattern<XComponentPattern>();
    ASSERT_NE(pattern, nullptr);

    InitSurfaceRenderContext(pattern);
    pattern->SetExtraFlag(XCOMPONENT_EXTRA_FLAG_ENABLE_SOLID_COLOR);

    InspectorFilter filter;
    auto jsonValue = JsonUtil::Create(true);
    DumpToJsonValue(pattern, jsonValue, filter);
    EXPECT_EQ(jsonValue->GetString("extraFlag"), "ENABLE_SOLID_COLOR");

    /**
     * @tc.steps: step2. Create another xcomponent node without setting extraFlag
     * @tc.expected: dump outputs empty extraFlag
     */
    auto frameNodeUnset = CreateXComponentNode(XCOMPONENT_SURFACE_TYPE_VALUE);
    ASSERT_NE(frameNodeUnset, nullptr);

    auto patternUnset = frameNodeUnset->GetPattern<XComponentPattern>();
    ASSERT_NE(patternUnset, nullptr);

    auto jsonValueUnset = JsonUtil::Create(true);
    DumpToJsonValue(patternUnset, jsonValueUnset, filter);
    EXPECT_EQ(jsonValueUnset->GetString("extraFlag"), "");
}

/**
 * @tc.name: SetExtraFlagComponentIgnore001
 * @tc.desc: Test SetExtraFlag stores flag on COMPONENT type without surface forwarding
 * @tc.type: FUNC
 */
HWTEST_F(XComponentTestExtraFlag, SetExtraFlagComponentIgnore001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create xcomponent node with COMPONENT type
     * @tc.expected: frameNode created successfully
     */
    auto frameNode = CreateXComponentNode(XCOMPONENT_COMPONENT_TYPE_VALUE);
    ASSERT_NE(frameNode, nullptr);

    auto pattern = frameNode->GetPattern<XComponentPattern>();
    ASSERT_NE(pattern, nullptr);

    /**
     * @tc.steps: step2. Call SetExtraFlag on COMPONENT type
     * @tc.expected: flag value is stored without surface forwarding on COMPONENT type
     */
    pattern->SetExtraFlag(XCOMPONENT_EXTRA_FLAG_ENABLE_SOLID_COLOR);
    EXPECT_EQ(GetExtraFlag(pattern), XCOMPONENT_EXTRA_FLAG_ENABLE_SOLID_COLOR);
}

/**
 * @tc.name: SetExtraFlagNotSet001
 * @tc.desc: Test behavior when SetExtraFlag is not set
 * @tc.type: FUNC
 */
HWTEST_F(XComponentTestExtraFlag, SetExtraFlagNotSet001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create xcomponent node with SURFACE type without setting extraFlag
     * @tc.expected: extraFlag_ keeps 0, dump outputs empty extraFlag
     */
    auto frameNode = CreateXComponentNode(XCOMPONENT_SURFACE_TYPE_VALUE);
    ASSERT_NE(frameNode, nullptr);

    auto pattern = frameNode->GetPattern<XComponentPattern>();
    ASSERT_NE(pattern, nullptr);

    InitSurfaceRenderContext(pattern);

    EXPECT_EQ(GetExtraFlag(pattern), 0);

    InspectorFilter filter;
    auto jsonValue = JsonUtil::Create(true);
    DumpToJsonValue(pattern, jsonValue, filter);
    EXPECT_EQ(jsonValue->GetString("extraFlag"), "");
}

/**
 * @tc.name: SetExtraFlagWithBackgroundColor001
 * @tc.desc: Test extraFlag and backgroundColor coexistence
 * @tc.type: FUNC
 */
HWTEST_F(XComponentTestExtraFlag, SetExtraFlagWithBackgroundColor001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create xcomponent node with SURFACE type
     * @tc.expected: frameNode created successfully
     */
    auto frameNode = CreateXComponentNode(XCOMPONENT_SURFACE_TYPE_VALUE);
    ASSERT_NE(frameNode, nullptr);

    auto pattern = frameNode->GetPattern<XComponentPattern>();
    ASSERT_NE(pattern, nullptr);

    InitSurfaceRenderContext(pattern);

    /**
     * @tc.steps: step2. Set extraFlag and backgroundColor together
     * @tc.expected: no crash, dump keeps both channels
     */
    pattern->SetExtraFlag(XCOMPONENT_EXTRA_FLAG_ENABLE_SOLID_COLOR);
    frameNode->GetRenderContext()->SetBackgroundColor(Color::BLACK.GetValue());
    EXPECT_EQ(GetExtraFlag(pattern), XCOMPONENT_EXTRA_FLAG_ENABLE_SOLID_COLOR);

    InspectorFilter filter;
    auto jsonValue = JsonUtil::Create(true);
    DumpToJsonValue(pattern, jsonValue, filter);
    EXPECT_EQ(jsonValue->GetString("extraFlag"), "ENABLE_SOLID_COLOR");
}
} // namespace OHOS::Ace::NG

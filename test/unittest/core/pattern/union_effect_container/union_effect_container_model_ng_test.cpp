/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include <cmath>
#include "gmock/gmock-actions.h"
#include "gtest/gtest.h"

#define private public
#define protected public
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"

#include "core/components_ng/pattern/union_effect_container/union_effect_container_model_ng.h"
#include "core/components_ng/pattern/union_effect_container/union_effect_container_pattern.h"
#include "core/components_ng/property/union_effect_container_options.h"
#include "core/components/common/properties/state_attributes.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
class UnionEffectContainerModelNgTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();
};

void UnionEffectContainerModelNgTest::SetUpTestSuite()
{
    MockPipelineContext::SetUp();
}

void UnionEffectContainerModelNgTest::TearDownTestSuite()
{
    MockPipelineContext::TearDown();
}

void UnionEffectContainerModelNgTest::SetUp()
{
    ViewStackProcessor::GetInstance()->ClearStack();
    // RenderContextCreateFunction func = []() -> RefPtr<RenderContext> {
    //     auto renderContext = AceType::MakeRefPtr<RosenRenderContext>();
    //     return renderContext;
    // };
    // MockRenderContextCreator::SetRenderContextCreateFunction(std::move(func));
}

void UnionEffectContainerModelNgTest::TearDown()
{
    ViewStackProcessor::GetInstance()->ClearStack();
    // MockRenderContextCreator::ResetRenderContextCreateFunction();
}

/**
 * @tc.name: Create001
 * @tc.desc: Create function with normal spacing.
 * @tc.type: FUNC
 */
HWTEST_F(UnionEffectContainerModelNgTest, Create001, TestSize.Level1)
{
    UnionEffectContainerModelNG model;
    float spacingRaw = 10.0f;
    UnionEffectContainerOptions options { .spacing = spacingRaw };
    model.Create(options);
    auto node = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(node, nullptr);
    auto pattern = node->GetPattern<UnionEffectContainerPattern>();
    ASSERT_NE(pattern, nullptr);
    auto spacing = pattern->GetSpacing();
    EXPECT_TRUE(NearEqual(spacing, spacingRaw));
}

/**
 * @tc.name: Create002
 * @tc.desc: Create function with nan spacing.
 * @tc.type: FUNC
 */
HWTEST_F(UnionEffectContainerModelNgTest, Create002, TestSize.Level1)
{
    /**
     * @tc.steps1: set nan spacing
     * @tc.expected: the spacing should be invalid and equal to default 0.
     */
    UnionEffectContainerModelNG model;
    float spacingNan = nanf("");
    UnionEffectContainerOptions options { .spacing = spacingNan };
    model.Create(options);
    auto node = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(node, nullptr);
    auto pattern = node->GetPattern<UnionEffectContainerPattern>();
    ASSERT_NE(pattern, nullptr);
    EXPECT_FALSE(std::isnan(pattern->GetSpacing()));
    EXPECT_TRUE(NearEqual(pattern->GetSpacing(), 0.0));
}

/**
 * @tc.name: Create003
 * @tc.desc: Create function with negative spacing.
 * @tc.type: FUNC
 */
HWTEST_F(UnionEffectContainerModelNgTest, Create003, TestSize.Level1)
{
    /**
     * @tc.steps1: set negative spacing
     * @tc.expected: the spacing should be invalid and equal to default 0.
     */
    UnionEffectContainerModelNG model;
    float spacingRaw = -20;
    UnionEffectContainerOptions options { .spacing = spacingRaw };
    model.Create(options);
    auto node = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(node, nullptr);
    auto pattern = node->GetPattern<UnionEffectContainerPattern>();
    ASSERT_NE(pattern, nullptr);
    EXPECT_TRUE(NearEqual(pattern->GetSpacing(), 0.0));
}

/**
 * @tc.name: SetUnionMode001
 * @tc.desc: SetUnionMode without visual state updates render context.
 * @tc.type: FUNC
 */
HWTEST_F(UnionEffectContainerModelNgTest, SetUnionMode001, TestSize.Level1)
{
    UnionEffectContainerModelNG model;
    UnionEffectContainerOptions options;
    model.Create(options);
    model.SetUnionMode(UnionMode::GRAVITY_UNION);
    auto node = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(node, nullptr);
    auto renderContext = node->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    auto unionMode = renderContext->GetUnionMode();
    ASSERT_TRUE(unionMode.has_value());
    EXPECT_EQ(unionMode.value(), UnionMode::GRAVITY_UNION);
}

/**
 * @tc.name: SetUnionMode002
 * @tc.desc: SetUnionMode without pushed frame node, frameNode null branch.
 * @tc.type: FUNC
 */
HWTEST_F(UnionEffectContainerModelNgTest, SetUnionMode002, TestSize.Level1)
{
    UnionEffectContainerModelNG model;
    ViewStackProcessor::GetInstance()->ClearStack();
    model.SetUnionMode(UnionMode::SMOOTH_UNION);
    EXPECT_EQ(ViewStackProcessor::GetInstance()->GetMainFrameNode(), nullptr);
}

/**
 * @tc.name: SetUnionMode003
 * @tc.desc: SetUnionMode early return when current visual state is not on.
 * @tc.type: FUNC
 */
HWTEST_F(UnionEffectContainerModelNgTest, SetUnionMode003, TestSize.Level1)
{
    UnionEffectContainerModelNG model;
    UnionEffectContainerOptions options;
    model.Create(options);
    ViewStackProcessor::GetInstance()->SetVisualState(VisualState::PRESSED);
    model.SetUnionMode(UnionMode::GRAVITY_UNION);
    ViewStackProcessor::GetInstance()->ClearVisualState();
    auto node = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(node, nullptr);
    auto renderContext = node->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    // state is off, union mode should not be updated
    EXPECT_FALSE(renderContext->GetUnionMode().has_value());

    // visual state set with empty stack: eventHub is null, early return
    ViewStackProcessor::GetInstance()->SetVisualState(VisualState::PRESSED);
    ViewStackProcessor::GetInstance()->ClearStack();
    model.SetUnionMode(UnionMode::SMOOTH_UNION);
    EXPECT_EQ(ViewStackProcessor::GetInstance()->GetMainFrameNode(), nullptr);
    ViewStackProcessor::GetInstance()->ClearVisualState();
}
} // namespace OHOS::Ace::NG

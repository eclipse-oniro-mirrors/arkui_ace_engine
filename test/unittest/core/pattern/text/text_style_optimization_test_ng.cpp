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

#include "test/unittest/core/pattern/text/text_base.h"

#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"
#include "test/unittest/core/pattern/test_ng.h"

#include "core/components_ng/pattern/text/text_model_ng.h"
#include "core/components_ng/pattern/text/text_layout_property.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
class TextStyleOptimizationTestNg : public TextBases {
};

// ======================== GetIncludeFontPadding tests ========================

/**
 * @tc.name: GetIncludeFontPadding_NotSet_ReturnsFalse
 * @tc.desc: Test GetIncludeFontPadding when property not set, returns false
 *           (getter uses bool value = false, not the pipeline flag).
 * @tc.type: FUNC
 */
HWTEST_F(TextStyleOptimizationTestNg, GetIncludeFontPadding_NotSet_ReturnsFalse,
    TestSize.Level0)
{
    auto pipeline = MockPipelineContext::GetCurrent();
    bool backup = pipeline->GetConfigTextStyleOptimizeFlag();
    pipeline->SetConfigTextStyleOptimizeFlag(false);

    TextModelNG textModel;
    textModel.Create(u"test");
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);

    EXPECT_FALSE(TextModelNG::GetIncludeFontPadding(frameNode));

    pipeline->SetConfigTextStyleOptimizeFlag(backup);
}

/**
 * @tc.name: GetIncludeFontPadding_PipelineTrue_NotSet_ReturnsFalse
 * @tc.desc: Test GetIncludeFontPadding when property not set and pipeline flag is true,
 *           still returns false (getter uses bool value = false, not the pipeline flag).
 * @tc.type: FUNC
 */
HWTEST_F(TextStyleOptimizationTestNg, GetIncludeFontPadding_PipelineTrue_NotSet_ReturnsFalse,
    TestSize.Level0)
{
    auto pipeline = MockPipelineContext::GetCurrent();
    bool backup = pipeline->GetConfigTextStyleOptimizeFlag();
    pipeline->SetConfigTextStyleOptimizeFlag(true);

    TextModelNG textModel;
    textModel.Create(u"test");
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);

    EXPECT_FALSE(TextModelNG::GetIncludeFontPadding(frameNode));

    pipeline->SetConfigTextStyleOptimizeFlag(backup);
}

/**
 * @tc.name: GetIncludeFontPadding_SetTrue_ReturnsTrue
 * @tc.desc: Test GetIncludeFontPadding when set to true, returns true.
 * @tc.type: FUNC
 */
HWTEST_F(TextStyleOptimizationTestNg, GetIncludeFontPadding_SetTrue_ReturnsTrue,
    TestSize.Level0)
{
    auto pipeline = MockPipelineContext::GetCurrent();
    bool backup = pipeline->GetConfigTextStyleOptimizeFlag();
    pipeline->SetConfigTextStyleOptimizeFlag(false);

    TextModelNG textModel;
    textModel.Create(u"test");
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);

    TextModelNG::SetIncludeFontPadding(frameNode, true);
    EXPECT_TRUE(TextModelNG::GetIncludeFontPadding(frameNode));

    pipeline->SetConfigTextStyleOptimizeFlag(backup);
}

/**
 * @tc.name: GetIncludeFontPadding_SetFalse_ReturnsFalse
 * @tc.desc: Test GetIncludeFontPadding when set to false, returns false.
 * @tc.type: FUNC
 */
HWTEST_F(TextStyleOptimizationTestNg, GetIncludeFontPadding_SetFalse_ReturnsFalse,
    TestSize.Level0)
{
    auto pipeline = MockPipelineContext::GetCurrent();
    bool backup = pipeline->GetConfigTextStyleOptimizeFlag();
    pipeline->SetConfigTextStyleOptimizeFlag(true);

    TextModelNG textModel;
    textModel.Create(u"test");
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);

    TextModelNG::SetIncludeFontPadding(frameNode, false);
    EXPECT_FALSE(TextModelNG::GetIncludeFontPadding(frameNode));

    pipeline->SetConfigTextStyleOptimizeFlag(backup);
}

/**
 * @tc.name: GetIncludeFontPadding_NullFrameNode_ReturnsFalse
 * @tc.desc: Test GetIncludeFontPadding returns false when frameNode is null.
 * @tc.type: FUNC
 */
HWTEST_F(TextStyleOptimizationTestNg, GetIncludeFontPadding_NullFrameNode_ReturnsFalse,
    TestSize.Level0)
{
    EXPECT_FALSE(TextModelNG::GetIncludeFontPadding(nullptr));
}

// ======================== GetFallbackLineSpacing tests ========================

/**
 * @tc.name: GetFallbackLineSpacing_NotSet_ReturnsFalse
 * @tc.desc: Test GetFallbackLineSpacing when property not set, returns false
 *           (getter uses bool value = false, not the pipeline flag).
 * @tc.type: FUNC
 */
HWTEST_F(TextStyleOptimizationTestNg, GetFallbackLineSpacing_NotSet_ReturnsFalse,
    TestSize.Level0)
{
    auto pipeline = MockPipelineContext::GetCurrent();
    bool backup = pipeline->GetConfigTextStyleOptimizeFlag();
    pipeline->SetConfigTextStyleOptimizeFlag(false);

    TextModelNG textModel;
    textModel.Create(u"test");
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);

    EXPECT_FALSE(TextModelNG::GetFallbackLineSpacing(frameNode));

    pipeline->SetConfigTextStyleOptimizeFlag(backup);
}

/**
 * @tc.name: GetFallbackLineSpacing_PipelineTrue_NotSet_ReturnsFalse
 * @tc.desc: Test GetFallbackLineSpacing when property not set and pipeline flag is true,
 *           still returns false (getter uses bool value = false, not the pipeline flag).
 * @tc.type: FUNC
 */
HWTEST_F(TextStyleOptimizationTestNg, GetFallbackLineSpacing_PipelineTrue_NotSet_ReturnsFalse,
    TestSize.Level0)
{
    auto pipeline = MockPipelineContext::GetCurrent();
    bool backup = pipeline->GetConfigTextStyleOptimizeFlag();
    pipeline->SetConfigTextStyleOptimizeFlag(true);

    TextModelNG textModel;
    textModel.Create(u"test");
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);

    EXPECT_FALSE(TextModelNG::GetFallbackLineSpacing(frameNode));

    pipeline->SetConfigTextStyleOptimizeFlag(backup);
}

/**
 * @tc.name: GetFallbackLineSpacing_SetTrue_ReturnsTrue
 * @tc.desc: Test GetFallbackLineSpacing when set to true, returns true.
 * @tc.type: FUNC
 */
HWTEST_F(TextStyleOptimizationTestNg, GetFallbackLineSpacing_SetTrue_ReturnsTrue,
    TestSize.Level0)
{
    auto pipeline = MockPipelineContext::GetCurrent();
    bool backup = pipeline->GetConfigTextStyleOptimizeFlag();
    pipeline->SetConfigTextStyleOptimizeFlag(false);

    TextModelNG textModel;
    textModel.Create(u"test");
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);

    TextModelNG::SetFallbackLineSpacing(frameNode, true);
    EXPECT_TRUE(TextModelNG::GetFallbackLineSpacing(frameNode));

    pipeline->SetConfigTextStyleOptimizeFlag(backup);
}

/**
 * @tc.name: GetFallbackLineSpacing_SetFalse_ReturnsFalse
 * @tc.desc: Test GetFallbackLineSpacing when set to false, returns false.
 * @tc.type: FUNC
 */
HWTEST_F(TextStyleOptimizationTestNg, GetFallbackLineSpacing_SetFalse_ReturnsFalse,
    TestSize.Level0)
{
    auto pipeline = MockPipelineContext::GetCurrent();
    bool backup = pipeline->GetConfigTextStyleOptimizeFlag();
    pipeline->SetConfigTextStyleOptimizeFlag(true);

    TextModelNG textModel;
    textModel.Create(u"test");
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);

    TextModelNG::SetFallbackLineSpacing(frameNode, false);
    EXPECT_FALSE(TextModelNG::GetFallbackLineSpacing(frameNode));

    pipeline->SetConfigTextStyleOptimizeFlag(backup);
}

/**
 * @tc.name: GetFallbackLineSpacing_NullFrameNode_ReturnsFalse
 * @tc.desc: Test GetFallbackLineSpacing returns false when frameNode is null.
 * @tc.type: FUNC
 */
HWTEST_F(TextStyleOptimizationTestNg, GetFallbackLineSpacing_NullFrameNode_ReturnsFalse,
    TestSize.Level0)
{
    EXPECT_FALSE(TextModelNG::GetFallbackLineSpacing(nullptr));
}

// ======================== Rendering path branch tests ========================

/**
 * @tc.name: Rendering_PipelineFalse_NotSet_ReturnsFalse
 * @tc.desc: Rendering path: pipeline=false, property not set →
 *           GetIncludeFontPaddingValue(pipeline_flag)=false, GetFallbackLineSpacingValue(pipeline_flag)=false
 *           (default value from pipeline flag is used when property not set).
 * @tc.type: FUNC
 */
HWTEST_F(TextStyleOptimizationTestNg, Rendering_PipelineFalse_NotSet_ReturnsFalse, TestSize.Level0)
{
    auto pipeline = MockPipelineContext::GetCurrent();
    bool backup = pipeline->GetConfigTextStyleOptimizeFlag();
    pipeline->SetConfigTextStyleOptimizeFlag(false);

    TextModelNG textModel;
    textModel.Create(u"test");
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto textLayoutProperty = frameNode->GetLayoutProperty<TextLayoutProperty>();
    ASSERT_NE(textLayoutProperty, nullptr);

    EXPECT_FALSE(textLayoutProperty->GetIncludeFontPaddingValue(
        pipeline->GetConfigTextStyleOptimizeFlag()));
    EXPECT_FALSE(textLayoutProperty->GetFallbackLineSpacingValue(
        pipeline->GetConfigTextStyleOptimizeFlag()));

    pipeline->SetConfigTextStyleOptimizeFlag(backup);
}

/**
 * @tc.name: Rendering_PipelineTrue_NotSet_ReturnsTrue
 * @tc.desc: Rendering path: pipeline=true, property not set →
 *           GetIncludeFontPaddingValue(pipeline_flag)=true, GetFallbackLineSpacingValue(pipeline_flag)=true
 *           (default value from pipeline flag is used when property not set).
 * @tc.type: FUNC
 */
HWTEST_F(TextStyleOptimizationTestNg, Rendering_PipelineTrue_NotSet_ReturnsTrue, TestSize.Level0)
{
    auto pipeline = MockPipelineContext::GetCurrent();
    bool backup = pipeline->GetConfigTextStyleOptimizeFlag();
    pipeline->SetConfigTextStyleOptimizeFlag(true);

    TextModelNG textModel;
    textModel.Create(u"test");
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto textLayoutProperty = frameNode->GetLayoutProperty<TextLayoutProperty>();
    ASSERT_NE(textLayoutProperty, nullptr);

    EXPECT_TRUE(textLayoutProperty->GetIncludeFontPaddingValue(
        pipeline->GetConfigTextStyleOptimizeFlag()));
    EXPECT_TRUE(textLayoutProperty->GetFallbackLineSpacingValue(
        pipeline->GetConfigTextStyleOptimizeFlag()));

    pipeline->SetConfigTextStyleOptimizeFlag(backup);
}

/**
 * @tc.name: Rendering_PipelineFalse_SetTrue_ReturnsTrue
 * @tc.desc: Rendering path: pipeline=false, property set to true →
 *           GetIncludeFontPaddingValue(pipeline_flag)=true, GetFallbackLineSpacingValue(pipeline_flag)=true
 *           (explicit value overrides pipeline default).
 * @tc.type: FUNC
 */
HWTEST_F(TextStyleOptimizationTestNg, Rendering_PipelineFalse_SetTrue_ReturnsTrue, TestSize.Level0)
{
    auto pipeline = MockPipelineContext::GetCurrent();
    bool backup = pipeline->GetConfigTextStyleOptimizeFlag();
    pipeline->SetConfigTextStyleOptimizeFlag(false);

    TextModelNG textModel;
    textModel.Create(u"test");
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto textLayoutProperty = frameNode->GetLayoutProperty<TextLayoutProperty>();
    ASSERT_NE(textLayoutProperty, nullptr);

    TextModelNG::SetIncludeFontPadding(frameNode, true);
    TextModelNG::SetFallbackLineSpacing(frameNode, true);

    EXPECT_TRUE(textLayoutProperty->GetIncludeFontPaddingValue(
        pipeline->GetConfigTextStyleOptimizeFlag()));
    EXPECT_TRUE(textLayoutProperty->GetFallbackLineSpacingValue(
        pipeline->GetConfigTextStyleOptimizeFlag()));

    pipeline->SetConfigTextStyleOptimizeFlag(backup);
}

/**
 * @tc.name: Rendering_PipelineTrue_SetFalse_ReturnsFalse
 * @tc.desc: Rendering path: pipeline=true, property set to false →
 *           GetIncludeFontPaddingValue(pipeline_flag)=false, GetFallbackLineSpacingValue(pipeline_flag)=false
 *           (explicit value overrides pipeline default).
 * @tc.type: FUNC
 */
HWTEST_F(TextStyleOptimizationTestNg, Rendering_PipelineTrue_SetFalse_ReturnsFalse, TestSize.Level0)
{
    auto pipeline = MockPipelineContext::GetCurrent();
    bool backup = pipeline->GetConfigTextStyleOptimizeFlag();
    pipeline->SetConfigTextStyleOptimizeFlag(true);

    TextModelNG textModel;
    textModel.Create(u"test");
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto textLayoutProperty = frameNode->GetLayoutProperty<TextLayoutProperty>();
    ASSERT_NE(textLayoutProperty, nullptr);

    TextModelNG::SetIncludeFontPadding(frameNode, false);
    TextModelNG::SetFallbackLineSpacing(frameNode, false);

    EXPECT_FALSE(textLayoutProperty->GetIncludeFontPaddingValue(
        pipeline->GetConfigTextStyleOptimizeFlag()));
    EXPECT_FALSE(textLayoutProperty->GetFallbackLineSpacingValue(
        pipeline->GetConfigTextStyleOptimizeFlag()));

    pipeline->SetConfigTextStyleOptimizeFlag(backup);
}
} // namespace OHOS::Ace::NG

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

#include "test/unittest/core/pattern/rich_editor/rich_editor_common_test_ng.h"

#include "core/components_ng/pattern/rich_editor/rich_editor_layout_property.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_model_static.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_model_ng.h"
#include "test/mock/frameworks/core/components_ng/render/mock_paragraph.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"
#include "test/mock/frameworks/core/common/mock_container.h"
#include "test/mock/frameworks/core/common/mock_theme_manager.h"
#include "test/mock/frameworks/core/common/mock_resource_adapter_v2.h"
#include "test/unittest/core/pattern/test_ng.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
class RichEditorTextStyleOptimizationTestNg : public TestNG {
};

// ======================== IsIncludeFontPadding tests ========================

/**
 * @tc.name: IsIncludeFontPadding_NotSet_ReturnsFalse
 * @tc.desc: Test IsIncludeFontPadding when not set, returns false (getter uses isIncludeFontPadding_,
 *           not the pipeline flag).
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorTextStyleOptimizationTestNg, IsIncludeFontPadding_NotSet_ReturnsFalse,
    TestSize.Level0)
{
    auto pipeline = MockPipelineContext::GetCurrent();
    bool backup = pipeline->GetConfigTextStyleOptimizeFlag();
    pipeline->SetConfigTextStyleOptimizeFlag(false);

    auto frameNode = RichEditorModelStatic::CreateFrameNode(0);
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);

    EXPECT_FALSE(pattern->IsIncludeFontPadding());

    pipeline->SetConfigTextStyleOptimizeFlag(backup);
}

/**
 * @tc.name: IsIncludeFontPadding_PipelineTrue_NotSet_ReturnsFalse
 * @tc.desc: Test IsIncludeFontPadding when not set and pipeline flag is true, still returns false
 *           (getter uses isIncludeFontPadding_, not the pipeline flag).
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorTextStyleOptimizationTestNg, IsIncludeFontPadding_PipelineTrue_NotSet_ReturnsFalse,
    TestSize.Level0)
{
    auto pipeline = MockPipelineContext::GetCurrent();
    bool backup = pipeline->GetConfigTextStyleOptimizeFlag();
    pipeline->SetConfigTextStyleOptimizeFlag(true);

    auto frameNode = RichEditorModelStatic::CreateFrameNode(0);
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);

    EXPECT_FALSE(pattern->IsIncludeFontPadding());

    pipeline->SetConfigTextStyleOptimizeFlag(backup);
}

/**
 * @tc.name: IsIncludeFontPadding_SetTrue_ReturnsTrue
 * @tc.desc: Test IsIncludeFontPadding when set to true, returns true.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorTextStyleOptimizationTestNg, IsIncludeFontPadding_SetTrue_ReturnsTrue,
    TestSize.Level0)
{
    auto pipeline = MockPipelineContext::GetCurrent();
    bool backup = pipeline->GetConfigTextStyleOptimizeFlag();
    pipeline->SetConfigTextStyleOptimizeFlag(false);

    auto frameNode = RichEditorModelStatic::CreateFrameNode(0);
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);

    pattern->SetIncludeFontPadding(true);
    EXPECT_TRUE(pattern->IsIncludeFontPadding());

    pipeline->SetConfigTextStyleOptimizeFlag(backup);
}

/**
 * @tc.name: IsIncludeFontPadding_SetFalse_ReturnsFalse
 * @tc.desc: Test IsIncludeFontPadding when set to false, returns false.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorTextStyleOptimizationTestNg, IsIncludeFontPadding_SetFalse_ReturnsFalse,
    TestSize.Level0)
{
    auto pipeline = MockPipelineContext::GetCurrent();
    bool backup = pipeline->GetConfigTextStyleOptimizeFlag();
    pipeline->SetConfigTextStyleOptimizeFlag(true);

    auto frameNode = RichEditorModelStatic::CreateFrameNode(0);
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);

    pattern->SetIncludeFontPadding(false);
    EXPECT_FALSE(pattern->IsIncludeFontPadding());

    pipeline->SetConfigTextStyleOptimizeFlag(backup);
}

// ======================== IsFallbackLineSpacing tests ========================

/**
 * @tc.name: IsFallbackLineSpacing_NotSet_ReturnsFalse
 * @tc.desc: Test IsFallbackLineSpacing when not set, returns false (getter uses isFallbackLineSpacing_,
 *           not the pipeline flag).
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorTextStyleOptimizationTestNg, IsFallbackLineSpacing_NotSet_ReturnsFalse,
    TestSize.Level0)
{
    auto pipeline = MockPipelineContext::GetCurrent();
    bool backup = pipeline->GetConfigTextStyleOptimizeFlag();
    pipeline->SetConfigTextStyleOptimizeFlag(false);

    auto frameNode = RichEditorModelStatic::CreateFrameNode(0);
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);

    EXPECT_FALSE(pattern->IsFallbackLineSpacing());

    pipeline->SetConfigTextStyleOptimizeFlag(backup);
}

/**
 * @tc.name: IsFallbackLineSpacing_PipelineTrue_NotSet_ReturnsFalse
 * @tc.desc: Test IsFallbackLineSpacing when not set and pipeline flag is true, still returns false
 *           (getter uses isFallbackLineSpacing_, not the pipeline flag).
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorTextStyleOptimizationTestNg, IsFallbackLineSpacing_PipelineTrue_NotSet_ReturnsFalse,
    TestSize.Level0)
{
    auto pipeline = MockPipelineContext::GetCurrent();
    bool backup = pipeline->GetConfigTextStyleOptimizeFlag();
    pipeline->SetConfigTextStyleOptimizeFlag(true);

    auto frameNode = RichEditorModelStatic::CreateFrameNode(0);
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);

    EXPECT_FALSE(pattern->IsFallbackLineSpacing());

    pipeline->SetConfigTextStyleOptimizeFlag(backup);
}

/**
 * @tc.name: IsFallbackLineSpacing_SetTrue_ReturnsTrue
 * @tc.desc: Test IsFallbackLineSpacing when set to true, returns true.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorTextStyleOptimizationTestNg, IsFallbackLineSpacing_SetTrue_ReturnsTrue,
    TestSize.Level0)
{
    auto pipeline = MockPipelineContext::GetCurrent();
    bool backup = pipeline->GetConfigTextStyleOptimizeFlag();
    pipeline->SetConfigTextStyleOptimizeFlag(false);

    auto frameNode = RichEditorModelStatic::CreateFrameNode(0);
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);

    pattern->SetFallbackLineSpacing(true);
    EXPECT_TRUE(pattern->IsFallbackLineSpacing());

    pipeline->SetConfigTextStyleOptimizeFlag(backup);
}

/**
 * @tc.name: IsFallbackLineSpacing_SetFalse_ReturnsFalse
 * @tc.desc: Test IsFallbackLineSpacing when set to false, returns false.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorTextStyleOptimizationTestNg, IsFallbackLineSpacing_SetFalse_ReturnsFalse,
    TestSize.Level0)
{
    auto pipeline = MockPipelineContext::GetCurrent();
    bool backup = pipeline->GetConfigTextStyleOptimizeFlag();
    pipeline->SetConfigTextStyleOptimizeFlag(true);

    auto frameNode = RichEditorModelStatic::CreateFrameNode(0);
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);

    pattern->SetFallbackLineSpacing(false);
    EXPECT_FALSE(pattern->IsFallbackLineSpacing());

    pipeline->SetConfigTextStyleOptimizeFlag(backup);
}

// ======================== ToJsonValue serialization tests ========================

/**
 * @tc.name: ToJsonValue_NotSet_SerializesFalse
 * @tc.desc: Test ToJsonValue serializes includeFontPadding and fallbackLineSpacing as "false"
 *           when not set (serialization uses member variables, not pipeline flag).
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorTextStyleOptimizationTestNg, ToJsonValue_NotSet_SerializesFalse,
    TestSize.Level0)
{
    auto pipeline = MockPipelineContext::GetCurrent();
    bool backup = pipeline->GetConfigTextStyleOptimizeFlag();
    pipeline->SetConfigTextStyleOptimizeFlag(true);

    auto frameNode = RichEditorModelStatic::CreateFrameNode(0);
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);

    auto json = JsonUtil::Create(true);
    InspectorFilter filter;
    filter.filterFixed = 0;
    filter.filterExt.clear();
    pattern->ToJsonValue(json, filter);
    EXPECT_EQ(json->GetString("includeFontPadding"), "false");
    EXPECT_EQ(json->GetString("fallbackLineSpacing"), "false");

    pipeline->SetConfigTextStyleOptimizeFlag(backup);
}

/**
 * @tc.name: ToJsonValue_SetTrue_SerializesTrue
 * @tc.desc: Test ToJsonValue serializes includeFontPadding and fallbackLineSpacing as "true"
 *           when explicitly set to true, regardless of pipeline flag.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorTextStyleOptimizationTestNg, ToJsonValue_SetTrue_SerializesTrue,
    TestSize.Level0)
{
    auto pipeline = MockPipelineContext::GetCurrent();
    bool backup = pipeline->GetConfigTextStyleOptimizeFlag();
    pipeline->SetConfigTextStyleOptimizeFlag(false);

    auto frameNode = RichEditorModelStatic::CreateFrameNode(0);
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);

    pattern->SetIncludeFontPadding(true);
    pattern->SetFallbackLineSpacing(true);

    auto json = JsonUtil::Create(true);
    InspectorFilter filter;
    filter.filterFixed = 0;
    filter.filterExt.clear();
    pattern->ToJsonValue(json, filter);
    EXPECT_EQ(json->GetString("includeFontPadding"), "true");
    EXPECT_EQ(json->GetString("fallbackLineSpacing"), "true");

    pipeline->SetConfigTextStyleOptimizeFlag(backup);
}

/**
 * @tc.name: ToJsonValue_SetFalse_SerializesFalse
 * @tc.desc: Test ToJsonValue serializes includeFontPadding and fallbackLineSpacing as "false"
 *           when explicitly set to false, regardless of pipeline flag.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorTextStyleOptimizationTestNg, ToJsonValue_SetFalse_SerializesFalse,
    TestSize.Level0)
{
    auto pipeline = MockPipelineContext::GetCurrent();
    bool backup = pipeline->GetConfigTextStyleOptimizeFlag();
    pipeline->SetConfigTextStyleOptimizeFlag(true);

    auto frameNode = RichEditorModelStatic::CreateFrameNode(0);
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);

    pattern->SetIncludeFontPadding(false);
    pattern->SetFallbackLineSpacing(false);

    auto json = JsonUtil::Create(true);
    InspectorFilter filter;
    filter.filterFixed = 0;
    filter.filterExt.clear();
    pattern->ToJsonValue(json, filter);
    EXPECT_EQ(json->GetString("includeFontPadding"), "false");
    EXPECT_EQ(json->GetString("fallbackLineSpacing"), "false");

    pipeline->SetConfigTextStyleOptimizeFlag(backup);
}

// ======================== Null frameNode tests ========================

/**
 * @tc.name: IsIncludeFontPadding_NullFrameNode_ReturnsFalse
 * @tc.desc: Test RichEditorModelNG::IsIncludeFontPadding returns false when frameNode is null.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorTextStyleOptimizationTestNg, IsIncludeFontPadding_NullFrameNode_ReturnsFalse,
    TestSize.Level0)
{
    EXPECT_FALSE(RichEditorModelNG::IsIncludeFontPadding(nullptr));
}

/**
 * @tc.name: IsFallbackLineSpacing_NullFrameNode_ReturnsFalse
 * @tc.desc: Test RichEditorModelNG::IsFallbackLineSpacing returns false when frameNode is null.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorTextStyleOptimizationTestNg, IsFallbackLineSpacing_NullFrameNode_ReturnsFalse,
    TestSize.Level0)
{
    EXPECT_FALSE(RichEditorModelNG::IsFallbackLineSpacing(nullptr));
}

// ======================== Rendering path branch tests ========================

/**
 * @tc.name: Rendering_PipelineFalse_NotSet_ReturnsFalse
 * @tc.desc: Rendering path: pipeline=false, property not set →
 *           GetIncludeFontPaddingValue(pipeline_flag)=false, GetFallbackLineSpacingValue(pipeline_flag)=false
 *           (default value from pipeline flag is used when property not set).
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorTextStyleOptimizationTestNg, Rendering_PipelineFalse_NotSet_ReturnsFalse, TestSize.Level0)
{
    auto pipeline = MockPipelineContext::GetCurrent();
    bool backup = pipeline->GetConfigTextStyleOptimizeFlag();
    pipeline->SetConfigTextStyleOptimizeFlag(false);

    auto frameNode = RichEditorModelStatic::CreateFrameNode(0);
    ASSERT_NE(frameNode, nullptr);
    auto richEditorLayoutProperty = frameNode->GetLayoutProperty<RichEditorLayoutProperty>();
    ASSERT_NE(richEditorLayoutProperty, nullptr);

    EXPECT_FALSE(richEditorLayoutProperty->GetIncludeFontPaddingValue(
        pipeline->GetConfigTextStyleOptimizeFlag()));
    EXPECT_FALSE(richEditorLayoutProperty->GetFallbackLineSpacingValue(
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
HWTEST_F(RichEditorTextStyleOptimizationTestNg, Rendering_PipelineTrue_NotSet_ReturnsTrue, TestSize.Level0)
{
    auto pipeline = MockPipelineContext::GetCurrent();
    bool backup = pipeline->GetConfigTextStyleOptimizeFlag();
    pipeline->SetConfigTextStyleOptimizeFlag(true);

    auto frameNode = RichEditorModelStatic::CreateFrameNode(0);
    ASSERT_NE(frameNode, nullptr);
    auto richEditorLayoutProperty = frameNode->GetLayoutProperty<RichEditorLayoutProperty>();
    ASSERT_NE(richEditorLayoutProperty, nullptr);

    EXPECT_TRUE(richEditorLayoutProperty->GetIncludeFontPaddingValue(
        pipeline->GetConfigTextStyleOptimizeFlag()));
    EXPECT_TRUE(richEditorLayoutProperty->GetFallbackLineSpacingValue(
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
HWTEST_F(RichEditorTextStyleOptimizationTestNg, Rendering_PipelineFalse_SetTrue_ReturnsTrue, TestSize.Level0)
{
    auto pipeline = MockPipelineContext::GetCurrent();
    bool backup = pipeline->GetConfigTextStyleOptimizeFlag();
    pipeline->SetConfigTextStyleOptimizeFlag(false);

    auto frameNode = RichEditorModelStatic::CreateFrameNode(0);
    ASSERT_NE(frameNode, nullptr);
    auto richEditorLayoutProperty = frameNode->GetLayoutProperty<RichEditorLayoutProperty>();
    ASSERT_NE(richEditorLayoutProperty, nullptr);

    RichEditorModelNG::SetIncludeFontPadding(AceType::RawPtr(frameNode), true);
    RichEditorModelNG::SetFallbackLineSpacing(AceType::RawPtr(frameNode), true);

    EXPECT_TRUE(richEditorLayoutProperty->GetIncludeFontPaddingValue(
        pipeline->GetConfigTextStyleOptimizeFlag()));
    EXPECT_TRUE(richEditorLayoutProperty->GetFallbackLineSpacingValue(
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
HWTEST_F(RichEditorTextStyleOptimizationTestNg, Rendering_PipelineTrue_SetFalse_ReturnsFalse, TestSize.Level0)
{
    auto pipeline = MockPipelineContext::GetCurrent();
    bool backup = pipeline->GetConfigTextStyleOptimizeFlag();
    pipeline->SetConfigTextStyleOptimizeFlag(true);

    auto frameNode = RichEditorModelStatic::CreateFrameNode(0);
    ASSERT_NE(frameNode, nullptr);
    auto richEditorLayoutProperty = frameNode->GetLayoutProperty<RichEditorLayoutProperty>();
    ASSERT_NE(richEditorLayoutProperty, nullptr);

    RichEditorModelNG::SetIncludeFontPadding(AceType::RawPtr(frameNode), false);
    RichEditorModelNG::SetFallbackLineSpacing(AceType::RawPtr(frameNode), false);

    EXPECT_FALSE(richEditorLayoutProperty->GetIncludeFontPaddingValue(
        pipeline->GetConfigTextStyleOptimizeFlag()));
    EXPECT_FALSE(richEditorLayoutProperty->GetFallbackLineSpacingValue(
        pipeline->GetConfigTextStyleOptimizeFlag()));

    pipeline->SetConfigTextStyleOptimizeFlag(backup);
}

// ======================== CHECK_NULL_VOID early return test ========================

/**
 * @tc.name: SetIncludeFontPadding_SameValue_NoDirtyMarking
 * @tc.desc: Test SetIncludeFontPadding early return when new value equals current value
 *           (CHECK_NULL_VOID(isIncludeFontPadding_ != isIncludeFontPadding) returns early).
 *           Verify no crash, then verify setter still works after early return.
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorTextStyleOptimizationTestNg, SetIncludeFontPadding_SameValue_NoDirtyMarking, TestSize.Level0)
{
    auto frameNode = RichEditorModelStatic::CreateFrameNode(0);
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);

    // isIncludeFontPadding_ starts as false
    EXPECT_FALSE(pattern->IsIncludeFontPadding());

    // Call SetIncludeFontPadding(false) - same as current value, CHECK_NULL_VOID returns early
    pattern->SetIncludeFontPadding(false);
    // Verify no crash, value unchanged
    EXPECT_FALSE(pattern->IsIncludeFontPadding());

    // Call SetIncludeFontPadding(true) - different value, setter proceeds normally
    pattern->SetIncludeFontPadding(true);
    EXPECT_TRUE(pattern->IsIncludeFontPadding());
}
} // namespace OHOS::Ace::NG

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

#include "search_base.h"

#include "core/components_ng/pattern/search/search_model_ng.h"
#include "core/components_ng/pattern/search/search_pattern.h"
#include "core/components_ng/pattern/text_field/text_field_layout_property.h"

namespace OHOS::Ace::NG {

class SearchTextStyleOptimizationTestNg : public SearchBases {
};

/**
 * @tc.name: GetIncludeFontPadding_NotSet_ReturnsFalse
 * @tc.desc: Test GetIncludeFontPadding when property is not set, returns false
 *           (getter uses bool value = false, not the pipeline flag).
 * @tc.type: FUNC
 */
HWTEST_F(SearchTextStyleOptimizationTestNg, GetIncludeFontPadding_NotSet_ReturnsFalse,
    TestSize.Level0)
{
    auto pipeline = MockPipelineContext::GetCurrent();
    bool backup = pipeline->GetConfigTextStyleOptimizeFlag();
    pipeline->SetConfigTextStyleOptimizeFlag(false);
    SearchModelNG searchModelInstance;
    searchModelInstance.Create(DEFAULT_TEXT_U16, PLACEHOLDER_U16, SEARCH_SVG);
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    EXPECT_EQ(SearchModelNG::GetIncludeFontPadding(frameNode), false);
    pipeline->SetConfigTextStyleOptimizeFlag(backup);
}

/**
 * @tc.name: GetIncludeFontPadding_PipelineTrue_NotSet_ReturnsFalse
 * @tc.desc: Test GetIncludeFontPadding when pipeline flag is true and property is not set,
 *           still returns false (getter uses bool value = false, not the pipeline flag).
 * @tc.type: FUNC
 */
HWTEST_F(SearchTextStyleOptimizationTestNg, GetIncludeFontPadding_PipelineTrue_NotSet_ReturnsFalse,
    TestSize.Level0)
{
    auto pipeline = MockPipelineContext::GetCurrent();
    bool backup = pipeline->GetConfigTextStyleOptimizeFlag();
    pipeline->SetConfigTextStyleOptimizeFlag(true);
    SearchModelNG searchModelInstance;
    searchModelInstance.Create(DEFAULT_TEXT_U16, PLACEHOLDER_U16, SEARCH_SVG);
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    EXPECT_EQ(SearchModelNG::GetIncludeFontPadding(frameNode), false);
    pipeline->SetConfigTextStyleOptimizeFlag(backup);
}

/**
 * @tc.name: GetIncludeFontPadding_SetTrue_ReturnsTrue
 * @tc.desc: Test GetIncludeFontPadding when property is explicitly set to true.
 * @tc.type: FUNC
 */
HWTEST_F(SearchTextStyleOptimizationTestNg, GetIncludeFontPadding_SetTrue_ReturnsTrue,
    TestSize.Level0)
{
    auto pipeline = MockPipelineContext::GetCurrent();
    bool backup = pipeline->GetConfigTextStyleOptimizeFlag();
    pipeline->SetConfigTextStyleOptimizeFlag(false);
    SearchModelNG searchModelInstance;
    searchModelInstance.Create(DEFAULT_TEXT_U16, PLACEHOLDER_U16, SEARCH_SVG);
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    SearchModelNG::SetIncludeFontPadding(frameNode, true);
    EXPECT_EQ(SearchModelNG::GetIncludeFontPadding(frameNode), true);
    pipeline->SetConfigTextStyleOptimizeFlag(backup);
}

/**
 * @tc.name: GetIncludeFontPadding_SetFalse_ReturnsFalse
 * @tc.desc: Test GetIncludeFontPadding when property is explicitly set to false.
 * @tc.type: FUNC
 */
HWTEST_F(SearchTextStyleOptimizationTestNg, GetIncludeFontPadding_SetFalse_ReturnsFalse,
    TestSize.Level0)
{
    auto pipeline = MockPipelineContext::GetCurrent();
    bool backup = pipeline->GetConfigTextStyleOptimizeFlag();
    pipeline->SetConfigTextStyleOptimizeFlag(true);
    SearchModelNG searchModelInstance;
    searchModelInstance.Create(DEFAULT_TEXT_U16, PLACEHOLDER_U16, SEARCH_SVG);
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    SearchModelNG::SetIncludeFontPadding(frameNode, false);
    EXPECT_EQ(SearchModelNG::GetIncludeFontPadding(frameNode), false);
    pipeline->SetConfigTextStyleOptimizeFlag(backup);
}

/**
 * @tc.name: GetIncludeFontPadding_NullFrameNode_ReturnsFalse
 * @tc.desc: Test GetIncludeFontPadding when frameNode is null (CHECK_NULL_RETURN path).
 * @tc.type: FUNC
 */
HWTEST_F(SearchTextStyleOptimizationTestNg, GetIncludeFontPadding_NullFrameNode_ReturnsFalse, TestSize.Level0)
{
    EXPECT_EQ(SearchModelNG::GetIncludeFontPadding(nullptr), false);
}

/**
 * @tc.name: GetFallbackLineSpacing_NotSet_ReturnsFalse
 * @tc.desc: Test GetFallbackLineSpacing when property is not set, returns false
 *           (getter uses bool value = false, not the pipeline flag).
 * @tc.type: FUNC
 */
HWTEST_F(SearchTextStyleOptimizationTestNg, GetFallbackLineSpacing_NotSet_ReturnsFalse,
    TestSize.Level0)
{
    auto pipeline = MockPipelineContext::GetCurrent();
    bool backup = pipeline->GetConfigTextStyleOptimizeFlag();
    pipeline->SetConfigTextStyleOptimizeFlag(false);
    SearchModelNG searchModelInstance;
    searchModelInstance.Create(DEFAULT_TEXT_U16, PLACEHOLDER_U16, SEARCH_SVG);
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    EXPECT_EQ(SearchModelNG::GetFallbackLineSpacing(frameNode), false);
    pipeline->SetConfigTextStyleOptimizeFlag(backup);
}

/**
 * @tc.name: GetFallbackLineSpacing_PipelineTrue_NotSet_ReturnsFalse
 * @tc.desc: Test GetFallbackLineSpacing when pipeline flag is true and property is not set,
 *           still returns false (getter uses bool value = false, not the pipeline flag).
 * @tc.type: FUNC
 */
HWTEST_F(SearchTextStyleOptimizationTestNg, GetFallbackLineSpacing_PipelineTrue_NotSet_ReturnsFalse,
    TestSize.Level0)
{
    auto pipeline = MockPipelineContext::GetCurrent();
    bool backup = pipeline->GetConfigTextStyleOptimizeFlag();
    pipeline->SetConfigTextStyleOptimizeFlag(true);
    SearchModelNG searchModelInstance;
    searchModelInstance.Create(DEFAULT_TEXT_U16, PLACEHOLDER_U16, SEARCH_SVG);
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    EXPECT_EQ(SearchModelNG::GetFallbackLineSpacing(frameNode), false);
    pipeline->SetConfigTextStyleOptimizeFlag(backup);
}

/**
 * @tc.name: GetFallbackLineSpacing_SetTrue_ReturnsTrue
 * @tc.desc: Test GetFallbackLineSpacing when property is explicitly set to true.
 * @tc.type: FUNC
 */
HWTEST_F(SearchTextStyleOptimizationTestNg, GetFallbackLineSpacing_SetTrue_ReturnsTrue,
    TestSize.Level0)
{
    auto pipeline = MockPipelineContext::GetCurrent();
    bool backup = pipeline->GetConfigTextStyleOptimizeFlag();
    pipeline->SetConfigTextStyleOptimizeFlag(false);
    SearchModelNG searchModelInstance;
    searchModelInstance.Create(DEFAULT_TEXT_U16, PLACEHOLDER_U16, SEARCH_SVG);
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    SearchModelNG::SetFallbackLineSpacing(frameNode, true);
    EXPECT_EQ(SearchModelNG::GetFallbackLineSpacing(frameNode), true);
    pipeline->SetConfigTextStyleOptimizeFlag(backup);
}

/**
 * @tc.name: GetFallbackLineSpacing_SetFalse_ReturnsFalse
 * @tc.desc: Test GetFallbackLineSpacing when property is explicitly set to false.
 * @tc.type: FUNC
 */
HWTEST_F(SearchTextStyleOptimizationTestNg, GetFallbackLineSpacing_SetFalse_ReturnsFalse,
    TestSize.Level0)
{
    auto pipeline = MockPipelineContext::GetCurrent();
    bool backup = pipeline->GetConfigTextStyleOptimizeFlag();
    pipeline->SetConfigTextStyleOptimizeFlag(true);
    SearchModelNG searchModelInstance;
    searchModelInstance.Create(DEFAULT_TEXT_U16, PLACEHOLDER_U16, SEARCH_SVG);
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    SearchModelNG::SetFallbackLineSpacing(frameNode, false);
    EXPECT_EQ(SearchModelNG::GetFallbackLineSpacing(frameNode), false);
    pipeline->SetConfigTextStyleOptimizeFlag(backup);
}

/**
 * @tc.name: GetFallbackLineSpacing_NullFrameNode_ReturnsFalse
 * @tc.desc: Test GetFallbackLineSpacing when frameNode is null (CHECK_NULL_RETURN path).
 * @tc.type: FUNC
 */
HWTEST_F(SearchTextStyleOptimizationTestNg, GetFallbackLineSpacing_NullFrameNode_ReturnsFalse, TestSize.Level0)
{
    EXPECT_EQ(SearchModelNG::GetFallbackLineSpacing(nullptr), false);
}

// ======================== Rendering path branch tests ========================

/**
 * @tc.name: Rendering_PipelineFalse_NotSet_ReturnsFalse
 * @tc.desc: Rendering path: pipeline=false, property not set →
 *           GetIncludeFontPaddingValue(pipeline_flag)=false, GetFallbackLineSpacingValue(pipeline_flag)=false
 *           (default value from pipeline flag is used when property not set).
 * @tc.type: FUNC
 */
HWTEST_F(SearchTextStyleOptimizationTestNg, Rendering_PipelineFalse_NotSet_ReturnsFalse, TestSize.Level0)
{
    auto pipeline = MockPipelineContext::GetCurrent();
    bool backup = pipeline->GetConfigTextStyleOptimizeFlag();
    pipeline->SetConfigTextStyleOptimizeFlag(false);
    SearchModelNG searchModelInstance;
    searchModelInstance.Create(DEFAULT_TEXT_U16, PLACEHOLDER_U16, SEARCH_SVG);
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto textFieldChild = AceType::DynamicCast<FrameNode>(frameNode->GetChildAtIndex(TEXTFIELD_INDEX));
    ASSERT_NE(textFieldChild, nullptr);
    auto textFieldLayoutProperty = textFieldChild->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(textFieldLayoutProperty, nullptr);

    EXPECT_FALSE(textFieldLayoutProperty->GetIncludeFontPaddingValue(
        pipeline->GetConfigTextStyleOptimizeFlag()));
    EXPECT_FALSE(textFieldLayoutProperty->GetFallbackLineSpacingValue(
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
HWTEST_F(SearchTextStyleOptimizationTestNg, Rendering_PipelineTrue_NotSet_ReturnsTrue, TestSize.Level0)
{
    auto pipeline = MockPipelineContext::GetCurrent();
    bool backup = pipeline->GetConfigTextStyleOptimizeFlag();
    pipeline->SetConfigTextStyleOptimizeFlag(true);
    SearchModelNG searchModelInstance;
    searchModelInstance.Create(DEFAULT_TEXT_U16, PLACEHOLDER_U16, SEARCH_SVG);
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto textFieldChild = AceType::DynamicCast<FrameNode>(frameNode->GetChildAtIndex(TEXTFIELD_INDEX));
    ASSERT_NE(textFieldChild, nullptr);
    auto textFieldLayoutProperty = textFieldChild->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(textFieldLayoutProperty, nullptr);

    EXPECT_TRUE(textFieldLayoutProperty->GetIncludeFontPaddingValue(
        pipeline->GetConfigTextStyleOptimizeFlag()));
    EXPECT_TRUE(textFieldLayoutProperty->GetFallbackLineSpacingValue(
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
HWTEST_F(SearchTextStyleOptimizationTestNg, Rendering_PipelineFalse_SetTrue_ReturnsTrue, TestSize.Level0)
{
    auto pipeline = MockPipelineContext::GetCurrent();
    bool backup = pipeline->GetConfigTextStyleOptimizeFlag();
    pipeline->SetConfigTextStyleOptimizeFlag(false);
    SearchModelNG searchModelInstance;
    searchModelInstance.Create(DEFAULT_TEXT_U16, PLACEHOLDER_U16, SEARCH_SVG);
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto textFieldChild = AceType::DynamicCast<FrameNode>(frameNode->GetChildAtIndex(TEXTFIELD_INDEX));
    ASSERT_NE(textFieldChild, nullptr);
    auto textFieldLayoutProperty = textFieldChild->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(textFieldLayoutProperty, nullptr);

    SearchModelNG::SetIncludeFontPadding(frameNode, true);
    SearchModelNG::SetFallbackLineSpacing(frameNode, true);

    EXPECT_TRUE(textFieldLayoutProperty->GetIncludeFontPaddingValue(
        pipeline->GetConfigTextStyleOptimizeFlag()));
    EXPECT_TRUE(textFieldLayoutProperty->GetFallbackLineSpacingValue(
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
HWTEST_F(SearchTextStyleOptimizationTestNg, Rendering_PipelineTrue_SetFalse_ReturnsFalse, TestSize.Level0)
{
    auto pipeline = MockPipelineContext::GetCurrent();
    bool backup = pipeline->GetConfigTextStyleOptimizeFlag();
    pipeline->SetConfigTextStyleOptimizeFlag(true);
    SearchModelNG searchModelInstance;
    searchModelInstance.Create(DEFAULT_TEXT_U16, PLACEHOLDER_U16, SEARCH_SVG);
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto textFieldChild = AceType::DynamicCast<FrameNode>(frameNode->GetChildAtIndex(TEXTFIELD_INDEX));
    ASSERT_NE(textFieldChild, nullptr);
    auto textFieldLayoutProperty = textFieldChild->GetLayoutProperty<TextFieldLayoutProperty>();
    ASSERT_NE(textFieldLayoutProperty, nullptr);

    SearchModelNG::SetIncludeFontPadding(frameNode, false);
    SearchModelNG::SetFallbackLineSpacing(frameNode, false);

    EXPECT_FALSE(textFieldLayoutProperty->GetIncludeFontPaddingValue(
        pipeline->GetConfigTextStyleOptimizeFlag()));
    EXPECT_FALSE(textFieldLayoutProperty->GetFallbackLineSpacingValue(
        pipeline->GetConfigTextStyleOptimizeFlag()));

    pipeline->SetConfigTextStyleOptimizeFlag(backup);
}

} // namespace OHOS::Ace::NG

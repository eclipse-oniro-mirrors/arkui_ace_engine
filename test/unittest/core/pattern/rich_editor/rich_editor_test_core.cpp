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

#include "rich_editor_test_min.h"

#define private public
#define protected public
#include "test/mock/frameworks/base/thread/mock_task_executor.h"
#include "test/mock/frameworks/core/common/mock_container.h"
#include "test/mock/frameworks/core/common/mock_theme_manager.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"
#include "core/components/text_field/textfield_theme.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_pattern.h"
#undef private
#undef protected

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Ace;
using namespace OHOS::Ace::NG;


namespace OHOS::Ace::NG {

// Test classes for TDD development (quick compile mode)
// These classes use different names from full test suite to avoid conflicts
// When ace_enable_full_test_suite=true, full test files will be compiled instead

class RichEditorLayoutAlgorithmTestCore : public RichEditorTestBase {};
class RichEditorPatternTestCore : public RichEditorTestBase {
public:
    void SetUp() override
    {
        RichEditorTestBase::SetUp();
        MockPipelineContext::SetUp();
        MockContainer::SetUp();
        MockContainer::Current()->taskExecutor_ = AceType::MakeRefPtr<MockTaskExecutor>();
        auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
        MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
        auto textFieldTheme = AceType::MakeRefPtr<TextFieldTheme>();
        EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(testing::Return(textFieldTheme));
        auto* stack = ViewStackProcessor::GetInstance();
        auto nodeId = stack->ClaimNodeId();
        richEditorNode_ = FrameNode::GetOrCreateFrameNode(
            V2::RICH_EDITOR_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<RichEditorPattern>(); });
        richEditorNode_->GetGeometryNode()->SetContentSize({});
    }

    void TearDown() override
    {
        richEditorNode_ = nullptr;
        MockPipelineContext::TearDown();
        MockContainer::TearDown();
        RichEditorTestBase::TearDown();
    }

protected:
    RefPtr<FrameNode> richEditorNode_;
};

// Placeholder test to ensure compilation
HWTEST_F(RichEditorLayoutAlgorithmTestCore, RichEditorTestPlaceholder, TestSize.Level1)
{
    EXPECT_TRUE(true);
}

/**
 * @tc.name: RichEditorUpdateCaretInfoNoFocus001
 * @tc.desc: Test UpdateCaretInfoToController returns early when not focused
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorPatternTestCore, UpdateCaretInfoNoFocus001, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    EXPECT_FALSE(pattern->HasFocus());
    pattern->UpdateCaretInfoToController();
    EXPECT_FALSE(pattern->HasFocus());
}

/**
 * @tc.name: RichEditorUpdateCaretInfoFocused001
 * @tc.desc: Test UpdateCaretInfoToController pushes caret info to input method when focused
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorPatternTestCore, UpdateCaretInfoFocused001, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    auto focusHub = richEditorNode_->GetOrCreateFocusHub();
    ASSERT_NE(focusHub, nullptr);
    focusHub->RequestFocusImmediately();
    EXPECT_TRUE(pattern->HasFocus());
    pattern->UpdateCaretInfoToController();
    EXPECT_TRUE(pattern->HasFocus());
}

/**
 * @tc.name: RichEditorPushCaretInfoNoFocus001
 * @tc.desc: Test PushCaretInfoToInputMethod returns early when not focused
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorPatternTestCore, PushCaretInfoNoFocus001, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    EXPECT_FALSE(pattern->HasFocus());
    pattern->PushCaretInfoToInputMethod();
    EXPECT_FALSE(pattern->HasFocus());
}

/**
 * @tc.name: RichEditorPushCaretInfoFocused001
 * @tc.desc: Test PushCaretInfoToInputMethod pushes caret info on the standard input path
 * @tc.type: FUNC
 */
HWTEST_F(RichEditorPatternTestCore, PushCaretInfoFocused001, TestSize.Level1)
{
    auto pattern = richEditorNode_->GetPattern<RichEditorPattern>();
    ASSERT_NE(pattern, nullptr);
    auto focusHub = richEditorNode_->GetOrCreateFocusHub();
    ASSERT_NE(focusHub, nullptr);
    focusHub->RequestFocusImmediately();
    EXPECT_TRUE(pattern->HasFocus());
    pattern->PushCaretInfoToInputMethod();
    EXPECT_TRUE(pattern->HasFocus());
}

} // namespace OHOS::Ace::NG

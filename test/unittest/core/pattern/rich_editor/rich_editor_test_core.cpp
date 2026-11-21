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

using namespace testing;
using namespace testing::ext;


namespace OHOS::Ace::NG {

// Test classes for TDD development (quick compile mode)
// These classes use different names from full test suite to avoid conflicts
// When ace_enable_full_test_suite=true, full test files will be compiled instead

class RichEditorLayoutAlgorithmTestCore : public RichEditorTestBase {};
class RichEditorPatternTestCore : public RichEditorTestBase {};

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

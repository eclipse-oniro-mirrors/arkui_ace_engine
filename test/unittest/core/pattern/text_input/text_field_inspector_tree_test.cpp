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

#include "text_input_base.h"

#include "core/components_ng/base/inspector_tree_request_context.h"

namespace OHOS::Ace::NG {
class InspectorTreeTask08 : public TextInputBases {};

TEST_F(InspectorTreeTask08, PrivacyDoesNotMutateInputOrPlaceholder)
{
    CreateTextField("protected-value", "hint");
    ParamConfig config;
    InspectorTreeRequestContext context(config, { true, "", 1 });
    auto attrs = JsonUtil::CreateSharedPtrJson();
    pattern_->DumpInspectorTreeInfo(attrs, context);
    EXPECT_EQ(attrs->GetString("content"), "");
    EXPECT_EQ(attrs->GetString("placeholder"), "hint");
    EXPECT_EQ(pattern_->GetTextValue(), "protected-value");
    EXPECT_FALSE(attrs->Contains("password"));
    InspectorTreeRequestContext ordinary(config, {});
    auto visible = JsonUtil::CreateSharedPtrJson();
    pattern_->DumpInspectorTreeInfo(visible, ordinary);
    EXPECT_EQ(visible->GetString("content"), "protected-value");
}

TEST_F(InspectorTreeTask08, EmptyInputKeepsFalseAndZeroSemantics)
{
    CreateTextField("", "hint");
    ParamConfig config;
    config.accessibilityInfo = true;
    InspectorTreeRequestContext context(config, {});
    auto attrs = JsonUtil::CreateSharedPtrJson();
    pattern_->DumpInspectorTreeInfo(attrs, context);
    EXPECT_TRUE(attrs->GetValue("content")->IsString());
    EXPECT_EQ(attrs->GetString("content"), "");
    EXPECT_TRUE(attrs->GetValue("password")->IsBool());
    EXPECT_FALSE(attrs->GetBool("password"));
    EXPECT_FALSE(attrs->GetBool("multiLine"));
    EXPECT_EQ(attrs->GetInt("selectionStart"), 0);
    EXPECT_EQ(attrs->GetInt("selectionEnd"), 0);
    EXPECT_FALSE(attrs->Contains("error"));
}
} // namespace OHOS::Ace::NG

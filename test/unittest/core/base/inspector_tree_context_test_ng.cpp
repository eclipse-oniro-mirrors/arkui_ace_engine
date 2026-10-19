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

#include "gtest/gtest.h"

#include "core/components_ng/base/inspector_tree_request_context.h"

namespace OHOS::Ace::NG {
TEST(InspectorTreeTask02, PrivacyMatrixAndPerRequestReasons)
{
    InspectorTreeRequestContext context({}, {}, { true, "Window", 42 });
    EXPECT_FALSE(context.CanExportText(InspectorTextKind::INPUT_VALUE, false, false));
    EXPECT_FALSE(context.CanExportText(InspectorTextKind::EDITABLE_TEXT, false, false));
    EXPECT_TRUE(context.CanExportText(InspectorTextKind::DISPLAY_TEXT, false, false));
    EXPECT_TRUE(context.CanExportText(InspectorTextKind::PLACEHOLDER, false, false));
    EXPECT_FALSE(context.CanExportText(InspectorTextKind::DISPLAY_TEXT, false, true));
    EXPECT_FALSE(context.MarkPrivacyReasonOnce(PrivacyReason::PRIVACY_WINDOW));
    InspectorTreeRequestContext next({}, {}, {});
    EXPECT_TRUE(next.CanExportText(InspectorTextKind::INPUT_VALUE, false, false));
    EXPECT_FALSE(next.CanExportText(InspectorTextKind::INPUT_VALUE, true, false));
    EXPECT_FALSE(next.MarkPrivacyReasonOnce(PrivacyReason::PASSWORD));
    EXPECT_EQ(context.window.windowTitle, "Window");
    EXPECT_EQ(context.window.pid, 42);
}

TEST(InspectorTreeTask10, MarqueeReplicasDoNotDeduplicateIndependentLabels)
{
    auto root = JsonUtil::CreateSharedPtrJson();
    auto children = JsonUtil::ParseJsonString(R"([{"$type":"Marquee","$children":[
        {"$type":"Text","$attrs":{"content":"same"}},
        {"$type":"Text","$attrs":{"content":"same"}}]},
        {"$type":"Text","$attrs":{"content":"same"}}])");
    root->Put("$children", children);
    InspectorTreeRequestContext context({}, {}, {});
    context.FinalizeNativeTree(root);
    auto output = root->GetValue("$children");
    auto replicas = output->GetArrayItem(0)->GetValue("$children");
    EXPECT_EQ(replicas->GetArrayItem(0)->GetObject("$attrs")->GetString("content"), "same");
    EXPECT_FALSE(replicas->GetArrayItem(1)->GetObject("$attrs")->Contains("content"));
    EXPECT_EQ(output->GetArrayItem(1)->GetObject("$attrs")->GetString("content"), "same");
}
} // namespace OHOS::Ace::NG

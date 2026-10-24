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
    InspectorTreeRequestContext context({}, { true, "Window", 42 });
    EXPECT_FALSE(context.CanExportText(InspectorTextKind::INPUT_VALUE, false, false));
    EXPECT_FALSE(context.CanExportText(InspectorTextKind::EDITABLE_TEXT, false, false));
    EXPECT_TRUE(context.CanExportText(InspectorTextKind::DISPLAY_TEXT, false, false));
    EXPECT_TRUE(context.CanExportText(InspectorTextKind::PLACEHOLDER, false, false));
    EXPECT_FALSE(context.CanExportText(InspectorTextKind::DISPLAY_TEXT, false, true));
    EXPECT_FALSE(context.MarkPrivacyReasonOnce(PrivacyReason::PRIVACY_WINDOW));
    InspectorTreeRequestContext next({}, {});
    EXPECT_TRUE(next.CanExportText(InspectorTextKind::INPUT_VALUE, false, false));
    EXPECT_FALSE(next.CanExportText(InspectorTextKind::INPUT_VALUE, true, false));
    EXPECT_FALSE(next.MarkPrivacyReasonOnce(PrivacyReason::PASSWORD));
    EXPECT_EQ(context.window.windowTitle, "Window");
    EXPECT_EQ(context.window.pid, 42);
}

TEST(InspectorTreeTask02, WindowPrivacyIsScopedAndReasonsRemainRequestWide)
{
    InspectorTreeRequestContext context({}, { false, "Main", 42 });
    context.CollectWithWindowPrivacy(InspectorWindowSnapshot { true, "Child", 43 }, [&]() {
        EXPECT_FALSE(context.CanExportText(InspectorTextKind::INPUT_VALUE, false, false));
        EXPECT_FALSE(context.CanExportText(InspectorTextKind::EDITABLE_TEXT, false, false));
        EXPECT_TRUE(context.CanExportText(InspectorTextKind::DISPLAY_TEXT, false, false));
        EXPECT_TRUE(context.CanExportText(InspectorTextKind::PLACEHOLDER, false, false));
        context.CollectWithWindowPrivacy(InspectorWindowSnapshot {},
            [&]() { EXPECT_TRUE(context.CanExportText(InspectorTextKind::INPUT_VALUE, false, false)); });
        EXPECT_TRUE(context.window.privacyWindow);
    });
    EXPECT_FALSE(context.window.privacyWindow);
    EXPECT_TRUE(context.CanExportText(InspectorTextKind::INPUT_VALUE, false, false));
    EXPECT_FALSE(context.MarkPrivacyReasonOnce(PrivacyReason::PRIVACY_WINDOW));
    EXPECT_EQ(context.window.windowTitle, "Main");
    EXPECT_EQ(context.window.pid, 42);
}

TEST(InspectorTreeTask02, MissingWindowSnapshotStopsBeforeCollecting)
{
    ParamConfig config;
    config.withResponseRegion = true;
    InspectorTreeRequestContext context(config, {});
    bool collected = false;
    context.CollectWithWindowPrivacy(std::nullopt, [&]() { collected = true; });
    EXPECT_FALSE(collected);
    EXPECT_TRUE(context.ShouldStop());
    auto root = JsonUtil::CreateSharedPtrJson();
    root->Put("content", "earlier result");
    EXPECT_EQ(context.FinalizeResult(root)->ToString(), R"({"errorMessage":"collection failed"})");
}

TEST(InspectorTreeTask02, WindowCollectionPreservesErrorsAndRestoresPrivacy)
{
    InspectorTreeRequestContext context({}, { true, "Main", 42 });
    context.CollectWithWindowPrivacy(InspectorWindowSnapshot {}, [&]() {
        EXPECT_FALSE(context.window.privacyWindow);
        context.ReportError(InspectorTreeError::PROVIDER_QUERY_FAILED);
    });
    EXPECT_TRUE(context.window.privacyWindow);
    EXPECT_TRUE(context.ShouldStop());
    context.CollectWithWindowPrivacy(InspectorWindowSnapshot {}, []() { ADD_FAILURE(); });
    EXPECT_STREQ(context.dependencyResult.GetErrorMessage(), "provider query failed");
}

TEST(InspectorTreeTask10, MarqueeReplicasDoNotDeduplicateIndependentLabels)
{
    auto root = JsonUtil::CreateSharedPtrJson();
    auto children = JsonUtil::ParseJsonString(R"([{"$type":"Marquee","$children":[
        {"$type":"Text","$attrs":{"content":"same"}},
        {"$type":"Text","$attrs":{"content":"same"}}]},
        {"$type":"Text","$attrs":{"content":"same"}}])");
    root->Put("$children", children);
    InspectorTreeRequestContext context({}, {});
    context.FinalizeNativeTree(root);
    auto output = root->GetValue("$children");
    auto replicas = output->GetArrayItem(0)->GetValue("$children");
    EXPECT_EQ(replicas->GetArrayItem(0)->GetObject("$attrs")->GetString("content"), "same");
    EXPECT_FALSE(replicas->GetArrayItem(1)->GetObject("$attrs")->Contains("content"));
    EXPECT_EQ(output->GetArrayItem(1)->GetObject("$attrs")->GetString("content"), "same");
}
TEST(InspectorTreeTask02, RootErrorGateAndWholeRequestFailure)
{
    for (bool regions : { false, true }) {
        for (bool customContent : { false, true }) {
            ParamConfig config;
            config.interactionInfo = true;
            config.accessibilityInfo = true;
            config.withResponseRegion = regions;
            config.withCustomContent = customContent;
            InspectorTreeRequestContext context(config, {});
            auto root = JsonUtil::CreateSharedPtrJson();
            root->Put("appInfo", JsonUtil::Create());
            root->Put("$children", JsonUtil::CreateArray());
            auto success = context.FinalizeResult(root);
            ASSERT_NE(success, nullptr);
            EXPECT_TRUE(success->Contains("appInfo"));
            EXPECT_EQ(success->Contains("errorMessage"), regions || customContent);
            if (regions || customContent) {
                EXPECT_EQ(success->GetString("errorMessage"), "no error");
            }

            context.ReportError(InspectorTreeError::PROVIDER_QUERY_FAILED);
            context.ReportError(InspectorTreeError::INVALID_PROVIDER_DATA);
            auto failure = context.FinalizeResult(root);
            ASSERT_NE(failure, nullptr);
            EXPECT_FALSE(failure->Contains("appInfo"));
            EXPECT_FALSE(failure->Contains("$children"));
            EXPECT_EQ(failure->Contains("errorMessage"), regions || customContent);
            if (regions || customContent) {
                EXPECT_EQ(failure->GetString("errorMessage"), "provider query failed");
            } else {
                EXPECT_EQ(failure->ToString(), "{}");
            }
        }
    }
}

TEST(InspectorTreeTask02, CancelledRequestDoesNotProduceAnErrorResult)
{
    ParamConfig config;
    config.withCustomContent = true;
    InspectorTreeRequestContext context(config, {});
    context.ReportError(InspectorTreeError::COLLECTION_FAILED);
    context.CancelResult();
    EXPECT_TRUE(context.ShouldStop());
    EXPECT_EQ(context.FinalizeResult(JsonUtil::CreateSharedPtrJson()), nullptr);
}

TEST(InspectorTreeTask16, DependencyReasonsAreMappedWithoutReplacingTheFirstError)
{
    for (const auto* reason : { "collection failed", "provider query failed", "invalid provider data",
             "node count exceeds limit", "tree data size exceeds limit" }) {
        InspectorTreeDependencyResult result;
        EXPECT_FALSE(result.HasError());
        result.ReportErrorMessage(reason);
        result.ReportErrorMessage("collection failed");
        EXPECT_TRUE(result.HasError());
        EXPECT_EQ(std::string(result.GetErrorMessage()), reason);
    }
    InspectorTreeDependencyResult unknown;
    unknown.ReportErrorMessage("untrusted dependency diagnostic");
    EXPECT_EQ(std::string(unknown.GetErrorMessage()), "collection failed");
}

TEST(InspectorTreeTask06, ExtensionFailurePropagatesToTheRequestRoot)
{
    for (const auto* reason : { "collection failed", "provider query failed", "invalid provider data",
             "node count exceeds limit", "tree data size exceeds limit" }) {
        ParamConfig config;
        config.withCustomContent = true;
        InspectorTreeRequestContext context(config, {});
        auto extension = JsonUtil::Create();
        extension->Put("errorMessage", reason);
        EXPECT_FALSE(context.AcceptExtensionTree(*extension));
        auto root = JsonUtil::CreateSharedPtrJson();
        root->Put("appInfo", JsonUtil::Create());
        root->Put("$child-uec", extension);
        auto result = context.FinalizeResult(root);
        EXPECT_EQ(result->GetString("errorMessage"), reason);
        EXPECT_FALSE(result->Contains("appInfo"));
        EXPECT_FALSE(result->Contains("$child-uec"));
    }
}

TEST(InspectorTreeTask06, SuccessfulExtensionKeepsItsTreeAndConsumesRootStatus)
{
    ParamConfig config;
    config.withCustomContent = true;
    InspectorTreeRequestContext context(config, {});
    auto extension = JsonUtil::ParseJsonString(
        R"({"errorMessage":"no error","$children":[{"$type":"Text"}]})");
    EXPECT_TRUE(context.AcceptExtensionTree(*extension));
    EXPECT_FALSE(extension->Contains("errorMessage"));
    EXPECT_EQ(extension->GetValue("$children")->GetArrayItem(0)->GetString("$type"), "Text");
    auto empty = JsonUtil::Create();
    EXPECT_FALSE(context.AcceptExtensionTree(*empty));
    EXPECT_EQ(std::string(context.dependencyResult.GetErrorMessage()), "collection failed");
}

TEST(InspectorTreeTask06, ExtensionProviderArraysShareTheRequestNodeLimit)
{
    ParamConfig config;
    config.withCustomContent = true;
    InspectorTreeRequestContext context(config, {});
    auto extension = JsonUtil::ParseJsonString(R"({"$children":[{"accessibilityTree":[]}]})");
    auto array = extension->GetValue("$children")->GetArrayItem(0)->GetValue("accessibilityTree");
    for (size_t i = 0; i < 5000; ++i) {
        array->Put(JsonUtil::Create());
    }
    EXPECT_TRUE(context.AcceptExtensionTree(*extension));
    EXPECT_TRUE(context.AcceptExtensionTree(*extension));
    EXPECT_FALSE(context.AcceptExtensionTree(*extension));
    EXPECT_EQ(std::string(context.dependencyResult.GetErrorMessage()), "node count exceeds limit");
}

} // namespace OHOS::Ace::NG

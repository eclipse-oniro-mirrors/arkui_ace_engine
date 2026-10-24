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
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"
#include "test/mock/frameworks/core/common/mock_container.h"

#include "base/json/json_util_internal.h"
#include "core/accessibility/accessibility_session_adapter.h"
#include "core/pipeline/container_window_manager.h"
#include "core/accessibility/native_interface_accessibility_provider.h"
#include "core/accessibility/third_provider_tree_exporter.h"
#include "core/components_ng/base/inspector_tree_request_context.h"
#include "core/components_ng/pattern/custom_frame_node/custom_pattern.h"
#include "core/components_ng/pattern/xcomponent/xcomponent_ext_surface_callback_client.h"
#include "core/components_ng/pattern/xcomponent/xcomponent_pattern.h"

namespace OHOS::Ace::NG {
namespace {
constexpr int64_t PROVIDER_ROOT = (1LL << 54) + 7;
class ScaledInspectorContainer : public MockContainer {
    DECLARE_ACE_TYPE(ScaledInspectorContainer, MockContainer);

public:
    using MockContainer::MockContainer;
    float GetWindowScale() const override { return scale; }
    float scale = 0.5f;
};

struct InspectorProviderInput {
    int calls = 0;
    int rootCalls = 0;
    bool failed = false;
    bool rootFailed = false;
    ArkUI_AccessibilityElementInfo root;
    std::vector<ArkUI_AccessibilityElementInfo> nodes;

    void Initialize()
    {
        root.SetElementId(PROVIDER_ROOT);
        root.SetParentId(-99);
        root.AddChildNodeId(PROVIDER_ROOT + 1);
        root.SetComponentType("ProviderRoot");
        root.SetContents("provider original\n\"中文\"");
        root.SetAccessibilityDescription("provider description");
        root.SetRect({ 110, 230, 130, 250 });
        ArkUI_AccessibilityElementInfo child;
        child.SetElementId(PROVIDER_ROOT + 1);
        child.SetParentId(PROVIDER_ROOT);
        child.SetContents("child");
        child.SetRect({ 115, 235, 120, 240 });
        // Deliberately not in tree traversal order.
        nodes = { child, root };
    }
};
thread_local InspectorProviderInput* providerInput = nullptr;

int32_t QueryInspectorFixture(int64_t id, int32_t mode, int32_t request, ArkUI_AccessibilityElementInfoList* list)
{
    EXPECT_EQ(id, -1);
    if (mode == ARKUI_ACCESSIBILITY_NATIVE_SEARCH_MODE_PREFETCH_CURRENT) {
        ++providerInput->rootCalls;
        if (providerInput->rootFailed) {
            return -1;
        }
        *list->AddAndGetElementInfo() = providerInput->root;
        return 0;
    }
    EXPECT_EQ(mode, ARKUI_ACCESSIBILITY_NATIVE_SEARCH_MODE_PREFETCH_RECURSIVE_CHILDREN);
    ++providerInput->calls;
    if (providerInput->failed) {
        return -1;
    }
    for (const auto& info : providerInput->nodes) {
        *list->AddAndGetElementInfo() = info;
    }
    return 0;
}

class InspectorProviderPattern : public XComponentPattern {
    DECLARE_ACE_TYPE(InspectorProviderPattern, XComponentPattern);

public:
    explicit InspectorProviderPattern(ArkUI_AccessibilityProvider* provider)
    {
        useNodeHandleAccessibilityProvider_ = true;
        arkuiAccessibilityProvider_ = provider;
    }

protected:
    void OnAttachToFrameNode() override {}
    void OnDetachFromFrameNode(FrameNode*) override {}
    void OnDetachFromMainTree() override {}
};
} // namespace

class InspectorTreeTask05Provider : public testing::Test {
protected:
    void SetUp() override
    {
        MockPipelineContext::SetUp();
        MockContainer::SetUp(MockPipelineContext::GetCurrentContext());
        providerInput = &input_;
        input_.Initialize();
        ArkUI_AccessibilityProviderCallbacks callbacks {};
        callbacks.findAccessibilityNodeInfosById = [](int64_t id, ArkUI_AccessibilitySearchMode mode, int32_t request,
                                                       ArkUI_AccessibilityElementInfoList* nodes) -> int32_t {
            return QueryInspectorFixture(id, static_cast<int32_t>(mode), request, nodes);
        };
        callbacks.findAccessibilityNodeInfosByText = [](int64_t, const char*, int32_t,
                                                         ArkUI_AccessibilityElementInfoList*) -> int32_t { return -1; };
        callbacks.findFocusedAccessibilityNode = [](int64_t, ArkUI_AccessibilityFocusType, int32_t,
                                                     ArkUI_AccessibilityElementInfo*) -> int32_t { return -1; };
        callbacks.findNextFocusAccessibilityNode = [](int64_t, ArkUI_AccessibilityFocusMoveDirection, int32_t,
                                                       ArkUI_AccessibilityElementInfo*) -> int32_t { return -1; };
        callbacks.executeAccessibilityAction = [](int64_t, ArkUI_Accessibility_ActionType,
                                                   ArkUI_AccessibilityActionArguments*,
                                                   int32_t) -> int32_t { return -1; };
        callbacks.clearFocusedFocusAccessibilityNode = []() -> int32_t { return 0; };
        callbacks.getAccessibilityNodeCursorPosition = [](int64_t, int32_t, int32_t*) -> int32_t { return -1; };
        callbacks_ = callbacks;
        ASSERT_EQ(provider_.AccessibilityProviderRegisterCallback(&callbacks_), 0);
    }
    void TearDown() override
    {
        providerInput = nullptr;
        MockPipelineContext::TearDown();
        MockContainer::TearDown();
    }
    RefPtr<FrameNode> Host(int32_t id)
    {
        return FrameNode::CreateFrameNode("XComponent", id, AceType::MakeRefPtr<InspectorProviderPattern>(&provider_));
    }
    std::shared_ptr<JsonValue> Dump(const RefPtr<FrameNode>& host, ParamConfig config)
    {
        InspectorTreeRequestContext context(config, {}, {});
        config.inspectorContext = &context;
        auto json = JsonUtil::CreateSharedPtrJson();
        host->DumpInspectorTreeInfo(json, config);
        auto root = JsonUtil::CreateSharedPtrJson();
        root->Put("appInfo", JsonUtil::Create());
        auto children = JsonUtil::CreateArray();
        children->Put(json);
        root->Put("$children", children);
        return context.FinalizeResult(root);
    }
    std::unique_ptr<JsonValue> HostJson(const std::shared_ptr<JsonValue>& tree)
    {
        return tree->GetValue("$children")->GetArrayItem(0);
    }

    ArkUI_AccessibilityProviderCallbacks callbacks_ {};
    InspectorProviderInput input_;
    ArkUI_AccessibilityProvider provider_;
};

TEST_F(InspectorTreeTask05Provider, FlatOutputPreservesFieldsOrderAndLargeIds)
{
    input_.nodes[1].SetGridInfo({ 2, 3, 1 });
    input_.nodes[1].AddOperationAction({ ARKUI_ACCESSIBILITY_NATIVE_ACTION_TYPE_CLICK, "" });
    input_.nodes[1].AddOperationAction({ ARKUI_ACCESSIBILITY_NATIVE_ACTION_TYPE_LONG_CLICK, nullptr });
    auto host = Host(301);
    MockPipelineContext::SetCurrentWindowRect(Rect(100, 200, 800, 600));
    ParamConfig config;
    config.withCustomContent = true;
    auto result = Dump(host, config);
    EXPECT_EQ(result->GetString("errorMessage"), "no error");
    auto node = HostJson(result);
    EXPECT_FALSE(node->Contains("errorMessage"));
    EXPECT_FALSE(node->Contains("accessibilityTreeComplete"));
    EXPECT_FALSE(node->Contains(THIRD_PROVIDER_JSON_CHILDREN));
    auto nodes = node->GetValue("accessibilityTree");
    ASSERT_TRUE(nodes->IsArray());
    ASSERT_EQ(nodes->GetArraySize(), 2);
    auto child = nodes->GetArrayItem(0);
    EXPECT_EQ(child->GetString("elementId"), std::to_string(PROVIDER_ROOT + 1));
    EXPECT_EQ(child->GetString("parentId"), std::to_string(PROVIDER_ROOT));
    EXPECT_EQ(child->GetString("$rect"), "[15.00, 35.00],[20.00,40.00]");
    auto root = nodes->GetArrayItem(1);
    EXPECT_EQ(root->GetString("contents"), input_.root.GetContents());
    EXPECT_EQ(root->GetString("parentId"), "-99");
    EXPECT_EQ(root->GetValue("childNodeIds")->GetArrayItem(0)->GetString(), std::to_string(PROVIDER_ROOT + 1));
    EXPECT_EQ(root->GetValue("screenRect")->GetInt("leftTopX"), 110);
    EXPECT_EQ(root->GetValue("screenRect")->GetInt("rightBottomY"), 250);
    EXPECT_FALSE(root->Contains("$children"));
    EXPECT_EQ(root->GetObject("grid")->GetInt("rowCount"), 2);
    EXPECT_TRUE(root->Contains("gridItem"));
    auto actions = root->GetValue("operationActions");
    ASSERT_EQ(actions->GetArraySize(), 2);
    auto click = actions->GetArrayItem(0);
    EXPECT_EQ(click->GetInt("actionType"), ARKUI_ACCESSIBILITY_NATIVE_ACTION_TYPE_CLICK);
    EXPECT_TRUE(click->Contains("description"));
    EXPECT_EQ(click->GetString("description"), "");
    EXPECT_TRUE(actions->GetArrayItem(1)->GetValue("description")->IsNull());
    EXPECT_EQ(input_.calls, 1);
    EXPECT_EQ(input_.rootCalls, 1);
}

TEST_F(InspectorTreeTask05Provider, CustomContentIsIndependentOfNativeAccessibility)
{
    auto host = Host(302);
    for (bool enabled : { false, true }) {
        for (bool accessibility : { false, true }) {
            ParamConfig config;
            config.withCustomContent = enabled;
            config.accessibilityInfo = accessibility;
            input_.calls = 0;
            auto json = Dump(host, config);
            EXPECT_EQ(HostJson(json)->Contains("accessibilityTree"), enabled);
            EXPECT_FALSE(HostJson(json)->Contains(THIRD_PROVIDER_JSON_CHILDREN));
            EXPECT_EQ(input_.calls, enabled ? 1 : 0);
        }
    }
}

TEST_F(InspectorTreeTask05Provider, CustomHostUsesTheSameReturnChannel)
{
    auto pattern = AceType::MakeRefPtr<CustomPattern>();
    auto host = FrameNode::CreateFrameNode("Custom", 303, pattern);
    auto provider = pattern->GetNativeAccessibilityProvider();
    ASSERT_NE(provider, nullptr);
    ASSERT_EQ(provider->AccessibilityProviderRegisterCallback(&callbacks_), 0);
    ParamConfig config;
    config.withCustomContent = true;
    auto json = Dump(host, config);
    EXPECT_EQ(HostJson(json)->GetValue("accessibilityTree")->GetArraySize(), 2);
    EXPECT_EQ(input_.calls, 1);
}

TEST_F(InspectorTreeTask05Provider, EmptySuccessDiffersFromUnregisteredAndQueryFailure)
{
    auto host = Host(304);
    ParamConfig config;
    config.withCustomContent = true;
    input_.nodes.clear();
    auto result = Dump(host, config);
    EXPECT_EQ(result->GetString("errorMessage"), "no error");
    auto empty = HostJson(result)->GetValue("accessibilityTree");
    EXPECT_TRUE(empty->IsArray());
    EXPECT_EQ(empty->GetArraySize(), 0);
    EXPECT_EQ(input_.rootCalls, 0);
    input_.failed = true;
    EXPECT_EQ(Dump(host, config)->ToString(), R"({"errorMessage":"provider query failed"})");
    ArkUI_AccessibilityProvider unregistered;
    auto other = FrameNode::CreateFrameNode("XComponent", 305,
        AceType::MakeRefPtr<InspectorProviderPattern>(&unregistered));
    result = Dump(other, config);
    EXPECT_FALSE(HostJson(result)->Contains("accessibilityTree"));
    EXPECT_EQ(result->GetString("errorMessage"), "no error");
    EXPECT_EQ(input_.calls, 2);
}

TEST_F(InspectorTreeTask05Provider, DependencyRejectsDuplicateCycleAndMissingAncestor)
{
    auto host = Host(306);
    ParamConfig config;
    config.withCustomContent = true;
    auto good = input_.nodes;
    input_.nodes.push_back(input_.nodes.front());
    EXPECT_EQ(Dump(host, config)->ToString(), R"({"errorMessage":"invalid provider data"})");
    input_.nodes = good;
    input_.nodes.front().SetParentId(PROVIDER_ROOT + 1);
    EXPECT_EQ(Dump(host, config)->ToString(), R"({"errorMessage":"invalid provider data"})");
    input_.nodes = good;
    input_.nodes.front().SetParentId(9876);
    EXPECT_EQ(Dump(host, config)->ToString(), R"({"errorMessage":"invalid provider data"})");
    input_.nodes = good;
    input_.nodes.back().SetParentId(PROVIDER_ROOT);
    EXPECT_EQ(Dump(host, config)->ToString(), R"({"errorMessage":"invalid provider data"})");
    input_.nodes = good;
    input_.nodes.back().AddChildNodeId(9876);
    EXPECT_EQ(Dump(host, config)->ToString(), R"({"errorMessage":"invalid provider data"})");
    input_.rootFailed = true;
    EXPECT_EQ(Dump(host, config)->ToString(), R"({"errorMessage":"provider query failed"})");
}

TEST_F(InspectorTreeTask05Provider, FailureDiscardsEarlierHostsAndStopsSubsequentQueries)
{
    auto host = Host(307);
    ParamConfig config;
    config.withCustomContent = true;
    InspectorTreeRequestContext context(config, {}, {});
    config.inspectorContext = &context;
    auto root = JsonUtil::CreateSharedPtrJson();
    root->Put("appInfo", JsonUtil::Create());
    auto first = JsonUtil::CreateSharedPtrJson();
    host->DumpInspectorTreeInfo(first, config);
    root->Put("earlierHost", first);
    input_.failed = true;
    auto second = JsonUtil::CreateSharedPtrJson();
    host->DumpInspectorTreeInfo(second, config);
    host->DumpInspectorTreeInfo(second, config);
    EXPECT_EQ(input_.calls, 2);
    EXPECT_EQ(context.FinalizeResult(root)->ToString(), R"({"errorMessage":"provider query failed"})");
}

TEST_F(InspectorTreeTask05Provider, BothTraversalEntrancesUseTheFlatReturnChannel)
{
    auto host = Host(308);
    host->SetActive(true);
    for (bool visible : { false, true }) {
        ParamConfig config;
        config.withCustomContent = true;
        InspectorTreeRequestContext context(config, {}, {});
        config.inspectorContext = &context;
        auto root = JsonUtil::CreateSharedPtrJson();
        host->DumpSimplifyTreeWithParamConfig(0, root, visible, config);
        EXPECT_NE(root->ToString().find("\"accessibilityTree\""), std::string::npos);
    }
    EXPECT_EQ(input_.calls, 2);
}

TEST_F(InspectorTreeTask05Provider, DependencyBudgetIsSharedAcrossHosts)
{
    auto host = Host(309);
    ThirdProviderTreeExportBudget budget;
    auto first = ThirdProviderTreeExporter::ExportFlatToInspectorJson(&provider_, host, budget);
    ASSERT_EQ(first.status, ThirdProviderExportStatus::SUCCESS);
    EXPECT_EQ(budget.nodes, 2);
    auto bytes = budget.bytes;
    auto second = ThirdProviderTreeExporter::ExportFlatToInspectorJson(&provider_, host, budget);
    ASSERT_EQ(second.status, ThirdProviderExportStatus::SUCCESS);
    EXPECT_EQ(budget.nodes, 4);
    EXPECT_EQ(budget.bytes, bytes * 2);
    budget.nodes = 9999;
    auto overflow = ThirdProviderTreeExporter::ExportFlatToInspectorJson(&provider_, host, budget);
    EXPECT_EQ(overflow.status, ThirdProviderExportStatus::NODE_LIMIT);
    EXPECT_EQ(overflow.nodes, nullptr);
}

TEST_F(InspectorTreeTask05Provider, DependencyCountsFinalUtf8ArraysIncludingEscapesAndRects)
{
    auto host = Host(310);
    ThirdProviderTreeExportBudget budget;
    auto result = ThirdProviderTreeExporter::ExportFlatToInspectorJson(&provider_, host, budget);
    ASSERT_EQ(result.status, ThirdProviderExportStatus::SUCCESS);
    const auto bytes = budget.bytes;
    auto envelope = JsonUtil::Create();
    envelope->Put("accessibilityTree", result.nodes);
    char* data = nullptr;
    size_t size = 0;
    ASSERT_EQ(WriteJsonBounded(*envelope, false, 10 * 1024 * 1024, &data, &size), JsonWriteResult::SUCCESS);
    std::free(data);
    EXPECT_EQ(bytes, size - std::string("{\"accessibilityTree\":}").size());
    budget = { 0, 10 * 1024 * 1024 - bytes };
    result = ThirdProviderTreeExporter::ExportFlatToInspectorJson(&provider_, host, budget);
    EXPECT_EQ(result.status, ThirdProviderExportStatus::SUCCESS);
    EXPECT_EQ(budget.bytes, 10 * 1024 * 1024);
    budget = { 0, 10 * 1024 * 1024 - bytes + 1 };
    result = ThirdProviderTreeExporter::ExportFlatToInspectorJson(&provider_, host, budget);
    EXPECT_EQ(result.status, ThirdProviderExportStatus::SIZE_LIMIT);
    EXPECT_EQ(result.nodes, nullptr);
}
TEST_F(InspectorTreeTask05Provider, CoordinatesFollowTheNativeFloatingWindowScale)
{
    auto pipeline = MockPipelineContext::GetCurrentContext();
    auto container = AceType::MakeRefPtr<testing::NiceMock<ScaledInspectorContainer>>(pipeline);
    MockContainer::container_ = container;
    MockPipelineContext::SetCurrentWindowRect(Rect(100, 200, 800, 600));
    auto host = Host(311);
    ParamConfig config;
    config.withCustomContent = true;
    for (bool floating : { false, true }) {
        pipeline->GetWindowManager()->SetWindowGetModeCallBack([floating]() {
            return floating ? WindowMode::WINDOW_MODE_FLOATING : WindowMode::WINDOW_MODE_FULLSCREEN;
        });
        auto result = Dump(host, config);
        auto child = HostJson(result)->GetValue("accessibilityTree")->GetArrayItem(0);
        EXPECT_EQ(child->GetString("$rect"), floating ? "[30.00, 70.00],[40.00,80.00]" :
            "[15.00, 35.00],[20.00,40.00]");
        EXPECT_EQ(child->GetValue("screenRect")->GetInt("leftTopX"), 115);
    }
}

TEST_F(InspectorTreeTask05Provider, NonInvertibleWindowScaleFailsTheWholeRequest)
{
    auto pipeline = MockPipelineContext::GetCurrentContext();
    auto container = AceType::MakeRefPtr<testing::NiceMock<ScaledInspectorContainer>>(pipeline);
    MockContainer::container_ = container;
    pipeline->GetWindowManager()->SetWindowGetModeCallBack([]() { return WindowMode::WINDOW_MODE_FLOATING; });
    ParamConfig config;
    config.withCustomContent = true;
    auto host = Host(312);
    for (float scale : { 0.0f, -1.0f }) {
        container->scale = scale;
        EXPECT_EQ(Dump(host, config)->ToString(), R"({"errorMessage":"collection failed"})");
    }
}

TEST_F(InspectorTreeTask05Provider, CustomContentIsCollectedOncePerRequest)
{
    auto host = Host(313);
    ParamConfig config;
    config.withCustomContent = true;
    auto result = Dump(host, config);
    auto node = HostJson(result);
    EXPECT_TRUE(node->Contains("accessibilityTree"));
    EXPECT_FALSE(node->Contains(THIRD_PROVIDER_JSON_CHILDREN));
    EXPECT_EQ(input_.calls, 1);
    EXPECT_EQ(input_.rootCalls, 1);
    EXPECT_TRUE(config.withCustomContent);
}

} // namespace OHOS::Ace::NG

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

#include "core/accessibility/accessibility_session_adapter.h"
#include "core/accessibility/native_interface_accessibility_provider.h"
#include "core/accessibility/third_provider_tree_exporter.h"
#include "core/components_ng/base/inspector_tree_request_context.h"
#include "core/components_ng/pattern/custom_frame_node/custom_pattern.h"
#include "core/components_ng/pattern/xcomponent/xcomponent_ext_surface_callback_client.h"
#include "core/components_ng/pattern/xcomponent/xcomponent_pattern.h"

namespace OHOS::Ace::NG {
namespace {
struct InspectorProviderInput {
    int calls = 0;
    bool empty = false;
    bool failed = false;
};
thread_local InspectorProviderInput* providerInput = nullptr;
constexpr int64_t PROVIDER_ROOT = 1234;

int32_t QueryInspectorFixture(int64_t id, int32_t mode, int32_t request, ArkUI_AccessibilityElementInfoList* list)
{
    ++providerInput->calls;
    EXPECT_EQ(id, -1);
    EXPECT_EQ(mode, ARKUI_ACCESSIBILITY_NATIVE_SEARCH_MODE_PREFETCH_RECURSIVE_CHILDREN);
    if (providerInput->empty || providerInput->failed) {
        return providerInput->failed ? -1 : 0;
    }
    auto root = list->AddAndGetElementInfo();
    if (!root) {
        return -1;
    }
    root->SetElementId(PROVIDER_ROOT);
    root->SetParentId(-1);
    root->AddChildNodeId(PROVIDER_ROOT + 1);
    root->SetComponentType("ProviderRoot");
    root->SetContents("provider original");
    root->SetAccessibilityDescription("provider description");
    root->SetRect({ 0, 0, 20, 20 });
    auto child = list->AddAndGetElementInfo();
    if (!child) {
        return -1;
    }
    child->SetElementId(PROVIDER_ROOT + 1);
    child->SetParentId(PROVIDER_ROOT);
    child->SetContents("child");
    child->SetRect({ 5, 5, 10, 10 });
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
        providerInput = &input_;
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
        context.FinalizeNativeTree(json);
        return json;
    }
    ArkUI_AccessibilityProviderCallbacks callbacks_ {};
    InspectorProviderInput input_;
    ArkUI_AccessibilityProvider provider_;
};

TEST_F(InspectorTreeTask05Provider, ReturnsExporterJsonWithoutReencodingOrSecondQuery)
{
    auto host = Host(301);
    auto expected = ThirdProviderTreeExporter::ExportToInspectorJson(&provider_, host);
    ASSERT_NE(expected, nullptr);
    input_.calls = 0;
    ParamConfig config;
    config.withXComponent = true;
    auto json = Dump(host, config);
    ASSERT_TRUE(json->Contains(THIRD_PROVIDER_JSON_CHILDREN));
    EXPECT_EQ(json->GetValue(THIRD_PROVIDER_JSON_CHILDREN)->ToString(), expected->ToString());
    EXPECT_EQ(input_.calls, 1);
    EXPECT_FALSE(json->Contains("accessibilityTree"));
    EXPECT_FALSE(json->Contains("accessibilityTreeComplete"));
    EXPECT_FALSE(json->Contains("errorMessage"));
    EXPECT_FALSE(config.withThirdProvider);
}

TEST_F(InspectorTreeTask05Provider, BothSwitchesReachTheSameHookIndependentlyOfNativeA11y)
{
    auto host = Host(302);
    for (bool direct : { false, true }) {
        for (bool compatibility : { false, true }) {
            for (bool accessibility : { false, true }) {
                ParamConfig config;
                config.withThirdProvider = direct;
                config.withXComponent = compatibility;
                config.accessibilityInfo = accessibility;
                input_.calls = 0;
                auto json = Dump(host, config);
                EXPECT_EQ(json->Contains(THIRD_PROVIDER_JSON_CHILDREN), direct || compatibility);
                EXPECT_EQ(input_.calls, direct || compatibility ? 1 : 0);
            }
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
    auto expected = ThirdProviderTreeExporter::ExportToInspectorJson(provider, host);
    ASSERT_NE(expected, nullptr);
    input_.calls = 0;
    ParamConfig config;
    config.withXComponent = true;
    auto json = Dump(host, config);
    ASSERT_TRUE(json->Contains(THIRD_PROVIDER_JSON_CHILDREN));
    EXPECT_EQ(json->GetValue(THIRD_PROVIDER_JSON_CHILDREN)->ToString(), expected->ToString());
    EXPECT_EQ(input_.calls, 1);
}

TEST_F(InspectorTreeTask05Provider, EmptyFailureAndUnregisteredFollowExporterOmission)
{
    auto host = Host(304);
    ParamConfig config;
    config.withXComponent = true;
    input_.empty = true;
    EXPECT_FALSE(Dump(host, config)->Contains(THIRD_PROVIDER_JSON_CHILDREN));
    input_.empty = false;
    input_.failed = true;
    EXPECT_FALSE(Dump(host, config)->Contains(THIRD_PROVIDER_JSON_CHILDREN));
    EXPECT_EQ(input_.calls, 2);
    ArkUI_AccessibilityProvider unregistered;
    auto other =
        FrameNode::CreateFrameNode("XComponent", 305, AceType::MakeRefPtr<InspectorProviderPattern>(&unregistered));
    EXPECT_FALSE(Dump(other, config)->Contains(THIRD_PROVIDER_JSON_CHILDREN));
    EXPECT_EQ(input_.calls, 2);
}

TEST_F(InspectorTreeTask05Provider, FullTreeTraversalCarriesTheNestedProviderTree)
{
    auto host = Host(306);
    ParamConfig config;
    config.withXComponent = true;
    InspectorTreeRequestContext context(config, {}, {});
    config.inspectorContext = &context;
    auto root = JsonUtil::CreateSharedPtrJson();
    host->DumpSimplifyTreeWithParamConfig(0, root, false, config);
    EXPECT_NE(root->ToString().find("\"$children-third\""), std::string::npos);
    EXPECT_EQ(input_.calls, 1);
}
} // namespace OHOS::Ace::NG
